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
   stackinit();
   assert(stack != NULL);
   printf(1, "before stack: %d\n", stack);
   printf(1, "before block: %d\n", block);
   block = malloc(32); 
   printf(1, "after block: %d\n", block);
   ppid = clone(worker, 0 , stack);
   clone(worker, 0 ,stack);
   printf(1, "first clone block: %d\n", block);
   while ((int)block != 32648) {
     ;
   }
   printf(1, "second clone block: %d\n", block);
   assert((int)block == 32648);
   printf(1, "after block: %d\n", stack);
   printf(1, "after block: %d\n", block);
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
