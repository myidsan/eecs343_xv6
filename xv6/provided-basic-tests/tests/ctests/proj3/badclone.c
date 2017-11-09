/* When passing a non page-aligned stack to clone, clone should return -1. */
#include "types.h"
#include "user.h"

#undef NULL
#define NULL ((void*)0)

#define PGSIZE (4096)

int ppid;

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
   void *stack = malloc(PGSIZE*2);
   assert(stack != NULL);
   if((uint)stack % PGSIZE == 0)
     stack += 4;

   assert(clone(worker, 0, stack) == -1);  // when stack is not page-aligned, clone should fail

   stack = sbrk(0);
   // sbrk assigns additional memory to the user program
   // here it is pointing the stack to be at the beginning of this user program memeory
   if((uint)stack % PGSIZE){
     stack = stack + (PGSIZE - (uint)stack % PGSIZE);
   } // at this point the stack is page alligned but no memory is allocated yet
   sbrk( ((uint)stack - (uint)sbrk(0)) + PGSIZE/2 );
   // sbrk here is allocating half PGSIZE to the user program her 
   // starting from stack - sbrk(0)
   assert((uint)stack % PGSIZE == 0);
   assert((uint)sbrk(0) - (uint)stack == PGSIZE/2);

   assert(clone(worker, 0, stack) == -1);

   printf(1, "TEST PASSED\n");
   exit();
}

void
worker(void *arg_ptr) {
   exit();
}
