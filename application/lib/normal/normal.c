#include "common.h"
#include "normal.h"
#include "debug.h"
#include "param.h"
#include "md5.h"
#include "socket.h"
#include "linux_list.h"
#include "wifi.h"
#include "param_base.h"

#include <stdlib.h>     /* for malloc */
#include <stdio.h>
#include <string.h>
#include <fcntl.h>
#include <unistd.h>
#include <sys/mount.h>
#include <sys/ipc.h>
#include <sys/shm.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/vfs.h>
#include <sys/time.h>
#include <sys/syscall.h>
#include <unistd.h> 
#include <errno.h>
#include <pthread.h>
#include <signal.h>
#include <dirent.h>

#undef		TARGET_LEVEL
#define		TARGET_LEVEL		LEVEL_INFO

/* for show time diff */
static struct timeval start_time;

void init_start_time(void)
{
    gettimeofday(&start_time, NULL);
}
void show_time_diff(char *tag)
{
    int time_ms;
    struct timeval time;

    gettimeofday(&time, NULL);
    time_ms = (time.tv_sec - start_time.tv_sec) * 1000 + (time.tv_usec - start_time.tv_usec) / 1000;
    memcpy(&start_time, &time, sizeof(time));
	LOG_INFO("%s:\ttime_diff(ms): %d\n", tag, time_ms);    
}
/* end show time diff */

void show_time(const char *tag)
{
    struct timeval time;

    gettimeofday(&time, NULL);
	LOG_INFO("%s:\t%ld:%ld\n", tag, time.tv_sec, time.tv_usec/1000);
}

void *system_param_map(unsigned int param_offset)
{
	int ShmID;
	static struct SYSTEM_PARAM_T *pSystemParam = NULL;

    ShmID = shmget(KEY_SHM_PARAM, sizeof(SYSTEM_PARAM), IPC_CREAT | 0666);  /* 在创建共享内存时,内存以及被初始化为0 */
    if (ShmID < 0)
    {
        LOG_ERROR("shmget error\n");
        return NULL;
    }

    if (pSystemParam == NULL)       /* 避免重复映射共享内存, 在同一个进程中只映射一次 */
    {
    //pSystemParam存储了共享内存地址
        pSystemParam =(SYSTEM_PARAM *) shmat(ShmID, (void *)0, 0);
        if (pSystemParam == NULL)
        {
            LOG_ERROR("shmat error\n");
            return NULL;
        }
    }
//返回值为共享内存的地址加上偏移值
//用于下次在调用这个函数的时候在原来的偏移地址上往上加
	return (char *)pSystemParam + param_offset;	
}

/* 使用完记得释放内存 */
char *my_strdup(const char *str)
{
	char *newstr = NULL;

	if (str) 
	{
		newstr = (char *)malloc(strlen(str) + 1);
		if (newstr) 
		{
			memset(newstr, 0, strlen(str) + 1);
			strcpy(newstr, str);
		}
	}

	return newstr;
}

const char bb_hexdigits_upcase[] = "0123456789ABCDEF";
char* bin2hex(char *p, const unsigned char *cp, int count)
{
	while (count) {
		unsigned char c = *cp++;
		/* put lowercase hex digits */
		*p++ = 0x20 | bb_hexdigits_upcase[c >> 4];
		*p++ = 0x20 | bb_hexdigits_upcase[c & 0xf];
		count--;
	}
	return p;
}

int checksum(char *file)
{
    int fd = -1;
    char read_value[CHECKSUM_LEN + 1] = {0};
    unsigned char real_value_bin[16] = {0};
    char real_value[CHECKSUM_LEN + 1] = {0};   
    struct stat statbuff;

    PARAM_OK(file);
    
    if (copy_file(file, CHECKFILE_PATH) != SDK_OK)
    {
        LOG_ERROR("error at copy file\n");
        return SDK_ERROR;        
    }
    fd = open(CHECKFILE_PATH, O_RDWR);
    if (fd < 0)
    {
        LOG_ERROR("error at open\n");
        return SDK_ERROR;
    }
    lseek(fd, -(CHECKSUM_LEN + 1), SEEK_END);   /* include a enter */
    read(fd, read_value, CHECKSUM_LEN);
    /* get file size */
    stat(CHECKFILE_PATH, &statbuff);
    /* cut of file */
    ftruncate(fd, statbuff.st_size - CHECKSUM_STRING_LEN - 1);   /* include a enter */
    close(fd);

    md5_file(CHECKFILE_PATH, real_value_bin);
    unlink(CHECKFILE_PATH);
    bin2hex(real_value, real_value_bin, sizeof(real_value_bin));
//    LOG_DEBUG("read: %s\n", read_value);
//    LOG_DEBUG("real: %s\n", real_value);        

    return !strcmp(read_value, real_value);
}

/* include a enter char */
void add_checksum(char *file)
{  
    char cmd[512] = {0};    

#if 0
    PARAM_OK(file);
#endif

    sprintf(cmd, "md5sum %s | awk {'print \"checksum =\", $1'} >> %s", file, file);
    system(cmd);
}

int copy_file(char *src, char *dst)
{
	int src_fd, dst_fd, ret;
	char buf[1024] = {0};
	
	PARAM_OK(src && dst);

	src_fd = open(src, O_RDONLY);
	if (src_fd < 0)
	{
		LOG_ERROR("error at open file\n");
		return SDK_ERROR;
	}

	dst_fd = open(dst, O_WRONLY | O_CREAT | O_TRUNC);
	if (dst_fd < 0)
	{
		LOG_ERROR("error at open file\n");
		return SDK_ERROR;
	}

	while (1)
	{
		ret = read(src_fd, buf, sizeof(buf));
		if (ret == 0)
			break;
		write(dst_fd, buf, ret);
	}

	close(src_fd);
	close(dst_fd);

	return SDK_OK;
}

#if 1
/* for hash */
unsigned long hash_string(const char *str)
{   
/* 
	unsigned long hash = 0x71e2c3d8;
	
    while (*str != 0)    
    {        
        hash <<= 1;       
        hash += *str++;      
    }   
    return hash;
*/

	
	unsigned int h=0;
	for(;*str;str++)
	{		
		h=*str+h*27;
	}
	return h;
}

int get_hash_pos(const char *str)
{
    return (hash_string(str) % HASH_MAP_SIZE);
}
/* end */
#endif

/* for string parser */
static char x2c(char *what) 
{
   register char digit;

   digit = (what[0] >= 'A' ? ((what[0] & 0xdf) - 'A') + 10 : (what[0] - '0'));
   digit *= 16;
   digit += (what[1] >= 'A' ? ((what[1] & 0xdf) - 'A') + 10 : (what[1] - '0'));

   return digit;
}

static void unescape_url(char *url) 
{
    register int i,j;

    for(i = 0, j = 0; url[j]; ++i,++j) 
    {
        if((url[i] = url[j]) == '%') 
        {
            url[i] = x2c(&url[j + 1]) ;
            j += 2 ;
        }
    }

    url[i] = '\0' ;
}

char **ParserString(const char *string)
{
    int             i, paircount;
  	char			**vars, **pairlist;  
	char		    *nvpair, *eqpos, *new_str = NULL;    

    pairlist = (char **)malloc(256 * sizeof(char **));
    paircount = 0;
    new_str = strdup(string);
    nvpair = strtok(new_str, "&");

    while (nvpair) 
    {
        pairlist[paircount++] = strdup(nvpair);
        if (!(paircount % 256)) 
            pairlist = (char **)realloc(pairlist, (paircount + 256) * sizeof(char **));

        nvpair = strtok(NULL, "&") ;
    }
    pairlist[paircount] = 0;
  
    vars = (char **)malloc((paircount * 2 + 1) * sizeof(char **));
    for (i = 0; i < paircount; i++) 
    {
        if ((eqpos = strchr(pairlist[i], '=')) != NULL)
        {
            *eqpos = '\0' ;
            unescape_url(vars[i * 2 + 1] = strdup(eqpos + 1));
        } 
        else 
        {
            unescape_url(vars[i * 2 + 1] = strdup(""));
        }
        unescape_url(vars[i * 2] = strdup(pairlist[i]));
    }
    vars[paircount * 2] = 0;	

    if (new_str) free(new_str);
    for (i = 0; pairlist[i]; i++) 
    {
		free(pairlist[i]);
	}
    free(pairlist);
    
    return vars;
}

void FreeParserVars(char *list[])
{
	char	**head;
	
	head = list;
	while (*list)
	{
		free(*list);
		list++;
	}
	
	if (head)
	{
		free(head);
	}
}
/* end for string parser */

