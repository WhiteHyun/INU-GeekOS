/*
 * Physical memory allocation
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
 * $Revision: 1.45 $
 * 
 */

#include <geekos/defs.h>
#include <geekos/ktypes.h>
#include <geekos/kassert.h>
#include <geekos/kthread.h>
#include <geekos/bootinfo.h>
#include <geekos/gdt.h>
#include <geekos/screen.h>
#include <geekos/int.h>
#include <geekos/malloc.h>
#include <geekos/string.h>
#include <geekos/paging.h>
#include <geekos/mem.h>
#include <geekos/smp.h>
#include <geekos/projects.h>

/* ----------------------------------------------------------------------
 * Global data
 * ---------------------------------------------------------------------- */

/*
 * List of Page structures representing each page of physical memory.
 */
struct Page *g_pageList;

/*
 * Number of pages currently available on the freelist.
 */
uint_t g_freePageCount = 0;

/* ----------------------------------------------------------------------
 * Private data and functions
 * ---------------------------------------------------------------------- */

/*
 * Defined in paging.c
 */
extern int debugFaults;
#define Debug(args...) if (debugFaults) Print(args)

/*
 * List of pages available for allocation.
 */
static struct Page_List s_freeList;

/*
 * Total number of physical pages.
 */
int unsigned g_numPages;


/*
 * Add a range of pages to the inventory of physical memory.
 */
static void Add_Page_Range(ulong_t start, ulong_t end, int flags) {
    ulong_t addr;

    KASSERT(Is_Page_Multiple(start));
    KASSERT(Is_Page_Multiple(end));
    KASSERT(start < end);

    for(addr = start; addr < end; addr += PAGE_SIZE) {
        struct Page *page = Get_Page(addr);

        page->flags = flags;

        if(flags == PAGE_AVAIL) {
            /* Add the page to the freelist */
            Unchecked_Add_To_Back_Of_Page_List(&s_freeList, page);

            /* Update free page count */
            ++g_freePageCount;
        } else {
            Set_Next_In_Page_List(page, 0);
            Set_Prev_In_Page_List(page, 0);
        }

        page->clock = 0;
        page->vaddr = 0;
        page->context = NULL;
        page->entry = 0;
    }
}

/* ----------------------------------------------------------------------
 * Public functions
 * ---------------------------------------------------------------------- */

/*
 * The linker defines this symbol to indicate the end of
 * the executable image.
 */
extern char end;

/*
 * Initialize memory management data structures.
 * Enables the use of Alloc_Page() and Free_Page() functions.
 */
