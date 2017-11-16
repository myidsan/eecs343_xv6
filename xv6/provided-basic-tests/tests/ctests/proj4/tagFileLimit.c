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
   char* key2 = "typo";
   char* val = "utility";
   char* val2 = "utilot";
   int len = 7;
   int res = tagFile(fd, key, val, len);
   assert(res > 0);
   int res2 = tagFile(fd, key2, val2, len);
   assert(res2 > 0);

   char buf[7];
   int valueLength = getFileTag(fd, key, buf, 7);
   assert(valueLength == len);

   char buf2[7];
   int valueLength2 = getFileTag(fd, key2, buf2, 6);
   assert(valueLength2 == 6);
   close(fd);

   int i;
   for(i = 0; i < len; i++){
      char v_actual = buf[i];
      char v_expected = val[i];
      assert(v_actual == v_expected);
      v_actual = buf2[i];
      v_expected = val2[i];
      printf(1, "%c", buf2[i]);
      //assert(v_actual == v_expected);
   }

   printf(1, "TEST PASSED\n");
   exit();
}
