#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>

#include "normal.h"
#include "hi_gpio.h"
#include "string_parser.h"

#if 1
int main(int argc, char *argv[])
{
	int value = 1;
	
	while (1)
	{
		value = ~value;
		set_gpio_value(LED_GPIO, value);
		sleep(1);
	}

	return 0;
}
#elif 0
int main(int argc, char *argv[])
{
	while (1)
	{
		ir_ctrl(IR_ON);
		sleep(2);
		ir_ctrl(IR_OFF);
		sleep(2);
	}
}
#else
int main(int argc, char *argv[])
{
	int value = 1;
	
	while (1)
	{
		value = ~value;
		set_gpio_value(COLOR_GPIO, value);
		sleep(1);
	}

	return 0;
}
#endif

