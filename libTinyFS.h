#ifndef LIBTINYFS_H
#define LIBTINYFS_H

#include <stdio.h>
#include <string.h>
#include <stdlib.h>

#include "tinyFS.h"
#include "libDisk.h"
#include "tinyFS_errno.h"

#define SUPER_BLOCK 1
#define MAGIC_NUM 0x44

int tfs_mkfs (char *filename, int nBytes);

#endif /* LIBTINYFS_H*/