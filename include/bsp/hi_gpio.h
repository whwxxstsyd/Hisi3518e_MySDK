#ifndef     __HI_GPIO_H__
#define     __HI_GPIO_H__

#include "my_timer.h"

#include <stdio.h>
#include <unistd.h>

#ifdef  __cplusplus
extern "C" {
#endif


#define     GPIO_PATH       "/sys/class/gpio/gpio"

#define     IRCUT_GPIO0            (8 * 8 + 0)             /* GPIO8_0 */
#define     IRCUT_GPIO1            (8 * 8 + 1)             /* GPIO8_1 */
#define     IRLED_GPIO             (7 * 8 + 5)              /* GPIO7_5 */
#define     DEFAULT_GPIO        (0 * 8 + 2)             /* GPIO0_2 */
#define     LED_GPIO            (7 * 8 + 3)             /* GPIO7_3 */
#define     LED_RED_GPIO		(7 * 8 + 4)				/* GPIO7_4 */

#define     ALARM_IN_GPIO       (5 * 8 + 3)             /* GPIO5_3 */
#define     ALARM_OUT_GPIO      (6)                     /* GPIO0_6 */
#define     AUDIO_GPIO      	(6 * 8 + 4)             /* GPIO6_4 */


typedef enum DIRECTION_T
{
    DIR_INPUT = 0,
    DIR_OUTPUT = 1,
}DIRECTION;


#ifdef  __cplusplus
class CGpio
{
    public:
		static CGpio *getInstance()
		{
			static CGpio *obj = NULL;
			if (NULL == obj)
			{
				obj = new CGpio();	
#if 0                
                if (obj->Init() < 0)
                {
                    delete(obj);
                    return NULL;
                }
#endif                
			}
			return obj;
		}        
        CGpio(void);
        ~CGpio(void);        
        int Init(void);
        void Uninit(void);
        int set_direction(int gpio, int direction);
        int set_value(int gpio, int value);
        int get_value(int gpio);        
        int ir_ctrl(int value);
        int ir_ctrl_status(void);

   private:
        int mGpioSemID;
        int mIrCutStatus;

   public:
        TIMER *mIrCutTimer;
        
};
#endif

int gpio_init(void);
int set_gpio_value(int gpio, int value);
int get_gpio_value(int gpio);
int set_gpio_direction(int gpio, int direction);
int ir_ctrl(int value);
int ir_status(void);


#ifdef  __cplusplus
}
#endif

#endif

