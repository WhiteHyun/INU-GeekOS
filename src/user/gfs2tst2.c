/*
 * Automated test program for Project 5x
 * Copyright (c) 2004 David H. Hovemeyer <daveho@cs.umd.edu>
 * Copyright (c) 2004 Iulian Neamtiu <neamtiu@cs.umd.edu>
 * Copyright (c) 2008 Neil Spring <nspring@cs.umd.edu>
 *
 * All rights reserved.
 *
 * This code may not be resdistributed without the permission of the copyright holders.
 * Any student solutions using any of this code base constitute derviced work and may
 * not be redistributed in any form.  This includes (but is not limited to) posting on
 * public forums or web sites, providing copies to (past, present, or future) students
 * enrolled in similar operating systems courses the University of Maryland's CMSC412 course.
 *
 * $Revision: 1.10 $
 * 
 */

#include <conio.h>
#include <process.h>
#include <fileio.h>
#include <string.h>
#include <geekos/errno.h>

#define SIZEOF_STRUCT_GFS2_INODE 32
// #include <geekos/gfs2.h>
// #include <geekos/vfs.h>

#define doTestOrDie(t,f,p,s,tt,st) do { if(doTest(t,f,p,s,tt,st) < 0) { Exit(1); } } while(0)

/* NOTE: these parameters used for the exhaustion tests,
   assumed to be correct, even though the file system might
   have been formatted differently */
unsigned int blocksize = 4096;
//unsigned int disksize_mb=2;
unsigned int blocks_on_disk = 0;
unsigned int disksize_mb = 6;
#define VFS_NO_MORE_DIR_ENTRIES 1

/* for the submit server, be prepared to run and exit. */
int ShutDown(void);

int doTest(const char *testName,
           int (*testFunction) (), int points, int *score,
           int *totalTests, int *successfulTests) {
    int ret;

    (*totalTests)++;

    Print("Testing: %s...", testName);

    ret = testFunction();

    if(ret < 0)
        Print("FAILED (%d)", ret);
    else {
        Print("PASSED (%d)", ret);
        (*score) += points;
        (*successfulTests)++;
    }

    Print(" crt score: %d \n", (*score));

    return ret;

}

/* 
int ttestFormat()
{
    int pid;
  char commandline[250];
  (void)snprintf(commandline, 249, "gfs2f.exe ide1 %u %u", disksize_mb, blocksize);
    pid = Spawn_With_Path("gfs2f.exe", commandline, "/c:/a", 0);
    return Wait(pid);
}
*/

int ttestMount(void) {
    int rc = Mount("ide1", "/d", "gfs2");
    Print("Mount returned %d.\n", rc);
    return rc;
}

int tOpenFile(void) {
    int retC;

    int fd = Open("/d/somefile", O_READ);
    if(fd < 0)
        return -1;

    retC = Close(fd);

    return (retC >= 0) ? 1 : -1;
}

int tOpenInexistentFile() {
    return (Open("/d/InexistentFile", O_READ) < 0) ? 1 : -1;
}

int tOpenDir() {
    int fd;

    fd = Open_Directory("/d/somedir");
    if(fd < 0) {
        Print("could not open /d/somedir: %d %s\n", fd,
              Get_Error_String(fd));
        return -1;
    }

    if(Close(fd) < 0) {
        Print("failed to close");
        return -1;
    }

    return 1;
}

int tCloseTwice() {
    int fd, retC;

    //fd = Open("/d/basic4f", O_CREATE|O_WRITE);
    fd = Open("/d/somefile", O_READ);
    if(fd < 0)
        return -1;

    if(Close(fd) < 0) {
        Print("failed to close");
        return -1;
    }

    retC = Close(fd);

    return (retC < 0) ? 1 : -1;
}

int tCloseAberrantFd() {
    int retC;

    retC = Close(100000);

    return (retC < 0) ? 1 : -1;
}

int tReadFile() {
    int fd, retR;
    char buffer[100];

    fd = Open("/d/basic", O_READ);
    if(fd < 0) {
        Print("could not open for reading: %d %s\n", fd,
              Get_Error_String(fd));
        return -1;
    }

    retR = Read(fd, buffer, 10);
    if(retR < 0) {
        Print("read return %d < 10\n", retR);
        return -1;
    }

    if(Close(fd) < 0) {
        Print("failed to close");
        return -1;
    }

    return (retR == 10) ? 1 : -1;
}

