#ifndef SEMAPHORE_H
#define SEMAPHORE_H

/*
  Author   : Benoit PAPILLAULT <benoit.papillault@free.fr>
  Creation : 17/12/2002
  License  : GPL

  $Id$
*/

#if defined(__GNU_LIBRARY__) && defined(_SEM_SEMUN_UNDEFINED)
/* l'union semun est définie en incluant <sys/sem.h> */
#else
/* d'après X/OPEN nous devons la définir nous-même */
typedef union  semun
{
   int val;                           /* value for SETVAL */
   struct semid_ds *buf;              /* buffer for IPC_STAT & IPC_SET */
   unsigned short int *array;         /* array for GETALL & SETALL */
   struct seminfo *__buf;             /* buffer for IPC_INFO */
};
#endif

typedef union semun* semunptr;

/*
  semaphore_init:

  create a semaphore and return the semaphore id or -1 in case of errors
*/

int semaphore_init(union semun  **count);

/*
  semaphore_incr:

  increment the value of the semaphore. this function is not blocking
*/

int semaphore_incr(int sem, int val);

/*
  semaphore_decr:

  decrement the value of the semaphore. this function is blocking if
  the current value of the semaphore is less than val
*/

int semaphore_decr(int sem, int val);

/*
  semaphore_done:

  destroy the semaphore (you can check with ipcs -a)
*/

int semaphore_done(int sem);

#endif
