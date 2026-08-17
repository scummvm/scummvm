#include <ddb.h>
#include <ddb_xmsg.h>

#include <os_file.h>
#include <os_lib.h>
#include <os_mem.h>

#if HAS_XMSG

#define XMSG_MAXFILESIZE     65536
#define XMSG_MAXMSGSIZE        512
#define XMSG_CACHEBLOCKSIZE   1024          // *MUST* be a power of 2
#define XMSG_TERMINATOR       0xF5

bool xmsgFilePresent = false;

typedef struct 
{
    bool        free;
    uint8_t*    data;
    uint32_t    offset;
    uint16_t    size;
    uint32_t    usage;
    bool        tail;
}
XMCacheBlock;

static File*            xmsgFile = 0;
static uint32_t         xmsgFileSize = 0;
static uint8_t*         xmsgScratch = 0;

static uint8_t*         cacheData = 0;
static int              cacheBlockCount = 0;
static XMCacheBlock*    cacheBlocks = 0;
static uint32_t         usageToken = 0;

static const uint8_t* CheckMessage(uint8_t* msg, uint32_t offset, uint16_t size)
{
    for (int n = 0; n < size; n++)
    {
        if (msg[n] == XMSG_TERMINATOR)
            return (const uint8_t*)msg;
    }
    DebugPrintf("WARNING: XMessage at offset %lu has no terminator character", (unsigned long)offset);
    msg[size-1] = XMSG_TERMINATOR;
    return (const uint8_t*)msg;
}

int DDB_InitializeXMessageCache(uint32_t size)
{
    if (!xmsgFilePresent)
        return DDB_ERROR_NONE;
    if (cacheData != 0 || cacheBlocks != 0)
    {
        DebugPrintf("WARNING: XMessage cache initialized twice!");
        DDB_FreeXMessageCache();
    }

    if (xmsgScratch == 0)
    {
        xmsgScratch = Allocate<uint8_t>("XMESSAGE Cache", XMSG_MAXMSGSIZE);
        if (xmsgScratch == 0)
            return DDB_ERROR_OUT_OF_MEMORY;
    }

    if (size > xmsgFileSize)
        size = xmsgFileSize;
    size = (size + XMSG_CACHEBLOCKSIZE - 1) & ~(XMSG_CACHEBLOCKSIZE - 1);

    cacheBlockCount = size / XMSG_CACHEBLOCKSIZE;
    cacheData = Allocate<uint8_t>("XMESSAGE Cache", cacheBlockCount * XMSG_CACHEBLOCKSIZE);
    if (cacheData == 0)
    {
        cacheBlockCount = 0;
        return DDB_ERROR_OUT_OF_MEMORY;
    }
    cacheBlocks = Allocate<XMCacheBlock>("XMESSAGE Cache", cacheBlockCount);
    if (cacheBlocks == 0)
    {
        cacheBlockCount = 0;
        Free(cacheData);
        cacheData = 0;
        return DDB_ERROR_OUT_OF_MEMORY;
    }

    uint8_t* ptr = cacheData;
    for (int n = 0; n < cacheBlockCount; n++)
    {
        cacheBlocks[n].data = ptr;
        cacheBlocks[n].offset = 0;
        cacheBlocks[n].free = true;
        ptr += XMSG_CACHEBLOCKSIZE;
    }

    return DDB_ERROR_NONE;
}

void DDB_FreeXMessageCache()
{
    if (xmsgScratch != 0)
    {
        Free(xmsgScratch);
        xmsgScratch = 0;
    }
    if (cacheData != 0)
    {
        Free(cacheData);
        cacheData = 0;
    }
    if (cacheBlocks != 0)
    {
        Free(cacheBlocks);
        cacheBlocks = 0;
    }
    cacheBlockCount = 0;
}

bool DDB_OpenXMessageFile (const char* filename)
{
    if (xmsgFile != 0)
        return true;

    xmsgFile = File_Open(ChangeExtension(filename, ".xmb"));
    if (!xmsgFile)
        xmsgFile = File_Open(ChangeExtension(filename, ".XMB"));
    if (!xmsgFile)
        xmsgFile = File_Open("0.XMB");
    if (!xmsgFile)
        xmsgFile = File_Open("0.xmb");

    if (xmsgFile)
    {
        xmsgFileSize = File_GetSize(xmsgFile);
        DebugPrintf("XMessage file opened (%lu bytes)\n", (unsigned long)xmsgFileSize);
        if (xmsgFileSize > XMSG_MAXFILESIZE)
        {
            DebugPrintf("WARNING: XMessage file size %lu exceedes specs max", (unsigned long)xmsgFileSize);
        }
    }

    return xmsgFilePresent = (xmsgFile != 0);
}

void DDB_CloseXMessageFile()
{
    if (xmsgFile != 0)
    {
        File_Close(xmsgFile);
        for (int n = 0; n < cacheBlockCount; n++)
            cacheBlocks[n].free = true;
        xmsgFile = 0;
    }

    xmsgFilePresent = false;
}

