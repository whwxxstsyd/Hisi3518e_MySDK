#include "hi_codec.h"
//#include "hi_gpio.h"
#include "common.h"
#include "hi_type.h"
//#include "hi_osd.h"
#include "task.h"
#include "param.h"
#include "param_base.h"

//#include "mpi_vpss.h"

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>

static int g_s32ChnNum = 0;									/* 编码通道数目 */
static VIDEO_NORM_E gs_enNorm = VIDEO_ENCODING_MODE_NTSC;	/* PAL/NTSC */
static PIC_SIZE_E enVichSize[3] = {PIC_HD720, PIC_VGA, PIC_QVGA};
static PIC_SIZE_E enVencSize[3] = {PIC_HD720, PIC_VGA, PIC_QVGA};
static int g_isDevInit = 0;
static int g_isRun = 0;
pthread_t ShowOsdId, GetStreamId, GetPicId, Isp3DenoiseId;


void hi_set_venc_resolution(PIC_SIZE_E *pSizeArray, int num)
{
	int i;
	
	for (i = 0; i < num; i++)
		enVencSize[i] = pSizeArray[i];
}


void hi_set_stream_cb(PUT_VIDEO_FRAME_CB handle, void *user_data)
{
	SAMPLE_COMM_VENC_SetStreamCB(handle, user_data);
}

