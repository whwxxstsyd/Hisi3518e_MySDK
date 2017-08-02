#!/bin/sh

. $BOARD_DIR/.config

SRC_DIR=$SDK_TOPDIR/rootfs
TARGET_DIR=$SDK_TOPDIR/out/$BOARD/rootfs/target

mkdir -p $TARGET_DIR

# 不同的电脑makedevs无法执行
which makedevs
if [ $? -eq 0 ]; then
	makedevs -d $SDK_TOPDIR/rootfs/device_table.txt $TARGET_DIR
else
	$TOOLS_DIR/makedevs -d $SDK_TOPDIR/rootfs/device_table.txt $TARGET_DIR
fi
tar zxf $PLATFORM_DIR/lib/lib.stdc++.tgz -C $TARGET_DIR
tar zxf $PLATFORM_DIR/lib/lib.uClibc.tgz -C $TARGET_DIR
cp $SRC_DIR/etc/* $TARGET_DIR/etc/ -rf
if [ "$DEBUG_MODULES" = "y" ]; then
	cp $SRC_DIR/passwd/shadow_debug $TARGET_DIR/etc/shadow -rf
else
	cp $SRC_DIR/passwd/shadow_release $TARGET_DIR/etc/shadow -rf
fi
cp $BOARD_DIR/.config $TARGET_DIR/etc -rf

# 备份文件系统
if [ "$WITH_BACKUP_ROOTFS" = "y" ]; then
	BACKUP_TARGET_DIR=$SDK_TOPDIR/out/$BOARD/rootfs/backup
	mkdir -p $BACKUP_TARGET_DIR
	which makedevs
	if [ $? -eq 0 ]; then
		makedevs -d $SDK_TOPDIR/rootfs/device_table.txt $BACKUP_TARGET_DIR
	else
		$TOOLS_DIR/makedevs -d $SDK_TOPDIR/rootfs/device_table.txt $BACKUP_TARGET_DIR
	fi
	tar zxf $PLATFORM_DIR/lib/lib.stdc++.tgz -C $BACKUP_TARGET_DIR
	tar zxf $PLATFORM_DIR/lib/lib.uClibc.tgz -C $BACKUP_TARGET_DIR
	cp $SRC_DIR/etc/* $BACKUP_TARGET_DIR/etc/ -rf
	cp $SRC_DIR/passwd/shadow_release $BACKUP_TARGET_DIR/etc/shadow -rf
	cp $BOARD_DIR/.config $BACKUP_TARGET_DIR/etc -rf
fi

echo "Create rootfs end"
