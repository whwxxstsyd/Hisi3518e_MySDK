#include "parseformdata.h"
#include "string_parser.h"
#include "param.h"
#include "common.h"
#include "debug.h"
#include "socket.h"
#include "normal.h"
#include "param_ext.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <fcntl.h>
 
int main(int argc, char *argv[])
{   
	int sockfd = -1, opt;
    char *CgiStr = NULL, *ReqData = NULL, RespData[STR_1024_LEN * 3] = {0}, str[STR_1024_LEN] = {0};
	char **formlist = NULL, *user = NULL, *pwd = NULL;
	int user_level = 0;	

	printf("Content-type: text/html\n\n");
	printf("<TITLE> Multi S</TITLE>");
	printf("<H3>Multi S</H3>");

	return 1;

	if (!strcmp("set_param.cgi", get_file_name(argv[0])))
		opt = SET_PARAM;
	else if (!strcmp("get_param.cgi", get_file_name(argv[0])))
		opt = GET_PARAM;
	else if (!strcmp("cmd_proc.cgi", get_file_name(argv[0])))
		opt = CMD_PROC; 
	else
	{
		sprintf(str, "var error_string = \"error cgi[%s]\"\r\n", argv[0]);
		goto ERROR_OUT;
	}
	
	/* 用户验证 */
    formlist = getCGIvars();
	
	user = findCGIvar(formlist, (char *)"user");   
	pwd  = findCGIvar(formlist, (char *)"pwd");	
	if ((user == NULL) || (pwd == NULL))
		goto AUTH_FAIL;

	user_level = check_user(user, pwd);	

	printf("var user_level = \"%s\";\r\n", "nihao");	
	printf("\r\n");			/* 否则会导致socket close_wait */


	return 0;

	
	if (user_level < 0)
		goto AUTH_FAIL;		
	
	/* 创建socket与CgiServer通信 */
	if ((sockfd = tcp_block_connect(LOCAL_HOST, CGI_PORT)) < 0)
	{
		sprintf(str, "var error_string = \"tcp_block_connect fail[%s_%d]\"\r\n", LOCAL_HOST, CGI_PORT);
		goto ERROR_OUT;
	}
	set_sock_recvtimeout(sockfd, 10000);		/* ftp/email测试时间比较久 */
	/* 发送表单 */
	CgiStr = getCGIstr();
	ReqData = (char *)calloc(1, strlen(CgiStr) + STR_32_LEN);
	sprintf(ReqData, "opt=%d&%s", opt, CgiStr);
	if (CgiStr)
		free(CgiStr);	
	if (tcp_block_send(sockfd, ReqData, strlen(ReqData)) < 0)
	{
		sprintf(str, "var error_string = \"tcp_block_send fail\"\r\n");
		goto ERROR_OUT;
	}
	
	/* 将tcp_block_recv_once 替换成 tcp_block_recv */
	if (tcp_block_recv_once(sockfd, RespData, sizeof(RespData) - 1) < 0)
	{
		sprintf(str, "var error_string = \"tcp_block_recv fail\"\r\n");
		goto ERROR_OUT;
	}	

	close_socket(&sockfd);
	if (ReqData)
		free(ReqData);
	freeCGIvars(formlist);
	printf(HTTP_HEAD);
	printf("var user_level = \"%s\";\r\n", Index2Str(user_level));	
	printf("%s", RespData);
	printf("\r\n");			/* 否则会导致socket close_wait */
	
    return 0;

AUTH_FAIL:
	close_socket(&sockfd);
	freeCGIvars(formlist);
	printf(HTTP_HEAD);
	printf("var user_level = \"null\";\r\n");
	printf("var auth_result = \"user or password error\";\r\n");
	
	return -1;

ERROR_OUT:
	close_socket(&sockfd);	
	if (CgiStr)
		free(CgiStr);		
	if (ReqData)	
		free(ReqData);
	if (formlist)
		freeCGIvars(formlist);
	printf(HTTP_HEAD);
	printf(str);
	
	return -1;
}

