#include "avilib.h"
#include "av_buffer.h"
#include "param.h"
#include "common.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#define		IN_FILE_NAME		"/sdcard/20141018/record/20141018_094223_095026.avi"
#define		OUT_FILE_NAME		"./bak.avi"
#define		VIDEO_WIDTH			(640)
#define		VIDEO_HEIGHT		(480)
#define		FRAME_CNT			(250)
#define		VIDEO_FPS			(30)

int main(int argc, char *argv[])
{
	int frame_cnt = 0;
	avi_t *in_file = NULL;	
	avi_t *out_file = NULL;
	int i;
	char *format;
	int fps;
	int width;
	int height;
	int frame_len = 0;
	int ret;
	char video_buf[600 * 1024] = {0};

	in_file = AVI_open_input_file(argv[1], 1);
	out_file = AVI_open_output_file(OUT_FILE_NAME);	
	
	AVI_set_video(out_file,  AVI_video_width(in_file), 
							AVI_video_height(in_file), 
							AVI_frame_rate(in_file), 
							AVI_video_compressor(in_file));
    AVI_set_audio(out_file, AVI_audio_channels(in_file),
                            AVI_audio_rate(in_file),
                            AVI_audio_bits(in_file),
                            AVI_audio_format(in_file),
                            AVI_audio_mp3rate(in_file));
	
	if (AVI_seek_start((in_file))) //寻找文件开始
	{
		printf("bad seek start\n");			
		return -1;
	}
	while (1)
	{				
		ret = read_avi_frame(in_file, video_buf, &frame_len);
		if (ret < 0) break;
		switch (ret)
		{
			case 1: 	/* audio */
				AVI_write_audio(out_file, video_buf, frame_len);
				break;
			case 2:		/* idr frame */
				AVI_write_frame(out_file, video_buf, frame_len, 1);
				break;
			case 3:		/* p/b frame */
				AVI_write_frame(out_file, video_buf, frame_len, 0);
				break;				
			default:	/* end */
				goto FILE_AT_END;
				break;
		}
	}

FILE_AT_END:
	AVI_close(in_file);	  
	AVI_close(out_file);	               

	return 0;
}

