#include "boot_save_default.h"
#include "param.h"
#include "normal.h"
#include "common.h"
#include "debug.h"

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>

int main(int argc, char *argv[])
{
	if (strstr(argv[0], "boot"))
		sys_boot_config();
	else if (strstr(argv[0], "default"))
		sys_default_config();
	else if (strstr(argv[0], "save"))
		sys_save_config();
 
	return SDK_OK;
}