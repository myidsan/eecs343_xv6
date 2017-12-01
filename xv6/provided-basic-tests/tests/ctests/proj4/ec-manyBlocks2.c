/* call tagFile 255 times (my method of creating unique keys is limited).  
   Close the file, then reopen it.  Then call 
   getFileTag to read the tags of that file. */
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

int
main(int argc, char *argv[])
{
   int i, j;
   int res;
   int valueLength;
   char v_actual;
   char v_expected;
   ppid = getpid();
   int fd = open("ls", O_RDWR);
   char key[10];
   for(i = 0; i < 9; i++){
      key[i] = 'k';
   }
   key[9] = 0;  // null termination

   char value[18];
   for(i = 0; i < 17; i++){
      value[i] = 'v';
   }
   value[17] = 0; // null termination

   int len = 17;

   // give the file 255 tags
   int numTags = 255;
   for(i = 0; i < numTags; i++){
      key[8] = (char)(i + 1);     // set the last character before the NULL
      value[16] = (char)(i + 1);  // set the last character before the NULL
      printf(1, "adding tag #%d\n", i + 1);
      res = tagFile(fd, key, value, len);
      assertEquals(1, res);
   }
   
   close(fd);

   fd = open("ls", O_RDONLY);
   char buf[18];

   // get the file tag 255 times
   for(i = 0; i < numTags; i++){
      key[8] = (char)(i + 1);
      valueLength = getFileTag(fd, key, buf, 18);
      assertEquals(len, valueLength);

      for(j = 0; j < len; j++){
         if(j == len - 1){
            v_expected = (char)(i + 1);
         } else {
            v_expected = value[j];
         }
         v_actual = buf[j];
         assertEquals(v_expected, v_actual);
      }
   }

   close(fd);

   printf(1, "TEST PASSED\n");
   exit();
}