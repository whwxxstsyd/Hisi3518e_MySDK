#ifndef     __MOTOR_DRV_H__
#define     __MOTOR_DRV_H__

#if 0
typedef enum SPEED_LEVEL_T
{
    SPEED_LOW = 0,
    SPEED_NORMAL,
    SPEED_HIGH,
    SPEED_MAX,
}SPEED_LEVEL;
#endif

typedef enum MOTOR_CMD_T
{
    UP = 0,
    DOWN,
    LEFT,
    RIGHT,
    UPDOWN,
    LEFTRIGHT,
    STOP,
    STOP_H,
    STOP_V,
    ALL, 
    SET_POS,
    GET_POS,    
    GOTO_POS,
    CLR_POS,
    MAX_CMD,
}MOTOR_CMD;

#endif
