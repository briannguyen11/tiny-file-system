#include "libTinyFS.h"

int main(){
    int res = tfs_mkfs("disk0.dsk", 0);
    printf("%d\n", res);
    return 0;
}