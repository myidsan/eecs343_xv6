#include "types.h"
#include "stat.h"
#include "user.h"
#include "x86.h"
#define PGSIZE 4096

int 
thread_create(void (*start_routine)(void*), void *arg) {
  void *stack;
  stack = malloc(2*PGSIZE);
  if((uint)stack % PGSIZE != 0)
    stack += (PGSIZE - (uint)stack % PGSIZE);
  return clone(start_routine, arg, stack);
}

int 
thread_join(int pid) {
  int ret = join(pid);
  return ret;
}

void 
lock_acquire(struct lock_t *lock) 
{
  while(xchg(&lock->locked, 1) != 0);
}

void 
lock_release(struct lock_t *lock) 
{
  xchg(&lock->locked, 0);
}

void 
lock_init(struct lock_t *lock) 
{
  lock->locked = 0;
}

/*
void 
cv_wait(cond_t* conditionVariable, lock_t *cv_lock) {
  cvwait(conditionVariable, cv_lock);
  return;
}


void 
cv_signal(cond_t *conditionVariable) {
  cvsignal(conditionVariable);
  return;
}
*/
