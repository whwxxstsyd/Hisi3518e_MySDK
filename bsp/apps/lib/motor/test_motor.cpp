#include "motor.h"
#include "common.h"

#include <unistd.h>

int main(int argc, char *argv[])
{
	while (1)
	{
		printf("motor status: %d\n", get_motor_status());
		sleep(1);
	}

	return 0;
}

