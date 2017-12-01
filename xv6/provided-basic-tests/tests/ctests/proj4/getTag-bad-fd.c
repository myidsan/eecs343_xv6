/* call tagFile to tag a file.  Call getFileTag to read the tag of that file. Close the file and try 
   to call getFileTag again.  Open the file in write only mode and call getFileTag again. Finally, 
   open the file properly and get the tag. */
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

#define assertCharEquals(expected, actual, i) if (expected[i] == actual[i]) {} else { \
   printf(1, "%s: %d ", __FILE__, __LINE__); \
   printf(1, "assert failed (%s == %s)\n", # expected, # actual); \
   printf(1, "assert failed (expected: %s)\n", expected); \
   printf(1, "assert failed (actual: %s)\n", actual); \
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

   char buf[7];
   int valueLength = getFileTag(fd, key, buf, 7);
   assert(valueLength == len);

   checkStringsAreEqual(val, buf, len);

   close(fd);

   valueLength = getFileTag(fd, key, buf, 7);  // fd is closed
   assertEquals(-1, valueLength);

   fd = open("ls", O_WRONLY);
   valueLength = getFileTag(fd, key, buf, 7);  // fd is write only
   assertEquals(-1, valueLength);

   fd = open("ls", O_RDONLY);
   valueLength = getFileTag(fd, key, buf, 7);  // OK
   assertEquals(len, valueLength);

   checkStringsAreEqual(val, buf, len);
   
   printf(1, "TEST PASSED\n");
   exit();
}
