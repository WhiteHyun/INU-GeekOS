/*
 * Pseudo-fat filesystem.
 * Copyright (c) 2001,2003,2004 David H. Hovemeyer <daveho@cs.umd.edu>
 * Copyright (c) 2003,2013,2014 Jeffrey K. Hollingsworth <hollings@cs.umd.edu>
 *
 * All rights reserved.
 *
 * This code may not be resdistributed without the permission of the copyright holders.
 * Any student solutions using any of this code base constitute derviced work and may
 * not be redistributed in any form.  This includes (but is not limited to) posting on
 * public forums or web sites, providing copies to (past, present, or future) students
 * enrolled in similar operating systems courses the University of Maryland's CMSC412 course.
 *
 */

#include <limits.h>
#include <geekos/errno.h>
#include <geekos/screen.h>
#include <geekos/string.h>
#include <geekos/malloc.h>
#include <geekos/ide.h>
#include <geekos/blockdev.h>
#include <geekos/bitset.h>
#include <geekos/vfs.h>
#include <geekos/list.h>
#include <geekos/synch.h>
#include <geekos/pfat.h>
#include <geekos/projects.h>

/*
 * History:
 * 13-Nov-2003: Converted to use new block device API.
 * 17-Dec-2003: Rewrite to conform to new VFS layer
 * 19-Feb-2004: Cache and share PFAT_File objects, instead of
 *   allocating them repeatedly
 */

/*
 * TODO:
 * - Support hierarchical directories
 */

/* ----------------------------------------------------------------------
 * Private functions
 * ---------------------------------------------------------------------- */

#define PAGEFILE_FILENAME "/pagefile.bin"

int debugPFAT = 0;
#define Debug(args...) if (debugPFAT) Print("PFAT: " args)

struct PFAT_File;
DEFINE_LIST(PFAT_File_List, PFAT_File);

/*
 * In-memory information describing a mounted PFAT filesystem.
 * This is kept in the fsInfo field of the Mount_Point.
 */
struct PFAT_Instance {
    bootSector fsinfo;
    int *fat;
    directoryEntry *rootDir;
    directoryEntry rootDirEntry;
    struct Mutex lock;
    struct PFAT_File_List fileList;
};

/*
 * In-memory information for a particular open file.
 * In particular, this object contains a cache of the contents
 * of the file.
 * Kept in fsInfo field of File.
 */
struct PFAT_File {
    directoryEntry *entry;      /* Directory entry of the file */
    ulong_t numBlocks;          /* Number of blocks used by file */
    int currBlock;
    char *fileDataCache;        /* File data cache */
    struct Mutex lock;          /* Synchronize concurrent accesses */
     DEFINE_LINK(PFAT_File_List, PFAT_File);
};
IMPLEMENT_LIST(PFAT_File_List, PFAT_File);

/*
 * Copy file metadata from directory entry into
 * struct VFS_File_Stat object.
 */
static void Copy_Stat(struct VFS_File_Stat *stat, directoryEntry * entry) {
    stat->size = entry->fileSize;
    stat->isDirectory = entry->directory;

    stat->isSetuid = entry->isSetUid;
    memcpy(stat->acls, entry->acls, sizeof(stat->acls));
}

/*
 * FStat function for PFAT files.
 */
static int PFAT_FStat(struct File *file, struct VFS_File_Stat *stat) {
    struct PFAT_File *pfatFile = (struct PFAT_File *)file->fsData;
    Copy_Stat(stat, pfatFile->entry);
    return 0;
}

/*
 * Read function for PFAT files.
 */
