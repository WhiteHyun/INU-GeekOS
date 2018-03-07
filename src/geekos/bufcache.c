/*
 * Filesystem buffer cache
 * Copyright (c) 2004, David H. Hovemeyer <daveho@cs.umd.edu>
 * $Revision: 1.14 $
 *
 * This is free software.  You are permitted to use,
 * redistribute, and modify it as specified in the file "COPYING".
 */

#include <geekos/errno.h>
#include <geekos/kassert.h>
#ifndef KASSERT0
#define KASSERT0(expr, message) KASSERT(expr)
#endif
#include <geekos/mem.h>
#include <geekos/malloc.h>
#include <geekos/blockdev.h>
#include <geekos/bufcache.h>
#include <geekos/screen.h>
#include <string.h>

/*
 * Maximum number of buffers that are cached per-filesystem.
 */
#define FS_BUFFER_CACHE_MAX_BLOCKS 128

/* ----------------------------------------------------------------------
 * Private functions
 * ---------------------------------------------------------------------- */

int bufCacheDebug = 0;
#define Debug(args...) if (bufCacheDebug) Print("bufcache: " args)

/* XXX */
int noEvict = 0;

/*
 * Get number of sectors per filesystem block for given
 * fs buffer cache.
 */
static uint_t Get_Num_Sectors_Per_FS_Block(struct FS_Buffer_Cache *cache) {
    return (cache->fsBlockSize / SECTOR_SIZE);
}

/*
 * Read or write a filesystem buffer.
 */
static int Do_Buffer_IO(struct FS_Buffer_Cache *cache,
                        struct FS_Buffer *buf,
                        int (*IO_Func) (struct Block_Device * dev,
                                        int blockNum, void *buf)) {
    uint_t offset;
    unsigned int sectorCount = 0;
    int blockNum = buf->fsBlockNum * Get_Num_Sectors_Per_FS_Block(cache);
    char *ptr = (char *)buf->data;

    for(offset = 0; offset < cache->fsBlockSize; offset += SECTOR_SIZE) {
        int rc = IO_Func(cache->dev, blockNum, ptr + offset);
        if(rc != 0)
            return rc;
        ++sectorCount;
        ++blockNum;
    }
    KASSERT(offset == cache->fsBlockSize);
    KASSERT(sectorCount == Get_Num_Sectors_Per_FS_Block(cache));

    return 0;
}

/*
 * If necessary, write back uncomitted buffer contents to block device.
 */
static int Sync_Buffer(struct FS_Buffer_Cache *cache,
                       struct FS_Buffer *buf) {
    int rc = 0;

    KASSERT(IS_HELD(&cache->mutex));

    if(buf->flags & FS_BUFFER_DIRTY) {
        if((rc = Do_Buffer_IO(cache, buf, Block_Write)) == 0)
            buf->flags &= ~(FS_BUFFER_DIRTY);
    }

    return rc;
}

/*
 * Move a buffer to the front of the cache buffer list,
 * to indicate that it has been used recently.
 */
static void Move_To_Front(struct FS_Buffer_Cache *cache,
                          struct FS_Buffer *buf) {
    Remove_From_FS_Buffer_List(&cache->bufferList, buf);
    Add_To_Front_Of_FS_Buffer_List(&cache->bufferList, buf);
}

/*
 * Get buffer for given block, and mark it in use.
 * Must be called with cache mutex held.
 */
