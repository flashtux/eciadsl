#ifndef SEMAPHORE_H
#define SEMAPHORE_H

/*
  Author   : Benoit PAPILLAULT <benoit.papillault@free.fr>
  Creation : 17/12/2002
  License  : GPL

  $Id$

  TODO list:
  - document the use of "union" in semaphore.c
  - document why we define "union" in semaphore.h
  - document semaphore_init(int count)
  - make a separate test program
*/

/*
  semaphore_init:

  create a semaphore and return the semaphore id initialized with the
  specified count or -1 in case of errors
*/

int semaphore_init(int count);

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
