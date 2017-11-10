/* thread user library functions */
#include "types.h"
#include "stat.h"
#include "user.h"
#include "fs.h"

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

  int arg = 21;
  int thread_pid = thread_create(worker, &arg);
  printf(1,"pid = %d\n",thread_pid);
  assert(thread_pid > 0);

  int join_pid = thread_join(thread_pid);
  assert(join_pid == thread_pid);
  assert(global == 2);
  printf(1, "TEST PASSED\n");
  exit();
}

void
worker(void *arg_ptr) {
  int arg = *(int*)arg_ptr;
  assert(arg == 21);
  assert(global == 1);
  global++;
  exit();
}
