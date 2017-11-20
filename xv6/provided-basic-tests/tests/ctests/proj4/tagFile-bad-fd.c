/* call tagFile with bad fd argument.  tagFile should return -1. */
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
   char* bad_key = "one to nine";
   char* bad_key2 = "";
   char* good_max_key = "one to ni";
   char* val = "utility";
   char* good_max_val = "ahung gimothi ahun";
   char* bad_val= "ahung gimothi ahung";
   int len = 7;
   int good_max_len = 18;
   int bad_len = 19;
   char buf[7];
   char longbuf[18];

   //invalid fd
   int res = tagFile(-1, key, val, len);
   assert(res == -1);
   res = getFileTag(-1, key, buf, len);
   assert(res == -1);
   res = removeFileTag(-1, key);
   assert(res == -1);

   //invalid fd
   res = tagFile(1000, key, val, len);
   assert(res == -1);
   res = getFileTag(1000, key, buf, len);
   assert(res == -1);
   res = removeFileTag(1000, key);
   assert(res == -1);
   //good
   res = tagFile(fd, key, val, len);
   assert(res == 1);
   res = getFileTag(fd, key, buf, len);
   assert(res == len);
   res = removeFileTag(fd, key);
   assert(res == 1);
   //good max for key
   res = tagFile(fd, good_max_key, val, len);
   assert(res == 1);
   res = getFileTag(fd, good_max_key, buf, len);
   assert(res == len);
   res = removeFileTag(fd, good_max_key);
   assert(res == 1);
   //good max len for value 
   res = tagFile(fd, key, good_max_val, good_max_len);
   assert(res == 1);
   res = getFileTag(fd, key, longbuf, good_max_len);
   printf(1, "good_max_len: %d\n", res);
   assert(res == good_max_len);
   res = removeFileTag(fd, key);
   assert(res == 1);
   //bad key
   res = tagFile(fd, bad_key, val, len);
   assert(res == -1);
   res = getFileTag(fd, bad_key, buf, len);
   assert(res == -1);
   res = removeFileTag(fd, bad_key);
   assert(res == -1);
   //bad key2
   res = tagFile(fd, bad_key2, val, len);
   assert(res == -1);
   res = getFileTag(fd, bad_key2, buf, len);
   assert(res == -1);
   res = removeFileTag(fd, bad_key);
   assert(res == -1);
   //bad value
   res = tagFile(fd, key, bad_val, bad_len);
   assert(res == -1);
   res = getFileTag(fd, key, longbuf, bad_len);
   assert(res == -1);
   res = removeFileTag(fd, key);
   assert(res == -1);

   close(fd);
   res = tagFile(fd, key, val, len);
   assert(res == -1);
   int fd_read = open("ls", O_RDONLY);
   // Read only
   printf(1, "fd of ls: %d\n", fd_read);
   res = tagFile(fd, key, val, len);
   assert(res == -1);

   printf(1, "TEST PASSED\n");
   exit();
}
