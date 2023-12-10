#include "libTinyFS.h"

// Global Variables
char *mDisk = NULL;         // mounted disk
FileEntry *headOFT = NULL;  // head of OFT containing file entries

/*
 * Opens a new disk and initializes it with a super block
 * and free block to store future files
 */
int tfs_mkfs(char *filename, int nBytes) {
    int diskFd;
    int numBlocks = nBytes / BLOCKSIZE;

    if ((diskFd = openDisk(filename, nBytes)) < 0) {
        printf("> Failed to open disk. Exited mkfs() with status: %d\n",
               OPEN_DISK_ERR);
        return OPEN_DISK_ERR;
    }

    // init empty buf (0x00)
    char *emptyBuf = calloc(BLOCKSIZE, sizeof(char));

    // populate disk w/ empty buf
    for (int i = 0; i < numBlocks; i++) {
        if ((writeBlock(diskFd, i, emptyBuf)) < 0) {
            printf(
                "> Failed to write block. Exited mkfs() with status: "
                "%d\n",
                WRITE_BLOCK_ERR);
            return WRITE_BLOCK_ERR;
        }
    }

    // free empty buf after writing to disk
    free(emptyBuf);

    // setup file system with super block and free blocks
    if ((setupFS(diskFd, numBlocks)) < 0) {
        printf("> Failed to write block. Exited mkfs() with status: %d\n",
               WRITE_BLOCK_ERR);
        return WRITE_BLOCK_ERR;
    }

    // log success
    printf("] Created new disk '%s'\n", filename);
    return 0;
}

/*
 * Set current disk being accessed to new disk
 */
int tfs_mount(char *diskname) {
    int diskFd;
    char sbBuf[BLOCKSIZE];

    /* Unmount current disk if another disk is mounted */
    if (mDisk != NULL) {
        tfs_unmount();
    }

    /* Mount to new disk by opening the disk */
    if ((diskFd = openDisk(diskname, 0)) < 0) {
        printf("> Failed to open disk. Exited mount() with status: %d\n",
               OPEN_DISK_ERR);
        return OPEN_DISK_ERR;
    }

    /* Read super block metadata to confirms magic number */
    if ((readBlock(diskFd, 0, sbBuf)) < 0) {
        printf("> Failed to read block. Exited mount() with status: %d\n",
               READ_BLOCK_ERR);
        return READ_BLOCK_ERR;
    }

    /* Validate magic number */
    if (sbBuf[1] != 0x44) {
        printf(
            "> Failed to verify magic number. Exited mount() with status: %d\n",
            INVALID_MNUM_ERR);
        return INVALID_MNUM_ERR;
    }

    /* Set mounted disk to new disk */
    mDisk = calloc(sizeof(char), strlen(diskname) + 1);
    strcpy(mDisk, diskname);

    // log success
    printf("] Mounted to disk '%s'\n", mDisk);
    return 0;
}

/*
 * Remove current disk being accessed
 */
int tfs_unmount() {
    if (mDisk == NULL) {
        printf("] Nothing to unmount");
    }
    printf("] Unmounted to '%s'\n", mDisk);
    free(mDisk);
    return 0;
}

/*
 * Opens or creates a new file. Updates OFT in
 * the process of creating a file
 */
fileDescriptor tfs_openFile(char *name) {
    fileDescriptor fd;
    time_t initTime;
    FileEntry *newFE = malloc(sizeof(FileEntry));  // debug remember to free
    FileEntry *curr1 = headOFT;
    FileEntry *curr2 = headOFT;

    /* Check if disk is mounted and open mounted disk */
    if (mDisk == NULL) {
        printf("> Failed to open disk. Exited openFile() with status: %d\n",
               NO_DISK_MOUNTED_ERR);
        return NO_DISK_MOUNTED_ERR;
    }

    /* Check if file exist in OFT LL -> if true, return fd */
    // if file is found in OFT, it is assumed that it has been opened
    while (curr1 != NULL) {
        if (strcmp(curr1->filename, name) == 0) {
            printf("] Opened file '%s' with fd: %d\n", name, curr1->fd);
            return curr1->fd;
        }
        curr1 = curr1->next;
    }

    /* Creating new file */
    // validate filename is within 8 characters
    if (strlen(name) > sizeof(newFE->filename) - 1) {
        printf(
            "> Filename must be within 8 alphanumeric characters. "
            "Exited openFile() with status %d\n",
            FILENAME_ERR);
        return FILENAME_ERR;
    }

    // open a new file for r/w operations
    if ((fd = open(name, O_RDWR | O_CREAT, S_IRUSR | S_IWUSR)) < 0) {
        printf("> Failed to open file. Exited openFile() with status: %d\n",
               OPEN_FILE_ERR);
        return OPEN_FILE_ERR;
    }

    // create file entry
    newFE->fd = fd;
    strcpy(newFE->filename, name);
    newFE->filename[8] = '\0';
    newFE->next = NULL;
    time(&initTime);
    newFE->initTime = initTime;

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
    printf("] Opened file '%s' with fd: %d\n", name, fd);
    return fd;
}

/*
 * Cloes file. Removes file entry from OFT
 */
