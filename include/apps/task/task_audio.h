#ifndef		__TASK_AUDIO_H__
#define		__TASK_AUDIO_H__

#include "task_base.h"
#include "hi_record.h"

class CTaskAudio: public CTaskBase
{
    public:
		static CTaskAudio *getInstance()
		{
			static CTaskAudio *obj = NULL;
			if (NULL == obj)
			{
				obj = new CTaskAudio();		
			}
			return obj;
		}   
        
        CTaskAudio();
        ~CTaskAudio();
		int Init(void);   
		void Uninit(void);	
        
   private:
        pthread_t mPlaybackId;
};

int audio_task_create(void);
void audio_task_destory(void);

#endif