int tReadEntry() {
    int fd, retR;               //, retC;
    struct VFS_Dir_Entry dirEntry;

    /*
       retC = Create_Directory("/d/basic11d");
       if (retC < 0) {
       Print("couldn't create basic11d: %d %s\n", retC, Get_Error_String(retC));
       return -1;
       }

       retC = Create_Directory("/d/basic11d/d1");
       if (retC < 0) {
       Print("couldn't create basic11d/d1: %d %s\n", retC, Get_Error_String(retC));
       return -1;
       }

       retC = Create_Directory("/d/basic11d/d2");
       if (retC < 0) {
       Print("couldn't create basic11d/d2: %d %s\n", retC, Get_Error_String(retC));
       return -1;
       }

       fd = Open("/d/basic11d/f1", O_CREATE);
       if (fd < 0) {
       Print("couldn't open basic11d/f1: %d %s\n", fd, Get_Error_String(fd));
       return -1;
       }

       if(Close(fd) < 0) { Print("failed to close"); return -1; }
     */

    fd = Open_Directory("/d/basic11d");
    if(fd < 0) {
        Print("couldn't opendir basic11d: %d %s\n", fd,
              Get_Error_String(fd));
        return -1;
    }
    // For .
    retR = Read_Entry(fd, &dirEntry);

    // For ..
    retR = Read_Entry(fd, &dirEntry);

    retR = Read_Entry(fd, &dirEntry);

    if((retR < 0) ||
       (strncmp(dirEntry.name, "d1", 2) != 0) ||
       (!dirEntry.stats.isDirectory)) {
        Print("Dir name (d1) not correct %s %d.\n", dirEntry.name,
              dirEntry.stats.isDirectory);
        return -1;
    }

    retR = Read_Entry(fd, &dirEntry);

    if((retR < 0) ||
       (strncmp(dirEntry.name, "d2", 2) != 0) ||
       (!dirEntry.stats.isDirectory))
        return -1;

    retR = Read_Entry(fd, &dirEntry);

    if((retR < 0) ||
       (strncmp(dirEntry.name, "f1", 2) != 0) ||
       (dirEntry.stats.isDirectory))
        return -1;

    if(Close(fd) < 0) {
        Print("failed to close");
        return -1;
    }

    fd = Open_Directory("/d/basic11d");
    if(fd < 0)
        return -1;

    // no  retR = Seek(fd, 2);
    // no if (retR < 0)
    // no   return -1;

    // no retR = Read_Entry(fd, &dirEntry);

    // no if ((retR < 0) ||
    // no     (strncmp(dirEntry.name, "f1", 2) != 0) ||
    // no      (dirEntry.stats.isDirectory))
    // no   return -1;

    if(Close(fd) < 0) {
        Print("failed to close");
        return -1;
    }

    /*
       (void)Delete("/d/basic11d/d1", false);
       (void)Delete("/d/basic11d/d2", false);
       (void)Delete("/d/basic11d/f1", false);

       (void)Delete("/d/basic11d", false);
     */

    return 1;
}

int tBasicStat() {
    int fd, retS;
    struct VFS_File_Stat s;

    /*
       fd = Open("/d/basic8f", O_CREATE|O_WRITE);
       if (fd < 0)
       return -1;

       if(Close(fd) < 0) { Print("failed to close"); return -1; }
     */

    //fd = Open("/d/basic8f", O_READ);
    fd = Open("/d/basic", O_READ);
    if(fd < 0)
        return -1;

    retS = FStat(fd, &s);

    if(Close(fd) < 0) {
        Print("failed to close");
        return -1;
    }
    //(void)Delete("/d/basic8f", false);

    return (retS >= 0) ? 1 : -1;
}

int tStatFile() {
    //int fd, retW, retS;
    int retS;
    //char buffer[11];
    struct VFS_File_Stat s;

    /*
       fd = Open("/d/basic9f", O_CREATE|O_WRITE);
       if (fd < 0)
       return -1;

       retW = Write( fd, buffer, 10);
       if (retW < 0)
       return -1;

       if(Close(fd) < 0) { Print("failed to close"); return -1; }
     */

    //retS = Stat("/d/basic9f", &s);
    retS = Stat("/d/basic", &s);

    if(retS < 0) {
        Print("stat failed %d\n", retS);
    }
    if(s.size != 10000) {
        Print("tStatFile stat returned incorrect size %d\n", s.size);
    }
    //(void)Delete("/d/basic9f", false);

    return ((retS >= 0) && (s.size == 10000)) ? 1 : -1;
}

int tStatDirectory() {
    //int fd, retS, retC;
    int fd, retS;
    struct VFS_File_Stat s;

    /*
       retC = Create_Directory("/d/basic10d");
       if (retC < 0) {
       Print("couldn't create basic10d: %d %s\n", retC, Get_Error_String(retC));
       return -1;
       }
     */

    //fd = Open_Directory("/d/basic10d");
    fd = Open_Directory("/d/recursive_stat1");
    if(fd < 0) {
        Print("could nt open /d/recursive_stat1: %d %s\n", fd,
              Get_Error_String(fd));
        return -1;
    }

    retS = FStat(fd, &s);

    if(Close(fd) < 0) {
        Print("failed to close");
        return -1;
    }
    //(void)Delete("/d/basic10d", false);

    if(retS < 0) {
        Print("couldn't fstat opened basic10d: %d %s\n", retS,
              Get_Error_String(retS));
    }
    if(!s.isDirectory) {
        Print("fstat didn't think basic10d was a directory\n");
    }

    return ((retS >= 0) && (s.isDirectory)) ? 1 : -1;
}

int tRecursiveStat() {
    //int retC, retS, fd;
    int retS;
    struct VFS_File_Stat s;

    /*      
       retC = Create_Directory("/d/recursive_stat1");
       if (retC < 0) return -1;

       retC = Create_Directory("/d/recursive_stat1/recursive_stat2");
       if (retC < 0) return -1;

       retC = Create_Directory("/d/recursive_stat1/recursive_stat2/recursive_stat3");
       if (retC < 0) return -1;

       fd = Open("/d/recursive_stat1/recursive_stat2/recursive_stat3/recursive_stat4", O_CREATE|O_WRITE);
       if (fd < 0) return -1;

       retC = Close(fd);
     */

    retS = Stat("/d/recursive_stat1", &s);
    if(retS < 0)
        return -1;

    retS = Stat("/d/recursive_stat1x", &s);
    if(retS == 0)
        return -1;

    retS = Stat("/d/recursive_stat1/recursive_stat2", &s);
    if(retS < 0)
        return -1;

    retS = Stat("/d/recursive_stat1x/recursive_stat2", &s);
    if(retS == 0)
        return -1;

    retS = Stat("/d/recursive_stat1/../recursive_stat1", &s);
    if(retS < 0)
        return -1;

    retS = Stat("/d/recursive_stat1/../recursive_stat2", &s);
    if(retS == 0)
        return -1;

    retS = Stat("/d/recursive_stat1/./recursive_stat2", &s);
    if(retS < 0)
        return -1;

    retS = Stat("/d/recursive_stat1/./recursive_stat1", &s);
    if(retS == 0)
        return -1;

    retS =
        Stat
        ("/d/recursive_stat1/../recursive_stat1/recursive_stat2/recursive_stat3",
         &s);
    if(retS < 0)
        return -1;

    retS =
        Stat
        ("/d/recursive_stat1/../recursive_stat2/recursive_stat2/recursive_stat3",
         &s);
    if(retS == 0)
        return -1;

    retS =
        Stat
        ("/d/recursive_stat1/./recursive_stat2/../recursive_stat2/recursive_stat3",
         &s);
    if(retS < 0)
        return -1;

    retS =
        Stat
        ("/d/recursive_stat1/./recursive_stat1../recursive_stat2/recursive_stat3",
         &s);
    if(retS == 0)
        return -1;

    /*      
       (void)Delete("/d/recursive_stat1/recursive_stat2/recursive_stat3/recursive_stat4", false);
       (void)Delete("/d/recursive_stat1/recursive_stat2/recursive_stat3", false);
       (void)Delete("/d/recursive_stat1/recursive_stat2", false);
       (void)Delete("/d/recursive_stat1", false);
     */

    return 0;

}

