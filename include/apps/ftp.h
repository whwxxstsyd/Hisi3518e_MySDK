#ifndef     __FTP_H__
#define     __FTP_H__

#ifdef		__cplusplus
extern "C" {
#endif

#include "param.h"

#define         CMD_BUF_SIZE        			(1023)
#define			DATA_BUF_SIZE					(30 * 1024)
#define 		CMD_DIR_SIZE 					(255)
#define 		FTP_CONNECT_TIMEOUT 			(10000)
#define 		FTP_MAX_CONFIRM_TIMEOUT 		(15000)
#define 		FTP_RECV_TIMEOUT    			(3000)
#define 		FTP_SEND_TIMEOUT    			(3000)
#define 		FTP_READ_SEND_BLOCK 			(4096)


struct line_data
{
    int count;
    char **lines;
};

typedef struct FTP_INFO_T
{
    int 	ctrlfd;
    int 	datafd;
    int 	portfd;
    int 	dataPort;   
    int 	startPort;
	int     endPort;	
    int 	dataSvrPort;
    char 	cmd_buffer[CMD_BUF_SIZE + 1];
	char 	data_buffer[DATA_BUF_SIZE];
    char 	cmd_line[CMD_BUF_SIZE + 1];
    char 	*cmd_ptr;
    FTP_PARAM param;
}FTP_INFO;

int ftp_upload_one_file(FTP_INFO *ftp_info, const char *src, const char *dst);  /* dst目标文件名 */

#ifdef		__cplusplus
}
#endif

#endif
