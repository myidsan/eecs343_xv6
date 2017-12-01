/* call tagFile to tag a file.  Call getFileTag to read the tag of that file. */
#include "types.h"
#include "user.h"

#define O_RDONLY  0x000
#define O_WRONLY  0x001
#define O_RDWR    0x002
#define O_CREATE  0x200

#undef NULL
#define NULL ((void*)0)

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
   char* key = "type";
   char* val = "utility";
   int len = 7;
   int res = tagFile(fd, key, val, len);
   assert(res > 0);

   close(fd);
   
   fd = open("ls", O_RDWR);
   char buf2[7];
   int valueLength = getFileTag(fd, key, buf2, 7);
   printf(1, "getFileTag response after reopen: %d\n", valueLength);
   assert(valueLength == len);
   int i;
   for(i = 0; i < len; i++){
     char v_actual = buf2[i];
     char v_expected = val[i];
     printf(1, "%c", v_actual);
     assert(v_actual == v_expected);
   }
   printf(1, "\n");

   printf(1, "TEST PASSED\n");
   exit();
}
