#include "types.h"
#include "stat.h"
#include "user.h"
int main() {
  int count = shmem_count(3);

  if(count == 0) {
    printf(1, "\ninitializtion working properly\n");
  } else {
    printf(1, "\n\n\ninitialization ERROR \n");
  }
  void* access = shmem_access(3);
  access = access + 0;
  count = shmem_count(3);
  
  if(count == 1) {
    printf(1, "\nshmem_count, shmem_access is able to access global array properly\n");
  }

  exit();
} 