void Init_Mem(struct Boot_Info *bootInfo) {
    if(!bootInfo->memSizeKB) {
        /* look through memsize for a region starting at 0x100000 */
        int i;

        for(i = 0; i < bootInfo->numMemRegions; i++) {
            if((bootInfo->memRegions[i].baseAddr_low == 0x100000) &&
               (bootInfo->memRegions[i].type == 1)) {
                bootInfo->memSizeKB =
                    bootInfo->memRegions[i].length_low / 1024;
            }
        }
        bootInfo->memSizeKB += 0x1000;
    }

    ulong_t numPages = bootInfo->memSizeKB >> 2;
    ulong_t endOfMem = numPages * PAGE_SIZE;
    unsigned numPageListBytes = sizeof(struct Page) * numPages;
    ulong_t pageListAddr;
    ulong_t kernEnd;
    ulong_t pageListEnd;

    KASSERT(bootInfo->memSizeKB > 0);

    /*
     * Before we do anything, switch from setup.asm's temporary GDT
     * to the kernel's permanent GDT.
     */
    Init_GDT(0);

    /*
     * We'll put the list of Page objects right after the end
     * of the kernel, and mark it as "kernel".  This will bootstrap
     * us sufficiently that we can start allocating pages and
     * keeping track of them.
     */
    pageListAddr = (HIGHMEM_START + KERNEL_HEAP_SIZE);
    if(pageListAddr >= endOfMem) {
        Print
            ("there is no memory for the page list.  physical memory is too small for the heap %u, bytes after %u. endOfMem=%lu .",
             KERNEL_HEAP_SIZE, HIGHMEM_START, endOfMem);
        KASSERT0(pageListAddr < endOfMem,
                 "there is no memory for the page list.  physical memory is too small for the heap.");
    }
    g_pageList = (struct Page *)pageListAddr;
    pageListEnd = Round_Up_To_Page(pageListAddr + numPageListBytes);

    // clear page list
    memset((void *)g_pageList, '\0',
           (pageListEnd - (ulong_t) g_pageList));

    kernEnd = Round_Up_To_Page((int)&end);
    g_numPages = numPages;

    /* would be clearly bad: */
    KASSERT(kernEnd < ISA_HOLE_START);

    /* encroaches the EBDA: */
    KASSERT0(kernEnd < 0x9c000,
             "kernel encroaches EBDA; reduce code or globals.");

    /*
     * The initial kernel thread and its stack are placed
     * just beyond the ISA hole.
     */
    KASSERT(ISA_HOLE_END == KERN_THREAD_OBJ);
    KASSERT(KERN_STACK == KERN_THREAD_OBJ + PAGE_SIZE);

    /* make sure BSS ends before our first structure */
    // extern char BSS_END, INITSEG;
    Print("BSS = %x\n", (int)&BSS_END);
    Print("start kern info = %x\n", bootInfo->startKernInfo);
    KASSERT0(((int)&BSS_END) < bootInfo->startKernInfo,
             "end of kernel BSS segment is after the start of, need a smaller kernel");

    /*
     * Memory looks like this:
     * 0 - start: available (might want to preserve BIOS data area)
     * start - end: kernel
     * end - ISA_HOLE_START: available
     * ISA_HOLE_START - ISA_HOLE_END: used by hardware (and ROM BIOS?)
     * ISA_HOLE_END - HIGHMEM_START: used by initial kernel thread
     * HIGHMEM_START - end of memory: available
     *    (the kernel heap is located at HIGHMEM_START; any unused memory
     *    beyond that is added to the freelist)
     */

    Add_Page_Range(0, PAGE_SIZE, PAGE_UNUSED);
    Add_Page_Range(PAGE_SIZE, KERNEL_START_ADDR, PAGE_AVAIL);
    Add_Page_Range(KERNEL_START_ADDR, kernEnd, PAGE_KERN);
    Add_Page_Range(kernEnd, ISA_HOLE_START, PAGE_AVAIL);
    Add_Page_Range(ISA_HOLE_START, ISA_HOLE_END, PAGE_HW);
    Add_Page_Range(ISA_HOLE_END, HIGHMEM_START, PAGE_ALLOCATED);
    Add_Page_Range(HIGHMEM_START, HIGHMEM_START + KERNEL_HEAP_SIZE,
                   PAGE_HEAP);
    Add_Page_Range(pageListAddr, pageListEnd, PAGE_KERN);
    if(pageListEnd > endOfMem) {
        KASSERT0(pageListEnd < endOfMem,
                 "there is no memory after the page list.  physical memory is too small.");
        /* this would fail at the next line (add_page_range), so this kassert just fails early. */
    }
    Add_Page_Range(pageListEnd, endOfMem, PAGE_AVAIL);

    /* Initialize the kernel heap */
    Init_Heap(HIGHMEM_START, KERNEL_HEAP_SIZE);

    Print
        ("%uKB memory detected, %u pages in freelist, %d bytes in kernel heap\n",
         bootInfo->memSizeKB, g_freePageCount, KERNEL_HEAP_SIZE);
}

/*
 * Initialize the .bss section of the kernel executable image.
 */
void Init_BSS(void) {
    extern char BSS_START, BSS_END;

    /* Fill .bss with zeroes */
    memset(&BSS_START, '\0', &BSS_END - &BSS_START);
}

