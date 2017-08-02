#ifndef		__WPA_CLI_BAK_H__
#define		__WPA_CLI_BAK_H__

#ifdef		__cplusplus
extern	"C"		{
#endif

int my_wpa_ctrl_command(struct wpa_ctrl *ctrl, const char *cmd, char *buf, int *len);
struct wpa_ctrl *wpa_init(void);
void wpa_uninit(void);

#ifdef		__cplusplus
}
#endif

#endif

