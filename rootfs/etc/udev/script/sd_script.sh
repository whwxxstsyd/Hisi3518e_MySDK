#!/bin/sh

var_retry=2

if [ "$ACTION" = "add" ]
then
	mknod /dev/mmcblk0p1 b 179 1
	while [ 1 ]
	do
		/bin/mount -t vfat /dev/mmcblk0p1 /mnt/sdcard
		if [ $? -eq 0 ]; then
			echo "---------------------mount ok-------------------" > /dev/console
			return;
		else
			echo "----------------------mount fail-----------------" > /dev/console;
		fi
		sleep 1;
		var_retry=`expr $var_retry - 1`;
		if [ $var_retry -eq 0 ]; then
			echo "-------------------------stop retry------------" > /dev/console;
			return;
		fi
	done
elif [ "$ACTION" = "remove" ]
then
	/bin/umount /mnt/sdcard
fi
