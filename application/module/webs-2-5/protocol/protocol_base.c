#include "protocol/protocol_base.h"
#include "protocol/h264_protocol.h"
#include "common.h"
#include "param.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>


int ProcStreamProcotol(int fd, char *http_request)
{
    /* tenvis procotol */
#if 0    
    if (CheckMjpegPushProcotol(fd, http_request) == SDK_OK)
        return SDK_OK;
#endif    
    if (CheckH264Procotol(fd, http_request) == SDK_OK)
        return SDK_OK;

    return SDK_ERROR;
}