#define     POINT_UNIT      (128)
char **StrParser(char *str)     /* 有问题,用ParserString代替 */
{
    char *tmp = strdup(str);
    int i = 0, count = 1;
    char *p;
    char **StrList;

    StrList = (char **)malloc(sizeof(char *) * POINT_UNIT);
    memset(StrList, 0, sizeof(char *) * POINT_UNIT);
    p = StrList[i] = tmp;
    while (*tmp)
    {
        if (*tmp == '=')
        {
            p = tmp;
            StrList[i + 1] = tmp + 1;
        }
        if (*tmp == '&')
        {
            if (p)
            {
                i += 2;
                if (i > (POINT_UNIT * count))
                {
                    count++;
                    StrList = (char **)realloc(StrList, sizeof(char *) * POINT_UNIT * count);
                    memset(StrList + sizeof(char *) * POINT_UNIT * (count - 1), 
                                      0, sizeof(char *) * POINT_UNIT);
                }
                StrList[i] = tmp + 1;
                *p = 0;
                *tmp = 0;
                p = NULL;
            }
        }
        tmp++;
    }
    if (p)                          /* 把最后的'='清零 */
        *p = 0;     
    if (StrList[i + 1] == NULL)     /* 如果字符串末尾没有'='，清除最后一个指针 */
        StrList[i] = NULL;

    return StrList;
}

void FreeStrList(char **str)
{
    if (*str)
        free(*str);
    if (str)
        free(str);
}

char *safe_strncpy(char *dst, const char *src, int dst_size)
{
    dst[dst_size-1] = '\0';
    return strncpy(dst, src, dst_size-1);
}


char *my_itoa(int num, char *str, int base)
{
	char index[] = "0123456789ABCDEFGHIJKLMNOPQRSTUVWXYZ";
	unsigned int unum;
	int i = 0, j, k;

	if (base == 10 && num < 0) {
		unum = (unsigned int)- num;
		str[i++] = '-';
	} else {
		unum = (unsigned int)num;
	}

	do {
		str[i++] = index[unum % (unsigned int)base];
		unum /= base;
	} while (unum);
	str[i] = '\0';

	if (str[0] == '-') {
		k = 1;
	} else {
		k = 0;
	}
	for (j = k; j <= (i - 1) / 2 + k; ++j) {
		num = str[j];
		str[j] = str[i-j-1+k];
		str[i-j-1+k] = num;
	}

	return str;
}

int check_ip(char *ip)
{
	return SDK_OK;
}

int check_netmask(char *ip)
{
	return SDK_OK;
}

int check_getway(char *ip)
{
	return SDK_OK;
}

int daemon(int nochdir, int noclose)
{
	int fd;

	switch (fork()) 
	{
		case -1:
			return (-1);
		case 0:
			break;
		default:
			_exit(0);
	}

	if (setsid() == -1)
		return (-1);

	if (!nochdir)
		chdir("/");

	/* 设置成/dev/null有问题 */
	if (!noclose && (fd = open("/dev/console", O_RDWR, 0)) != -1) 
	{
		dup2(fd, STDIN_FILENO);
		dup2(fd, STDOUT_FILENO);
		dup2(fd, STDERR_FILENO);
		if (fd > 2)
			close (fd);
	}
	
	return (0);
}

int write_data_to_file(const char *file, char *data, int len)
{
	int fd;
	int ret;
	
	if ((file == NULL) || (data == NULL) || (len == 0))
		return -1;

	if ((fd = open(file, O_CREAT | O_TRUNC | O_WRONLY)) < 0)		/* O_TRUNC: 文件存在覆盖 */
	{
		LOG_INFO("open file error\n");
		return -1;
	}

	ret = write(fd, data, len);
	if (ret != len)
	{
		LOG_INFO("write data error: %d\n", ret);
		close(fd);
		return -1;
	}

	close(fd);
	return 0;
}

int read_data_from_file(const char *file, char *data, int *len)
{
    int fd, ret, index = 0;

 	if ((file == NULL) || (data == NULL) || (len == NULL))
		return -1; 

	if ((fd = open(file, O_RDONLY)) < 0)		/* O_TRUNC: 文件存在覆盖 */
	{
		LOG_INFO("open file error\n");
		return -1;
	} 
    while (1)
    {
        ret = read(fd, data + index, 1024);
        if (ret == 0)    /* 到达文件末尾 */
            break;
        index += ret;
    }
    close(fd);
    *len = index;

    return 0;
}

int sdk_select(int fd, int fd_type, int time_out)
{
	int ret;
	struct timeval tv, *p_tv;
	fd_set read_fd, *p_read_fd;
	fd_set write_fd, *p_write_fd;
	fd_set except_fd, *p_except_fd;

	tv.tv_sec = time_out / 1000;
    tv.tv_usec = (time_out % 1000) * 1000;
	p_read_fd = p_write_fd = p_except_fd = NULL;
	if (time_out <= 0)
		p_tv = NULL;
	else
		p_tv = &tv;

	switch (fd_type)
	{
		case READ_FD:
			p_read_fd = &read_fd;
			FD_ZERO(p_read_fd);
			FD_SET(fd, p_read_fd);				
			break;
		case WRITE_FD:
			p_write_fd = &write_fd;			
			FD_ZERO(p_write_fd);			
			FD_SET(fd, p_write_fd);			
			break;
		case EXCEPT_FD:
			p_except_fd = &except_fd;			
			FD_ZERO(p_except_fd);			
			FD_SET(fd, p_except_fd);			
			break;
		default:
			return SELECT_ERROR;
			break;
	}
	while (1)
	{
		ret = select(fd + 1, p_read_fd, p_write_fd, p_except_fd, p_tv);
		if ((ret < 0) && (errno == EINTR))
		{
		    continue;
		}    
		else if (ret < 0)
			return SELECT_ERROR;
		else if (ret == 0)
			return SELECT_TIMEOUT;
		else
			return SELECT_OK;
	}
}

int check_frame(REAL_FRAME *frame)
{
	FRAME_HEAD *frame_head;

	frame_head = (FRAME_HEAD *)frame->data;
	if (frame->len != (frame_head->len + sizeof(FRAME_HEAD)))
	{
        LOG_INFO("len: [%d:%d], head:[%d] type:0x%x\n", 
            frame->len, frame_head->len, sizeof(FRAME_HEAD), frame->type);
		return SDK_ERROR;
    }
	if (strncmp(frame_head->mark, TWS_MARK, strlen(TWS_MARK)))
	{
		LOG_INFO("frame_flag: 0x%x 0x%x 0x%x 0x%x\n", 
				frame_head->mark[0], frame_head->mark[1], frame_head->mark[2], frame_head->mark[3]);
		return SDK_ERROR;
	}
	
	return SDK_OK;
}

void roi_to_str(ROI roi, char *str)
{
	sprintf(str, "%d_%d_%d_%d", roi.x, roi.x, roi.x, roi.x);
}

void str_to_roi(ROI *roi, char *str)
{
	sscanf(str, "%u_%u_%u_%u", (unsigned int *)&roi->x, (unsigned int *)&roi->y, 
								(unsigned int *)&roi->width, (unsigned int *)&roi->height);
}

char *get_file_name(const char *path)
{
    char *pStr = NULL;
    
	if (path[strlen(path) - 1] == '/')
		return NULL;

    pStr = strrchr(path, '/');
    if (NULL == pStr)
        return (char *)path;
    else
        return pStr + 1;
}

char *get_dir_name(const char *path, char *dir)
{
    char *pStr = NULL;
    
	if (path[strlen(path) - 1] == '/')
		return NULL;

	pStr = strrchr(path, '/');
    if (pStr == NULL)
        return dir;
    strncpy(dir, path, (int)(pStr - path));

    return dir;
}

int path_exist(const char *path)
{
    if (access(path, F_OK) == 0)
        return 0;
    else
        return -1;
}

int save_file_to_sd(const char *path, const char *sd_path)
{
	int ret;
	char data_buf[1024] = {0};
	int src_fd, dst_fd;

	if ((src_fd = open(path, O_RDONLY)) < 0)
	{
		LOG_INFO("error at open file\n");
		return SDK_ERROR;
	}
	if ((dst_fd = open(sd_path, O_CREAT | O_TRUNC | O_WRONLY)) < 0)		/* O_TRUNC: 文件存在覆盖 */
	{
		LOG_INFO("open file error\n");
		return -1;
	}	

	while(1)
	{
		ret = read(src_fd, data_buf, sizeof(data_buf));
		if (ret == 0)
			goto COPY_OK;
		if (ret < 0)
			goto IO_ERROR;
		ret = write(dst_fd, data_buf, ret);
		if (ret < 0)
			goto IO_ERROR;
	}

COPY_OK:
	close(src_fd);
	close(dst_fd);
	return SDK_OK;
	
IO_ERROR:
	close(src_fd);
	close(dst_fd);
	return SDK_ERROR;
}

/* 2014-09-10 11:20:11 */
int set_time(const char *value)
{  
#if 0    
    char cmd[STR_64_LEN] = {0};
    
    sprintf(cmd, "date -s %s", value);
    if (system(cmd) == 0)
        sprintf(value, "ok");
    else
        sprintf(value, "fail");
#else
    struct tm new_tm;
    time_t tm;
   
    sscanf(value, "%4d-%2d-%2d %2d:%2d:%2d", 
                &new_tm.tm_year, &new_tm.tm_mon, &new_tm.tm_mday, 
                &new_tm.tm_hour, &new_tm.tm_min, &new_tm.tm_sec);
    new_tm.tm_year -= 1900;
    new_tm.tm_mon--;
    
    tm = mktime(&new_tm);
    if (tm < 0)
    {
        LOG_ERROR("mktime fail[%s]\n", value);
        return SDK_ERROR;
    }
    stime(&tm);
#endif    

    return SDK_OK;
}

