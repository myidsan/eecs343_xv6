/**/

#include "types.h"
#include "user.h"

#undef NULL
#define NULL ((void*)0)

#define PGSIZE (4096)

int ppid;
int global = 1;

#define assert(x) if (x) {} else { \
   printf(1, "%s: %d ", __FILE__, __LINE__); \
   printf(1, "assert failed (%s)\n", # x); \
   printf(1, "TEST FAILED\n"); \
   kill(ppid); \
   exit(); \
}

void worker(void *arg_ptr);

int
main(int argc, char *argv[])
{
   ppid = getpid();
   int main_join_pid = join(ppid);
   assert(main_join_pid == -1);

   void *stack = malloc(PGSIZE*2);
   assert(stack != NULL);
   if((uint)stack % PGSIZE)
     stack = stack + (PGSIZE - (uint)stack % PGSIZE);

   int arg = 42;
   int new_thread_pid = clone(worker, &arg, stack);
   assert(new_thread_pid > 0);
   printf(1, "new_thread_pid: %d\n", new_thread_pid);

   int non_new_thread_pid = new_thread_pid + 6; 
   assert(new_thread_pid != non_new_thread_pid);
   printf(1, "non_new_thread_pid: %d\n", non_new_thread_pid);
   int wrong_join_pid = join(non_new_thread_pid);
   printf(1, "wrong_join_pid:  %d\n", wrong_join_pid);
   assert(wrong_join_pid == -1);
   

   int join_pid = join(new_thread_pid);
   assert(join_pid == new_thread_pid);
   assert(global == 2);

   printf(1, "TEST PASSED\n");
   exit();
}

void
worker(void *arg_ptr) {
   int arg = *(int*)arg_ptr;
   assert(arg == 42);
   assert(global == 1);
   global++;
   exit();
}

