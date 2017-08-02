#include "string_parser.h"
#include "common.h"
#include "debug.h"
#include "normal.h"

#include <stdio.h>
#include <stdlib.h>

/* ------------------ class CStringParser 提高代码的复用性 ------------------ */
int CStringParser::init_parser_tab(const char *key_word, int index)	/* 字符串转整形数据 */
{
	PARSER_INDEX_INFO *pIndexRecord = NULL;
	PARSER_STRING_INFO *pParserRecord = NULL;	
	
	pIndexRecord = (PARSER_INDEX_INFO *)mIndexHash.AllocRecord(key_word);
	if (NULL == pIndexRecord)
		return -1;
	pIndexRecord->number = index;

	pParserRecord = (PARSER_STRING_INFO *)mStringHash.AllocRecord(index);
	if (NULL == pParserRecord)
		return -1;
	
	pParserRecord->string = (char *)key_word;	

	return SDK_OK;
}

#define		INIT_UNIT(index, string)	init_parser_tab(string, index)


/* (保持兼容设计)外部接口,网页设计需要 */
CStringParser::CStringParser(void)
{
	mInit = 0;
}

CStringParser::~CStringParser()
{

}

int CStringParser::Init(void)
{
	if (mInit)
		return 0;
	
	mIndexHash.Init(HASH_MAP_SIZE, sizeof(PARSER_INDEX_INFO));
	mStringHash.Init(HASH_MAP_SIZE, sizeof(PARSER_INDEX_INFO));	

	INIT_UNIT(MAIN_STREAM_CH, 	"main_stream");
	INIT_UNIT(SUB1_STREAM_CH, 	"sub1_stream");
	INIT_UNIT(SUB2_STREAM_CH, 	"sub2_stream");
	INIT_UNIT(SUB3_STREAM_CH, 	"sub3_stream");	
	INIT_UNIT(AUDIO_TYPE_G711, 	"g711");
	INIT_UNIT(AUDIO_TYPE_G726, 	"g726");
	INIT_UNIT(AUDIO_TYPE_ADPCM, "adpcm");
	INIT_UNIT(AUDIO_TYPE_PCM, 	"pcm");
	INIT_UNIT(DDNS_NO_IP, 		"no-ip");
	INIT_UNIT(DDNS_DYNDNS, 		"dyndns");
	INIT_UNIT(DDNS_3322, 		"3322");
	INIT_UNIT(DDNS_9299, 		"9299");
	INIT_UNIT(DDNS_TWS, 		"tws_ddns");
	INIT_UNIT(DDNS_EASYN, 		"easyn_ddns");	
	INIT_UNIT(SPEED_LOW, 		"low");
	INIT_UNIT(SPEED_NORMAL, 	"normal");
	INIT_UNIT(SPEED_HIGH, 		"high");
	INIT_UNIT(USER_ADMIN, 		"admin");
	INIT_UNIT(USER_OPT, 		"opt");
	INIT_UNIT(USER_GUEST, 		"guest");
	INIT_UNIT(AUTH_WEP, 		"wep");
	INIT_UNIT(AUTH_WPA, 		"wpa");
	INIT_UNIT(AUTH_WPA2, 		"wpa2");
	INIT_UNIT(AUTH_OPEN, 		"open");	
	INIT_UNIT(ENC_NONE, 		"none");
	INIT_UNIT(ENC_SHARE, 		"share");
	INIT_UNIT(ENC_AES, 			"aes");
	INIT_UNIT(ENC_TKIP, 		"tkip");	
	INIT_UNIT(WIFI_DISCONNECT, 	"disconnect");	
	INIT_UNIT(WIFI_CONNECT, 	"connected");			
	INIT_UNIT(ON, 				"on");
	INIT_UNIT(OFF, 				"off");	
	INIT_UNIT(GMT12, 			"GMT-12");
	INIT_UNIT(GMT_12, 			"GMT12");	
	INIT_UNIT(GMT11, 			"GMT-11");
	INIT_UNIT(GMT_11, 			"GMT11");
	INIT_UNIT(GMT10, 			"GMT-10");
	INIT_UNIT(GMT_10, 			"GMT10");
	INIT_UNIT(GMT9, 			"GMT-9");
	INIT_UNIT(GMT_9, 			"GMT9");
	INIT_UNIT(GMT8, 			"GMT-8");
	INIT_UNIT(GMT_8, 			"GMT8");
	INIT_UNIT(GMT7, 			"GMT-7");
	INIT_UNIT(GMT_7, 			"GMT7");	
	INIT_UNIT(GMT6, 			"GMT-6");
	INIT_UNIT(GMT_6, 			"GMT6");
	INIT_UNIT(GMT5, 			"GMT-5");
	INIT_UNIT(GMT_5, 			"GMT5");
	INIT_UNIT(GMT4, 			"GMT-4");
	INIT_UNIT(GMT_4, 			"GMT4");
	INIT_UNIT(GMT3, 			"GMT-3");
	INIT_UNIT(GMT_3, 			"GMT3");	
	INIT_UNIT(GMT2, 			"GMT-2");
	INIT_UNIT(GMT_2, 			"GMT2");
	INIT_UNIT(GMT1, 			"GMT-1");
	INIT_UNIT(GMT_1, 			"GMT1");	
	INIT_UNIT(GMT0, 			"GMT0");		
	INIT_UNIT(FLIP_NORMAL, 		"flip_normal");
	INIT_UNIT(FLIP_H, 			"flip_h");
	INIT_UNIT(FLIP_V, 			"flip_v");
	INIT_UNIT(FLIP_HV, 			"flip_hv");
	INIT_UNIT(EMAIL_TLS, 		"tls");	
	INIT_UNIT(EMAIL_SSL, 		"ssl");
	INIT_UNIT(NORMAL_SYNC, 		"nomal_sync");
	INIT_UNIT(SYNC_TO_PC, 		"pc_sync");
	INIT_UNIT(NTP_SYNC, 		"ntp_sync"); 
	INIT_UNIT(NTP_SYNC_OK, 		"ntp_sync_ok");	
	INIT_UNIT(NTP_SYNC_FAIL, 	"ntp_sync_fail");			
	INIT_UNIT(NET_TYPE_NULL, 	"use_unkown");
	INIT_UNIT(USE_ETH, 			"use_eth");
	INIT_UNIT(USE_WLAN, 		"use_wlan");
	INIT_UNIT(DHCP_IP, 			"dhcp_ip");
	INIT_UNIT(DHCP_ALL, 		"dhcp_all");
	INIT_UNIT(DHCP_OFF, 		"dhcp_off");
	INIT_UNIT(TEST_OK, 			"test_ok");
	INIT_UNIT(TEST_FAIL, 		"test_fail");	
	INIT_UNIT(F50HZ, 			"50Hz");
	INIT_UNIT(F60HZ, 			"60Hz");	
	INIT_UNIT(TVS_DDNS_OFF, 	"tvs_ddns_off");	
	INIT_UNIT(THR_DDNS_OFF, 	"thr_ddns_off");	
	INIT_UNIT(UPDATE_OK, 		"update_ok");	
	INIT_UNIT(DDNS_UPDATE_FAIL, "update_fail");		
	INIT_UNIT(TVS_911_ERROR, 	"911_error");	
	INIT_UNIT(TVS_911CC_ILLEGAL,"911cc_error");
	INIT_UNIT(TVS_901_AUTH_FAIL,"901_auth_fail");	
	INIT_UNIT(TVS_DELETE_OK, 	"delele_ok");	
	INIT_UNIT(EASYN_SERVER_NOAUTH, 	"ERRIDS_SERVER_NOAUTH");		
	INIT_UNIT(EASYN_SERVER_NOID, 	"ERRIDS_SERVER_NOID");	
	INIT_UNIT(EASYN_SERVER_OVER, 	"ERRIDS_SERVER_OVER");	
	INIT_UNIT(EASYN_SERVER_ERR_IDDISABLE, 	"ERRIDS_SERVER_ERR_IDDISABLE");	
	INIT_UNIT(EASYN_SERVER_ERR_PARAM, 	"ERRIDS_SERVER_ERR_PARAM");		
	INIT_UNIT(UPNP_FAIL, 		"upnp_fail");		
	INIT_UNIT(UPNP_OFF, 		"upnp_off");
	INIT_UNIT(QVGA, 			"QVGA");			
	INIT_UNIT(VGA, 				"VGA");	
	INIT_UNIT(P720, 			"720P");		
	INIT_UNIT(P1080, 			"1080P");	
	INIT_UNIT(MD_HIGHTEST, 		"md_highest");	
	INIT_UNIT(MD_HIGHT, 		"md_high");	
	INIT_UNIT(MD_NORMAL, 		"md_normal");	
	INIT_UNIT(MD_LOW, 			"md_low");	
	INIT_UNIT(PASV_MODE, 		"pasv_mode");	
	INIT_UNIT(PORT_MODE, 		"port_mode");
	INIT_UNIT(TUTK_P2P, 		"tutk_p2p");
	INIT_UNIT(ILNK_P2P, 		"ilnk_p2p");
	INIT_UNIT(DANALE_P2P, 		"danale_p2p");	
	INIT_UNIT(IRLED_AUTO, 			"ir_auto");	
	INIT_UNIT(IRLED_ON, 			"ir_on");
	INIT_UNIT(IRLED_OFF, 			"ir_off");	
	INIT_UNIT(LED_CTL_AUTO,		"led_ctl_auto");
	INIT_UNIT(LED_CTL_MANUL,	"led_ctl_manul");

	mInit = 1;

	return 0;
}

