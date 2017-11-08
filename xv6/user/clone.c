#include "types.h"
#include "user.h"

#define PGSIZE (4096)

volatile int global = 1;

void DoThreadWork(void* arg_ptr); // function signature for our DoThreadFunction

int
main(int argc, char* argv[])
{
   void* stack = malloc(PGSIZE*2); // allocate 2 pages of space
    if((uint)stack % PGSIZE) {
       // make sure that stack is page-aligned
      stack = stack + (PGSIZE - (uint)stack % PGSIZE);
    }
    void* arg = NULL; // our DoThreadWork function is simple and doesn't need an arg
    int clone_pid = clone(DoThreadWork, arg, stack);
    if(clone_pid < 0) {
      printf(1, "clone failed\n");
      exit();
    }
      // the main thread of execution (aka parent process) continues executing here
    while(global != 5) {
      printf(1, "waiting.... global should be 1...\n");
      printf(1, "global: %d\n", global); // prints "global: 1"
      //;
    }
    printf(1, "done waiting! global should be 5...\n");
    printf(1, "global: %d\n", global); // prints "global: 5"
    exit();
}

void
DoThreadWork(void* arg_ptr) {
// clone creates a new thread, which begins execution here
  printf(1, "accessed thread\n");
  global = 5;
  exit();
}
