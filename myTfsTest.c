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

    res = tfs_mount(DEFAULT_DISK_NAME);
    printf("] Initial mount to disk should fail w/ status: %d\n", res);
    if (res < 0) {
        res = tfs_mkfs(DEFAULT_DISK_NAME, DEFAULT_DISK_SIZE);
        printf("] Create new disk w/ status: %d\n", res);

        res = tfs_mkfs("tinyFSDiskRand", DEFAULT_DISK_SIZE);
        printf("] Create new disk w/ status: %d\n", res);

        // assumes tfs_mkfs worked
        res = tfs_mount(DEFAULT_DISK_NAME);
        printf("] Successfully mounted w/ status: %d\n", res);

        res = tfs_mount("tinyFSDiskRand");
        printf("] Successfully mounted w/ status: %d\n", res);
    }

    /************** Testing file operations **************/
    // creating files
    int fd1 = tfs_openFile("file1");
    if (res < 0) {
        printf("] Failed to create a new file w/ status: %d\n", fd1);
    } else {
        printf("] Created a new file w/ fd: %d\n", fd1);
    }

    int fd2 = tfs_openFile("file2");
    if (res < 0) {
        printf("] Failed to create a new file w/ status: %d\n", fd2);
    } else {
        printf("] Created a new file w/ fd: %d\n", fd2);
    }

    int fd3 = tfs_openFile("file3");
    if (res < 0) {
        printf("] Failed to create a new file w/ status: %d\n", fd3);
    } else {
        printf("] Created a new file w/ fd: %d\n", fd3);
    }

    // closing files
    // res = tfs_closeFile(fd1);
    // if (res < 0) {
    //     printf("] Failed to close file w/ status: %d\n", res);
    // } else {
    //     printf("] Closed file w/ status: %d\n", res);
    // }

    res = tfs_closeFile(fd2);
    if (res < 0) {
        printf("] Failed to close file w/ status: %d\n", res);
    } else {
        printf("] Closed file w/ status: %d\n", res);
    }

    res = tfs_closeFile(999);
    if (res < 0) {
        printf("] Failed to close file w/ status: %d\n", res);
    } else {
        printf("] Closed file w/ status: %d\n", res);
    }

    // writiing to a file
    char *file1Content = NULL;
    int file1Size = 1500;
    char file1Phrase[] = "hello world from file 1";

    file1Content = (char *)malloc(file1Size * sizeof(char));
    if (fillBufferWithPhrase(file1Phrase, file1Content, file1Size) < 0) {
        perror("failed");
        return -1;
    }
    if (tfs_writeFile(fd1, file1Content, file1Size) < 0) {
        perror("] tfs_writeFile failed");
    } else
        printf("] Successfully wrote to file\n");

    free(file1Content);

    return 0;
}