int tBasicSeek() {
    //int fd, retW, retS1, retS2;
    int fd, retS1, retS2;

    /*
       char buffer[11];

       fd = Open("/d/basic6f", O_CREATE|O_WRITE);
       if (fd < 0)
       return -1;

       retW = Write( fd, buffer, 10);
       if (retW < 0)
       return -1;
     */

    fd = Open("/d/basic7f", O_READ);

    if(fd < 0) {
        Print("failed to open seekable file: %d\n", fd);
        return -1;
    }

    retS1 = Seek(fd, 0);
    retS2 = Seek(fd, 9);

    if(Close(fd) < 0) {
        Print("failed to close\n");
        return -1;
    }
    //(void)Delete("/d/basic6f", false);

    return ((retS1 >= 0) && (retS2 >= 0)) ? 1 : -1;
}

int tSeekReread() {
    //int fd, retW, retR, retS;
    int fd, retR, retS;
    //char buffer[11]="0123456789\0", buffer2[2], buffer3[2];
    char buffer2[2], buffer3[2];

    /*
       fd = Open("/d/basic7f", O_CREATE|O_WRITE);
       if (fd < 0)
       return -1;

       retW = Write( fd, buffer, 10);
       if (retW < 0)
       return -1;

       if(Close(fd) < 0) { Print("failed to close"); return -1; }
     */

    fd = Open("/d/basic7f", O_READ);
    if(fd < 0) {
        Print("failed to open\n");
        return -1;
    }

    retS = Seek(fd, 0);
    if(retS < 0) {
        Print("failed to seek\n");
        return -1;
    }

    retR = Read(fd, buffer2, 1);
    if(retR < 0) {
        Print("failed to read\n");
        return -1;
    }

    retS = Seek(fd, 9);
    if(retS < 0) {
        Print("failed to seek (2)\n");
        return -1;
    }

    retR = Read(fd, buffer3, 1);
    if(retR < 0) {
        Print("failed to read (2)\n");
        return -1;
    }

    if(Close(fd) < 0) {
        Print("failed to close");
        return -1;
    }
    //(void)Delete("/d/basic7f", false);

    return ((buffer2[0] == '0') && (buffer3[0] == '9')) ? 1 : -1;
}

int tCreat() {
    int retC;

    int fd = Open("/d/somefile", O_CREATE | O_READ);
    if(fd < 0)
        return -1;

    retC = Close(fd);

    (void)Delete("/d/somefile", false);

    return (retC >= 0) ? 1 : -1;
}

int tCreatLongFilename() {
    int retC;

    int fd =
        Open("/d/somePrettyLongFileNameToBeCreated", O_CREATE | O_READ);
    if(fd < 0)
        return -1;

    retC = Close(fd);

    (void)Delete("/d/somePrettyLongFileNameToBeCreated", false);

    return (retC >= 0) ? 1 : -1;
}

int tCreatInexistentPath() {
    int fd;

    fd = Open("/d/InexistentPath/file", O_CREATE | O_READ);

    return (fd < 0) ? 1 : -1;
}

int tBasicReadWrite() {
    int fd, retW, retR;
    char buffer[100];

    memset(buffer, 'g', 100);

    fd = Open("/d/basic10w", O_CREATE | O_WRITE);
    if(fd < 0)
        return -1;

    retW = Write(fd, buffer, 10);
    if(retW < 0) {
        Print("write return %d < 10\n", retW);
        return -1;
    }

    if(Close(fd) < 0) {
        Print("failed to close");
        return -1;
    }

    fd = Open("/d/basic10w", O_READ);
    if(fd < 0) {
        Print("couldn't reopen for reading: %d %s\n", fd,
              Get_Error_String(fd));
        return -1;
    }

    retR = Read(fd, buffer, 10);
    if(retR < 0) {
        Print("read return %d < 10\n", retR);
        return -1;
    }

    if(Close(fd) < 0) {
        Print("failed to close");
        return -1;
    }

    (void)Delete("/d/basic10w", false);

    return ((retW == 10) || (retR == 10)) ? 1 : -1;
}


int tReadFromWriteOnly() {
    int fd, retR, retW;
    char buffer[100];

    fd = Open("/d/basic2f", O_CREATE | O_WRITE);
    if(fd < 0)
        return -1;

    memset(buffer, 'n', sizeof(buffer));

    retW = Write(fd, buffer, 10);
    if(retW < 0)
        return -1;

    if(Close(fd) < 0) {
        Print("failed to close");
        return -1;
    }

    fd = Open("/d/basic2f", O_WRITE);
    if(fd < 0)
        return -1;

    retR = Read(fd, buffer, 10);

    if(Close(fd) < 0) {
        Print("failed to close");
        return -1;
    }

    (void)Delete("/d/basic2f", false);


    return (retR < 0) ? 1 : -1;
}

