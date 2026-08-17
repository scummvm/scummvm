#ifdef HAS_VIRTUALFILESYSTEM

#include <dim_fat.h>
#include <os_mem.h>

#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <ctype.h>
#include <time.h>

#define FAT_SECTOR_SIZE            512u
#define FAT_BOOT_SIGNATURE         0x29
#define FAT_DIRECTORY_ENTRY_SIZE   32
#define FAT_MAX_DIRECTORY_DEPTH    32
#define FAT_MAX_PATH               256

#pragma pack(push,1)

typedef struct _FAT_BootSector
{
	uint8_t    jmpBoot[3];
	uint8_t    OEMName[8];
	uint16_t   bytesPerSector;
	uint8_t    sectorsPerCluster;
	uint16_t   reservedSectorCount;
	uint8_t    numFATs;
	uint16_t   maxRootEntries;
	uint16_t   totalSectorsFAT16;
	uint8_t    mediaType;				// Ignore
	uint16_t   sectorsPerFAT;
	uint16_t   sectorsPerTrack;
	uint16_t   numHeads;
	uint32_t   hiddenSectors;			// Ignore
	uint32_t   totalSectorsFAT32;
	uint8_t    driveNumber;				// Ignore
	uint8_t    reserved;				// Ignore
	uint8_t    bootSignature;			// 0x29 to indicate next 3 fields are valid
	uint32_t   volumeID;
	uint8_t    volumeLabel[11];
	uint8_t    fileSystemType[8];		// Informative only, usually "FAT12   " or "FAT16   "
	uint8_t    codeReserved[448];
	uint16_t   signature;
} 
FAT_BootSector;

#pragma pack(pop)

typedef enum
{
	FAT_FAT12,
	FAT_FAT16,
	FAT_FAT32,
}
FAT_FATType;

static void FAT_GetDateTime(uint16_t* date, uint16_t* time)
{
	time_t t = ::time(NULL);
	struct tm* tm = ::localtime(&t);
	
	if (tm)
	{
		*date = (uint16_t)(((tm->tm_year - 80) << 9) | ((tm->tm_mon + 1) << 5) | tm->tm_mday);
		*time = (uint16_t)((tm->tm_hour << 11) | (tm->tm_min << 5) | (tm->tm_sec / 2));
	}
	else
	{
		*date = 0;
		*time = 0;
	}
}

#define FAT_UNUSED    0u
#define FAT_RESERVED  0xFFFFFFF0u
#define FAT_BAD       0xFFFFFFF7u
#define FAT_LAST      0xFFFFFFFFu

struct FAT_Disk
{
	File*       	file;
	uint32_t		size;
	uint32_t    	numSectors;
	uint32_t        numClusters;
	bool            readOnly;

	FAT_BootSector* bootSector;
	FAT_FATType		fatType;
	uint8_t*		fat;
	bool            fatModified;

	uint32_t    	rootFolderSector;
	uint32_t        firstDataSector;

	uint32_t        cwdFirstSector;
	FAT_DirEntry    cwd[FAT_MAX_DIRECTORY_DEPTH];
	int 			cwdDepth;
};

static FAT_Disk* currentDisk = NULL;
static uint32_t  currentSector = 0xFFFFFFFF;
static uint8_t   sectorData[FAT_SECTOR_SIZE];

uint32_t  FAT_ReadFileFromDE        (FAT_Disk* dist, FAT_DirEntry* entry, uint8_t* buffer, uint32_t size);
bool      FAT_RemoveFileFromDE      (FAT_Disk* disk, FAT_DirEntry* entry, uint32_t sector, uint32_t offset);
uint32_t  FAT_CountBadClusters      (FAT_Disk* disk);
uint32_t  FAT_CountFreeClusters     (FAT_Disk* disk);
uint32_t  FAT_FindFreeCluster       (FAT_Disk* disk);
bool      FAT_WriteSectorToDisk     (FAT_Disk* disk, uint32_t sector, const uint8_t* data);
bool      FAT_UpdateDirectoryEntry  (FAT_Disk* disk, FAT_DirEntry* entry, uint32_t sector, uint32_t offset);
bool      FAT_ReadSector            (FAT_Disk* disk, uint32_t sector);
static bool FAT_DecodeMSA           (File* file, uint32_t fileSize, uint8_t** outData, uint32_t* outSize);

static inline uint32_t FAT_Sector2Cluster (FAT_Disk* disk, uint32_t sector)
{
	return (sector - disk->firstDataSector) / disk->bootSector->sectorsPerCluster + 2;
}

static inline uint32_t FAT_Cluster2Sector (FAT_Disk* disk, uint32_t cluster)
{
	if (cluster < 2 || cluster >= FAT_RESERVED)
		return FAT_LAST;
	return disk->firstDataSector + (cluster - 2) * disk->bootSector->sectorsPerCluster;
}

static inline uint32_t FAT_FAT12Value(uint32_t cluster)
{
	return cluster >= 0xFF8 ? FAT_LAST :
		   cluster == 0xFF7 ? FAT_BAD :
		   cluster >= 0xFF0 ? FAT_RESERVED : cluster;
}

static inline uint32_t FAT_FAT16Value(uint32_t cluster)
{
	return cluster >= 0xFFF8 ? FAT_LAST :
		   cluster == 0xFFF7 ? FAT_BAD :
		   cluster >= 0xFFF0 ? FAT_RESERVED : cluster;
}

static inline uint32_t FAT_FAT32Value(uint32_t cluster)
{
	return cluster >= 0xFFFFFFF8 ? FAT_LAST :
		   cluster == 0xFFFFFFF7 ? FAT_BAD :
		   cluster >= 0xFFFFFFF0 ? FAT_RESERVED : cluster;
}

static uint32_t FAT_SetClusterFATValue(FAT_Disk* disk, uint32_t cluster, uint32_t value)
{
	int offset;
	
	switch (disk->fatType)
	{
		case FAT_FAT12:
			offset = (cluster >> 1) * 3;
			if (cluster & 1)
			{
				disk->fat[offset + 1] = (disk->fat[offset + 1] & 0x0F) | ((value & 0x0F) << 4);
				disk->fat[offset + 2] = (value >> 4) & 0xFF;
			}
			else
			{
				disk->fat[offset] = value & 0xFF;
				disk->fat[offset + 1] = (disk->fat[offset + 1] & 0xF0) | ((value >> 8) & 0x0F);
			}
			break;
		case FAT_FAT16:
			offset = cluster * 2;
			disk->fat[offset] = value & 0xFF;
			disk->fat[offset + 1] = (value >> 8) & 0xFF;
			break;
		case FAT_FAT32:
			offset = cluster * 4;
			disk->fat[offset] = value & 0xFF;
			disk->fat[offset + 1] = (value >> 8) & 0xFF;
			disk->fat[offset + 2] = (value >> 16) & 0xFF;
			disk->fat[offset + 3] = (value >> 24) & 0xFF;
			break;
	}

	disk->fatModified = true;
	return value;
}

