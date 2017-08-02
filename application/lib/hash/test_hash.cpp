#include "common.h"
#include "debug.h"
#include "string_parser.h"

#include <stdio.h>
#include <stdlib.h>
#include <sys/ipc.h>
#include <sys/msg.h>
#include <sys/types.h>

int main(int argc, char *argv[])
{
	char str[] = "ntp_sync"; 
	
	Str2Index(str);

	return 0;
}

