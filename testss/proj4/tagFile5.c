/* call tagFile to tag a file, then call getFileTag to read the tag of that file. Then call tagFile 
   to change the tag (same key), then call getFileTag to verify that the tag has changed. */

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

#define assertFirstGreaterThanSecond(first, second) if (first > second) {} else { \
   printf(1, "%s: %d ", __FILE__, __LINE__); \
   printf(1, "assert failed (%s > %s)\n", # first, # second); \
   printf(1, "assert failed (%d is not greater than %d)\n", first, second); \
   printf(1, "TEST FAILED\n"); \
   kill(ppid); \
   exit(); \
}

#define assertCharEquals(expected, actual, i) if (expected[i] == actual[i]) {} else { \
   printf(1, "%s: %d ", __FILE__, __LINE__); \
   printf(1, "assert failed (%s == %s)\n", # expected, # actual); \
   printf(1, "assert failed (expected: %s)\n", expected); \
   printf(1, "assert failed (actual: %s)\n", actual); \
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

void
checkStringsAreEqual(char* expected, char* actual, int expectedLength)
{
   int i;
   for(i = 0; i < expectedLength; i++){
      assertCharEquals(expected, actual, i);
   }
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

   fd = open("ls", O_RDONLY);
   char buf[7];
   int valueLength = getFileTag(fd, key, buf, 7);
   assertEquals(len, valueLength);

   close(fd);

   checkStringsAreEqual(val, buf, len);

   // change tag
   fd = open("ls", O_RDWR);
   char* newVal = "other12345";
   int newLen = 10;
   res = tagFile(fd, key, newVal, newLen);
   assertFirstGreaterThanSecond(res, 0);
   close(fd);

   fd = open("ls", O_RDONLY);
   char newBuf[10];
   valueLength = getFileTag(fd, key, newBuf, 10);
   assertEquals(newLen, valueLength);

   close(fd);

   checkStringsAreEqual(newVal, newBuf, newLen);

   printf(1, "TEST PASSED\n");
   exit();
}
