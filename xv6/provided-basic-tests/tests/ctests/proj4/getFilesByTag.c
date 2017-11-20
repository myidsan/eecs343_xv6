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
   /*
   struct Key keys[16];
   int number = getAllTags(fd, keys, 16);
   printf(1, "number of tags: %d\n", number);
   assert(number == 3);
   */
   int fd2 = open("mkdir", O_RDWR);
   res = tagFile(fd2, key2, val, len);     
   assert(res > 0);

   // opened mkdir and ls, but mkdir has wrong key.
   char result[10];
   int files = getFilesByTag(key, val, 7, result, 10);     
   assert(files == 1); 

   assert(result[0] == 'l');
   assert(result[1] == 's');
   assert(!result[2]);


   printf(1, "%s\n", result);
   res = tagFile(fd2, key, val2, len2);     
   assert(res > 0);

   // opened mkdir and ls but mkdir has wrong val.
   files = getFilesByTag(key, val, 7, result, 10);     
   assert(files == 1); 

   assert(result[0] == 'l');
   assert(result[1] == 's');
   assert(!result[2]);

   res = tagFile(fd2, key, val, len);     
   assert(res > 0);

   // opened mkdir and ls, both have key and val
   char results[10];
   files = getFilesByTag(key, val, 7, results, 10);     
   printf(1, "files for second call: %d\n", files);
   assert(files == 2); 
   printf(1, "results is : %s\n\n", results);
   assert(results[0] == 'm');
   assert(results[1] == 'k');
   assert(results[2] == 'd');
   assert(results[3] == 'i');
   assert(results[4] == 'r');
   assert(!results[5]);
   assert(results[6] == 'l');
   assert(results[7] == 's');
   assert(!results[8]);
   
   close(fd);
   close(fd2);
   char results2[10];
   char not_enough[6];
   files = getFilesByTag(key, val, 7, not_enough, 6);     
   assert(files == -1);
   files = getFilesByTag(key, val, 7, results2, 10);     
   printf(1, "files for second call: %d\n", files);
   assert(files == 2); 
   /*
   assert(results[0] == 'm');
   assert(results[1] == 'k');
   assert(results[2] == 'd');
   assert(results[3] == 'i');
   assert(results[4] == 'r');
   assert(!results[5]);
   assert(results[6] == 'l');
   assert(results[7] == 's');
   assert(!results[8]);
   */

   printf(1, "TEST PASSED\n");
   exit();
}
