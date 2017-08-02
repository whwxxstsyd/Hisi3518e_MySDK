#ifndef		__TASK_RTMP_H__
#define		__TASK_RTMP_H__

#include "task_base.h"
#include "common.h"
#include "param.h"
#include "rtmpsrv.h"

class CTaskRtmp: public CTaskBase
{
    public:
    	static CTaskRtmp *getInstance()
    	{
    		static CTaskRtmp *obj = NULL;
    		if (NULL == obj)
    		{
    			obj = new CTaskRtmp();		
    		}
    		return obj;
    	}

        CTaskRtmp(void);
        ~CTaskRtmp(void);
    	int Init(void);
    	void Uninit(void);	

    private:
        RTMP_PARAM *mpRtmpParam;
};

int rtmp_task_create(void);
void rtmp_task_destory(void);


#endif
