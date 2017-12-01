/* Add a couple tags to a file.  Call getAllTags.  Confirm that the array of returned keys is complete 
   and correct.  Then, for all keys in the returned array, call getFileTag and confirm that the 
   returned values are correct. */

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
expectedKeyIsInActualKeys(char* expected, struct Key* actualKeys, int numberOfKeys)
{
    int i;
    for(i = 0; i < numberOfKeys; i++){
        if(strcmp(expected, actualKeys[i].key) == 0){
            return;
        }
    }

    printf(1, "keys array should contain '%s', but it does not.\n", expected);
    testFailed();
}

void
actualKeyIsInExpectedKeys(struct Key actual, char** expectedKeys, int numberOfKeys)
{
    int i;
    for(i = 0; i < numberOfKeys; i++){
        if(strcmp(actual.key, expectedKeys[i]) == 0){
            return;
        }
    }

    printf(1, "keys array contains '%s', but it should not.\n", actual.key);
    testFailed();
}

void
allExpectedKeysAreInActualKeys(char** expectedKeys, struct Key* actualKeys, int numberOfKeys)
{
    int i;
    for(i = 0; i < numberOfKeys; i++){
        expectedKeyIsInActualKeys(expectedKeys[i], actualKeys, numberOfKeys);
    }
}

void
allActualKeysAreInExpectedKeys(char** expectedKeys, struct Key* actualKeys, int numberOfKeys)
{
    int i;
    for(i = 0; i < numberOfKeys; i++){
        actualKeyIsInExpectedKeys(actualKeys[i], expectedKeys, numberOfKeys);
    }
}

void
checkKeys(char** expectedKeys, struct Key* actualKeys, int numberOfKeys)
{
    allExpectedKeysAreInActualKeys(expectedKeys, actualKeys, numberOfKeys);
    allActualKeysAreInExpectedKeys(expectedKeys, actualKeys, numberOfKeys);
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

int
main(int argc, char *argv[])
{
    ppid = getpid();

    char* expectedKeys[2];
    expectedKeys[0] = "type";
    expectedKeys[1] = "author";
    char* expectedValues[2];
    expectedValues[0] = "utility";
    expectedValues[1] = "a";

    // test checkKeys
    struct Key keysTest[16];
    strcpy(keysTest[0].key, "author");
    strcpy(keysTest[1].key, "type");
    checkKeys(expectedKeys, keysTest, 2);

    // test checkValues
    char* valuesTest[16];
    valuesTest[0] = "a";
    valuesTest[1] = "utility";
    checkValues(expectedValues, valuesTest, 2);

    // add two tags to ls
    int fd = open("ls", O_RDWR);
    char* key = "type";
    char* val = "utility";
    int len = 7;
    int res = tagFile(fd, key, val, len);
    assertEquals(1, res);

    char* key2 = "author";
    char* val2 = "a";
    int len2 = 1;
    int res2 = tagFile(fd, key2, val2, len2);
    assertEquals(1, res2);

    // call getAllTags
    struct Key keys[16];
    int numTags = getAllTags(fd, keys, 16);
    assertEquals(2, numTags);

    // check keys
    checkKeys(expectedKeys, keys, numTags);

    // allocate buffers
    char* buffers[16];
    int i;
    for(i = 0; i < 16; i++){
        buffers[i] = malloc(18);
    }

    // getFileTag for each key returned by getAllTags
    for(i = 0; i < numTags; i++){
        int valueLength = getFileTag(fd, keys[i].key, buffers[i], 18);
        assert(valueLength > 0);
    }
    close(fd);

    // check values
    checkValues(expectedValues, buffers, numTags);

    printf(1, "TEST PASSED\n");
    exit();
}
