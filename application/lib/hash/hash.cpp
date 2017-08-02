#include "linux_list.h"
#include "hash.h"
#include "debug.h"

#include <stdlib.h>
#include <stdio.h>
#include <string.h>

#undef		TARGET_LEVEL
#define		TARGET_LEVEL	LEVEL_INFO

CHash::CHash(void)
{

}

CHash::~CHash(void)
{

}

unsigned int HashString(const char *str)		/* hash算法 */
{   
#if 0
	unsigned long hash = 0x71e2c3d8;
	
    while (*str != 0)    
    {        
        hash <<= 1;       
        hash += *str++;      
    } 
	
    return hash;
#else
	unsigned int h = 0;

	for (; *str; str++)
	{		
		h = *str + h * 27;
	}
	
	return h;
#endif	
}

unsigned int CHash::GetHashPos(const char *key_word)
{
    return (HashString(key_word) % mSize);
}

unsigned int CHash::GetHashPos(int index)
{
    return (index % mSize);
}

int CHash::Init(int size, int unit_size)
{
	int i;

	mSize = size;
	mUnitSize = unit_size;

	mListArray = (struct list_head *)calloc(size, sizeof(struct list_head));
	if (NULL == mListArray)
	{
		LOG_ERROR("calloc fail\n");
		return -1;
	}

	for (i = 0; i < size; i++)
		INIT_LIST_HEAD(&mListArray[i]);

	return 0;
}

void *CHash::AllocRecord(const char *key_word)		/* 用于插入表项 */
{
	int pos = GetHashPos(key_word);
	HASH_UNIT *pNewRecord = NULL;

	pNewRecord = (HASH_UNIT *)calloc(1, sizeof(HASH_UNIT));
	if (NULL == pNewRecord)
	{
		LOG_WARN("calloc fail\n");
		return NULL;
	}
	pNewRecord->pData = calloc(1, mUnitSize);
	if (NULL == pNewRecord)
	{
		LOG_WARN("calloc fail\n");
		free(pNewRecord);
		return NULL;
	}		
	strcpy(pNewRecord->own.key_word, key_word);	
	list_add_tail(&pNewRecord->list, &mListArray[pos]);
	
	return pNewRecord->pData;		
}

void *CHash::AllocRecord(int index)
{
	int pos = GetHashPos(index);
	HASH_UNIT *pNewRecord = NULL;

	pNewRecord = (HASH_UNIT *)calloc(1, sizeof(HASH_UNIT));
	if (NULL == pNewRecord)
	{
		LOG_WARN("calloc fail\n");
		return NULL;
	}
	pNewRecord->pData = calloc(1, mUnitSize);
	if (NULL == pNewRecord)
	{
		LOG_WARN("calloc fail\n");
		free(pNewRecord);
		return NULL;
	}		
	pNewRecord->own.index = index;	
	list_add_tail(&pNewRecord->list, &mListArray[pos]);
	
	return pNewRecord->pData;		
}

void *CHash::GetRecord(const char *key_word)		/* 用于获取表项 */
{
	int pos = GetHashPos(key_word);
	HASH_UNIT *pRecord = NULL;
		
	list_for_each_entry(pRecord, &mListArray[pos], list)
	{
		if (!strcmp(pRecord->own.key_word, key_word))
			break;
	}
		
	if (&pRecord->list == &mListArray[pos])
	{
		LOG_DEBUG("bad key_word[%s]\n", key_word);	
		return NULL;		
	}

	return pRecord->pData;
}

void *CHash::GetRecord(int index)		/* 用于获取表项 */
{
	int pos = GetHashPos(index);
	HASH_UNIT *pRecord = NULL;
		
	list_for_each_entry(pRecord, &mListArray[pos], list)
	{
		if (pRecord->own.index == index)
			break;
	}
		
	if (&pRecord->list == &mListArray[pos])
	{
		LOG_INFO("bad index[0x%x]\n", index);	
		return NULL;		
	}

	return pRecord->pData;
}

void CHash::Uninit(void)
{
	int i;
	HASH_UNIT *n_pRecord = NULL, *pRecord = NULL;

	for (i = 0; i < mSize; i++)
	{
		list_for_each_entry_safe(pRecord, n_pRecord, &mListArray[i], list)
		{
			list_del(&pRecord->list);
			free(pRecord);
		}
	}
	free(mListArray);
}