/******************************************************************************
* function :  H.264@720p@30fps+H.264@VGA@30fps+H.264@QVGA@30fps
******************************************************************************/
HI_S32 SAMPLE_VENC_720P_CLASSIC(HI_VOID)
	{
		PAYLOAD_TYPE_E enPayLoad[3]= {PT_H264, PT_H264, PT_H264};
		PIC_SIZE_E enSize[3] = {PIC_HD720, PIC_CIF, PIC_QVGA};
	
		VB_CONF_S stVbConf;
		SAMPLE_VI_CONFIG_S stViConfig;
		
		VPSS_GRP VpssGrp;
		VPSS_CHN VpssChn;
		VPSS_GRP_ATTR_S stVpssGrpAttr;
		VPSS_CHN_ATTR_S stVpssChnAttr;
		VPSS_CHN_MODE_S stVpssChnMode;
		VPSS_EXT_CHN_ATTR_S stVpssExtChnAttr;
		
		VENC_GRP VencGrp;
		VENC_CHN VencChn;
		SAMPLE_RC_E enRcMode= SAMPLE_RC_CBR;
	
		HI_S32 s32ChnNum = 1;
		
		HI_S32 s32Ret = HI_SUCCESS;
		HI_U32 u32BlkSize;
		SIZE_S stSize;

		/******************************************
		 step  1: init sys variable 
		******************************************/
		memset(&stVbConf,0,sizeof(VB_CONF_S));
	
		switch(SENSOR_TYPE)
		{
			case SONY_IMX122_DC_1080P_30FPS:
			case APTINA_MT9P006_DC_1080P_30FPS:
				enSize[0] = PIC_HD1080;
				break;
	
			default:
				break;
		}

	
		stVbConf.u32MaxPoolCnt = 128; //128
	
		/*video buffer*/   
		u32BlkSize = SAMPLE_COMM_SYS_CalcPicVbBlkSize(gs_enNorm,enSize[0], SAMPLE_PIXEL_FORMAT, SAMPLE_SYS_ALIGN_WIDTH);
		stVbConf.astCommPool[0].u32BlkSize = u32BlkSize;
		stVbConf.astCommPool[0].u32BlkCnt =8;//10
	
	   if(s32ChnNum>=2)
	   {
			u32BlkSize = SAMPLE_COMM_SYS_CalcPicVbBlkSize(gs_enNorm,enSize[1], SAMPLE_PIXEL_FORMAT, SAMPLE_SYS_ALIGN_WIDTH);
			stVbConf.astCommPool[1].u32BlkSize = u32BlkSize;
			stVbConf.astCommPool[1].u32BlkCnt =6;//6 8
	   }
	
	
	
#if 0
		if(s32ChnNum>=3)
		{
			u32BlkSize = SAMPLE_COMM_SYS_CalcPicVbBlkSize(gs_enNorm, enSize[2], SAMPLE_PIXEL_FORMAT, SAMPLE_SYS_ALIGN_WIDTH);
			stVbConf.astCommPool[2].u32BlkSize = u32BlkSize;
			stVbConf.astCommPool[2].u32BlkCnt = 8;
			 /* hist buf*/
		}
	
#endif
	
		if(s32ChnNum==1)
		{
			stVbConf.astCommPool[1].u32BlkSize = (196*4);
			stVbConf.astCommPool[1].u32BlkCnt = 8;
		}
		else if(s32ChnNum==2)
		{
			stVbConf.astCommPool[2].u32BlkSize = (196*4);
			stVbConf.astCommPool[2].u32BlkCnt =8; //8
	
		}
	
#if 0
		else
		{
			stVbConf.astCommPool[3].u32BlkSize = (196*4);
			stVbConf.astCommPool[3].u32BlkCnt = 10;//6
		}
	
#endif
		 //ISP_SetAEattr();
		 //ISP_Setsaturation();
		/******************************************
		 step 2: mpp system init. 
		******************************************/
		s32Ret = SAMPLE_COMM_SYS_Init(&stVbConf);
		if (HI_SUCCESS != s32Ret)
		{
			SAMPLE_PRT("system init failed with %d!\n", s32Ret);
			goto END_VENC_720P_CLASSIC_0;
		}
	
		/******************************************
		 step 3: start vi dev & chn to capture
		******************************************/
		stViConfig.enViMode   = SENSOR_TYPE;
		stViConfig.enRotate   = ROTATE_NONE;
		stViConfig.enNorm	  = VIDEO_ENCODING_MODE_AUTO;
		stViConfig.enViChnSet = VI_CHN_SET_NORMAL;
		s32Ret = SAMPLE_COMM_VI_StartVi(&stViConfig);
		if (HI_SUCCESS != s32Ret)
		{
			SAMPLE_PRT("start vi failed!\n");
			goto END_VENC_720P_CLASSIC_1;
		}
		
		/******************************************
		 step 4: start vpss and vi bind vpss
		******************************************/
		s32Ret = SAMPLE_COMM_SYS_GetPicSize(gs_enNorm, enSize[0], &stSize);
		if (HI_SUCCESS != s32Ret)
		{
			SAMPLE_PRT("SAMPLE_COMM_SYS_GetPicSize failed!\n");
			goto END_VENC_720P_CLASSIC_1;
		}
	
		VpssGrp = 0;
		stVpssGrpAttr.u32MaxW = stSize.u32Width;
		stVpssGrpAttr.u32MaxH = stSize.u32Height;
		stVpssGrpAttr.bDrEn = HI_FALSE;
		stVpssGrpAttr.bDbEn = HI_FALSE;
		stVpssGrpAttr.bIeEn = HI_TRUE;
		stVpssGrpAttr.bNrEn = HI_TRUE;
		stVpssGrpAttr.bHistEn = HI_TRUE;
		stVpssGrpAttr.enDieMode = VPSS_DIE_MODE_AUTO;
		stVpssGrpAttr.enPixFmt = SAMPLE_PIXEL_FORMAT;
		s32Ret = SAMPLE_COMM_VPSS_StartGroup(VpssGrp, &stVpssGrpAttr);
		if (HI_SUCCESS != s32Ret)
		{
			SAMPLE_PRT("Start Vpss failed!\n");
			goto END_VENC_720P_CLASSIC_2;
		}
	
		s32Ret = SAMPLE_COMM_VI_BindVpss(stViConfig.enViMode);
		if (HI_SUCCESS != s32Ret)
		{
			SAMPLE_PRT("Vi bind Vpss failed!\n");
			goto END_VENC_720P_CLASSIC_3;
		}
	/////////
		VpssChn = 0;
		memset(&stVpssChnAttr, 0, sizeof(stVpssChnAttr));
		stVpssChnAttr.bFrameEn = HI_FALSE;
		stVpssChnAttr.bSpEn    = HI_TRUE;	 
		s32Ret = SAMPLE_COMM_VPSS_EnableChn(VpssGrp, VpssChn, &stVpssChnAttr, HI_NULL, HI_NULL);
		if (HI_SUCCESS != s32Ret)
		{
			SAMPLE_PRT("Enable vpss chn failed!\n");
			goto END_VENC_720P_CLASSIC_4;
		}
	/////////
		if(s32ChnNum>=2)
		{
			VpssChn = 1;
			stVpssChnMode.enChnMode 	= VPSS_CHN_MODE_USER;
			stVpssChnMode.bDouble		= HI_FALSE;
			stVpssChnMode.enPixelFormat = SAMPLE_PIXEL_FORMAT;
			//stVpssChnMode.u32Width		= 320;
			//stVpssChnMode.u32Height	= 240;
			stVpssChnMode.u32Width		= 352;
			stVpssChnMode.u32Height 	= 288;
			s32Ret = SAMPLE_COMM_VPSS_EnableChn(VpssGrp, VpssChn, &stVpssChnAttr, &stVpssChnMode, HI_NULL);
			if (HI_SUCCESS != s32Ret)
			{
				SAMPLE_PRT("Enable vpss chn failed!\n");
				goto END_VENC_720P_CLASSIC_4;
			}
		}
	
	////////////
		if(s32ChnNum>=3)
		{
			VpssChn = 3;
			stVpssExtChnAttr.s32BindChn = 1;
			stVpssExtChnAttr.s32SrcFrameRate = 30;
			stVpssExtChnAttr.s32DstFrameRate = 25;
			stVpssExtChnAttr.enPixelFormat	 = SAMPLE_PIXEL_FORMAT;
			stVpssExtChnAttr.u32Width		 = 320;
			stVpssExtChnAttr.u32Height		 = 240;
			s32Ret = SAMPLE_COMM_VPSS_EnableChn(VpssGrp, VpssChn, HI_NULL, HI_NULL, &stVpssExtChnAttr);
			if (HI_SUCCESS != s32Ret)
			{
				SAMPLE_PRT("Enable vpss chn failed!\n");
				goto END_VENC_720P_CLASSIC_4;
			}
		}
		//ISP_SetAEattr();
		//ISP_Setsaturation();
		/******************************************
		 step 5: start stream venc
		******************************************/
		/*** HD720P **/
		VpssGrp = 0;
		VpssChn = 0;
		VencGrp = 0;
		VencChn = 0;
		s32Ret = SAMPLE_COMM_VENC_Start(VencGrp, VencChn, enPayLoad[0],gs_enNorm, enSize[0], enRcMode);
		if (HI_SUCCESS != s32Ret)
		{
			SAMPLE_PRT("Start Venc failed!\n");
			goto END_VENC_720P_CLASSIC_5;
		}
	
		s32Ret = SAMPLE_COMM_VENC_BindVpss(VencGrp, VpssGrp, VpssChn);
		if (HI_SUCCESS != s32Ret)
		{
			SAMPLE_PRT("Start Venc failed!\n");
			goto END_VENC_720P_CLASSIC_5;
		}
	
		
		if(s32ChnNum>=2)
		{
			/*** vga **/
			VpssChn = 1;
			VencGrp = 1;
			VencChn = 1;
			s32Ret = SAMPLE_COMM_VENC_Start(VencGrp, VencChn, enPayLoad[1], gs_enNorm, enSize[1], enRcMode);
			if (HI_SUCCESS != s32Ret)
			{
				SAMPLE_PRT("Start Venc failed!\n");
				goto END_VENC_720P_CLASSIC_5;
			}
			
			s32Ret = SAMPLE_COMM_VENC_BindVpss(VencChn, VpssGrp, VpssChn);
			if (HI_SUCCESS != s32Ret)
			{
				SAMPLE_PRT("Start Venc failed!\n");
				goto END_VENC_720P_CLASSIC_5;
			}
	
		}
	
	
	
		if(s32ChnNum>=3)
		{
			/*** qvga **/
			VpssChn = 3;
			VencGrp = 2;
			VencChn = 2;
			s32Ret = SAMPLE_COMM_VENC_Start(VencGrp, VencChn, enPayLoad[2], gs_enNorm, enSize[2], enRcMode);
			if (HI_SUCCESS != s32Ret)
			{
				SAMPLE_PRT("Start Venc failed!\n");
				goto END_VENC_720P_CLASSIC_5;
			}
	
			s32Ret = SAMPLE_COMM_VENC_BindVpss(VencChn, VpssGrp, VpssChn);
			if (HI_SUCCESS != s32Ret)
			{
				SAMPLE_PRT("Start Venc failed!\n");
				goto END_VENC_720P_CLASSIC_5;
			}
	
		}
	
		//ISP_SetAEattr();
		//ISP_Setsaturation();
		/******************************************
		 step 6: stream venc process -- get stream, then save it to file. 
		******************************************/

			SAMPLE_PRT("Ready into SAMPLE_COMM_VENC_GetStreamProc!\n");
			
			g_isRun = 1;
			s32Ret = CreateDetachedTask(&GetStreamId, SAMPLE_COMM_VENC_GetStreamProc, &g_isRun);
			if (HI_SUCCESS != s32Ret)
			{
				SAMPLE_PRT("Start Venc failed!\n");
				goto END_VENC_720P_CLASSIC_5;
			}
			g_isDevInit = 1;
			
			return SDK_OK;
		
	END_VENC_720P_CLASSIC_5:
		VpssGrp = 0;
		
		VpssChn = 0;
		VencGrp = 0;   
		VencChn = 0;
		SAMPLE_COMM_VENC_UnBindVpss(VencGrp, VpssGrp, VpssChn);
		SAMPLE_COMM_VENC_Stop(VencGrp,VencChn);
	
		VpssChn = 1;
		VencGrp = 1;   
		VencChn = 1;
		SAMPLE_COMM_VENC_UnBindVpss(VencGrp, VpssGrp, VpssChn);
		SAMPLE_COMM_VENC_Stop(VencGrp,VencChn);
	/*
		VpssChn = 3;
		VencGrp = 2;   
		VencChn = 2;
		SAMPLE_COMM_VENC_UnBindVpss(VencGrp, VpssGrp, VpssChn);
		SAMPLE_COMM_VENC_Stop(VencGrp,VencChn);
	*/
		SAMPLE_COMM_VI_UnBindVpss(stViConfig.enViMode);
	END_VENC_720P_CLASSIC_4:	//vpss stop
		VpssGrp = 0;
		VpssChn = 3;
		SAMPLE_COMM_VPSS_DisableChn(VpssGrp, VpssChn);
		VpssChn = 0;
		SAMPLE_COMM_VPSS_DisableChn(VpssGrp, VpssChn);
		VpssChn = 1;
		SAMPLE_COMM_VPSS_DisableChn(VpssGrp, VpssChn);
	END_VENC_720P_CLASSIC_3:	//vpss stop 	  
		SAMPLE_COMM_VI_UnBindVpss(stViConfig.enViMode);
	END_VENC_720P_CLASSIC_2:	//vpss stop   
		SAMPLE_COMM_VPSS_StopGroup(VpssGrp);
	END_VENC_720P_CLASSIC_1:	//vi stop
		SAMPLE_COMM_VI_StopVi(&stViConfig);
	END_VENC_720P_CLASSIC_0:	//system exit
		SAMPLE_COMM_SYS_Exit();
		
		return NULL;	
	}


	


