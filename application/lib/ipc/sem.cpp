#include "sem.h"
#include "debug.h"

#include <sys/ipc.h>
#include <sys/shm.h>
#include <sys/sem.h>
#include <errno.h>

int GetSem(int SemID)
{
	struct sembuf sb;

	sb.sem_num = 0;
	sb.sem_op = -1;
	sb.sem_flg = SEM_UNDO;		/* 进程异常退出后,自动释放,避免死锁 */
	
	if (semop(SemID, &sb, 1) < 0)
	{
		LOG_ERROR("semop error\n");
		return -1;		
	}

	return 0;
}

int PostSem(int SemID)
{
	int ret;
	struct sembuf sb;

	sb.sem_num = 0;
	sb.sem_op = 1;
	sb.sem_flg = SEM_UNDO;		/* 进程异常退出后,自动释放,避免死锁 */

	//操作一个或一组信号
	//当semop小于0 时，表示获取信号量失败
	if ((ret = semop(SemID, &sb, 1)) < 0)
	{
		LOG_ERROR("semop error[%d]\n", ret);
		perror("semop");
		return -1;
	}

	return 0;
}

int CreateSem(int SemKey)
{
	int SemID;
	union semun sem_union;  

	//创建一个新的信号量或获取一个已经存在的信号量的键值。
	SemID = semget(SemKey, 1, IPC_CREAT | IPC_EXCL | 0666);
    if ((SemID < 0) && (errno == EEXIST))	/* 信号量已经存在 */
    {
		SemID = semget(SemKey, 1, IPC_CREAT | 0666);
		if (SemID < 0)
		{
	        LOG_ERROR("shmget error\n");
			return -1;	
		}
    }
	else if (SemID >= 0)	/* 信号量不存在,创建并初始化 */
	{
		union semun sem_union;  

		sem_union.val = 1;  
		if (semctl(SemID, 0, SETVAL, sem_union) < 0) 
		{
			LOG_ERROR("error at semctl\n");
			return -1;  
		}
	}
	else
    {
        LOG_ERROR("shmget error\n");
		return -1;		
    }

	return SemID;  
}

int CleanSem(int SemID)
{
	union semun sem_union;  

	sem_union.val = 1;  
	if(semctl(SemID, 0, SETVAL, sem_union) < 0) 
	{
		LOG_ERROR("error at semctl\n");
		return -1;  
	}

	return 0;
}

int UninitSem(int SemID)
{
	union semun sem_union;  

	if(semctl(SemID, 0, IPC_RMID, sem_union) < 0)
	{
		LOG_INFO("error at del sem\n");
		return -1;
	}

	return 0;
}
