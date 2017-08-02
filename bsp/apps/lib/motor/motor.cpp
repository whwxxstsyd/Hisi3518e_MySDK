#include "motor.h"
#include "motor_drv.h"
#include "debug.h"

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <fcntl.h>
#include <string.h>

#undef		TARGET_LEVEL
#define		TARGET_LEVEL		LEVEL_INFO

#define		CMD_PATH		"/sys/class/motor/motor/ctrl"
#define		SPEED_PATH		"/sys/class/motor/motor/speed"
#define		POS_PATH		"/sys/class/motor/motor/position"		/* index_x_y */
#define		MOTOR_STATUS	"/sys/class/motor/motor/status"
#define		CIRCLE_PATH		"/sys/class/motor/motor/circle_cnt"
#define		LIMIT_PATH		"/sys/class/motor/motor/limit"

/* [up/down/left/right] */
int motor_do_cmd(const char *cmd)
{
	int fd;
	char data[STR_64_LEN] = {0};

	if ((fd = open(CMD_PATH, O_RDWR)) < 0)
	{
		LOG_ERROR("open fail[%s]\n", CMD_PATH);	
		return SDK_ERROR;
	}
	sprintf(data, "%s\n", cmd);
	LOG_INFO("data : %s\n",data);
	write(fd, data, strlen(data));
	close(fd);

	return SDK_OK;
}

/* [go_to_pos/get_pos] */
int motor_pos_ctrl(int method, MOTOR_POS *pMotorPos)
{
	int fd;
	char data[STR_64_LEN] = {0};

	if ((fd = open(POS_PATH, O_RDWR)) < 0)
	{
		LOG_ERROR("open fail[%s]\n", POS_PATH);
		return SDK_ERROR;
	}
	if (method == GOTO_POS)
	{
		LOG_INFO("goto pos[%d_%d]\n", pMotorPos->x, pMotorPos->y);
		sprintf(data, "%d_%d", pMotorPos->x, pMotorPos->y);
		write(fd, data, strlen(data));
	}
	else if (method == GET_POS)
	{
		read(fd, data, sizeof(data));
		sscanf(data, "%d_%d", &pMotorPos->x, &pMotorPos->y);
		LOG_INFO("get pos [%d:%d]\n", pMotorPos->x, pMotorPos->y);
	}

	close(fd);

	return SDK_OK;	
}

int motor_set_speed(int speed)
{
	int fd;
	char data[STR_64_LEN] = {0};

	if ((fd = open(SPEED_PATH, O_RDWR)) < 0)
	{
		LOG_ERROR("open fail[%s]\n", SPEED_PATH);
		return SDK_ERROR;
	}
	LOG_INFO("set_speed[0x%x]\n", speed);
	sprintf(data, "0x%x\n", speed);
	write(fd, data, strlen(data));
	close(fd);

	return SDK_OK;
}

int motor_set_circle(int cnt)
{
	int fd;
	char data[STR_64_LEN] = {0};

	if ((fd = open(CIRCLE_PATH, O_RDWR)) < 0)
	{
		LOG_ERROR("open fail[%s]\n", CIRCLE_PATH);
		return SDK_ERROR;
	}
	LOG_INFO("cnt [%d]\n", cnt);
	sprintf(data, "0x%x\n", cnt);
	write(fd, data, strlen(data));
	close(fd);

	return SDK_OK;
}

int motor_set_limit(int x, int y)
{
	int fd;
	char data[STR_64_LEN] = {0};

	if ((fd = open(LIMIT_PATH, O_RDWR)) < 0)
	{
		LOG_ERROR("open fail[%s]\n", CIRCLE_PATH);
		return SDK_ERROR;
	}	
	LOG_INFO("limit [%d_%d]\n", x, y);
	sprintf(data, "%d_%d\n", x, y);
	write(fd, data, strlen(data));
	close(fd);

	return SDK_OK;
}

int get_motor_status(void)	/* 0: stop 1: run */
{
#if 1
	int fd;
	char data[STR_64_LEN] = {0};
	int status;

	fd = open(MOTOR_STATUS, O_RDWR);
	if (fd < 0)
	{
//		LOG_ERROR("open %s fail\n", MOTOR_STATUS);
		return SDK_ERROR;
	}
	read(fd, data, sizeof(data));
	sscanf(data, "%d", &status);
	close(fd);

	return status;
#else
	return 0;
#endif
}

