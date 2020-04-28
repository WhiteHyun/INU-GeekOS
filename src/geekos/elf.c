/*
 * ELF executable loading
 * Copyright (c) 2003, Jeffrey K. Hollingsworth <hollings@cs.umd.edu>
 * Copyright (c) 2003, David H. Hovemeyer <daveho@cs.umd.edu>
 *
 * All rights reserved.
 *
 * This code may not be resdistributed without the permission of the copyright holders.
 * Any student solutions using any of this code base constitute derviced work and may
 * not be redistributed in any form.  This includes (but is not limited to) posting on
 * public forums or web sites, providing copies to (past, present, or future) students
 * enrolled in similar operating systems courses the University of Maryland's CMSC412 course.
 *
 * $Revision: 1.31 $
 *
 */

#include <geekos/errno.h>
#include <geekos/kassert.h>
#include <geekos/ktypes.h>
#include <geekos/screen.h> /* for debug Print() statements */
#include <geekos/pfat.h>
#include <geekos/malloc.h>
#include <geekos/string.h>
#include <geekos/user.h>
#include <geekos/fileio.h>
#include <geekos/elf.h>

#include <geekos/paging.h>

int elfDebug = 0;

/**
 * From the data of an ELF executable, determine how its segments
 * need to be loaded into memory.
 * @param exeFileData buffer containing the executable file
 * @param exeFileLength length of the executable file in bytes
 * @param exeFormat structure describing the executable's segments
 *   and entry address; to be filled in
 * @return 0 if successful, < 0 on error
 */
#define FAIL -1
#define NEXT 0
int Parse_ELF_Executable(char *exeFileData, ulong_t exeFileLength,
                         struct Exe_Format *exeFormat)
{
    elfHeader *ehdr;
    programHeader *phdr;
    struct Exe_Segment *exeSegment;
    short emptySegments = 0;

    ehdr = (elfHeader *)exeFileData;
    phdr = (programHeader *)(exeFileData + ehdr->phoff);
    exeSegment = exeFormat->segmentList;
    //Print("exeFileData = %p\nelfHeader = %p\n", exeFileData, ehdr);
    Print("ehdr->phoff = %d\nehdr->phnum = %d\nehdr->phentsize=%d\nehdr->entry = %d\n", ehdr->phoff, ehdr->phnum, ehdr->phentsize, ehdr->entry);
    if (exeFileData == 0)
    {
        Print("Error! exeFileData = 0!");
        return FAIL;
    }

    if (ehdr->ident[0] != 0x7f || ehdr->ident[1] != 'E' || ehdr->ident[2] != 'L' || ehdr->ident[3] != 'F')
    {
        Print("ehdr->ident is not ELF");
        return FAIL;
    }

    for (int i = 0; i < ehdr->phnum; i++)
    {
        if (ehdr->phentsize == 0) /* Processing empty segments */
        {
            exeSegment++; //next segment
            phdr++;       //next program header
            emptySegments++;
            continue;
        }
        exeSegment->offsetInFile = phdr->offset;
        exeSegment->lengthInFile = phdr->fileSize;
        exeSegment->startAddress = phdr->vaddr;
        exeSegment->sizeInMemory = phdr->memSize;
        exeSegment->protFlags = phdr->flags;

        exeSegment++;
        phdr++;
    }
    exeFormat->numSegments = ehdr->phnum - emptySegments;
    exeFormat->entryAddr = ehdr->entry;
    Print("201601639 Hong Seung Hyeon\n");
    return NEXT;
}
