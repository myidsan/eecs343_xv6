#include "types.h"
#include "stat.h"
#include "user.h"

int main() {
  int result = getprocs(ptable);
  printf("%d\n", result);
  return 0;
}
