#ifndef TINYFSERRNO_H
#define TINYFSERRNO_H

// Success Status
#define TFS_MKFS_SUCCESS 200
#define TFS_MOUNT_SUCCESS 201
#define TFS_UNMOUNT_SUCCESS 201
#define TFS_CLOSE_FILE_SUCCESS 202

// Error Status
#define OPEN_DISK_ERR -400
#define WRITE_BLOCK_ERR -401
#define READ_BLOCK_ERR -402
#define INVALID_MNUM_ERR -403
#define NO_DISK_MOUNTED_ERR -404
#define FILENAME_ERR -405
#define OPEN_FILE_ERR -406
#define CLOSE_FILE_ERR -407
#define WRITE_FILE_ERR -408
#define NO_SPACE_ERR -409

#endif /* TINYFSERRNO_H*/