int tfs_closeFile(fileDescriptor fd) {
    char rmvFile[9];
    FileEntry *rmvFE = NULL;
    FileEntry *curr1 = headOFT;

    /* Check if disk is mounted and open mounted disk */
    if (mDisk == NULL) {
        printf(
            "> Failed to open disk. Exited closeFile() with status: "
            "%d\n",
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
            printf(
                "> File not in OFT. Exited closeFile() with status: "
                "%d\n",
                CLOSE_FILE_ERR);
            return CLOSE_FILE_ERR;
        }
    } else {
        // nothing in OFT -> no file to close
        printf("> File not in OFT. Exited closeFile() with status: %d\n",
               CLOSE_FILE_ERR);
        return CLOSE_FILE_ERR;
    }

    printf("] Closed file '%s'\n", rmvFile);
    return 0;
}

/*
 * Write to file and update disk
 */

int tfs_writeFile(fileDescriptor fd, char *buffer, int size) {
    int diskFd;
    int ibIndex;
    int fcbLen;
    char filename[9];
    int rdOnlyFlg = -1;
    time_t initTime;
    time_t newTime;
    SuperBlock sBlock;
    InodeBlock iBlock;

    /* Check if disk is mounted and open it */
    if (mDisk == NULL) {
        return NO_DISK_MOUNTED_ERR;
    } else {
        // open mounted disk
        if ((diskFd = openDisk(mDisk, 0)) < 0) {
            printf(
                "> Failed to open disk. Exited writeFile() with "
                "status: "
                "%d\n",
                OPEN_DISK_ERR);
            return OPEN_DISK_ERR;
        }
    }

    /* Confirm fd is in OFT and get associated filename */
    int foundFd = -1;
    FileEntry *curr = headOFT;
    while (curr != NULL) {
        if (curr->fd == fd) {
            foundFd = 0;
            strcpy(filename, curr->filename);  // getting filename and init time
            initTime = curr->initTime;
        }
        curr = curr->next;
    }

    if (foundFd < 0) {
        printf(
            "> File not in OFT. Exited writeFile() with status: "
            "%d\n",
            WRITE_FILE_ERR);
        return WRITE_FILE_ERR;
    }

    /* Get write size in terms of blocks */
    fcbLen = (int)ceil((double)size / (BLOCKSIZE - 2));

    /* Get metadata from super block */
    if ((readBlock(diskFd, 0, &sBlock)) < 0) {
        printf(
            "> Failed to read block. Exited writeFile() with status: "
            "%d\n ",
            READ_BLOCK_ERR);
        return READ_BLOCK_ERR;
    }

    /* Check if inode exists */
    int foundIn = -1;
    InodeBlock tmpIn;
    for (int i = 0; i < sBlock.numBlocks; i++) {
        if (sBlock.dMap[i] == 'I') {
            if ((readBlock(diskFd, i, &tmpIn)) < 0) {
                printf(
                    "> Failed to read block. Exited writeFile() with "
                    "status: %d\n",
                    READ_BLOCK_ERR);
                return READ_BLOCK_ERR;
            }
            if (strcmp(tmpIn.filename, filename) == 0) {
                foundIn = 0;
                rdOnlyFlg = tmpIn.rdOnly;
                break;
            }
        }
    }

    /* If read only flag is set -> return with error */
    if (rdOnlyFlg == 0) {
        printf(
            "> File '%s' is READ only. Exited writeFile() with status: "
            "%d\n",
            filename, READ_ONLY_ERR);
        return READ_ONLY_ERR;
    }

    /* If inode exist -> store inode and fcb as backup */
    char backup[BLOCKSIZE * (tmpIn.fcbLen + 1)];
    if (foundIn == 0) {
        int rdIdx = tmpIn.posInDsk;
        char buf[BLOCKSIZE];
        size_t offset = 0;

        for (int i = 0; i < tmpIn.fcbLen + 1; i++) {
            if ((readBlock(diskFd, rdIdx, &buf)) < 0) {
                printf(
                    "> Failed to read block. Exited writeFile() with "
                    "status: "
                    "%d\n",
                    READ_BLOCK_ERR);
                return READ_BLOCK_ERR;
            }
            memcpy(backup + offset, buf, BLOCKSIZE);
            offset += BLOCKSIZE;
            rdIdx += 1;
        }

        /* Remove inode and associate fcbs */
        if (removeInAndFcb(diskFd, filename) < 0) {
            printf(
                "> Failed to write to file. Exited writeFile() with "
                "status: "
                "%d\n",
                WRITE_FILE_ERR);
            return WRITE_FILE_ERR;
        };
    }

    /* Get metadata from super block after deletion */
    if (readBlock(diskFd, 0, &sBlock) < 0) {
        printf(
            "> Failed to read block. Exited writeFile() with status: "
            "%d\n ",
            READ_BLOCK_ERR);
        return READ_BLOCK_ERR;
    }

    /* Get start update index of where to write in super block after deletion */
    if ((ibIndex = getStartBlock(fcbLen, sBlock.dMap, sBlock.numBlocks)) < 0) {
        if (foundIn == 0) {
            // if no space -> write the backup buf back to disk and update dMap
            int wrIdx = tmpIn.posInDsk;
            char buf[BLOCKSIZE];
            size_t offset = 0;
            for (int i = 0; i < tmpIn.fcbLen + 1; i++) {
                memcpy(buf, backup + offset, BLOCKSIZE);

                // update disk
                if (writeBlock(diskFd, wrIdx, &buf) < 0) {
                    printf(
                        "> Failed to write block. Exited writeFile() "
                        "with status: "
                        "%d\n",
                        WRITE_BLOCK_ERR);
                    return WRITE_BLOCK_ERR;
                }

                // update dMap in super block
                if (i == 0) {
                    sBlock.dMap[wrIdx] = 'I';
                } else {
                    sBlock.dMap[wrIdx] = 'C';
                }

                offset += BLOCKSIZE;
                wrIdx += 1;
            }

            // update disk with restored dMap in super block
            if (writeBlock(diskFd, 0, &sBlock) < 0) {
                printf(
                    "> Failed to write block. Exited writeFile() with "
                    "status: %d\n",
                    WRITE_BLOCK_ERR);
                return WRITE_BLOCK_ERR;
            }
        }
        printf(
            "> No space to write. Exited writeFile() with status: "
            "%d\n",
            NO_SPACE_ERR);
        return NO_SPACE_ERR;
    }

    /* Get metadata from super block after deletion */
    if (readBlock(diskFd, 0, &sBlock) < 0) {
        printf(
            "> Failed to read block. Exited writeFile() with status: "
            "%d\n ",
            READ_BLOCK_ERR);
        return READ_BLOCK_ERR;
    }

    /* Create inode for fd and write block */
    iBlock.type = 2;
    iBlock.mNum = 0x44;
    strcpy(iBlock.filename, filename);
    iBlock.fSize = size;
    iBlock.fcbLen = fcbLen;
    iBlock.fp = 0;
    iBlock.posInDsk = ibIndex;
    iBlock.rdOnly = -1;

    if (foundIn == -1) {
        iBlock.createTime = initTime;
        iBlock.modTime = initTime;
        iBlock.accessTime = initTime;
    } else {
        iBlock.createTime = initTime;
        time(&newTime);
        iBlock.modTime = newTime;
        iBlock.accessTime = newTime;
    }

    memset(iBlock.data, 0, sizeof(iBlock.data));

    /* Update disk map with new inode */
    sBlock.dMap[ibIndex] = 'I';

    /* Write inode block into disk */
    if (writeBlock(diskFd, ibIndex, &iBlock) < 0) {
        printf(
            "> Failed to write block. Exited writeFile() with status: "
            "%d\n",
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
            printf(
                "> Failed to write block. Exited writeFile() with "
                "status: %d\n",
                WRITE_BLOCK_ERR);
        }

        // update disk map with file context blocks
        sBlock.dMap[fcbIndex] = 'C';

        // move to the next block
        fcbIndex++;
        offset += sizeof(fcBlock.context);
    }

    /* Update super block w/inode */
    if (writeBlock(diskFd, 0, &sBlock) < 0) {
        printf(
            "> Failed to write block. Exited writeFile() with status: "
            "%d\n",
            WRITE_BLOCK_ERR);
        return WRITE_BLOCK_ERR;
    }

    // log success
    printf("] Wrote to '%s'\n", filename);

    return 0;
}

