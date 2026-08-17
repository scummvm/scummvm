#pragma once

#include <os_types.h>
#include <os_file.h>
#include <dim.h>

#include <stdint.h>
#include <stdbool.h>
#include <stdio.h>

struct FAT_Disk;

struct FAT_DirEntry
{
	char       filename[8];				// 0xE5 = deleted, 0x00 = unused
	char       extension[3];
	uint8_t    attributes;
	uint8_t    reserved[8];
	uint16_t   fat32ClusterHigh;
	uint16_t   modifyTime;
	uint16_t   modifyDate;
	uint16_t   startingCluster;
	uint32_t   fileSize;
};

struct FAT_FindResults
{
	FAT_Disk*		disk;				
	FAT_DirEntry 	entry;				
	uint32_t		sector;
	uint16_t   		offset;
	char			matchFilename[8];
	char			matchExtension[3];
};

enum FAT_Attributes
{
	FAT_READONLY   = 0x01,
	FAT_HIDDEN     = 0x02,
	FAT_SYSTEM     = 0x04,
	FAT_VOLUMEID   = 0x08,
	FAT_DIRECTORY  = 0x10,
	FAT_ARCHIVE    = 0x20,
	FAT_LONGNAME   = 0x0F
};

extern FAT_Disk* FAT_CreateDisk        (const char* filename, uint32_t size);
extern FAT_Disk* FAT_OpenDisk          (const char* filename);
extern bool		 FAT_FindFile          (FAT_Disk* disk, FAT_FindResults* result, const char* name);
extern bool		 FAT_FindFirstFile     (FAT_Disk* disk, FAT_FindResults* result, const char* pattern);
extern bool		 FAT_FindNextFile      (FAT_Disk* disk, FAT_FindResults* result);
extern uint32_t	 FAT_ReadFile          (FAT_Disk* dist, const char* path, uint8_t* buffer, uint32_t size);
extern uint32_t	 FAT_WriteFile         (FAT_Disk* dist, const char* path, const uint8_t* data, uint32_t size);
extern bool		 FAT_RemoveFile        (FAT_Disk* dist, const char* path);
extern uint32_t	 FAT_GetCWD 		   (FAT_Disk* disk, char* buffer, uint32_t bufferSize);
extern bool 	 FAT_ChangeDirectory   (FAT_Disk* disk, const char* name);
extern bool 	 FAT_MakeDirectory     (FAT_Disk* disk, const char* name);
extern bool 	 FAT_RemoveDirectory   (FAT_Disk* disk, const char* name);
extern void		 FAT_CloseDisk         (FAT_Disk* disk);
extern uint32_t  FAT_GetVolumeLabel    (FAT_Disk* disk, char* buffer, uint32_t bufferSize);
extern bool      FAT_SetVolumeLabel    (FAT_Disk* disk, const char* label);
extern uint64_t  FAT_GetFreeSpace      (FAT_Disk* disk);
extern uint32_t  FAT_GetFileName       (FAT_DirEntry* entry, char* buffer, uint32_t bufferSize);
extern void      FAT_DumpInfo          (FAT_Disk* disk);