int tWriteToReadOnly() {
    int fd, retW;
    char buffer[100];

    fd = Open("/d/basic3f", O_CREATE | O_WRITE);
    if(fd < 0)
        return -1;

    memset(buffer, 'n', sizeof(buffer));
    retW = Write(fd, buffer, 10);
    if(retW < 0)
        return -1;

    if(Close(fd) < 0) {
        Print("failed to close");
        return -1;
    }

    fd = Open("/d/basic3f", O_READ);
    if(fd < 0)
        return -1;

    retW = Write(fd, buffer, 10);

    if(Close(fd) < 0) {
        Print("failed to close");
        return -1;
    }

    (void)Delete("/d/basic3f", false);


    return (retW < 0) ? 1 : -1;
}

int tBasicDelete() {
    int fd, retD, retS;
    struct VFS_File_Stat s;

    fd = Open("/d/basic5f", O_CREATE | O_WRITE);
    if(fd < 0)
        return -1;

    if(Close(fd) < 0) {
        Print("failed to close");
        return -1;
    }

    retD = Delete("/d/basic5f", false);

    retS = Stat("/d/basic5f", &s);

    return (retD >= 0 && retS < 0) ? 1 : -1;
}

int tDeleteInexistentFile() {
    int retD;

    retD = Delete("/d/InexistentFile2", false);

    return (retD < 0) ? 1 : -1;
}

int tBasicCreateDirectory() {
    int retC;

    retC = Create_Directory("/d/dir1d");

    (void)Delete("/d/dir1d", false);

    return (retC >= 0) ? 1 : -1;
}

int tRecursivelyCreateDirectory() {
    int retC;

    retC = Create_Directory("/d/dir2d");

    if(retC < 0)
        return -1;

    retC = Create_Directory("/d/dir2d/dir3d");

    (void)Delete("/d/dir2d/dir3d", false);
    (void)Delete("/d/dir2d", false);

    return (retC >= 0) ? 1 : -1;
}

int tFileInRecursivelyCreatedDirectory() {

    int retC;

    retC = Create_Directory("/d/dir2d");

    if(retC < 0)
        return -1;

    retC = Create_Directory("/d/dir2d/dir3d");

    int fd = Open("/d/dir2d/dir3d/file4f", O_CREATE | O_READ);
    if(fd < 0) {
        Print("failed to open d234");
        return -1;
    }
    if(Close(fd) < 0) {
        Print("failed to close");
        return -1;
    }

    (void)Delete("/d/dir2d/dir3d/file4f", false);
    (void)Delete("/d/dir2d/dir3d", false);
    (void)Delete("/d/dir2d", false);


    return (fd >= 0) ? 1 : -1;
}

int tDeleteEmptyDirectory() {
    int retC, retD;

    retC = Create_Directory("/d/dir3d");
    if(retC < 0)
        return -1;

    retD = Delete("/d/dir3d", false);

    return (retD >= 0) ? 1 : -1;
}

int tDeleteNonEmptyDirectory() {
    int retC, retD, fd;

    retC = Create_Directory("/d/dir4d");
    if(retC < 0)
        return -1;

    fd = Open("/d/dir4d/file", O_CREATE | O_READ);
    if(fd < 0)
        return -1;

    if(Close(fd) < 0) {
        Print("failed to close");
        return -1;
    }

    retD = Delete("/d/dir4d", false);

    (void)Delete("/d/dir4d/file", false);
    (void)Delete("/d/dir4d", false);

    return (retD >= 0) ? -1 : 1;
}

int tSync() {
    return Sync();
}

int tWriteReread(unsigned int howManyKBs, char const *fileName,
                 bool reverseWrite) {
    int fd, retW, retR, retS;
    int i, j;
    char buffer[100], buffer2[100];
    int ret = 1;
    struct VFS_File_Stat s;


    if(howManyKBs > ((blocksize / 4) + 4) * blocksize / 1024) {
        howManyKBs = ((blocksize / 4) + 4) * blocksize / 1024;
        Print("capping reread test at %d KB\n", howManyKBs);
    }

    for(j = 0; j < 100; j++)
        buffer[j] = j;

    fd = Open(fileName, O_CREATE | O_WRITE);
    if(fd < 0)
        return -1;

    if(reverseWrite) {
        for(i = (howManyKBs * 10) - 1; i >= 0; i--) {
            buffer[0] = i % 256;
            retS = Seek(fd, i * 100);
            if(retS != 0) {
                Print("seek failed\n");
                ret = -1;
                break;
            }
            retW = Write(fd, buffer, 100);
            if(retW != 100) {
                Print("write %d failed\n", i);
                ret = -1;
                break;
            }

            retS = FStat(fd, &s);
            if(retS != 0) {
                Print("fstat failed\n");
                ret = -1;
                break;
            }
            if((unsigned int)s.size != howManyKBs * 1000) {
                Print("fstat size check failed: %d != %d\n", s.size,
                      howManyKBs * 1000);
                ret = -1;
                break;
            }
            if(s.isDirectory) {
                Print("fstat isDirectory check failed\n");
                ret = -1;
                break;
            }

            if(i % 50 == 0 && i > 10)
                Print(" %d", i);
        }
    } else {
        for(i = 0; i < (int)(howManyKBs * 10); i++) {
            buffer[0] = i % 256;
            retW = Write(fd, buffer, 100);
            if(retW != 100) {
                ret = -1;
                break;
            }
            retS = FStat(fd, &s);
            if(retS != 0) {
                Print("fstat failed\n");
                ret = -1;
                break;
            }
            if(s.size < (i + 1) * 100) {
                Print("fstat size check failed: %d != %d\n", s.size,
                      (i + 1) * 100);
                ret = -1;
                break;
            }
            if(s.isDirectory) {
                Print("fstat isDirectory check failed\n");
                ret = -1;
                break;
            }


            if(i % 50 == 0 && i > 10)
                Print(" %d", i);
        }
    }

    if(Close(fd) < 0) {
        Print("failed to close");
        return -1;
    }

    retS = Stat(fileName, &s);
    if(retS != 0) {
        Print("stat failed\n");
        ret = -1;
    }
    if(s.size != (int)howManyKBs * 1000) {
        Print("stat size check failed: %d != %d\n", s.size,
              howManyKBs * 1000);
        ret = -1;
    }
    if(s.isDirectory) {
        Print("stat isDirectory check failed\n");
        ret = -1;
    }


    if(ret != -1) {
        fd = Open(fileName, O_READ);
        if(fd < 0)
            return -1;

        for(i = 0; i < (int)(howManyKBs * 10); i++) {
            retR = Read(fd, buffer2, 100);

            if(retR != 100) {
                Print("read %d failed", i);
                ret = -1;
                break;
            }

            if((unsigned char)buffer2[0] != (unsigned char)(i % 256)) {
                Print("mismatched ident %d != %d", buffer2[0], i % 256);
                ret = -1;
                break;
            }
            for(j = 1; j < 100; j++) {
                if(buffer2[j] != j) {
                    ret = -1;
                    break;
                }
            }

            if(ret < 0)
                break;

            if(i % 50 == 0 && i > 10)
                Print(" %d", i);
        }

        if(Close(fd) < 0) {
            Print("failed to close");
            return -1;
        }
        (void)Delete(fileName, false);
    }

    return ret;
}

