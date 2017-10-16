#include "types.h"
#include "stat.h"
#include "user.h"
int main() {
  int count = shmem_count(3);
  void* access = shmem_access(3);
  
  if( count == 3) {
    printf(1, "\nshmem_count working properly\n");
  }
  printf(1, "%x", access);

  exit();
} 