static uint32_t FAT_GetClusterFATValue(FAT_Disk* disk, uint32_t cluster)
{
	int offset;
	
	switch (disk->fatType)
	{
		case FAT_FAT12:
			offset = (cluster >> 1) * 3;
			if (cluster & 1)
				return FAT_FAT12Value( (disk->fat[offset + 2] << 4) | (disk->fat[offset + 1] >> 4) );
			else
				return FAT_FAT12Value(disk->fat[offset] | ((disk->fat[offset + 1] << 8) & 0xF00) );
		case FAT_FAT16:
			offset = cluster * 2;
			return FAT_FAT16Value( disk->fat[offset] | (disk->fat[offset + 1] << 8) );
		case FAT_FAT32:
			offset = cluster * 4;
			return FAT_FAT32Value( disk->fat[offset] | (disk->fat[offset + 1] << 8) | (disk->fat[offset + 2] << 16) | (disk->fat[offset + 3] << 24) );
	}

	return 0;
}

static inline int FAT_NextSector (FAT_Disk* disk, uint32_t sector)
{
	if (sector < disk->firstDataSector)
	{
		if (sector == disk->firstDataSector - 1)
			return FAT_LAST;
		return sector + 1;
	}

	int sectorIndexInsideCluster = (sector - disk->firstDataSector) % disk->bootSector->sectorsPerCluster;
	if (sectorIndexInsideCluster < disk->bootSector->sectorsPerCluster - 1)
		return sector + 1;
	
	int cluster = FAT_Sector2Cluster(disk, sector);
	int nextCluster = FAT_GetClusterFATValue(disk, cluster);
	if (nextCluster < 2 || nextCluster >= FAT_RESERVED)
		return FAT_LAST;

	return FAT_Cluster2Sector(disk, (uint32_t)nextCluster);
}

bool FAT_MatchPattern (const FAT_DirEntry* entry, const char* pattern)
{
	size_t len = strlen(pattern);
	const char* dot = strrchr(pattern, '.');
	if (dot == NULL)
	{
		return DIM_MatchWildcards(entry->filename, 8, pattern, (int)len);
	}
	else
	{
		if (!DIM_MatchWildcards(entry->filename, 8, pattern, (int)(dot - pattern)))
			return false;
		return DIM_MatchWildcards(entry->extension, 3, dot + 1, (int)strlen(dot + 1));
	}
}

FAT_Disk* FAT_OpenDisk (const char* filename)
{
	bool ok = false;
	bool readOnly = false;

	FAT_Disk* disk = Allocate<FAT_Disk>("FAT Disk");
	if (!disk)
	{
		DIM_SetError(DIMError_OutOfMemory);
		return NULL;
	}

	disk->file = File_Open(filename, ReadWrite);
	if (!disk->file)
	{
		// Allow opening read-only images (common for archived floppy dumps).
		disk->file = File_Open(filename, ReadOnly);
		if (!disk->file)
		{
			DIM_SetError(DIMError_FileNotFound);
			Free(disk);
			return NULL;
		}
		readOnly = true;
	}

	DIM_SetError(DIMError_InvalidFile);

	disk->size = (uint32_t)File_GetSize(disk->file);
	disk->numSectors = disk->size / FAT_SECTOR_SIZE;
	disk->bootSector = (FAT_BootSector*)Allocate<uint8_t>("FAT Disk boot", FAT_SECTOR_SIZE);
	if (!disk->bootSector)
	{
		DIM_SetError(DIMError_OutOfMemory);
		File_Close(disk->file);
		Free(disk);
		return NULL;
	}

	FAT_BootSector* boot = disk->bootSector;
	if (File_Read(disk->file, disk->bootSector, FAT_SECTOR_SIZE) != FAT_SECTOR_SIZE)
	{
		DIM_SetError(DIMError_ReadError);
	}
	else
	{
		uint8_t* bootBytes = (uint8_t*)boot;
		if (bootBytes[0] == 0x0E && bootBytes[1] == 0x0F)
		{
			uint8_t* rawImage = NULL;
			uint32_t rawSize = 0;
			if (!FAT_DecodeMSA(disk->file, disk->size, &rawImage, &rawSize))
			{
				DIM_SetError(DIMError_InvalidFile);
			}
			else
			{
				File_Close(disk->file);
				disk->file = Memory_Open(rawImage, rawSize);
				if (!disk->file)
				{
					Free(rawImage);
					DIM_SetError(DIMError_OutOfMemory);
				}
				else
				{
					readOnly = true;
					disk->size = rawSize;
					disk->numSectors = rawSize / FAT_SECTOR_SIZE;
					if (File_Read(disk->file, disk->bootSector, FAT_SECTOR_SIZE) != FAT_SECTOR_SIZE)
						DIM_SetError(DIMError_ReadError);
				}
			}
		}

		if (boot->bytesPerSector != FAT_SECTOR_SIZE ||
			boot->sectorsPerCluster == 0 ||
			boot->sectorsPerCluster > 64 ||
			boot->sectorsPerFAT == 0 ||
			boot->sectorsPerFAT * FAT_SECTOR_SIZE >= disk->size ||
			boot->numFATs < 1 ||
			boot->numFATs > 4)
		{
			DIM_SetError(DIMError_InvalidFile);
		}
		else
		{
			uint32_t totalSectors = (boot->totalSectorsFAT16 != 0) ? boot->totalSectorsFAT16 : boot->totalSectorsFAT32;
			if (totalSectors == 0 || (uint64_t)totalSectors * FAT_SECTOR_SIZE > disk->size)
			{
				DIM_SetError(DIMError_InvalidFile);
			}
			else
			{
				uint32_t rootDirSectors = (boot->maxRootEntries * 32 + FAT_SECTOR_SIZE - 1) / FAT_SECTOR_SIZE;
				uint32_t fatSectors = boot->sectorsPerFAT * boot->numFATs;
				uint32_t dataSectors = totalSectors - (boot->reservedSectorCount + fatSectors + rootDirSectors);
				uint32_t countOfClusters = dataSectors / boot->sectorsPerCluster;

				disk->numSectors = totalSectors;
				ok = true;

				if (countOfClusters < 4085)
					disk->fatType = FAT_FAT12;
				else if (countOfClusters < 65525)
					disk->fatType = FAT_FAT16;
				else
					disk->fatType = FAT_FAT32;
			}
		}
	}

	if (ok)
	{
		disk->readOnly = readOnly;
		uint64_t fatSize = boot->sectorsPerFAT * FAT_SECTOR_SIZE * boot->numFATs;
		disk->fat = Allocate<uint8_t>("FAT Disk fat", fatSize);
		if (disk->fat == NULL)
		{
			DIM_SetError(DIMError_OutOfMemory);
			ok = false;
		}
		else
		{
			disk->rootFolderSector = 1 + boot->sectorsPerFAT * boot->numFATs;
			disk->firstDataSector  = disk->rootFolderSector + boot->maxRootEntries * FAT_DIRECTORY_ENTRY_SIZE / FAT_SECTOR_SIZE;
			disk->numClusters      = (disk->numSectors - disk->firstDataSector) / boot->sectorsPerCluster;
			disk->cwdFirstSector   = disk->rootFolderSector;
			disk->cwdDepth         = 0;

			if (File_Read(disk->file, disk->fat, fatSize) != fatSize)
			{
				DIM_SetError(DIMError_ReadError);
				ok = false;
			}

			// TODO: Check copies for consistency
		}
	}

	if (!ok)
	{
		File_Close(disk->file);
		if (disk->fat)
			Free(disk->fat);
		Free(disk->bootSector);
		Free(disk);
		return NULL;
	}

	return disk;
}

