/* Add an identical tag to a couple files.  Make a new file and add the same tag to that.  
   Make a new directory, make a new file in that directory, and add the same tag to that.  
   Call getFilesByTag.  Confirm that the array of returned 
   filenames is complete and correct. */

#include "types.h"
#include "user.h"
// make sure that struct Key is included via either types.h or user.h above

#define O_RDONLY 0x000
#define O_WRONLY 0x001
#define O_RDWR 0x002
#define O_CREATE 0x200

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

void
testFailed()
{
    printf(1, "TEST FAILED\n");
    kill(ppid);
    exit();
}

void
expectedValueIsInActualValues(char* expectedValue, char** actualValues, int numberOfKeys)
{
    int i;
    for(i = 0; i < numberOfKeys; i++){
        if(strcmp(expectedValue, actualValues[i]) == 0){
            return;
        }
    }

    printf(1, "value '%s' should have been in a file tag, but was not found.\n", expectedValue);
    testFailed();
}

void
actualValuesIsInExpectedValues(char* actualValue, char** expectedValues, int numberOfKeys)
{
    int i;
    for(i = 0; i < numberOfKeys; i++){
        if(strcmp(actualValue, expectedValues[i]) == 0){
            return;
        }
    }

    printf(1, "file contained tag with value '%s', which is incorrect.\n", actualValue);
    testFailed();
}

void
allExpectedValuesAreInActualValues(char** expectedValues, char** actualValues, int numberOfKeys)
{
    int i;
    for(i = 0; i < numberOfKeys; i++){
        expectedValueIsInActualValues(expectedValues[i], actualValues, numberOfKeys);
    }
}

void
allActualValuesAreInExpectedValues(char** expectedValues, char** actualValues, int numberOfKeys)
{
    int i;
    for(i = 0; i < numberOfKeys; i++){
        actualValuesIsInExpectedValues(actualValues[i], expectedValues, numberOfKeys);
    }
}

void
checkValues(char** expectedValues, char** actualValues, int numberOfKeys)
{
    allExpectedValuesAreInActualValues(expectedValues, actualValues, numberOfKeys);
    allActualValuesAreInExpectedValues(expectedValues, actualValues, numberOfKeys);
}

void
readResultsIntoArray(char* results, int resultsLength, char** actualFilenames, int numFiles)
{
    int numFilenamesCopied;
    char* currPointer = results;
    int nameLength;

    for(numFilenamesCopied = 0; numFilenamesCopied < numFiles; numFilenamesCopied++){
        nameLength = strlen(currPointer);
        actualFilenames[numFilenamesCopied] = malloc(nameLength + 1);
        strcpy(actualFilenames[numFilenamesCopied], currPointer);
        currPointer += nameLength + 1;
    }
}

int
main(int argc, char *argv[])
{
    ppid = getpid();

    char* expectedFilenames[5];
    expectedFilenames[0] = "ls";
    expectedFilenames[1] = "mkdir";
    expectedFilenames[2] = "cat";
    expectedFilenames[3] = "new_file";
    expectedFilenames[4] = "new_file2";

    // add an identical tag to a couple files
    int fd;
    char* key = "type";
    char* val = "utility";
    int len = 7;
    int res;

    fd = open("ls", O_RDWR);
    res = tagFile(fd, key, val, len);
    assertEquals(1, res);
    close(fd);

    fd = open("cat", O_RDWR);
    res = tagFile(fd, key, val, len);
    assertEquals(1, res);
    close(fd);

    fd = open("mkdir", O_RDWR);
    res = tagFile(fd, key, val, len);
    assertEquals(1, res);
    close(fd);

    char* val2 = "syscall";
    int len2 = 7;
    fd = open("kill", O_RDWR);
    res = tagFile(fd, key, val2, len2);
    assertEquals(1, res);
    close(fd);

    // make a new file and add the same tag to that
    fd = open("new_file", O_CREATE | O_RDWR);
    res = tagFile(fd, key, val, len);
    assertEquals(1, res);
    close(fd);

    // make a new directory and put a file in it.  Add the same tag to that.
    res = mkdir("new_dir");
    assert(res >= 0);
    chdir("new_dir");
    fd = open("new_file2", O_CREATE | O_RDWR);
    res = tagFile(fd, key, val, len);
    assertEquals(1, res);
    close(fd);

    // get files by tag
    char results[100];
    int resultsLength = 100;
    int numFiles = getFilesByTag(key, val, len, results, resultsLength);
    assertEquals(5, numFiles);

    char* actualFilenames[10];
    readResultsIntoArray(results, resultsLength, actualFilenames, numFiles);

    // check filenames
    checkValues(expectedFilenames, actualFilenames, numFiles);

    printf(1, "TEST PASSED\n");
    exit();
}
