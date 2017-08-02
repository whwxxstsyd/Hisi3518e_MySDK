#ifndef     __HI_VIDEO_H__
#define     __HI_VIDEO_H__

#ifdef	__cplusplus
extern "C" { 
#endif

#include "common.h"

#include <stdio.h>
#include <stdlib.h>
int set_ldc_attr(int x,int y,int value);

int set_csc_attr(int type, int value);
int set_rotate(int value);
int hi_get_idr(int ch);
int hi_set_pwrfreq(int freq);

#ifdef __cplusplus
}
#endif	/* __cplusplus */

#endif

