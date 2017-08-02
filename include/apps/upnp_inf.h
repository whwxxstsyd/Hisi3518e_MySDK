#ifndef     __UPNP_INF_H__
#define     __UPNP_INF_H__

#ifdef      __cplusplus
extern   "C"    {
#endif

int upnp_del_map_port(unsigned long UpnpHandle, int externalPort, int protocal);
int upnp_add_map_port(unsigned long UpnpHandle,  
    					   char *localip, 
    					   int    internalPort, 
    					   int    externalPort, 
    					   int    protocal, 
    					   const char  *szDesc);
int upnp_map_init(void);
void upnp_map_uninit(unsigned long UpnpHandle);
int upnp_get_extern_ip(char *ip);

#ifdef      __cplusplus
}
#endif

#endif