/*
 * Delete file (must be open) and update disk
 */
int tfs_deleteFile(fileDescriptor fd) {
    int diskFd;
    int rdOnlyFlg = -1;
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
                "> Failed to open disk. Exited deleteFile() with status: "
                "%d\n",
                OPEN_DISK_ERR);
            return OPEN_DISK_ERR;
        }
    }

    /* Confirm fd is in OFT, get assoicate filename, close the file */
    int foundFd = -1;
    while (curr != NULL) {
        if (curr->fd == fd) {
            foundFd = 0;
            strcpy(filename, curr->filename);  // getting filename
            break;
        }
        curr = curr->next;
    }
    if (foundFd < 0) {
        printf("> File not in OFT. Exited deleteFile() with status: %d\n",
               DELETE_FILE_ERR);
        return DELETE_FILE_ERR;
    }

    /* Get metadata from super block */
    if (readBlock(diskFd, 0, &sBlock) < 0) {
        printf(
            "> Failed to read block. Exited deleteFile() with status: "
            "%d\n ",
            READ_BLOCK_ERR);
        return READ_BLOCK_ERR;
    }

    /* Check if inode exists */
    InodeBlock tmpIn;
    for (int i = 0; i < sBlock.numBlocks; i++) {
        if (sBlock.dMap[i] == 'I') {
            if (readBlock(diskFd, i, &tmpIn) < 0) {
                printf(
                    "> Failed to read block. Exited deleteFile() with "
                    "status: %d\n",
                    READ_BLOCK_ERR);
                return READ_BLOCK_ERR;
            }
            if (strcmp(tmpIn.filename, filename) == 0) {
                rdOnlyFlg = tmpIn.rdOnly;
                break;
            }
        }
    }

    /* If read only flag is set -> return with error */
    if (rdOnlyFlg == 0) {
        printf(
            "> File '%s' is READ only. Exited deleteFile() with status: "
            "%d\n",
            filename, READ_ONLY_ERR);
        return READ_ONLY_ERR;
    }

    /* Close file in OFT */
    tfs_closeFile(fd);

    /* Remove inode and associated FCBs */
    if (removeInAndFcb(diskFd, filename) < 0) {
        printf(
            "> Failed to remove blocks. Exited deleteFile() with status: %d\n",
            DELETE_FILE_ERR);
        return DELETE_FILE_ERR;
    }

    /* Delete actual file */
    if (remove(filename) < 0) {
        printf("> Failed to delete file. Exited deleteFile() with status: %d\n",
               DELETE_FILE_ERR);
    }
    // log success
    printf("] Deleted '%s'\n", filename);
    return 0;
}

