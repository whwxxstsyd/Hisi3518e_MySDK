#ifndef     __SEM_H__
#define     __SEM_H__

#include <sys/ipc.h>
#include <sys/shm.h>
#include <sys/sem.h>

union semun   
{  
    int val;  
    struct semid_ds *buf;  
    unsigned short *array;  
}; 

int GetSem(int SemID);
int PostSem(int SemID);
int CreateSem(int SemKey);
int UninitSem(int SemID);
int CleanSem(int SemID);

#endif
