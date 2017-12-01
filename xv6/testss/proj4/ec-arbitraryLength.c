/* call tagFile to tag a file with a 500 char value.  Call getFileTag to read the tag of that file. 
   Call removeFileTag to remove it.  Call getFileTag to confirm that it has been removed. 
   Call tagFile again with a shorter value.  Call get FileTag to confirm that the tag is correct. */
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
   char* key = "k";
   
   char val[500];
   int i;
   for(i = 0; i < 499; i++){
      val[i] = 'z';
   }
   val[499] = '\0';

   int len = 499;
   int res = tagFile(fd, key, val, len);
   assertEquals(1, res);

   char buf[500];
   int valueLength = getFileTag(fd, key, buf, 500);
   assertEquals(len, valueLength);

   checkStringsAreEqual(val, buf, len);

   res = removeFileTag(fd, key);
   assertEquals(1, res);

   valueLength = getFileTag(fd, key, buf, 500);
   assertEquals(-1, valueLength);

   char* key2 = "key2";
   char* val2 = "this value is much shorter but still longer than 18 chars.";
   int len2 = 58;
   res = tagFile(fd, key2, val2, len2);
   assertEquals(1, res);

   valueLength = getFileTag(fd, key2, buf, 500);
   assertEquals(len2, valueLength);

   checkStringsAreEqual(val2, buf, len2);

   close(fd);
   printf(1, "TEST PASSED\n");
   exit();
}
