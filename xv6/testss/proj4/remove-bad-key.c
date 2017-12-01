/* call tagFile to tag a file.  Call removeFileTag to remove the tag.  Call removeFileTag again 
   on the same key, should return -1 because it's already removed.  Call it again with keys that never 
   existed, should return -1. */
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

   int res2 = removeFileTag(fd, key);
   assert(res2 > 0);
   
   res2 = removeFileTag(fd, key);  // remove a key that was already removed
   assertEquals(-1, res2);

   char* badKey = "type1";
   res2 = removeFileTag(fd, badKey);  // remove a key that was already removed
   assertEquals(-1, res2);

   char* badKey2 = "typ";
   res2 = removeFileTag(fd, badKey2);  // remove a key that was already removed
   assertEquals(-1, res2);

   close(fd);
   printf(1, "TEST PASSED\n");
   exit();
}
