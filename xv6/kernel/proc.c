#include "types.h"
#include "defs.h"
#include "param.h"
#include "mmu.h"
#include "x86.h"
#include "proc.h"
#include "spinlock.h"

struct {
  struct spinlock lock;
  struct proc proc[NPROC];
} ptable;

static struct proc *initproc;

int nextpid = 1;
extern void forkret(void);
extern void trapret(void);

static void wakeup1(void *chan);

struct spinlock growproclock;

void
pinit(void)
{
  initlock(&ptable.lock, "ptable");
}

// Look in the process table for an UNUSED proc.
// If found, change state to EMBRYO and initialize
// state required to run in the kernel.
// Otherwise return 0.
static struct proc*
allocproc(void)
{
  struct proc *p;
  char *sp;

  acquire(&ptable.lock);
  for(p = ptable.proc; p < &ptable.proc[NPROC]; p++)
    if(p->state == UNUSED)
      goto found;
  release(&ptable.lock);
  return 0;

found:
  p->state = EMBRYO;
  p->pid = nextpid++;
  release(&ptable.lock);
  // init lock - requirement 12
  // making a new process
  p->isThread = 0;
  p->parent = proc;
  // trap happening here
  initlock(&p->lock, "proc_lock");

  // Allocate kernel stack if possible.
  if((p->kstack = kalloc()) == 0){
    p->state = UNUSED;
    return 0;
  }
  sp = p->kstack + KSTACKSIZE;
  //cprintf("kalloc success\n");

  // Leave room for trap frame.
  sp -= sizeof *p->tf;
  p->tf = (struct trapframe*)sp;
  
  // Set up new context to start executing at forkret,
  // which returns to trapret.
  sp -= 4;
  *(uint*)sp = (uint)trapret;

  sp -= sizeof *p->context;
  p->context = (struct context*)sp;
  memset(p->context, 0, sizeof *p->context);
  p->context->eip = (uint)forkret;

  //cprintf("before return success\n");
  return p;
}

// Set up first user process.
void
userinit(void)
{
  struct proc *p;
  extern char _binary_initcode_start[], _binary_initcode_size[];
  
  p = allocproc();
  acquire(&ptable.lock);
  initproc = p;
  if((p->pgdir = setupkvm()) == 0)
    panic("userinit: out of memory?");
  inituvm(p->pgdir, _binary_initcode_start, (int)_binary_initcode_size);
  p->sz = PGSIZE;
  memset(p->tf, 0, sizeof(*p->tf));
  p->tf->cs = (SEG_UCODE << 3) | DPL_USER;
  p->tf->ds = (SEG_UDATA << 3) | DPL_USER;
  p->tf->es = p->tf->ds;
  p->tf->ss = p->tf->ds;
  p->tf->eflags = FL_IF;
  p->tf->esp = PGSIZE;
  p->tf->eip = 0;  // beginning of initcode.S

  safestrcpy(p->name, "initcode", sizeof(p->name));
  p->cwd = namei("/");

  p->state = RUNNABLE;
  release(&ptable.lock);
}

// Grow current process's memory by n bytes.
// Return 0 on success, -1 on failure.
// requirement 12
int
growproc(int n)
{
  /*
  uint sz;
  // acquire(&ptable.lock);
  struct proc *p;
  if (proc->isThread == 0) {
    acquire(&proc->lock);
  } else {
	p = proc;
	while (p->isThread == 1) {
	  p->parent = p;
	}
	proc->parent = p; 
    //for(p = proc; p->isThread == 1; p->parent) {
	//  proc->parent = p;
	//} // finding the process as parent
	acquire(&proc->parent->lock);
  }

  sz = proc->sz;
  if(n > 0){
    if((sz = allocuvm(proc->pgdir, sz, sz + n)) == 0) {
	  if(proc->isThread == 0) release(&proc->lock);
	  else release(&proc->parent->lock);
	  return -1;
	}
  } else if(n < 0){
    if((sz = deallocuvm(proc->pgdir, sz, sz + n)) == 0) {
	  if(proc->isThread == 0) release(&proc->lock);
	  else release(&proc->parent->lock);
      return -1;
	}
  }
  proc->sz = sz;
  switchuvm(proc);
  if(proc->isThread == 0) release(&proc->lock);
  else release(&proc->parent->lock);
  return 0;
  */
   uint sz;
   struct proc *p;

   pde_t* pgdirtemp = proc->pgdir;
   sz = proc->sz;
   if(n > 0){
     if((sz = allocuvm(proc->pgdir, sz, sz + n)) == 0)
       return -1;
   } else if(n < 0){
     if((sz = deallocuvm(proc->pgdir, sz, sz + n)) == 0)
       return -1;
   }
   proc->sz = sz;
   switchuvm(proc);

   acquire(&ptable.lock);
   for(p = ptable.proc; p < &ptable.proc[NPROC]; p++){
     if(p->pgdir == pgdirtemp){
       p->sz = sz;
       acquire(&growproclock);
       switchuvm(p);
       release(&growproclock);
     }
   }
   release(&ptable.lock);
   return 0;
}

