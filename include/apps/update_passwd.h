#ifndef     __UPDATE_PASSWD_H__
#define     __UPDATE_PASSWD_H__

#ifdef	__cplusplus
extern "C" { 
#endif

int update_passwd(const char *filename, const char *name, const char *new_passwd, const char *member);

#ifdef	__cplusplus
}
#endif

#endif

