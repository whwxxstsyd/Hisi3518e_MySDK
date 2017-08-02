#ifndef __OS_SYSCALL_H__ 	
#define __OS_SYSCALL_H__

/* 线程部分从ffmpeg-2.4.2/compat/W32pthreads.h移植过来 */
#ifdef WIN32
#include <winsock2.h>
#include <windows.h>
#include <process.h> 

#if 1		/* 不会生成.lib文件 */
#define		SDK_API		__declspec(dllexport)
#else
#define		SDK_API		WINAPI
#endif
#define   	__thread  	__declspec(thread)      /* 线程局部变量 */

#if _WIN32_WINNT < 0x0600
#else
#define InitializeCriticalSection(x) InitializeCriticalSectionEx(x, 0, 0)
#define WaitForSingleObject(a, b) WaitForSingleObjectEx(a, b, FALSE)
#endif

typedef CRITICAL_SECTION pthread_mutex_t;

typedef void* pthread_t;
typedef struct pthread_arg_t
{
    void *arg;
    void *(*func)(void *arg);
}pthread_arg;

static unsigned int _stdcall win32thread_worker(void *arg)
{
    pthread_arg *thread_arg = (pthread_arg *)arg;

    thread_arg->func(thread_arg->arg);    
    free(arg);    

    return 0;
}

static int pthread_create(pthread_t *handle, const void *unused_attr,
                                    void *(*start_routine)(void*), void *arg)
{
	pthread_arg *thread_arg = (pthread_arg *)malloc(sizeof(pthread_arg));

    thread_arg->arg    = arg;
    thread_arg->func   = start_routine;
    *handle = (pthread_t)_beginthreadex(NULL, 0, win32thread_worker, thread_arg, 0, NULL);
    return !handle;
}

static void pthread_join(pthread_t handle, void **unused_value_ptr)     /* 无法取回返回值 */
{
    DWORD ret = WaitForSingleObject(handle, INFINITE);
    if (ret != WAIT_OBJECT_0)
        return;
    CloseHandle(handle);
}

static int pthread_mutex_init(pthread_mutex_t *m, void* attr)
{
    InitializeCriticalSection(m);
    return 0;
}
static int pthread_mutex_destroy(pthread_mutex_t *m)
{
    DeleteCriticalSection(m);
    return 0;
}
static int pthread_mutex_lock(pthread_mutex_t *m)
{
    EnterCriticalSection(m);
    return 0;
} 
static int pthread_mutex_unlock(pthread_mutex_t *m)
{
    LeaveCriticalSection(m);
    return 0;
}

#define snprintf  _snprintf 

/* 网络部分 */
#include <ws2tcpip.h>

#pragma comment(lib,"ws2_32.lib")

typedef     int     socklen_t;
typedef     int     ssize_t;

#define     MSG_NOSIGNAL    (0)
#define		EINPROGRESS		WSAEINPROGRESS
#define		EWOULDBLOCK		WSAEWOULDBLOCK
#define		ECONNABORTED	WSAECONNABORTED

static int winsock_init(void)
{
	WSADATA wd;

	if (WSAStartup(MAKEWORD(2, 2), &wd) != 0)
        return -1;
	return 0;
}

static void winsock_uninit(void)
{
	WSACleanup();
}

#endif      /* WIN32 */

#ifdef __LINUX__ 

#include <sys/socket.h>
#include <netinet/in.h>
#include <unistd.h>
#include <unistd.h>
#include <netdb.h>
#include <arpa/inet.h>
#include <netinet/in.h>
#include <netinet/tcp.h>
#include <netinet/ip_icmp.h>
#include <net/if.h>
#include <sys/time.h>
#include <sys/ioctl.h>
#include <sys/un.h>
#include <sys/socket.h>

typedef     int             SOCKET;
typedef     unsigned char   BYTE;
typedef     unsigned long   DWORD;

#define     FALSE   0
#define     SOCKET_ERROR    (-1)

#define		SDK_API

static inline int winsock_init(void)
{
    return 0;
}

static inline void winsock_uninit(void)
{

}

static inline void Sleep(int time_ms)
{
    usleep(time_ms * 1000);
}

static inline void closesocket(SOCKET socket)
{
    close(socket);
}

static inline int ioctlsocket(SOCKET fd, long cmd, unsigned long *arg)
{
    return ioctl(fd, cmd, arg);
}

static inline unsigned int GetTickCount(void)
{
	struct timeval tv;

	gettimeofday(&tv, NULL);
	
	return (tv.tv_sec * 1000) + (tv.tv_usec / 1000);
}

#endif      /* __LINUX__ */


#endif /* __W32_SYSCALL_H__ */

