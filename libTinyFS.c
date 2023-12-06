#include "libTinyFS.h"

// Global Variables
int numBlocks;
char *mDisk = NULL;         // mounted disk
FileEntry *headOFT = NULL;  // head of OFT containing file entries

/**
 * Opens a new disk and initializes it with a super block
 * and free block to store future files
 */
int tfs_mkfs(char *filename, int nBytes) {
    int diskFd, tmp = 0;
    numBlocks = nBytes / BLOCKSIZE;

    if ((diskFd = openDisk(filename, nBytes)) < 0) {
        printf("> Error: Failed to open disk '%s'. Existed with status: %d\n",
               filename, OPEN_DISK_ERR);
        return OPEN_DISK_ERR;
    }

    // init empty buf (0x00)
    char *emptyBuf = calloc(BLOCKSIZE, sizeof(char));

    // populate disk w/ empty buf
    for (int i = 0; i < numBlocks; i++) {
        if ((tmp = writeBlock(diskFd, i, emptyBuf)) < 0) {
            printf(
                "Error: Failed to write to block. Existed with status: "
                "%d\n",
                WRITE_BLOCK_ERR);
            return WRITE_BLOCK_ERR;
        }
    }

    // free empty buf after writing to disk
    free(emptyBuf);

    // setup file system with super block and free blocks
    if ((tmp = setupFS(diskFd)) < 0) {
        printf("Error: Failed to setup tiny FS. Existed with status: %d\n",
               WRITE_BLOCK_ERR);
        return WRITE_BLOCK_ERR;
    }

    // log success
    printf("] Created new disk '%s' with status: %d\n", filename,
           TFS_MKFS_SUCCESS);
    return TFS_MKFS_SUCCESS;
}

/**
 * Set current disk being accessed to new disk
 */
int tfs_mount(char *diskname) {
    int diskFd, tmp = 0;
    char sbBuf[BLOCKSIZE];

    /* Unmount current disk if another disk is mounted */
    if (mDisk != NULL) {
        // log success
        printf("] Unmounted disk '%s' with status: %d\n", mDisk,
               TFS_UNMOUNT_SUCCESS);
        tfs_unmount();
    }

    /* Mount to new disk by opening the disk */
    if ((diskFd = openDisk(diskname, 0)) < 0) {
        printf("> Error: Failed to open disk '%s'. Exited with status: %d\n",
               diskname, OPEN_DISK_ERR);
        return OPEN_DISK_ERR;
    }

    /* Read super block metadata to confirms magic number */
    if ((tmp = readBlock(diskFd, 0, sbBuf)) < 0) {
        printf("> Error: Failed to read block. Exited with status: %d\n",
               READ_BLOCK_ERR);
        return READ_BLOCK_ERR;
    }

    /* Validate magic number */
    if (sbBuf[1] != 0x44) {
        printf("> Error: Invalid magic number. Exited with status: %d\n",
               INVALID_MNUM_ERR);
        return INVALID_MNUM_ERR;
    }

    /* Set mounted disk to new disk */
    mDisk = calloc(sizeof(char), strlen(diskname) + 1);
    strcpy(mDisk, diskname);

    /* Set numBlock according to new mounted disk size*/
    // TODO

    // log success
    printf("] Mounted to disk '%s' with status:: %d\n", mDisk,
           TFS_MOUNT_SUCCESS);
    return TFS_MOUNT_SUCCESS;
}

/**
 * Remove current disk being accessed
 */