static int Get_Buffer(struct FS_Buffer_Cache *cache,
                      ulong_t fsBlockNum, struct FS_Buffer **pBuf) {
    struct FS_Buffer *buf, *lru = 0;
    int rc;

    Debug("Request block %lu\n", fsBlockNum);

    KASSERT(IS_HELD(&cache->mutex));

    /*
     * Look for existing buffer.
     * As a side-effect, finds the least recently used
     * buffer that is not in use (if any).
     */
    for(buf = Get_Front_Of_FS_Buffer_List(&cache->bufferList);
        buf != 0; buf = Get_Next_In_FS_Buffer_List(buf)) {

        if(buf->fsBlockNum == fsBlockNum) {
            Debug("Found cached block %lu\n", fsBlockNum);
            /* If buffer is in use, wait until it is available. */
            while (buf->flags & FS_BUFFER_INUSE) {
                Debug("Waiting for in-use cached block %lu, RA=%lx\n",
                      fsBlockNum, (ulong_t) __builtin_return_address(0));
                Cond_Wait(&cache->cond, &cache->mutex);
            }
            goto done;
        }

        /* If buffer isn't in use, it's a candidate for LRU. */
        if(!(buf->flags & FS_BUFFER_INUSE))
            lru = buf;

    }

    /*
     * If number of allocated buffers does not exceed the
     * limit, allocate a new one.
     */
    if(cache->numCached < FS_BUFFER_CACHE_MAX_BLOCKS) {
        buf = (struct FS_Buffer *)Malloc(sizeof(*buf));
        if(buf != 0) {
            buf->data = Alloc_Page();   /* kinda lame, always allocate 4096 for data */
            if(buf->data == 0) {
                Free(buf);
            } else {
                /* Successful creation */
                buf->fsBlockNum = fsBlockNum;
                buf->flags = 0;
                Add_To_Front_Of_FS_Buffer_List(&cache->bufferList, buf);
                ++cache->numCached;
                goto readAndAcquire;
            }
        }
    }

    /*
     * If there is no LRU buffer, then we have exceeded
     * the number of available buffers.
     */
    if(lru == 0)
        return ENOMEM;

    KASSERT(!noEvict);

    /* Make sure the LRU buffer is clean. */
    if((rc = Sync_Buffer(cache, lru)) != 0)
        return rc;

    /* LRU buffer is clean, so we can steal it. */
    buf = lru;
    buf->flags = 0;
    buf->fsBlockNum = fsBlockNum;       /* ns */
    Move_To_Front(cache, buf);

  readAndAcquire:
    /*
     * The buffer selected should be clean (no uncommitted data),
     * and should have been moved to the front of the buffer list
     * (to show it has just been referenced).
     */
    KASSERT(!(buf->flags & FS_BUFFER_DIRTY));
    KASSERT(Get_Front_Of_FS_Buffer_List(&cache->bufferList) == buf);

    /* Read block data into buffer. */
    if((rc = Do_Buffer_IO(cache, buf, Block_Read)) != 0)
        return rc;

  done:
    /* Buffer is now in use. */
    buf->flags |= FS_BUFFER_INUSE;
    /* Success! */
    Debug("Acquired block %lu\n", fsBlockNum);
    *pBuf = buf;
    return 0;
}

/*
 * Synchronize cache with disk.
 */
static int Sync_Cache(struct FS_Buffer_Cache *cache) {
    int rc = 0;
    struct FS_Buffer *buf;

    KASSERT(IS_HELD(&cache->mutex));

    for(buf = Get_Front_Of_FS_Buffer_List(&cache->bufferList);
        buf != 0; buf = Get_Next_In_FS_Buffer_List(buf)) {
        if((rc = Sync_Buffer(cache, buf)) != 0)
            break;
    }

    return rc;
}

/*
 * Free the memory used by a filesystem buffer.
 */
static void Free_Buffer(struct FS_Buffer *buf) {
    KASSERT(!(buf->flags & (FS_BUFFER_DIRTY | FS_BUFFER_INUSE)));
    Free_Page(buf->data);
    Free(buf);
}

/* ----------------------------------------------------------------------
 * Public functions
 * ---------------------------------------------------------------------- */

/*
 * Create a cache of filesystem buffers.
 */
struct FS_Buffer_Cache *Create_FS_Buffer_Cache(struct Block_Device *dev,
                                               uint_t fsBlockSize) {
    struct FS_Buffer_Cache *cache;

    KASSERT(dev != 0);
    KASSERT(dev->inUse);

    /*
     * Currently, we don't allow filesystem blocks
     * larger than the hardware page size.
     */
    KASSERT(fsBlockSize <= PAGE_SIZE);
    KASSERT(fsBlockSize > 0);   /* Guess we could be this defensive. */

    cache = (struct FS_Buffer_Cache *)Malloc(sizeof(*cache));
    if(cache == 0)
        return 0;
    memset(cache, 0, sizeof(struct FS_Buffer_Cache));

    cache->dev = dev;
    cache->fsBlockSize = fsBlockSize;
    cache->numCached = 0;
    Clear_FS_Buffer_List(&cache->bufferList);
    Mutex_Init(&cache->mutex);
    Cond_Init(&cache->cond);

    return cache;
}

