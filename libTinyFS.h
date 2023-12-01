#ifndef LIBTINYFS_H
#define LIBTINYFS_H

#include <stdio.h>
#include <string.h>
#include <stdlib.h>

#include "tinyFS.h"
#include "libDisk.h"

int tfs_mkfs (char *filename, int nBytes);

#endif /* LIBTINYFS_H*/