int tfs_unmount() {
    free(mDisk);
    if (mDisk == NULL) {
        printf("] Nothing to unmount");
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
        printf("> Error: No Disk Mounted. Exited with status %d\n",
               NO_DISK_MOUNTED_ERR);
        return NO_DISK_MOUNTED_ERR;
    }

    /* Check if file exist in OFT LL -> if true, return fd */
    // if file is found in OFT, it is assumed that it has been opened
    while (curr1 != NULL) {
        if (strcmp(curr1->filename, name) == 0) {
            printf("] Opened file '%s' with fd: %d and status: %d\n", name,
                   curr1->fd, TFS_OPEN_FILE_SUCCESS);
            return curr1->fd;
        }
        curr1 = curr1->next;
    }

    /* Creating new file */
    // validate filename is within 8 characters
    if (strlen(name) > sizeof(newFE->filename) - 1) {
        printf(
            "> Error: Filename must be within 8 alphanumeric characters. "
            "Exited with status %d\n",
            FILENAME_ERR);
        return FILENAME_ERR;
    }

    // open a new file for r/w operations
    if ((fd = open(name, O_RDWR | O_CREAT)) < 0) {
        printf("> Error: Could not create file. Exited with status: %d\n",
               OPEN_FILE_ERR);
        return OPEN_FILE_ERR;
    }

    // create file entry
    newFE->fd = fd;
    strcpy(newFE->filename, name);
    newFE->filename[8] = '\0';
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

    // log success
    printf("] Opened file '%s' with fd: %d and status: %d\n", name, fd,
           TFS_OPEN_FILE_SUCCESS);
    return fd;
}

/**
 * Cloes file. Removes file entry from OFT
 */
int tfs_closeFile(fileDescriptor fd) {
    /******* debug ********/
    // FileEntry *tmp = headOFT;
    // while (tmp != NULL) {
    //     printf("This file is in the OFT: '%s'\n", tmp->filename);
    //     tmp = tmp->next;
    // }
    /**********************/
    char rmvFile[9];
    FileEntry *rmvFE = NULL;
    FileEntry *curr1 = headOFT;

    /* Check if disk is mounted and open mounted disk */
    if (mDisk == NULL) {
        printf("> Error: No Disk Mounted. Exited with status %d\n",
               NO_DISK_MOUNTED_ERR);
        return NO_DISK_MOUNTED_ERR;
    }

    /* Search and remove file entry from OFT */
    int foundFd = -1;
    if (curr1 != NULL) {
        // first check head of OFT
        if (curr1->fd == fd) {
            foundFd = 0;
            rmvFE = headOFT;
            headOFT = curr1->next;

            memcpy(rmvFile, rmvFE->filename, strlen(rmvFE->filename));
            free(rmvFE);
        } else {
            while (curr1->next != NULL) {
                if (curr1->next->fd == fd) {
                    // the next file entry is be removed
                    foundFd = 0;
                    rmvFE = curr1->next;
                    curr1->next = curr1->next->next;

                    memcpy(rmvFile, rmvFE->filename, strlen(rmvFE->filename));
                    free(rmvFE);
                } else {
                    curr1 = curr1->next;
                }
            }
        }

        // fd not found -> return > Error
        if (foundFd < 0) {
            printf("> Error: File is not open. Exited with status %d\n",
                   CLOSE_FILE_ERR);
            return CLOSE_FILE_ERR;
        }
    } else {
        // nothing in OFT -> no file to close
        printf("> Error: File is not open. Exited with status %d\n",
               CLOSE_FILE_ERR);
        return CLOSE_FILE_ERR;
    }

    printf("] Closed file '%s' with status: %d\n", rmvFile,
           TFS_CLOSE_FILE_SUCCESS);
    return TFS_CLOSE_FILE_SUCCESS;
}

/**
 * Write to file and update disk
 */
