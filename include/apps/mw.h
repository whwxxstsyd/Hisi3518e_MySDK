#ifndef     __MW_H__
#define     __MW_H__

#ifdef	__cplusplus
extern "C" { 
#endif

#include "param.h"
#include "common.h"
#include "debug.h"
#include "av_buffer.h"

#include <sys/ipc.h>
#include <sys/msg.h>
#include <sys/types.h>

int email_pic(EMAIL_PARAM *pEmailParam, int cnt);
int ftp_pic(FTP_PARAM *pFtpParam, int cnt);
int sd_pic(const char *path, int cnt);
int email_rec(EMAIL_PARAM *pEmailParam, int sec, const char *tmp_dir);
int ftp_rec(FTP_PARAM *pFtpParam, int sec, const char *tmp_dir);
int sd_rec(const char *path, int sec);
char **get_pic(int cnt);
int snapshot(const char *path);
char *get_rec(int sec, const char *tmp_dir, int *stop_flag);


#ifdef __cplusplus
}
#endif	/* __cplusplus */

#endif

