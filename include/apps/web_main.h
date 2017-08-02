#ifndef		__WEB_MAIN_H__
#define		__WEB_MAIN_H__

#ifdef __cplusplus
extern "C" {
#endif

#include "common.h"

void restart_web_server(void);
void set_web_port(int http_port);
int start_web_server(void);
void stop_web_server(void);
void web_set_interface(SET_PARAM_STR_FUNC set_param_str_inf, 
                        SET_PARAM_INT_FUNC set_param_int_inf, 
                        GET_PARAM_FUNC get_param_inf, 
                        CMD_PROC_FUNC cmd_proc_inf);

#ifdef __cplusplus
}
#endif

#endif
