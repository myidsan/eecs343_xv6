/* thread user library functions, testing lock */
#include "types.h"
#include "user.h"
#include "x86.h"

#undef NULL
#define NULL ((void*)0)

#define PGSIZE (4096)

int ppid;
int global = 1;
struct lock_t* lk;

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
  int i;
  int thread_pid[32];
  int join_pid;
  lk = (struct lock_t*)malloc(sizeof(struct lock_t));
  for(i = 0; i < arg; i++) {
    thread_pid[i] = thread_create(worker, &arg);
    assert(thread_pid[i] > 0);
  }
  for(i = 0; i < arg; i++) {
    join_pid = thread_join(thread_pid[i]);
    assert(join_pid > 0);
  }
  assert(global == 22);
  printf(1, "TEST PASSED\n");
  exit();
}

void
worker(void *arg_ptr) {
  int arg = *(int*)arg_ptr;
  assert(arg == 21);
  lock_acquire(lk);
  global++;
  lock_release(lk);

  exit();
}
