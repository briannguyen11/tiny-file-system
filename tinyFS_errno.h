#ifndef TINYFSERRNO_H
#define TINYFSERRNO_H

// Success Status
#define TFS_MKFS_SUCCESS 200
#define TFS_MOUNT_SUCCESS 201
#define TFS_UNMOUNT_SUCCESS 202
#define TFS_OPEN_FILE_SUCCESS 203
#define TFS_CLOSE_FILE_SUCCESS 204
#define TFS_WRITE_FILE_SUCCESS 205
#define TFS_SEEK_FILE_SUCCESS 206
#define TFS_DELETE_FILE_SUCCESS 207
#define TFS_READ_BYTE_SUCESS 208
#define TFS_RENAME_FILE_SUCCESS 209
#define TFS_DISPLAY_MAP_SUCCESS 210
#define TFS_READDIR_SUCCESS 211

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
#define DELETE_FILE_ERR -409
#define NO_SPACE_ERR -410
#define READ_BYTE_ERR -411
#define INVALID_SEEK_ERR -412

#endif /* TINYFSERRNO_H*/