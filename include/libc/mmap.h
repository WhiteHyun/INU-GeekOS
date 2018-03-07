

#define PROT_EXEC  0x01         // Pages may be executed.
#define PROT_READ  0x02         // Pages may be read.
#define PROT_WRITE 0x04         // Pages may be written.
#define PROT_NONE  0x00         // Pages may not be accessed.

// Flags
#define MAP_SHARED  0x01        // Share this mapping.
#define MAP_PRIVATE 0x2         // Only this process see changes

extern void *Mmap(void *addr, unsigned int length, int prot, int flags,
                  int fd);
extern int Munmap(void *addr);