static int PFAT_Read(struct File *file, void *buf, ulong_t numBytes) {
    struct PFAT_File *pfatFile = (struct PFAT_File *)file->fsData;
    struct PFAT_Instance *instance =
        (struct PFAT_Instance *)file->mountPoint->fsData;
    ulong_t start = file->filePos;
    ulong_t end = file->filePos + numBytes;

    if(pfatFile->entry->directory)
        return EINVALID;

    if(end > file->endPos) {
        numBytes = file->endPos - file->filePos;
        end = file->endPos;
    }
    //Print("start:%d end:%d nb:%d  \n",start,end,numBytes);

    ulong_t startBlock, endBlock, curBlock;
    ulong_t i;

    /* Special case: can't handle reads longer than INT_MAX */
    if(numBytes > INT_MAX)
        return EINVALID;

    /* if the file is an even multiple of numBytes, then with repeated reads, 
       we may end up with a read starting just at the end of the file.  such
       is not invalid, it is just the end of the file. (ns) */
    if(start == file->endPos) {
        return 0;
    }

    /* Make sure request represents a valid range within the file */
    if(start >= file->endPos || end > file->endPos || end < start) {
        Debug
            ("Invalid read position: filePos=%lu, numBytes=%lu, endPos=%lu\n",
             file->filePos, numBytes, file->endPos);
        return EINVALID;
    }

    /*
     * Now the complicated part; read all blocks containing the
     * data we need.
     */
    startBlock = start / SECTOR_SIZE;
    endBlock = Round_Up_To_Block(end) / SECTOR_SIZE;
    // Print("start block: %d end block: %d\n",startBlock,endBlock);

    ulong_t startOffset = start - (startBlock * SECTOR_SIZE);

    /*
     * Traverse the FAT finding the blocks of the file.
     * As we encounter requested blocks that aren't in the
     * file data cache, we issue requests to read them.
     */
    KASSERT(pfatFile->entry);
    curBlock = pfatFile->currBlock;
    ulong_t currOffset = 0;
    ulong_t firstBlock;

    for(i = startBlock; i < endBlock; ++i) {
        /* Are we at a valid block? */
        if(curBlock == FAT_ENTRY_FREE || curBlock == FAT_ENTRY_EOF) {
            Print
                ("Unexpected end of file in FAT at file block %lu of %lu\n",
                 i, endBlock);
            return EIO;         /* probable filesystem corruption */
        }

        /* Do we need to read this block? */
        if(i >= startBlock) {
            int rc = 0;

            /* Only allow one thread at a time to read this block. */
            Mutex_Lock(&pfatFile->lock);

            rc = Block_Read(file->mountPoint->dev, curBlock,
                            pfatFile->fileDataCache);

            if(i == startBlock) {
                // only copy part of the first block
                int share;
                if(numBytes < SECTOR_SIZE - startOffset) {
                    // reading less than to end of sector
                    memcpy(buf, pfatFile->fileDataCache + startOffset,
                           numBytes);
                    currOffset = numBytes;
                } else {
                    memcpy(buf, pfatFile->fileDataCache + startOffset,
                           SECTOR_SIZE - startOffset);
                    currOffset = SECTOR_SIZE - startOffset;
                    /* Continue to next block */
                    curBlock = instance->fat[curBlock];
                }
            } else if(i == endBlock - 1) {
                memcpy(buf + currOffset, pfatFile->fileDataCache,
                       numBytes - currOffset);
                if(numBytes - currOffset == SECTOR_SIZE) {
                    curBlock = instance->fat[curBlock];
                }
                currOffset += numBytes - currOffset;
            } else {
                memcpy(buf + currOffset, pfatFile->fileDataCache,
                       SECTOR_SIZE);
                currOffset += SECTOR_SIZE;
                /* Continue to next block */
                curBlock = instance->fat[curBlock];
            }

            /* Done attempting to fetch the block */
            Mutex_Unlock(&pfatFile->lock);
        }
    }

    //update position in file!
    file->filePos += numBytes;
    pfatFile->currBlock = curBlock;

    KASSERT(currOffset == numBytes);
    Debug("Read satisfied!\n");

    return numBytes;
}

/*
 * Write function for PFAT files.
 */
