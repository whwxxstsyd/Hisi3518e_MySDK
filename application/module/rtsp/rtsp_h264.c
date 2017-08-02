#include <string.h>
#include <stdio.h>
#include "rtsp_h264.h"
#include "rtsp_global.h"
#include "socket.h"
#include "base64.h"
#include "rtsp_transfer.h"
#include "rtsp_rtp.h"
#include "debug.h"
#include "normal.h"

int rtsp_h264_send_nal(RTSP_SESSION_S* pSess, const char *szNalData, int nNalLen, 
				  int bLast, unsigned int nPts)
{
	RTP_HDR_S 			rtpHdr;
	RTP_OVER_TCP_HEADER rtpOverTcpHdr;
	FU_ADef 			stFU_A;
	char *				pSendBuf = (char *)pSess->stRtpSender.szBuff;
	
	int    nRtpOverTcpLen = sizeof(RTP_OVER_TCP_HEADER);
	int    nRtpHdrLen     = sizeof(RTP_HDR_S);
	int    nFuHdrLen      = sizeof(FU_ADef);
	int    nPlayLoadLen   = 0, nFuNum = 0,  nDataLen = 0;
	
	memset(&rtpHdr,        0, sizeof(RTP_HDR_S));
	memset(&rtpOverTcpHdr, 0, sizeof(RTP_OVER_TCP_HEADER));
	memset(&stFU_A,        0, sizeof(FU_ADef));

	rtpOverTcpHdr.u8Dollar     = '$';
	rtpOverTcpHdr.u8ChnId = 0;
	
	rtpHdr.version   = RTP_VERSION;
	rtpHdr.cc        = 0;
	rtpHdr.x         = 0;
	rtpHdr.p         = 0;

	rtpHdr.ts		 = htonl(nPts);
	rtpHdr.pt        = RTP_PT_H264;
	rtpHdr.ssrc      = htonl(pSess->stRtpSender.nVideoSsrc);

	if (pSess->eTransType != RTP_TRANSPORT_TCP)
	{
		nRtpOverTcpLen = 0;
	}
	
	nPlayLoadLen = RTP_MAX_PACKET_BUFF - nRtpOverTcpLen - nRtpHdrLen - nFuHdrLen;
	if (nNalLen + nRtpOverTcpLen + nRtpHdrLen <= RTP_MAX_PACKET_BUFF)
	{
		rtpHdr.seqno = htons(++pSess->stRtpSender.u16LastSndSeq[0]);
		rtpHdr.marker= 1;//bLast
		
		if (nRtpOverTcpLen != 0)
		{
			rtpOverTcpHdr.u16PacketSize = htons(nNalLen+nRtpHdrLen);
			memcpy(pSendBuf, &rtpOverTcpHdr, nRtpOverTcpLen);
		}
		memcpy(&pSendBuf[nRtpOverTcpLen], &rtpHdr, nRtpHdrLen);
		pSess->stRtpSender.nBuffLen = nRtpHdrLen+nRtpOverTcpLen;
		memcpy(&pSendBuf[pSess->stRtpSender.nBuffLen], szNalData, nNalLen);
		pSess->stRtpSender.nBuffLen += nNalLen;
		if (send_rtp_data(pSess, pSendBuf, pSess->stRtpSender.nBuffLen, 0) < 0)
			return -1;
	}
	else
	{
		stFU_A.stFUIndicator.F   = (szNalData[0] & 0x80) >> 7;
		stFU_A.stFUIndicator.NRI = (szNalData[0] & 0x60) >> 5;
		stFU_A.stFUIndicator.TYPE= 28;
		stFU_A.stFUHeader.Type   = szNalData[0] & 0x1f;

		szNalData++;   //the first packet skip the first byte
		nNalLen -= 1;		
		while (nNalLen > 0)
		{
			pSess->stRtpSender.nBuffLen = 0;	
			rtpHdr.seqno = htons(++pSess->stRtpSender.u16LastSndSeq[0]);

			stFU_A.stFUHeader.S = (nFuNum == 0) ? 1 : 0;			
			if (nNalLen + nRtpOverTcpLen + nRtpHdrLen + nFuHdrLen <= RTP_MAX_PACKET_BUFF)
			{
				rtpHdr.marker       = 1; ////bLast
				stFU_A.stFUHeader.E = 1;
				nDataLen            = nNalLen;
			}	
			else
			{
				stFU_A.stFUHeader.E = 0;
				rtpHdr.marker       = 0;
				nDataLen            = nPlayLoadLen;
			}

			if (nRtpOverTcpLen != 0)
			{
				rtpOverTcpHdr.u16PacketSize = htons(nDataLen+nRtpHdrLen+nFuHdrLen);
				memcpy(pSendBuf, &rtpOverTcpHdr, nRtpOverTcpLen);
			}
			
			memcpy(&pSendBuf[nRtpOverTcpLen], &rtpHdr, nRtpHdrLen);
			pSess->stRtpSender.nBuffLen = nRtpHdrLen+nRtpOverTcpLen;

			memcpy(&pSendBuf[pSess->stRtpSender.nBuffLen], &stFU_A, nFuHdrLen);
			pSess->stRtpSender.nBuffLen += nFuHdrLen;
			
			memcpy(&pSendBuf[pSess->stRtpSender.nBuffLen], szNalData, nDataLen);
			pSess->stRtpSender.nBuffLen += nDataLen;
			nNalLen  -= nDataLen;
			szNalData+= nDataLen;
			nFuNum++;
			if (send_rtp_data(pSess, pSendBuf, pSess->stRtpSender.nBuffLen, 0) < 0)
				return -1;	
		}/*end while*/
			
	}/*end else*/	
	return 0;
}

