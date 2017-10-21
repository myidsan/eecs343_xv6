#include "types.h"
#include "stat.h"
#include "user.h"

#define USERTOP 0xA0000
#define PGSIZE 4096

void
testPassed(void)
{
  printf(1, "....Passed\n");
}

void
testFailed(void)
{
  printf(1, "....FAILED\n");
}

void expectedVersusActualNumeric(char* name, int expected, int actual)
{
  printf(1, "      %s expected: %d, Actual: %d\n", name, expected, actual);
}

void
whenRequestingSharedMemory_ValidAddressIsReturned(void)
{
  printf(1, "Test: whenRequestingSharedMemory_ValidAddressIsReturned...");
  char* sharedPage = shmem_access(0);
  char* highestPage =       (char*)(USERTOP - PGSIZE);
  char* secondHighestPage = (char*)(USERTOP - 2*PGSIZE);
  char* thirdHighestPage =  (char*)(USERTOP - 3*PGSIZE);
  char* fourthHighestPage = (char*)(USERTOP - 4*PGSIZE);
  
  if(sharedPage == highestPage ||
     sharedPage == secondHighestPage ||
     sharedPage == thirdHighestPage ||
     sharedPage == fourthHighestPage) {
    testPassed();
  } else {
    testFailed(); 
  }
}

void
afterRequestingSharedMemory_countReturns1_pageZero()
{
  printf(1, "Test: afterRequestingSharedMemory_countReturns1_pageZero...");
  char* sharedPage = shmem_access(0);
  int count = shmem_count(0);

  if(count == 1) {
    testPassed();
  } else {
    testFailed();
    expectedVersusActualNumeric("'count'", 1, count);
  }

  // silence the error about unused variable
  sharedPage = sharedPage + 0;
}

void
afterRequestingSharedMemory_countReturns1_pageOne()
{
  printf(1, "Test: afterRequestingSharedMemory_countReturns1_pageOne...");
  char* sharedPage = shmem_access(1);
  int count = shmem_count(1);

  if(count == 1) {
    testPassed();
  } else {
    testFailed();
    expectedVersusActualNumeric("'count'", 1, count);
  }

  // silence the error about unused variable
  sharedPage = sharedPage + 0;
}

void
afterRequestingSharedMemory_countReturns1_pageTwo()
{
  printf(1, "Test: afterRequestingSharedMemory_countReturns1_pageTwo...");
  char* sharedPage = shmem_access(2);
  int count = shmem_count(2);

  if(count == 1) {
    testPassed();
  } else {
    testFailed();
    expectedVersusActualNumeric("'count'", 1, count);
  }

  // silence the error about unused variable
  sharedPage = sharedPage + 0;
}

void
afterRequestingSharedMemory_countReturns1_pageThree()
{
  printf(1, "Test: afterRequestingSharedMemory_countReturns1_pageThree...");
  char* sharedPage = shmem_access(3);
  int count = shmem_count(3);

  if(count == 1) {
    testPassed();
  } else {
    testFailed();
    expectedVersusActualNumeric("'count'", 1, count);
  }

  // silence the error about unused variable
  sharedPage = sharedPage + 0;
}

void
whenMultipleProcsShareAPage_countReturnsCorrectly()
{
  printf(1, "Test: whenMultipleProcsShareAPage_countReturnsCorrectly...");
  char* sharedPage = shmem_access(3);
  char* sharedPageTwo = shmem_access(3);
  char* sharedPageThree = shmem_access(3);
  int count = shmem_count(3);

  if(count == 3) {
    testPassed();
  } else {
    testFailed();
    expectedVersusActualNumeric("'count'", 3, count);
  }

  // silence the error about unused variable
  sharedPage = sharedPage + 0;
  sharedPageTwo = sharedPageTwo + 0;
  sharedPageThree = sharedPageThree + 0;
}