static int PFAT_Write(struct File *file, void *ptr, ulong_t numBytes) {
    char *buf = (char *)ptr;
    struct PFAT_File *pfatFile = (struct PFAT_File *)file->fsData;
    struct PFAT_Instance *instance =
        (struct PFAT_Instance *)file->mountPoint->fsData;
    ulong_t start = file->filePos;
    ulong_t end = file->filePos + numBytes;

    if(pfatFile->entry->directory)
        return EINVALID;

    if(!(file->mode & O_WRITE)) {
        return EINVALID;
    }

    if(numBytes % SECTOR_SIZE) {
        /* only write multiples of full sectors */
        return EINVALID;
    }

    if(file->filePos % SECTOR_SIZE) {
        /* only write at start of sector */
        return EINVALID;
    }

    KASSERT(file->filePos <= file->endPos);
    // allowed to write the last sector even if file is not full
    if(file->filePos + numBytes > file->endPos) {
        numBytes = file->endPos - file->filePos;
    }

    /*
     * Now the complicated part; write all blocks containing the data we need.
     */
    ulong_t startBlock = start / SECTOR_SIZE;
    ulong_t endBlock = Round_Up_To_Block(end) / SECTOR_SIZE;
    // Print("start block: %d end block: %d\n",startBlock,endBlock);

    int startOffset = start - (startBlock * SECTOR_SIZE);

    /*
     * Traverse the FAT finding the blocks of the file.
     * As we encounter requested blocks write them with the new data.
     */
    KASSERT(pfatFile->entry);
    ulong_t curBlock = pfatFile->entry->firstBlock;
    ulong_t currOffset = 0;
    ulong_t i;
    for(i = 0; i < endBlock; ++i) {
        /* Are we at a valid block? */
        if(curBlock == FAT_ENTRY_FREE || curBlock == FAT_ENTRY_EOF) {
            Print("Unexpected end of file in FAT at file block %lu\n", i);
            return EIO;         /* probable filesystem corruption */
        }

        /* Do we need to write this block? */
        if(i >= startBlock) {
            int rc = 0;

            /* Only allow one thread at a time to read this block. */
            Mutex_Lock(&pfatFile->lock);

            rc = Block_Write(file->mountPoint->dev, curBlock,
                             &buf[currOffset]);

            currOffset += SECTOR_SIZE;

            /* Done attempting to fetch the block */
            Mutex_Unlock(&pfatFile->lock);

            if(rc != 0)
                return rc;
        }

        /* Continue to next block */
        ulong_t nextBlock = instance->fat[curBlock];
        curBlock = nextBlock;
    }

    //update position in file!
    file->filePos += numBytes;
    pfatFile->currBlock = curBlock;

    return numBytes;
}

/*
 * Seek function for PFAT files.
 */
static int PFAT_Seek(struct File *file, ulong_t pos) {
    struct PFAT_File *pfatFile = (struct PFAT_File *)file->fsData;
    struct PFAT_Instance *instance =
        (struct PFAT_Instance *)file->mountPoint->fsData;

    if(pos >= file->endPos)
        return EINVALID;
    if(file->filePos != pos) {
        // read from start of FAT linked list for this file to desired position 
        file->filePos = pos;
        KASSERT(pfatFile->entry);
        ulong_t curBlock = pfatFile->entry->firstBlock;
        uint_t i;
        for(i = 0; i < pos; i += 512) {
            curBlock = instance->fat[curBlock];
        }
        pfatFile->currBlock = curBlock;
    }
    return 0;
}

/*
 * Close function for PFAT files.
 */
static int PFAT_Close(struct File *file __attribute__ ((unused))) {
    /*
     * The PFAT_File object caching the contents of the file
     * will remain in the PFAT_Instance object, to speed up
     * future accesses to this file.
     */
    return 0;
}

/*
 * File_Ops for PFAT files.
 */
static struct File_Ops s_pfatFileOps = {
    &PFAT_FStat,
    &PFAT_Read,
    &PFAT_Write,
    &PFAT_Seek,
    &PFAT_Close,
    0,                          /* Read_Entry */
};

static int PFAT_FStat_Dir(struct File *dir, struct VFS_File_Stat *stat) {
    /* FIXME: for now, there is only one directory */
    struct PFAT_Instance *instance =
        (struct PFAT_Instance *)dir->mountPoint->fsData;
    Copy_Stat(stat, &instance->rootDirEntry);
    return 0;
}

/*
 * Close function for PFAT directories.
 */
static int PFAT_Close_Dir(struct File *dir __attribute__ ((unused))) {
    /* This is a no-op. */
    return 0;
}

/*
 * Read a directory entry.
 */
