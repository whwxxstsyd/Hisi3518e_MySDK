#include "hi_codec.h"
#include "rtsp_global.h"
#include "av_buffer.h"
#include <unistd.h>

int main(int argc, char *argv[])
{
    rtsp_start();
    while(1) 
		sleep(1);
    
    rtsp_stop();
}

