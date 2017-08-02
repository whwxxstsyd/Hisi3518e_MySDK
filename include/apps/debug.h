#ifndef		__DEBUG_H__
#define		__DEBUG_H__

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#ifdef	__cplusplus
extern "C" {
#endif

#ifndef		DEBUG_ON
#define		DEBUG_ON        (1)
#endif

#ifndef		TARGET_LEVEL
#define		TARGET_LEVEL	LEVEL_DEBUG
#endif

enum DEBUG_LEVEL
{
    LEVEL_NONE,
	LEVEL_ERROR,
	LEVEL_WARN,
	LEVEL_INFO,
	LEVEL_DEBUG,
	LEVEL_MAX,
};

static char debug_str[LEVEL_MAX][20] =
{
	"error",
	"warn",
	"info",
	"debug",
};

static char *debug_strdup(const char *str)
{
	char *newstr = NULL;

	if (str) 
	{
		newstr = (char *)malloc(strlen(str) + 1);
		if (newstr) 
		{
			memset(newstr, 0, strlen(str) + 1);
			strcpy(newstr, str);
		}
	}

	return newstr;
}

static char *get_filename(const char *dir)
{
	char *pcStr, *pcRet;

#ifdef WIN32
	pcStr = (char *)strrchr(dir, '\\') + 1;
#else
	pcStr = strrchr(dir, '/') + 1;
#endif	
    if (pcStr)
	    pcRet = debug_strdup(pcStr);
    else
        pcRet = NULL;

	return pcRet;
}

#ifdef WIN32

#if DEBUG_ON										
#define		LOG_OUT(level, ...)											\
				do{														\
					if (level <= TARGET_LEVEL)							\
					{													\
						char *pcStr = get_filename((char *)__FILE__);		\
																		\
						printf("[%s-%s-%d]\t", debug_str[level], pcStr, __LINE__);		\
						printf(__VA_ARGS__);							\
						free(pcStr);									\
					}													\
				}while(0)
#else
	#define		LOG_OUT(level, ...)
#endif

#define zprint_r(fmt, args...) printf ("\e[31m#zprint#[%s][%d]"fmt"\e[0m\n", __func__, __LINE__, ##args);
#define zprint_g(fmt, args...) printf ("\e[42m#zprint#[%s][%d]"fmt"\e[0m\n", __func__, __LINE__, ##args);
#define zprint_y(fmt, args...) printf ("\e[33m#zprint#[%s][%d]"fmt"\e[0m\n", __func__, __LINE__, ##args);
#define zprint_b(fmt, args...) printf ("\e[44m#zprint#[%s][%d]"fmt"\e[0m\n", __func__, __LINE__, ##args);
#define zprint_G(fmt, args...) printf ("\e[36m#zprint#[%s][%d]"fmt"\e[0m\n", __func__, __LINE__, ##args);

#define	LOG_DEBUG(...)		LOG_OUT(LEVEL_DEBUG, __VA_ARGS__)
#define	LOG_INFO(...)		LOG_OUT(LEVEL_INFO, __VA_ARGS__)
#define	LOG_WARN(...)		LOG_OUT(LEVEL_WARN, __VA_ARGS__)
#define	LOG_ERROR(...)		LOG_OUT(LEVEL_ERROR, __VA_ARGS__)

#else	/* end WIN32 */

#if DEBUG_ON											
#define		LOG_OUT(level, arg...)										\
				do{														\
					if (level <= TARGET_LEVEL)							\
					{													\
						char *pcStr = get_filename((char *)__FILE__);	\
																		\
						printf("[%s-%s-%d]\t", debug_str[level], pcStr, __LINE__);		\
						printf(arg);									\
						free(pcStr);									\
					}													\
				}while(0)
#else
	#define		LOG_OUT(level, ...)
#endif

#define zprint_r(fmt, args...) printf ("\e[31m#zprint#[%s][%d]"fmt"\e[0m\n", __func__, __LINE__, ##args);
#define zprint_g(fmt, args...) printf ("\e[42m#zprint#[%s][%d]"fmt"\e[0m\n", __func__, __LINE__, ##args);
#define zprint_y(fmt, args...) printf ("\e[33m#zprint#[%s][%d]"fmt"\e[0m\n", __func__, __LINE__, ##args);
#define zprint_b(fmt, args...) printf ("\e[44m#zprint#[%s][%d]"fmt"\e[0m\n", __func__, __LINE__, ##args);
#define zprint_G(fmt, args...) printf ("\e[36m#zprint#[%s][%d]"fmt"\e[0m\n", __func__, __LINE__, ##args);

#define	LOG_DEBUG(arg...)		LOG_OUT(LEVEL_DEBUG, ##arg)
#define	LOG_INFO(arg...)		LOG_OUT(LEVEL_INFO, ##arg)
#define	LOG_WARN(arg...)		LOG_OUT(LEVEL_WARN, ##arg)
#define	LOG_ERROR(arg...)		LOG_OUT(LEVEL_ERROR, ##arg)
#define	LOG_STATUS(str)			LOG_INFO("status:\t%s\n", str);
#endif	/* end LINUX */

#ifdef	__cplusplus
}
#endif


#endif
