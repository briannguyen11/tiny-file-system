#include "libTinyFS.h"

int main(){
    int res = tfs_mkfs(DEFAULT_DISK_NAME, DEFAULT_DISK_SIZE);
    printf("%d\n", res);
    return 0;
}