void CStringParser::Uninit(void)
{
	if (!mInit)
		return;
	
	mIndexHash.Uninit();
	mStringHash.Uninit();	

	mInit = 0;
}

int CStringParser::get_index(const char *key_word)
{
	PARSER_INDEX_INFO *pParserRecord = NULL;
	
	pParserRecord = (PARSER_INDEX_INFO *)mIndexHash.GetRecord(key_word);
	if (NULL == pParserRecord)
	{
		LOG_WARN("bad key_word[%s]\n", key_word);	
		return BAD_KEY_WORD;	
	}
		
	return pParserRecord->number;
}

char *CStringParser::get_string(int index)
{
	PARSER_STRING_INFO *pStringRecord = NULL;
	
	pStringRecord = (PARSER_STRING_INFO *)mStringHash.GetRecord(index);
	if (NULL == pStringRecord)
	{
		LOG_WARN("bad index[%d]\n", index);		/* 该索引还没定义对应的字符串 */
		return NULL;	
	}
		
	return pStringRecord->string;
}

int string_parser_init(void)
{
	return CStringParser::getInstance().Init();
}

void string_parser_uninit(void)
{
	return CStringParser::getInstance().Uninit();
}

int Str2Index(const char *string)
{
	return CStringParser::getInstance().get_index(string);
}

char *Index2Str(int index)
{
	return CStringParser::getInstance().get_string(index);
}

/* 如果参数不匹配不设置，确保数据的稳定性 */
void set_int_safe(int *var, char *string)
{
	int index;

	if ((index = Str2Index(string)) < 0)
		LOG_ERROR("the value is bad\n");
	else
		*var = index;
}

void set_int_safe1(int *var, int min, int max, int value)		/* 保证数据的绝对稳定性 */
{
	if ((value >= min) && (value <= max))
		*var = value;
	else
		LOG_ERROR("the value is bad\n");
}