static int PFAT_Read_Entry(struct File *dir, struct VFS_Dir_Entry *entry) {
    directoryEntry *directory;
    directoryEntry *pfatDirEntry;
    struct PFAT_Instance *instance =
        (struct PFAT_Instance *)dir->mountPoint->fsData;

    if(dir->filePos >= dir->endPos)
        return VFS_NO_MORE_DIR_ENTRIES; /* Reached the end of the directory. */

    directory = (directoryEntry *) dir->fsData;
    pfatDirEntry = &directory[dir->filePos++];

    /*
     * Note: we don't need to bounds check here, because
     * generic struct VFS_Dir_Entry objects have much more space for filenames
     * than PFAT directoryEntry objects.
     */
    strncpy(entry->name, pfatDirEntry->fileName,
            sizeof(pfatDirEntry->fileName));
    entry->name[sizeof(pfatDirEntry->fileName)] = '\0';

    Copy_Stat(&entry->stats, pfatDirEntry);

    return 0;
}

/*
 * File_Ops for PFAT directories.
 */
static struct File_Ops s_pfatDirOps = {
    &PFAT_FStat_Dir,
    0,                          /* Read */
    0,                          /* Write */
    0,                          /* Seek */
    &PFAT_Close_Dir,
    &PFAT_Read_Entry,
};

/*
 * Look up a directory entry in a PFAT filesystem.
 */
static directoryEntry *PFAT_Lookup(struct Mount_Point *mountPoint,
                                   struct PFAT_Instance *instance,
                                   const char *path) {
    directoryEntry *rootDir = instance->rootDir;
    bootSector *fsinfo = &instance->fsinfo;
    int i;

    KASSERT(*path == '/');

    /* Special case: root directory. */
    if(strcmp(path, "/") == 0)
        return &instance->rootDirEntry;

    /* Skip leading '/' character. */
    ++path;

    directoryEntry *currDir = rootDir;
    int dirSize = fsinfo->rootDirectoryCount;
    while (path) {
        char *rest = strchr(path, '/');
        if(rest) {
            *rest = '\0';
            rest++;
        }
        int found = 0;
        for(i = 0; i < dirSize; ++i) {
            directoryEntry *entry = &currDir[i];
            if(strcmp(entry->fileName, path) == 0) {
                /* Found it! */
                found = 1;
                if(!rest) {
                    Debug("Found matching dir entry for %s\n", path);
                    return entry;
                } else {
                    /* read directory */
                    currDir = Malloc(512);
                    // XXXXX leaking memory here on currDir
                    dirSize = entry->fileSize / sizeof(directoryEntry);
                    Block_Read(mountPoint->dev, entry->firstBlock,
                               currDir);
                    path = rest;
                    break;
                }
            }
        }
        if(!found)
            return 0;
    }

    /* Not found. */
    return 0;
}

/*
 * Get a PFAT_File object representing the file whose directory entry
 * is given.
 */
static struct PFAT_File *Get_PFAT_File(struct PFAT_Instance *instance,
                                       directoryEntry * entry) {
    ulong_t numBlocks;
    struct PFAT_File *pfatFile = 0;
    char *fileDataCache = 0;

    KASSERT(entry != 0);
    KASSERT(instance != 0);

    Mutex_Lock(&instance->lock);

    /*
     * See if this file has already been opened.
     * If so, use the existing PFAT_File object.
     */
    for(pfatFile = Get_Front_Of_PFAT_File_List(&instance->fileList);
        pfatFile != 0; pfatFile = Get_Next_In_PFAT_File_List(pfatFile)) {
        if(pfatFile->entry == entry)
            break;
    }

    if(pfatFile == 0) {
        /* Determine size of data block cache for file. */
        numBlocks = Round_Up_To_Block(entry->fileSize) / SECTOR_SIZE;

        /*
         * Allocate File object, PFAT_File object, file block data cache,
         * and valid cache block bitset
         */
        if((pfatFile = (struct PFAT_File *)Malloc(sizeof(*pfatFile))) == 0
           || (fileDataCache = Malloc(SECTOR_SIZE)) == 0) {
            goto memfail;
        }

        /* Populate PFAT_File */
        pfatFile->entry = entry;
        pfatFile->numBlocks = numBlocks;
        pfatFile->fileDataCache = fileDataCache;
        Mutex_Init(&pfatFile->lock);

        /* Add to instance's list of PFAT_File objects. */
        Add_To_Back_Of_PFAT_File_List(&instance->fileList, pfatFile);
        KASSERT(pfatFile->nextPFAT_File_List == 0);
    }

    /* Success! */
    goto done;

  memfail:
    if(pfatFile != 0)
        Free(pfatFile);
    if(fileDataCache != 0)
        Free(fileDataCache);
    pfatFile = NULL;

  done:
    Mutex_Unlock(&instance->lock);
    return pfatFile;
}