int tfs_writeFile(fileDescriptor fd, char *buffer, int size) {
    int tmp;
    int diskFd;
    int ibIndex;
    int fcbLen;
    char filename[9];
    FileEntry *curr = headOFT;
    SuperBlock sBlock;
    InodeBlock iBlock;

    /* Check if disk is mounted and open it */
    if (mDisk == NULL) {
        return NO_DISK_MOUNTED_ERR;
    } else {
        // open mounted disk
        if ((diskFd = openDisk(mDisk, 0)) < 0) {
            printf(
                "> Error: Failed to open disk '%s'. Existed with status: %d\n",
                mDisk, OPEN_DISK_ERR);
            return OPEN_DISK_ERR;
        }
    }

    /* Confirm fd is in OFT and get assoicate filename */
    int foundFd = -1;
    while (curr != NULL) {
        if (curr->fd == fd) {
            foundFd = 0;
            strcpy(filename, curr->filename);  // getting filename
        }
        curr = curr->next;
    }
    if (foundFd < 0) {
        printf("> Error: File is not open. Existed with status: %d\n",
               WRITE_FILE_ERR);
        return WRITE_FILE_ERR;
    }

    /* Get write size in terms of blocks */
    fcbLen = (int)ceil((double)size / (BLOCKSIZE - 2));

    // /* Get metadata from super block */
    // if ((tmp = readBlock(diskFd, 0, &sBlock)) < 0) {
    //     printf("> Error: Failed to read block. Existed with status: %d\n ",
    //            READ_BLOCK_ERR);
    //     return READ_BLOCK_ERR;
    // }

    // /* Initial check to see if can write to file */
    // if ((ibIndex = getStartBlock(fcbLen, sBlock.dMap)) < 0) {
    //     printf("> Error: No space to write to disk\n");
    //     return NO_SPACE_ERR;
    // }
    // printf("] Writing inode to block %d\n", ibIndex);  // debug

    /* Check if inode exists -> Remove inode and FCBs */
    tmp = removeInAndFcb(diskFd, filename);

    /* Get updated metadata from super block after deletion */
    if ((tmp = readBlock(diskFd, 0, &sBlock)) < 0) {
        printf("> Error: Failed to read block. Existed with status: %d\n ",
               READ_BLOCK_ERR);
        return READ_BLOCK_ERR;
    }

    /* Get start update index of where to write in super block after deletion */
    if ((ibIndex = getStartBlock(fcbLen, sBlock.dMap)) < 0) {
        printf("> Error: No space to write to disk\n");
        return NO_SPACE_ERR;
    }
    printf("] Writing inode to block %d\n", ibIndex);  // debug

    /* Create inode for fd and write block */
    iBlock.type = 2;
    iBlock.mNum = 0x44;
    strcpy(iBlock.filename, filename);
    iBlock.fSize = size;
    iBlock.fcbLen = fcbLen;
    iBlock.fp = 0;
    iBlock.posInDsk = ibIndex;
    memset(iBlock.data, 0, sizeof(iBlock.data));

    /* Update disk map with new inode */
    sBlock.dMap[ibIndex] = 'I';

    /* Write inode block into disk */
    if ((tmp = writeBlock(diskFd, ibIndex, &iBlock)) < 0) {
        printf("> Error: Failed to write block. Exited with status: %d\n",
               WRITE_BLOCK_ERR);
        return WRITE_BLOCK_ERR;
    }

    /* Write file context blocks to disk */
    int fcbIndex = ibIndex + 1;
    size_t offset = 0;

    while (offset < size) {
        FileContextBlock fcBlock;
        fcBlock.type = 3;
        fcBlock.mNum = 0x44;

        memset(fcBlock.context, 0, sizeof(fcBlock.context));
        size_t ctxSize;
        if (size - offset > sizeof(fcBlock.context)) {
            ctxSize = sizeof(fcBlock.context);
        } else {
            ctxSize = size - offset;
        }
        memcpy(fcBlock.context, buffer + offset, ctxSize);

        // write the fcbBlock to disk
        if (writeBlock(diskFd, fcbIndex, &fcBlock) < 0) {
            printf("> Error: Failed to write block. Exited with status: %d\n",
                   WRITE_BLOCK_ERR);
        }

        // update disk map with file context blocks
        sBlock.dMap[fcbIndex] = 'C';

        // move to the next block
        fcbIndex++;
        offset += sizeof(fcBlock.context);
    }

    /* Update super block w/inode */
    if ((tmp = writeBlock(diskFd, 0, &sBlock)) < 0) {
        printf("> Error: Failed to write block. Exited with status: %d\n",
               WRITE_BLOCK_ERR);
        return WRITE_BLOCK_ERR;
    }

    printf("Just wrote inode\n");
    for (int i = 0; i < numBlocks; i++) {
        printf("%c", sBlock.dMap[i]);
        if ((i + 1) % 8 == 0) {
            printf("\n");
        }
    }

    /* Write to actual file */
    if ((tmp = write(fd, buffer, size)) < 0) {
        printf("> Error: Failed to write to file. Exited with status: %d\n",
               WRITE_FILE_ERR);
        return WRITE_FILE_ERR;
    }

    // log success
    printf("] Sucessfully wrote to '%s' with status: %d\n", filename,
           TFS_WRITE_FILE_SUCCESS);

    return TFS_WRITE_FILE_SUCCESS;
}

/**
 * Delete file (must be open) and update disk
 */