int t10KWriteReread() {
    return tWriteReread(10, "/d/file_10k", false);
}

int t100KWriteReread() {
    return tWriteReread(100, "/d/file_100k", false);
}
int t10KWriteRereadR() {
    return tWriteReread(10, "/d/file_10kr", true);
}

int t100KWriteRereadR() {
    return tWriteReread(100, "/d/file_100kr", true);
}

int tBigDir() {
    int retC, retD, fi, retS;
    char fname[75];
    struct VFS_File_Stat s;
    const char *stuffing =
        "abcdefghijklmnopqrstuvwxyzABCDEFGHIJKLMNOPQRSTUVWXYZ";

    retC = Create_Directory("/d/bigdir");
    if(retC != 0 && retC != EEXIST) {
        Print("couldn't create /d/bigdir: %d\n", retC);
        return -1;
    }

    Print("Populating...\n");
    for(fi = 0; fi < 100; fi++) {
        int fd;

        snprintf(fname, 75, "/d/bigdir/%04d%s%04d", fi, stuffing, fi);
        Print((fi % 25 == 0) ? ":" : ".");
        fd = Open(fname, O_WRITE | O_CREATE);
        if(fd < 0) {
            Print("bad open/creat at %d\n", fi);
            return -1;
        }
        if(Close(fd) < 0) {
            Print("failed to close");
            return -1;
        }
        retS = Stat(fname, &s);
        if(retS < 0) {
            Print("bad stat at %d\n", fi);
            return -1;
        }
    }

    Print("Statting...\n");
    snprintf(fname, 75, "/d/bigdir/%04d%s%04d", fi, stuffing, fi);
    retS = Stat(fname, &s);
    if(retS == 0) {
        Print("bad extra stat at %d\n", fi);
        return -1;
    }

    Print("Cleaning up...\n");
    for(fi = 0; fi < 100; fi++) {
        snprintf(fname, 75, "/d/bigdir/%04d%s%04d", fi, stuffing, fi);
        Print((fi % 25 == 0) ? ":" : ".");
        (void)Delete(fname, false);
        retS = Stat(fname, &s);
        if(retS == 0) {
            return -1;
        }
    }

    retD = Delete("/d/bigdir", false);
    if(retD != 0) {
        Print("failed to remove /d/bigdir: %d", retD);
        return retD;
    }

    return 0;
}

int tExhaustDisk() {
    /* (indirect + direct) * blocksize div by B/KB */
    unsigned int max_file_size_k =
        ((blocksize / 4) + 4) * blocksize / 1024;
    int files_needed_to_fill_disk =
        disksize_mb * 1024 * 63 / 64 / max_file_size_k;
    int files_needed_to_use_inodes =
        disksize_mb * 1024 * 1024 / 64 / SIZEOF_STRUCT_GFS2_INODE;
    int fi, retC, retW, retD;
    char writeme[512];
    char fname[50];
    char dirname[25] = "";
    int repetition;
    unsigned int u;


    Print("need %d files to fill disk, %d to use all inodes\n",
          files_needed_to_fill_disk, files_needed_to_use_inodes);

    if(files_needed_to_fill_disk > files_needed_to_use_inodes) {
        files_needed_to_fill_disk = files_needed_to_use_inodes;
    }
#define MARK  Print("%d:", __LINE__);
    retC = Create_Directory("/d/exhaust");
    if(retC != 0) {
        Print("couldn't create /d/exhaust\n");
        return -1;
    }

    for(u = 0; u < sizeof(writeme); u++) {
        writeme[u] = u % 256;
    }

    for(repetition = 0; repetition < 3; repetition++) {
        int files_worked_on;
        retW = 0;
        for(fi = 0; retW >= 0 && fi < files_needed_to_fill_disk; fi++) {
            int fd;
            unsigned long b;

            if(fi % 100 == 0) {
                snprintf(dirname, 25, "/d/exhaust/%d", fi / 100);
                retC = Create_Directory(dirname);
                Print("%d/%d", fi, files_needed_to_fill_disk);
            }

            snprintf(fname, 50, "%s/%d", dirname, fi);
            Print((fi % 25 == 0) ? ":" : ".");

            fd = Open(fname, O_WRITE | O_CREATE);
            if(fd < 0) {
                Print("failed to open %s\n", fname);
                return -1;
            }

            for(b = 0; b < max_file_size_k * 1024 - 100; b += retW) {
                retW = Write(fd, writeme, 100);
                if(retW < 0) {
                    Print("write %s %lu (of %u) failed: %d\n", fname, b,
                          max_file_size_k * 1024, retW);
                    break;
                }
            }

            if(Close(fd) < 0) {
                Print("failed to close\n");
                return -1;
            }
        }
        files_worked_on = fi;
        for(; fi >= 0; fi--) {
            snprintf(dirname, 25, "/d/exhaust/%d", fi / 100);
            snprintf(fname, 50, "%s/%d", dirname, fi);
            (void)Delete(fname, false);
        }
        for(fi = 0; fi < files_worked_on; fi += 100) {
            snprintf(dirname, 25, "/d/exhaust/%d", fi / 100);
            if(Delete(dirname, false) < 0) {
                Print("couldnt remove %s", dirname);
                return -1;
            }
        }

    }

    retD = Delete("/d/exhaust", false);
    if(retD != 0) {
        Print("failed to remove /d/exhaust: %d", retD);
        return retD;
    }

    return 0;
}

