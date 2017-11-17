/*calls getFileTag before tagFile, should return -1. calls tagFile with valid input, should create and return 1. calls getFileTag with invalid length, should return correct length. calls removeFileTag to remove tag, should return 1. calls getFileTag, should return -1.*/
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
   char buf[7];
   int before_created = getFileTag(fd, key, buf, len);
   assert(before_created < 0);
   printf(1, "stuck here?\n");
   int res = tagFile(fd, key, val, len);
   printf(1, "stuck here? later\n");
   int i;
   assert(res > 0);
   for(i = 0; i < 7; i++){
      buf[i] = ' ';
   }

   // not large enough buffer length, return needed without write
   int valueLength = getFileTag(fd, key, buf, 5);
   assert(valueLength == 7);
   for(i = 0; i < valueLength; i++) {
      char v_actual = buf[i];
      assert(v_actual == ' ');
   }

   int correctLength = getFileTag(fd, key, buf, valueLength);
   assert(correctLength == 7);

   for(i = 0; i < len; i++) {
     char v_actual = buf[i];
     char v_expected = val[i];
     assert(v_actual == v_expected);
   }

   char* new_val = "wowowwowowwow";
   int len2 = 13;
   char buf3[13];
   printf(1, "fd is before tagFile extend: %d\n", fd);
   int res1 = tagFile(fd, key, new_val, len2);
   printf(1, "fd is after tagFile extend: %d\n", fd);
   int j;
   assert(res1 > 0);
   for(j = 0; j < 13; j++) {
      buf3[j] = ' ';
   }
   printf(1, "fd is fucked: %d\n", fd);
   int new_correctLength = getFileTag(fd, key, buf3, 13);
   printf(1, "new length: %d\n", new_correctLength);
   assert(new_correctLength == 13);
   for(j = 0; j < len2; j++) {
     char v_actual = buf3[j];
     char v_expected = new_val[j];
     assert(v_actual == v_expected);
   }

   int res2 = removeFileTag(fd, key);
   assert(res2 > 0);
   valueLength = getFileTag(fd, key, buf3, 13);
   assert(valueLength == -1);

   close(fd);
   printf(1, "TEST PASSED\n");
   exit();
}
