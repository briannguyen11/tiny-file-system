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
    int diskFd1;
    int diskFd2;

    /************** Testing Disk Mount and Setup FS **************/
    // expected to fail
    res = tfs_mount(DEFAULT_DISK_NAME);

    if (res < 0) {
        // create first disk
        diskFd1 = tfs_mkfs(DEFAULT_DISK_NAME, DEFAULT_DISK_SIZE);
        // create second disk
        diskFd2 = tfs_mkfs("tinyFSDiskRand", DEFAULT_DISK_SIZE);
        // mount to first disk
        res = tfs_mount(DEFAULT_DISK_NAME);
        // unmount first disk, mount to new disk
        // res = tfs_mount("tinyFSDiskRand");
    }

    /************** Testing file operations **************/
    // creating files
    int fd1 = tfs_openFile("file1");
    int fd2 = tfs_openFile("file2");
    int fd3 = tfs_openFile("file3");

    // closing files
    // res = tfs_closeFile(fd1);
    // res = tfs_closeFile(fd2);
    // res = tfs_closeFile(999);

    // writiing to a file
    char *fileCont1, *fileCont2, *fileCont3;
    int fileSize1 = 300;
    int fileSize2 = 1500;
    int fileSize3 = 700;
    char filePhrase1[] = "fileOne";
    char filePhrase2[] = "fileTwo";
    char filePhrase3[] = "fileThree";

    fileCont1 = (char *)malloc(fileSize1 * sizeof(char));
    if (fillBufferWithPhrase(filePhrase1, fileCont1, fileSize1) < 0) {
        perror("failed");
        return -1;
    }

    fileCont2 = (char *)malloc(fileSize2 * sizeof(char));
    if (fillBufferWithPhrase(filePhrase2, fileCont2, fileSize2) < 0) {
        perror("failed");
        return -1;
    }

    fileCont3 = (char *)malloc(fileSize3 * sizeof(char));
    if (fillBufferWithPhrase(filePhrase3, fileCont3, fileSize3) < 0) {
        perror("failed");
        return -1;
    }

    res = tfs_writeFile(fd1, fileCont1, fileSize1);
    res = tfs_writeFile(fd2, fileCont2, fileSize2);
    res = tfs_writeFile(fd3, fileCont3, fileSize3);
    res = tfs_deleteFile(fd2);

    /************** Testing read and seek operations **************/
    res = tfs_seek(fd1, 277);

    char rByte;
    // res = tfs_readByte(fd1, &rByte);
    // printf("\nRead Byte: %c\n", rByte);

    printf("More Read Bytes: \n");
    // go until readByte fails
    int i = 0;
    while (tfs_readByte(fd1, &rByte) >= 0) {
        printf("%c", rByte);
        if ((i + 1) % 16 == 0) {
            printf("\n");
        }
        i++;
    }

    free(fileCont1);

    // test getStartBlock
    // char testStartBlock[] = "SICCICCCCCCFFFFFFFFFFFFFFFFFFFFFFFFFFFFF";
    // res = getStartBlock(diskFd1, 4, testStartBlock);
    // printf("%d", getStartBlock);

    return 0;
}