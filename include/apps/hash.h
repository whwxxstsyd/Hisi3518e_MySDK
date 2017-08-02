#ifndef     __HASH_H__
#define     __HASH_H__

#include "linux_list.h"

#ifdef	__cplusplus
extern "C" {
#endif

typedef struct HASH_UNIT_T
{
	void *pData;
    union
    {
        int index;
	    char key_word[32];      
    }own;       /* 关键字或者索引,用来解决冲突 */
	struct list_head list;
}HASH_UNIT;

#ifdef  __cplusplus
class CHash
{
    public:
        CHash(void);
        ~CHash(void);
        int Init(int size, int unit_size);
        void Uninit(void);
        unsigned int GetHashPos(const char *key_word);   
        unsigned int GetHashPos(int index);          
        void *AllocRecord(const char *key_word);
        void *AllocRecord(int index);        
        void *GetRecord(const char *key_word);
        void *GetRecord(int index);          

    private:
        struct list_head *mListArray;
        int mSize;
        int mUnitSize;
};
#endif

#ifdef		__cplusplus
}
#endif

#endif

