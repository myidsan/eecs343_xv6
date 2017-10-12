#include "types.h"
#include "stat.h"
#include "user.h"
int main(){
  int* null_pointer = 0;
  int x = (*null_pointer);
  printf(1, "%d", x);
  printf(1, "something wrong with your part 1");
  exit();
}


