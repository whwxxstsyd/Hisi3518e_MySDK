#include "fifo.h"

/* 判断是否为2的次幂 */
#define     is_power_of_2(x)    ((x) != 0 && (((x) & ((x) - 1)) == 0))
/* 取a和b中的最小值 */
#define     min(a,b)    (((a) < (b)) ? (a) : (b))

/* 在双线程模式下可以直接调用__fifo_put/__fifo_get,无需加锁 */
fifo_t *fifo_init(unsigned int size, pthread_mutex_t *lock)
{
    fifo_t *fifo = NULL;
    
    if (!is_power_of_2(size))
    {
        printf("size must be power of 2\n");
        return fifo;
    }
    fifo = (fifo_t *)malloc(sizeof(fifo_t));
    if (!fifo)
    {
        printf("malloc fail\n");
        return fifo;
    }
    memset(fifo, 0, sizeof(fifo_t));
    fifo->buffer = malloc(size);
    if (!fifo->buffer)
    {
        printf("malloc fail\n");
        free(fifo);
        fifo = NULL;
        return NULL;
    }
    fifo->size = size;
    fifo->in = 0;
    fifo->out = 0;
    fifo->lock = lock;

    return fifo;
}

void fifo_free(fifo_t *fifo)
{
    if (!fifo)
        return;
    if (fifo->buffer)
    {
        free(fifo->buffer);
        fifo->buffer = NULL;
    }
    free(fifo);
    fifo = NULL;
}

/* 数据长度 */
unsigned int __fifo_len(fifo_t *fifo)
{
    return (fifo->in - fifo->out);
}

/* 取数据 */
unsigned int __fifo_get(fifo_t *fifo, void *buffer, unsigned int size)
{
    unsigned int len;
    
    if ((fifo == NULL) || (buffer == NULL))
        return -1;

    size = min(size, fifo->in - fifo->out);

#ifdef	__LINUX__
    __sync_synchronize();   /* 放置内存屏障,实现无锁并发 */
#endif

    len = min(size, fifo->size - (fifo->out & (fifo->size - 1)));
    memcpy(buffer, fifo->buffer + (fifo->out & (fifo->size - 1)), len);
    memcpy((char *)buffer + len, fifo->buffer, (int)(size - len));

#ifdef	__LINUX__
    __sync_synchronize();    
#endif

    fifo->out += size;
    
    return size;
}

unsigned int __fifo_put(fifo_t *fifo, void *buffer, unsigned int size)
{
    unsigned int len = 0;

    if ((fifo == NULL) || (buffer == NULL))
        return -1;

    size = min(size, fifo->size - fifo->in + fifo->out);

#ifdef	__LINUX__
    __sync_synchronize();
#endif

    len  = min(size, fifo->size - (fifo->in & (fifo->size - 1)));
    memcpy(fifo->buffer + (fifo->in & (fifo->size - 1)), buffer, len);
    memcpy(fifo->buffer, (char *)buffer + len, (int)(size - len));

#ifdef	__LINUX__
    __sync_synchronize();
#endif

    fifo->in += size;
    
    return size;
}

unsigned int fifo_len(fifo_t *fifo)
{
    unsigned int len = 0;

    if (fifo == NULL)
        return -1;    
    
    pthread_mutex_lock(fifo->lock);
    len = __fifo_len(fifo);
    pthread_mutex_unlock(fifo->lock);
    
    return len;
}

unsigned int fifo_get(fifo_t *fifo, void *buffer, unsigned int size)
{
    unsigned int ret;

    if ((fifo == NULL) || (buffer == NULL))
        return -1;
        
    pthread_mutex_lock(fifo->lock);
    ret = __fifo_get(fifo, buffer, size);
    if (fifo->in == fifo->out)
        fifo->in = fifo->out = 0;
    pthread_mutex_unlock(fifo->lock);
    
    return ret;
}
 
unsigned int fifo_put(fifo_t *fifo, void *buffer, unsigned int size)
{
    unsigned int ret;
    
    pthread_mutex_lock(fifo->lock);
    ret = __fifo_put(fifo, buffer, size);
    pthread_mutex_unlock(fifo->lock);
    
    return ret;
}