int tfs_deleteFile(fileDescriptor fd) {
    int tmp;
    int diskFd;
    int foundFd = -1;
    char filename[9];
    FileEntry *curr = headOFT;

    /* Check if disk is mounted and open it */
    if (mDisk == NULL) {
        return NO_DISK_MOUNTED_ERR;
    } else {
        // open mounted disk
        if ((diskFd = openDisk(mDisk, 0)) < 0) {
            printf(
                "> Error: Failed to open disk '%s'. Existed with status: %d\n",
                mDisk, OPEN_DISK_ERR);
            return OPEN_DISK_ERR;
        }
    }

    /* Confirm fd is in OFT, get assoicate filename, close the file */
    while (curr != NULL) {
        if (curr->fd == fd) {
            foundFd = 0;
            strcpy(filename, curr->filename);  // getting filename
            tmp = tfs_closeFile(fd);           // close file before deletion
            break;
        }
        curr = curr->next;
    }
    if (foundFd < 0) {
        printf("> Error: File is not open. Existed with status: %d\n",
               DELETE_FILE_ERR);
        return DELETE_FILE_ERR;
    }

    /* Remove inode and associated FCBs*/
    if ((tmp = removeInAndFcb(diskFd, filename)) < 0) {
        printf("> Error: Could not delete '%s'. Existed with status: %d\n",
               filename, DELETE_FILE_ERR);
        return DELETE_FILE_ERR;
    }

    /* Delete actual file */
    if (remove(filename) < 0) {
        printf("> Error: Could not delete '%s'. Existed with status: %d\n",
               filename, DELETE_FILE_ERR);
    }
    // log success
    printf("] Sucessfully deleted '%s' with status: %d\n", filename,
           TFS_DELETE_FILE_SUCCESS);
    return TFS_DELETE_FILE_SUCCESS;
}

/**
 * Reads one byte from file and coppies it into buffer.
 */
int tfs_readByte(fileDescriptor fd, char *buffer) {
    int tmp;
    int diskFd;
    int fp;
    int fSize;
    int fcbIndex;
    int foundIn = -1;
    char filename[9];
    FileEntry *curr = headOFT;
    SuperBlock sBlock;
    InodeBlock iBlock;
    FileContextBlock tmpFCB;

    /* Check if disk is mounted and open it */
    if (mDisk == NULL) {
        return NO_DISK_MOUNTED_ERR;
    } else {
        // open mounted disk
        if ((diskFd = openDisk(mDisk, 0)) < 0) {
            printf(
                "> Error: Failed to open disk '%s'. Existed with status: %d\n",
                mDisk, OPEN_DISK_ERR);
            return OPEN_DISK_ERR;
        }
    }

    /* Confirm fd is in OFT and get assoicate filename */
    int foundFd = -1;
    while (curr != NULL) {
        if (curr->fd == fd) {
            foundFd = 0;
            strcpy(filename, curr->filename);  // getting filename
        }
        curr = curr->next;
    }
    if (foundFd < 0) {
        printf("> Error: File is not open. Existed with status: %d\n",
               WRITE_FILE_ERR);
        return WRITE_FILE_ERR;
    }

    /* Get metadata from super block */
    if ((tmp = readBlock(diskFd, 0, &sBlock)) < 0) {
        printf("> Error: Failed to read block. Existed with status: %d\n ",
               READ_BLOCK_ERR);
        return READ_BLOCK_ERR;
    }

    /* Get inode to know file size and fp */
    for (int i = 0; i < numBlocks; i++) {
        if (sBlock.dMap[i] == 'I') {
            if ((tmp = readBlock(diskFd, i, &iBlock)) < 0) {
                printf(
                    "> Error: Failed to read block. Exited with status: %d\n",
                    READ_BLOCK_ERR);
                return READ_BLOCK_ERR;
            }
            if (strcmp(iBlock.filename, filename) == 0) {
                foundIn = 0;
                fp = iBlock.fp;
                fSize = iBlock.fSize;
                fcbIndex = iBlock.posInDsk + 1;
                break;
            }
        }
    }

    /* Read byte */
    if (foundIn == 0) {
        // copying fcb into temp buffer
        int offset = 0;
        char tmpBuf[iBlock.fcbLen * (BLOCKSIZE - 2)];  // 254 bytes
        for (int i = 0; i < iBlock.fcbLen; i++) {
            if ((tmp = readBlock(diskFd, fcbIndex, &tmpFCB)) < 0) {
                printf(
                    "> Error: Failed to read block. Exited with status: %d\n",
                    READ_BLOCK_ERR);
                return READ_BLOCK_ERR;
            }
            memcpy(tmpBuf + offset, tmpFCB.context, sizeof(tmpFCB.context));
            fcbIndex++;
            offset += sizeof(tmpFCB.context);
        }

        // Check if fp did not exceed file size -> copy byte at fp to buffer
        if (fp < fSize) {
            *buffer = tmpBuf[fp];
            fp++;

            // update fp in inode block in disk
            iBlock.fp = (char)fp;
            if ((tmp = writeBlock(diskFd, iBlock.posInDsk, &iBlock)) < 0) {
                printf(
                    "> Error: Failed to write block. Exited with status: %d\n",
                    WRITE_BLOCK_ERR);
                return WRITE_BLOCK_ERR;
            }
        } else {
            printf("> Error: Failed to read byte. Exited with status: %d\n",
                   READ_BYTE_ERR);
            return READ_BYTE_ERR;
        }
    }

    return TFS_READ_BYTE_SUCESS;
}