int parse_url(const char *url, char *server, int *port, char *path)
{
	char *pre = (char *)url;
	char *next = NULL;

	if (strchr(pre, '?'))				/* 不处理get参数 */
		return SDK_ERROR;

	if (!strncmp(pre, "http://", strlen("http://")))
		pre += strlen("http://");

	next = strpbrk(pre, ":/");	
	if (next == NULL)					/* http://192.168.0.11 */
	{
		strcpy(server, pre);
		strcpy(path, "/");
		*port = 80;
		return SDK_OK;
	}
	if (*next == ':')			
	{
		strncpy(server, pre, next - pre);
		next++;
		sscanf(next, "%d", port);
		if ((*port <= 0) || (*port >= 65535))
			return SDK_ERROR;
		next = strchr(next, '/');
		if (next != NULL)				/* http://192.168.0.11:7777/dyndns/get_ip */
			strcpy(path, next);
		else							/* http://192.168.0.11:7777 */
			strcpy(path, "/");
		return SDK_OK;
	}	
	else if (*next == '/')			/* http://192.168.0.11/dyndns/get_ip */
	{
		strncpy(server, pre, next - pre);
		strcpy(path, next);
		*port = 80;
		return SDK_OK;
	}
	return SDK_ERROR;
}



static char get_request_format[] =
			"GET %s?%s HTTP/1.1\r\n"
			"Host: %s:%d\r\n"
#if 0			
			"Accept: text/html, application/xhtml+xml, */*\r\n"
			"Accept-Language: zh-CN\r\n"
			"User-Agent: Mozilla/5.0 (compatible; MSIE 9.0; Windows NT 6.1; WOW64; Trident/5.0)\r\n"
			"Accept-Encoding: gzip, deflate\r\n"
			"Connection: Keep-Alive\r\n\r\n";
#else
            "Connection: keep-alive\r\n"
            "Accept: text/html,application/xhtml+xml,application/xml;q=0.9,image/webp,*/*;q=0.8\r\n"
            "User-Agent: Mozilla/5.0 (Windows NT 6.1; WOW64) AppleWebKit/537.36 (KHTML, like Gecko) Chrome/43.0.2357.130 Safari/537.36\r\n"
            "Accept-Encoding: gzip, deflate, sdch\r\n"
            "Accept-Language: zh-CN,zh;q=0.8\r\n\r\n";
#endif

int http_get_request(const char *url, char *param, char *resp)
{
	int ret;
	int fd;
	char server[STR_64_LEN] = {0}; 
	char path[STR_128_LEN] = {0};
	char send_buf[STR_1024_LEN * 4] = {0};
	char recv_buf[STR_1024_LEN] = {0};	
	char *pStr;
	int port = 0;

	if ((url == NULL) || (resp == NULL))
	{
		LOG_ERROR("param error\n");
		return SDK_ERROR;
	}
	
	if (parse_url((char *)url, server, &port, path) != SDK_OK)
	{
		LOG_ERROR("parse_url fail\n");
		return SDK_ERROR;
	}
	fd = tcp_noblock_connect(server, port, 5000);
	if (fd < 0)
	{
		LOG_ERROR("tcp_block_connect fail\n");
		return SDK_ERROR;
	}
    set_sock_attr(fd, 1, 10000, 10000, 2048, 2048);

	if (param != NULL)	
		sprintf(send_buf, get_request_format, path, param, server, port);
	else
		sprintf(send_buf, get_request_format, path, "", server, port);
	
//    LOG_INFO("send_buf[%s]\n", send_buf);
	ret = tcp_noblock_send(fd, send_buf, strlen(send_buf));
	if (ret != strlen(send_buf))
	{
		LOG_ERROR("tcp_block_send fail\n");
		return SDK_ERROR;
	}
	ret = tcp_noblock_recv(fd, recv_buf, sizeof(recv_buf) - 1);
	if (ret <= 0)
	{
		LOG_ERROR("tcp_block_recv fail\n");
		return SDK_ERROR;
	}
//    LOG_INFO("recv_buf[%s]\n", recv_buf);

	pStr = strstr(recv_buf, "\r\n\r\n") + strlen("\r\n\r\n");
	strncpy(resp, pStr, strlen(pStr));

    close_socket(&fd);
	
	return SDK_OK;
}


static char curl_request_format[] =
			"GET %s HTTP/1.1\r\n"			
			"User-Agent: curl/7.19.7 (i486-pc-linux-gnu) libcurl/7.19.7 OpenSSL/0.9.8k zlib/1.2.3.3 libidn/1.15\r\n"
			"Host: %s\r\n"
			"Accept: */*\r\n\r\n";

int curl_get_request(const char *url, char *resp)
{
	int ret;
	int fd;
	char server[STR_64_LEN] = {0}; 
	char path[STR_128_LEN] = {0};
	char send_buf[STR_1024_LEN] = {0};
	char recv_buf[STR_1024_LEN] = {0};	
	char *pStr;
	char *pre;
	int port = 0;

	if ((url == NULL) || (resp == NULL))
	{
		LOG_ERROR("param error\n");
		return SDK_ERROR;
	}
	
	if (parse_url((char *)url, server, &port, path) != SDK_OK)
	{
		LOG_ERROR("parse_url fail\n");
		return SDK_ERROR;
	}
	
	fd = tcp_noblock_connect(server, port, 5000);
	if (fd < 0)
	{
		LOG_ERROR("tcp_block_connect fail[%s_%d]\n", server, port);      
		return SDK_ERROR;
	}

	sprintf(send_buf, curl_request_format, path, server);
	ret = tcp_noblock_send(fd, send_buf, strlen(send_buf));
	if (ret != strlen(send_buf))
	{
		LOG_ERROR("tcp_block_send fail\n");
        close_socket(&fd);          
		return SDK_ERROR;
	}
	
	ret = tcp_noblock_recv(fd, recv_buf, sizeof(recv_buf) - 1);
	if (ret <= 0)
	{
		LOG_ERROR("tcp_block_recv fail\n");
        close_socket(&fd);          
		return SDK_ERROR;
	}

	pre = pStr = strstr(recv_buf, "\r\n\r\n") + strlen("\r\n\r\n");
	pStr = strstr(pStr, "\r\n");
	if (pStr)
	{
	 	pStr += strlen("\r\n");
		sscanf(pStr, "%s\n", resp);
	}
	else
		sscanf(pre, "%s\n", resp);

    close_socket(&fd);
	
	return SDK_OK;
}

static char table64[]=
  "ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789+/";
void base64Encode(char *intext, char *output)
{
  unsigned char ibuf[3];
  unsigned char obuf[4];
  int i;
  int inputparts;

  while(*intext) {
    for (i = inputparts = 0; i < 3; i++) { 
      if(*intext) {
        inputparts++;
        ibuf[i] = *intext;
        intext++;
      }
      else
        ibuf[i] = 0;
    }

    obuf [0] = (ibuf [0] & 0xFC) >> 2;
    obuf [1] = ((ibuf [0] & 0x03) << 4) | ((ibuf [1] & 0xF0) >> 4);
    obuf [2] = ((ibuf [1] & 0x0F) << 2) | ((ibuf [2] & 0xC0) >> 6);
    obuf [3] = ibuf [2] & 0x3F;

    switch(inputparts) {
      case 1: /* only one byte read */
        sprintf(output, "%c%c==", 
            table64[obuf[0]],
            table64[obuf[1]]);
        break;
      case 2: /* two bytes read */
        sprintf(output, "%c%c%c=", 
            table64[obuf[0]],
            table64[obuf[1]],
            table64[obuf[2]]);
        break;
      default:
        sprintf(output, "%c%c%c%c", 
            table64[obuf[0]],
            table64[obuf[1]],
            table64[obuf[2]],
            table64[obuf[3]] );
        break;
    }
    output += 4;
  }
  *output=0;
}

void dump_sch(char *sch, int n, int m)
{
    int i, j;

    LOG_INFO("sch:\n");
    for (i = 0; i < n; i++)
    {
        for (j = 0; j < m; j++)
            printf("%c", *(sch + j + (i * m)));
        printf("\n");
    }
}

/* sch format [ sch[0]-sch[6] ] */
/* 30min 一个片段,实际使用48个字节 */
int check_sch(char *sch, int i, int j)
{
	int day, mins;
	time_t now;	
	struct tm timenow;  
	int now_min;
	int start_min, end_min;
    
	now = time(NULL);	
	localtime_r(&now, &timenow); 
	day = timenow.tm_wday;
	mins = ROUND_DOWN((timenow.tm_hour * MINS_PER_HOUR) + timenow.tm_min, 30);
//    LOG_INFO("day[%d] mins[%d]\n", day, mins);
	if (*(sch + mins + day * j) == '1')
		return SDK_OK;
	
	return SDK_ERROR;
}

#if 0
#define timercmp(a, b, CMP) 						     \
  (((a)->tv_sec == (b)->tv_sec) ? 					     \
   ((a)->tv_usec CMP (b)->tv_usec) : 					 \
   ((a)->tv_sec CMP (b)->tv_sec))

