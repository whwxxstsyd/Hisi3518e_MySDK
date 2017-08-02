#include "avilib.h"
#include "av_buffer.h"
#include "param.h"
#include "common.h"
#include "debug.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#if 0
#define		OUT_FILE_NAME		"/tmp/stream.avi"
#define		VIDEO_WIDTH			(640)
#define		VIDEO_HEIGHT		(480)
#define		FRAME_CNT			(450)
#define		VIDEO_FPS			(30)

int main(int argc, char *argv[])
{
	int frame_cnt = 0;
	avi_t *out_file = NULL;
	REAL_FRAME frame;
	int type = MAIN_STREAM_FRAME | FRAME_TYPE_NEAR_G711;

	out_file = AVI_open_output_file(OUT_FILE_NAME);
	AVI_set_video(out_file, VIDEO_WIDTH, VIDEO_HEIGHT, VIDEO_FPS, "H264");
	AVI_set_audio(out_file, 1, 8000, 16, WAVE_FORMAT_ALAW, 64);
	
	frame.magic = 0;
	while (1)
	{
		av_read_frame(type, &frame);		
		if (frame.type & MAIN_STREAM_FRAME)
			AVI_write_frame(out_file, frame.data + FRAME_HEAD_LEN, frame.len - FRAME_HEAD_LEN, !!(frame.type & IDR_MASK));
		else if (frame.type == FRAME_TYPE_NEAR_G711)
		{
			AVI_write_audio(out_file, frame.data + FRAME_HEAD_LEN, frame.len - FRAME_HEAD_LEN);
		}
		frame_cnt++;
		if (frame_cnt == FRAME_CNT)
			break;
	}
	AVI_close(out_file);	               

	return 0;
}
#else
#define		AVI_FIEL_PATH		"/mnt/sdcard/record/alarm/19700101/19700101_080048_080103.avi"

int main(int argc, char *argv[])
{
	avi_t *avifile;
	int fps, frame_len, frame_type, av_index, frame_cnt = 0;
	char FrameBuffer[MAX_FRAME_SIZE] = {0};

	avifile = AVI_open_input_file(AVI_FIEL_PATH, 1);
	fps = AVI_frame_rate(avifile);
	AVI_seek_start(avifile);
	while (1)
	{
		frame_len = read_avi_frame(avifile, FrameBuffer, &frame_type);
		if (frame_len < 0) 
		{
			LOG_INFO("frame_len[%d][%s][%d]\n", frame_len, AVI_FIEL_PATH, frame_cnt);
			break;
		}
		frame_cnt++;
		LOG_INFO("----------- read a frame[%d][%d][%d]\n", frame_cnt, frame_type, frame_len);	
		switch (frame_type)
		{
			case 1: 	/* audio */
				break;
			case 2:		/* P/B frame */
				usleep(1000 * 1000 / fps);
				break;
			case 3:		/* i frame */
				usleep(1000 * 1000 / fps);				
				break;				
			default:	/* end */                                  
				LOG_INFO("Record is end\n");
				goto PLAYBACK_END;				
                break;
		}		
	}

PLAYBACK_END:
	if (avifile)
	    AVI_close(avifile);	
}
#endif
