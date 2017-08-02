#ifndef __FONT_H__
#define __FONT_H__

#include "typedef.h"

#define PIXELS_PER_BYTE					8
#define	FONT_INDEX_TAB_SIZE				4    //单个字符对应的字体检索信息长度为 4Byte  (b0~b25: 记录点阵信息的起始地址, b26~b31: 记录当前字符的象素宽度)
#define	GET_FONT_WIDTH(charinfo)		(charinfo >> 26)
#define	GET_FONT_OFFADDR(charinfo)		(charinfo & 0x3ffffff)
#define	DB2UC(c1,c2)					(c1 | (((U16)c2)<<8))

typedef struct tagFlSectionInfo{
            WORD  First;        
            WORD  Last;         
            DWORD OffAddr;     
       } FL_SECTION_INF, *PFL_SECTION_INF;

typedef struct tagFontLibHeader{
            BYTE    magic[4];    //'U'(or ’M’), 'F', 'L', X---Unicode(or MBCS) Font Library, X: 表示版本号. 分高低4位。如 0x12表示 Ver 1.2
            DWORD 	Size;            
            BYTE    nSection; // 共分几段数据，主要针对 UNICODE 编码有效。
            BYTE    YSize;                 
            WORD    wCpFlag;    // codepageflag:  bit0~bit13 每个bit分别代表一个CodePage 标志，如果是1，则表示当前CodePage 被选定，否则为非选定。
            char    reserved[4];      // 预留字节    
            FL_SECTION_INF  *pSection;
        } FL_HEADER, *PFL_HEADER;


int  font_init(char *pFontFile);
void font_exit();

BYTE font_get_ysize();
int  font_read_char_distX(WORD wCode);
int  font_read_char_dot_array(WORD wCode, BYTE *fontArray, WORD *bytesPerLine);

#endif // __FONT_H__
