/*
  Author   : Benoit PAPILLAULT <benoit.papillault@free.fr>
  Creation : 17/12/2002
  License  : GPL

  $Id$
*/

#include "semaphore.h"

#include <sys/types.h>
#include <sys/ipc.h>
#include <sys/sem.h>
#include <string.h>
#include <errno.h>

/* for ident(1) command */
static const char id[] = "@(#) $Id$";

/*
  09/12/2003 Benoit PAPILLAULT

  We define union semun here (in .c file) since it's not needed in .h
  file.  Currently <sys/sem.h> includes <bits/sem.h> where union semun
  is not defined, but _SEM_SEMUN_UNDEFINED is set to 1.
*/

#if defined(__GNU_LIBRARY__) && !defined(_SEM_SEMUN_UNDEFINED)
/* l'union semun est d�finie en incluant <sys/sem.h> */
#else
/* d'apr�s X/OPEN nous devons la d�finir nous-m�me */
union  semun
{
   int val;                           /* value for SETVAL */
   struct semid_ds *buf;              /* buffer for IPC_STAT & IPC_SET */
   unsigned short int *array;         /* array for GETALL & SETALL */
   struct seminfo *__buf;             /* buffer for IPC_INFO */
};
#endif

int semaphore_init(int count)
{
    int sem;
    union semun un;

    sem = semget(IPC_PRIVATE, 1, 0666);
    if (sem == -1)
        return -1;

    un.val = count;

    if (semctl(sem, 0, SETVAL, un) == -1)
        return(-1);

    return(sem);
}

int semaphore_incr(int sem, int val)
{
	int ret;
    struct sembuf buf;

    buf.sem_num = 0;
    buf.sem_op  = val;
    buf.sem_flg = 0;

	do{
    	ret = semop(sem, &buf, 1);
	}while (ret<0 && errno==EINTR);
    return(ret);
}

int semaphore_decr(int sem, int val)
{
	int ret;
    struct sembuf buf;

    buf.sem_num = 0;
    buf.sem_op  = -val;
    buf.sem_flg = 0;

	do{
    	ret = semop(sem, &buf, 1);
	}while (ret<0 && errno==EINTR);
    return(ret);
}

int semaphore_done(int sem)
{
   union semun un;

   /* union semun is ignored when used with IPC_RMID */
 
   if (semctl(sem, 0, IPC_RMID, un) == -1)
        return(-1);

    return(0);
}