FAT_Disk* FAT_CreateDisk (const char* filename, uint32_t size)
{
	FAT_Disk* disk = Allocate<FAT_Disk>("FAT Disk");
	if (!disk)
	{
		DIM_SetError(DIMError_OutOfMemory);
		return NULL;
	}
    if (size == 0)
        size = DISK_SIZE_1440KB;

	disk->file = File_Create(filename);
	if (!disk->file)
	{
		DIM_SetError(DIMError_CreatingFile);
		Free(disk);
		return NULL;
	}

	disk->size = size;
	disk->numSectors = size / FAT_SECTOR_SIZE;
	disk->readOnly = false;
	disk->bootSector = (FAT_BootSector*)Allocate<uint8_t>("FAT Disk boot", FAT_SECTOR_SIZE);
	if (!disk->bootSector)
	{
		DIM_SetError(DIMError_OutOfMemory);
		File_Close(disk->file);
		Free(disk);
		return NULL;
	}
	memset(disk->bootSector, 0, FAT_SECTOR_SIZE);

	// Determine FAT type and cluster size based on disk size
	// FAT12: up to 4085 clusters, FAT16: 4086-65525 clusters
	uint8_t sectorsPerCluster;
	uint32_t maxRootEntries;
	FAT_FATType fatType;
	bool isFloppy = (disk->numSectors <= 5760);
	
	// Standard floppy uses 112 or 224 root entries, larger disks use 512
	if (disk->numSectors <= 720)
		maxRootEntries = 112;
	else if (disk->numSectors <= 5760)
		maxRootEntries = 224;
	else
		maxRootEntries = 512;
	
	// Calculate initial sectors per cluster based on disk size
	if (disk->numSectors <= 5760)          // <= 2.88MB: floppy, use 2 sectors/cluster
		sectorsPerCluster = 2;
	else if (disk->numSectors <= 32680)    // <= 16MB: 4 sectors/cluster  
		sectorsPerCluster = 4;
	else if (disk->numSectors <= 262144)   // <= 128MB: 8 sectors/cluster
		sectorsPerCluster = 8;
	else if (disk->numSectors <= 524288)   // <= 256MB: 16 sectors/cluster
		sectorsPerCluster = 16;
	else if (disk->numSectors <= 1048576)  // <= 512MB: 32 sectors/cluster
		sectorsPerCluster = 32;
	else                                    // > 512MB: 64 sectors/cluster
		sectorsPerCluster = 64;
	
	uint32_t rootDirSectors = (maxRootEntries * FAT_DIRECTORY_ENTRY_SIZE + FAT_SECTOR_SIZE - 1) / FAT_SECTOR_SIZE;
	uint32_t sectorsPerFAT = 0;
	uint32_t actualClusters = 0;
	
	// Iterate to find appropriate cluster size for FAT12 or FAT16
	for (;;)
	{
		uint32_t estimatedClusters = disk->numSectors / sectorsPerCluster;
		
		// Try FAT12 first
		uint32_t bytesPerFat = (estimatedClusters * 3 + 1) / 2;
		sectorsPerFAT = (bytesPerFat + FAT_SECTOR_SIZE - 1) / FAT_SECTOR_SIZE;
		uint32_t overhead = 1 + (2 * sectorsPerFAT) + rootDirSectors;
		uint32_t dataSectors = disk->numSectors - overhead;
		actualClusters = dataSectors / sectorsPerCluster;
		
		if (actualClusters <= 4085)
		{
			fatType = FAT_FAT12;
			break;
		}
		
		// Try FAT16
		bytesPerFat = estimatedClusters * 2;
		sectorsPerFAT = (bytesPerFat + FAT_SECTOR_SIZE - 1) / FAT_SECTOR_SIZE;
		overhead = 1 + (2 * sectorsPerFAT) + rootDirSectors;
		dataSectors = disk->numSectors - overhead;
		actualClusters = dataSectors / sectorsPerCluster;
		
		if (actualClusters >= 4086 && actualClusters < 65525)
		{
			fatType = FAT_FAT16;
			break;
		}
		
		// Need more sectors per cluster
		if (sectorsPerCluster >= 128)
		{
			DIM_SetError(DIMError_InvalidFile);
			File_Close(disk->file);
			Free(disk->bootSector);
			Free(disk);
			return NULL;
		}
		sectorsPerCluster *= 2;
	}

	FAT_BootSector* boot = disk->bootSector;
	
	// Jump instruction (x86 JMP SHORT + NOP)
	boot->jmpBoot[0] = 0xEB;
	boot->jmpBoot[1] = 0x3C;
	boot->jmpBoot[2] = 0x90;
	
	// OEM Name
	memcpy(boot->OEMName, "MSDOS5.0", 8);
	
	// BPB (BIOS Parameter Block)
	boot->bytesPerSector = FAT_SECTOR_SIZE;
	boot->sectorsPerCluster = sectorsPerCluster;
	boot->reservedSectorCount = 1;
	boot->numFATs = 2;
	boot->maxRootEntries = (uint16_t)maxRootEntries;
	boot->totalSectorsFAT16 = (disk->numSectors < 65536) ? (uint16_t)disk->numSectors : 0;
	
	// Media descriptor: 0xF8 for hard disk, 0xF0 for removable, 0xFD for 360K
	if (disk->numSectors == 720)
		boot->mediaType = 0xFD;  // 360K 5.25"
	else if (isFloppy)
		boot->mediaType = 0xF0;  // Other floppies
	else
		boot->mediaType = 0xF8;  // Hard disk
	
	boot->sectorsPerFAT = (uint16_t)sectorsPerFAT;
	
	// Disk geometry - sensible defaults based on disk type
	if (disk->numSectors == 720)       // 360K 5.25"
	{
		boot->sectorsPerTrack = 9;
		boot->numHeads = 2;
	}
	else if (disk->numSectors == 1440) // 720K 3.5"
	{
		boot->sectorsPerTrack = 9;
		boot->numHeads = 2;
	}
	else if (disk->numSectors == 2880) // 1.44M 3.5"
	{
		boot->sectorsPerTrack = 18;
		boot->numHeads = 2;
	}
	else if (disk->numSectors == 5760) // 2.88M 3.5"
	{
		boot->sectorsPerTrack = 36;
		boot->numHeads = 2;
	}
	else if (isFloppy) // Other floppy sizes
	{
		boot->sectorsPerTrack = 18;
		boot->numHeads = 2;
	}
	else // Hard disk - use reasonable geometry
	{
		boot->sectorsPerTrack = 63;
		boot->numHeads = 255;
	}
	
	boot->hiddenSectors = 0;
	boot->totalSectorsFAT32 = (disk->numSectors >= 65536) ? disk->numSectors : 0;
	
	// Extended boot record fields
	boot->driveNumber = isFloppy ? 0x00 : 0x80;  // 0x00 for floppy, 0x80 for hard disk
	boot->reserved = 0;
	boot->bootSignature = 0x29;
	boot->volumeID = 0x12345678;
	memset(boot->volumeLabel, ' ', 11);
	memcpy(boot->fileSystemType, (fatType == FAT_FAT12) ? "FAT12   " : "FAT16   ", 8);
	
	// Boot sector signature
	boot->signature = 0xAA55;
	
	disk->fatType = fatType;
	disk->rootFolderSector = 1 + boot->sectorsPerFAT * boot->numFATs;
	disk->firstDataSector  = disk->rootFolderSector + boot->maxRootEntries * FAT_DIRECTORY_ENTRY_SIZE / FAT_SECTOR_SIZE;
	disk->numClusters	   = (disk->numSectors - disk->firstDataSector) / boot->sectorsPerCluster;
	disk->cwdFirstSector   = disk->rootFolderSector;
	disk->cwdDepth         = 0;
	
	disk->fat = Allocate<uint8_t>("FAT Disk fat", boot->sectorsPerFAT * FAT_SECTOR_SIZE * boot->numFATs);
	if (disk->fat == NULL)
	{
		DIM_SetError(DIMError_OutOfMemory);
		File_Close(disk->file);
		Free(disk->bootSector);
		Free(disk);
		return NULL;
	}
	
	// Initialize FAT with media descriptor
	memset(disk->fat, 0, boot->sectorsPerFAT * FAT_SECTOR_SIZE * boot->numFATs);
	disk->fat[0] = boot->mediaType;
	disk->fat[1] = 0xFF;
	disk->fat[2] = 0xFF;
	disk->fatModified = false;

	uint32_t fatSize = boot->sectorsPerFAT * FAT_SECTOR_SIZE * boot->numFATs;
	bool ok = File_Write(disk->file, disk->bootSector, FAT_SECTOR_SIZE) == FAT_SECTOR_SIZE;
	ok = ok && File_Write(disk->file, disk->fat, fatSize) == fatSize;

	currentDisk = NULL;
	memset(sectorData, 0, FAT_SECTOR_SIZE);
	for (uint32_t n = disk->rootFolderSector; n < disk->numSectors && ok ; n++)
		ok = ok && File_Write(disk->file, sectorData, FAT_SECTOR_SIZE) == FAT_SECTOR_SIZE;

	if (!ok)
	{
		DIM_SetError(DIMError_CreatingFile);
		File_Close(disk->file);
		Free(disk->fat);
		Free(disk->bootSector);
		Free(disk);
		return NULL;
	}

	return disk;
}

