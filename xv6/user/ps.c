#include "types.h"
#include "stat.h"
#include "user.h"

int main() {

  struct ProcessInfo* proc_info_table;
  struct ProcessInfo head;
  proc_info_table = &head;

  int result = getprocs(proc_info_table);
  //printf(1, "%x\n", &head);
  printf(1, "%d\n", result);
  exit();
}
