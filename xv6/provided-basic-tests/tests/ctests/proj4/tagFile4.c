/* Test that tag data is actually written to disk, not just stored in memory: Call fork to create 
   a child process.  Have the child process open a file, tag it, and close it.  Let the child 
   process exit.  Then the parent process should open many other files (at least NINODE- the 
   maximum number of active i-nodes) to try to flush the child-tagged file from memory to disk.  
   Then the parent should open the original file and read the tag. */
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
   int fd;
   int fdList[64];
   char v_actual;
   char v_expected;
   ppid = getpid();

   char* key = "author";
   char* value = "unknown";
   int len = 7;

   int child_pid = fork();
   if(child_pid > 0){
      // in child
      fd = open("ls", O_RDWR);
      int res = tagFile(fd, key, value, len);
      assert(res > 0);
      close(fd);
      exit();
   }

   // in parent
   wait();

   char fname[3];
   fname[0] = 'f';
   fname[2] = 0;
   int i, j;

   for(j = 0; j < 4; j++){
      // open 14 files and write to them
      for(i = 0; i < 13; i++){
         fname[1] = (char)(i + j + 65);
         printf(1, "opening: %s\n", fname);
         fd = open(fname, O_CREATE | O_WRONLY);
         assert(fd > 0);
         fdList[i + j] = fd;
         write(fd, "hello", 5);
      }
      // close the 13 files so that you can open 16 more
      for(i = 0; i < 13; i++){
         fd = fdList[i + j];
         close(fd);
      }
   }

   // open the child-tagged file and read the tag
   fd = open("ls", O_RDONLY);
   char buf[8];
   int valueLength = getFileTag(fd, key, buf, 8);
   assertEquals(len, valueLength); 
   assert(valueLength == len);

   // check that the tag matches
   for(j = 0; j < len; j++){
      v_expected = value[j];
      v_actual = buf[j];
      assertEquals(v_expected, v_actual);
      assert(v_actual == v_expected);
   }

   close(fd);

   printf(1, "TEST PASSED\n");
   exit();
}
