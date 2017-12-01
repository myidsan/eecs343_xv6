/* call tagFile to tag a file.  Close the file and call removeFileTag, should return -1.  Open the file 
   in read-only mode and call removeFileTag, should return -1.  Finally open the file properly and 
   remove the tag. */
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

#define assertEquals(expected, actual) if (expected == actual) {} else { \
   printf(1, "%s: %d ", __FILE__, __LINE__); \
   printf(1, "assert failed (%s == %s)\n", # expected, # actual); \
   printf(1, "assert failed (expected: %d)\n", expected); \
   printf(1, "assert failed (actual: %d)\n", actual); \
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

   close(fd);

   int res2 = removeFileTag(fd, key);  // fd is closed
   assertEquals(-1, res2);

   fd = open("ls", O_RDONLY);
   res2 = removeFileTag(fd, key);  // fd is read only
   assertEquals(-1, res2);

   fd = open("ls", O_RDWR);
   res2 = removeFileTag(fd, key);  // OK
   assert(res2 > 0);
   
   printf(1, "TEST PASSED\n");
   exit();
}
