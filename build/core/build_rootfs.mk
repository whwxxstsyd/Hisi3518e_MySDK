#
# create by liwei
#

LOCAL_PATH:=$(call my-dir)

.PHONY: rootfs

rootfs: $(DOT_CONFIG)
	bash $(SDK_TOPDIR)/tools/gen_fw_info/gen_fw_info.sh > $(ROOTFS_OUT_DIR)/etc/fw_info.ini
	
	if [ ! -d $(IMAGES_DIR) ]; then		\
		mkdir -p $(IMAGES_DIR);	\
	fi
	
	rm -rf $(IMAGES_DIR)/rootfs*
	rm -rf $(ROOTFS_OUT_DIR)/local/bin/test_*
	rm -rf $(ROOTFS_OUT_DIR)/local/bin/*.cgi
	rm -rf $(ROOTFS_OUT_DIR)/lib/modules/3.0.8/extra/gpio_i2c.ko
	rm -rf $(ROOTFS_OUT_DIR)/lib/modules/3.0.8/extra/gpioi2c_ex.ko	
	rm -rf $(ROOTFS_OUT_DIR)/lib/modules/3.0.8/extra/hii2c.ko		
	rm -rf $(ROOTFS_OUT_DIR)/lib/modules/3.0.8/extra/pwm.ko
	rm -rf $(ROOTFS_OUT_DIR)/lib/modules/3.0.8/extra/ssp_ad9020.ko		
	rm -rf $(ROOTFS_OUT_DIR)/lib/modules/3.0.8/extra/ssp_pana.ko	
	rm -rf $(ROOTFS_OUT_DIR)/lib/modules/3.0.8/extra/ssp_sony.ko		
	rm -rf $(ROOTFS_OUT_DIR)/lib/modules/3.0.8/extra/tw2865.ko	
	mkdir $(ROOTFS_OUT_DIR)/www/cgi-bin -p
	$(CROSS_COMPILE)strip $(ROOTFS_OUT_DIR)/lib/*so*
	$(CROSS_COMPILE)strip $(ROOTFS_OUT_DIR)/local/lib/*so*
	$(CROSS_COMPILE)strip $(ROOTFS_OUT_DIR)/local/sbin/*
	-$(CROSS_COMPILE)strip $(ROOTFS_OUT_DIR)/local/bin/*
#	$(SDK_TOPDIR)/tools/mkfs.jffs2 -d $(ROOTFS_OUT_DIR) -l -e 0x40000 --pad=0xc80000 -o $(IMAGES_DIR)/rootfs_256k.jffs2
#	$(SDK_TOPDIR)/tools/mkfs.jffs2 -d $(ROOTFS_OUT_DIR) -l -e 0x20000 --pad=0xc80000 -o $(IMAGES_DIR)/rootfs_128k.jffs2
#	$(SDK_TOPDIR)/tools/mkfs.jffs2 -d $(ROOTFS_OUT_DIR) -l -e 0x10000 --pad=0xc80000 -o $(IMAGES_DIR)/rootfs_64k.jffs2	
#	$(SDK_TOPDIR)/tools/mkfs.jffs2 -d $(ROOTFS_OUT_DIR) -l -e 0x10000 -o $(IMAGES_DIR)/rootfs_64k.jffs2	
ifeq ($(WITH_BACKUP_ROOTFS), y)	
	$(CROSS_COMPILE)strip $(BACKUP_ROOTFS_OUT_DIR)/lib/*so*
	-$(CROSS_COMPILE)strip $(BACKUP_ROOTFS_OUT_DIR)/local/lib/*so*
	$(CROSS_COMPILE)strip $(BACKUP_ROOTFS_OUT_DIR)/local/sbin/*
	-$(CROSS_COMPILE)strip $(BACKUP_ROOTFS_OUT_DIR)/local/bin/*
#	rm -rf $(BACKUP_ROOTFS_OUT_DIR)/local/lib/libWifiCli.a
	rm -rf $(ROOTFS_OUT_DIR)/www/setup.exe
	$(SDK_TOPDIR)/tools/mkfs.jffs2 -d $(BACKUP_ROOTFS_OUT_DIR) -l -e 0x10000 -o $(IMAGES_DIR)/rootfs_backup_64k.jffs2	
#	$(SDK_TOPDIR)/tools/mksquashfs $(BACKUP_ROOTFS_OUT_DIR) $(IMAGES_DIR)/rootfs_backup_64k.squashfs -b 64K -comp xz 1>/dev/null
endif	
	if [ ! -d $(ROOTFS_OUT_DIR)/../config ]; then		\
		mkdir -p $(ROOTFS_OUT_DIR)/../config;	\
	fi	
	$(SDK_TOPDIR)/tools/mkfs.jffs2 -d $(ROOTFS_OUT_DIR)/../config -l -e 0x10000 --pad=0x80000 -o $(IMAGES_DIR)/config_64k.jffs2		
	$(SDK_TOPDIR)/tools/mksquashfs $(ROOTFS_OUT_DIR) $(IMAGES_DIR)/rootfs_256k.squashfs -b 256K -comp xz 1>/dev/null
	$(SDK_TOPDIR)/tools/mksquashfs $(ROOTFS_OUT_DIR) $(IMAGES_DIR)/rootfs_128k.squashfs -b 128K -comp xz 1>/dev/null
	$(SDK_TOPDIR)/tools/mksquashfs $(ROOTFS_OUT_DIR) $(IMAGES_DIR)/rootfs_64k.squashfs -b 64K -comp xz 1>/dev/null
	
#$(call add-target-into-build, rootfs)			/* Ä¿±ê°´Ë³ĞòÒÀÀµ */
