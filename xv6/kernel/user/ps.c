#include <proc.c>
#include <syscall.c>

int main() {
  int result = getprocs(ptable);
  printf("%d\n", result);
  return 0;
}
