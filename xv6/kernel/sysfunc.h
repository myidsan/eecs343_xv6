#ifndef _SYSFUNC_H_
#define _SYSFUNC_H_

// System call handlers
int sys_chdir(void);
int sys_close(void);
int sys_dup(void);
int sys_exec(void);
int sys_exit(void);
int sys_fork(void);
int sys_fstat(void);
int sys_getpid(void);
int sys_kill(void);
int sys_link(void);
int sys_mkdir(void);
int sys_mknod(void);
int sys_open(void);
int sys_pipe(void);
int sys_read(void);
int sys_sbrk(void);
int sys_sleep(void);
int sys_unlink(void);
int sys_wait(void);
int sys_write(void);
int sys_uptime(void);
int sys_clone(void);
int sys_join(void);
<<<<<<< HEAD
int sys_listproc(void);

=======
int sys_cvwait(void);
int sys_cvsignal(void);
>>>>>>> c78075218f76b73a88960cd69d0308590f26a2ba

#endif // _SYSFUNC_H_
