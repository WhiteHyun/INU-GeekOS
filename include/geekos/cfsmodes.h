
typedef struct {
    int caseInsensitiveFileNames:1;
    int usesBufferCache:1;
    int usesLFS:1;
    int recursiveDelete:1;
    int usesBU:1;
    int bigBlockSize:1;
    int usesHardLinks:1;
    int usesGlobalNamePool:1;
} cfsModes;
