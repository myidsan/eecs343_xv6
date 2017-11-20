/*call tagFile 16 times to fill up the whole allocated block. tagFile should return -1 on 17th tag creation*/
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
   char* key2 = "typo";
   char* key3 = "three";
   char* key4 = "four";
   char* key5 = "five";
   char* key6 = "six";
   char* key7 = "seven";
   char* key8 = "eight";
   char* key9 = "nine";
   char* key10 = "ten";
   char* key11 = "eleven";
   char* key12 = "twelve";
   char* key13 = "thirteen";
   char* key14 = "fourteen";
   char* key15 = "fifteen";
   char* key16 = "sixteen";
   char* key17 = "error";
   char* buf = "utility";
   char* buf2 = "atility";
   int len = 1;
   int getFileTag_length;
   char buffer[7];

   int valueLength = tagFile(fd, key, buf, 7);
   assert(valueLength == len);
   getFileTag_length = getFileTag(fd, key, buffer, 7);
   assert(getFileTag_length == 7);
   valueLength = tagFile(fd, key2, buf, 7);
   assert(valueLength == len);
   getFileTag_length = getFileTag(fd, key2, buffer, 7);
   assert(getFileTag_length == 7);
   valueLength = tagFile(fd, key3, buf, 7);
   assert(valueLength == len);
   getFileTag_length = getFileTag(fd, key3, buffer, 7);
   assert(getFileTag_length == 7);
   valueLength = tagFile(fd, key4, buf, 7);
   assert(valueLength == len);
   getFileTag_length = getFileTag(fd, key4, buffer, 7);
   assert(getFileTag_length == 7);
   valueLength = tagFile(fd, key5, buf, 7);
   assert(valueLength == len);
   getFileTag_length = getFileTag(fd, key5, buffer, 7);
   assert(getFileTag_length == 7);
   // inconsistent value of length 
   //valueLength = tagFile(fd, key6, buf, 5);
   //assert(valueLength == -1);
   valueLength = tagFile(fd, key6, buf, 7);
   assert(valueLength == len);
   getFileTag_length = getFileTag(fd, key6, buffer, 7);
   assert(getFileTag_length == 7);
   valueLength = tagFile(fd, key7, buf, 7);
   assert(valueLength == len);
   valueLength = tagFile(fd, key8, buf, 7);
   assert(valueLength == len);
   valueLength = tagFile(fd, key9, buf, 7);
   assert(valueLength == len);
   valueLength = tagFile(fd, key10, buf, 7);
   assert(valueLength == len);
   valueLength = tagFile(fd, key11, buf, 7);
   assert(valueLength == len);
   valueLength = tagFile(fd, key12, buf, 7);
   assert(valueLength == len);
   valueLength = tagFile(fd, key13, buf, 7);
   assert(valueLength == len);
   valueLength = tagFile(fd, key14, buf, 7);
   assert(valueLength == len);
   valueLength = tagFile(fd, key15, buf, 7);
   assert(valueLength == len);
   valueLength = tagFile(fd, key16, buf, 7);
   assert(valueLength == len);
   valueLength = tagFile(fd, key16, buf2, 7);
   assert(valueLength == len);
   valueLength = tagFile(fd, key17, buf, 7);
   printf(1, "error17: should be -1, but is: %d\n", valueLength);
   assert(valueLength == -1);



   close(fd); 
   printf(1, "TEST PASSED\n");
   exit();
}
