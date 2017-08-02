#ifndef     __MEM_WATCH_H__
#define     __MEM_WATCH_H__

#ifdef	__cplusplus
extern "C" { 
#endif

#include "linux_list.h"

#include <stdlib.h>     /* 避免calloc/malloc/free重复定义 */
#include <string.h>     /* 避免strdup重复定义 */

#ifndef NONE_MEMWATCH   /* 定义了NONE_MEMWATCH, memwatch无效 */
#define     calloc(num, size)       db_calloc(num, size, __FILE__, __func__, __LINE__)
#define     malloc(size)            db_malloc(size, __FILE__, __func__, __LINE__)
#define     realloc(addr, size)     db_realloc(addr, size, __FILE__, __func__, __LINE__)
#define     free(addr)              db_free(addr, __FILE__, __func__, __LINE__)
#define     strdup(str)             db_strdup(str, __FILE__, __func__, __LINE__)
#endif

#define     MEMTRACE_ENABLE_FILE    "/tmp/memtrace"
#define     MEMTRACE_LOG_FILE       "/tmp/memtrace.log"

typedef struct MEM_RECORD_T
{
    void *addr;
    int size;
    char file[32];
    char func[32];
    int line;
    struct list_head list;
}MEM_RECORD;

void mem_check_init(void);
void mem_check_uninit(void);
void mem_dump(const char *file);
void *db_calloc(int num, int size, const char *path, const char *func, int line);
void *db_malloc(int size, const char *path, const char *func, int line);
void *db_realloc(void *addr, int size, const char *path, const char *func, int line);
void db_free(void *addr, const char *path, const char *func, int line);
char *db_strdup(const char *str, const char *path, const char *func, int line);

#ifdef	__cplusplus
}
#endif

#endif