void
whenSharingAPage_ParentSeesChangesMadeByChild()
{
  printf(1, "Test: whenSharingAPage_ParentSeesChangesMadeByChild...");
  char* sharedPage = shmem_access(0);
  sharedPage[0] = 42;

  int pid = fork();
  if(pid == 0){
    // in child
    char* childsSharedPage = shmem_access(0);
    childsSharedPage[0] = childsSharedPage[0] + 1;
    exit();
  } else {
    // in parent
    wait(); // wait for child to terminate
    if(sharedPage[0] == 43){
      testPassed();
    } else {
      testFailed();
      expectedVersusActualNumeric("'sharedPage[0]'", 43, sharedPage[0]);
    }
  }
}

void
whenProcessExits_SharedPageIsFreed()
{
  printf(1, "Test: whenProcessExits_SharedPageIsFreed...");
  int pid = fork();

  if(pid == 0){
    // in child
    char* sharedPage = shmem_access(0);
    sharedPage[0] = 42;
    exit();
  } else {
    // in parent
    wait();
    //
    char* parentsSharedPage = shmem_access(0);
    if(parentsSharedPage[0] != 42){
      testPassed();
    } else {
      // should be garbage value after being freed, but it's still 42
      testFailed();
      expectedVersusActualNumeric("'parentsSharedPage[0]'", 1, parentsSharedPage[0]);
    }
  }
}

void
whenSharingAPageBetween2Processes_countReturns2()
{
  printf(1, "Test: whenSharingAPageBetween2Processes_countReturns2...");

  char* sharedPage = shmem_access(0);
  sharedPage = sharedPage + 0;  // silence unused variable error

  int pid = fork();

  if(pid == 0){
    // in child
    char* childsSharedPage = shmem_access(0);
    childsSharedPage = childsSharedPage + 0;  // silence unused variable error

    int count = shmem_count(0);
    if(count != 2){
      testFailed();
      expectedVersusActualNumeric("'count'", 2, count);
    }

    exit();
  } else{
    // in parent
    wait(); // wait for child to exit
    int parentsCount = shmem_count(0);
    if(parentsCount != 1){
      testFailed();
      expectedVersusActualNumeric("'parentsCount'", 1, parentsCount);
    }
  }

  testPassed();
}

void
whenProcessExists_countReturns0()
{
  printf(1, "Test: whenProcessExists_countReturns0...");

  int pid = fork();

  if(pid == 0){
    // in child
    char* sharedPage = shmem_access(0);
    sharedPage = sharedPage + 0;  // silence unused variable error
    exit();
  } else {
    // in parent
    wait();
    int count = shmem_count(0);

    if(count != 0){
      testFailed();
      expectedVersusActualNumeric("'count'", 0, count);
    } else {
      testPassed();
    }

  }
}

void
beforeRequestingSharedMemory_countReturns0()
{
  printf(1, "Test: beforeRequestingSharedMemory_countReturns0...");

  int count = shmem_count(0);

  if(count != 0){
    testFailed();
    expectedVersusActualNumeric("'count'", 0, count);
  } else {
    testPassed();
  }
}

// If a process calls this syscall twice with the same argument, 
// the syscall should recognize that this process has already mapped this shared page and simply return the virtual address again.
void
ifAlreadyMapped_ReturnTheVirtualAddressAgain()
{
  printf(1, "Test: ifAlreadyMapped_ReturnTheVirtualAddressAgain...");
  char* sharedPage = shmem_access(3);
  char* sharedPageTwo = shmem_access(3);
  //char* sharedPageThree = shmem_access(3);
  int sp = sharedPage[0];
  int spt = sharedPageTwo[0];  
  if( sharedPage == sharedPageTwo ){
	testPassed();
  } else {
	testFailed();
	expectedVersusActualNumeric("'va'", sp, spt);
  }

}

// shmem_access syscall will fail when trying to access invalid shemem page
void
whenAccessInvalidPage_ReturnNegativeNULL()
{
  printf(1, "whenAccessInvalidPage_ReturnNegativeNULL..."); 
  if( (int)shmem_access(5) == -1) {
	testPassed();
  } else {
	testFailed();
  }
}

