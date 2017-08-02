#ifndef     __HI_AUDIO_H__
#define     __HI_AUDIO_H__


#ifdef	__cplusplus
extern "C" { 
#endif

#include "common.h"

#include <stdio.h>
#include <stdlib.h>

int set_vin_enable(int value);
int set_vout_enable(int value);
int set_vout_volume(int value);
int set_vin_volume(int value);

#ifdef __cplusplus
}
#endif	/* __cplusplus */

#endif