uint32_t FAT_GetVolumeLabel (FAT_Disk* disk, char* buffer, uint32_t bufferSize)
{
	if (bufferSize == 0)
		return 0;
	
	// First, search for volume label in root directory (set by MS-DOS LABEL command)
	uint32_t sector = disk->rootFolderSector;
	uint32_t maxRootSectors = disk->firstDataSector - disk->rootFolderSector;
	
	for (uint32_t s = 0; s < maxRootSectors; s++, sector++)
	{
		if (!FAT_ReadSector(disk, sector))
			break;
		
		for (uint32_t offset = 0; offset < FAT_SECTOR_SIZE; offset += FAT_DIRECTORY_ENTRY_SIZE)
		{
			FAT_DirEntry* entry = (FAT_DirEntry*)(sectorData + offset);
			
			// End of directory
			if (entry->filename[0] == 0)
				goto check_boot_sector;
			
			// Deleted entry
			if ((uint8_t)entry->filename[0] == 0xE5)
				continue;
			
			// Check for volume label attribute
			if ((entry->attributes & FAT_VOLUMEID) && !(entry->attributes & FAT_DIRECTORY))
			{
				// Found volume label in directory
				// Volume labels use all 11 characters (filename+extension) as one field
				char* out = buffer;
				char* end = buffer + bufferSize - 1;
				
				// Copy all 11 characters, trimming trailing spaces
				int lastNonSpace = -1;
				for (int i = 0; i < 11 && out < end; i++)
				{
					char c = (i < 8) ? entry->filename[i] : entry->extension[i - 8];
					*out++ = c;
					if (c != ' ')
						lastNonSpace = (int)(out - buffer) - 1;
				}
				
				// Trim trailing spaces
				if (lastNonSpace >= 0)
					buffer[lastNonSpace + 1] = 0;
				else
					buffer[0] = 0;
				
				return (uint32_t)(lastNonSpace + 1);
			}
		}
	}
	
check_boot_sector:
	// Fall back to boot sector volume label
	if (disk->bootSector->bootSignature != FAT_BOOT_SIGNATURE)
		return 0;

	char* out = buffer;
	char* end = buffer + bufferSize - 1;
	char* ptr = (char *)disk->bootSector->volumeLabel;
	while (out < end && *ptr && *ptr != ' ')
		*out++ = *ptr++;
	*out = 0;
	return (uint32_t)(out - buffer);
}

bool FAT_SetVolumeLabel (FAT_Disk* disk, const char* label)
{
	FAT_BootSector* boot = disk->bootSector;
	uint32_t labelLen = 0;
	
	if (label != NULL)
	{
		while (labelLen < 11 && label[labelLen] != 0)
			labelLen++;
	}
	
	// First, find and update/delete existing volume label in root directory
	uint32_t sector = disk->rootFolderSector;
	uint32_t maxRootSectors = disk->firstDataSector - disk->rootFolderSector;
	uint32_t existingLabelSector = 0;
	uint32_t existingLabelOffset = 0;
	bool foundExisting = false;
	
	for (uint32_t s = 0; s < maxRootSectors && !foundExisting; s++, sector++)
	{
		if (!FAT_ReadSector(disk, sector))
			continue;
		
		for (uint32_t offset = 0; offset < FAT_SECTOR_SIZE; offset += FAT_DIRECTORY_ENTRY_SIZE)
		{
			FAT_DirEntry* entry = (FAT_DirEntry*)(sectorData + offset);
			
			if (entry->filename[0] == 0)
				break;
			
			if ((uint8_t)entry->filename[0] == 0xE5)
				continue;
			
			if ((entry->attributes & FAT_VOLUMEID) && !(entry->attributes & FAT_DIRECTORY))
			{
				foundExisting = true;
				existingLabelSector = sector;
				existingLabelOffset = offset;
				break;
			}
		}
	}
	
	if (labelLen == 0)
	{
		// Delete existing label
		if (foundExisting)
		{
			if (!FAT_ReadSector(disk, existingLabelSector))
				return false;
			FAT_DirEntry* entry = (FAT_DirEntry*)(sectorData + existingLabelOffset);
			entry->filename[0] = (char)0xE5;  // Mark as deleted
			if (!FAT_WriteSectorToDisk(disk, existingLabelSector, sectorData))
			{
				DIM_SetError(DIMError_WriteError);
				return false;
			}
		}
	}
	else
	{
		// Create/update volume label entry in root directory
		FAT_DirEntry labelEntry;
		memset(&labelEntry, 0, sizeof(FAT_DirEntry));
		
		// Fill in the 11-character label (filename + extension as one field)
		// Volume labels use all 11 characters continuously, not split at 8.3
		memset(labelEntry.filename, ' ', 8);
		memset(labelEntry.extension, ' ', 3);
		
		for (uint32_t i = 0; i < labelLen && i < 11; i++)
		{
			char c = (char)toupper(label[i]);
			if (i < 8)
				labelEntry.filename[i] = c;
			else
				labelEntry.extension[i - 8] = c;
		}
		
		labelEntry.attributes = FAT_VOLUMEID;
		
		if (foundExisting)
		{
			// Update existing entry
			if (!FAT_UpdateDirectoryEntry(disk, &labelEntry, existingLabelSector, existingLabelOffset))
			{
				DIM_SetError(DIMError_WriteError);
				return false;
			}
		}
		else
		{
			// Find free entry in root directory
			sector = disk->rootFolderSector;
			bool foundFree = false;
			
			for (uint32_t s = 0; s < maxRootSectors; s++, sector++)
			{
				if (!FAT_ReadSector(disk, sector))
					continue;
				
				for (uint32_t offset = 0; offset < FAT_SECTOR_SIZE; offset += FAT_DIRECTORY_ENTRY_SIZE)
				{
					FAT_DirEntry* entry = (FAT_DirEntry*)(sectorData + offset);
					
					if (entry->filename[0] == 0 || (uint8_t)entry->filename[0] == 0xE5)
					{
						if (!FAT_UpdateDirectoryEntry(disk, &labelEntry, sector, offset))
						{
							DIM_SetError(DIMError_WriteError);
							return false;
						}
						foundFree = true;
						break;
					}
				}
				if (foundFree)
					break;
			}
			
			if (!foundFree)
			{
				DIM_SetError(DIMError_DirectoryFull);
				return false;
			}
		}
	}
	
	// Also update boot sector volume label for compatibility
	if (boot->bootSignature != FAT_BOOT_SIGNATURE)
	{
		boot->bootSignature = FAT_BOOT_SIGNATURE;
		boot->volumeID = 0x12345678;
	}
	
	memset(boot->volumeLabel, ' ', 11);
	for (uint32_t i = 0; i < 11 && i < labelLen; i++)
		boot->volumeLabel[i] = (uint8_t)toupper(label[i]);
	
	if (!FAT_WriteSectorToDisk(disk, 0, (uint8_t*)boot))
	{
		DIM_SetError(DIMError_WriteError);
		return false;
	}
	
	return true;
}

