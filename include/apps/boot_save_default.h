#ifndef		__BOOT_SAVE_DEFAULT_H__
#define		__BOOT_SAVE_DEFAULT_H__

#ifdef      __cplusplus
extern  "C"     {
#endif

int default_flash_config(char *file, struct SYSTEM_PARAM_T *pSystemParam);
int default_factory_config(char *file, struct SYSTEM_PARAM_T *pSystemParam);

/* 外部接口 */
int sys_boot_config(void);
int sys_default_config(void);
int sys_save_flash_config(void);
int sys_save_factory_config(void);

#ifdef      __cplusplus
}
#endif

#endif