void pre_check_peroid(struct timeval *start)
{
	gettimeofday(start, NULL);
}
#endif

#if 0
/* 超时时间间隔 TIMEOUT: return SDK_OK */
/* peroid unit: s */
int check_peroid(struct timeval start, int sec)		
{
	struct timeval now, timeout;

	gettimeofday(&now, NULL);
	timeout.tv_sec = start.tv_sec + sec;
	timeout.tv_usec = start.tv_usec;
	if (timercmp(&now, &timeout, >=) != 0)
		return SDK_OK;	/* 当前时间超时了 */
	else
		return SDK_ERROR;
}
#else
int check_peroid(long start_time_ms, int timeout_ms)    		
{
	long now;

    	now = get_time_ms();
	if (now >= (start_time_ms + timeout_ms))
		return SDK_OK;	/* 当前时间超时了 */
	else if (start_time_ms == 0)	//start a new record
		return SDK_OK;
	else
		return SDK_ERROR;
}
#endif

/* time_format: [20140112112350] day_format [20140112]  */
char *get_time_str(char *time_str, char *day_str, char *other_str, int *wday)
{
	time_t now;	
	struct tm timenow; 

	now = time(NULL);	
	localtime_r(&now, &timenow); 
    if (time_str)
    	sprintf(time_str, "%04d%02d%02d%02d%02d%02d", timenow.tm_year + 1900, timenow.tm_mon + 1, timenow.tm_mday, timenow.tm_hour, timenow.tm_min, timenow.tm_sec);
    if (day_str)
	    sprintf(day_str, "%04d%02d%02d", timenow.tm_year + 1900, timenow.tm_mon + 1, timenow.tm_mday);
    if (other_str)
        sprintf(other_str, "%02d%02d%02d", timenow.tm_hour, timenow.tm_min, timenow.tm_sec);
    if (wday)
        *wday = timenow.tm_wday;
	
	return time_str;
}

/* diff_sec必须为正 */
char *time_add(char *time_str, int diff_sec)
{
    int hour, min, sec;
    
    if ((time_str == NULL) || (diff_sec <= 0))
        return NULL;

    sscanf(time_str, "%2d%2d%2d", &hour, &min, &sec);
    LOG_INFO("START: [%02d%02d%02d][%d]\n", hour, min, sec, diff_sec);
    sec += diff_sec;
    min += (sec / 60);    
    sec = sec % 60;
    hour += (min / 60);    
    min = min % 60;
    hour = hour % 24;
    LOG_INFO("END: [%02d%02d%02d]\n", hour, min, sec);    
    sprintf(time_str, "%02d%02d%02d", hour, min, sec);

    return time_str;
}

/* statfs / fstatfs */
/* remain: 剩余, total: 总容量 unit: KB */
int get_sd_capacity(long long *remain, long long *total)
{
	struct statfs fs;

	if (path_exist("/dev/mmcblk0p1") < 0)
	{
		*total = 0;
		*remain = 0;
		
		LOG_INFO("dev mmcblk0p1 is not exist!\n");
		return 0;
	}
	if (statfs("/mnt/sdcard", &fs) < 0)
	{
		*total = 0;
		*remain = 0;
		
		LOG_INFO("No Sdcard!\n");
		return 0;
	}
	*total = fs.f_bsize * (fs.f_blocks>>10);
	*remain = fs.f_bsize * (fs.f_bfree>>10);
	LOG_DEBUG("[%d_%lu_%lu %lld/%lld]\n", fs.f_bsize, fs.f_blocks, fs.f_bfree, *remain, *total);
	
	if (*total < 256)   /* 此时无SD卡 */
	{
		*total = 0;
		*remain = 0;

		return 0;
	}

	return SDK_OK;
}

/*		str_format [2014-12-01 11:23:24]	*/
char *time_now(char *str)
{
	time_t now;	
	struct tm timenow; 

	now = time(NULL);	
	localtime_r(&now, &timenow);     
//	p_now = gmtime(&now);     
	sprintf(str, "%04d-%02d-%02d %02d:%02d:%02d", timenow.tm_year + 1900, timenow.tm_mon, timenow.tm_mday,
										timenow.tm_hour, timenow.tm_min, timenow.tm_sec);
//    LOG_INFO("str [%s]\n", str);

	return str;
}

/* 检查文件类型 */
int check_file_type(const char *file_name, const char *type)
{
    char *pStr = NULL;
    
    if ((file_name == NULL) || (type == NULL))
        return SDK_ERROR;

    pStr = strrchr(file_name, '.');
    if (pStr == NULL)
        return SDK_ERROR;
    pStr++;
    if (!strcmp(pStr, type))
        return SDK_OK;

    return SDK_ERROR;
}

/* dir_path 绝对路径, type文件类型(文件后缀名) */
#define     MALLOC_ONCE_LEN         (1024)
char *get_file_list(const char *dir_path, const char *type)        /* ? 怎么使用,忘了 */
{
    DIR *dir;
    struct dirent *ptr;
    char *pStr;
    int len = 0;
    int malloc_cnt = 0;
    char *subdir_list = NULL;
    char subdir_path[64] = {0};

    pStr = (char *)malloc(MALLOC_ONCE_LEN * (malloc_cnt + 1));
    if (pStr == NULL)
    {
        printf("malloc fail\n");
        return NULL;
    }
    memset(pStr, 0, MALLOC_ONCE_LEN * (malloc_cnt + 1));
    malloc_cnt++;
    len += MALLOC_ONCE_LEN;    
    dir = opendir(dir_path);
    while ((ptr = readdir(dir)) != NULL)
    {
        if ((!strcmp(ptr->d_name, ".")) || 
            (!strcmp(ptr->d_name, "..")))
            continue;
        if (ptr->d_type == DT_DIR)
        {
            sprintf(subdir_path, "%s/%s", dir_path, ptr->d_name);
            subdir_list = get_file_list(subdir_path, type);
            if (len > (strlen(subdir_list) + 1))
            {                
                strcat(pStr, subdir_list);                
                len -= strlen(subdir_list);
            }
            else
            {
                int cnt;
                
                cnt = ROUND_UP(strlen(subdir_list) + 1, MALLOC_ONCE_LEN);
                pStr = (char *)realloc(pStr, MALLOC_ONCE_LEN * (malloc_cnt + cnt));
                if (pStr == NULL)
                    return NULL;
                malloc_cnt += cnt;
                len += cnt * MALLOC_ONCE_LEN;    
                strcat(pStr, subdir_list);
                len -= strlen(subdir_list);
            }  
            free(subdir_list);
        }
        else if ((ptr->d_type == DT_REG) && (check_file_type(ptr->d_name, type) == SDK_OK))
        {
            if (len > (strlen(ptr->d_name) + 2))
            {
                strcat(pStr, ptr->d_name);
                strcat(pStr, "\n");
                len -= (strlen(ptr->d_name) + 1);
            }
            else
            {
                pStr = (char *)realloc(pStr, MALLOC_ONCE_LEN * (malloc_cnt + 1));
                if (pStr == NULL)
                    return NULL;                
                malloc_cnt++;
                len += MALLOC_ONCE_LEN;    
                strcat(pStr, ptr->d_name);
                strcat(pStr, "\n");
                len -= (strlen(ptr->d_name) + 1);
            }
        }   
    }
    closedir(dir);
    return pStr;
}

long long check_rec_filename(const char *filename, const char *time_start, const char *time_end)
{
	long long ret;
	long long ret_start, ret_end;
	int day, time;

	sscanf(filename, "%d_%d_%*d.avi", &day, &time);
	ret = ((long long)day) * 1000000 + time;
	sscanf(time_start, "%8d%6d", &day, &time);
	ret_start = ((long long)day) * 1000000 + time;
	sscanf(time_end, "%8d%6d", &day, &time);
	ret_end = ((long long)day) * 1000000 + time;

//	LOG_INFO("ret: %lld, start: %lld, end: %lld\n", ret, ret_start, ret_end);

	if ((ret < ret_start) || (ret > ret_end))
		return 0;

	return ret;
}

int check_rec_dirname(const char *filename, const char *time_start, const char *time_end)
{
	int value, value_start, value_end;

	if ((!strcmp(filename, ".")) || (!strcmp(filename, "..")))
		return SDK_ERROR;
	sscanf(filename, "%8d", &value);
	sscanf(time_start, "%8d", &value_start);
	sscanf(time_end, "%8d", &value_end);	
	if ((value >= value_start) && (value <= value_end))
	{
//		LOG_INFO("value: %s, start: %s, end: %s\n", filename, time_start, time_end);	
		return SDK_OK;
	}
	else
	{
//		LOG_INFO("value: %s, start: %s, end: %s\n", filename, time_start, time_end);
		return SDK_ERROR;	
	}
}

#define     SD_PATH     		"/sdcard"

typedef struct FILE_UNIT_T
{
	char name[STR_64_LEN];
	long long num;
	struct list_head list;
}FILE_UNIT;


