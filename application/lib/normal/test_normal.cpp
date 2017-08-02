#include "normal.h"
#include "debug.h"

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>

#if 0
int main(int argc, char *argv[])
{
	int total, remain;
	char *pStr;
	
	get_sd_capacity(&remain, &total);
	LOG_INFO("sd_info: [%dMB/%dMB]\n", remain, total);
#if 1
	pStr = find_rec("20140925211010", "20141017000000");
#else
	pStr = find_rec("20140925", "20141017");
#endif
	printf("file_list:\n%s\n", pStr);
	free(pStr);

	return 0;
}
#elif 0
int main(int argc, char *argv[])
{
	set_time("2014-09-10 11:20:11");

	return 0;	
}
#elif 0
int main(int argc, char *argv[])
{
	if (sd_exist() < 0)
		printf("sd is not exist\n");
	else
	{
		int remain, total;

		get_sd_capacity(&remain, &total);
		printf("sd exist[%d/%d]\n", remain, total);
	}

	return 0;
}
#elif 0
int main(int argc, char *argv[])
{
	printf("record_info_len [%d]\n", sizeof(RECORD_INFO));

	return 0;
}
#else
int main(int argc, char *argv[])
{
	printf("%ld\n", get_time_ms());
	system("cat /proc/uptime");

	return 0;
}
#endif

