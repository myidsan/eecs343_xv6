#include "types.h"
#include "stat.h"
#include "user.h"

int main(int argc, char *argv[])
{
  if(argc > 1){
    printf(1, "Error. tag(s) is(are) not supported in this version. Try just ps.\n");
    exit();
  }
  enum procstate { UNUSED, EMBRYO, SLEEPING, RUNNABLE, RUNNING, ZOMBIE };
  struct ProcessInfo ptable[65];
  struct ProcessInfo *head;
  int flag;
  
  flag = getprocs(65*sizeof(struct ProcessInfo), &ptable);
  if( flag != 0){
    printf(1, "error getting table");
    exit();
  }
  //printf(1, "%x\n", &head);
  head = &ptable[0];
  printf(1,"%d  %d", head -> pid, -1);
    switch(head -> state){
      case UNUSED:
        printf(1,"  %s", "UNUSED");
        break;
      case EMBRYO:
        printf(1,"  %s", "EMBRYO");
        break;
      case SLEEPING:
        printf(1,"  %s", "SLEEPING");
        break;
      case RUNNABLE:
        printf(1,"  %s", "RUNNABLE");
        break;
      case RUNNING:
        printf(1,"  %s", "RUNNABLE");
        break;
      case ZOMBIE:
        printf(1,"  %s", "RUNNABLE");
        break;
    }
    printf(1,"  %d  %s \n", head -> sz, head -> name);
  head++;
  while(head != &ptable[65] && head -> state != UNUSED){
    printf(1,"%d  %d", head -> pid, head -> ppid);
    switch(head -> state){
      case UNUSED:
        printf(1,"  %s", "UNUSED");
        break;
      case EMBRYO:
        printf(1,"  %s", "EMBRYO");
        break;
      case SLEEPING:
        printf(1,"  %s", "SLEEPING");
        break;
      case RUNNABLE:
        printf(1,"  %s", "RUNNABLE");
        break;
      case RUNNING:
        printf(1,"  %s", "RUNNABLE");
        break;
      case ZOMBIE:
        printf(1,"  %s", "RUNNABLE");
        break;
    } 
    printf(1,"  %d  %s \n", head -> sz, head -> name);
    head++;
  }
  exit();
}