char* find_rec(const char *time_start, const char *time_end)    /* 暂不使用 */
{	
	FILE_UNIT head, *unit, *n_unit;
	long long num;
    int malloc_cnt = 0;
	int len = 0;	
    char *pStr;
    DIR *dir;
    struct dirent *ptr;

	memset(&head, 0, sizeof(FILE_UNIT));
    INIT_LIST_HEAD(&head.list);
	dir = opendir(SD_PATH);
	while ((ptr = readdir(dir)) != NULL)		/* 查找文件排序插入 */
	{
		if ((ptr->d_type == DT_DIR) && (SDK_OK == check_rec_dirname(ptr->d_name, time_start, time_end)))
		{
			DIR *sub_dir;
			struct dirent *sub_dirent;
			char path[STR_64_LEN] = {0};

			sprintf(path, "%s/%s/record", SD_PATH, ptr->d_name);
			sub_dir = opendir(path);
			while ((sub_dirent = readdir(sub_dir)) != NULL)
			{	
				if ((sub_dirent->d_type == DT_REG) && 
						(num = check_rec_filename(sub_dirent->d_name, time_start, time_end)))
				{
					FILE_UNIT *new_unit;

//					LOG_INFO("new: %s\n", sub_dirent->d_name);
					new_unit = (FILE_UNIT *)malloc(sizeof(FILE_UNIT));
					strcpy(new_unit->name, sub_dirent->d_name);
					new_unit->num = num;
#if 0
					list_for_each_entry_safe(unit, n_unit, &head.list, list)
					{
						if (new_unit->num < unit->num)
						{					
							list_add_tail(&new_unit->list, &unit->list);
							break;
						}
					}
					if (unit == &head)
						list_add_tail(&new_unit->list, &unit->list);
#else	/* 反向遍历, 效率更高 */
					list_for_each_entry_safe_ext(unit, n_unit, &head.list, list)	
					{
						if (new_unit->num > unit->num)
						{
							list_add(&new_unit->list, &unit->list);
							break;
						}
					}
					if (unit == &head)
						list_add(&new_unit->list, &unit->list);
#endif					
				}				
			}
			closedir(sub_dir);
		}
	}
	closedir(dir);		 
	/* 遍历链表输出查找结果 */
    pStr = (char *)malloc(MALLOC_ONCE_LEN * (malloc_cnt + 1));
    if (pStr == NULL)
    {
        printf("malloc fail\n");
        return NULL;
    }
	memset(pStr, 0, MALLOC_ONCE_LEN * (malloc_cnt + 1));
    malloc_cnt++;
    len += MALLOC_ONCE_LEN;    
    list_for_each_entry_safe(unit, n_unit, &head.list, list)
    {
#if 0    
		if (len > (strlen(unit->name) + 1))		/* 判断是否要重新申请内存 */
		{
			strcat(pStr, unit->name);    		
			len -= strlen(unit->name);
		}
		else
		{
			pStr = (char *)realloc(pStr, MALLOC_ONCE_LEN * (malloc_cnt + 1));
			if (pStr == NULL)
				return NULL;				
			malloc_cnt++;
			len += MALLOC_ONCE_LEN;    
			strcat(pStr, unit->name);
			len -= strlen(unit->name);
		}
#else
		if (len > (strlen(unit->name) + 2)) 	/* 判断是否要重新申请内存 */
		{
			strcat(pStr, unit->name);	
			strcat(pStr, "\n");
			len -= (strlen(unit->name) + 1);
		}
		else
		{
			pStr = (char *)realloc(pStr, MALLOC_ONCE_LEN * (malloc_cnt + 1));
			if (pStr == NULL)
				return NULL;				
			malloc_cnt++;
			len += MALLOC_ONCE_LEN;    
			strcat(pStr, unit->name);
			strcat(pStr, "\n");			
			len -= (strlen(unit->name) + 1);
		}
#endif
		list_del(&unit->list);
    }
	
	return pStr;
}

#define     NORMAL_RECORD_PATH          "/mnt/sdcard/record/normal"
#define     ALARM_RECORD_PATH           "/mnt/sdcard/record/alarm"
#define     TOPWAY_RECORD_PATH		"/mnt/sdcard/record/topway"
#define     NORMAL_SNAP_PATH            "/mnt/sdcard/snap/normal"
#define     ALARM_SNAP_PATH             "/mnt/sdcard/snap/alarm"
#define     TOPWAY_SNAP_PATH	   "/mnt/sdcard/snap/topway"	
int remove_sd_path(char *path,char *sub_path, int record_type, int limit, RECOREING_PATH *recpath)
{
	int ret;
	DIR *dir;
	struct dirent *ptr;
	long long remain, total;

	get_sd_capacity(&remain, &total);
	remain = remain >> 10;
	if ((remain) > limit)
	{
		return 0;
	}

	if(NULL == sub_path) goto SNAP_LOP;
	
	if( access(sub_path,F_OK) == 0 )
	{
		wait_record_index_file_free(recpath->mRecIndexOptFlag);
		recpath->mRecIndexOptFlag = 1;
		ret = RemoveRecordIndex(sub_path, record_type, limit, recpath);
		recpath->mRecIndexOptFlag = 0;
		if (ret < 0)
		{
			return -1;
		}
	}

	if( access( path ,F_OK) == 0)
	{
		dir = opendir(path);
		while((ptr = readdir(dir)) != NULL)
		{
			if(!strcmp(ptr->d_name , ".") || !strcmp(ptr->d_name, ".."))
			{
				continue;
			}
			if(ptr->d_type == DT_DIR)
			{
				DIR *s_dir;
				char s_path[STR_256_LEN] = {0};
				sprintf(s_path, "%s/%s", path, ptr->d_name);
				s_dir = opendir(s_path);
				LOG_INFO("s_dir[%s] path[%s]\n", ptr->d_name,s_path);

				ptr = readdir(s_dir);
				if(ptr->d_type != DT_REG)/*判断文件夹中是否存在普通文件*/
				{
					closedir(s_dir);
					if( 0 == remove(s_path))
					{
						LOG_INFO(" flold is NULL ,deleted is Sucess!\n");	
					}
					//goto END_DIR;
					break;
				}
				closedir(s_dir);
				//END_DIR:
			}
		}
		closedir(dir);
	}
	return 0;
	
SNAP_LOP :    
	if (access(path, F_OK) == 0)
	{
		dir = opendir(path);   
		while ((ptr = readdir(dir)) != NULL)
		{
			if (!strcmp(ptr->d_name, ".") || !strcmp(ptr->d_name, ".."))
			{
				continue;
			}
			if (ptr->d_type == DT_DIR)
			{   
				DIR *sub_dir;
				struct dirent *sub_dirent;
				char sub_path[STR_256_LEN] = {0};

				sprintf(sub_path, "%s/%s", path, ptr->d_name);
				sub_dir = opendir(sub_path);
				LOG_INFO("sub_dir[%s]\n", ptr->d_name);
				while ((sub_dirent = readdir(sub_dir)) != NULL)
				{
					if (sub_dirent->d_type == DT_REG) 
					{
						char file_path[STR_256_LEN] = {0};

						if (!strcmp(ptr->d_name, ".") || !strcmp(ptr->d_name, ".."))
						continue;                        
						sprintf(file_path, "%s/%s", sub_path, sub_dirent->d_name);
					#if 1
						ret = unlink(file_path);
					#else
						if(0 == (unlink(file_path)))///*如果unlink成功，才删除录像索引*/
						{
							RemoveRecordIndex(RECORD_INFO_FILE);
						}
					#endif
						//LOG_INFO("del file[%s][%d]\n", file_path, ret);     
						get_sd_capacity(&remain, &total);
						if (remain > limit)
						{
							closedir(sub_dir);                            
							goto REMOVE_OK;
						}
					}
				}
				closedir(sub_dir);
				rmdir(sub_path);
			}
		}
		closedir(dir);          
	}    
	return -1;
     
REMOVE_OK: 
	closedir(dir);
	return 0;
}

int remove_sd(int file_type, int limit, RECOREING_PATH recpath)
{
    if (file_type == FILE_TYPE_RECORD)
    {
        if (remove_sd_path(NORMAL_RECORD_PATH, RECORD_INFO_FILE, REC_NORMAL, limit, &recpath) == 0)
            return 0;
        if (remove_sd_path(ALARM_RECORD_PATH, RECORD_INFO_FILE, REC_MOTION_DETECT, limit, &recpath) == 0)
            return 0;        
    }
    if (file_type == FILE_TYPE_SNAP)
    {
        if (remove_sd_path(NORMAL_SNAP_PATH, NULL, 0, limit, &recpath) == 0)
            return 0;
        if (remove_sd_path(ALARM_SNAP_PATH, NULL, 0, limit, &recpath) == 0)
            return 0;        
    }  
	return -1;
}