/* test will probably take forever if the buffer cache is not in use. */
int tBufferCacher(void) {
    int i, retW, retS, retR;
    unsigned int u;
    unsigned char writeme[512];
    unsigned char readme[512];
    int fd = Open("/d/buffa", O_CREATE | O_WRITE | O_READ);

    for(u = 0; u < sizeof(writeme); u++) {
        writeme[u] = u % 256;
    }
    for(i = 0; i < 10000; i++) {
        if(i % 100 == 0) {
            if(i % 1000 == 0)
                Print(":");
            else
                Print(".");
        }
        retW = Write(fd, writeme, 512);
        if(retW < 0) {
            Print("Write failed: %d\n", retW);
            return -1;
        }
        retW = Write(fd, writeme, 512);
        if(retW < 0) {
            Print("Write failed: %d\n", retW);
            return -1;
        }
        retS = Seek(fd, 0);
        if(retS < 0) {
            Print("Seek failed: %d\n", retS);
            return -1;
        }
    }

    for(i = 0; i < 2; i++) {
        retR = Read(fd, readme, 512);
        if(retR < 0) {
            Print("Read back failed: %d\n", retR);
            return -1;
        }
        if(retR < 512) {
            Print("Read back too few: %d\n", retR);
            return -1;
        }
        for(u = 0; u < 512; u++) {
            if(readme[u] != u % 256) {
                Print("readback check %d failed.\n", i);
                return -1;
            }
        }
    }
    Close(fd);
    Delete("/d/file_1", false);
    return 0;
}

/* tests that should leave the filesystem happy. */
typedef int (*test_type) ();
test_type clean_tests[] = {
    t10KWriteReread,
    t10KWriteRereadR,
    tRecursiveStat,
    tCreat,
    tCreatLongFilename,
    tRecursivelyCreateDirectory,
    tFileInRecursivelyCreatedDirectory,
};

int tClean() {
    struct VFS_Dir_Entry dirEntry;
    int fd, retR;
    unsigned int i;

    for(i = 0; i < sizeof(clean_tests) / sizeof(void *); i++) {
        Print("prep %u\n", i);
        (clean_tests[i]) ();
    }

    Delete("/d/hello", false);  /* if in there */

    fd = Open_Directory("/d");
    for(i = 0; (retR = Read_Entry(fd, &dirEntry)) == 0 &&
        (dirEntry.name[0] == '.' ||
         strcmp("somedir", dirEntry.name) == 0 ||
         strcmp("basic", dirEntry.name) == 0 ||
         strcmp("basic7f", dirEntry.name) == 0 ||
         strcmp("basic11d", dirEntry.name) == 0 ||
         strcmp("recursive_stat1", dirEntry.name) == 0); i++) ;
    //  if(dirEntry.name[0] != '.') {
    // Print("non-dot entry %s\n", dirEntry.name);
    // return -1; /* not empty */
    //}
    //if(i>2) {
    //  Print("%u entries\n", i);
    //  return -1; /* many dotfiles */
    //}
    if(retR != VFS_NO_MORE_DIR_ENTRIES) {
        return -1;              /* failed out */
    }
    return 1;
}

int tDiskProp() {
    /* do nothing, since the disk properties below should be fine. */
    if(blocksize > 0 && blocks_on_disk > 0 && ((blocksize & 0xf) == 0))
        return 1;
    else
        return -1;              /* seem like bad params to have read. */
}

/* not used. 
void prepare_image_write()
{
  int ret;

  Print("Preparing the disk image now...\n");

  ret = ttestFormat();
  if (ret < 0) {
    Print("Format disk: failed.\n");
    return;
  }
}
*/