extern int GetUid();


/*
 * Open function for PFAT filesystems.
 */
static int PFAT_Open(struct Mount_Point *mountPoint, const char *path,
                     int mode, struct File **pFile) {
    int rc = 0;
    struct PFAT_Instance *instance =
        (struct PFAT_Instance *)mountPoint->fsData;
    directoryEntry *entry;
    struct PFAT_File *pfatFile = 0;
    struct File *file = 0;

    /* Reject attempts to create or write */
    if((mode & O_CREATE) != 0)
        return EACCESS;

    /* Look up the directory entry */
    entry = PFAT_Lookup(mountPoint, instance, path);
    if(entry == 0)
        return ENOTFOUND;


    /* Get PFAT_File object */
    pfatFile = Get_PFAT_File(instance, entry);
    if(pfatFile == 0) {
        *pFile = NULL;
        rc = ENOMEM;
        goto done;
    }

    pfatFile->currBlock = pfatFile->entry->firstBlock;

    /* Create the file object. */
    file =
        Allocate_File(&s_pfatFileOps, 0, entry->fileSize, pfatFile, mode,
                      0);
    if(file == 0) {
        rc = ENOMEM;
        goto done;
    }

    /* Success! */
    *pFile = file;

  done:
    return rc;
}

/*
 * Open_Directory function for PFAT filesystems.
 */
static int PFAT_Open_Directory(struct Mount_Point *mountPoint,
                               const char *path, struct File **pDir) {
    /*
     * FIXME: for now, we only support a single directory.
     * This makes this function pretty simple.
     * We just store the current cursor index in the File object.
     */
    struct PFAT_Instance *instance =
        (struct PFAT_Instance *)mountPoint->fsData;
    struct File *dir;

    if(strcmp(path, "/") == 0) {
        dir = (struct File *)Malloc(sizeof(*dir));
        if(dir == 0)
            return ENOMEM;

        dir->ops = &s_pfatDirOps;
        dir->filePos = 0;       /* next dir entry to be read */
        dir->endPos = instance->fsinfo.rootDirectoryCount;      /* number of directory entries */
        dir->fsData = (void *)instance->rootDir;
        *pDir = dir;
        return 0;
    } else {
        directoryEntry *entry;
        entry = PFAT_Lookup(mountPoint, instance, path);
        if(!entry) {
            return ENOTFOUND;
        }
        if(!entry->directory) {
            return ENOTFOUND;
        }

        dir = (struct File *)Malloc(sizeof(*dir));
        if(dir == 0)
            return ENOMEM;

        dir->ops = &s_pfatDirOps;
        dir->filePos = 0;       /* next dir entry to be read */

        // read disk block with directory data;
        directoryEntry *items =
            (directoryEntry *) Malloc(entry->fileSize);
        if(Block_Read(mountPoint->dev, entry->firstBlock, items) < 0) {
            Print("block read at %d sector failed\n", entry->firstBlock);
            return ENOMEM;
        }
        dir->fsData = (void *)items;
        unsigned int i;
        for(i = 0; i < entry->fileSize / sizeof(directoryEntry); i++) {
            if(items[i].fileName[0] == '\0')
                break;
        }
        dir->endPos = i;        /* number of directory entries */
        *pDir = dir;
        return 0;
    }
}

/*
 * Stat function for PFAT filesystems.
 */
static int PFAT_Stat(struct Mount_Point *mountPoint, const char *path,
                     struct VFS_File_Stat *stat) {
    struct PFAT_Instance *instance =
        (struct PFAT_Instance *)mountPoint->fsData;
    directoryEntry *entry;

    KASSERT(path != 0);
    KASSERT(stat != 0);

    Debug("PFAT_Stat(%s)\n", path);

    entry = PFAT_Lookup(mountPoint, instance, path);
    if(entry == 0)
        return ENOTFOUND;

    Copy_Stat(stat, entry);

    return 0;
}

