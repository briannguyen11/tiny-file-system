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

    // expected to fail
    res = tfs_mount(DEFAULT_DISK_NAME);

    /************** Testing Disk Mount #1 and Setup FS **************/

    if (res < 0) {
        // create two FS
        diskFd1 = tfs_mkfs(DEFAULT_DISK_NAME, DEFAULT_DISK_SIZE);  // 40 blocks
        diskFd2 = tfs_mkfs("tinyFSDiskRand", 2560);                // 10 blocks

        // mount to first disk
        res = tfs_mount(DEFAULT_DISK_NAME);
    }

    /** Testing file operations **/
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
    int fileSize1 = 300;   // 2 + inode
    int fileSize2 = 1200;  // 5 + inode
    int fileSize3 = 700;   // 3 + inode
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
    res = tfs_writeFile(fd1, fileCont3, fileSize3);
    // res = tfs_deleteFile(fd2);

    /** Testing read and seek operations **/
    res = tfs_rename(fd2, "newName");
    res = tfs_seek(fd2, 693);

    char rByte;
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

    res = tfs_displayFragments();
    res = tfs_readdir();

    /************** Testing Disk #2 Mount  **************/

    // res = tfs_mount("tinyFSDiskRand");

    // /** Testing file operations **/
    // // creating files
    // int fd4 = tfs_openFile("file4");
    // int fd5 = tfs_openFile("file5");
    // int fd6 = tfs_openFile("file6");

    // // closing files
    // // res = tfs_closeFile(fd1);
    // // res = tfs_closeFile(fd2);
    // // res = tfs_closeFile(999);

    // // writiing to a file
    // char *fileCont4, *fileCont5, *fileCont6;
    // int fileSize4 = 300;   // 2 + inode
    // int fileSize5 = 1200;  // 5 + inode
    // int fileSize6 = 700;   // 3 + inode
    // char filePhrase4[] = "fileFour";
    // char filePhrase5[] = "fileFive";
    // char filePhrase6[] = "fileSix";

    // fileCont4 = (char *)malloc(fileSize4 * sizeof(char));
    // if (fillBufferWithPhrase(filePhrase4, fileCont4, fileSize4) < 0) {
    //     perror("failed");
    //     return -1;
    // }

    // fileCont5 = (char *)malloc(fileSize5 * sizeof(char));
    // if (fillBufferWithPhrase(filePhrase5, fileCont5, fileSize5) < 0) {
    //     perror("failed");
    //     return -1;
    // }

    // fileCont6 = (char *)malloc(fileSize6 * sizeof(char));
    // if (fillBufferWithPhrase(filePhrase6, fileCont6, fileSize6) < 0) {
    //     perror("failed");
    //     return -1;
    // }

    // res = tfs_writeFile(fd4, fileCont4, fileSize4);
    // res = tfs_writeFile(fd5, fileCont5, fileSize5);
    // res = tfs_writeFile(fd6, fileCont6, fileSize6);

    /************** Clean Up **************/
    free(fileCont1);
    free(fileCont2);
    free(fileCont3);
    // free(fileCont4);
    // free(fileCont5);
    // free(fileCont6);
    // free(fileCont4);
    // free(fileCont5);
    // free(fileCont6);

    // test getStartBlock
    // char testStartBlock[] = "SICCICCCCCCFFFFFFFFFFFFFFFFFFFFFFFFFFFFF";
    // res = getStartBlock(diskFd1, 4, testStartBlock);
    // printf("%d", getStartBlock);

    return 0;
}