void FAT_DumpInfo (FAT_Disk* disk)
{
	FAT_BootSector* boot = disk->bootSector;

	printf("Disk size:           %7dK\n", (disk->size + 1023) / 1024);
	printf("Disk format:         %8s\n", disk->fatType == FAT_FAT12 ? "FAT12" : disk->fatType == FAT_FAT16 ? "FAT16" : "FAT32");
	
	char volumeLabel[12];
	if (FAT_GetVolumeLabel(disk, volumeLabel, sizeof(volumeLabel)) > 0)
		printf("Volume label:     %11s\n", volumeLabel);

	printf("-----------------------------\n");
	printf("Bytes per sector:    %8d\n", boot->bytesPerSector);
	printf("Bytes per cluster:   %8d\n", boot->sectorsPerCluster * boot->bytesPerSector);
	printf("FAT copies:          %8d\n", boot->numFATs);
	printf("Sectors per FAT:     %8d\n", boot->sectorsPerFAT);
	printf("Max root entries:    %8d\n", boot->maxRootEntries);
	printf("Total sectors:       %8d\n", disk->numSectors);
	printf("Total clusters:      %8d\n", disk->numClusters);
	printf("First data sector:   %8d\n", disk->firstDataSector);
	printf("Free clusters:       %8d\n", FAT_CountFreeClusters(disk));
	printf("Bad clusters:        %8d\n", FAT_CountBadClusters(disk));
}

bool FAT_ReadSectorToBuffer (FAT_Disk* disk, uint32_t sector, uint8_t* data)
{
	if (sector < 0 || sector >= disk->numSectors)
		return false;
	File_Seek(disk->file, sector * FAT_SECTOR_SIZE);
	if (File_Read(disk->file, data, FAT_SECTOR_SIZE) == FAT_SECTOR_SIZE)
		return true;
	DIM_SetError(DIMError_ReadError);
	return false;
}

bool FAT_ReadSector (FAT_Disk* disk, uint32_t sector)
{
	if (currentDisk == disk && currentSector == sector)
		return true;

	if (sector < 0 || sector >= disk->numSectors)
		return false;
	File_Seek(disk->file, sector * FAT_SECTOR_SIZE);
	if (File_Read(disk->file, sectorData, FAT_SECTOR_SIZE) == FAT_SECTOR_SIZE)
	{
		currentDisk = disk;
		currentSector = sector;
		return true;
	}

	DIM_SetError(DIMError_ReadError);
	return false;
}

bool FAT_WriteSectorToDisk (FAT_Disk* disk, uint32_t sector, const uint8_t* data)
{
	if (disk->readOnly)
	{
		DIM_SetError(DIMError_WriteError);
		return false;
	}

	if (sector < 0 || sector >= disk->numSectors)
		return false;

	File_Seek(disk->file, sector * FAT_SECTOR_SIZE);
	if (File_Write(disk->file, data, FAT_SECTOR_SIZE) == FAT_SECTOR_SIZE)
		return true;

	DIM_SetError(DIMError_WriteError);
	return false;
}

bool FAT_UpdateDirectoryEntry (FAT_Disk* disk, FAT_DirEntry* entry, uint32_t sector, uint32_t offset)
{
	if (sector < 0 || sector >= disk->numSectors)
		return false;
	if (!FAT_ReadSector(disk, sector))
		return false;

	memcpy(sectorData + offset, entry, FAT_DIRECTORY_ENTRY_SIZE);
	return FAT_WriteSectorToDisk(disk, sector, sectorData);
}

bool FAT_RemoveFileFromDE (FAT_Disk* disk, FAT_DirEntry* entry, uint32_t sector, uint32_t offset)
{
	if (sector < 0 || sector >= disk->numSectors)
		return false;

	uint32_t cluster = entry->startingCluster;
	while (cluster != FAT_LAST)
	{
		uint32_t nextCluster = FAT_GetClusterFATValue(disk, cluster);
		FAT_SetClusterFATValue(disk, cluster, 0);
		cluster = nextCluster;
	}

	entry->filename[0] = (int8_t)(uint8_t)0xE5;
	return FAT_UpdateDirectoryEntry(disk, entry, sector, offset);
}

bool FAT_RemoveFile (FAT_Disk* disk, const char* path)
{
	FAT_FindResults result;
	if (!FAT_FindFile(disk, &result, path))
		return false;

	FAT_DirEntry* entry = &result.entry;
	uint32_t sector = result.sector;
	uint32_t offset = result.offset;

	if ((entry->attributes & FAT_DIRECTORY) ||
		(uint8_t)entry->filename[0] == 0xE5 ||
		(uint8_t)entry->filename[0] == 0)
	{
		DIM_SetError(DIMError_InvalidFile);
		return false;
	}

	return FAT_RemoveFileFromDE(disk, entry, sector, offset);
}

bool FAT_RemoveDirectory (FAT_Disk* disk, const char* path)
{
	FAT_FindResults result;

	if (!FAT_ChangeDirectory(disk, path))
		return false;
	if (FAT_FindFirstFile(disk, &result, "*"))
	{
		DIM_SetError(DIMError_DirectoryNotEmpty);
		return false;
	}
	FAT_ChangeDirectory(disk, "..");

	if (!FAT_FindFile(disk, &result, path))
		return false;
	FAT_DirEntry* entry = &result.entry;
	uint32_t sector = result.sector;
	uint32_t offset = result.offset;
	FAT_RemoveFileFromDE(disk, entry, sector, offset);
	return true;
}

uint32_t FAT_FindFreeCluster (FAT_Disk* disk)
{
	for (uint32_t i = 2; i < disk->numClusters; i++)
	{
		if (FAT_GetClusterFATValue(disk, i) == 0)
			return i;
	}
	return FAT_LAST;
}

uint32_t FAT_CountFreeClusters(FAT_Disk* disk)
{
	uint32_t FreeClusters = 0;
	for (uint32_t i = 2; i < disk->numClusters; i++)
	{
		if (FAT_GetClusterFATValue(disk, i) == 0)
			FreeClusters++;
	}
	return FreeClusters;
}