int rtsp_h264_find_nal(const char *pData, int nDataLen)
{
	int pos = 0;

	while(pos < nDataLen - 4)
	{
		if(pData[pos]   == 0x00 && pData[pos+1] == 0x00 
		&& pData[pos+2] == 0x00 && pData[pos+3] == 0x01)
		{
			return pos;
		}
		pos++;
	}
	
	return -1;
}

/* not use */
int rtsp_h264_get_media_info(char *pBuffData, int nFrameLen, char *proLevelId, char *szSpsPps)
{
	unsigned char szSps[128] ={0}, szPps[128] ={0};
	unsigned char szBase64sps[256] ={0}, szBase64pps[256] ={0};
	int nPpsLen = 128, nSpsLen = 0, nPos = 0;
	int nNalUnitType = 0, nSps = 0, nPps = 0, nCurPos = 0;

	while(nCurPos < nFrameLen)
	{
		if((nPos = rtsp_h264_find_nal(&pBuffData[nCurPos], nFrameLen - nCurPos)) < 0)
		{
			return -1;
		}
		
		if(nNalUnitType == 7)
		{
			nSps++;
			if (nPos > 0)
				memcpy(szSps, &pBuffData[nCurPos], nPos);
			nSpsLen = nPos;
		}
		
		if(nNalUnitType == 8)
		{
			nPps++;
			if (nPos > 0)
				memcpy(szPps, &pBuffData[nCurPos], nPos);
			nPpsLen = nPos;
		}
		
		if(nSps > 0 && nPps > 0)
		{
			break;
		}
		
		nCurPos     += (nPos + 4);
		nNalUnitType = pBuffData[nCurPos] & 0x1f;
	}

	if(nSps == 0 || nPps == 0) 
	{		
		return -1; 
	}

	sprintf(proLevelId,"%02x%02x%02x", szSps[1], szSps[2], szSps[3]);
	to64frombits((unsigned char*)szBase64sps, szSps, nSpsLen);
	to64frombits((unsigned char*)szBase64pps, szPps,  nPpsLen);
//    LOG_DEBUG("sps len:%d,  pps len:%d\n", nSpsLen, nPpsLen);    
	sprintf(szSpsPps, "%s,%s", szBase64sps, szBase64pps);

	return 0;
}

int rtsp_h264_video_send(RTSP_SESSION_S* pSess, const char *szBuff, 
                            	int nFrameLen, unsigned long u32TimeStamp)
{
	char *szData = (char *)(szBuff + sizeof(FRAME_HEAD));	/* 去掉(00 00 00 01) 和 帧头 */
	int len = (nFrameLen - sizeof(FRAME_HEAD));
	int pos = 0;
	int nal_type = 0;
	unsigned int pts = u32TimeStamp;

	//fwrite(szData,len,1,pFile);
	while (1)
	{
		pos = rtsp_h264_find_nal(szData, len);

		//当pos小于0时候表示没有找到
		if (pos < 0)
		{
			if (rtsp_h264_send_nal(pSess, szData, len, 1, pts) < 0)
				return -1;
		}
		else
		{
			if (rtsp_h264_send_nal(pSess, szData, pos, 1, pts) < 0)
				return -1;
		}

		if (pos < 0)
			break;
		szData += (pos + 4);
		len    -= (pos + 4);		   
	}
	pSess->stRtpSender.u32LastSndTs[0] = pts;
	
	return 0;
}