//delete destroy record index from record index file
int delete_destroy_record_index(RECOREING_PATH *recpath)
{
	int fd = -1,fdw = -1;
	int ret = 0,retw = 0, result = 0;
	RECORD_INFO RecordInfo ;
	RAM_PARAM *pRamParam = NULL;// 
	
	if((fd = open(RECORD_INFO_FILE , O_RDWR )) < 0)
	{
		LOG_INFO("open file error!\n");
		return -1;
	}
	if ((fdw = open(RECORD_INFO_TMP_FILE , O_CREAT | O_WRONLY )) < 0)
	{
		LOG_INFO("open file error!\n");
		return -1;
	}
		
	while((ret = read(fd , (void*)&RecordInfo ,sizeof(RecordInfo))) > 0)
	{
		//not write the destroy file index
		if (RecordInfo.isdestroy)
		{
			result = 0;
			if (RecordInfo.type == REC_NORMAL)
			{
				if ((memcmp_ext(RecordInfo.path, recpath->normal, STR_64_LEN)) == 0)
					result = 1;
			}
			else if (RecordInfo.type == REC_MOTION_DETECT)
			{
				if ((memcmp_ext(RecordInfo.path, recpath->alarm, STR_64_LEN)) == 0)
					result = 1;
			}
			if (result == 0)
			{
				LOG_INFO("jump the destroy file index[%s]",RecordInfo.path);
				continue;
			}
		}
		retw = write(fdw, &RecordInfo, sizeof(RecordInfo));
		if (retw != sizeof(RecordInfo))
		{
			LOG_WARN("write data error: %d\n", sizeof(RecordInfo));
			close(fd);
			close(fdw);
			return -1;
		}
	}
	close(fd);
	close(fdw);
	if(0 != ( ret = unlink(RECORD_INFO_FILE)))
	{
		LOG_WARN("File [%s] Del is fail!\n",RECORD_INFO_FILE);
	}
	if(0 != ( rename(RECORD_INFO_TMP_FILE ,RECORD_INFO_FILE)) )
	{
		 LOG_WARN("File [%s] rename is fail!\n",RECORD_INFO_TMP_FILE);
	}
	return 0;
}

#if 0
int get_file_len(const char *name)
{
    struct stat temp;   
	
    stat(name, &temp);   
    return temp.st_size; 	
}

void auth_enc_parser(char *auth_enc, int *auth, int *enc)
{
	if (strstr(auth_enc, "WPA2-PSK"))		/* WPA2-PSK/WPA-PSK --> WPA2-PSK */
	{
        *auth = AUTH_WPA2;
		if (strstr(auth_enc, "TKIP"))		/* TKIP/AES --> TKIP */
			*enc = ENC_TKIP;
		else
			*enc = ENC_AES;
	}
	else if (strstr(auth_enc, "WPA-PSK"))
	{
        *auth = AUTH_WPA;
		if (strstr(auth_enc, "TKIP"))
			*enc = ENC_TKIP;
		else
			*enc = ENC_AES;	
	}
	else if (strstr(auth_enc, "WEP"))
	{
        *auth = AUTH_WEP;
		*enc = ENC_SHARE;	
	}
	else
	{
        *auth = AUTH_OPEN;
		*enc = ENC_NONE;					
	}
}

#define     ONCE_OBJECT     (20)
int get_ap_list(AP_INFO **pApList)
{
    char *p = NULL;
	char *str = NULL;	
	char ssid[STR_128_LEN] = {0};
	char auth_enc[STR_64_LEN] = {0};
	int enc, auth, freq;	
	char signal[STR_64_LEN] = {0};
	int id = 0, len = 0, retry = 20;
    int malloc_obj_cnt = 0;

    *pApList = (AP_INFO *)calloc(ONCE_OBJECT, sizeof(AP_INFO));
    if (*pApList == NULL)
        return MALLOC_FAIL;
    malloc_obj_cnt += ONCE_OBJECT;
    if (wifi_scan() < 0)
	{
		LOG_ERROR("wifi_scan fail\n");
        if (*pApList)
            free(*pApList);
        *pApList = NULL;
		return -1;
	}
    
NEXT_LOOP:
    usleep(500 * 1000);
	if (wifi_scan_result(&str, &len) < 0)
	{
		LOG_ERROR("wifi_scan_result fail\n");
        if (*pApList)
            free(*pApList);        
        *pApList = NULL;
		return -1;
	} 

//    LOG_INFO("str: [%s]\n", str);
//    LOG_INFO("scan result len [%d]\n", len);
    p = strtok(str, "\n");
	if (p == NULL)					/* 过滤第一行数据 */
	{
        if (*pApList)
            free(*pApList);        
        *pApList = NULL;
		return -1;        
    }
	p = strtok(NULL, "\n");		 
	while (p != NULL)
	{
        memset(ssid, 0, sizeof(ssid));
		sscanf(p, "%*s\t%d\t%s\t%s\t%[^\n]", &freq, signal, auth_enc, ssid);
        if (strlen(ssid) == 0)
        {
    		p = strtok(NULL, "\n");
            continue;
        }
//        LOG_INFO("auth_enc:%s[%s]\n", auth_enc, ssid);
		auth_enc_parser(auth_enc, &auth, &enc);
	if (id == MAX_AP_CNT)
	{
		break;
	}
        if (id == (malloc_obj_cnt - 1))       /* 再申请内存 */
        {
            void *pTmp;

            pTmp = *pApList;
            *pApList = (AP_INFO *)realloc(pTmp, sizeof(AP_INFO) * (malloc_obj_cnt + ONCE_OBJECT));
            if (NULL == *pApList)
            {
                free(pTmp);
                LOG_ERROR("realloc fail\n");
                if (*pApList)
                    free(*pApList);                
                *pApList = NULL;
                return -1;
            }
//            memset(*pApList + (malloc_obj_cnt * sizeof(AP_INFO)), 0, (ONCE_OBJECT * sizeof(AP_INFO)));
            malloc_obj_cnt += ONCE_OBJECT;
        }        
        strncpy((*pApList)[id].ssid, ssid, STR_128_LEN - 1);
        (*pApList)[id].signal = atoi(signal);
        (*pApList)[id].enc = enc;
        (*pApList)[id].auth = auth;
        (*pApList)[id].channel = freq;      
		id++;        
		p = strtok(NULL, "\n");
	} 
    if ((id == 0) && retry--)
    {
        goto NEXT_LOOP;
    }
#if 0    
    if (str) 
        free(str);
#endif 

    return id;
}

int connect_wifi_wep(char *ssid)
{
	int ret ;

	if (wifi_scan() < 0)
	{
		LOG_ERROR("wifi_scan fail\n");
	}
	sleep(2);
	ret = wifi_is_wep(ssid);
	
	return ret;
}
	
int connect_wifi(char *ssid, char *key, int authmode)
{
	int net_id,len = -1;  

	len = strlen(key);
	wifi_disable_network(0);
	wifi_del_network(0);
	net_id = wifi_add_network();
	LOG_INFO("connect wifi[ssid:%s@key:%s@auth:%d@id:%d]\n",ssid,key,authmode,net_id);
	if (AUTH_WEP == authmode)
	{
		wifi_set_network(net_id, "ssid", ssid);
		wifi_set_network2wep(net_id, "key_mgmt", "NONE");
		if ((10 == len) || (26 == len))
		{
			wifi_set_network2wep(net_id, "wep_key0", key);
		}
		else if ((5 == len) || (13 == len))
		{
			wifi_set_network(net_id, "wep_key0", key);
		}
		else
		{
			LOG_INFO("Not Those Key len!\n");
		}
		wifi_set_network3wep(net_id, "wep_tx_keyidx", 0);
		wifi_select_network(net_id);
		wifi_enable_network(net_id);
	}
	else if ((AUTH_OPEN == authmode) && (len == 0))
	{
		LOG_INFO("auth open\n");
		wifi_set_network(net_id, "ssid", ssid);
		wifi_set_network2wep(net_id, "key_mgmt", "NONE");
		wifi_enable_network(net_id);
	}
	else
	{
		wifi_set_network(net_id, "ssid", ssid);
		wifi_set_network(net_id, "psk", key);
		wifi_select_network(net_id);
		wifi_enable_network(net_id);
	}
	return SDK_OK;
	
}

int stop_connect_wifi(void)
{
    int net_id = 0;
    
    wifi_disable_network(net_id);
    
    return SDK_OK;
}

void dump_data(const char *flag, unsigned char *data, int len)
{
    int index = 0;
    
    LOG_INFO("dump_data[%s]:\n", flag);
    while (len--)
    {
        printf("0x%02x ", data[index++]);
        if (index%8 == 0)
            printf("\n");
    }
}

int sd_exist(void)
{
   	struct statfs fs; 
    
    statfs("/mnt/sdcard", &fs);
    if (fs.f_type == JFFS2_SUPER_MAGIC)
        return -1;
    
    if (!path_exist("/dev/mmcblk0p1"))
        return 0;
    else
        return -1;
}

void mkdirs_real(const char *dir)
{
    char tmp[1024];
    char *p;

    if (strlen(dir) == 0 || dir == NULL) {
        printf("strlen(dir) is 0 or dir is NULL.\n");
        return;
    }

    memset(tmp, '\0', sizeof(tmp));
    strncpy(tmp, dir, strlen(dir));
    if (tmp[0] == '/') 
        p = strchr(tmp + 1, '/');
    else 
        p = strchr(tmp, '/');

    if (p) {
        *p = '\0';
        mkdir(tmp, 0777);
        chdir(tmp);
    } else {
        mkdir(tmp, 0777);
        chdir(tmp);
        return;
    }

    mkdirs_real(p + 1);
}