/*
 * Sync function for PFAT filesystems.
 */
static int PFAT_Sync(struct Mount_Point *mountPoint
                     __attribute__ ((unused))) {
    /* Read only filesystem: this is a no-op. */
    return 0;
}

static int PFAT_SetSetUid(struct Mount_Point *mp, const char *file,
                          int setuid) {
    TODO_P(PROJECT_USER, "pfat file system SetSetUID operation");
    return EUNSUPPORTED;
}

static int PFAT_SetAcl(struct Mount_Point *mp, const char *file, int uid,
                       int permissions) {
    TODO_P(PROJECT_USER, "pfat file system SetAcl operation");
    return EUNSUPPORTED;
}

/*
 * Mount_P int_Ops for PFAT filesystem.
 */
struct Mount_Point_Ops s_pfatMountPointOps = {
    PFAT_Open,
    0,                          /* Create_Directory() */
    PFAT_Open_Directory,
    PFAT_Stat,
    PFAT_Sync,
    0,                          /* Delete */
    0,                          /* Rename */
    0,                          /* Link */
    0,                          /* SymLink */
    PFAT_SetSetUid,
    PFAT_SetAcl,
    0,                          /* Disk_Properties */
};

/*
 * If the given PFAT instance has a paging file,
 * register it as the paging device, unless a paging device
 * has already been registered.
 */
static void PFAT_Register_Paging_File(struct Mount_Point *mountPoint,
                                      struct PFAT_Instance *instance) {
    directoryEntry *pagefileEntry;
    struct Paging_Device *pagedev = 0;
    size_t nameLen;
    char *fileName = 0;

    if(Get_Paging_Device() != 0)
        return;                 /* A paging device is already registered */

    pagefileEntry = PFAT_Lookup(mountPoint, instance, PAGEFILE_FILENAME);
    if(pagefileEntry == 0)
        return;                 /* No paging file in this filesystem */

    /* TODO: verify that paging file is contiguous */

    /* Create Paging_Device object. */
    pagedev = (struct Paging_Device *)Malloc(sizeof(*pagedev));
    if(pagedev == 0)
        goto memfail;
    nameLen =
        strlen(mountPoint->pathPrefix) + strlen(PAGEFILE_FILENAME) + 3;
    fileName = (char *)Malloc(nameLen);
    if(fileName == 0)
        goto memfail;

    /* Format page filename */
    snprintf(fileName, nameLen, "/%s%s", mountPoint->pathPrefix,
             PAGEFILE_FILENAME);

    /* Initialize Paging_Device */
    pagedev->fileName = fileName;
    pagedev->dev = mountPoint->dev;
    pagedev->startSector = pagefileEntry->firstBlock;
    pagedev->numSectors = pagefileEntry->fileSize / SECTOR_SIZE;

    /* Register it */
    Register_Paging_Device(pagedev);
    return;

  memfail:
    Print("  Error: could not create paging device for pfat on %s (%s)\n",
          mountPoint->pathPrefix, mountPoint->dev->name);
    if(pagedev != 0)
        Free(pagedev);
    if(fileName != 0)
        Free(fileName);
}

/*
 * Mount function for PFAT filesystem.
 */
