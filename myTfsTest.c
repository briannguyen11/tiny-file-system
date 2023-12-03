#include "libTinyFS.h"

/* simple helper function to fill Buffer with as many inPhrase strings as
 * possible before reaching size */
int fillBufferWithPhrase(char *inPhrase, char *Buffer, int size) {
    int index = 0, i;
    if (!inPhrase || !Buffer || size <= 0 || size < strlen(inPhrase)) return -1;

    while (index < size) {
        for (i = 0; inPhrase[i] != '\0' && (i + index < size); i++)
            Buffer[i + index] = inPhrase[i];
        index += i;
    }
    Buffer[size - 1] = '\0'; /* explicit null termination */
    return 0;
}

int main() {
    int res = 0;

    /************** Testing Disk Mount and Setup FS **************/
    // expected to fail
    res = tfs_mount(DEFAULT_DISK_NAME);

    if (res < 0) {
        // create first disk
        res = tfs_mkfs(DEFAULT_DISK_NAME, DEFAULT_DISK_SIZE);
        // create second disk
        res = tfs_mkfs("tinyFSDiskRand", DEFAULT_DISK_SIZE);
        // mount to first disk
        res = tfs_mount(DEFAULT_DISK_NAME);
        // unmount first disk, mount to new disk
        res = tfs_mount("tinyFSDiskRand");
    }

    /************** Testing file operations **************/
    // creating files
    int fd1 = tfs_openFile("file1");
    int fd2 = tfs_openFile("file2");
    int fd3 = tfs_openFile("file3");

    // closing files
    // res = tfs_closeFile(fd1);
    res = tfs_closeFile(fd2);
    // res = tfs_closeFile(999);

    // writiing to a file
    char *fileCont1;
    int fileSize1 = 1500;
    char filePhrase1[] = "hello world from file 1";

    fileCont1 = (char *)malloc(fileSize1 * sizeof(char));
    if (fillBufferWithPhrase(filePhrase1, fileCont1, fileSize1) < 0) {
        perror("failed");
        return -1;
    }

    res = tfs_writeFile(fd1, fileCont1, fileSize1);
    free(fileCont1);

    return 0;
}