int prepare_image_read() {
    int ret;
    int fd;
    int i;
    char buffer[1000];
    char buffer_seek[11] = "0123456789\0";

    // For open file
    fd = Open("/d/somefile", O_CREATE | O_READ);
    if(fd < 0) {
        Print("Create /d/somefile: failed %d %s\n", fd,
              Get_Error_String(fd));
        return -1;
    }

    ret = Close(fd);
    if(ret < 0) {
        Print("Close /d/somefile: failed %d %s\n", ret,
              Get_Error_String(ret));
        return -1;
    }
    // For open dir
    ret = Create_Directory("/d/somedir");
    if(ret < 0) {
        Print("Create /d/somedir: failed %d %s\n", ret,
              Get_Error_String(ret));
        return -1;
    }
    // For read file
    fd = Open("/d/basic", O_CREATE | O_WRITE);
    if(fd < 0) {
        Print("failed to open /d/basic: %d %s\n", fd,
              Get_Error_String(fd));
        return -1;
    }

    /* thanks to D. Minch for noticing the interaction between p4 stack expansion 
       and this write buffer. */
    memset(buffer, 'g', 1000);
    for(i = 0; i < 10; i++) {
        ret = Write(fd, buffer, 1000);
        if(ret < 0) {
            Print("write to /d/basic return %d < 10\n", ret);
            return -1;
        }
    }

    if(Close(fd) < 0) {
        Print("failed to close /d/basic: %d %s\n", fd,
              Get_Error_String(fd));
        return -1;
    }
    // For seek
    fd = Open("/d/basic7f", O_CREATE | O_WRITE);
    if(fd < 0) {
        Print("Create /d/basic7f: failed %d %s\n", ret,
              Get_Error_String(ret));
        return -1;
    }

    ret = Write(fd, buffer_seek, 10);
    if(ret < 0) {
        Print("Write to /d/basic7f: failed %d %s\n", ret,
              Get_Error_String(ret));
        return -1;
    }

    if(Close(fd) < 0) {
        Print("failed to close /d/basic7d: %d %s\n", fd,
              Get_Error_String(fd));
        return -1;
    }
    // For read entry
    ret = Create_Directory("/d/basic11d");
    if(ret < 0) {
        Print("couldn't create basic11d: %d %s\n", ret,
              Get_Error_String(ret));
        return -1;
    }

    ret = Create_Directory("/d/basic11d/d1");
    if(ret < 0) {
        Print("couldn't create basic11d/d1: %d %s\n", ret,
              Get_Error_String(ret));
        return -1;
    }

    ret = Create_Directory("/d/basic11d/d2");
    if(ret < 0) {
        Print("couldn't create basic11d/d2: %d %s\n", ret,
              Get_Error_String(ret));
        return -1;
    }

    fd = Open("/d/basic11d/f1", O_CREATE | O_WRITE);

    for(i = 0; i < 10; i++) {
        ret = Write(fd, buffer, 1000);
        if(ret < 0) {
            Print("write to /d/basic11d/f1 return %d < 10\n", ret);
            return -1;
        }
    }

    if(fd < 0) {
        Print("couldn't open /d/basic11d/f1: %d %s\n", fd,
              Get_Error_String(fd));
        return -1;
    }

    if(Close(fd) < 0) {
        Print("failed to close /d/basic11d/f1: %d %s\n", fd,
              Get_Error_String(fd));
        return -1;
    }
    // For recursive stat
    ret = Create_Directory("/d/recursive_stat1");
    if(ret < 0) {
        Print("Create /d/recursive_stat1: failed %d %s\n", ret,
              Get_Error_String(ret));
        return -1;
    }

    ret = Create_Directory("/d/recursive_stat1/recursive_stat2");
    if(ret < 0) {
        Print("Create /d/recursive_stat1/recursive_stat2: failed %d %s\n",
              ret, Get_Error_String(ret));
        return -1;
    }

    ret =
        Create_Directory
        ("/d/recursive_stat1/recursive_stat2/recursive_stat3");
    if(ret < 0) {
        Print
            ("Create /d/recursive_stat1/recursive_stat2/recursive_stat3: failed %d %s\n",
             ret, Get_Error_String(ret));
        return -1;
    }

    fd = Open
        ("/d/recursive_stat1/recursive_stat2/recursive_stat3/recursive_stat4",
         O_CREATE | O_WRITE);
    if(fd < 0) {
        Print
            ("Create /d/recursive_stat1/recursive_stat2/recursive_stat3/recursive_stat4: failed %d %s\n",
             ret, Get_Error_String(ret));
        return -1;
    }

    for(i = 0; i < 10; i++) {
        ret = Write(fd, buffer, 1000);
        if(ret < 1000) {
            Print
                ("write to /d/recursive_stat1/recursive_stat2/recursive_stat3/recursive_stat4 return %d < 1000\n",
                 ret);
            return -1;
        }
    }

    ret = Close(fd);
    if(ret < 0) {
        Print
            ("Close /d/recursive_stat1/recursive_stat2/recursive_stat3/recursive_stat4: failed %d %s\n",
             fd, Get_Error_String(fd));
        return -1;
    }

    if((ret = Sync()) < 0) {
        Print("FAIL: unable to prep because Sync returned %d\n", ret);
        return -1;
    }
    /* for good measure */
    return 1;
}