uint32_t FAT_CountBadClusters(FAT_Disk* disk)
{
	uint32_t FreeClusters = 0;
	for (uint32_t i = 2; i < disk->numClusters; i++)
	{
		if (FAT_GetClusterFATValue(disk, i) == FAT_BAD)
			FreeClusters++;
	}
	return FreeClusters;
}

uint64_t FAT_GetFreeSpace(FAT_Disk* disk)
{
	return FAT_CountFreeClusters(disk) * disk->bootSector->sectorsPerCluster * FAT_SECTOR_SIZE;
}

bool FAT_FindFile (FAT_Disk* disk, FAT_FindResults* result, const char* pattern)
{
	const char* ptr = pattern;
	while (*ptr != 0)
	{
		if (*ptr == '?')
			return false;
		if (*ptr == '*')
			return false;
		ptr++;
	}
	return FAT_FindFirstFile(disk, result, pattern);
}

bool FAT_FindFirstFile (FAT_Disk* disk, FAT_FindResults* result, const char* pattern)
{
	result->disk = disk;
	result->entry.filename[0] = 0;
	result->sector = disk->cwdFirstSector;
	result->offset = (uint16_t) -FAT_DIRECTORY_ENTRY_SIZE;

	DIM_CopyPatternTo8D3(pattern, result->matchFilename, result->matchExtension);

	return FAT_FindNextFile(disk, result);
}

bool FAT_FindNextFile (FAT_Disk* disk, FAT_FindResults* result)
{
	if ((int16_t)result->offset > FAT_SECTOR_SIZE - FAT_DIRECTORY_ENTRY_SIZE)
	{
		if (!FAT_ReadSector(disk, result->sector))
		{
			DIM_SetError(DIMError_ReadError);
			return false;
		}
	}

	for (;;)
	{
		result->offset += FAT_DIRECTORY_ENTRY_SIZE;
		if (result->offset >= FAT_SECTOR_SIZE)
		{
			result->sector = FAT_NextSector(disk, result->sector);
			result->offset = 0;
			if (result->sector == FAT_LAST)
			{
				DIM_SetError(DIMError_FileNotFound);
				return false;
			}
		}
		if (currentSector != result->sector)
		{
			if (!FAT_ReadSector(disk, result->sector))
			{
				DIM_SetError(DIMError_ReadError);
				return false;
			}
		}

		FAT_DirEntry* entry = (FAT_DirEntry*) (sectorData + result->offset);
		if (entry->filename[0] == 0)
			continue;
		if ((uint8_t)entry->filename[0] == 0xE5)
			continue;
		if (entry->attributes & FAT_LONGNAME)
			continue;
		if (entry->filename[0] == '.')
			continue;
		if (entry->filename[0] == ' ')
			continue;
		if (!DIM_MatchWildcards(entry->filename, 8, result->matchFilename, 8))
			continue;
		if (!DIM_MatchWildcards(entry->extension, 3, result->matchExtension, 3))
			continue;

		result->entry = *entry;
		return true;
	}
}

uint32_t FAT_ReadFileFromDE (FAT_Disk* disk, FAT_DirEntry* entry, uint8_t* buffer, uint32_t size)
{
	uint32_t total = 0;
	uint32_t remaining = entry->fileSize;
	if (remaining > size)
		remaining = size;
	uint32_t cluster = entry->startingCluster;
	if (cluster >= FAT_RESERVED || cluster < 2)
	{
		DIM_SetError(DIMError_ReadError);
		return 0;
	}
	while (cluster != FAT_LAST)
	{
		int sector = FAT_Cluster2Sector(disk, cluster);
		for (int n = 0; n < disk->bootSector->sectorsPerCluster; n++, sector++)
		{
			if (remaining > FAT_SECTOR_SIZE)
			{
				if (!FAT_ReadSectorToBuffer(disk, sector, buffer))
					return total;
				buffer += FAT_SECTOR_SIZE;
				remaining -= FAT_SECTOR_SIZE;
				total += FAT_SECTOR_SIZE;
			}
			else
			{
				if (!FAT_ReadSector(disk, sector))
					return total;
				memcpy(buffer, sectorData, remaining);
				total += remaining;
				return total;
			}
		}
		cluster = FAT_GetClusterFATValue(disk, cluster);
		if (cluster >= FAT_RESERVED || cluster < 2)
		{
			DIM_SetError(DIMError_ReadError);
			return remaining;
		}
	}
	return total;
}

uint32_t FAT_ReadFile (FAT_Disk* disk, const char* path, uint8_t* buffer, uint32_t size)
{
	FAT_FindResults result;
	if (!FAT_FindFile(disk, &result, path))
		return 0;

	return FAT_ReadFileFromDE(disk, &result.entry, buffer, size);
}

void FAT_CloseDisk (FAT_Disk* disk)
{
	if (disk)
	{
		if (disk->fatModified && !disk->readOnly)
		{
			File_Seek(disk->file, FAT_SECTOR_SIZE);
			for (int n = 0; n < disk->bootSector->numFATs; n++)
				File_Write(disk->file, disk->fat, disk->bootSector->sectorsPerFAT * FAT_SECTOR_SIZE);
		}
		File_Close(disk->file);

		Free(disk->bootSector);
		Free(disk->fat);
		Free(disk);
	}
}

uint32_t FAT_GetFileName (FAT_DirEntry* entry, char* buffer, uint32_t bufferSize)
{
	char* ptr = buffer;
	char* end = buffer + bufferSize - 1;
	for (int i = 0; i < 8 && entry->filename[i] != ' ' && ptr < end; i++)
		*ptr++ = entry->filename[i];
	if (entry->extension[0] != ' ' && ptr < end)
	{
		*ptr++ = '.';
		for (int i = 0; i < 3 && entry->extension[i] != ' ' && ptr < end; i++)
			*ptr++ = entry->extension[i];
	}
	*ptr = 0;
	return (uint32_t)(ptr - buffer);
}

bool FAT_ChangeDirectory (FAT_Disk* disk, const char* name)
{
	if (name[0] == '\\')
	{
		disk->cwdDepth = 0;
		disk->cwdFirstSector = disk->rootFolderSector;
		name++;
	}

	const char* separator = strchr(name, '\\');
	if (separator)
	{
		char buffer[13];
		size_t len = separator - name;
		if (len > 12) len = 12;
		memcpy(buffer, name, len);
		buffer[len] = 0;
		if (!FAT_ChangeDirectory(disk, buffer))
			return false;
		return FAT_ChangeDirectory(disk, separator + 1);
	}

	if (name[0] == 0 || strcmp(name, ".") == 0)
		return true;

	if (strcmp(name, "..") == 0)
	{
		if (disk->cwdDepth > 0)
		{
			disk->cwdDepth--;
			if (disk->cwdDepth == 0)
				disk->cwdFirstSector = disk->rootFolderSector;
			else
				disk->cwdFirstSector = FAT_Cluster2Sector(disk, disk->cwd[disk->cwdDepth-1].startingCluster);
			return true;
		}
		return false;
	}

	FAT_FindResults results;
	if (!FAT_FindFile(disk, &results, name))
	{
		DIM_SetError(DIMError_DirectoryNotFound);
		return false;
	}

	if (results.entry.attributes & FAT_DIRECTORY)
	{
		if (disk->cwdDepth < FAT_MAX_DIRECTORY_DEPTH)
		{
			disk->cwd[disk->cwdDepth] = results.entry;
			disk->cwdDepth++;
			disk->cwdFirstSector = FAT_Cluster2Sector(disk, results.entry.startingCluster);
			return true;
		}
	}
	DIM_SetError(DIMError_NotADirectory);
	return false;
}

