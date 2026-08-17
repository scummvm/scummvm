#pragma once

#include <os_types.h>
#include <os_file.h>

#define DISK_SIZE_880KB		901120
#define DISK_SIZE_1760KB	1802240
#define DISK_SIZE_720KB		737280
#define DISK_SIZE_1440KB	1474560
#define DISK_SIZE_2880KB	2949120

struct FAT_Disk;
struct CPC_Disk;
struct ADF_Disk;

enum DIM_Error
{
	DIMError_None,
	DIMError_CreatingFile,
	DIMError_DiskFull,
	DIMError_DirectoryFull,
	DIMError_DirectoryNotEmpty,
	DIMError_DirectoryNotFound,
	DIMError_NotADirectory,
	DIMError_FileNotFound,
	DIMError_InvalidFile,
	DIMError_InvalidDisk,
	DIMError_OutOfMemory,
	DIMError_ReadError,
	DIMError_WriteError,
	DIMError_FileExists,
	DIMError_CommandNotSupported,
    DIMError_FormatNotSupported,
};

enum DIM_DiskType
{
	DIM_FAT,
	DIM_CPC,
	DIM_ADF,
};

enum DIM_DiskCapabilities
{
	DIMCaps_Directories = 0x01,
	DIMCaps_All         = 0xFF,
};

struct DIM_Disk
{
	union
	{
		FAT_Disk*      fat;
		CPC_Disk*      cpc;
		ADF_Disk*      adf;
	};
	
	DIM_DiskType       type;
	int                capabilities;
};

extern DIM_Disk*			DIM_CreateDisk        (const char* filename, uint32_t size, const char* preset = NULL);
extern DIM_Disk*			DIM_OpenDisk          (const char* filename);
extern bool					DIM_FindFile          (DIM_Disk* disk, FindFileResults* result, const char* name);
extern bool					DIM_FindFirstFile     (DIM_Disk* disk, FindFileResults* result, const char* pattern);
extern bool					DIM_FindNextFile      (DIM_Disk* disk, FindFileResults* result);
extern uint32_t				DIM_ReadFile          (DIM_Disk* dist, const char* path, uint8_t* buffer, uint32_t size);
extern uint32_t				DIM_WriteFile         (DIM_Disk* dist, const char* path, const uint8_t* data, uint32_t size);
extern bool					DIM_RemoveFile        (DIM_Disk* dist, const char* name);
extern bool 				DIM_ChangeDirectory	  (DIM_Disk* disk, const char* name);
extern bool 			    DIM_MakeDirectory     (DIM_Disk* disk, const char* name);
extern bool 			    DIM_RemoveDirectory   (DIM_Disk* disk, const char* name);
extern uint32_t				DIM_GetCWD 		      (DIM_Disk* disk, char* buffer, uint32_t bufferSize);
extern uint64_t             DIM_GetFreeSpace      (DIM_Disk* disk);
extern void                 DIM_DumpInfo          (DIM_Disk* disk);
extern uint32_t             DIM_GetVolumeLabel    (DIM_Disk* disk, char* buffer, uint32_t bufferSize);
extern bool                 DIM_SetVolumeLabel    (DIM_Disk* disk, const char* label);
extern bool                 DIM_ReadBootBlock     (DIM_Disk* disk, void* buffer, uint32_t size);
extern bool                 DIM_WriteBootBlock    (DIM_Disk* disk, const void* buffer, uint32_t size);
extern void					DIM_CloseDisk         (DIM_Disk* disk);

extern DIM_Error			DIM_GetError          (void);
extern const char*          DIM_GetErrorString    ();
extern void                 DIM_SetError          (DIM_Error error);

extern void                 DIM_CopyPatternTo8D3  (const char* pattern, char* fileName, char* extension);
extern bool                 DIM_MatchWildcards    (const char* file, int fileSize, const char *pattern, int patternSize);
