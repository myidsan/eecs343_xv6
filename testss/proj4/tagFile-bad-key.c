/* call tagFile with bad key argument (too short and too long).  tagFile should return -1. */
#include "types.h"
#include "user.h"

#undef NULL
#define NULL ((void*)0)

#define O_RDONLY  0x000
#define O_WRONLY  0x001
#define O_RDWR    0x002
#define O_CREATE  0x200

int ppid;
volatile int global = 1;

#define assert(x) if (x) {} else { \
   printf(1, "%s: %d ", __FILE__, __LINE__); \
   printf(1, "assert failed (%s)\n", # x); \
   printf(1, "TEST FAILED\n"); \
   kill(ppid); \
   exit(); \
}

int
main(int argc, char *argv[])
{
   ppid = getpid();
   int fd = open("ls", O_RDWR);
   printf(1, "fd of ls: %d\n", fd);
   char* key_short = "";
   char* val = "some_val";
   int len = 8;
   int res = tagFile(fd, key_short, val, len);
   assert(res == -1);

   char* key_long = "1234567890";
   res = tagFile(fd, key_long, val, len);
   assert(res == -1);

   printf(1, "TEST PASSED\n");
   exit();
}