uint32_t FAT_GetCWD (FAT_Disk* disk, char* buffer, uint32_t bufferSize)
{
	char* ptr = buffer;
	char* end = buffer + bufferSize - 1;

	if (disk->cwdDepth == 0 && ptr < end)
		*ptr++ = '\\';
	for (int i = 0; i < disk->cwdDepth && ptr < end; i++)
	{
		if (ptr < end)
			*ptr++ = '\\';
		ptr += FAT_GetFileName(&disk->cwd[i], ptr, (uint32_t)(end - ptr));
	}
	*ptr = 0;
	return (uint32_t)(ptr - buffer);
}

bool FAT_FindFreeDirectoryEntry(FAT_Disk* disk, FAT_FindResults* result, bool deletedOk, bool makeDirectoryBigger)
{
	result->disk = disk;
	result->entry.filename[0] = 0;
	result->sector = disk->cwdFirstSector;
	result->offset = 0;

	if (result->sector == FAT_LAST)
	{
		DIM_SetError(DIMError_InvalidFile);
		return false;
	}

	if (!FAT_ReadSector(disk, result->sector))
	{
		DIM_SetError(DIMError_ReadError);
		return false;
	}

	for (;;)
	{
		FAT_DirEntry* entry = (FAT_DirEntry*)(sectorData + result->offset);
		if (entry->filename[0] == 0 || (deletedOk && (uint8_t)entry->filename[0] == 0xE5))
		{
			result->entry = *entry;
			return true;
		}

		result->offset += FAT_DIRECTORY_ENTRY_SIZE;
		if (result->offset >= FAT_SECTOR_SIZE)
		{
			uint32_t nextSector = FAT_NextSector(disk, result->sector);
			if (nextSector == FAT_LAST)
				break;

			result->sector = nextSector;
			result->offset = 0;
			if (!FAT_ReadSector(disk, result->sector))
			{
				DIM_SetError(DIMError_ReadError);
				return false;
			}
		}
	}

	if (!makeDirectoryBigger)
	{
		DIM_SetError(DIMError_DirectoryFull);
		return false;
	}

	if (disk->cwdDepth == 0 && disk->fatType != FAT_FAT32)
	{
		DIM_SetError(DIMError_DirectoryFull);
		return false;
	}

	uint32_t nextCluster = FAT_FindFreeCluster(disk);
	if (nextCluster == FAT_LAST)
	{
		DIM_SetError(DIMError_DiskFull);
		return false;
	}

	uint32_t currentCluster = FAT_Sector2Cluster(disk, result->sector);
	if (currentCluster == FAT_LAST)
	{
		DIM_SetError(DIMError_InvalidFile);
		return false;
	}

	FAT_SetClusterFATValue(disk, currentCluster, nextCluster);
	FAT_SetClusterFATValue(disk, nextCluster, (unsigned)FAT_LAST);

	result->sector = FAT_Cluster2Sector(disk, nextCluster);
	result->offset = 0;

	memset(sectorData, 0, FAT_SECTOR_SIZE);
	currentDisk = disk;
	currentSector = result->sector;
	return FAT_WriteSectorToDisk(disk, result->sector, sectorData);
}

bool FAT_MakeDirectory (FAT_Disk* disk, const char* filename)
{
	FAT_FindResults results;
	if (FAT_FindFile(disk, &results, filename))
	{
		DIM_SetError(DIMError_FileExists);
		return false;
	}

	if (!FAT_FindFreeDirectoryEntry(disk, &results, false, true))
		return false;

	// Allocate a cluster for the new directory
	uint32_t newCluster = FAT_FindFreeCluster(disk);
	if (newCluster == FAT_LAST)
	{
		DIM_SetError(DIMError_DiskFull);
		return false;
	}
	FAT_SetClusterFATValue(disk, newCluster, FAT_LAST);

	// Initialize the new directory cluster with . and .. entries
	uint32_t sector = FAT_Cluster2Sector(disk, newCluster);
	uint8_t newDirData[FAT_SECTOR_SIZE];
	memset(newDirData, 0, FAT_SECTOR_SIZE);
	
	uint16_t date, time;
	FAT_GetDateTime(&date, &time);

	FAT_DirEntry* dot = (FAT_DirEntry*)newDirData;
	memset(dot, ' ', 11);
	dot->filename[0] = '.';
	dot->attributes = FAT_DIRECTORY;
	dot->startingCluster = (uint16_t)newCluster;
	dot->fat32ClusterHigh = (uint16_t)(newCluster >> 16);
	dot->modifyTime = time;
	dot->modifyDate = date;
	
	FAT_DirEntry* dotdot = (FAT_DirEntry*)(newDirData + FAT_DIRECTORY_ENTRY_SIZE);
	memset(dotdot, ' ', 11);
	dotdot->filename[0] = '.';
	dotdot->filename[1] = '.';
	dotdot->attributes = FAT_DIRECTORY;
	
	uint32_t parentCluster = 0;
	if (disk->cwdDepth > 0)
		parentCluster = disk->cwd[disk->cwdDepth-1].startingCluster | ((uint32_t)disk->cwd[disk->cwdDepth-1].fat32ClusterHigh << 16);
		
	dotdot->startingCluster = (uint16_t)parentCluster;
	dotdot->fat32ClusterHigh = (uint16_t)(parentCluster >> 16);
	dotdot->modifyTime = time;
	dotdot->modifyDate = date;
	
	if (!FAT_WriteSectorToDisk(disk, sector, newDirData))
	{
		FAT_SetClusterFATValue(disk, newCluster, 0);
		return false;
	}
	
	// Clear remaining sectors of the cluster
	if (disk->bootSector->sectorsPerCluster > 1)
	{
		memset(newDirData, 0, FAT_SECTOR_SIZE);
		for (int i = 1; i < disk->bootSector->sectorsPerCluster; i++)
		{
			if (!FAT_WriteSectorToDisk(disk, sector + i, newDirData))
				return false;
		}
	}

	memset(&results.entry, 0, sizeof(FAT_DirEntry));
	results.entry.attributes = FAT_DIRECTORY;
	results.entry.startingCluster = (uint16_t)newCluster;
	results.entry.fat32ClusterHigh = (uint16_t)(newCluster >> 16);
	FAT_GetDateTime(&results.entry.modifyDate, &results.entry.modifyTime);

	const char* ptr = filename;
	for (int n = 0; n < 8; n++)
	{
		if (*ptr == 0 || *ptr == '.')
			results.entry.filename[n] = ' ';
		else
			results.entry.filename[n] = (char)toupper(*ptr++);
	}
	if (*ptr == '.')
		ptr++;
	for (int n = 0; n < 3; n++)
	{
		if (*ptr == 0)
			results.entry.extension[n] = ' ';
		else
			results.entry.extension[n] = (char)toupper(*ptr++);
	}

	return FAT_UpdateDirectoryEntry(disk, &results.entry, results.sector, results.offset);
}

