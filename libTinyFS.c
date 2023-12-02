#include "libTinyFS.h"

// Global Variables
char *mDisk = NULL;         // mounted disk
FileEntry *headOFT = NULL;  // head of OFT containing file entries
int numBlocks = 0;

/**
 * Opens a new disk and initializes it with a super block
 * and free block to store future files
 */
int tfs_mkfs(char *filename, int nBytes) {
    int fd = 0;
    int tmp = 0;
    numBlocks = nBytes / BLOCKSIZE;

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

    // unmount current disk if another disk is mounted
    if (mDisk != NULL) {
        printf("Unmouting %s\n", mDisk);  // debug
        tfs_unmount();
    }

    // mount to new disk by opening the disk
    if ((fd = openDisk(diskname, 0)) < 0) {
        printf("Error: Failed to open disk in function: tfs_mount\n");
        return OPEN_DISK_ERR;
    }

    // read super block metadata to confirms magic number
    if ((tmp = readBlock(fd, 0, buf)) < 0) {
        printf("Error: Failed to read in function: tfs_mount\n");
        return READ_BLOCK_ERR;
    } else {
        // validate magic number
        if (buf[1] != 0x44) {
            printf("Error: Non-valid magic number in function: tfs_mount\n");
            return INVALID_MNUM_ERR;
        }
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
 * Opens or creates a new file. Updates OFT in
 * the process of creating a file
 */
fileDescriptor tfs_openFile(char *name) {
    fileDescriptor fd;
    FileEntry *newFE = malloc(sizeof(FileEntry));  // debug remember to free
    FileEntry *curr1 = headOFT;
    FileEntry *curr2 = headOFT;

    /* Check if disk is mounted and open mounted disk */
    if (mDisk == NULL) {
        return NO_DISK_MOUNTED_ERR;
    }

    /* Check if file exist in OFT LL -> if true, return fd */
    // -- note: if file is found in OFT, it is assumed that it has been opened
    while (curr1 != NULL) {
        if (strcmp(curr1->filename, name) == 0) {
            return curr1->fd;
        }
        curr1 = curr1->next;
    }

    /* Creating new file */
    // validate filename is within 8 characters
    if (strlen(name) > sizeof(newFE->filename) - 1) {
        printf("Error: Filename must be within 8 alphanumeric characters.\n");
        return FILENAME_ERR;
    }

    // open a new file for r/w operations
    if ((fd = open(name, O_RDWR | O_CREAT)) < 0) {
        printf("Error: Could not create file in function: openFile.\n");
        return OPEN_FILE_ERR;
    }

    // create file entry
    newFE->fd = fd;
    strcpy(newFE->filename, name);
    newFE->next = NULL;

    // add new file entry to OFT
    if (headOFT == NULL) {
        headOFT = newFE;
    } else {
        while (curr2->next != NULL) {
            curr2 = curr2->next;
        }
        curr2->next = newFE;
    }

    return fd;
}

/**
 * Cloes file. Removes file entry from OFT
 */
int tfs_closeFile(fileDescriptor fd) {
    /******* debug ********/
    // FileEntry *tmp = headOFT;
    // while (tmp != NULL) {
    //     printf("This file is in the OFT: %s\n", tmp->filename);
    //     tmp = tmp->next;
    // }
    /**********************/

    FileEntry *curr1 = headOFT;

    /* Check if disk is mounted and open mounted disk */
    if (mDisk == NULL) {
        return NO_DISK_MOUNTED_ERR;
    }

    /* Search and remove file entry from OFT */
    int foundFd = -1;
    if (curr1 != NULL) {
        // first check head of OFT
        if (curr1->fd == fd) {
            foundFd = 0;
            headOFT = curr1->next;
        } else {
            while (curr1->next != NULL) {
                if (curr1->next->fd == fd) {
                    // the next file entry is be removed
                    foundFd = 0;
                    FileEntry *rmvFE = curr1->next;
                    curr1->next = curr1->next->next;
                    free(rmvFE);
                } else {
                    curr1 = curr1->next;
                }
            }
        }

        // fd not found -> return error
        if (foundFd < 0) {
            return CLOSE_FILE_ERR;
        }
    } else {
        // nothing in OFT -> no file to close
        return CLOSE_FILE_ERR;
    }

    return TFS_CLOSE_FILE_SUCCESS;
}

/**
 * Write to file and update disk
 */
int tfs_writeFile(fileDescriptor fd, char *buffer, int size) {
    int diskFd;
    FileEntry *curr = headOFT;
    int startIdx;

    /* Check if disk is mounted and open mounted disk */
    if (mDisk == NULL) {
        return NO_DISK_MOUNTED_ERR;
    }

    /* Confirm fd is in OFT */
    int foundFd = -1;
    while (curr != NULL) {
        if (curr->fd == fd) {
            foundFd = 0;
        }
        curr = curr->next;
    }
    if (foundFd < 0) {
        return WRITE_FILE_ERR;
    }

    /*** Writing to an empty file ***/

    /* Open the disk to access super block */
    if ((diskFd = openDisk(mDisk, 0)) < 0) {
        printf("Error: Failed to open disk in function: tfs_mount\n");
        return OPEN_DISK_ERR;
    }

    /* Get start index of where to write in super block */
    if ((startIdx = getIndexToWrite(diskFd, size)) < 0) {
        return NO_SPACE_ERR;
    }

    // create inode for fd and write block

    // create file context and write blocks

    /* Write to file */

    return 0;
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

    // init free block bit vector
    char freeBlockArr[numBlocks];  // creating free block array
    memset(freeBlockArr, 'F',
           numBlocks);      // init free block array to all free
    freeBlockArr[0] = 'T';  // set first block to taken by super block

    // putting bit vector into super block
    memset(superBlock.data, 0, sizeof(superBlock.data));
    memcpy(superBlock.data, freeBlockArr, sizeof(freeBlockArr));

    // put super block into disk
    if ((tmp = writeBlock(fd, 0, &superBlock)) < 0) {
        printf("Error: Failed to write in function: setupFileSystems\n");
        return WRITE_BLOCK_ERR;
    }

    /* Init Free Blocks */
    FreeBlock freeBlock;
    freeBlock.type = 4;
    freeBlock.mNum = 0x44;

    memset(freeBlock.data, 0, sizeof(superBlock.data));

    for (int i = 1; i < numBlocks; i++) {
        if ((tmp = writeBlock(fd, i, &freeBlock)) < 0) {
            printf("Error: Failed to write in function: setupFileSystems\n");
            return WRITE_BLOCK_ERR;
        }
    }

    return 0;
}

/**
 * Checks if there is enough space in write in disk and retuns
 * index of where to start writing.
 */
int getIndexToWrite(int diskFd, int writeSize) {
    int tmp;
    char buf[BLOCKSIZE];
    char freeBlockArr[numBlocks];

    // get write size in blocks
    int writeBlockSize = (int)ceil((double)writeSize / (BLOCKSIZE - 2));

    // getting the free block array
    if ((tmp = readBlock(diskFd, 0, buf)) < 0) {
        printf("Error: Failed to read in function: tfs_writeBlock\n");
        return READ_BLOCK_ERR;
    } else {
        memcpy(freeBlockArr, buf + 2, sizeof(freeBlockArr));
    }

    // get index of where to start writing in disk
    int l = -1;
    int r = -1;
    for (int i = 0; i < strlen(freeBlockArr); i++) {
        if (freeBlockArr[i] == 'F') {
            if (l == -1) {
                // set the left boundary if it's the first 'F' encountered
                l = i;
            }
            r = i;
            // check if the window is at least writeBlockSize + 1
            if (r - l >= writeBlockSize + 1) {
                return l;
            }
        } else {
            // reset the boundaries if a 'T' is encountered
            l = -1;
            r = -1;
        }
    }
    // did not find available size
    return -1;

    //     for (int i = 0; i < sizeof(freeBlockArr); i++) {
    //     printf("%c, ", freeBlockArr[i]);
    // }
}