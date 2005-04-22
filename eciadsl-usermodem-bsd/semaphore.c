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

/* for ident(1) command */
static const char id[] = "@(#) $Id$";

/*
  On Linux, __GNU_LIBRARY__ is defined in <features.h> which is included
  by <sys/ipc.h>. _SEM_SEMUN_UNDEFINED is defined in <bits/sem.h>
*/

#if defined(__GNU_LIBRARY__) && defined(_SEM_SEMUN_UNDEFINED)
union semun
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

    sem = semget(IPC_PRIVATE, 1, 0666);
    if (sem == -1)
        return -1;

    if (semctl(sem, 0, SETVAL, count) == -1)
        return(-1);

    return sem;
}

int semaphore_incr(int sem, int val)
{
    struct sembuf buf;

    buf.sem_num = 0;
    buf.sem_op  = val;
    buf.sem_flg = 0;

    if (semop(sem, &buf, 1) == -1)
        return(-1);

    return 0;
}

int semaphore_decr(int sem, int val)
{
    struct sembuf buf;

    buf.sem_num = 0;
    buf.sem_op  = -val;
    buf.sem_flg = 0;

    if (semop(sem, &buf, 1) == -1)
        return(-1);

    return 0;
}

int semaphore_done(int sem)
{
   union semun un;
 
   if (semctl(sem, 0, IPC_RMID, un) == -1)
        return(-1);

    return 0;
}
