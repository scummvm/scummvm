#pragma once

#include <os_types.h>

struct File;
struct ADF_Disk;

#define ADF_INVALID_BLOCK 0
#define ADF_MAX_DEPTH 32

#pragma pack(push,1)

struct ADF_RootBlock
{
	uint32_t    type;
	uint32_t    headerKey;
	uint32_t    highSeq;				// Highest index in the block list table
	uint32_t    hashTableEntries;
	uint32_t    firstData;				// Block number of the first data block of a file
	uint32_t    checksum;
	uint32_t    hashTable[72];
	int32_t     bitmapFlag;
	uint32_t    bitmapPage[25];
	uint32_t    bitmapExtensionBlocks;	// Hard disk only
	uint8_t     __padding0__[12];
	uint8_t     nameLength;
	char        name[30];
	uint8_t     __padding1__[9];
	uint32_t    aDays;
	uint32_t    aMinutes;
	uint32_t    aTicks;
	uint32_t    cDays;
	uint32_t    cMinutes;
	uint32_t    cTicks;
	uint32_t    nextHash;				// Unused
	uint32_t    parentDir;				// Unused
	uint32_t    extension;				// FFS
	uint32_t    blockSecondaryType;
};

struct ADF_HeaderBlock
{
	uint32_t    type;
	uint32_t    headerKey;
	uint32_t    highSeq;				// Unused
	uint32_t    dataSize;
	uint32_t    firstData;
	uint32_t    checksum;
	uint32_t    dataBlocks[72];
	uint8_t     __padding0__[8];
	uint32_t 	protectionFlags;
	uint32_t    fileSize;
	uint8_t     commentLength;
	char        comment[22];
	uint8_t     __padding1__[69];
	uint32_t    aDays;
	uint32_t    aMinutes;
	uint32_t    aTicks;
	uint8_t     nameLength;
	char        name[30];
	uint8_t     __padding2__[5];
	uint32_t    realEntry;
	uint32_t    nextLink;
	uint8_t     __padding3__[20];
	uint32_t    hashChain;
	uint32_t    parent;
	uint32_t    extension;
	uint32_t    secondaryType;
};

#pragma pack(pop)

struct ADF_Cwd
{
	char            name[32];
	uint32_t        block;
	uint32_t        hashTableEntries;
	uint32_t        hashTable[72];
};

enum ADF_FileSystem
{
	ADF_OFS,
	ADF_FFS
};


enum ADF_BlockConstants
{
    ADF_ROOT_TYPE     =  2,
    ADF_DIR_TYPE      =  2,
    ADF_DATA_TYPE     =  8,
    ADF_EXT_TYPE      = 16,
    ADF_ST_ROOT       =  1,
    ADF_ST_DIR        =  2,
    ADF_ST_FILE       = -3,
};

struct ADF_Disk
{
	File*			file;
	ADF_FileSystem  fs;
	uint32_t		size;
	uint32_t		numBlocks;
	uint32_t    	rootBlock;
	uint32_t        hashTableSize;

	ADF_RootBlock	root;
	uint8_t			boot[1024];
	ADF_HeaderBlock block;
	uint32_t*       bitmap;

	ADF_Cwd         cwd[ADF_MAX_DEPTH];
	uint8_t         cwdIndex;
};

struct ADF_FindResults
{
	uint32_t     block;
	int32_t      hashIndex;
	uint32_t     nextBlock;

	bool         directory;
	char         fileName[32];
	uint32_t     fileSize;
	uint32_t     days;
	uint32_t     minutes;
	uint32_t     ticks;

	char         pattern[32];
	uint8_t      patternLen;
};

ADF_Disk*  ADF_OpenDisk        (const char* filename);
bool       ADF_FindFile        (ADF_Disk* disk, ADF_FindResults* results, const char* name);
bool       ADF_FindFirstFile   (ADF_Disk* disk, ADF_FindResults* results, const char* pattern);
bool       ADF_FindNextFile    (ADF_Disk* disk, ADF_FindResults* results);
uint32_t   ADF_ReadFile        (ADF_Disk* dist, const char* path, uint8_t* buffer, uint32_t size);
uint32_t   ADF_GetCWD 		   (ADF_Disk* disk, char* buffer, uint32_t bufferSize);
bool 	   ADF_ChangeDirectory (ADF_Disk* disk, const char* name);
uint64_t   ADF_GetFreeSpace    (ADF_Disk* disk);
uint32_t   ADF_GetVolumeLabel  (ADF_Disk* disk, char* buffer, uint32_t bufferSize);
bool       ADF_SetVolumeLabel  (ADF_Disk* disk, const char* label);
void       ADF_DumpInfo        (ADF_Disk* disk);
void       ADF_CloseDisk       (ADF_Disk* disk);

// Write support
ADF_Disk*  ADF_CreateDisk      (const char* filename, uint32_t size);
bool       ADF_RemoveFile      (ADF_Disk* disk, const char* path);
bool       ADF_RemoveDirectory (ADF_Disk* disk, const char* path);
bool       ADF_MakeDirectory   (ADF_Disk* disk, const char* path);
bool       ADF_WriteFile       (ADF_Disk* disk, const char* path, const void* data, uint32_t dataSize);
bool       ADF_ReadBootBlock   (ADF_Disk* disk, void* buffer, uint32_t size);
bool       ADF_WriteBootBlock  (ADF_Disk* disk, const void* buffer, uint32_t size);
