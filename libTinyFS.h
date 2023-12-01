#ifndef LIBTINYFS_H
#define LIBTINYFS_H

#include <stdio.h>
#include <string.h>
#include <stdlib.h>

#include "tinyFS.h"
#include "libDisk.h"
#include "tinyFS_errno.h"

typedef struct SuperBlock {
   char type;
   char mNum;
   char data[BLOCKSIZE - 2];
} SuperBlock;

int tfs_mkfs (char *filename, int nBytes);

#endif /* LIBTINYFS_H*/