#include "libTinyFS.h"

int main() {
    int res = 0;
    // res = tfs_mkfs(DEFAULT_DISK_NAME, DEFAULT_DISK_SIZE);
    // printf("%d\n", res);

    // res = tfs_mkfs("DISK_0", DEFAULT_DISK_SIZE);
    // printf("%d\n", res);

    res = tfs_mount(DEFAULT_DISK_NAME);
    printf("%d\n", res);
    if (res < 0) {
        res = tfs_mkfs(DEFAULT_DISK_NAME, DEFAULT_DISK_SIZE);
        printf("%d\n", res);

        res = tfs_mkfs("tinyFSDiskRand", DEFAULT_DISK_SIZE);
        printf("%d\n", res);

        // assumes tfs_mkfs worked
        res = tfs_mount(DEFAULT_DISK_NAME);
        printf("%d\n", res);

        res = tfs_mount("tinyFSDiskRand");
        printf("%d\n", res);
    }

    res = tfs_openFile("file1");
    printf("%d\n", res);
    return 0;
}