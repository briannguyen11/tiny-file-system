#include "libTinyFS.h"

// Global Variables
char *mDisk = NULL;         // mounted disk
FileEntry *headDRT = NULL;  // head of DRT containing file entries

/**
 * Opens a new disk and initializes it with a super block
 * and free block to store future files
 */
int tfs_mkfs(char *filename, int nBytes) {
    int fd = 0;
    int tmp = 0;
    int numBlocks = nBytes / BLOCKSIZE;

    if ((fd = openDisk(filename, nBytes)) < 0) {
        printf("Error: Failed to open disk in function: tfs_mkfs\n");
        return OPEN_DISK_ERR;
    } else {
        // init empty buf (0x00)
        char *emptyBuf = calloc(BLOCKSIZE, sizeof(char));

        // populate disk w/ empty buf
        for (int i = 0; i < numBlocks; i++) {
            if ((tmp = writeBlock(fd, i, emptyBuf)) < 0) {
                printf("Error: Failed to write in function: tfs_mkfs\n");
                return WRITE_BLOCK_ERR;
            }
        }
        free(emptyBuf);

        // setup file system with super block and free blocks
        if ((tmp = setupFileSystem(fd, numBlocks)) < 0) {
            printf("Error: Failed to write in function: tfs_mkfs\n");
            return WRITE_BLOCK_ERR;
        }
    }

    return TFS_MKFS_SUCCESS;
}

/**
 * Set current disk being accessed to new disk
 */
int tfs_mount(char *diskname) {
    int fd = 0;
    int tmp = 0;
    char buf[BLOCKSIZE];

    // unmount current disk if applicable
    if (mDisk != NULL) {
        printf("%s is currently mounted. Unmouting now.\n", mDisk);  // debug
        tfs_unmount();
    }

    // mount to new disk by opening
    if ((fd = openDisk(diskname, 0)) < 0) {
        printf("Error: Failed to open disk in function: tfs_mount\n");
        return OPEN_DISK_ERR;
    }

    // read super block metadata
    if ((tmp = readBlock(fd, 0, buf)) < 0) {
        printf("Error: Failed to read in function: tfs_mount\n");
        return READ_BLOCK_ERR;
    }

    // validate magic number
    if (buf[1] != 0x44) {
        printf("Error: Non-valid magic number in function: tfs_mount\n");
        return INVALID_MNUM_ERR;
    }

    // set mounted disk to new disk
    mDisk = calloc(sizeof(char), strlen(diskname) + 1);
    strcpy(mDisk, diskname);

    printf("Now mounted to %s\n", mDisk);  // debug
    return TFS_MOUNT_SUCCESS;
}

/**
 * Remove current disk being accessed
 */
int tfs_unmount() {
    free(mDisk);
    if (mDisk == NULL) {
        printf("Nothing to unmount.");
    }
    mDisk = NULL;
    return TFS_UNMOUNT_SUCCESS;
}

/**
 * Opens or creates a new file. Updates DRT in
 * the process of creating a file
 */
fileDescriptor tfs_openFile(char *name) {
    int tmp = 0;
    fileDescriptor fd;
    FileEntry *newFE = malloc(sizeof(FileEntry));  // debug remember to free
    FileEntry *tmpFE = headDRT;

    /* Check if disk is mounted and open mounted disk */
    if (mDisk == NULL) {
        return NO_DISK_MOUNTED_ERR;
    } else {
        // open mounted disk to read
        if ((tmp = openDisk(mDisk, 0)) < 0) {
            return OPEN_DISK_ERR;
        }
    }

    /* Check if file exist in DRT LL -> if true, return fd */
    // -- note: if file is found in DRT, it is assumed that it has been opened
    while (tmpFE != NULL) {
        if (strcmp(tmpFE->filename, name) == 0) {
            return tmpFE->fd;
        }
        tmpFE = tmpFE->next;
    }

    /* Creating new file */
    // open a new file for r/w operations
    if ((fd = open(name, O_RDWR | O_CREAT)) < 0) {
        printf("Error: Could not create file in functoin: openFile.\n");
        return OPEN_FILE_ERR;
    }

    // create a new file entry
    if (strlen(name) > sizeof(newFE->filename) - 1) {
        printf("Error: Filename must be within 8 alphanumeric characters.\n");
        return FILENAME_ERR;
    } else {
        newFE->fd = fd;
        strcpy(newFE->filename, name);
        newFE->next = NULL;
    }

    // add new file entry to DRT
    if (headDRT == NULL) {
        headDRT = newFE;
    } else {
        FileEntry *curr = headDRT;
        while (curr->next != NULL) {
            curr = curr->next;
        }
        curr->next = newFE;
    }

    return fd;
}

/*********************** Helper Functions ***********************/

/**
 * Initializes super block and free blocks and writes the blocks
 * into the recently opened disk
 */
int setupFileSystem(int fd, int numBlocks) {
    int tmp = 0;

    /* Init Super Block */
    SuperBlock superBlock;
    superBlock.type = 1;
    superBlock.mNum = 0x44;

    char freeBlockArr[numBlocks];  // creating free block array
    memset(freeBlockArr, 'F',
           numBlocks);      // init free block array to all free
    freeBlockArr[0] = 'T';  // set first block to taken by super block

    memset(superBlock.data, 0, sizeof(superBlock.data));
    memcpy(superBlock.data, freeBlockArr,
           numBlocks);  // copies free block array into super block

    tmp = writeBlock(fd, 0, &superBlock);
    if (tmp < 0) {
        printf("Error: Failed to write in function: setupFileSystems\n");
        return WRITE_BLOCK_ERR;
    }

    /* Init Free Blocks */
    FreeBlock freeBlock;
    freeBlock.type = 4;
    freeBlock.mNum = 0x44;
    memset(freeBlock.data, 0, sizeof(superBlock.data));
    for (int i = 1; i < numBlocks; i++) {
        tmp = writeBlock(fd, i, &freeBlock);
        if (tmp < 0) {
            printf("Error: Failed to write in function: setupFileSystems\n");
            return WRITE_BLOCK_ERR;
        }
    }

    return 0;
}
