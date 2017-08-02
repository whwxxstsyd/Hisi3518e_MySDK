#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <fcntl.h>
#include <sys/reboot.h>

#include "param.h"
#include "debug.h"
#include "normal.h"
#include "msg.h"
#include "param_base.h"
#include "md5.h"
#include "task/task_upgrade_backup.h"
#include "common.h"
#include "socket.h"
#include "task.h"
#include "dictionary.h"
#include "iniparser.h"
#include "fw_env.h"


#undef		TARGET_LEVEL
#define		TARGET_LEVEL		LEVEL_DEBUG

CTaskUpgradeBackup::CTaskUpgradeBackup(void)
{
	strcpy(mTaskName, "UPGRADE_BACKUP");
}

CTaskUpgradeBackup::~CTaskUpgradeBackup(void)
{

}

int KillTask(void)
{
	system("killall -9 platform_hisi");
	//system("killall udevd");

	return SDK_OK;
}

int CheckVersion(const char *pNewVersion, const char *pCurrentVersion)		/* pNewVersion > pCurrentVersion return 1 */
{
	int v1, v2, v3, v4, NewVers, CurrentVers;

	sscanf(pNewVersion, "%d.%d.%d.%d", &v1, &v2, &v3, &v4);
	NewVers = (v1 << 24) + (v2 << 16) + (v3 << 8) + v4;
	sscanf(pCurrentVersion, "%d.%d.%d.%d", &v1, &v2, &v3, &v4);
	CurrentVers = (v1 << 24) + (v2 << 16) + (v3 << 8) + v4;	

	return (NewVers > CurrentVers ? 1 : 0);
}

char http_request_formate[] = 
{
	"GET %s HTTP/1.1\r\n"
	"Host: %s\r\n"
	"Connection: keep-alive\r\n"
	"Accept: text/html,application/xhtml+xml,application/xml;q=0.9,*/*;q=0.8\r\n"
	"User-Agent: Mozilla/5.0 (Windows NT 6.1) AppleWebKit/537.36 (KHTML, like Gecko) Chrome/28.0.1500.72 Safari/537.36\r\n"
	"Referer: %s\r\n"
	"Accept-Encoding: gzip,deflate,sdch\r\n"
	"Accept-Language: zh-CN,zh;q=0.8\r\n\r\n"
};

int GetRemoteFile(const char *remote_url, char *file)		/* 输入文件目录,返回文件路径 */
{
	int port, sockfd, fd, ret;
	char ip[32] = {0}, path[64] = {0}, *pString = NULL;
	char send_buf[1024] = {0}, recv_buf[1024 * 10] = {0};
	
	if ((remote_url == NULL) || (file == NULL))
		return -1;
	
	if (parse_url(remote_url, ip, &port, path) < 0)
		return -1;
	printf("GetRemoteFile: %s\n", remote_url);
	LOG_DEBUG("GetRemoteFile: %s\n", remote_url);
	
	pString = strrchr(path, '/');
	if (pString)
	{
		pString += 1;
		strcat(file, pString);
	}
	else
		strcat(file, path);
	fd = open(file, O_CREAT | O_WRONLY | O_TRUNC);
	if (fd < 0)
		return -1;
	if ((sockfd = tcp_noblock_connect(ip, port, 5000)) < 0)
	{
		LOG_ERROR("tcp_block_connect fail[%s_%d]\n", ip, port);
		return -1;
	}
	set_sock_block(sockfd);
	set_sock_attr(sockfd, 1, 10000, 10000, 1024, 1024 * 100);
	sprintf(send_buf, http_request_formate, path, ip, remote_url);
	if ((ret = tcp_block_send(sockfd, send_buf, strlen(send_buf))) < 0)
	{
		LOG_ERROR("tcp_block_send fail\n");
		return -1;
	}
	/* 读取头 */
	while (1)
	{
		if ((ret = tcp_block_recv(sockfd, recv_buf, sizeof(recv_buf))) <= 0)
		{
			LOG_ERROR("tcp_block_recv fail[%d]\n", ret);
			break;
		}
		pString = strstr(recv_buf, "\r\n\r\n");
		if (pString)
		{
			pString += strlen("\r\n\r\n");
			ret -= (pString - recv_buf);
			break;
		}
	}
	if (ret > 0)
		write(fd, pString, ret);
	while (1)
	{
		ret = tcp_block_recv(sockfd, recv_buf, sizeof(recv_buf));
		if (ret < 0)
		{
			LOG_ERROR("tcp_block_recv fail[%d]\n", ret);
			break;
		}
		else if (ret == 0)	/* 接收完毕 */
			break;
		write(fd, recv_buf, ret);
	}
	close_socket(&sockfd);
	close(fd);

	return 0;
}