void mkdirs(const char *dir)
{
    char cwd[512] = {0};

    getcwd(cwd, sizeof(cwd) -1);
    mkdirs_real(dir);
    chdir(cwd);
}
 
/* GMT12 GMT11 --- GMT-11 GMT-12 */
int AjustTZ(void)
{
#if 1
	char data[1280] = {0};
	char *pStr = NULL;
	int len,i;
	char tz[STR_16_LEN] = {0};
	
	read_data_from_file(SET_TIMEZONE_FILE, data, &len);
	if ((pStr = strstr(data, "TZ=")))
	{
		for(i=0; i<6; i++)
		{
			if (*(pStr+strlen("TZ=")+i) == ' ')
			{
				break;
			}
			tz[i] = *(pStr+strlen("TZ=")+i);
		}
		//strncpy(tz,pStr+strlen("TZ="),6);
		len = strlen(tz);
		//read_data_from_file(TIMEZONE_FILE, tz, &len);
		tz[len] = 0;
		LOG_INFO("tz [%s]\n", tz);
		setenv("TZ", tz, 1);
	}

	return 0;
#endif
}

int mount_nfs(const char *local_path, const char *remote_path)
{
    if (!mount(remote_path, local_path, "nfs", 0, NULL))
        return SDK_OK;
    else
        return SDK_ERROR;
}
#endif
int get_param_type(void *param)       /* FACTORY / FLASH / RAM */
{
    SYSTEM_PARAM *pSystemParam;
    FACTORY_PARAM *pFactoryParam;
    FLASH_PARAM *pFlashParam;
    RAM_PARAM *pRamParam;

    if (param == NULL)
        return -1;

    pSystemParam = system_param_map(0);
    pFactoryParam = &pSystemParam->factory_param;
    pFlashParam = &pSystemParam->flash_param;
    pRamParam = &pSystemParam->ram_param;

    if (((int)param >= (int)pFactoryParam) && ((int)param < (int)pFactoryParam + sizeof(FACTORY_PARAM)))
        return FACTORY_TYPE;
    else if (((int)param >= (int)pFlashParam) && ((int)param < (int)pFlashParam + sizeof(FLASH_PARAM)))
        return FLASH_TYPE;
    else if (((int)param >= (int)pRamParam) && ((int)param < (int)pRamParam + sizeof(RAM_PARAM)))
        return RAM_TYPE;  
    else
        return BAD_TYPE;
}

int calculate_avi_file_real_length(char * file, char *path, int rec_type)
{
	int fd = -1,ret;
	RECORD_INFO pRecordInfo ;
	int difmin = 0,difsec = 0,diftime = 0;
	struct tm end_tm,start_tm;
				
	if((fd = open(file , O_RDONLY)) < 0)
	{
		LOG_INFO("open file error!\n");
		return -1;
	}
			
	while((ret = read(fd , (void*)&pRecordInfo ,sizeof(pRecordInfo))) > 0)
	{
		if (strcmp(pRecordInfo.path,path) != 0)
			continue;
		sscanf(pRecordInfo.EndTime,"%4d%2d%2d%2d%2d%2d",
			&end_tm.tm_year, &end_tm.tm_mon, &end_tm.tm_mday,
			&end_tm.tm_hour, &end_tm.tm_min, &end_tm.tm_sec);
		sscanf(pRecordInfo.StartTime,"%4d%2d%2d%2d%2d%2d",
			&start_tm.tm_year, &start_tm.tm_mon, &start_tm.tm_mday,
			&start_tm.tm_hour, &start_tm.tm_min, &start_tm.tm_sec);
		
		if (end_tm.tm_year < start_tm.tm_year)
			return 0;
    		if (REC_NORMAL == rec_type)
		{
			if (start_tm.tm_hour != end_tm.tm_hour)//eg:23:59:50, 12:50:50
				difmin = (60 - start_tm.tm_min) + end_tm.tm_min;
			else
				difmin = end_tm.tm_min - start_tm.tm_min;
			if (end_tm.tm_sec >= start_tm.tm_sec)
			{
				difsec = end_tm.tm_sec - start_tm.tm_sec;
			}
			else
			{
				difmin -= 1;
				difsec = 60 + end_tm.tm_sec - start_tm.tm_sec;
			}
			diftime = difmin*60 + difsec;
			if (diftime > NORMAL_RECORD_PEROID)
				diftime = NORMAL_RECORD_PEROID;
    		}
		else if (REC_MOTION_DETECT == rec_type)
		{
			if (start_tm.tm_min != end_tm.tm_min)//eg:23:59:30, 23:59:50, 00:00:50
				diftime = (60 - start_tm.tm_sec) + end_tm.tm_sec;
			else
				diftime = end_tm.tm_sec - start_tm.tm_sec;
			if (diftime > ALARM_RECORD_PEROID)
				diftime = ALARM_RECORD_PEROID;
		}
		break;
	}
	close(fd);
	LOG_INFO("avi_file diftime[%d]\n",diftime);
	return diftime;
	
}

int FindAlarmRecordFile(struct tm time, char *path, int period)
{
    char EndTimeStr[STR_32_LEN] = {0};
    
	if (NULL == path)
		return -1;
    sprintf(EndTimeStr, "%02d%02d%02d",  time.tm_hour, time.tm_min, time.tm_sec);
    time_add(EndTimeStr, period);
	sprintf(path, "/mnt/sdcard/record/alarm/%04d%02d%02d/%04d%02d%02d_%02d%02d%02d_%s.avi", 
                    time.tm_year, 
                    time.tm_mon, 
                    time.tm_mday,         
                    time.tm_year, 
                    time.tm_mon, 
                    time.tm_mday, 
                    time.tm_hour, 
                    time.tm_min, 
                    time.tm_sec,
                    EndTimeStr);
	if (access(path,F_OK) == 0)
		return 0;
	else
		return -1;
}

int FindNormalRecordFile(struct tm time, char *path, int period)
{
    char EndTimeStr[STR_32_LEN] = {0};
    
	if (NULL == path)
		return -1;
    sprintf(EndTimeStr, "%02d%02d%02d",  time.tm_hour, time.tm_min, time.tm_sec);
    time_add(EndTimeStr, period);
	sprintf(path, "/mnt/sdcard/record/normal/%04d%02d%02d/%04d%02d%02d_%02d%02d%02d_%s.avi", 
                    time.tm_year, 
                    time.tm_mon, 
                    time.tm_mday,         
                    time.tm_year, 
                    time.tm_mon, 
                    time.tm_mday, 
                    time.tm_hour, 
                    time.tm_min, 
                    time.tm_sec,
                    EndTimeStr);
	if (access(path,F_OK) == 0)
		return 0;
	else
		return -1;
}

/*SD卡录满时、删除最早的视频文件同时 删掉视频索引*/
int RemoveRecordIndex(const char *InfoFilePath, int record_type, int limit, RECOREING_PATH *recpath)
{	
	int fd = -1,fdw = -1;
	int ret = 0,retw = 0,findflag = 0,curfileflag = -1;
	RECORD_INFO RecordInfo ;
	long long remain, total;

	if((fd = open(InfoFilePath , O_RDWR )) < 0)
	{
		LOG_INFO("open file error!\n");
		return -1;
	}
	if ((fdw = open(RECORD_INFO_TMP_FILE , O_CREAT | O_WRONLY )) < 0)
	{
		LOG_INFO("open file error!\n");
		return -1;
	}
		
	while((ret = read(fd , (void*)&RecordInfo ,sizeof(RecordInfo))) > 0)
	{
	    	if (0 == findflag)
	    	{
	    		if ((RecordInfo.type) == record_type)
			{
				curfileflag = 1;
				if (RecordInfo.isdestroy)
				{
					if (REC_NORMAL == record_type)
						curfileflag = memcmp_ext(RecordInfo.path, recpath->normal, STR_64_LEN);
					else if (REC_MOTION_DETECT == record_type)
						curfileflag = memcmp_ext(RecordInfo.path, recpath->alarm, STR_64_LEN);
				}
				if (curfileflag != 0)
				{
					findflag = 1;
				 	if(0 == ( ret = unlink(RecordInfo.path)))
				 	{
						get_sd_capacity(&remain, &total);
						remain = remain >> 10;
						if ((remain) < limit)	
							findflag = 0;
						LOG_INFO("File [%s] Del is OK!\nSDcard size[%lld],limit[%d]\n",RecordInfo.path,remain,limit);
				 	}
					continue; //not save the delete's file path
				}
			}
		}
		retw = write(fdw, &RecordInfo, sizeof(RecordInfo));
		if (retw != sizeof(RecordInfo))
		{
			LOG_WARN("write data error: %d\n", sizeof(RecordInfo));
			close(fd);
			close(fdw);
			return -1;
		}
	}
	close(fd);
	close(fdw);

	if(0 != ( ret = unlink(InfoFilePath)))
	{
		LOG_WARN("File [%s] Del is fail!\n",InfoFilePath);
	}
	if(0 != ( rename(RECORD_INFO_TMP_FILE ,InfoFilePath)) )
	{
		 LOG_WARN("File [%s] rename is fail!\n",RECORD_INFO_TMP_FILE);
	}
	get_sd_capacity(&remain, &total);
	remain = remain >> 10;
	if ((remain) < limit)
		return -1;	
	else
		return 0;
}

