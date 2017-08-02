#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>

#include <sys/ipc.h>
#include <sys/shm.h>
#include <sys/sem.h>

#include "normal.h"
#include "hi_gpio.h"
#include "debug.h"
#include "sem.h"
#include "my_timer.h"
#include "common.h"
#include "param_base.h"

CGpio::CGpio(void)
{
	mIrCutStatus = -1;
	mGpioSemID = -1;
	mIrCutTimer = NULL;
}

CGpio::~CGpio(void)
{

}

int CGpio::Init(void)
{
	mGpioSemID = CreateSem(KEY_SEM_GPIO_CTRL);	
    if (mGpioSemID < 0)
    {
        LOG_ERROR("shmget error\n");
		return SDK_ERROR;		
    }	

	set_direction(DEFAULT_GPIO, DIR_INPUT);/*reset*/
	set_direction(LED_GPIO, DIR_OUTPUT);	/*bule led*/
	set_direction(LED_RED_GPIO, DIR_OUTPUT);/*red led*/
	set_direction(IRCUT_GPIO0, DIR_OUTPUT);	/*ir_cut 0*/
	set_direction(IRCUT_GPIO1, DIR_OUTPUT);	/*ir_cut 1*/
	set_direction(IRLED_GPIO, DIR_OUTPUT);	/*ir led*/			
	set_direction(AUDIO_GPIO, DIR_OUTPUT);  /*speaker*/
	return 0;	
}

void CGpio::Uninit(void)
{
	UninitSem(mGpioSemID);
}

int CGpio::set_direction(int gpio, int direction)
{
	char file[128] = {0};
	char data[64] = {0};

	GetSem(mGpioSemID);

	/* write gpio */
	sprintf(file, "%s/gpio_var", GPIO_PATH);
	sprintf(data, "0x%x", gpio);
	write_data_to_file(file, data, strlen(data));
	/* write direction */
	sprintf(file, "%s/direction_var", GPIO_PATH);
	sprintf(data, "0x%x", direction);
	write_data_to_file(file, data, strlen(data));

	PostSem(mGpioSemID);	
	
	return 0;	
}

int CGpio::set_value(int gpio, int value)
{
	char file[128] = {0};
	char data[64] = {0};

	GetSem(mGpioSemID);

	/* write gpio */
	sprintf(file, "%s/gpio_var", GPIO_PATH);
	sprintf(data, "0x%x", gpio);
	write_data_to_file(file, data, strlen(data));
	/* write value */	
	sprintf(file, "%s/value_var", GPIO_PATH);
	sprintf(data, "0x%x", value);
	write_data_to_file(file, data, strlen(data));	

	PostSem(mGpioSemID);
	
	return 0;
}

int CGpio::get_value(int gpio)
{
	char file[128] = {0};
	char data[64] = {0};
	int value = 0, len;

	GetSem(mGpioSemID);

	/* write gpio */
	sprintf(file, "%s/gpio_var", GPIO_PATH);
	sprintf(data, "0x%x", gpio);
	write_data_to_file(file, data, strlen(data));
	/* read value */
	sprintf(file, "%s/value_var", GPIO_PATH);
	sprintf(data, "0x%x", value);	
	read_data_from_file(file, data, &len);	
	sscanf(data, "g_var: 0x%x\n", &value);

	PostSem(mGpioSemID);

	return value;
}

void IrCtrlProc(void *pArg, void *pTimer)
{
	CGpio *pCGpio = (CGpio *)pArg;

	pCGpio->set_value(IRCUT_GPIO0, 0);
	pCGpio->set_value(IRCUT_GPIO1, 0);	

	pCGpio->mIrCutTimer = NULL;
	LOG_INFO("restore ir gpio\n");
}

int CGpio::ir_ctrl(int value)
{	
	if (value == IRLED_ON)
	{
		set_value(IRCUT_GPIO0, 1);
		set_value(IRCUT_GPIO1, 0);
		
		LED_STATUS_ONOFF *led_status = get_led_status_onoff_param();
		if(led_status->ir_led_status == LED_CTL_MANUL)
		{
			set_value(IRLED_GPIO, 0);
		}
		else
		{
			set_value(IRLED_GPIO, 1);
		}
	
	}
	else if (value == IRLED_OFF)		
	{
		set_value(IRCUT_GPIO0, 0);
		set_value(IRCUT_GPIO1, 1);	
		
		set_value(IRLED_GPIO, 0);		
	}

	if (NULL == mIrCutTimer)
		mIrCutTimer = alloc_timer(3000, TYPE_ONCE, IrCtrlProc, 0, this);
	mIrCutStatus = value;

	return SDK_OK;
}

int CGpio::ir_ctrl_status(void)
{
	return mIrCutStatus;
}

int set_gpio_value(int gpio, int value)
{
//	LOG_INFO("gpio value: %d_%d\n", gpio, value);
	return CGpio::getInstance()->set_value(gpio, value);
}

int get_gpio_value(int gpio)
{
	return CGpio::getInstance()->get_value(gpio);
}

int set_gpio_direction(int gpio, int direction)
{
	return CGpio::getInstance()->set_direction(gpio, direction);
}

int ir_ctrl(int value)
{
	return CGpio::getInstance()->ir_ctrl(value);
}

int ir_status(void)
{
	return CGpio::getInstance()->ir_ctrl_status();
}

int gpio_init(void)
{
	return CGpio::getInstance()->Init();
}