int LocalUpgrade(const char *path)
{
	int fd, type;
	char cmd[256] = {0};
	FW_HEADER fw_header;
	unsigned char Md5Bin[16] = {0};
	char Md5Str[CHECKSUM_LEN + 1] = {0};	
	
	if (path == NULL)
		return BAD_PARAM;

	LOG_INFO("upgrade_file: %s\n", path);

	/* 处理升级固件 */
	fd = open(path, O_RDWR);
	if (fd < 0)
	{
		LOG_ERROR("open fail[%s]\n", path);
		return -1;
	}
	lseek(fd, -sizeof(FW_HEADER), SEEK_END);
	read(fd, &fw_header, sizeof(FW_HEADER));
	LOG_INFO("get fw:\n"
			"name: |%s|\n"
			"version: %s\n"
			"md5sum: %s\n"
			"len: %d\n", fw_header.filename, fw_header.version, fw_header.md5sum, fw_header.filelen);
	truncate(path, fw_header.filelen);		/* 剪掉文件尾ftruncate */
	close(fd);
	/* 校验checksum */
    md5_file(path, Md5Bin);	
	bin2hex(Md5Str, Md5Bin, sizeof(Md5Bin));
	if (strcmp(Md5Str, fw_header.md5sum))
	{
		LOG_ERROR("firmware checksum fail\n");		
		return -1;
	}	
	/* 开始升级 */
	if (!strcmp(fw_header.filename, "uboot"))
	{
#if 0	/* for nand flash */
		system("flash_eraseall /dev/mtd0");
		sprintf(cmd, "nandwrite -p /dev/mtd0 %s", path);
		system(cmd);
#else
		
		system("echo 3 > /proc/sys/vm/drop_caches");
		system("/local/sbin/flash_eraseall /dev/mtd0");
		sprintf(cmd, "/local/sbin/flashcp %s /dev/mtd0", path);
		system(cmd);
#endif
	}
	else if(!strcmp(fw_header.filename, "kernel"))
	{
		system("echo 3 > /proc/sys/vm/drop_caches");
		system("/local/sbin/flash_eraseall /dev/mtd1");
		sprintf(cmd, "/local/sbin/flashcp %s /dev/mtd1", path);
		system(cmd);
	}
	else if(!strcmp(fw_header.filename, "rootfs"))	
	{
		system("echo 3 > /proc/sys/vm/drop_caches");
		system("/local/sbin/flash_eraseall /dev/mtd2");
		sprintf(cmd, "/local/sbin/flashcp %s /dev/mtd2", path);
		system(cmd);
	}
	return SDK_OK;
}

