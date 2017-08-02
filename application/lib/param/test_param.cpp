#include "param_ext.h"
#include "debug.h"

#include <string.h>

int main(int argc, char *argv[])
{
	AVC_HEADER *pAvcHeader = NULL;

	pAvcHeader = get_avc_header();
	LOG_INFO("%x %x %x %x\n", pAvcHeader[1].pps.data[0], pAvcHeader[1].pps.data[1], 
			pAvcHeader[1].pps.data[2], pAvcHeader[1].pps.data[3]);

	return 0;
}

