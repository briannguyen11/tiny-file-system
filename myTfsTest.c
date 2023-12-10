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
    char rdBuf;
    char *fileCont1, *fileCont2, *fileCont3, *fileCont4;
    int fileSize1 = 300;   // 2 + inode
    int fileSize2 = 1200;  // 5 + inode
    int fileSize3 = 700;   // 3 + inode
    int fileSize4 = 900;   // 4 + inode
    char filePhrase1[] = "fileOne";
    char filePhrase2[] = "fileTwo";
    char filePhrase3[] = "fileThree";
    char filePhrase4[] = "fileFour";

    fileDescriptor fd1, fd2, fd3, fd4;

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

    fileCont4 = (char *)malloc(fileSize4 * sizeof(char));
    if (fillBufferWithPhrase(filePhrase4, fileCont4, fileSize4) < 0) {
        perror("failed");
        return -1;
    }

    /************** Testing Disk Mount #1 **************/
    /* try to mount the disk */
    if (tfs_mount(DEFAULT_DISK_NAME) < 0) /* if mount fails */
    {
        tfs_mkfs(DEFAULT_DISK_NAME,
                 DEFAULT_DISK_SIZE);      /* then make a new disk */
        tfs_mkfs("tinyFSDiskRand", 2560); /* and another new disk */

        tfs_mount(DEFAULT_DISK_NAME); /* mount to first disk */
    }

    /* Init file 1 */
    fd1 = tfs_openFile("file1");
    if (tfs_readByte(fd1, &rdBuf) < 0) {
        /* If readByte() fails, there was no afile, so we write to it */
        tfs_writeFile(fd1, fileCont1, fileSize1);

        /* Get time stamps */
        tfs_readFileInfo(fd1);

        /* Display fragments */
        tfs_displayFragments();

    } else {
        /* Display overwritten bytes */
        while (tfs_readByte(fd1, &rdBuf) >= 0) {
            printf("%c", rdBuf);
        }

        /* Seek to halfway of the file1 ((300/2) - 1) and write */
        tfs_seek(fd1, 149);
        while (tfs_writeByte(fd1, 'x') >= 0) {
            printf("%c", rdBuf);
        }

        /* Add delay before reading */
        sleep(2);

        /* Seek to beginning */
        tfs_seek(fd1, 0);

        /* Display overwritten bytes */
        while (tfs_readByte(fd1, &rdBuf) >= 0) {
            printf("%c", rdBuf);
        }

        /* Get time stamps */
        tfs_readFileInfo(fd1);
    }

    /* Init file 2 */
    fd2 = tfs_openFile("file2");
    if (tfs_readByte(fd2, &rdBuf) < 0) {
        /* if readByte() fails, there was no afile, so we write to it */
        tfs_writeFile(fd2, fileCont2, fileSize2);

        /* Make file2 RO */
        tfs_makeRO("file2");

        /* Get time stamps */
        tfs_readFileInfo(fd2);

        /* Display fragments */
        tfs_displayFragments();

    } else {
        /* Get time stamps */
        tfs_readFileInfo(fd2);

        /* Attempt to delete but should FAIL */
        if (tfs_deleteFile(fd2) < 0) {
            /* Reopen file 2 and delete */
            fd2 = tfs_openFile("file2");

            /* Make file2 RW */
            tfs_makeRW("file2");

            /* Should now be able to delete file2 */
            tfs_deleteFile(fd2);

            /* Display fragments */
            tfs_displayFragments();

            /* Remove fragments */
            tfs_defrag();

            /* Display no fragemnts */
            tfs_displayFragments();
        }
    }

    // /* Init file 3 */
    fd3 = tfs_openFile("file3");
    if (tfs_readByte(fd3, &rdBuf) < 0) {
        /* If readByte() fails, there was no afile, so we write to it */
        tfs_writeFile(fd3, fileCont3, fileSize3);

        /* Get time stamps */
        tfs_readFileInfo(fd3);

        /* Display fragments */
        tfs_displayFragments();

    } else {
        /* Get time stamps */
        tfs_readFileInfo(fd3);

        /* Show files */
        tfs_readdir();

        /* Reanme file1 */
        tfs_rename(fd3, "newfile3");

        /* Show files after rename */
        tfs_readdir();

        /* Close file */
        tfs_closeFile(fd1);
        tfs_closeFile(fd3);
    }

    /************** Testing Disk Mount #2 **************/

    tfs_mount("tinyFSDiskRand");

    /* Init file 4 */
    fd4 = tfs_openFile("file4");
    if (tfs_readByte(fd1, &rdBuf) < 0) {
        /* If readByte() fails, there was no afile, so we write to it */
        tfs_writeFile(fd4, fileCont4, fileSize4);

        /* Get time stamps */
        tfs_readFileInfo(fd4);

        /* Display fragments */
        tfs_displayFragments();

    } else {
        /* Display overwritten bytes */
        while (tfs_readByte(fd4, &rdBuf) >= 0) {
            printf("%c", rdBuf);
        }

        /* Seek to 1/3 of the file4 ((900/3) - 1) and write */
        tfs_seek(fd4, 299);
        while (tfs_writeByte(fd1, 'x') >= 0) {
            printf("%c", rdBuf);
        }

        /* Seek to beginning */
        tfs_seek(fd4, 0);

        /* Display overwritten bytes */
        while (tfs_readByte(fd4, &rdBuf) >= 0) {
            printf("%c", rdBuf);
        }
    }

    /************** Clean Up **************/
    free(fileCont1);
    free(fileCont2);
    free(fileCont3);
    free(fileCont4);

    return 0;
}