struct test {
    const char *name;
    int (*function) (void);
    int alleged_points;
    int dieOnFailure;
} all_tests[] = {
    {
    "", NULL, 0, 0}, {
    "Mount", ttestMount, 1, 1}, {
    "Open-Inexistent File", tOpenInexistentFile, 1, 0}, {
    "Creat", tCreat, 2, 1}, {
    "Creat-Long Filename", tCreatLongFilename, 3, 0},
        /* 5: */
    {
    "Creat-Inexistent Path", tCreatInexistentPath, 3, 0}, {
    "Basic Read/Write", tBasicReadWrite, 5, 0}, {
    "Read from WriteOnly", tReadFromWriteOnly, 3, 0}, {
    "Write to ReadOnly", tWriteToReadOnly, 3, 0}, {
    "Close Twice", tCloseTwice, 3, 0},
        /* 10: */
    {
    "Close Illegal FD", tCloseAberrantFd, 1, 0}, {
    "Basic Delete", tBasicDelete, 3, 0}, {
    "Delete-Inexistent File", tDeleteInexistentFile, 1, 0}, {
    "Basic Create Directory", tBasicCreateDirectory, 3, 0}, {
    "Recursively Create Directory", tRecursivelyCreateDirectory, 5, 0},
        /* 15: */
    {
    "Create File in Recursively Created Directory",
            tFileInRecursivelyCreatedDirectory, 3, 0}, {
    "Basic Seek", tBasicSeek, 2, 0}, {
    "Seek w/ Reread", tSeekReread, 5, 0}, {
    "Basic Stat", tBasicStat, 2, 0}, {
    "Stat-File", tStatFile, 2, 0},
        /* 20: */
    {
    "Stat-Directory", tStatDirectory, 2, 0}, {
    "Delete Empty Directory", tDeleteEmptyDirectory, 3, 0}, {
    "Delete Non-Empty Directory", tDeleteNonEmptyDirectory, 2, 0}, {
    "10k Write/Reread", t10KWriteReread, 4, 0}, {
    "100k Write/Reread", t100KWriteReread, 6, 0},
        /* 25: */
    {
    "10k Write/Reread Reverse", t10KWriteRereadR, 4, 0}, {
    "100k Write/Reread Reverse", t100KWriteRereadR, 6, 0}, {
    "Read Entry", tReadEntry, 4, 0}, {
    "Sync", tSync, 1, 0}, {
    "Recursive Stat", tRecursiveStat, 4, 0},
        /* 30: */
    {
    "Big Directory", tBigDir, 7, 0}, {
    "Exhaust Disk", tExhaustDisk, 7, 0}, {
    "House Is Clean", tClean, 2, 0}, {
    "Open existing file", tOpenFile, 2, 0}, {
    "Open existing directory", tOpenDir, 2, 0},
        /* 35: */
    {
    "Read file", tReadFile, 2, 0}, {
    "Buffer Cache", tBufferCacher, 4, 0}, {
"Disk Properties", tDiskProp, 4, 0},};

int main(int argc, char **argv) {


    int score = 0;
    int totalTests = 0;
    int successfulTests = 0;
    int task_index;

    /* Stuff for formatting, if formatting done within the kernel. */
    /*
       if(argc > 2) {
       disksize_mb = atoi(argv[1]);
       blocksize = atoi(argv[2]);
       if(blocksize != 512 && blocksize != 1024 && blocksize != 4096) {
       Print("illegal blocksize %s", argv[2]);
       Print("usage p5test [disksize_mb blocksize_b]");
       }
       if(disksize_mb == 0 || disksize_mb > 32) {
       Print("illegal disksize %s", argv[1]);
       Print("usage p5test [disksize_mb blocksize_b]");
       }
       }
     */
    if(argc != 2 && argc != 3) {
        Print("Usage: gfs2tst2 [<task index>|all|prep] [halt]\n");
        return 0;
    }

    task_index = atoi(argv[1]);
    Print("Task index is %d.\n", task_index);

    if(task_index != 1) {
        Print("Invoking preliminary mount\n");
        if(ttestMount() < 0) {
            int fd = Open_Directory("/d/.");
            if(fd >= 0) {
                Print("perhaps already mounted\n");
                Close(fd);
            } else {
                Exit(1);
            }
        }
        Print("preliminary mount apparently successful\n");
    }

    if(Disk_Properties("/d/.", &blocksize, &blocks_on_disk) == 0) {
        /* this message searched for by the diskprop test: */
        Print("Retrieved disk parameters, blocksize %u, blocks %u\n",
              blocksize, blocks_on_disk);
        disksize_mb = blocksize * blocks_on_disk / 1024 / 1024;
    };

    /* the disk image on submit is already prepared. */
    /* this action populates the readable components onto the 
       file system, but requires that the creation / writing 
       functions be implemented. */
    if(strcmp(argv[1], "prep") == 0) {
        int rc;
        if((rc = prepare_image_read()) < 0) {
            Print("FAIL: unable to prep for reasons stated above.\n");
        }
        if((rc = Sync()) < 0) {
            Print("FAIL: unable to prep because Sync returned %d\n", rc);
        }                       /* for good measure */
        if(argc == 3 && strcmp(argv[2], "halt") == 0) {
            Print("Shutting down...\n");
            ShutDown();
        }
        Exit(0);                /* unreachable */
    }
    // 0
    //doTestOrDie( "Format", ttestFormat, 1,  &score, &totalTests, &successfulTests);
    // 1

    if(task_index == 0) {
        for(task_index = 2;
            task_index < (int)(sizeof(all_tests) / sizeof(struct test));
            task_index++) {
            if(all_tests[task_index].dieOnFailure) {
                doTestOrDie(all_tests[task_index].name,
                            all_tests[task_index].function,
                            all_tests[task_index].alleged_points,
                            &score, &totalTests, &successfulTests);
            } else {
                doTest(all_tests[task_index].name,
                       all_tests[task_index].function,
                       all_tests[task_index].alleged_points,
                       &score, &totalTests, &successfulTests);
            }
        }
        if(argc == 3 && strcmp(argv[2], "halt") == 0) {
            Print("Shutting down...\n");
            ShutDown();
        }
        Exit(0);
    }

    if(all_tests[task_index].dieOnFailure) {
        doTestOrDie(all_tests[task_index].name,
                    all_tests[task_index].function,
                    all_tests[task_index].alleged_points,
                    &score, &totalTests, &successfulTests);
    } else {
        doTest(all_tests[task_index].name,
               all_tests[task_index].function,
               all_tests[task_index].alleged_points,
               &score, &totalTests, &successfulTests);
    }

    if(argc == 3 && strcmp(argv[2], "halt") == 0) {
        Print("Shutting down...\n");
        ShutDown();
    }

    Exit(0);

    //Print ("********************************************\n");
    //Print ("Tests attempted: %d. Passed: %d. Failed: %d\n", totalTests, successfulTests, (totalTests-successfulTests) );
    //Print ("SCORE: %d\n", score);

    return 0;
}
