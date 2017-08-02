#ifndef		__CGI_SERVER_EXT__
#define		__CGI_SERVER_EXT__

#ifdef	__cplusplus
extern "C" {
#endif

int set_param_ext(const char *in, char *out);
int get_param_ext(const char *in, char *out);
int cmd_proc_ext(const char *in, char *out);
int param_ext(unsigned int type, const char *in, char *out);

#ifdef	__cplusplus
}
#endif

#endif
