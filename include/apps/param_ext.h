#ifndef      __PARAM_EXT_H__
#define     __PARAM_EXT_H__

#ifdef	__cplusplus
extern "C" { 
#endif

#include "param.h"

SYSTEM_STATUS *ram_get_system_status(void);
void set_talk_alive(int alive);
int get_talk_alive(void);
void set_ftp_test_result(int result);
char *get_camera_name(void);
int check_user(const char *user, const char *pwd);
int get_pwd(const char *user, char *pwd);
unsigned char *get_aac_header(void);
AVC_HEADER *get_avc_header(void);
STREAM_PARAM *get_stream_param(void);
int param_is_init(void);

#ifdef __cplusplus
}
#endif	/* __cplusplus */

#endif  /* __COMMON_H__ */
