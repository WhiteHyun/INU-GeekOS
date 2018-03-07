#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/mman.h>
#include <fcntl.h>
#include <string.h>
#include <stdarg.h>
#include <errno.h>
#include <assert.h>

#ifndef GEEKOS
#define GEEKOS
#endif

#define KASSERT0(expr,msg) assert(expr)
/* void KASSERT(int expr); * fake */

#include <geekos/screen.h>
#include <geekos/synch.h>
#include <geekos/vfs.h>

int submitTesting;

void *Alloc_Page() {
    return malloc(4096);
}
void Free_Page(void *pg) {
    free(pg);
}
void *Malloc(size_t size) {
    return (malloc(size));
}
void Free(void *m) {
    free(m);
}

void Print(const char *fmt, ...) {
    va_list args;
    va_start(args, fmt);
    vprintf(fmt, args);
    va_end(args);
}

struct Kernel_Thread *g_currentThread = NULL;

void Set_Current_Attr(unsigned char attr __attribute__ ((unused))) {
    /* noop */
}

// enum { MUTEX_UNLOCKED, MUTEX_LOCKED };
// struct Mutex {
//    int state;
//    struct Kernel_Thread* owner;
//    struct Thread_Queue waitQueue;
// };

void Mutex_Init(struct Mutex *mutex) {
    mutex->state = MUTEX_UNLOCKED;
    mutex->owner = (struct Kernel_Thread *)get_current_thread(0);
}
void Mutex_Lock(struct Mutex *mutex) {
    KASSERT(mutex->state == MUTEX_UNLOCKED);
    mutex->state = MUTEX_LOCKED;
}
void Mutex_Unlock(struct Mutex *mutex) {
    KASSERT(mutex->state == MUTEX_LOCKED);
    mutex->state = MUTEX_UNLOCKED;
}

// struct Condition {
//     struct Thread_Queue waitQueue;
// };

void Cond_Init(struct Condition *cond) {
    (void)cond;
}
void Cond_Wait(struct Condition *cond, struct Mutex *mutex) {
    (void)cond;
    (void)mutex;
}
void Cond_Signal(struct Condition *cond) {
    (void)cond;
}
void Cond_Broadcast(struct Condition *cond) {
    (void)cond;
}

#undef KASSERT0
void KASSERT0(int expr, const char *message) {
    if(!expr)
        Print("%s", message);
    assert(expr);
}
void Exit(int code) {
    printf("terminating %d\n", code);
    exit(code);
}

// void KASSERT(int expr) {
//   assert(expr);
// }
struct Kernel_Thread *get_current_thread(int atomic
                                         __attribute__ ((unused))) {
    return (void *)0x1000;
}
void Hardware_Shutdown(void) {
    exit(EXIT_FAILURE);
}


int device_fd;
/* not actually using mmap, just extra complexity for no fun. */
void *device_mmap;
void setup_device_mmap(char *filename, int blocksize
                       __attribute__ ((unused)), int pages
                       __attribute__ ((unused))) {
    if(device_fd == 0) {
        device_fd = open(filename, O_RDWR);
        if(device_fd < 0) {
            fprintf(stderr, "failed to open %s: %s", filename,
                    strerror(errno));
            exit(EXIT_FAILURE);
        }
        //  device_mmap = mmap(NULL, blocksize * pages, PROT_READ|PROT_WRITE, MAP_SHARED, device_fd, 0);
        //   KASSERT(device_mmap);
        //   KASSERT(device_mmap != (void *)-1);
        //   char c = *((char *)(device_mmap));
        //   *((char *)(device_mmap)) = c;
    }
}

// from filio.h, which redefines things I need defined normally
#define SECTOR_SIZE 512
struct Block_Device;

int Block_Write(struct Block_Device *dev __attribute__ ((unused)),
                int block_index, void *block_data) {
    assert(block_data);
//      assert(device_mmap);
    // fprintf(stderr, "block write: %d from %p\n", block_index, block_data);
    // setup_device_mmap();

    lseek(device_fd, block_index * SECTOR_SIZE, SEEK_SET);
    assert(write(device_fd, block_data, SECTOR_SIZE) > 0);
    // memcpy( device_mmap + block_index * SECTOR_SIZE, block_data, SECTOR_SIZE );
    return 0;
}

int Block_Read(struct Block_Device *dev __attribute__ ((unused)),
               int block_index, void *block_data) {

//      assert(device_mmap);
    assert(block_data);
    //  fprintf(stderr, "block read: %d into %p\n", block_index, block_data);
    // setup_device_mmap();
    // memcpy( block_data, device_mmap + block_index * SECTOR_SIZE, SECTOR_SIZE );
    lseek(device_fd, block_index * SECTOR_SIZE, SEEK_SET);
    assert(read(device_fd, block_data, SECTOR_SIZE) > 0);
    return 0;

}

void assertion_failed_endless_loop(void) {
    abort();
}

struct File *Allocate_File(struct File_Ops *ops, int filePos, int endPos,
                           void *fsData, int mode,
                           struct Mount_Point *mountPoint) {
    struct File *ret = (struct File *)malloc(sizeof(struct File));
    ret->ops = ops;
    ret->filePos = filePos;
    ret->endPos = endPos;
    ret->fsData = fsData;
    ret->mode = mode;
    ret->mountPoint = mountPoint;
    return ret;
}

struct Filesystem_Ops *gfs_ops;

bool Register_Filesystem(const char *fsName
                         __attribute__ ((unused)),
                         struct Filesystem_Ops *fsOps) {
    gfs_ops = fsOps;
    return 1;
}
