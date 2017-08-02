#include "param_base.h"
#include "param_ext.h"
#include "param.h"
#include "debug.h"
#include "task/task_ptz.h"
#include "motor.h"
#include "motor_drv.h"
#include "string_parser.h"

#include <stdio.h>
#include <stdlib.h>
#include <fcntl.h>
#include <unistd.h>

#undef		TARGET_LEVEL
#define		TARGET_LEVEL		LEVEL_INFO

CTaskPtz::CTaskPtz(void)
{
	strcpy(mTaskName, "PTZ");
}

CTaskPtz::~CTaskPtz(void)
{
	
}	

void CTaskPtz::Process(void)
{
	while (get_motor_status())		/* µÈ´ýÍ£Ö¹ */
		usleep(10 * 1000);

	LOG_INFO("start_pos : %d\n", mpPtzParam->start_pos);
	
	if (mpPtzParam->start_pos > 0)
		motor_pos_ctrl(GOTO_POS, &mpPtzParam->motor_pos[mpPtzParam->start_pos - 1]);
}

int CTaskPtz::Init(void)
{
	FACTORY_PARAM *pFactoryParam = get_factory_param();

	mpPtzParam = get_ptz_param();

	/*
	switch (mpPtzParam->speed)
	{
		case SPEED_LOW:
			motor_set_speed(0x800);				
			break;
		case SPEED_NORMAL:
			motor_set_speed(0x500);				
			break;				
		case SPEED_HIGH:	
						
			break;				
		default:
			break;
	}
	*/

	motor_set_speed(0x350);	
	motor_set_limit(pFactoryParam->limit_x, pFactoryParam->limit_y);
	motor_set_circle(mpPtzParam->circle_cnt);	
	motor_do_cmd("start");	
	
	return SDK_OK;
}

void CTaskPtz::Uninit(void)
{

}

int ptz_task_create(void)
{
	return CTaskPtz::getInstance()->Create();
}

void ptz_task_destory(void)
{
	return CTaskPtz::getInstance()->Destory();
}	

