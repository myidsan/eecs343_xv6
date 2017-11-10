#include "types.h"
#include "x86.h"
#include "defs.h"
#include "param.h"
#include "mmu.h"
#include "proc.h"
#include "sysfunc.h"

int
sys_fork(void)
{
  return fork();
}

int
sys_exit(void)
{
  exit();
  return 0;  // not reached
}

int
sys_wait(void)
{
  return wait();
}

int
sys_kill(void)
{
  int pid;

  if(argint(0, &pid) < 0)
    return -1;
  return kill(pid);
}

int
sys_getpid(void)
{
  return proc->pid;
}

int
sys_sbrk(void)
{
  int addr;
  int n;

  if(argint(0, &n) < 0)
    return -1;
  addr = proc->sz;
  if(growproc(n) < 0)
    return -1;
  return addr;
}

int
sys_sleep(void)
{
  int n;
  uint ticks0;
  
  if(argint(0, &n) < 0)
    return -1;
  acquire(&tickslock);
  ticks0 = ticks;
  while(ticks - ticks0 < n){
    if(proc->killed){
      release(&tickslock);
      return -1;
    }
    sleep(&ticks, &tickslock);
  }
  release(&tickslock);
  return 0;
}

// return how many clock tick interrupts have occurred
// since boot.
int
sys_uptime(void)
{
  uint xticks;
  
  acquire(&tickslock);
  xticks = ticks;
  release(&tickslock);
  return xticks;
}

int 
sys_clone(void)
{
  void (*fcn)(void*);
  void *arg;
  void *stack;

  if (argptr(0, (char**)&fcn, 4) < 0) return -1;
  if (argptr(1, (char**)&arg, 4) < 0) return -1;
  if (argptr(2, (char**)&stack, 2 * PGSIZE < 0)) return -1;

  return clone(fcn, arg, stack);
}

int 
sys_join(void)
{
  int pid;
  if (argint(0, &pid) < 0)
    return -1;
  return join(pid);
}
int
sys_cvwait(void)
{
  void *cv, *temp;

  if(argptr(0, (void *)&cv, sizeof(void *)) < 0)
    return -1;
  if(argptr(1, (void *)&temp, sizeof(void *)) < 0)
    return -1;

  struct lock_t *lock = (struct lock_t*)temp;
        
  cvwait(cv, lock);
        
  return 0;
}
int
sys_cvsignal(void)
{
  void *cv;
    
  if(argptr(0, (void *)&cv, sizeof(void *)) < 0)
    return -1;

  cvsignal(cv);

  return 0;
}
<<<<<<< HEAD
*/
int sys_listproc(void)
{
  listproc();
  return 0;
}
=======
>>>>>>> c78075218f76b73a88960cd69d0308590f26a2ba
