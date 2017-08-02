#ifndef		__TASK_GPIO_H__
#define		__TASK_GPIO_H__

#include "task_base.h"
#include "common.h"
#include "my_timer.h"

typedef enum ADC_LIMIT_T
{
    ADC_UP_LIMIT = 0xe,    /* IRLED 关闭(切换到白天模式) */
    ADC_DOWN_LIMIT = 0x7,  /* IRLED 打开(切换到夜视模式) */
    AGAIN_LIMIT = 13 * 1024,    /* again 大于 AGAIN_LIMIT, 才切换到夜视模式*/
}ADC_LIMIT;

typedef enum VIDEO_MODE_T
{
    GRAY_MODE = 0,      /* 灰度模式 */
    COLOR_MODE = 1,     /* 彩色模式 */
}VIDEO_MODE;

class CTaskGpio: public CTaskBase
{
    public:
		static CTaskGpio *getInstance()
		{
			static CTaskGpio *obj = NULL;
			if (NULL == obj)
			{
				obj = new CTaskGpio();		
			}
			return obj;
		}

        CTaskGpio();
        ~CTaskGpio();
		int Init(void);
		void Uninit(void);       
		void Process(void);  

    public:
        int mMode;
        int mGpioValue;
        TIMER *mLedTimer;
};

int gpio_task_create(void);
void gpio_task_destory(void);

#endif