static void *Alloc_Page_Frame(void) {
    struct Page *page;
    void *result = 0;

    /* ns14 rewritten partly to avoid race. */

    /* See if we have a free page */
    /* Remove the first page on the freelist. */
    page = Remove_From_Front_Of_Page_List(&s_freeList);
    if(page) {
        KASSERT((page->flags & PAGE_ALLOCATED) == 0);
        /* Mark page as having been allocated. */
        page->flags |= PAGE_ALLOCATED;
        g_freePageCount--;
        result = (void *)Get_Page_Address(page);
    }


    if(result) {
        memset(result, '\0', 4096);
    }
    return result;
}

/*
 * Choose a page to evict.
 * Returns null if no pages are available.
 */
static struct Page *Find_Page_To_Page_Out() {
    unsigned int i;
    struct Page *curr, *best;
    best = NULL;
    for(i = 0; i < g_numPages; i++) {
        if((g_pageList[i].flags & PAGE_PAGEABLE) &&
           (g_pageList[i].flags & PAGE_ALLOCATED)) {
            if(!best)
                best = &g_pageList[i];
            curr = &g_pageList[i];
            if((curr->clock < best->clock) &&
               (curr->flags & PAGE_PAGEABLE)) {
                best = curr;
            }
        }
    }
    return best;
}


void Lock_Page(struct Page *page) {
    KASSERT(page->flags & PAGE_ALLOCATED);
    KASSERT(!(page->flags & PAGE_LOCKED));
    /* Lock the page */
    page->flags |= (PAGE_LOCKED);
}

/* actually called both when a page fault reader / writer unlocks the page
   and at the end of free_page. Leaves the page in an unlocked state, but 
   can be called to release a page even when not locked. */
void Unlock_Page(struct Page *page) {

    /* When a page is freed, the ALLOCATED bit is cleared.  If a locked page 
       is freed, the page is not returned to the free list, since some thread
       is still busy evicting it. */

    if(!(page->flags & PAGE_ALLOCATED)) {
        /* clear the PTE this used to refer to */
        page->entry = 0;

        page->context = (void *)0xbad10000;

        /* Put the page back on the freelist */
        Unchecked_Add_To_Back_Of_Page_List(&s_freeList, page);
        g_freePageCount++;

    }

    /* Unlock the page */
    page->flags &= ~(PAGE_LOCKED);
}


/**
 * Allocate a page of pageable physical memory, to be mapped
 * into a user address space.
 *
 * @param entry pointer to user page table entry which will
 *   refer to the allocated page
 * @param vaddr virtual address where page will be mapped
 *   in user address space
 */
static void *Alloc_Or_Reclaim_Page(pte_t * entry, ulong_t vaddr,
                                   bool pinnedPage) {
    bool iflag;
    bool mappedPage;
    void *paddr;
    struct Page *page;
    int enabled = 0;

  start_over:

    /* Alloc_Page_Frame should be called before the atomics,
       since it acts over a locked list. */
    paddr = Alloc_Page_Frame();

    Disable_Interrupts();

    KASSERT(!Interrupts_Enabled());
    KASSERT(Kernel_Is_Locked());
    KASSERT(Is_Page_Multiple(vaddr));

    if(paddr != 0) {
        page = Get_Page((ulong_t) paddr);
        KASSERT((page->flags & PAGE_PAGEABLE) == 0);
    } else {
        int pagefileIndex = 0;

        /* Select a page to steal from another process */
        Debug("About to hunt for a page to page out\n");
        page = Find_Page_To_Page_Out();
        KASSERT(page->flags & PAGE_PAGEABLE);
        paddr = (void *)Get_Page_Address(page);
        Debug("Selected page at addr %p (age = %d)\n", paddr,
              page->clock);

        /* Make the page temporarily unpageable (can't let another process steal it) */
        page->flags &= ~(PAGE_PAGEABLE);

        /* Lock the page so it cannot be freed while we're writing */
        Debug("locking page at %p for writing\n", paddr);
        Lock_Page(page);
        TODO_P(PROJECT_VIRTUAL_MEMORY_B,
               "write page out to backing storage");
        TODO_P(PROJECT_MMAP, "write page out to backing storage");



        Unlock_Page(page);

        /* XXX - flush TLB should only flush the one page */
        Flush_TLB();
    }

    /* Fill in accounting information for page */
    if(pinnedPage) {
        page->flags &= ~(PAGE_PAGEABLE);
        page->entry = NULL;     /* will not appear in a page table, since it's not a pageable page. */
        page->vaddr = 0;        /* has no virtual address */
        page->context = NULL;   /* and is not associated with a user process */
    } else {
        page->flags |= PAGE_PAGEABLE;
        page->entry = entry;
        page->entry->kernelInfo = 0;
        page->vaddr = vaddr;
        /* note that the page context here will not be correct while copying during a fork. */
        page->context = CURRENT_THREAD->userContext;
        KASSERT(page->flags & PAGE_ALLOCATED);
    }

  done:
    KASSERT(Kernel_Is_Locked());
    if(!enabled)
        Enable_Interrupts();
    return paddr;
}

