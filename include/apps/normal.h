#ifndef     __NORMAL_H__
#define     __NORMAL_H__

#ifdef	__cplusplus
extern "C" {
#endif

#include "common.h"
#include "param.h"

void show_time(const char *tag);
void *system_param_map(unsigned int param_offset);
void add_checksum(char *file);
char* bin2hex(char *p, const unsigned char *cp, int count);
int checksum(char *file);
char **ParserString(const char *string);
void FreeParserVars(char *list[]);
char **StrParser(char *str);        /* 不在使用 */
void FreeStrList(char **str);
int get_hash_pos(const char *str);
int copy_file(char *src, char *dst);
char *safe_strncpy(char *dst, const char *src, int size);
char *my_itoa(int num, char *str, int base);
int write_data_to_file(const char *file, char *data, int len);
int read_data_from_file(const char *file, char *data, int *len);
int sdk_select(int fd, int fd_type, int time_out);
int check_frame(REAL_FRAME *frame);
void roi_to_str(ROI roi, char *str);
void str_to_roi(ROI *roi, char *str);
char *get_file_name(const char *path);
char *get_dir_name(const char *path, char *dir);
int path_exist(const char *path);
#if 0
pthread_t CreateTask(THREAD_ENTER thread_enter, void *arg);
void DestoryTask(pthread_t task_id);
#endif
int save_file_to_sd(const char *path, const char *sd_path);
int set_time(const char *value);
int parse_url(const char *url, char *server, int *port, char *path);
int http_get_request(const char *url, char *param, char *resp);
int curl_get_request(const char *url, char *resp);
void base64Encode(char *intext, char *output);
void dump_sch(char *sch, int n, int m);
int check_sch(char *sch, int i, int j);
int check_peroid(long start_time_ms, int timeout_ms);
char *get_time_str(char *time_str, char *day_str, char *other_str, int *wday);
char *time_add(char *time_str, int diff_sec);
int get_sd_capacity(long long *remain, long long *total);
char *time_now(char *str);
int check_file_type(const char *file_name, const char *type);
char *get_file_list(const char *dir_path, const char *type);
int remove_sd(int file_type, int limit, RECOREING_PATH recpath);
long long check_rec_filename(const char *filename, const char *time_start, const char *time_end);
int check_rec_dirname(const char *filename, const char *time_start, const char *time_end);
char* find_rec(const char *time_start, const char *time_end);
int get_file_len(const char *name);
int time_diff(struct timeval start, struct timeval end, struct timeval *diff);
int get_ap_list(AP_INFO **pApList);
int connect_wifi(char *ssid, char *key, int authmode);
int connect_wifi_wep(char *ssid);
int stop_connect_wifi(void);
int format_sd(void);
void dump_data(const char *flag, unsigned char *data, int len);
int sd_exist(void);
void mkdirs(const char *dir);
int AjustTZ(void);
int mount_nfs(const char *local_path, const char *remote_path);
int get_param_type(void *param);
int calculate_avi_file_real_length(char * file, char *path, int rec_type);
int FindAlarmRecordFile(struct tm time, char *path, int period);
int FindNormalRecordFile(struct tm time, char *path, int period);
int RemoveRecordIndex(const char *InfoFilePath, int record_type, int limit, RECOREING_PATH *recpath);
int WriteRecordIndex(RECORD_INFO RecordInfo, const char *InfoFilePath);
int ChangeRecIndexEndtimeType(char *endtime, char *path, int flag);
int delete_destroy_record_index(RECOREING_PATH *recpath);
long get_time_ms(void);
void print_pid(const char *tag);
int wifi_test(char *new_ssid, char *new_key, int new_authmode, char *ssid, char *key, int authmode);
int isIpv4(const char *str);
int CheckSch_Alarm(ALARM_PARAM *pAlarmParam);
int write_danale_uid_file(char *data,char *md5);
int memcmp_ext(const void *buffer1, const void *buffer2, int count);
int wait_record_index_file_free(int flag);

#ifdef __cplusplus
}
#endif	/* __cplusplus */

#endif