static XMCacheBlock* GetFirstFreeBlock()
{
    for (int n = 0; n < cacheBlockCount; n++)
    {
        if (cacheBlocks[n].free)
            return &cacheBlocks[n];
    }
    return 0;
}

static XMCacheBlock* EvictOldestCachedBlock(XMCacheBlock* inuse)
{
    if (cacheBlocks == 0)
        return 0;

    XMCacheBlock* oldest = 0;
    for (int n = 0; n < cacheBlockCount; n++)
    {
        if ((oldest == 0 || oldest->usage > cacheBlocks[n].usage) 
            && inuse != &cacheBlocks[n])
        {
            oldest = &cacheBlocks[n];
        }
    }
    if (oldest != 0)
        oldest->free = true;
    return oldest;
}

static XMCacheBlock* GetCachedBlock(uint32_t offset, XMCacheBlock* inuse = 0)
{
    for (int n = 0; n < cacheBlockCount; n++)
    {
        if (cacheBlocks[n].free == false &&
            cacheBlocks[n].offset <= offset &&
            cacheBlocks[n].offset + XMSG_CACHEBLOCKSIZE > offset)
            return &cacheBlocks[n];
    }

    XMCacheBlock* block = GetFirstFreeBlock();
    if (block == 0)
        block = EvictOldestCachedBlock(inuse);
    if (block != 0)
    {
        uint32_t fileOffset = offset - (offset & (XMSG_CACHEBLOCKSIZE-1));
        if (!File_Seek(xmsgFile, fileOffset))
        {
            DDB_SetError(DDB_ERROR_SEEKING_FILE);
            return 0;
        }

        uint32_t readSize = XMSG_CACHEBLOCKSIZE;
        if (readSize + fileOffset > xmsgFileSize)
            readSize = xmsgFileSize - fileOffset;
        if (File_Read(xmsgFile, block->data, readSize) != readSize)
        {
            DDB_SetError(DDB_ERROR_READING_FILE);
            return 0;
        }

        block->free   = false;
        block->offset = fileOffset;
        block->usage  = usageToken;
        block->size   = readSize;
        block->tail   = fileOffset + readSize >= xmsgFileSize;
        return block;
    }
    return 0;
}

const uint8_t* DDB_GetXMessage(uint32_t offset)
{
    if (xmsgFile == 0)
    {
        DDB_SetError(DDB_ERROR_FILE_NOT_FOUND);
        return 0;
    }
    if (offset >= xmsgFileSize)
    {
        DebugPrintf("Offset %lu out of bounds (%lu bytes available)\n",
            (unsigned long)offset, (unsigned long)xmsgFileSize);
        DDB_SetError(DDB_ERROR_SEEKING_FILE);
        return 0;
    }
    if (xmsgScratch == 0)
    {
        DDB_SetError(DDB_ERROR_OUT_OF_MEMORY);
        DebugPrintf("WARNING: XMessage cache not initialized!");
        return 0;
    }

    usageToken++;
    
    if (usageToken == 0)
    {
        // Pathological case that will never happen IRL, but...
        for (int n = 0; n < cacheBlockCount; n++)
            cacheBlocks[n].free = true;
    }

    XMCacheBlock* cache = GetCachedBlock(offset);
    if (cache)
    {
        cache->usage = usageToken;

        uint32_t disp  = offset - cache->offset;
        uint8_t* msg   = cache->data + disp;
        uint32_t avail = cache->size - disp;
        if (disp + XMSG_MAXMSGSIZE <= cache->size || cache->tail)
            return CheckMessage(msg, offset, cache->size - disp);

        // Message doesn't fit in the cache block
        XMCacheBlock* next = GetCachedBlock(cache->offset + XMSG_CACHEBLOCKSIZE, cache);
        if (next != 0)
        {
            uint32_t total = next->size + (cache->size - disp);
            if (total > XMSG_MAXMSGSIZE)
                total = XMSG_MAXMSGSIZE;

            next->usage = usageToken;
            if (next == cache+1)
                return CheckMessage(msg, offset, total);

            MemCopy(xmsgScratch, msg, avail);
            MemCopy(xmsgScratch + avail, next->data, total - avail);
            return CheckMessage(xmsgScratch, offset, total);
        }
    }

    if (!File_Seek(xmsgFile, offset))
    {
        DDB_SetError(DDB_ERROR_SEEKING_FILE);
        return 0;
    }

    uint32_t readSize = XMSG_MAXMSGSIZE;
    if (readSize + offset > xmsgFileSize)
        readSize = xmsgFileSize - offset;
    if (File_Read(xmsgFile, xmsgScratch, readSize) != readSize)
    {
        DDB_SetError(DDB_ERROR_READING_FILE);
        return 0;
    }
    return CheckMessage(xmsgScratch, offset, readSize);
}

#endif
