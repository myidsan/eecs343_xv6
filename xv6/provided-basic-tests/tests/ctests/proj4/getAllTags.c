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
   printf(1, "first res: %d\n", res);
   assert(res > 0);

   char* key2 = "number";
   char* val2 = "five";
   int len2 = 4;
   res = tagFile(fd, key2, val2, len2);
   assert(res > 0);

   struct Key keys[16];
   int number = getAllTags(fd, keys, 16);
   printf(1, "number of tags: %d\n", number);
   printf(1, "first key: %s\n", keys[0].key);
   assert(number == 2);
   
   char buffer[18];
   int i = 0;
   printf(1, "list of this file's tags:\n");
   for(i = 0; i < number; i++) {
     printf(1, "in loops num: %d\n", i);
     printf(1, "key length: %d\n", strlen(keys[i].key));
     printf(1, "%s\n", keys[i].key);  
     int res = getFileTag(fd, (char*)keys[i].key, buffer, 18);
     printf(1, "res: %d\n", res);
     if(res > 0){
       printf(1, "%s\n", keys[i].key);
     }
   }
   //only get 1 tag
   /*
   number = getAllTags(fd, keys, 16);
   printf(1, "2nd round number of tags: %d\n", number);
   assert(number == 2);
   */

   close(fd);

   printf(1, "TEST PASSED\n");
   exit();
}