/*
 * Synchronize contents of cache with the disk
 * by writing out all dirty buffers.
 */
int Sync_FS_Buffer_Cache(struct FS_Buffer_Cache *cache) {
    int rc;

    Mutex_Lock(&cache->mutex);
    rc = Sync_Cache(cache);
    Mutex_Unlock(&cache->mutex);

    return rc;
}

/*
 * Destroy a filesystem buffer cache.
 * None of the buffers in the cache must be in use.
 * The cache must not be used after this function returns!
 */
int Destroy_FS_Buffer_Cache(struct FS_Buffer_Cache *cache) {
    int rc;
    struct FS_Buffer *buf;

    Mutex_Lock(&cache->mutex);

    /* Flush all contents back to disk. */
    rc = Sync_Cache(cache);

    /* Free all of the buffers. */
    buf = Get_Front_Of_FS_Buffer_List(&cache->bufferList);
    while (buf != 0) {
        struct FS_Buffer *next = Get_Next_In_FS_Buffer_List(buf);
        Free_Buffer(buf);
        buf = next;
    }
    Clear_FS_Buffer_List(&cache->bufferList);

    Mutex_Unlock(&cache->mutex);

    /* Free the cache object itself. */
    Free(cache);

    return rc;
}

/*
 * Get a buffer for given filesystem block.
 */
int Get_FS_Buffer(struct FS_Buffer_Cache *cache, ulong_t fsBlockNum,
                  struct FS_Buffer **pBuf) {
    int rc;

    KASSERT0(cache != NULL,
             "Null FS_Buffer_Cache passed to Get_FS_Buffer.");
    KASSERT0(pBuf != NULL,
             "Null FS_Buffer pointer address passed to Get_FS_Buffer.");

    Debug("Getting FS use cached block %lu, RA=%lx\n", fsBlockNum,
          (ulong_t) __builtin_return_address(0));

    Mutex_Lock(&cache->mutex);
    rc = Get_Buffer(cache, fsBlockNum, pBuf);
    Mutex_Unlock(&cache->mutex);

    return rc;
}

/*
 * Mark the given buffer as being modified.
 */
void Modify_FS_Buffer(struct FS_Buffer_Cache *cache,
                      struct FS_Buffer *buf) {
    KASSERT0(cache != NULL,
             "Null FS_Buffer_Cache passed to Modify_FS_Buffer.");
    KASSERT0(buf != NULL, "Null FS_Buffer passed to Modify_FS_Buffer.");

    KASSERT(buf->flags & FS_BUFFER_INUSE);
    buf->flags |= FS_BUFFER_DIRTY;
}

/*
 * Explicitly synchronize given buffer with its on-disk storage,
 * without releasing the buffer.
 */
int Sync_FS_Buffer(struct FS_Buffer_Cache *cache, struct FS_Buffer *buf) {
    int rc;

    KASSERT(buf->flags & FS_BUFFER_INUSE);

    Mutex_Lock(&cache->mutex);
    rc = Sync_Buffer(cache, buf);
    Mutex_Unlock(&cache->mutex);

    return rc;
}

/*
 * Release given buffer.
 */
int Release_FS_Buffer(struct FS_Buffer_Cache *cache,
                      struct FS_Buffer *buf) {
    int rc = 0;

    KASSERT(buf->flags & FS_BUFFER_INUSE);

    Mutex_Lock(&cache->mutex);

    /*
     * If the buffer is OK to release,
     * mark it as no longer in use and notify any
     * thread waiting to use it.
     */
    if(rc == 0) {
        buf->flags &= ~(FS_BUFFER_INUSE);
        Cond_Broadcast(&cache->cond);
    }
    Debug("Released block %lu\n", buf->fsBlockNum);

    Mutex_Unlock(&cache->mutex);

    return rc;
}