/*
 * Allocate a pinned (non-pageable) page of physical memory.
 */
void *Alloc_Page(void) {
    void *ret;
    int enabled = 0;

    if(!Interrupts_Enabled()) {
        Enable_Interrupts();
        enabled = 1;
    }
    ret = Alloc_Or_Reclaim_Page(NULL, 0, true);

    if(enabled)
        Disable_Interrupts();

    /*  nice idea, maybe, but there are call sites that handle 
       Alloc_Page failure. and quitting is not cool; uncomment
       if you want.  likely better to kassert. */

    /* if (!ret) {
       Print("Kernel Memory Exhausted, shutting down\n");
       extern void Hardware_Shutdown();
       Hardware_Shutdown();
       }
     */

    return ret;
}

/**
 * Allocate a page of pageable physical memory, to be mapped
 * into a user address space.
 *
 * @param entry pointer to user page table entry which will
 *   refer to the allocated page
 * @param vaddr virtual address where page will be mapped
 *   in user address space
 */
void *Alloc_Pageable_Page(pte_t * entry, ulong_t vaddr) {
    void *ret;
    int enabled = 0;

    if(!Interrupts_Enabled()) {
        enabled = 1;
        Enable_Interrupts();
    }
    ret = Alloc_Or_Reclaim_Page(entry, vaddr, false);
    if(enabled)
        Disable_Interrupts();
    return ret;
}

/*
 * Free a page of physical memory.
 */
void Free_Page(void *pageAddr) {
    ulong_t addr = (ulong_t) pageAddr;
    struct Page *page;

    KASSERT0(addr < (g_numPages << 12),
             "Attempted to free an invalid physical page");
    KASSERT(Is_Page_Multiple(addr));
    /* Get the Page object for this page */
    page = Get_Page(addr);
    /* should be impossible to get a bad page. */
    KASSERT0(page,
             "Couldn't find a struct Page * for the given pageAddr");

    /* waste of time? */
    memset(pageAddr, '\0', 4096);
    // Print("freeing %p because of %lx\n", pageAddr, (ulong_t) __builtin_return_address(0));

    KASSERT0((page->flags & PAGE_ALLOCATED) != 0,
             "Expected Free_Page parameter to have been allocated.");

    /* 
       Debug("Free %spage at %p (%d others)\n", 
       ( page->flags & PAGE_LOCKED ) ? "locked " : "", pageAddr, g_freePageCount);
     */
    /* Clear the allocation bit */
    page->flags &= ~(PAGE_ALLOCATED);

    /* When a page is locked, don't free it just let other thread know its not needed 
       by clearing PAGE_ALLOCATED */
    if(page->flags & PAGE_LOCKED) {
        page->entry = 0;
        page->context = NULL;
        return;
    }

    /* Clear the pageable bit */
    page->flags &= ~(PAGE_PAGEABLE);

    /* page is no longer locked or allocated.  free it. */
    Unlock_Page(page);
}
