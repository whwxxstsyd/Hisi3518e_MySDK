#ifndef     __HI_CODEC_H__
#define     __HI_CODEC_H__

#include "sample_comm.h"

void hi_set_venc_resolution(PIC_SIZE_E *pSizeArray, int num);
void hi_set_stream_cb(PUT_VIDEO_FRAME_CB put_stream_cb, void *user_data);
void hi_dev_uninit(void);
void hi_mpp_uninit(void);
int hi_set_bitrate(int VencCh, int bitrate);
int hi_set_fps(int VencCh, int fps);
int hi_set_res(int VencCh, int resolution);
int hi_set_color_mode(int mode);		/* 0: black 1: color mode */
int hi_get_color_mode(void);
char** hi_get_jpg(int cnt, const char *tmp_dir);
int hi_get_jpg_ext(char **pData);
HI_S32 SAMPLE_VENC_720P_CLASSIC(HI_VOID);

#endif
