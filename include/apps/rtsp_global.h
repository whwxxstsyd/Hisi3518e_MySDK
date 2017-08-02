#ifndef _RTSP_GLOBAL_H_
#define _RTSP_GLOBAL_H_

#ifdef __cplusplus
extern "C" {
#endif 

int rtsp_set_port(int port);
int rtsp_restart(void);
int rtsp_start(void);
int rtsp_stop(void);
int	rtsp_start_real(int nRtspPort, int bUseAuth);

#ifdef __cplusplus
}
#endif 

#endif