// Create a new process copying p as the parent.
// Sets up stack to return as if from system call.
// Caller must set state of returned proc to RUNNABLE.
int
fork(void)
{
  int i, pid;
  struct proc *np;

  // Allocate process.
  if((np = allocproc()) == 0)
    return -1;

  // Copy process state from p.
  if((np->pgdir = copyuvm(proc->pgdir, proc->sz)) == 0){
    kfree(np->kstack);
    np->kstack = 0;
    np->state = UNUSED;
    return -1;
  }
  np->sz = proc->sz;
  np->parent = proc;
  *np->tf = *proc->tf;

  // Clear %eax so that fork returns 0 in the child.
  np->tf->eax = 0;

  for(i = 0; i < NOFILE; i++)
    if(proc->ofile[i])
      np->ofile[i] = filedup(proc->ofile[i]);
  np->cwd = idup(proc->cwd);
 
  pid = np->pid;
  np->state = RUNNABLE;
  safestrcpy(np->name, proc->name, sizeof(proc->name));
  return pid;
}


// Exit the current process.  Does not return.
// An exited process remains in the zombie state
// until its parent calls wait() to find out it exited.
void
exit(void)
{
  struct proc *p;
  int fd;

  if(proc == initproc)
    panic("init exiting");

  // Close all open files.
  for(fd = 0; fd < NOFILE; fd++){
    if(proc->ofile[fd]){
      fileclose(proc->ofile[fd]);
      proc->ofile[fd] = 0;
    }
  }

  iput(proc->cwd);
  proc->cwd = 0;

  acquire(&ptable.lock);

  // Parent might be sleeping in wait().
  wakeup1(proc->parent);

  // Pass abandoned children to init.
  for(p = ptable.proc; p < &ptable.proc[NPROC]; p++){
	  if(p->parent == proc) { // check if it is a thread
      if(p->isThread == 1) {
        kfree(p->kstack);
        p->kstack = 0;
        p->ustack = UNUSED;
        p->state = ZOMBIE;
		p->killed = 3;
      }
      else {
        p->parent = initproc;
        if(p->state == ZOMBIE)
          wakeup1(initproc);
      }
    }
  }

  // Jump into the scheduler, never to return.
  proc->state = ZOMBIE;
  sched();
  panic("zombie exit");
}

// Wait for a child process to exit and return its pid.
// Return -1 if this process has no children.
int
wait(void)
{
  struct proc *p;
  int havekids, pid;

  acquire(&ptable.lock);
  for(;;){
    // Scan through table looking for zombie children.
    havekids = 0;
    for(p = ptable.proc; p < &ptable.proc[NPROC]; p++){
      if(p->isThread == 1 || p->parent != proc)
        continue;
      havekids = 1;
      if(p->state == ZOMBIE){
        // Found one.
        pid = p->pid;
        kfree(p->kstack);
        p->kstack = 0;
        freevm(p->pgdir);
        p->state = UNUSED;
        p->pid = 0;
        p->parent = 0;
        p->name[0] = 0;
        p->killed = 0;
        release(&ptable.lock);
        return pid;
      }
    }

    // No point waiting if we don't have any children.
    if(!havekids || proc->killed){
      release(&ptable.lock);
      return -1;
    }

    // Wait for children to exit.  (See wakeup1 call in proc_exit.)
    sleep(proc, &ptable.lock);  //DOC: wait-sleep
  }
}

// Per-CPU process scheduler.
// Each CPU calls scheduler() after setting itself up.
// Scheduler never returns.  It loops, doing:
//  - choose a process to run
//  - swtch to start running that process
//  - eventually that process transfers control
//      via swtch back to the scheduler.
void
scheduler(void)
{
  struct proc *p;

  for(;;){
    // Enable interrupts on this processor.
    sti();

    // Loop over process table looking for process to run.
    acquire(&ptable.lock);
    for(p = ptable.proc; p < &ptable.proc[NPROC]; p++){
      if(p->state != RUNNABLE)
        continue;

      // Switch to chosen process.  It is the process's job
      // to release ptable.lock and then reacquire it
      // before jumping back to us.
      proc = p;
      switchuvm(p);
      p->state = RUNNING;
      swtch(&cpu->scheduler, proc->context);
      switchkvm();

      // Process is done running for now.
      // It should have changed its p->state before coming back.
      proc = 0;
    }
    release(&ptable.lock);

  }
}

