/* call tagFile to tag a file with a 5 different tags with long values.  Remove one of the middle tags.  
   Call tagFile again with a tag that wouldn't fit in the slot left by the middle tag, but would fit 
   if the block is defragmented.  Confirm that this tag has been added successsfully. */
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
   char* key1 = "a";
   char* key2 = "b";
   char* key3 = "c";
   char* key4 = "d";
   char* key5 = "e";
   
   char val[81];
   int i;
   for(i = 0; i < 80; i++){
      val[i] = 'z';
   }
   val[80] = '\0';

   int len = 80;
   int res;
   res = tagFile(fd, key1, val, len);
   assertEquals(1, res);
   res = tagFile(fd, key2, val, len);
   assertEquals(1, res);
   res = tagFile(fd, key3, val, len);
   assertEquals(1, res);
   res = tagFile(fd, key4, val, len);
   assertEquals(1, res);
   res = tagFile(fd, key5, val, len);
   assertEquals(1, res);

   char buf[500];
   int valueLength;
   valueLength = getFileTag(fd, key1, buf, 500);
   assertEquals(len, valueLength);
   checkStringsAreEqual(val, buf, len);

   valueLength = getFileTag(fd, key2, buf, 500);
   assertEquals(len, valueLength);
   checkStringsAreEqual(val, buf, len);

   valueLength = getFileTag(fd, key3, buf, 500);
   assertEquals(len, valueLength);
   checkStringsAreEqual(val, buf, len);

   valueLength = getFileTag(fd, key4, buf, 500);
   assertEquals(len, valueLength);
   checkStringsAreEqual(val, buf, len);

   valueLength = getFileTag(fd, key5, buf, 500);
   assertEquals(len, valueLength);
   checkStringsAreEqual(val, buf, len);

   res = removeFileTag(fd, key3);
   assertEquals(1, res);

   char val2[108];
   for(i = 0; i < 107; i++){
      val2[i] = '2';
   }
   val2[107] = '\0';

   int len2 = 107;
   res = tagFile(fd, key3, val2, len2);
   assertEquals(1, res);

   valueLength = getFileTag(fd, key3, buf, 500);
   assertEquals(len2, valueLength);
   checkStringsAreEqual(val2, buf, len2);

   close(fd);
   printf(1, "TEST PASSED\n");
   exit();
}
