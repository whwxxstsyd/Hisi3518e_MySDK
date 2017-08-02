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
#include "task/task_upgrade.h"
#include "common.h"
#include "socket.h"
#include "task.h"
#include "dictionary.h"
#include "iniparser.h"
#include "fw_env.h"
#include "av_buffer.h"

#undef		TARGET_LEVEL
#define		TARGET_LEVEL		LEVEL_NONE
void *OnlineUpgradeProc(void *arg);

CTaskUpgrade::CTaskUpgrade(void)
{
	strcpy(mTaskName, "UPGRADE");
}

CTaskUpgrade::~CTaskUpgrade(void)
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
	return 0;/*closed by zhang*/
#if 0
	int timeout_count=0,countsize=0;
	int port, sockfd, fd, ret=-1,filesize=0,temp_size=0,i=0;
	char ip[32] = {0}, path[64] = {0}, *pString = NULL,*pLength0 = NULL,pLength1[12];
	char send_buf[1024] = {0}, recv_buf[1024 * 10] = {0};
	RAM_PARAM *mpRamParam = get_ram_param();
	if ((remote_url == NULL) || (file == NULL))
		return -1;
	
	if (parse_url(remote_url, ip, &port, path) < 0)
		return -1;
	
	pString = strrchr(path, '/');
	if (pString)
	{
		pString += 1;
		strcat(file, pString);
	}
	else
		strcat(file, path);
	sprintf(mpRamParam->ota_param.update_filename,"%s",file);
	fd = open(file, O_CREAT | O_WRONLY | O_TRUNC);
	if (fd < 0)
	{
		return -1;
	}
	if ((sockfd = tcp_noblock_connect(ip, port, 5000)) < 0)
	{
		LOG_ERROR("tcp_block_connect fail[%s_%d]\n", ip, port);
		ret = -1;
		goto OUT;
	}
	set_sock_block(sockfd);
	set_sock_attr(sockfd, 1, 10000, 10000, 1024, 1024 * 100);
	sprintf(send_buf, http_request_formate, path, ip, remote_url);
	if ((ret = tcp_block_send(sockfd, send_buf, strlen(send_buf))) < 0)
	{
		LOG_ERROR("tcp_block_send fail\n");		
		ret = -1;
		goto OUT;
	}
	/* 读取头 */
	while (1)
	{
		if ((ret = tcp_block_recv(sockfd, recv_buf, sizeof(recv_buf))) <= 0)
		{
			LOG_ERROR("tcp_block_recv fail[%d]\n", ret);
			break;
		}
		if(pLength0==NULL)
		{
			pLength0=strstr(recv_buf, "Content-Length:");
			for(i=0;i<1024;i++)
			{
				if(pLength0[strlen("Content-Length:")+i]=='\r')
					break;
				pLength1[i]=pLength0[strlen("Content-Length:")+i];
				
			}
			filesize=atoi(pLength1);
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
	{
		write(fd, pString, ret);
	}
	if(ret==-1)
	{
		goto OUT;
	}
	
	countsize=ret;
	
	while (1)
	{
		ret = tcp_block_recv(sockfd, recv_buf, sizeof(recv_buf));
		if (ret < 0)
		{
			LOG_ERROR("tcp_block_recv fail[%d]\n", ret);
			timeout_count++;
			usleep(500000);
			if(timeout_count==2)
			{
				mpRamParam->ota_param.update_statue=UPGRADE_STATUE_FREE;
				goto OUT;
			}
			else
			{
				continue;
			}	
		}
		
		else if (ret == 0)	/* 接收完毕 */
			break;
		write(fd, recv_buf, ret);
		countsize += ret;
		temp_size  = countsize*100/filesize;
		mpRamParam->ota_param.update_process=temp_size;
		mpRamParam->ota_param.update_statue=UPGRADE_STATUE_DOWNLOADING;
		usleep(100 * 1000);
	}	
	if(temp_size!=100)
	{	
		mpRamParam->ota_param.update_statue=UPGRADE_STATUE_FREE;
	}
	else if(temp_size==100)
	{
		mpRamParam->ota_param.update_statue=UPGRADE_STATUE_DOWNLOADOK;
	}
OUT:	
	close_socket(&sockfd);
	close(fd);
	return ret;
#endif
}

//等待msg_upgrade进程升级返回的消息
int UpdateMsgManage(int MsgHandle,int MsgType)
{
return 0;
#if 0
	int ret=0;
	MESSAGE recv_msg, send_msg;
	FACTORY_PARAM *pFactoryParam = get_factory_param();
	//发送升级指令消息
	send_msg.mType = FW_UPGRADE_TYPE;
    send_msg.mCommand = MsgType;
    ret=SendMsg(MsgHandle,send_msg);
	if(ret!=SDK_OK)
	{
		LOG_ERROR("=====SendMsg is error=====\n");
		
	}
	//等待升级成功返回消息
	if (RecvMsg(MsgHandle, FW_UPGRADE_TYPE, &recv_msg) == SDK_OK)
	{
		LOG_INFO("recv msg: %d\n", recv_msg.mCommand);
		switch (recv_msg.mCommand)
		{
			/* 在线升级 */			
			case ONLINE_UPGRADE_SUCESS:
				printf("Play the audioen sucess\n");
#ifndef hi3518ev200
				if(0 == strncmp(pFactoryParam->region, "china", sizeof(pFactoryParam->region)))
				{
					system("/tmp/cat /tmp/update_ok_ch.wav > /tmp/g711");
				}
				else			
				{
					system("/tmp/cat /tmp/update_ok_en.wav > /tmp/g711");
				}
#endif
				LOG_INFO("======Upgrade is sucess====\n");
				break;
			default:
				LOG_INFO("========ERROR MSG=======\n");
				break;
		}
	}
	return SDK_OK;
#endif
}
int LocalUpgrade(const char *path)
{
	return 0;
#if 0
	int fd, type,ret;
	int UpgradeMsgHandle;
	
	char cmd[256] = {0};
	FW_HEADER fw_header;
	unsigned char Md5Bin[16] = {0};
	char Md5Str[CHECKSUM_LEN + 1] = {0};

	//创建共享内存与处理外部指令进程通信
	if (CreateMsgHandle(&UpgradeMsgHandle, MSG_PROCESS_KEY) < 0)
	{
		LOG_ERROR("CreateMsgHandle fail\n");
		return SDK_ERROR;
	}
	
	if (path == NULL)
		return BAD_PARAM;


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
		UpdateMsgManage(UpgradeMsgHandle,ONLINE_UPGRADE_UBOOT);
	}
	else if(!strcmp(fw_header.filename, "kernel"))
	{
		UpdateMsgManage(UpgradeMsgHandle,ONLINE_UPGRADE_KERNEL);
	}
	else if(!strcmp(fw_header.filename, "rootfs"))	
	{
		UpdateMsgManage(UpgradeMsgHandle,ONLINE_UPGRADE_ROOTFS);
	}
	return SDK_OK;
#endif
}
int GetFilePath(const char *remote_url, char *file)
{
	int port;
	char ip[32]={0},*pString=NULL,path[64] = {0};
	if (parse_url(remote_url, ip, &port, path) < 0)
		return -1;
	
	pString = strrchr(path, '/');
	if (pString)
	{
		pString += 1;
		strcat(file, pString);
	}
	else
		strcat(file, path);
	return 0;
}
//下载线程
void *OnlineDownLoadProc(void *pArg)
{
return NULL;/*closed by zhang*/
#if 0
	int ret;
	CTaskUpgrade *pTaskUpgrade = (CTaskUpgrade *)pArg;
	OTA_INFO OtaInfo;
	char *pString = NULL, key[64] = {0};
	char path[STR_64_LEN] = {"/tmp/"};
	dictionary	*Dictionary = NULL;
	FACTORY_PARAM *pFactoryParam = pTaskUpgrade->mpFactoryParam;	
	MISC_PARAM *pMiscParam = pTaskUpgrade->mpMiscParam;
	RAM_PARAM *pRamParam = pTaskUpgrade->mpRamParam;	
	
	GetFilePath(pFactoryParam->version_desc, path);
	Dictionary = iniparser_load(path);
	if (Dictionary == NULL)
	{
		LOG_ERROR("error at iniparser_load\n");
		return NULL;
	}
	strcpy(path, "/tmp/");
	if (pRamParam->ota_param.need_upgrade & UBOOT_MASK)
	{
		sprintf(key, "%s:%s", KEY_VERSION, LASTEST_UBOOT_VERSION);	
		if (NULL != (pString = iniparser_getstring(Dictionary, key, NULL)))
			strcpy(OtaInfo.lastest_uboot, pString);		
		if (CheckVersion(OtaInfo.lastest_uboot, pMiscParam->uboot_version))
		{
			LOG_INFO("download lastest uboot, and upgrade firmware\n");		
			sprintf(key, "%s_server:%s", pFactoryParam->region, UBOOT_URL);
			if (NULL != (pString = iniparser_getstring(Dictionary, key, NULL)))
			{
#ifdef hi3518ev200			
				KillTask();
#endif	
				pRamParam->system_status.upgrade = 1;				
				strcpy(OtaInfo.uboot_url, pString);				
				GetRemoteFile(OtaInfo.uboot_url, path);	
				goto EXIT;
			}
		}	
	}
	else if (pRamParam->ota_param.need_upgrade & KERNEL_MASK)
	{
		sprintf(key, "%s:%s", KEY_VERSION, LASTEST_KERNEL_VERSION);
		if (NULL != (pString = iniparser_getstring(Dictionary, key, NULL)))
			strcpy(OtaInfo.lastest_kernel, pString);		
		if (CheckVersion(OtaInfo.lastest_kernel, pMiscParam->kernel_version))
		{
			LOG_INFO("download lastest kernel, and upgrade firmware\n");		
			sprintf(key, "%s_server:%s", pFactoryParam->region, KERNEL_URL);
			if (NULL != (pString = iniparser_getstring(Dictionary, key, NULL)))
			{
#ifdef hi3518ev200			
				KillTask();
#endif
				pRamParam->system_status.upgrade = 1;								
				strcpy(OtaInfo.kernel_url, pString);				
				GetRemoteFile(OtaInfo.kernel_url, path);
				goto EXIT;				
			}
		}
	}
	
	else if (pRamParam->ota_param.need_upgrade & ROOTFS_MASK)
	{
		sprintf(key, "%s:%s", KEY_VERSION, LASTEST_ROOTFS_VERSION);
		if (NULL != (pString = iniparser_getstring(Dictionary, key, NULL)))
		{
			strcpy(OtaInfo.lastest_rootfs, pString);
		}
		if (CheckVersion(OtaInfo.lastest_rootfs, pMiscParam->soft_version))
		{
			LOG_INFO("download lastest rootfs, and upgrade firmware\n");	
			sprintf(key, "%s_server:%s", pFactoryParam->region, ROOTFS_URL);
			if (NULL != (pString = iniparser_getstring(Dictionary, key, NULL)))
			{
#ifdef hi3518ev200			
				KillTask();
#endif
				system("sync");
				system("echo 3 > /proc/sys/vm/drop_caches");
				pRamParam->system_status.upgrade = 1;								
				strcpy(OtaInfo.rootfs_url, pString);				
				GetRemoteFile(OtaInfo.rootfs_url, path);
				strcpy(pMiscParam->soft_version,OtaInfo.lastest_rootfs);
				goto EXIT;				
			}
		}
	}	

	iniparser_freedict(Dictionary);	
	return NULL;

EXIT:
#ifdef	WITH_BACKUP_ROOTFS
//如果定义了241BA型号下载完直接升级
#ifdef hi3518ev200

	if(pRamParam->ota_param.update_statue==UPGRADE_STATUE_DOWNLOADOK)
	{
		pthread_t OnlineUpgradeId;
		int argc = 3;
		char *argv[3] = {(char *)"fw_setenv", 
		             	(char *)"bootargs", 
		             	(char *)"mem=36M console=ttyAMA0,115200 root=/dev/mtdblock4 rootfstype=squashfs mtdparts=hi_sfc:512K(boot),2560K(kernel),9728K(rootfs),512K(config),3072K(backup) init=/linuxrc"};
		//升级之前将系统启动参数修改为第四分区，防止升级失败设备起不来
		ret=fw_setenv(argc, argv);
		if(ret!=0)
		{
			LOG_INFO("fw_setenv is error\n");
			return NULL;
		}
		pRamParam->ota_param.update_statue=UPGRADE_STATUE_UPGRADING;
		CreateDetachedTask(&OnlineUpgradeId,OnlineUpgradeProc,NULL);	
	}
#endif
#endif
	return NULL;
#endif
}
//升级线程
void *OnlineUpgradeProc(void *arg)
{
return NULL;
#if 0
	int ret;
	int argc = 3;
	char *argv[3] = {(char *)"fw_setenv", 
		             (char *)"bootargs", 
		             (char *)"mem=36M console=ttyAMA0,115200 root=/dev/mtdblock2 rootfstype=squashfs mtdparts=hi_sfc:512K(boot),2560K(kernel),9728K(rootfs),512K(config),3072K(backup) init=/linuxrc"};
	RAM_PARAM *mpRamParam;
	mpRamParam = get_ram_param();
	ret=LocalUpgrade(mpRamParam->ota_param.update_filename);
	if(ret!=SDK_OK)
	{
		LOG_ERROR("update is failed\n");
		return NULL;
	}
	mpRamParam->ota_param.update_statue=UPGRADE_STATUE_UPGRADOK;
	sleep(3);
#ifdef	WITH_BACKUP_ROOTFS	
	//升级成功从第二分区正常启动
	fw_setenv(argc, argv);
#endif
	reboot(RB_AUTOBOOT);	
	return NULL;
#endif
}
void CTaskUpgrade::Process(void)
{
	return ;
#if 0
	int ret;
	MESSAGE recv_msg, send_msg;
	pthread_t OnlineUpgradeId;
	pthread_t OnlineDownloadId;
	RAM_PARAM *mpRamParam = get_ram_param();
#ifdef	WITH_BACKUP_ROOTFS
	int argc = 3;
	char *argv[3] = {(char *)"fw_setenv", 
		             (char *)"bootargs", 
		             (char *)"mem=36M console=ttyAMA0,115200 root=/dev/mtdblock4 rootfstype=squashfs mtdparts=hi_sfc:512K(boot),2560K(kernel),9728K(rootfs),512K(config),3072K(backup) init=/linuxrc"};
#endif
	while (mRunning)
	{
		memset(&recv_msg, 0, sizeof(MESSAGE));
		memset(&send_msg, 0, sizeof(MESSAGE));	
		if (RecvMsg(mMsgHandle, FW_UPGRADE_TYPE, &recv_msg) == SDK_OK)
		{
			switch (recv_msg.mCommand)
			{
				/* 网页本地升级 */
				case LOCAL_UPGRADE:
			    case LOCAL_UPGRADE_UBOOT:
			    case LOCAL_UPGRADE_KERNEL:
			    case LOCAL_UPGRADE_ROOTFS:        
			    case LOCAL_UPGRADE_WEB:  	
					KillTask();
					LocalUpgrade(recv_msg.mText);
					reboot(RB_AUTOBOOT);
					break;
				case ONLINE_UPGRADE_DOWNLOAD:
					mpRamParam->ota_param.update_statue=UPGRADE_STATUE_DOWNLOADING;
					CreateDetachedTask(&OnlineDownloadId,OnlineDownLoadProc, this);
					break;
				/* 在线升级 */			
			    case ONLINE_UPGRADE:
#ifdef	WITH_BACKUP_ROOTFS
#ifndef hi3518ev200
					//升级之前将系统启动参数修改为第四分区，防止升级失败设备起不来
					ret=fw_setenv(argc, argv);
					if(ret!=0)
					{
						LOG_INFO("=======fw_setenv is error====\n");
						break;
					}
					mpRamParam->ota_param.update_statue=UPGRADE_STATUE_UPGRADING;
					CreateDetachedTask(&OnlineUpgradeId,OnlineUpgradeProc,NULL);
#endif
#else
					LOG_INFO("nothing to do\n");
#endif
					break;
				default:
					break;
			}
		}
		else
		{
			usleep(1000000); 
		}
	}
#endif
}
//检测版本
void *CheckVersionProc(void *pArg)
{
return NULL;
#if 0
	CTaskUpgrade *pTaskUpgrade = (CTaskUpgrade *)pArg;
	FACTORY_PARAM *pFactoryParam = pTaskUpgrade->mpFactoryParam;
	MISC_PARAM *pMiscParam = pTaskUpgrade->mpMiscParam;
	RAM_PARAM *pRamParam = pTaskUpgrade->mpRamParam;	
	OTA_INFO OtaInfo;
	char *pString = NULL, key[64] = {0};
	pRamParam->ota_param.need_upgrade=0;
	pRamParam->ota_param.update_statue=UPGRADE_STATUE_FREE;
	while (pTaskUpgrade->mRunning)
	{
		char path[128] = {0};	
		dictionary	*Dictionary = NULL;			
		int retry = 10;

		while (retry)
		{
			strcpy(path, "/tmp/");
			if (GetRemoteFile(pFactoryParam->version_desc, path) == 0)	/* 输入存储目录,返回文件路径 */
			{
				LOG_INFO("=====connect update server is ok=====\n");
				pRamParam->ota_param.connect_server=true;
				break;
			}
			pRamParam->ota_param.connect_server=false;
			sleep(3);
		}
		Dictionary = iniparser_load(path);
		if (Dictionary == NULL)
		{
			LOG_ERROR("error at iniparser_load\n");
			return NULL;
		}			
		/* get lastest uboot version */
		sprintf(key, "%s:%s", KEY_VERSION, LASTEST_UBOOT_VERSION);
		if (NULL != (pString = iniparser_getstring(Dictionary, key, NULL)))
			strcpy(OtaInfo.lastest_uboot, pString);
		/* get lastest kernel version */
		sprintf(key, "%s:%s", KEY_VERSION, LASTEST_KERNEL_VERSION);
		if (NULL != (pString = iniparser_getstring(Dictionary, key, NULL)))
			strcpy(OtaInfo.lastest_kernel, pString);		
		/* get lastest rootfs version */		
		sprintf(key, "%s:%s", KEY_VERSION, LASTEST_ROOTFS_VERSION);
		if (NULL != (pString = iniparser_getstring(Dictionary, key, NULL)))
			strcpy(OtaInfo.lastest_rootfs, pString);		
		/* get lastest version description */		
		sprintf(key, "%s:%s", KEY_VERSION, VERSION_DESCRIPTION);		
		if (NULL != (pString = iniparser_getstring(Dictionary, key, NULL)))
			strcpy(OtaInfo.description, pString);	
		iniparser_freedict(Dictionary);
			
		LOG_INFO("lastest_version: %s_%s_%s\n", OtaInfo.lastest_uboot, OtaInfo.lastest_kernel, OtaInfo.lastest_rootfs);

		/* check uboot */
		if (CheckVersion(OtaInfo.lastest_uboot, pMiscParam->uboot_version))
		{
			pRamParam->ota_param.need_upgrade |= UBOOT_MASK;
			pRamParam->ota_param.isnew_version=true;
			strcpy(pRamParam->ota_param.lastest_version, OtaInfo.lastest_uboot);
			strcpy(pRamParam->ota_param.version_description, OtaInfo.description);
			LOG_INFO("find new uboot firmware: %s\n", OtaInfo.lastest_uboot);
		}
		/* check kernel */
		if (CheckVersion(OtaInfo.lastest_kernel, pMiscParam->kernel_version))
		{
			pRamParam->ota_param.need_upgrade |= KERNEL_MASK;
			pRamParam->ota_param.isnew_version=true;
			strcpy(pRamParam->ota_param.lastest_version, OtaInfo.lastest_kernel);
			strcpy(pRamParam->ota_param.version_description, OtaInfo.description);
			LOG_INFO("find new kernel firmware: %s\n", OtaInfo.lastest_kernel);			
		}
	
		/* check rootfs */
		if (CheckVersion(OtaInfo.lastest_rootfs, pMiscParam->soft_version))
		{
			pRamParam->ota_param.need_upgrade |= ROOTFS_MASK;
			pRamParam->ota_param.isnew_version=true;
			strcpy(pRamParam->ota_param.lastest_version, OtaInfo.lastest_rootfs);
			strcpy(pRamParam->ota_param.version_description, OtaInfo.description);
			LOG_INFO("find new rootfs firmware: %s\n", OtaInfo.lastest_rootfs);			
		}			
		sleep(ONLINE_FW_CHECK_PEROID);
	}
	return NULL;
#endif
}

int CTaskUpgrade::Init(void)
{
	mpFactoryParam = get_factory_param();
	mpMiscParam = get_misc_param();
	mpRamParam = get_ram_param();
	if (mpRamParam->UpgradeMsgHandle == -1)
	{
		if (CreateMsgHandle(&mpRamParam->UpgradeMsgHandle, MSG_UPGRADE_KEY) < 0)
		{
			LOG_ERROR("CreateMsgHandle fail\n");
			return SDK_ERROR;
		}
		LOG_INFO("CreateMsgHandle [%d]\n", mpRamParam->UpgradeMsgHandle);
	}
	mMsgHandle = mpRamParam->UpgradeMsgHandle;	
	/* 到服务器上检查最新版本 */
	CreateDetachedTask(&mCheckVersionId, CheckVersionProc, this);

	return SDK_OK;
}

void CTaskUpgrade::Uninit(void)
{

}

int upgrade_task_create(void)
{	
	return CTaskUpgrade::getInstance()->Create();
}

void upgrade_task_destory(void)
{
	return CTaskUpgrade::getInstance()->Destory();
}

