#ifndef		__STRING_PARSER_H__
#define		__STRING_PARSER_H__

#ifdef	__cplusplus
extern "C" {
#endif

#include "param.h"
#include "common.h"
#include "hash.h"

#include <stdio.h>

typedef struct PARSER_INDEX_INFO_T
{
    int number;
}PARSER_INDEX_INFO;

typedef struct PARSER_STRING_INFO_T
{
    char *string;
}PARSER_STRING_INFO;

#ifdef  __cplusplus

class CStringParser
{
	public:
		static CStringParser& getInstance()
		{
			static CStringParser mInstance;

            return mInstance;
		}
		CStringParser();
		~CStringParser();
        int Init(void);
        void Uninit(void);        
        int init_parser_tab(const char *str, int index);		
		int get_index(const char *string);
		char *get_string(int index);
		
	private:
		/* ×Ö·û´®×ªË÷Òý */
        CHash mIndexHash;	
		/* Ë÷Òý×ª×Ö·û´® */	        
        CHash mStringHash;
        int mInit;
};
#endif

void set_int_safe(int *var, char *string);
void set_int_safe1(int *var, int min, int max, int value);
int Str2Index(const char *string);
char *Index2Str(int index);
int string_parser_init(void);
void string_parser_uninit(void);

#ifdef		__cplusplus
}
#endif


#endif