static int PFAT_Mount(struct Mount_Point *mountPoint) {
    struct PFAT_Instance *instance = 0;
    bootSector *fsinfo;
    void *bootSect = 0;
    int rootDirSize;
    int rc;
    int i;

    /* Allocate instance. */
    instance = (struct PFAT_Instance *)Malloc(sizeof(*instance));
    if(instance == 0)
        goto memfail;
    memset(instance, '\0', sizeof(*instance));
    fsinfo = &instance->fsinfo;
    Debug("Created instance object\n");

    /*
     * Allocate buffer to read bootsector,
     * which contains metainformation about the PFAT filesystem.
     */
    bootSect = Malloc(SECTOR_SIZE);
    if(bootSect == 0)
        goto memfail;

    /* Read boot sector */
    if((rc = Block_Read(mountPoint->dev, 0, bootSect)) < 0)
        goto fail;
    Debug("Read boot sector\n");

    /* Copy filesystem parameters from boot sector */
    memcpy(&instance->fsinfo,
           ((char *)bootSect) + PFAT_BOOT_RECORD_OFFSET,
           sizeof(bootSector));
    Debug("Copied boot record\n");

    /* Does magic number match? */
    if(fsinfo->magic != PFAT_MAGIC) {
        Print("Bad magic number (%x) for PFAT filesystem\n",
              fsinfo->magic);
        goto invalidfs;
    }
    Debug("Magic number is good!\n");

    /* Do filesystem params look reasonable? */
    if(fsinfo->fileAllocationOffset <= 0 ||
       fsinfo->fileAllocationLength <= 0 ||
       fsinfo->rootDirectoryCount < 0 ||
       fsinfo->rootDirectoryOffset <= 0) {
        Print("Invalid parameters for PFAT filesystem\n");
        goto invalidfs;
    }
    Debug("PFAT filesystem parameters appear to be good!\n");

    /* Allocate in-memory FAT */
    instance->fat =
        (int *)Malloc(fsinfo->fileAllocationLength * SECTOR_SIZE);
    if(instance->fat == 0)
        goto memfail;

    /* Read the FAT */
    for(i = 0; i < fsinfo->fileAllocationLength; ++i) {
        int blockNum = fsinfo->fileAllocationOffset + i;
        char *p = ((char *)instance->fat) + (i * SECTOR_SIZE);
        if((rc = Block_Read(mountPoint->dev, blockNum, p)) < 0)
            goto fail;
    }
    Debug("Read FAT successfully!\n");

    if(fsinfo->rootDirectoryCount > 0) {        /* nspring attempting to avoid stupidity of malloc(0) */
        /* Allocate root directory */
        rootDirSize =
            Round_Up_To_Block(sizeof(directoryEntry) *
                              fsinfo->rootDirectoryCount);
        instance->rootDir = (directoryEntry *) Malloc(rootDirSize);

        /* Read the root directory */
        Debug("Root directory size = %d\n", rootDirSize);
        for(i = 0; i < rootDirSize; i += SECTOR_SIZE) {
            int blockNum = fsinfo->rootDirectoryOffset + i / SECTOR_SIZE;
            if((rc =
                Block_Read(mountPoint->dev, blockNum,
                           (void *)((int)instance->rootDir + i))) < 0)
                goto fail;
        }
        Debug("Read root directory successfully!\n");
    } else {
        Print("Warning: missing root directory in PFAT");
        instance->rootDir = NULL;       /* how can this bee? for it is a borken fs. */
    }

    /* Create the fake root directory entry. */
    memset(&instance->rootDirEntry, '\0', sizeof(directoryEntry));
    instance->rootDirEntry.readOnly = 1;
    instance->rootDirEntry.directory = 1;
    instance->rootDirEntry.fileSize =
        instance->fsinfo.rootDirectoryCount * sizeof(directoryEntry);

    /* Initialize instance lock and PFAT_File list. */
    Mutex_Init(&instance->lock);
    Clear_PFAT_File_List(&instance->fileList);
    Spin_Lock_Init(&instance->fileList.lock);   /* ns15 */

    /* Attempt to register a paging file */
    PFAT_Register_Paging_File(mountPoint, instance);

    /*
     * Success!
     * This mount point is now ready
     * to handle file accesses.
     */
    mountPoint->ops = &s_pfatMountPointOps;
    mountPoint->fsData = instance;
    return 0;

  memfail:
    rc = ENOMEM;
    goto fail;
  invalidfs:
    rc = EINVALIDFS;
    goto fail;
  fail:
    if(instance != 0) {
        if(instance->fat != 0)
            Free(instance->fat);
        if(instance->rootDir != 0)
            Free(instance->rootDir);
        Free(instance);
    }
    if(bootSect != 0)
        Free(bootSect);
    return rc;
}

static struct Filesystem_Ops s_pfatFilesystemOps = {
    0,                          // Format
    &PFAT_Mount,
};

/* ----------------------------------------------------------------------
 * Public functions
 * ---------------------------------------------------------------------- */

void Init_PFAT(void) {
    Register_Filesystem("pfat", &s_pfatFilesystemOps);
}
