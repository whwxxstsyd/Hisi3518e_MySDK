#ifndef     __ONVIF_INF_H__
#define     __ONVIF_INF_H__

#include <pthread.h>

#include "common.h"

#ifdef	__cplusplus
extern "C" { 
#endif

#define DEFAULT_ONVIF_SERVER_PORT	8000
#define DEFAULT_ONVIF_WSD_PORT 		3702

typedef struct _HI_ONVIF_MNG_S_
{
	int quit;
	int udpSocket;
	pthread_t probeThread;
	pthread_t serverThread;
}HI_ONVIF_MNG_S;

int onvif_init(void);
void onvif_uninit(void);
void onvif_set_interface(SET_PARAM_STR_FUNC set_param_str_inf, 
                        SET_PARAM_INT_FUNC set_param_int_inf, 
                        GET_PARAM_FUNC get_param_inf, 
                        CMD_PROC_FUNC cmd_proc_inf);

#ifdef	__cplusplus
}
#endif

#endif

