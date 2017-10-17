#include "types.h"
#include "x86.h"
#include "defs.h"
#include "param.h"
#include "mmu.h"
#include "proc.h"
#include "sysfunc.h"
#include "ProcessInfo.h"

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
// write int sys_getprocs() it will need to populate getprocs table for all processes.
struct proc * getprocs(void);
int 
sys_getprocs(void)
{ 
  int table_size;
  char *traverse;
  char *table_start;
  struct proc *ptable;
  
  if(argint(0, &table_size) < 0) {
    return -1;
  }
  if(argptr(1, &table_start, table_size) < 0) {
    return -1;
  }
  
  traverse = table_start;
  ptable = getprocs();

  while(table_start + table_size > traverse  && ptable -> state != UNUSED) {
    *(int *)traverse = ptable -> pid;
    traverse += 4;
    *(int *)traverse = ptable -> parent -> pid;
    traverse += 4;
    *(int *)traverse = ptable -> state;
    traverse += 4;
    *(int *)traverse = ptable -> sz; 
    traverse += 4;
    memmove(traverse, ptable -> name, 16);
    traverse += 16;
    ptable++;  
  }
  
  return 0;
} 

int
sys_shmem_access(void)
{
  int page_number;
  if (argint(0, &page_number) < 0) {
    return -1;
   }
  argint(0, &page_number);
  return (int)shmem_access(page_number); 
}

int
sys_shmem_count(void)
{
  int page_number = 0;
  if (argint(0, &page_number < 0)) {
    return -1;
  }
  argint(0, &page_number);
  if(page_number < 0 || page_number > 3)
    return -1;
  return shmem_count(page_number);
}