/**
 * seek function
 */
int tfs_seek(fileDescriptor fd, int offset) {
    int tmp;
    int diskFd;
    int size;
    char filename[9];
    FileEntry *curr = headOFT;
    SuperBlock sBlock;
    /* Check if disk is mounted and open it */
    if (mDisk == NULL) {
        return NO_DISK_MOUNTED_ERR;
    } else {
        // open mounted disk
        if ((diskFd = openDisk(mDisk, 0)) < 0) {
            printf(
                "> Error: Failed to open disk '%s'. Existed with status: %d\n",
                mDisk, OPEN_DISK_ERR);
            return OPEN_DISK_ERR;
        }
    }

    /* Confirm fd is in OFT and get assoicate filename */
    int foundFd = -1;
    while (curr != NULL) {
        if (curr->fd == fd) {
            foundFd = 0;
            strcpy(filename, curr->filename);  // getting filename
        }
        curr = curr->next;
    }
    if (foundFd < 0) {
        printf("> Error: File is not open. Existed with status: %d\n",
               WRITE_FILE_ERR);
        return WRITE_FILE_ERR;
    }

    /* Get metadata from super block */
    if ((tmp = readBlock(diskFd, 0, &sBlock)) < 0) {
        printf("> Error: Failed to read block. Existed with status: %d\n ",
               READ_BLOCK_ERR);
        return READ_BLOCK_ERR;
    }

    /* Find inode to get size of file */
    int foundIn = -1;
    InodeBlock tmpIn;
    for (int i = 0; i < numBlocks; i++) {
        if (sBlock.dMap[i] == 'I') {
            if ((tmp = readBlock(diskFd, i, &tmpIn)) < 0) {
                printf(
                    "> Error: Failed to read block. Exited with status: %d\n",
                    READ_BLOCK_ERR);
                return READ_BLOCK_ERR;
            }
            if (strcmp(tmpIn.filename, filename) == 0) {
                foundIn = 0;
                size = tmpIn.fSize;
                break;
            }
        }
    }

    if (foundIn == 0) {
        /* Check if seek is valid */
        if (offset > size) {
            return INVALID_SEEK_ERR;
        }

        tmpIn.fp = (char)offset;

        // Update fp in inode block in disk
        if ((tmp = writeBlock(diskFd, tmpIn.posInDsk, &tmpIn)) < 0) {
            printf(
                "> Error: Failed to write block. Exited with status: %d\n",
                WRITE_BLOCK_ERR);
            return WRITE_BLOCK_ERR;
        }
    }
    return TFS_SEEK_FILE_SUCCESS;
}

/*********************** Helper Functions ***********************/

/**
 * Initializes super block and free blocks and writes the blocks
 * into the recently opened disk
 */