// shmem_count syscall will fail when trying to access invalid shemem page
void
whenAccessInvalidPage_shmemCountReturnNegativeNULL()
{
  printf(1, "whenAccessInvalidPage_shmemCountReturnNegativeNULL..."); 
  char* sharedPage = shmem_access(3);
  char* sharedPageTwo = shmem_access(2);
  if( shmem_count(5) == -1) {
	testPassed();
  } else {
	testFailed();
  }
  // to silence unused warning 
  sharedPage = sharedPage + 0;
  sharedPageTwo = sharedPageTwo + 0;
}

// No memory leaks. For example, suppose several processes have been sharing a page, but now process #42 is the last process left alive that was using that page. When process #42 terminates, the shared page must be freed.
void
whenLastProcessExits_SharedPageIsFreed()
{
  printf(1, "Test: whenLastProcessExits_SharedPageIsFreed...");
  int pid = fork();

  if(pid == 0){
	int pid = fork();
	if( pid == 0) {
	  char*sharedPageTwo = shmem_access(0);
	  sharedPageTwo[0] = 11;
	  exit();
	} else {
      // in child
      char* sharedPage = shmem_access(0);
      sharedPage[0] = 42;
      exit();
	}
  } else {
      // in parent
      wait();
      //
      char* parentsSharedPage = shmem_access(0);
      if(parentsSharedPage[0] != 42){
        testPassed();
       } else {
        // should be garbage value after being freed, but it's still 42
        testFailed();
        expectedVersusActualNumeric("'parentsSharedPage[0]'", 1, parentsSharedPage[0]);
      }
    }
}

/*
// If a virtual memory page is being used as shared memory, 
// it cannot also be used as “normal” unshared memory and vice versa.
void
whenUsedAsSharedMem_cannotBeUsedAsNormalMem()
{
  printf(1, "whenUsedAsSharedMem_cannotBeUsedAsNormalMem...");
     
}
*/
int
main(void)
{
  int pid;

  // we fork then run each test in a child process to keep the main process
  // free of any shared memory
  pid = fork();
  if(pid == 0){
    whenRequestingSharedMemory_ValidAddressIsReturned();
    exit();
  }
  wait();
  
  pid = fork();
  if(pid == 0){
    afterRequestingSharedMemory_countReturns1_pageZero();
    exit();
  }
  wait();
  
  pid = fork();
  if(pid == 0){
    afterRequestingSharedMemory_countReturns1_pageOne();
    exit();
  }
  wait();
	 
  pid = fork();
  if(pid == 0){
    afterRequestingSharedMemory_countReturns1_pageTwo();
    exit();
  }
  wait();

  pid = fork();
  if(pid == 0){
    afterRequestingSharedMemory_countReturns1_pageThree();
    exit();
  }
  wait();

  pid = fork();
  if(pid == 0){
    whenSharingAPage_ParentSeesChangesMadeByChild();
    exit();
  }
  wait();

  pid = fork();
  if(pid == 0){
    whenSharingAPageBetween2Processes_countReturns2();
    exit();
  }
  wait();

  pid = fork();
  if(pid == 0){
    whenProcessExits_SharedPageIsFreed();
    exit();
  }
  wait();

  pid = fork();
  if(pid == 0){
    whenProcessExists_countReturns0();
    exit();
  }
  wait();

  pid = fork();
  if(pid == 0){
    beforeRequestingSharedMemory_countReturns0();
    exit();
  }
  wait();

  pid = fork();
  if(pid ==0){
	ifAlreadyMapped_ReturnTheVirtualAddressAgain();
  }
  wait();

  pid = fork();
  if(pid ==0){
	whenAccessInvalidPage_ReturnNegativeNULL();
  } 
  wait();

  pid = fork();
  if(pid == 0) {
  whenAccessInvalidPage_shmemCountReturnNegativeNULL();
  }
  wait();

  pid = fork();
  if(pid == 0) {
  whenLastProcessExits_SharedPageIsFreed();
  }
  wait();

  exit();
}
