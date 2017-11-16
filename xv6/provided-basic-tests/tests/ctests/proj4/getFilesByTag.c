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
   char* key = "type";
   char* val = "utility";
   int len = 7;
   int res = tagFile(fd, key, val, len);
   assert(res > 0);

   char* key2 = "number";
   char* val2 = "five";
   int len2 = 4;
   res = tagFile(fd, key2, val2, len2);
   assert(res > 0);
   
   key2 = "character";
   val2 = "alphabet";
   len2 = 8;
   res = tagFile(fd, key2, val2, len2);
   
   struct Key keys[16];
   int number = getAllTags(fd, keys, 16);
   printf(1, "number of tags: %d\n", number);
   assert(number == 3);

   close(fd);

   int fd2 = open("ps", O_RDWR);
   res = tagFile(fd2, key, val, lens);     
   assert(res > 0);
   close(fd);

   char result[10];
   int files = getFilesByTag(key, val, 8, result, 20);     
   assert(files == 2); 
   assert(result[0] == 'l');
   assert(result[1] == 's');
   assert(result[2] == NULL);
   assert(result[3] == 'p');
   assert(result[4] == 's');

   printf(1, "TEST PASSED\n");
   exit();
}