/*
 * Reads one byte from file and coppies it into buffer.
 */
int tfs_readByte(fileDescriptor fd, char *buffer) {
    int diskFd;
    int fp;
    int fSize;
    int fcbIndex;
    int foundIn = -1;
    char filename[9];
    time_t newTime;
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
                "> Failed to open disk. Exited readByte() with status: "
                "%d\n",
                OPEN_DISK_ERR);
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
        printf("> File not in OFT. Exited readByte() with status: %d\n",
               READ_BYTE_ERR);
        return READ_BYTE_ERR;
    }

    /* Get metadata from super block */
    if (readBlock(diskFd, 0, &sBlock) < 0) {
        printf("> Failed to read block. Exited readByte() with status: %d\n ",
               READ_BLOCK_ERR);
        return READ_BLOCK_ERR;
    }

    /* Get inode to know file size and fp */
    for (int i = 0; i < sBlock.numBlocks; i++) {
        if (sBlock.dMap[i] == 'I') {
            if (readBlock(diskFd, i, &iBlock) < 0) {
                printf(
                    "> Failed to read block. Exited readByte() with status: "
                    "%d\n",
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
            if (readBlock(diskFd, fcbIndex, &tmpFCB) < 0) {
                printf(
                    "> Failed to read block. Exited readByte() with status: "
                    "%d\n",
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
            iBlock.fp = fp;
            time(&newTime);
            iBlock.accessTime = newTime;
            if (writeBlock(diskFd, iBlock.posInDsk, &iBlock) < 0) {
                printf(
                    "> Failed to write block. Exited readByte() with status: "
                    "%d\n",
                    WRITE_BLOCK_ERR);
                return WRITE_BLOCK_ERR;
            }
        } else {
            printf("\n> Exited readByte() with status: %d\n", READ_BYTE_ERR);
            return READ_BYTE_ERR;
        }
    } else {
        printf("> Exited readByte() with status: %d\n", READ_BYTE_ERR);
        return READ_BYTE_ERR;
    }

    return 0;
}

/*
 * Moves fp to desired offset
 */
int tfs_seek(fileDescriptor fd, int offset) {
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
                "> Failed to open disk. Exited seek() status: "
                "%d\n",
                OPEN_DISK_ERR);
            return OPEN_DISK_ERR;
        }
    }

    /* Confirm fd is in OFT and get associated filename */
    int foundFd = -1;
    while (curr != NULL) {
        if (curr->fd == fd) {
            foundFd = 0;
            strcpy(filename, curr->filename);  // getting filename
        }
        curr = curr->next;
    }
    if (foundFd < 0) {
        printf("> File not in OFT. Exited seek() status:  %d\n",
               INVALID_SEEK_ERR);
        return INVALID_SEEK_ERR;
    }

    /* Get metadata from super block */
    if (readBlock(diskFd, 0, &sBlock) < 0) {
        printf("> Failed to read block. Exited seek() status:  %d\n ",
               READ_BLOCK_ERR);
        return READ_BLOCK_ERR;
    }

    /* Find inode to get size of file */
    int foundIn = -1;
    InodeBlock tmpIn;
    for (int i = 0; i < sBlock.numBlocks; i++) {
        if (sBlock.dMap[i] == 'I') {
            if (readBlock(diskFd, i, &tmpIn) < 0) {
                printf(
                    "> Failed to read block. Exited seek() status: "
                    "%d\n",
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
            printf("> Invalid seek. Exited seek() status: %d\n",
                   INVALID_SEEK_ERR);
            return INVALID_SEEK_ERR;
        }

        tmpIn.fp = offset;

        // Update fp in inode block in disk
        if (writeBlock(diskFd, tmpIn.posInDsk, &tmpIn) < 0) {
            printf("> Failed to write block. Exited seek() status: %d\n",
                   WRITE_BLOCK_ERR);
            return WRITE_BLOCK_ERR;
        }
    }
    printf("] Seeked '%s'\n", filename);
    return 0;
}

/******************* Additional Functionality *******************/

/*
 * Renames an open file
 */
int tfs_rename(fileDescriptor fd, char *newName) {
    int diskFd;
    char oldFilename[9];
    FileEntry *curr = headOFT;
    SuperBlock sBlock;
    InodeBlock iBlock;

    /* Ensure new name is within 8 character */
    if (strlen(newName) > 8) {
        printf(
            "> Filename must be within 8 alphanumeric characters. "
            "Exited openFile() with status %d\n",
            FILENAME_ERR);
        return FILENAME_ERR;
    }

    /* Check if disk is mounted and open it */
    if (mDisk == NULL) {
        return NO_DISK_MOUNTED_ERR;
    } else {
        // open mounted disk
        if ((diskFd = openDisk(mDisk, 0)) < 0) {
            printf("> Failed to open disk. Exited rename() with status: %d\n",
                   OPEN_DISK_ERR);
            return OPEN_DISK_ERR;
        }
    }

    /* Confirm fd is in OFT and get associated filename */
    int foundFd = -1;
    while (curr != NULL) {
        if (curr->fd == fd) {
            foundFd = 0;
            strcpy(
                oldFilename,
                curr->filename);  // copy old filename to use for inode search
            strcpy(curr->filename, newName);  // updates filename in OFT
            break;
        }
        curr = curr->next;
    }
    if (foundFd < 0) {
        printf("> File not in OFT. Exited rename() with status: %d\n",
               FILENAME_ERR);
        return FILENAME_ERR;
    }

    /* Get metadata from super block */
    if (readBlock(diskFd, 0, &sBlock) < 0) {
        printf("> Failed to read block. Exited rename() with status: %d\n ",
               READ_BLOCK_ERR);
        return READ_BLOCK_ERR;
    }

    /* Find inode */
    for (int i = 0; i < sBlock.numBlocks; i++) {
        if (sBlock.dMap[i] == 'I') {
            if (readBlock(diskFd, i, &iBlock) < 0) {
                printf(
                    "> Failed to read block. Exited rename() with status: "
                    "%d\n",
                    READ_BLOCK_ERR);
                return READ_BLOCK_ERR;
            }
            if (strcmp(iBlock.filename, oldFilename) == 0) {
                strcpy(iBlock.filename, newName);
                // Update filename in inode block in disk
                if (writeBlock(diskFd, iBlock.posInDsk, &iBlock) < 0) {
                    printf(
                        "> Failed to write block. Exited rename() with status: "
                        "%d\n",
                        WRITE_BLOCK_ERR);
                    return WRITE_BLOCK_ERR;
                }
                break;
            }
        }
    }

    printf("] Renamed '%s' to '%s'\n", oldFilename, iBlock.filename);
    return 0;
}

/*
 * Prints filename of every file in the directory (disk)
 */
int tfs_readdir() {
    int diskFd;
    SuperBlock sBlock;
    FileEntry *curr = headOFT;
    /* Check if disk is mounted and open it */
    if (mDisk == NULL) {
        return NO_DISK_MOUNTED_ERR;
    } else {
        // open mounted disk
        if ((diskFd = openDisk(mDisk, 0)) < 0) {
            printf(
                "> Error: Failed to open disk '%s'. Exited with status: %d\n",
                mDisk, OPEN_DISK_ERR);
            return OPEN_DISK_ERR;
        }
    }

    /* Get metadata from super block */
    if (readBlock(diskFd, 0, &sBlock) < 0) {
        printf("> Error: Failed to read block. Existed with status: %d\n ",
               READ_BLOCK_ERR);
        return READ_BLOCK_ERR;
    }

    /* Get filenames from OFT */
    if (curr == NULL) {
        printf("No filenames to print\n");
        return 0;
    }
    while (curr != NULL) {
        printf("          %s          ", curr->filename);
        curr = curr->next;
        if (curr == NULL) {
            printf("\n");
        }
    }
    return 0;
}

/*
 * Displays map of disk blocks labeled by S,I,C and F which stands
 * for super block, inode, file context, and free blocks
 */
int tfs_displayFragments() {
    int diskFd;
    SuperBlock sBlock;
    /* Check if disk is mounted and open it */
    if (mDisk == NULL) {
        return NO_DISK_MOUNTED_ERR;
    } else {
        // open mounted disk
        if ((diskFd = openDisk(mDisk, 0)) < 0) {
            printf(
                "> Failed to open disk. Exited displayFragments() with "
                "status: "
                "%d\n",
                OPEN_DISK_ERR);
            return OPEN_DISK_ERR;
        }
    }

    /* Get metadata from super block */
    if (readBlock(diskFd, 0, &sBlock) < 0) {
        printf(
            "> Failed to read block. Exited displayFragments() with "
            "status: "
            " %d\n ",
            READ_BLOCK_ERR);
        return READ_BLOCK_ERR;
    }

    printf("] Disk Overview: \n");
    for (int i = 0; i < sBlock.numBlocks; i++) {
        printf("%c", sBlock.dMap[i]);
        if ((i + 1) % 8 == 0) {
            printf("\n");
        }
    }
    printf("\n");

    return 0;
}

/*
 * Shifts all blocks to the left to reduce external
 * fragmentation, leaving free blocks at the end
 */
int tfs_defrag() {
    int diskFd;
    SuperBlock sBlock;
    /* Check if disk is mounted and open it */
    if (mDisk == NULL) {
        return NO_DISK_MOUNTED_ERR;
    } else {
        // open mounted disk
        if ((diskFd = openDisk(mDisk, 0)) < 0) {
            printf(
                "> Failed to open disk. Exited defrag() with status: "
                "%d\n",
                OPEN_DISK_ERR);
            return OPEN_DISK_ERR;
        }
    }

    /* Get metadata from super block */
    if (readBlock(diskFd, 0, &sBlock) < 0) {
        printf("> Failed to read block. Exited defrag() with status: %d\n ",
               READ_BLOCK_ERR);
        return READ_BLOCK_ERR;
    }

    /* Iterate over disk map -> shift taken blocks to where free blocks are */
    char buf[BLOCKSIZE];
    int wrIdx = 0;

    for (int rdIdx = 0; rdIdx < sBlock.numBlocks; rdIdx++) {
        // if the current element is not 'F', shift it to the write index
        if (sBlock.dMap[rdIdx] != 'F') {
            // update disk map
            sBlock.dMap[wrIdx] = sBlock.dMap[rdIdx];

            // update disk by moving blocks
            if (readBlock(diskFd, rdIdx, &buf) < 0) {
                printf(
                    "> Failed to read block. Exited defrag() with status: %d\n",
                    READ_BLOCK_ERR);
                return READ_BLOCK_ERR;
            }
            if (writeBlock(diskFd, wrIdx, &buf) < 0) {
                printf(
                    "> Failed to write block. Exited defrag() with status: "
                    "%d\n",
                    WRITE_BLOCK_ERR);
                return WRITE_BLOCK_ERR;
            }

            wrIdx++;
        }
    }

    // fill the remaining slots with free blocks
    while (wrIdx < sBlock.numBlocks) {
        // update disk map
        sBlock.dMap[wrIdx] = 'F';

        // update disk by wiriting free blocks
        FreeBlock fBlock;
        fBlock.type = 4;
        fBlock.mNum = 0x44;
        memset(fBlock.data, 0, sizeof(fBlock.data));

        if (writeBlock(diskFd, wrIdx, &fBlock) < 0) {
            return WRITE_BLOCK_ERR;
        }

        wrIdx++;
    }

    if (writeBlock(diskFd, 0, &sBlock) < 0) {
        printf(
            "> Failed to write block. Exited defrag() with status: "
            "> Error: Failed in defrag(). Exited with "
            "status: "
            "%d\n",
            WRITE_BLOCK_ERR);
        return WRITE_BLOCK_ERR;
    }

    printf("] Resolved fragmentation\n");
    return 0;
}

/*
 * Makes a file read only
 */
int tfs_makeRO(char *name) {
    int diskFd;
    int foundIn = -1;
    SuperBlock sBlock;
    InodeBlock iBlock;

    /* Check if disk is mounted and open it */
    if (mDisk == NULL) {
        return NO_DISK_MOUNTED_ERR;
    } else {
        // open mounted disk
        if ((diskFd = openDisk(mDisk, 0)) < 0) {
            printf(
                "> Failed to open disk. Exited makeRO() with "
                "status: "
                "%d\n",
                OPEN_DISK_ERR);
            return OPEN_DISK_ERR;
        }
    }

    /* Get metadata from super block */
    if (readBlock(diskFd, 0, &sBlock) < 0) {
        printf("> Failed to read block. Exited makeRO() with status: %d\n ",
               READ_BLOCK_ERR);
        return READ_BLOCK_ERR;
    }

    /* Find inode */
    for (int i = 0; i < sBlock.numBlocks; i++) {
        if (sBlock.dMap[i] == 'I') {
            if (readBlock(diskFd, i, &iBlock) < 0) {
                printf(
                    "> Failed to read block. Exited makeRO() with status: "
                    "%d\n",
                    READ_BLOCK_ERR);
                return READ_BLOCK_ERR;
            }
            if (strcmp(iBlock.filename, name) == 0) {
                foundIn = 0;
                // set to read only
                iBlock.rdOnly = 0;

                // write inode back to disk
                if (writeBlock(diskFd, iBlock.posInDsk, &iBlock) < 0) {
                    printf(
                        "> Failed to write block. Exited makeRO() with status: "
                        "%d\n",
                        WRITE_BLOCK_ERR);
                    return WRITE_BLOCK_ERR;
                }
                // log status
                printf("] Made '%s' access rights to READ only\n", name);
                break;
            }
        }
    }

    if (foundIn < 0) {
        printf("Failed to make '%s' READ only\n", name);
    }

    return 0;
}

/*
 * Makes a file read and write
 */
int tfs_makeRW(char *name) {
    int diskFd;
    int foundIn = -1;
    SuperBlock sBlock;
    InodeBlock iBlock;

    /* Check if disk is mounted and open it */
    if (mDisk == NULL) {
        return NO_DISK_MOUNTED_ERR;
    } else {
        // open mounted disk
        if ((diskFd = openDisk(mDisk, 0)) < 0) {
            printf(
                "> Failed to open disk. Exited makeRW() with "
                "status: "
                "%d\n",
                OPEN_DISK_ERR);
            return OPEN_DISK_ERR;
        }
    }

    /* Get metadata from super block */
    if (readBlock(diskFd, 0, &sBlock) < 0) {
        printf("> Failed to read block. Exited makeRW() with status: %d\n ",
               READ_BLOCK_ERR);
        return READ_BLOCK_ERR;
    }

    /* Find inode */
    for (int i = 0; i < sBlock.numBlocks; i++) {
        if (sBlock.dMap[i] == 'I') {
            if (readBlock(diskFd, i, &iBlock) < 0) {
                printf(
                    "> Failed to read block. Exited makeRW() with status: "
                    "%d\n",
                    READ_BLOCK_ERR);
                return READ_BLOCK_ERR;
            }
            if (strcmp(iBlock.filename, name) == 0) {
                foundIn = 0;
                // set to read and write
                iBlock.rdOnly = -1;

                // write inode back to disk
                if (writeBlock(diskFd, iBlock.posInDsk, &iBlock) < 0) {
                    printf(
                        "> Failed to write block. Exited makeRW() with status: "
                        "%d\n",
                        WRITE_BLOCK_ERR);
                    return WRITE_BLOCK_ERR;
                }
                // log status
                printf("] Made '%s' access rights to READ and WRITE\n", name);
                break;
            }
        }
    }

    if (foundIn < 0) {
        printf("Failed to make '%s' READ only\n", name);
    }
    return 0;
}

/*
 * Write byte to file at fp location
 */
int tfs_writeByte(fileDescriptor fd, uint8_t data) {
    int diskFd;
    int fp;
    int fSize;
    int fcbIndex;
    int foundIn = -1;
    char filename[9];
    time_t newTime;
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
                "> Failed to open disk. Exited writeByte() with status: "
                "%d\n",
                OPEN_DISK_ERR);
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
        printf("> File not in OFT. Exited writeByte() with status: %d\n",
               READ_BYTE_ERR);
        return READ_BYTE_ERR;
    }

    /* Get metadata from super block */
    if (readBlock(diskFd, 0, &sBlock) < 0) {
        printf("> Failed to read block. Exited writeByte() with status: %d\n ",
               READ_BLOCK_ERR);
        return READ_BLOCK_ERR;
    }

    /* Get inode to know file size and fp */
    for (int i = 0; i < sBlock.numBlocks; i++) {
        if (sBlock.dMap[i] == 'I') {
            if (readBlock(diskFd, i, &iBlock) < 0) {
                printf(
                    "> Failed to read block. Exited writeByte() with status: "
                    "%d\n",
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

    /* Write byte */
    if (foundIn == 0) {
        // copying fcb into temp buffer
        int offset = 0;
        int rdIdx = fcbIndex;
        char tmpBuf[iBlock.fcbLen * (BLOCKSIZE - 2)];  // 254 bytes
        for (int i = 0; i < iBlock.fcbLen; i++) {
            if (readBlock(diskFd, rdIdx, &tmpFCB) < 0) {
                printf(
                    "> Failed to read block. Exited writeByte() with status: "
                    "%d\n",
                    READ_BLOCK_ERR);
                return READ_BLOCK_ERR;
            }
            memcpy(tmpBuf + offset, tmpFCB.context, sizeof(tmpFCB.context));
            rdIdx++;
            offset += sizeof(tmpFCB.context);
        }

        // Check if fp did not exceed file size -> copy byte at fp to buffer
        if (fp < fSize) {
            tmpBuf[fp] = data;
            fp++;

            // update time
            time(&newTime);
            iBlock.modTime = newTime;
            iBlock.accessTime = newTime;

            // update fp in inode block in disk
            iBlock.fp = fp;
            if (writeBlock(diskFd, iBlock.posInDsk, &iBlock) < 0) {
                printf(
                    "> Failed to write block. Exited writeByte() with status: "
                    "%d\n",
                    WRITE_BLOCK_ERR);
                return WRITE_BLOCK_ERR;
            }

            // update file context blocks in disk
            offset = 0;
            int wrIdx = fcbIndex;
            for (int i = 0; i < iBlock.fcbLen; i++) {
                FileContextBlock fcBlock;
                fcBlock.type = 3;
                fcBlock.mNum = 0x44;
                memcpy(fcBlock.context, tmpBuf + offset,
                       sizeof(fcBlock.context));

                if (writeBlock(diskFd, wrIdx, &fcBlock) < 0) {
                    printf(
                        "> Failed to write block. Exited writeByte() with "
                        "status: "
                        "%d\n",
                        WRITE_BLOCK_ERR);
                    return WRITE_BLOCK_ERR;
                }
                wrIdx++;
                offset += sizeof(fcBlock.context);
            }
        } else {
            printf(
                "> Stopped writing byte '%c'. Exited writeByte() with status: "
                "%d\n",
                data, WRITE_BYTE_ERR);
            return WRITE_BYTE_ERR;
        }
    } else {
        printf(
            "> Stopped writing byte '%c'. Exited writeByte() with status: "
            "%d\n",
            data, WRITE_BYTE_ERR);
        return WRITE_BYTE_ERR;
    }

    return 0;
}
/*********************** Helper Functions ***********************/

/*
 * Prints create, modify, and access time for a file
 */
int tfs_readFileInfo(fileDescriptor fd) {
    int diskFd;
    int foundIn = -1;
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
                "> Failed to open disk. Exited in readFileInfo() with status: "
                "%d\n",
                OPEN_DISK_ERR);
            return OPEN_DISK_ERR;
        }
    }

    /* Confirm fd is in OFT and get associated filename */
    int foundFd = -1;
    while (curr != NULL) {
        if (curr->fd == fd) {
            foundFd = 0;
            strcpy(filename, curr->filename);  // getting filename
        }
        curr = curr->next;
    }
    if (foundFd < 0) {
        printf("> File not in OFT. Exited readFileInfo() with status: %d\n",
               WRITE_FILE_ERR);
        return WRITE_FILE_ERR;
    }

    /* Get metadata from super block */
    if ((readBlock(diskFd, 0, &sBlock)) < 0) {
        printf(
            "> Failed to read block. Exited readFileInfo() with status: %d\n ",
            READ_BLOCK_ERR);
        return READ_BLOCK_ERR;
    }

    /* Find inode to get size of file */
    InodeBlock tmpIn;
    for (int i = 0; i < sBlock.numBlocks; i++) {
        if (sBlock.dMap[i] == 'I') {
            if ((readBlock(diskFd, i, &tmpIn)) < 0) {
                printf(
                    "> Failed to read block. Exited readFileInfo() with "
                    "status: %d\n ",
                    READ_BLOCK_ERR);
                return READ_BLOCK_ERR;
            }
            if (strcmp(tmpIn.filename, filename) == 0) {
                foundIn = 0;
                struct tm *cTimeInfo = localtime(&tmpIn.createTime);

                printf("File Create Time: %d-%02d-%02d %02d:%02d:%02d\n",
                       cTimeInfo->tm_year + 1900, cTimeInfo->tm_mon + 1,
                       cTimeInfo->tm_mday, cTimeInfo->tm_hour,
                       cTimeInfo->tm_min, cTimeInfo->tm_sec);

                struct tm *mTimeInfo = localtime(&tmpIn.modTime);

                printf("File Modif Time:  %d-%02d-%02d %02d:%02d:%02d\n",
                       mTimeInfo->tm_year + 1900, mTimeInfo->tm_mon + 1,
                       mTimeInfo->tm_mday, mTimeInfo->tm_hour,
                       mTimeInfo->tm_min, mTimeInfo->tm_sec);

                struct tm *aTimeInfo = localtime(&tmpIn.accessTime);

                printf("File Access Time: %d-%02d-%02d %02d:%02d:%02d\n",
                       aTimeInfo->tm_year + 1900, aTimeInfo->tm_mon + 1,
                       aTimeInfo->tm_mday, aTimeInfo->tm_hour,
                       aTimeInfo->tm_min, aTimeInfo->tm_sec);
                break;
            }
        }
    }
    if (foundIn < 0) {
        printf(
            "> Failed because inode created after write operation. Failed in "
            "readFileInfo()\n");
    }
    return 0;
}

/*
 * Initializes super block and free blocks and writes the blocks
 * into the recently opened disk
 */
int setupFS(int diskFd, int numBlocks) {
    /* Init Super Block */
    SuperBlock sBlock;
    sBlock.type = 1;
    sBlock.mNum = 0x44;
    sBlock.numBlocks = numBlocks;

    // init disk map bit vector
    char dMap[numBlocks];
    memset(dMap, 'F', numBlocks);
    dMap[0] = 'S';  // first block is super block

    // putting bit vector into super block
    memset(sBlock.dMap, 0, sizeof(sBlock.dMap));
    memcpy(sBlock.dMap, dMap, sizeof(dMap));

    // put super block into disk
    if (writeBlock(diskFd, 0, &sBlock) < 0) {
        return WRITE_BLOCK_ERR;
    }

    /* Init Free Blocks */
    FreeBlock fBlock;
    fBlock.type = 4;
    fBlock.mNum = 0x44;

    memset(fBlock.data, 0, sizeof(fBlock.data));

    // put free blocks into disk
    for (int i = 1; i < numBlocks; i++) {
        if (writeBlock(diskFd, i, &fBlock) < 0) {
            return WRITE_BLOCK_ERR;
        }
    }

    return 0;
}

/*
 * Remove by overwriting inode and FCB with free blocks
 */
int removeInAndFcb(int diskFd, char *filename) {
    int rmvIbIndex;
    int rmvBlocks;
    int foundIn = -1;
    SuperBlock sBlock;
    InodeBlock tmpIn;

    /* Get metadata from super block */
    if (readBlock(diskFd, 0, &sBlock) < 0) {
        return READ_BLOCK_ERR;
    }

    /* Get inode and FCBs to delete */
    for (int i = 0; i < sBlock.numBlocks; i++) {
        if (sBlock.dMap[i] == 'I') {
            if (readBlock(diskFd, i, &tmpIn) < 0) {
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

    /* Update super block w/ new free blocks */
    if (writeBlock(diskFd, 0, &sBlock) < 0) {
        return WRITE_BLOCK_ERR;
    }

    return 0;
}

/*
 * Checks if there is enough space in write in disk and retuns
 * index of where to start writing.
 */
int getStartBlock(int fcbLen, char dMap[], int numBlocks) {
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