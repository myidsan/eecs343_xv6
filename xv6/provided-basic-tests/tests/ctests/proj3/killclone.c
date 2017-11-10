/* When passing a non page-aligned stack to clone, clone should return -1. */
#include "types.h"
#include "user.h"

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
   struct proc *p;

   stackinit();
   ppid = getpid();
   assert(stack != NULL);
   block = malloc(32); 
   int clone_pid_one = clone(worker, 0 , stack);
   p = ptable.proc[clone_pid_one];
   printf(1, "p_name: %s\n", p);
   int clone_pid_two = clone(worker, 0 ,stack);
   kill(clone_pid_one);
   while ((int)block != 32648) {
     ;
   }
   printf(1, "parent: %d, clone_one: %d, clone_two: %d\n", ppid, clone_pid_one, clone_pid_two); 
   assert((int)block == 32648);
   // sbrk assigns additional memory to the user program
   // here it is pointing the stack to be at the beginning of this user program memeory
   // at this point the stack is page alligned but no memory is allocated yet

   printf(1, "TEST PASSED\n");
   exit();
}

void
worker(void *arg_ptr) {
   //assert(stack == 40);
   block = malloc(32);
   exit();
}