// Enter scheduler.  Must hold only ptable.lock
// and have changed proc->state.
void
sched(void)
{
  int intena;

  if(!holding(&ptable.lock))
    panic("sched ptable.lock");
  if(cpu->ncli != 1)
    panic("sched locks");
  if(proc->state == RUNNING)
    panic("sched running");
  if(readeflags()&FL_IF)
    panic("sched interruptible");
  intena = cpu->intena;
  swtch(&proc->context, cpu->scheduler);
  cpu->intena = intena;
}

// Give up the CPU for one scheduling round.
void
yield(void)
{
  acquire(&ptable.lock);  //DOC: yieldlock
  proc->state = RUNNABLE;
  sched();
  release(&ptable.lock);
}

// A fork child's very first scheduling by scheduler()
// will swtch here.  "Return" to user space.
void
forkret(void)
{
  // Still holding ptable.lock from scheduler.
  release(&ptable.lock);
  
  // Return to "caller", actually trapret (see allocproc).
}

// Atomically release lock and sleep on chan.
// Reacquires lock when awakened.
void
sleep(void *chan, struct spinlock *lk)
{
  if(proc == 0)
    panic("sleep");

  if(lk == 0)
    panic("sleep without lk");

  // Must acquire ptable.lock in order to
  // change p->state and then call sched.
  // Once we hold ptable.lock, we can be
  // guaranteed that we won't miss any wakeup
  // (wakeup runs with ptable.lock locked),
  // so it's okay to release lk.
  if(lk != &ptable.lock){  //DOC: sleeplock0
    acquire(&ptable.lock);  //DOC: sleeplock1
    release(lk);
  }

  // Go to sleep.
  proc->chan = chan;
  proc->state = SLEEPING;
  sched();

  // Tidy up.
  proc->chan = 0;

  // Reacquire original lock.
  if(lk != &ptable.lock){  //DOC: sleeplock2
    release(&ptable.lock);
    acquire(lk);
  }
}

// Wake up all processes sleeping on chan.
// The ptable lock must be held.
static void
wakeup1(void *chan)
{
  struct proc *p;

  for(p = ptable.proc; p < &ptable.proc[NPROC]; p++)
    if(p->state == SLEEPING && p->chan == chan)
      p->state = RUNNABLE;
}

// Wake up all processes sleeping on chan.
void
wakeup(void *chan)
{
  acquire(&ptable.lock);
  wakeup1(chan);
  release(&ptable.lock);
}

// Kill the process with the given pid.
// Process won't exit until it returns
// to user space (see trap in trap.c).
int
kill(int pid)
{
  struct proc *p;

  acquire(&ptable.lock);
  for(p = ptable.proc; p < &ptable.proc[NPROC]; p++){
    if(p->pid == pid){
      p->killed = 1;
      // Wake process from sleep if necessary.
      if(p->state == SLEEPING)
        p->state = RUNNABLE;
      release(&ptable.lock);
      return 0;
    }
  }
  release(&ptable.lock);
  return -1;
}

// Print a process listing to console.  For debugging.
// Runs when user types ^P on console.
// No lock to avoid wedging a stuck machine further.
void
procdump(void)
{
  static char *states[] = {
  [UNUSED]    "unused",
  [EMBRYO]    "embryo",
  [SLEEPING]  "sleep ",
  [RUNNABLE]  "runble",
  [RUNNING]   "run   ",
  [ZOMBIE]    "zombie"
  };
  int i;
  struct proc *p;
  char *state;
  uint pc[10];
  
  for(p = ptable.proc; p < &ptable.proc[NPROC]; p++){
    if(p->state == UNUSED)
      continue;
    if(p->state >= 0 && p->state < NELEM(states) && states[p->state])
      state = states[p->state];
    else
      state = "???";
    cprintf("%d %s %s %d", p->pid, state, p->name, p->killed);
    if(p->state == SLEEPING){
      getcallerpcs((uint*)p->context->ebp+2, pc);
      for(i=0; i<10 && pc[i] != 0; i++)
        cprintf(" %p", pc[i]);
    }
    cprintf("\n");
  }
}

int 
listproc(void)
{
  procdump();
  return 0;
}

