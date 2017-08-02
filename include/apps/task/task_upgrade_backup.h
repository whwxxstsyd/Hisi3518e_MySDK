#ifndef		__TASK_UPGRADE_BACKUP_H__
#define		__TASK_UPGRADE_BACKUP_H__

#include "task_base.h"
#include "common.h"

#define     USE_NAND_FLASH          0

#define     KEY_VERSION                 "version"
#define     KEY_USA_SERVER              "usa_server"
#define     KEY_JAPAN_SERVER            "japan_server"
#define     KEY_CHINA_SERVER            "china_server"
#define     KEY_EUROPE_SERVER           "europe_server"

#define     LASTEST_UBOOT_VERSION       "lastest_uboot"
#define     LASTEST_KERNEL_VERSION      "lastest_kernel"
#define     LASTEST_ROOTFS_VERSION      "lastest_rootfs"
#define     BUILD_TIME                  "build_time"
#define     VERSION_DESCRIPTION         "description"

#define     UBOOT_URL               "uboot_url"
#define     KERNEL_URL              "kernel_url"
#define     ROOTFS_URL              "rootfs_url"
#define     WEB_URL                 "web_url"

typedef struct OTA_INFO_t
{
    char lastest_uboot[16];
    char lastest_kernel[16];    
    char lastest_rootfs[16];
    char build_time[16];
    char description[1024];
    char uboot_url[256];
    char kernel_url[256];    
    char rootfs_url[256];    
    char web_url[256];        
}OTA_INFO;

typedef enum IMAGE_TYPE_MASK_T
{
    UBOOT_MASK = BIT(1),
    KERNEL_MASK = BIT(2),
    ROOTFS_MASK = BIT(3),    
    WEB_MASK = BIT(4),        
}IMAGE_TYPE_MASK;

class CTaskUpgradeBackup: public CTaskBase
{
    public:
		static CTaskUpgradeBackup *getInstance()
		{
			static CTaskUpgradeBackup *obj = NULL;
			if (NULL == obj)
			{
				obj = new CTaskUpgradeBackup();		
			}
			return obj;
		}	
		
        CTaskUpgradeBackup();
        ~CTaskUpgradeBackup();
		int Init(void);
		void Uninit(void);	
        void Process(void);	
        int OnlineUpgradeProc(void);

	private:	
        int mMsgHandle;
        pthread_t mCheckVersionId;

    public:
        RAM_PARAM *mpRamParam;
        MISC_PARAM *mpMiscParam;        
        FACTORY_PARAM *mpFactoryParam;                
};

int upgrade_backup_task_create(void);
void upgrade_backup_task_destory(void);


#endif