uint32_t FAT_WriteFile (FAT_Disk* disk, const char* filename, const uint8_t* buffer, uint32_t size)
{
	FAT_FindResults results;
	bool existingEntry = false;
	uint8_t preservedAttributes = 0;
	if (FAT_FindFile(disk, &results, filename))
	{
		existingEntry = true;
		preservedAttributes = results.entry.attributes;
		if (!FAT_RemoveFileFromDE(disk, &results.entry, results.sector, results.offset))
			return 0;
	}
	else if (!FAT_FindFreeDirectoryEntry(disk, &results, false, false) &&
			 !FAT_FindFreeDirectoryEntry(disk, &results, true, true))
		return 0;

	uint32_t FreeClusters = FAT_CountFreeClusters(disk);
	uint32_t clusterSize = disk->bootSector->sectorsPerCluster * FAT_SECTOR_SIZE;
	uint32_t clustersNeeded = (size + clusterSize - 1) / clusterSize;
	if (clustersNeeded > FreeClusters)
	{
		DIM_SetError(DIMError_OutOfMemory);
		return 0;
	}

	uint32_t previousCluster = FAT_LAST;
	results.entry.fileSize = 0;
	results.entry.fat32ClusterHigh = (uint16_t)(FAT_LAST >> 16);
	results.entry.startingCluster = 0;
	results.entry.attributes = existingEntry ? preservedAttributes : 0;
	FAT_GetDateTime(&results.entry.modifyDate, &results.entry.modifyTime);
	
	const char* ptr = filename;
	for (int n = 0; n < 8; n++)
	{
		if (*ptr == 0 || *ptr == '.')
			results.entry.filename[n] = ' ';
		else
			results.entry.filename[n] = (char)toupper(*ptr++);
	}
	if (*ptr == '.')
		ptr++;
	for (int n = 0; n < 3; n++)
	{
		if (*ptr == 0)
			results.entry.extension[n] = ' ';
		else
			results.entry.extension[n] = (char)toupper(*ptr++);
	}

	while (size > 0)
	{
		uint32_t cluster = FAT_FindFreeCluster(disk);
		if (cluster == FAT_LAST)
		{
			DIM_SetError(DIMError_DiskFull);
			return 0;
		}
		if (previousCluster != FAT_LAST)
			FAT_SetClusterFATValue(disk, previousCluster, cluster);
		else
		{
			results.entry.fat32ClusterHigh = (uint16_t)(cluster >> 16);
			results.entry.startingCluster = (uint16_t)cluster;
		}

		FAT_SetClusterFATValue(disk, cluster, FAT_LAST);
		previousCluster = cluster;

		uint32_t sector = FAT_Cluster2Sector(disk, cluster);
		for (int n = 0; n < disk->bootSector->sectorsPerCluster; n++, sector++)
		{
			if (size > FAT_SECTOR_SIZE)
			{
				if (!FAT_WriteSectorToDisk(disk, sector, buffer))
				{
					DIM_SetError(DIMError_WriteError);
					FAT_UpdateDirectoryEntry(disk, &results.entry, results.sector, results.offset);
					return results.entry.fileSize;
				}
				buffer += FAT_SECTOR_SIZE;
				size -= FAT_SECTOR_SIZE;
				results.entry.fileSize += FAT_SECTOR_SIZE;
			}
			else
			{
				memcpy(sectorData, buffer, size);
				memset(sectorData + size, 0, FAT_SECTOR_SIZE - size);
				currentDisk = disk;
				currentSector = sector;
				if (!FAT_WriteSectorToDisk(disk, sector, sectorData))
				{
					DIM_SetError(DIMError_WriteError);
					FAT_UpdateDirectoryEntry(disk, &results.entry, results.sector, results.offset);
					return results.entry.fileSize;
				}
				results.entry.fileSize += size;
				size = 0;
				break;
			}
		}
	}

	FAT_UpdateDirectoryEntry(disk, &results.entry, results.sector, results.offset);
	return results.entry.fileSize;
}

static uint16_t FAT_ReadBE16(const uint8_t* ptr)
{
	return (uint16_t)((ptr[0] << 8) | ptr[1]);
}

static bool FAT_DecodeMSA(File* file, uint32_t fileSize, uint8_t** outData, uint32_t* outSize)
{
	if (fileSize < 10)
		return false;

	if (!File_Seek(file, 0))
		return false;

	uint8_t* input = Allocate<uint8_t>("MSA input", fileSize);
	if (!input)
		return false;
	if (File_Read(file, input, fileSize) != fileSize)
	{
		Free(input);
		return false;
	}

	if (FAT_ReadBE16(input) != 0x0E0F)
	{
		Free(input);
		return false;
	}

	uint32_t sectorsPerTrack = FAT_ReadBE16(input + 2);
	uint32_t sideId = FAT_ReadBE16(input + 4);
	uint32_t firstTrack = FAT_ReadBE16(input + 6);
	uint32_t lastTrack = FAT_ReadBE16(input + 8);

	if (sectorsPerTrack == 0 || sectorsPerTrack > 32 || sideId > 1 || lastTrack < firstTrack)
	{
		Free(input);
		return false;
	}

	uint32_t sides = sideId + 1;
	uint32_t tracks = lastTrack - firstTrack + 1;
	uint32_t trackSize = sectorsPerTrack * FAT_SECTOR_SIZE;
	uint64_t rawSize64 = (uint64_t)trackSize * tracks * sides;
	if (rawSize64 == 0 || rawSize64 > 0xFFFFFFFFu)
	{
		Free(input);
		return false;
	}

	uint32_t rawSize = (uint32_t)rawSize64;
	uint8_t* output = Allocate<uint8_t>("MSA output", rawSize);
	if (!output)
	{
		Free(input);
		return false;
	}

	uint32_t inPos = 10;
	uint32_t outPos = 0;

	for (uint32_t track = 0; track < tracks; track++)
	{
		for (uint32_t side = 0; side < sides; side++)
		{
			if (inPos + 2 > fileSize)
			{
				Free(output);
				Free(input);
				return false;
			}

			uint32_t packedSize = FAT_ReadBE16(input + inPos);
			inPos += 2;

			if (packedSize == trackSize)
			{
				if (inPos + trackSize > fileSize || outPos + trackSize > rawSize)
				{
					Free(output);
					Free(input);
					return false;
				}
				memcpy(output + outPos, input + inPos, trackSize);
				inPos += trackSize;
				outPos += trackSize;
			}
			else
			{
				uint32_t remaining = trackSize;
				while (remaining > 0)
				{
					if (inPos >= fileSize || outPos >= rawSize)
					{
						Free(output);
						Free(input);
						return false;
					}

					uint8_t value = input[inPos++];
					if (value != 0xE5)
					{
						output[outPos++] = value;
						remaining--;
					}
					else
					{
						if (inPos + 3 > fileSize)
						{
							Free(output);
							Free(input);
							return false;
						}
						uint8_t repeatValue = input[inPos++];
						uint32_t repeatCount = FAT_ReadBE16(input + inPos);
						inPos += 2;
						if (repeatCount == 0 || repeatCount > remaining || outPos + repeatCount > rawSize)
						{
							Free(output);
							Free(input);
							return false;
						}
						memset(output + outPos, repeatValue, repeatCount);
						outPos += repeatCount;
						remaining -= repeatCount;
					}
				}
			}
		}
	}

	Free(input);
	*outData = output;
	*outSize = rawSize;
	return true;
}

#endif