int
clone(void(*fcn)(void*), void*arg, void* stack){
  int i, tid;
  struct proc *thread, *p;
  void *retaddr, *myarg;
  uint load[2];

  // requirement 8
  if ((thread = allocproc()) == 0) {
    return -1;
  }

  // check page allignment
  if((uint)stack%PGSIZE != 0){
    return -1;
  }

  if((uint)stack%PGSIZE != 0){
    return -1;
  }
  if((proc->sz - (uint)stack) < PGSIZE){
    return -1;
  }
  
  p = proc;
  while (p->isThread == 1) {
    p = p->parent;
  }

  thread->isThread = 1; // requirement 11;
  thread->pgdir = p->pgdir; // requirement 2
  thread->sz = p->sz;
  thread->parent = p;
  thread->killed = 0; // non-zero have been killed
  thread->ustack = (char*)stack; // requirement 5
  *(thread->tf) = *(p->tf); // trap frame holds register values

  // find the top parent 
  // requirement 9
  load[0] = 0xffffffff;
  load[1] = (uint)arg;
  

  // requirement 7
  retaddr = stack + PGSIZE - 2 * sizeof(void*);
  *(uint*)retaddr = 0xFFFFFFFF;

  // requirement 6
  myarg = stack + PGSIZE - sizeof(void*); 
  *(uint*)myarg = (uint)arg;
  // calling convention is to push esp to ebp, as mentioned in the wikipedia of calling conventions of os dev
  thread->tf->esp = (uint)(stack);
  memmove((void*)thread->tf->esp, stack, PGSIZE);
  thread->tf->esp += PGSIZE - 2 * 4;
  copyout(thread->pgdir, thread->tf->esp, load, 8);
  thread->tf->ebp = thread->tf->esp;
  thread->tf->eip = (uint)fcn; // requirement 4
  thread->tf->eax = 0;
  // requirement 3
  // taken from fork, nofile == number of files
  for(i = 0; i < NOFILE; i++)
    if(p->ofile[i])
      thread->ofile[i] = filedup(p->ofile[i]);
  thread->cwd = idup(p->cwd);


  tid = thread->pid;

  acquire(&ptable.lock);
  thread->state = RUNNABLE;
  release(&ptable.lock);
  safestrcpy(p->name, thread->name, sizeof(p->name));
 
  return tid;
}

int
join(int pid)
{
  if (proc->pid == pid)
    return -1;
 
  cprintf("trying to find %d in process %d.\n", pid, proc->pid);

  int havekids;
  int found = 0;
  struct proc *p;
  

  acquire(&ptable.lock);
  for(;;){
    havekids = 0;
    for(p = ptable.proc; p < &ptable.proc[NPROC]; p++) {
      if(p->parent->pid != proc->pid || p -> isThread != 1)
        continue;
      cprintf("thread's pid is(in kernel): %d while finding %d\n", p->pid, pid);
      cprintf("thread %d parent pid is %d\n", p->pid, p->parent->pid);
      cprintf("state of thread pid %d is %d\n", p->pid, p->state);
      if(p->pid == pid) {
        found = 1;
      }

      if(p->state == ZOMBIE && pid == p->pid) {
        cprintf("releasing pid %d\n", pid);
        havekids = 1;
        kfree(p->kstack);
        p -> state = UNUSED;
        p -> pid = 0;
        p -> parent = 0;
        p -> name[0] = 0;
        p -> killed = 0;
        release(&ptable.lock);
        return pid;
      }
    }
    cprintf("%d is whether it found a thread or not\n", havekids);
    if((havekids == 0 && found != 1) || proc -> killed) {
      release(&ptable.lock);
      cprintf("couldn't find %d. exiting with -1\n", pid);
      return -1;
    }
    sleep(proc, &ptable.lock);
  }
}

void
cvwait(void *chan, struct lock_t *lock)
{
  acquire(&ptable.lock);  
  xchg(&lock->locked, 0); //unlock
     
  //proc->lock = lock;//store the user lock
  proc->chan = chan;
  proc->state = SLEEPING;
  sched();
  proc->chan = 0;
  release(&ptable.lock);
  while(xchg(&lock->locked, 1)!=0);//lock
}

void
cvsignal(void *chan)
{
  struct proc *p;
  for(p = ptable.proc; p < &ptable.proc[NPROC]; p++)
    if(p->state == SLEEPING && p->chan == chan) {
      acquire(&ptable.lock);
      p->state = RUNNABLE;
      release(&ptable.lock);
      break;
    }
}
