#include <geekos/gfs2.h>
#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include <strings.h>
#include <string.h>
#include <fcntl.h>
#include <assert.h>
#include <errno.h>
#include <math.h>

/* pretend that we're in the kernel. */
#define GEEKOS
#include <geekos/vfs.h>
#include <geekos/blockdev.h>
#include <geekos/bitset.h>

#define Debug(x...)
// #define Debug(x...) printf(x)

struct gfs2_superblock the_super;


int main(int argc, const char *argv[]) {

    if(argc < 2) {
        printf
            ("usage: gfs2f image-file-name blocksize numblocks files-to-include\n");
        exit(EXIT_FAILURE);
    }

    the_super.block_size = atoi(argv[2]);
    if(the_super.block_size != 512 &&
       the_super.block_size != 1024 && the_super.block_size != 4096) {
        printf("unsupported blocksize: %s\n", argv[2]);
        exit(EXIT_FAILURE);
    }

    the_super.blocks_per_disk = atoi(argv[3]);
    if(the_super.blocks_per_disk < 10) {
        printf("unsupported blocks_per_disk: %s\n", argv[3]);
        exit(EXIT_FAILURE);
    }

    exit(EXIT_SUCCESS);
}
