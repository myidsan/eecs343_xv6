/* when a thread exits it should not affect other thread. If it does it should return -1.*/
#include "types.h"
#include "user.h"
#include "user.h"
//#include "proc.h"

#undef NULL
#define NULL ((void*)0)

#define PGSIZE (4096)

int ppid;
void *stack = NULL;
void *block = NULL;

void stackinit() {
  stack = malloc(PGSIZE*2);
}
#define assert(x) if (x) {} else { \
   printf(1, "%s: %d ", __FILE__, __LINE__); \
   printf(1, "NULLrt failed (%s)\n", # x); \
   printf(1, "TEST FAILED\n"); \
   kill(ppid); \
   exit(); \
}

void worker(void *arg_ptr);

int
main(int argc, char *argv[])
{
   //struct proc *p;

   stackinit();
   ppid = getpid();

   assert(stack != NULL);
   block = malloc(32); 
   int clone_pid_one = clone(worker, 0 , stack);
   //p = ptable.proc[clone_pid_one];
   //printf(1, "p_name: %s\n", p);
   printf(1, "before procdump\n");
   listproc();
   printf(1, "initial procdump\n");
   //int clone_pid_two = clone(worker, 0 ,stack);
   //kill(clone_pid_one);
   listproc();
   printf(1, "parent: %d, clone_one: %d,", ppid, clone_pid_one); 
   //printf(1, "parent: %d, clone_one: %d, clone_two: %d\n", ppid, clone_pid_one, clone_pid_two); 

   printf(1, "TEST PASSED\n");
   exit();
}

void
worker(void *arg_ptr) {
   block = malloc(32);
   exit();
}
