#ifndef     __CGI_CMD_PROC_H__
#define     __CGI_CMD_PROC_H__

#ifdef		__cplusplus
extern "C" {
#endif

#include "param.h"

#include <stdio.h>
#include <stdlib.h>

int get_alarm_param_str(const char *key_word, const char *value, char *out_str);
int get_ftp_param_str(const char *key_word, const char *value, char *out_str);
int get_email_param_str(const char *key_word, const char *value, char *out_str);
int get_video_param_str(const char *key_word, const char *value, char *out_str);
int get_audio_param_str(const char *key_word, const char *value, char *out_str);
int get_ptz_param_str(const char *key_word, const char *value, char *out_str);
int get_record_param_str(const char *key_word, const char *value, char *out_str);
int get_snap_param_str(const char *key_word, const char *value, char *out_str);
int get_wifi_param_str(const char *key_word, const char *value, char *out_str);
int get_net_param_str(const char *key_word, const char *value, char *out_str);
int get_ddns_param_str(const char *key_word, const char *value, char *out_str);
int get_time_param_str(const char *key_word, const char *value, char *out_str);
int get_user_param_str(const char *key_word, const char *value, char *out_str);
int get_multi_param_str(const char *key_word, const char *value, char *out_str);
int get_misc_param_str(const char *key_word, const char *value, char *out_str);
int get_rtmp_param_str(const char *key_word, const char *value, char *out_str);
int get_factory_param_str(const char *key_word, const char *value, char *out_str);
int get_ram_param_str(const char *key_word, const char *value, char *out_str);
int get_private_param_str(const char *key_word, const char *value, char *out_str);

/****************************   misc func   ***************************/
int time_now_str(const char *key_word, const char *value, char *out_str);
int one_search_str(const char *key_word, const char *value, char *out_str);
int cgi_format_sd(const char *key_word, const char *value, char *out_str);
int cgi_remove_sd(const char *key_word, const char *value, char *out_str);
int system_reboot(const char *key_word, const char *value, char *out_str);
int get_run_time(const char *key_word, const char *value, char *out_str);
int get_aplist_str(const char *key_word, const char *value, char *out_str);
int cgi_sd_capacity(const char *key_word, const char *value, char *out_str);
int cgi_wifi_link(const char *key_word, const char *arg, char *out_str);
int cgi_set_time_v2(const char *key_word, const char *value, char *out_str);
int cgi_get_idr(const char *key_word, const char *value, char *out_str);
int save_flash_param(const char *key_word, const char *value, char *out_str);
int save_factory_param(const char *key_word, const char *value, char *out_str);


#ifdef		__cplusplus
}
#endif

#endif