int CTaskUpgradeBackup::OnlineUpgradeProc(void)
{
	int ret;
	OTA_INFO OtaInfo;
	char *pString = NULL, key[64] = {0};
	char path[STR_64_LEN] = {"/tmp/"};
	dictionary	*Dictionary = NULL;
	int argc = 3;
	char *argv[3] = {(char *)"fw_setenv", 
		             (char *)"bootargs", 
		             (char *)"mem=40M console=ttyAMA0,115200 root=/dev/mtdblock2 rootfstype=squashfs mtdparts=hi_sfc:512K(boot),2560K(kernel),9728K(rootfs),512K(config),3072K(backup) init=/linuxrc"};
	
	ret=GetRemoteFile(mpFactoryParam->version_desc, path);
	if(ret==-1)
	{
		LOG_ERROR("===========GetRemoteFile is error=============\n");
		return -1;
	}
	Dictionary = iniparser_load(path);
	if (Dictionary == NULL)
	{
		LOG_ERROR("error at iniparser_load\n");
		return -1;
	}
	/* check uboot */
	sprintf(key, "%s:%s", KEY_VERSION, LASTEST_UBOOT_VERSION);	
	if (NULL != (pString = iniparser_getstring(Dictionary, key, NULL)))
		strcpy(OtaInfo.lastest_uboot, pString);		
	
	if (CheckVersion(OtaInfo.lastest_uboot, mpMiscParam->uboot_version))
	{
		LOG_INFO("download lastest uboot, and upgrade firmware\n");		
		sprintf(key, "%s_server:%s", mpFactoryParam->region, UBOOT_URL);
		if (NULL != (pString = iniparser_getstring(Dictionary, key, NULL)))
		{
			KillTask();		
			mpRamParam->system_status.upgrade = 1;				
			strcpy(OtaInfo.uboot_url, pString);				
			strcpy(path, "/tmp/");					
			GetRemoteFile(OtaInfo.uboot_url, path);	
			goto LOCAL_UPGRADE;
		}
	}	
	/* check kernel */
	sprintf(key, "%s:%s", KEY_VERSION, LASTEST_KERNEL_VERSION);
	if (NULL != (pString = iniparser_getstring(Dictionary, key, NULL)))
		strcpy(OtaInfo.lastest_kernel, pString);
	if (CheckVersion(OtaInfo.lastest_kernel, mpMiscParam->kernel_version))
	{
		LOG_INFO("download lastest kernel, and upgrade firmware\n");		
		sprintf(key, "%s_server:%s", mpFactoryParam->region, KERNEL_URL);
		if (NULL != (pString = iniparser_getstring(Dictionary, key, NULL)))
		{
			KillTask();
			mpRamParam->system_status.upgrade = 1;								
			strcpy(OtaInfo.kernel_url, pString);	
			strcpy(path, "/tmp/");			
			GetRemoteFile(OtaInfo.kernel_url, path);
			goto LOCAL_UPGRADE;				
		}
	}
	/* check rootfs */	
	sprintf(key, "%s:%s", KEY_VERSION, LASTEST_ROOTFS_VERSION);
	if (NULL != (pString = iniparser_getstring(Dictionary, key, NULL)))
		strcpy(OtaInfo.lastest_rootfs, pString);	
	
	printf("==========OtaInfo.lastest_rootfs=%s==mpMiscParam->soft_version=%s==\n",OtaInfo.lastest_rootfs, mpMiscParam->soft_version);
	if (CheckVersion(OtaInfo.lastest_rootfs, mpMiscParam->soft_version))
	{
		LOG_INFO("download lastest rootfs, and upgrade firmware\n");		
		sprintf(key, "%s_server:%s", mpFactoryParam->region, ROOTFS_URL);
		if (NULL != (pString = iniparser_getstring(Dictionary, key, NULL)))
		{
			KillTask();			
			mpRamParam->system_status.upgrade = 1;								
			strcpy(OtaInfo.rootfs_url, pString);	
			strcpy(path, "/tmp/");					
			GetRemoteFile(OtaInfo.rootfs_url, path);	
			goto LOCAL_UPGRADE;				
		}
	}
	iniparser_freedict(Dictionary);	
	return SDK_OK;

LOCAL_UPGRADE:	
	ret=LocalUpgrade(path);
	if(ret!=SDK_OK)
	{
		LOG_ERROR("=======update is failed\n");
		return SDK_OK;
	}
	//升级成功从第二分区正常启动
	fw_setenv(argc, argv);
	reboot(RB_AUTOBOOT);	
	return 1;
}

void CTaskUpgradeBackup::Process(void)
{	
	while (mRunning)
	{
		if (OnlineUpgradeProc() > 0)
			break;
		else
			usleep(500 * 1000);
	}
}

int CTaskUpgradeBackup::Init(void)
{
	mpFactoryParam = get_factory_param();
	mpMiscParam = get_misc_param();
	mpRamParam = get_ram_param();

	return SDK_OK;
}

void CTaskUpgradeBackup::Uninit(void)
{

}

int upgrade_backup_task_create(void)
{	
	return CTaskUpgradeBackup::getInstance()->Create();
}

void upgrade_backup_task_destory(void)
{
	return CTaskUpgradeBackup::getInstance()->Destory();
}

