#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include <strings.h>
#include <string.h>
#include <fcntl.h>
#include <assert.h>
#include <errno.h>
#include <math.h>

#define KASSERT0(expr,msg) assert(expr)

#include <geekos/gfs3.h>
#include <geekos/pfat.h>
/* pretend that we're in the kernel. */
#include <geekos/vfs.h>
#include <geekos/blockdev.h>
#include <geekos/bitset.h>
#include <geekos/bufcache.h>
#include <geekos/screen.h>
#define TODO(x) printf(x)
#include <geekos/projects.h>

// #define Debug(x...)
#define Debug(x...) printf(x)

struct gfs3_superblock the_super;



int main(int argc, const char *argv[]) {
    int bootsect_fd;

    if(argc < 2) {
        printf
            ("usage: gfs2f image-file-name bootsector numblocks files-to-include\n");
        exit(EXIT_FAILURE);
    }

    if((bootsect_fd = open(argv[2], 0)) < 0) {
        fprintf(stderr, "couldn't open boot sector");
        exit(EXIT_FAILURE);
    }

    the_super.blocks_per_disk = atoi(argv[3]);
    if(the_super.blocks_per_disk < 10) {
        printf("unsupported (too few) blocks_per_disk: %s\n", argv[3]);
        exit(EXIT_FAILURE);
    }

    TODO_P(PROJECT_GFS3,
           "User mode formatter outside geekos; not for fall 2015");
    exit(EXIT_SUCCESS);
}