int WriteRecordIndex(RECORD_INFO RecordInfo, const char *InfoFilePath)
{
	int fd;
	int ret;
	
	if (InfoFilePath == NULL)
		return -1;
	
	if ((fd = open(InfoFilePath, O_CREAT | O_WRONLY | O_APPEND)) < 0)		/* O_TRUNC: 文件存在覆盖 */
	{
		LOG_INFO("open file error\n");
		return -1;
	}

	ret = write(fd, &RecordInfo, sizeof(RecordInfo));
	if (ret != sizeof(RecordInfo))
	{
		LOG_INFO("write data error: %d\n", sizeof(RecordInfo));
		close(fd);
		return -1;
	}
	close(fd);
		
	return 0;
}

/*
change endtime | isdestroy of record index file(data.db)
flag:1->change endtime  0->not change
path:recording file path
*/
int ChangeRecIndexEndtimeType(char *endtime, char *path, int flag)
{
	int fd, ret, recinfolen,offset;
	RECORD_INFO RecordInfo;
	char isdestroy[4] = {0};

	if (path == NULL)
	{
		LOG_WARN("the path is NULL\n");
		return -1;
	}
		
	if ((fd = open(RECORD_INFO_FILE, O_CREAT | O_RDWR)) < 0)		/* O_TRUNC: 文件存在覆盖 */
	{
		LOG_WARN("open file error\n");
		return -1;
	}

	recinfolen = sizeof(RECORD_INFO);
	if ((lseek(fd,-recinfolen,SEEK_END)) < 0)
	{
		LOG_WARN("seek file error\n");
		goto OPREATE_ERROR;
	}
	while((ret = read(fd , (void*)&RecordInfo , recinfolen)) > 0)
	{
		if ((memcmp_ext(RecordInfo.path, path, STR_64_LEN)) == 0)
		{
			break;
		}
		if ((lseek(fd,-(recinfolen * 2), SEEK_CUR)) < 0)
		{
			offset = lseek(fd, 0, SEEK_CUR);
			LOG_WARN("seek file error[%d]savepath[%s]\n[%s]\n",offset,path,RecordInfo.path);
			goto OPREATE_ERROR;
		}
	}
	if (ret < 0)
	{
		LOG_INFO("read record index file fail\n");
		goto OPREATE_ERROR;
	}
	
	//小端法，字节对齐
	lseek(fd, -12, SEEK_CUR);	
	write(fd, isdestroy, 4);
		
	if (flag)
	{
		if (endtime == NULL)
		{
			LOG_WARN("the endtime is NULL!\n");
			goto OPREATE_ERROR;
		}
		lseek(fd, -40 + 8, SEEK_CUR);	//字节对齐
		write(fd, endtime, 16);
		LOG_INFO("change endtime[%s]\n",endtime);
	}
	close(fd);
	return 0;
	
OPREATE_ERROR:
	close(fd);
	return -1;
}

long get_time_ms_bak(void)
{
    int fd;
    float result;
    char data[STR_32_LEN] = {0};
    long msecond;

    fd = open("/proc/uptime", O_RDONLY);
    if (fd < 0)
    {
        LOG_ERROR("open fail[/proc/uptime]\n");
        return -1;
    }
    read(fd, data, sizeof(data));
    close(fd);    
    sscanf(data, "%f", &result);
    msecond = result * 1000;

    return msecond;
}

long get_time_ms(void)
{
    long msecond;
    struct timespec current = {0, 0}; 

    clock_gettime(CLOCK_MONOTONIC, &current);
    msecond = current.tv_sec * 1000 + current.tv_nsec / 1000000;

    return msecond;
}

pid_t gettid(void) 
{      
    return syscall(SYS_gettid); 
} 

void print_pid(const char *tag)
{
    if (tag == NULL)
        LOG_INFO("pid [%d]\n", gettid());
    else
        LOG_INFO("pid [%s:%d]\n", tag, gettid());
}
#if 0
int wifi_test(char *new_ssid, char *new_key, int new_authmode, char *ssid, char *key, int authmode)
{
    int ret = -1, retry = 20;
    char link_ssid[64] = {0};

    if (!strlen(new_ssid) || !strlen(new_key))
        return -1;
    
	connect_wifi(new_ssid, new_key, new_authmode);
    while (retry--)
    {
        wifi_check_link(link_ssid);
        if (!strcmp(new_ssid, link_ssid))
        {
            ret = 0;
            break;       
        }
        sleep(1);
    }

	connect_wifi(ssid, key, authmode);      

    if (ret == 0)    
        LOG_INFO("wifi test ok\n");
    else
        LOG_INFO("wifi test fail\n");

    return ret;
}
#endif
int isIpv4(const char *str)
{
    int ret, a, b, c, d;

    if (NULL == str)
        return -1;

    ret = sscanf(str, "%d.%d.%d.%d", &a, &b, &c, &d);

    if ((ret == 4) &&
        (a >= 0) && (a <= 255) &&
        (b >= 0) && (b <= 255) &&
        (c >= 0) && (c <= 255) &&
        (d >= 0) && (d <= 255))
        return 0;

    return -1;
}

int CheckSch_Alarm(ALARM_PARAM *pAlarmParam)
{
	time_t now;	
	struct tm timenow; 
	long long times_now, times_start, times_end,time_final;
	int hour, min, sec, TimeFlag = 0, WeekFlag = 0,num,forwad;

	now = time(NULL);	
	localtime_r(&now, &timenow);  	
	times_now = timenow.tm_hour * 3600 + timenow.tm_min * 60 + timenow.tm_sec;
	//LOG_INFO("now:%d,%d,%d,%d\n",timenow.tm_hour,timenow.tm_min,timenow.tm_sec,timenow.tm_wday);
	
	if (timenow.tm_wday == 0)	//sunday
		num = 6;
	else
		num = timenow.tm_wday -1;
	//23:59:59
	time_final = 23 * 3600 + 59 * 60 + 59;
	//start time
	sscanf(pAlarmParam->alarm_rec_plan.start_time, "%d:%d:%d", &hour, &min, &sec);
	times_start = hour * 3600 + min * 60 + sec;
	//end time
	sscanf(pAlarmParam->alarm_rec_plan.end_time, "%d:%d:%d", &hour, &min, &sec);
	times_end = hour * 3600 + min * 60 + sec;
	
	if (pAlarmParam->alarm_rec_plan.stepoverflag)
	{
		if (num == 0)
			forwad = 6;
		else
			forwad = num - 1;
		if ((pAlarmParam->alarm_rec_plan.day[num] == ON)
			&&(times_now >= times_start) && (times_now <= time_final))
		{
			WeekFlag = 1;
			TimeFlag = 1;
		}
		else if ((pAlarmParam->alarm_rec_plan.day[forwad] == ON)
			&&(times_now >= 0) && (times_now <= times_end))
		{
			WeekFlag = 1;
			TimeFlag = 1;
		}
	}
	else
	{
		if (pAlarmParam->alarm_rec_plan.day[num] == ON)
			WeekFlag = 1;
		if ((times_now >= times_start) && (times_now <= times_end))
			TimeFlag = 1;
	}
	
	if (TimeFlag && WeekFlag)	
	{
		return SDK_OK;
	}
	else
	{
		TimeFlag = 0;
		WeekFlag = 0;
	}
	
	return -1;
}

int write_danale_uid_file(char *data,char *md5)
{
	int fd_uid,ret;
	unsigned char Md5Bin[16] = {0};
	char Md5Str[CHECKSUM_LEN + 1] = {0};	
	char path[] = "/mnt/sdcard/danale.conf";
	char danalepath[] = "/mnt/config/danale.conf";
	
	remove("danale_file");
	fd_uid = open(path,O_RDWR|O_CREAT);
	if (fd_uid < 0)
	{
		printf("open temp danale uid file fail\n");
		return -1;
	}
	write(fd_uid,data,strlen(data));
	close(fd_uid);
	
	//check md5
	md5_file(path, Md5Bin);	
	bin2hex(Md5Str, Md5Bin, sizeof(Md5Bin));
	if (strcmp(Md5Str, md5))
	{
		printf("md5 checksum fail\n");		
		return -1;
	}	
	//move file
	ret = rename(path,danalepath);
	if (ret < 0)
	{
		printf("rename uid file fail\n");
		return -1;
	}
	
	return 0;
}

int memcmp_ext(const void *buffer1, const void *buffer2, int count)
{
	if (!count)
		return 0;
	while(--count && *(char *)buffer1 == *(char *)buffer2)
	{
		buffer1 = (char *)buffer1 + 1;
		buffer2 = (char *)buffer2 + 1;
	}
	return (int)(*((unsigned char *)buffer1) - *((unsigned char *)buffer2));
}

int wait_record_index_file_free(int flag)
{
	int waitcnt = 30;
	
	while(flag)
	{
		usleep(100*1000);
		waitcnt --;
		if (waitcnt < 0)
		{
			LOG_WARN("wait record index timeout[3s]!\n");
			return -1;
		}
	}
	return 0;
}

