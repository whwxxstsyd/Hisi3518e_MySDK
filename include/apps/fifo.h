#ifndef     __FIFO_H__
#define     __FIFO_H__

#ifdef	__cplusplus
extern "C" { 
#endif

#include "os_syscall.h"

#ifdef	__LINUX__
#include <pthread.h>
#include <unistd.h>
#endif

#include <stdio.h> 
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>

typedef struct _fifo_t
{     
    unsigned char *buffer;      /* the buffer holding the data */
    unsigned int size;          /* the size of the allocated buffer */
    unsigned int in;            /* data is added at offset (in % size) */
    unsigned int out;           /* data is extracted from off. (out % size) */
    pthread_mutex_t *lock;     /* protects concurrent modifications */
}fifo_t;

fifo_t *fifo_init(unsigned int size, pthread_mutex_t *lock);

void fifo_free(fifo_t *fifo);

unsigned int fifo_len(fifo_t *fifo);

unsigned int fifo_get(fifo_t *fifo, void *buffer, unsigned int size);
 
unsigned int fifo_put(fifo_t *fifo, void *buffer, unsigned int size);

unsigned int __fifo_len(fifo_t *fifo);

unsigned int __fifo_get(fifo_t *fifo, void *buffer, unsigned int size);

unsigned int __fifo_put(fifo_t *fifo, void *buffer, unsigned int size);

#ifdef __cplusplus
}
#endif	/* __cplusplus */

#endif

