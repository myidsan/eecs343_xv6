/* when a thread exits it should not affect other thread. If it does it should return -1.*/
#include "types.h"
#include "user.h"
//#include "proc.h"

#undef NULL
#define NULL ((void*)0)

#define PGSIZE (4096)

int ppid;
void *stack = NULL;
int first_thread_worked = 0;
int second_thread_working = 0;
int clone_pid_two = 0;

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
void worker2(void *arg_ptr);

int
main(int argc, char *argv[])
{
   stackinit();
   ppid = getpid();

   assert(stack != NULL);
   int clone_pid_one = clone(worker, 0 , stack);

   first_thread_worked = join(clone_pid_one);
   assert(first_thread_worked != 0);

   assert(clone_pid_two != 0);
   int working = join(clone_pid_two);
   printf(1, "%d\n", second_thread_working);
   assert(second_thread_working == 1);
   assert(working > 0);

   printf(1, "parent: %d, clone_one: %d,", ppid, clone_pid_one); 

   printf(1, "TEST PASSED\n");
   exit();
}

void
worker(void *arg_ptr) {
  printf(1, "entered worker1\n");
   clone_pid_two = clone(worker2, 0, stack);
  printf(1, "finished worker1\n");
   exit();
}
void 
worker2(void *arg_ptr) {
  printf(1, "entered worker2\n");
  while(first_thread_worked == 0);
  printf(1, "starting work for worker2\n");
  second_thread_working++;
  printf(1, "finished work for worker2\n");
  exit();
}
