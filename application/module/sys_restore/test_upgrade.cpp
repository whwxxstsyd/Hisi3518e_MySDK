#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <fcntl.h>
#include <sys/reboot.h>

int main(int argc, char *argv[])
{
	system("flashcp -v /tmp/rootfs_64k.jffs2 /dev/mtd2");
	reboot(RB_AUTOBOOT);

	return 0;
}