int setupFS(int diskFd) {
    int tmp = 0;

    /* Init Super Block */
    SuperBlock sBlock;
    sBlock.type = 1;
    sBlock.mNum = 0x44;

    // init disk map bit vector
    char dMap[numBlocks];
    memset(dMap, 'F', numBlocks);
    dMap[0] = 'S';  // first block is super block

    // putting bit vector into super block
    memset(sBlock.dMap, 0, sizeof(sBlock.dMap));
    memcpy(sBlock.dMap, dMap, sizeof(dMap));

    // put super block into disk
    if ((tmp = writeBlock(diskFd, 0, &sBlock)) < 0) {
        printf("> Error: Failed to write block. Exited with status: %d\n",
               WRITE_BLOCK_ERR);
        return WRITE_BLOCK_ERR;
    }

    /* Init Free Blocks */
    FreeBlock fBlock;
    fBlock.type = 4;
    fBlock.mNum = 0x44;

    memset(fBlock.data, 0, sizeof(fBlock.data));

    // put free blocks into disk
    for (int i = 1; i < numBlocks; i++) {
        if ((tmp = writeBlock(diskFd, i, &fBlock)) < 0) {
            printf(
                "> Error: Failed to write block. Exited with status: "
                "%d\n",
                WRITE_BLOCK_ERR);
            return WRITE_BLOCK_ERR;
        }
    }

    return 0;
}

/**
 * Remove by overwriting inode and FCB with free blocks
 */
int removeInAndFcb(int diskFd, char *filename) {
    int tmp;
    int rmvIbIndex;
    int rmvBlocks;
    int foundIn = -1;
    SuperBlock sBlock;
    InodeBlock tmpIn;

    /* Get metadata from super block */
    if ((tmp = readBlock(diskFd, 0, &sBlock)) < 0) {
        printf("> Error: Failed to read block. Existed with status: %d\n ",
               READ_BLOCK_ERR);
        return READ_BLOCK_ERR;
    }

    /* Get inode and FCBs to delete */
    for (int i = 0; i < numBlocks; i++) {
        if (sBlock.dMap[i] == 'I') {
            if ((tmp = readBlock(diskFd, i, &tmpIn)) < 0) {
                printf(
                    "> Error: Failed to read block. Exited with "
                    "status: %d\n",
                    READ_BLOCK_ERR);
                return READ_BLOCK_ERR;
            }
            if (strcmp(tmpIn.filename, filename) == 0) {
                foundIn = 0;
                rmvIbIndex = i;
                rmvBlocks = tmpIn.fcbLen + 1;  // add one bc inode
                break;
            }
        }
    }

    /* Delete inode and associated FCBs */
    if (foundIn == 0) {
        FreeBlock fBlock;
        fBlock.type = 4;
        fBlock.mNum = 0x44;
        memset(fBlock.data, 0, sizeof(fBlock.data));

        for (int i = 0; i < rmvBlocks; i++) {
            if (writeBlock(diskFd, rmvIbIndex, &fBlock) < 0) {
                printf(
                    "> Error: Failed to write block. Exited with "
                    "status: %d\n",
                    WRITE_BLOCK_ERR);
                return WRITE_BLOCK_ERR;
            }

            // update disk map with new free block
            sBlock.dMap[rmvIbIndex] = 'F';

            // move to the next block
            rmvIbIndex++;
        }
    } else {
        return -1;
    }

    // debug
    printf("Just deleted: \n");
    for (int i = 0; i < numBlocks; i++) {
        printf("%c", sBlock.dMap[i]);
        if ((i + 1) % 8 == 0) {
            printf("\n");
        }
    }

    /* Update super block w/ new free blocks */
    if ((tmp = writeBlock(diskFd, 0, &sBlock)) < 0) {
        printf("> Error: Failed to write block. Exited with status: %d\n",
               WRITE_BLOCK_ERR);
        return WRITE_BLOCK_ERR;
    }

    return 0;
}

/**
 * Checks if there is enough space in write in disk and retuns
 * index of where to start writing.
 */
int getStartBlock(int fcbLen, char dMap[]) {
    // get index of where to start writing in disk
    int l = -1;
    int r = -1;
    for (int i = 0; i < numBlocks; i++) {
        if (dMap[i] == 'F') {
            if (l == -1) {
                // set the left boundary if it's the first 'F'
                // encountered
                l = i;
            }
            r = i;
            // check if the window is at least writeBlockSize + 1
            if ((r - l + 1) >= fcbLen + 1) {
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
}