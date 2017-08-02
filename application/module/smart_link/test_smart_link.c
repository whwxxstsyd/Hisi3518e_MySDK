#include "smart_link.h"
#include "debug.h"
#include "common.h"

int main(int argc, char *argv[])
{
    char ssid[32] = {0}, key[32] = {0} ,authmode[0] = {32};

#ifdef MEMORY_WATCH
	mem_check_init();
	LOG_INFO("init memory watch\n");
#endif	
    
    if (wifi_init() < 0)
    {
        LOG_ERROR("wifi_init fail\n"); 
        return -1;
    }
    SmartLinkProc(ssid, key, authmode, SMART_LINK_TIMEOUT_MS);

    return 0;
}

