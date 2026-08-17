#ifdef HAS_VIRTUALFILESYSTEM

#include <dim_cpc.h>
#include <dim.h>
#include <os_bito.h>
#include <os_mem.h>

#include <ctype.h>
#include <string.h>
#include <stdlib.h>

#pragma pack(push,1)

#define CPC_INFO_SIGNATURE 		"MV - CPCEMU Disk-File\r\nDisk-Info\r\n"
#define CPC_EXTENDED_SIGNATURE	"EXTENDED CPC DSK File\r\nDisk-Info\r\n"
#define CPC_TRACKINFO_SIGNATURE	"Track-Info\r\n"

struct CPC_DiskInfoBlock
{
	char     signature[34];
	char     creator[14];

	uint8_t  tracks;
	uint8_t  sides;
	uint16_t trackSize;		// Little endian
	uint8_t  trackSizes[204];
};

struct CPC_SectorInfo
{
	uint8_t track;
	uint8_t side;
	uint8_t id;
	uint8_t size;
	uint8_t fdc1;
	uint8_t fdc2;

	uint16_t dataLength;			// Only present on extended disks
};

struct CPC_TrackInfoBlock
{
	char signature[12];				// Track-Info\x0D\x0A
	char __unused0__[4];

	uint8_t trackNumber;
	uint8_t sideNumber;
	uint8_t __unused1__[2];
	uint8_t sectorSize;
	uint8_t sectorCount;
	uint8_t gap3Length;
	uint8_t fillerByte;

	CPC_SectorInfo sectors[232];
};

struct CPC_DirEntry
{
	uint8_t user;
	char    fileName[8];
	char    extension[3];
	uint8_t extentLow;
	uint8_t lastRecordByteCount;
	uint8_t extentHigh;
	uint8_t recordCount;
	uint8_t allocation[16];
};

struct CPC_DiskSpec
{
	uint8_t    diskType;
	uint8_t    sideness;
	uint8_t    tracksPerSide;
	uint8_t    sectorsPerTrack;
	uint8_t    sectorSizeLog;
	uint8_t    reservedTracks;
	uint8_t    blockShift;
	uint8_t    directoryBlocks;
	uint8_t    gapReadWrite;
	uint8_t    gapFormat;
	uint8_t    reserved[5];
	uint8_t    checksum;
};

#pragma pack(pop)

enum CPC_DiskType
{
	CPC_FORMAT_SPECTRUM,		// Also PCW SS range
	CPC_FORMAT_CPC_DATA,
	CPC_FORMAT_CPC_SYSTEM,
	CPC_FORMAT_PCW_DS,
};

struct CPC_Disk
{
	File* 		 file;
	uint32_t	 size;
	bool         writable;
	bool         extended;
	bool         doubleSide;
	bool         doubleTracks;

	CPC_DiskType diskType;
	uint16_t     sectorSize;
	uint16_t     tracksPerSide;
	uint16_t     blockSize;

	uint8_t      sectorsPerTrack;
	uint8_t      reservedTracks;
	uint8_t      directoryBlocks;

	CPC_DiskInfoBlock header;

	CPC_DirEntry *directory;
	uint32_t      directoryEntries;
};

struct CPC_DiskPreset
{
	const char* name;
	CPC_DiskType type;
	uint8_t tracksPerSide;
	uint8_t sides;
	uint8_t sectorsPerTrack;
	uint16_t sectorSize;
	uint8_t reservedTracks;
	uint8_t directoryBlocks;
	uint8_t firstSectorId;
	bool writeSpec;
};

static const CPC_DiskPreset cpcDiskPresets[] =
{
	{ "plus3", CPC_FORMAT_SPECTRUM,   40, 1, 9, 512, 1, 2, 0x01, false },
	{ "cpc",   CPC_FORMAT_CPC_SYSTEM, 40, 1, 9, 512, 2, 2, 0x41, false },
	{ "pcw",   CPC_FORMAT_PCW_DS,     80, 2, 9, 512, 2, 2, 0x01, true  },
	{ NULL }
};

uint32_t CPC_GetTrackOffsetInFile (CPC_Disk* disk, uint32_t trackNumber);
uint32_t CPC_GetBlockCount (CPC_Disk* disk);

static const CPC_DiskPreset* CPC_FindPreset(const char* preset)
{
	if (preset == NULL || preset[0] == 0)
		preset = "cpc";

	for (int n = 0; cpcDiskPresets[n].name != NULL; n++)
		if (StrIComp(cpcDiskPresets[n].name, preset) == 0)
			return cpcDiskPresets + n;
	return NULL;
}

static bool CPC_WriteTrackInfo(File* file, const CPC_DiskPreset* preset, uint32_t trackIndex)
{
	uint8_t trackInfoBlock[256];
	MemClear(trackInfoBlock, sizeof(trackInfoBlock));
	CPC_TrackInfoBlock* info = (CPC_TrackInfoBlock*)trackInfoBlock;
	MemCopy(info->signature, CPC_TRACKINFO_SIGNATURE, 12);
	info->trackNumber = (uint8_t)(trackIndex / preset->sides);
	info->sideNumber = (uint8_t)(trackIndex % preset->sides);
	info->sectorSize = 2;
	info->sectorCount = preset->sectorsPerTrack;
	info->gap3Length = 0x2A;
	info->fillerByte = 0xE5;
	for (int n = 0; n < preset->sectorsPerTrack; n++)
	{
		info->sectors[n].track = info->trackNumber;
		info->sectors[n].side = info->sideNumber;
		info->sectors[n].id = (uint8_t)(preset->firstSectorId + n);
		info->sectors[n].size = 2;
		info->sectors[n].fdc1 = 0;
		info->sectors[n].fdc2 = 0;
		info->sectors[n].dataLength = fix16(preset->sectorSize, true);
	}
	return File_Write(file, trackInfoBlock, sizeof(trackInfoBlock)) == sizeof(trackInfoBlock);
}

static bool CPC_WritePresetSpec(uint8_t* sectorData, const CPC_DiskPreset* preset)
{
	if (!preset->writeSpec)
		return true;

	CPC_DiskSpec spec;
	MemClear(&spec, sizeof(spec));
	spec.diskType = (uint8_t)preset->type;
	spec.sideness = preset->sides > 1 ? 1 : 0;
	spec.tracksPerSide = preset->tracksPerSide;
	spec.sectorsPerTrack = preset->sectorsPerTrack;
	spec.sectorSizeLog = 2;
	spec.reservedTracks = preset->reservedTracks;
	spec.blockShift = 3;
	spec.directoryBlocks = preset->directoryBlocks;
	spec.gapReadWrite = 0x2A;
	spec.gapFormat = 0x52;
	MemCopy(sectorData, &spec, sizeof(spec));
	return true;
}

CPC_Disk* CPC_CreateDisk(const char* fileName, const char* presetName)
{
	const CPC_DiskPreset* preset = CPC_FindPreset(presetName);
	if (preset == NULL)
	{
		DIM_SetError(DIMError_FormatNotSupported);
		return NULL;
	}

	File* file = File_Create(fileName);
	if (file == NULL)
	{
		DIM_SetError(DIMError_CreatingFile);
		return NULL;
	}

	uint16_t trackSize = (uint16_t)(256 + preset->sectorsPerTrack * preset->sectorSize);
	uint32_t totalTracks = preset->tracksPerSide * preset->sides;

	CPC_DiskInfoBlock header;
	MemClear(&header, sizeof(header));
	MemCopy(header.signature, CPC_EXTENDED_SIGNATURE, sizeof(header.signature));
	StrCopy(header.creator, sizeof(header.creator), "ADP");
	header.tracks = preset->tracksPerSide;
	header.sides = preset->sides;
	header.trackSize = 0;
	for (uint32_t n = 0; n < totalTracks && n < sizeof(header.trackSizes); n++)
		header.trackSizes[n] = (uint8_t)(trackSize >> 8);
	if (File_Write(file, &header, sizeof(header)) != sizeof(header))
	{
		File_Close(file);
		DIM_SetError(DIMError_WriteError);
		return NULL;
	}

	uint8_t* sectorData = Allocate<uint8_t>("CPC Create Disk Sector", preset->sectorSize);
	if (sectorData == NULL)
	{
		File_Close(file);
		DIM_SetError(DIMError_OutOfMemory);
		return NULL;
	}
	MemSet(sectorData, 0xE5, preset->sectorSize);

	for (uint32_t track = 0; track < totalTracks; track++)
	{
		if (!CPC_WriteTrackInfo(file, preset, track))
		{
			Free(sectorData);
			File_Close(file);
			DIM_SetError(DIMError_WriteError);
			return NULL;
		}

		for (int sector = 0; sector < preset->sectorsPerTrack; sector++)
		{
			MemSet(sectorData, 0xE5, preset->sectorSize);
			if (track == 0 && sector == 0)
				CPC_WritePresetSpec(sectorData, preset);
			if (File_Write(file, sectorData, preset->sectorSize) != preset->sectorSize)
			{
				Free(sectorData);
				File_Close(file);
				DIM_SetError(DIMError_WriteError);
				return NULL;
			}
		}
	}

	Free(sectorData);
	File_Close(file);
	return CPC_OpenDisk(fileName);
}

static bool CPC_IsActiveEntry(const CPC_DirEntry* entry)
{
	return entry->user < 16 && entry->fileName[0] != 0;
}

static bool CPC_MatchesName(const CPC_DirEntry* entry, const char* fileName, const char* extension)
{
	return CPC_IsActiveEntry(entry) &&
		MemComp((void*)entry->fileName, fileName, 8) == 0 &&
		MemComp((void*)entry->extension, extension, 3) == 0;
}

static bool CPC_MatchesHeader8D3(const uint8_t* headerName, const char* entryName, int length)
{
	for (int n = 0; n < length; n++)
		if ((headerName[n] & 0x7F) != (entryName[n] & 0x7F))
			return false;
	return true;
}

static bool CPC_NormalizeFileName(const char* name, char fileName[8], char extension[3])
{
	memset(fileName, ' ', 8);
	memset(extension, ' ', 3);

	if (name == NULL || *name == 0)
		return false;

	const char* ptr = name;
	const char* dot = strrchr(name, '.');
	if (dot == name)
		return false;

	int out = 0;
	while (*ptr && ptr != dot)
	{
		if (*ptr == '/' || *ptr == '\\' || *ptr == ':' || *ptr == '*' || *ptr == '?')
			return false;
		if (out < 8)
			fileName[out++] = (char)toupper((unsigned char)*ptr);
		ptr++;
	}
	if (out == 0)
		return false;

	if (dot != NULL)
	{
		ptr = dot + 1;
		out = 0;
		while (*ptr)
		{
			if (*ptr == '/' || *ptr == '\\' || *ptr == ':' || *ptr == '*' || *ptr == '?' || *ptr == '.')
				return false;
			if (out < 3)
				extension[out++] = (char)toupper((unsigned char)*ptr);
			ptr++;
		}
	}

	return true;
}

static bool CPC_ValidateInferredFilesystemLayout(CPC_Disk* disk, uint8_t expectedFirstSectorId)
{
	uint32_t totalTracks = disk->header.tracks * (disk->header.sides == 0 ? 1 : disk->header.sides);
	if (totalTracks == 0)
		return false;
	uint32_t protectedTrackCount = disk->reservedTracks;
	if (disk->sectorsPerTrack != 0 && disk->sectorSize != 0)
	{
		uint32_t bytesPerTrack = (uint32_t)disk->sectorsPerTrack * disk->sectorSize;
		uint32_t directoryBytes = (uint32_t)disk->directoryBlocks * disk->blockSize;
		protectedTrackCount += (directoryBytes + bytesPerTrack - 1) / bytesPerTrack;
	}

	for (uint32_t trackNumber = 0; trackNumber < totalTracks; trackNumber++)
	{
		if (disk->extended && disk->header.trackSizes[trackNumber] == 0)
			continue;

		uint32_t offsetInFile = CPC_GetTrackOffsetInFile(disk, trackNumber);
		if (offsetInFile + 256 > disk->size)
			return false;

		uint8_t trackInfoBlock[256];
		CPC_TrackInfoBlock* info = (CPC_TrackInfoBlock*)trackInfoBlock;
		if (!File_Seek(disk->file, offsetInFile) || File_Read(disk->file, trackInfoBlock, 256) != 256)
			return false;
		if (MemComp(info->signature, CPC_TRACKINFO_SIGNATURE, 12) != 0)
			return false;
		bool allowRawFallback = !disk->extended && trackNumber >= protectedTrackCount &&
			disk->header.trackSize == (uint16_t)(256 + (uint32_t)disk->sectorsPerTrack * disk->sectorSize);

		if (info->sectorCount != disk->sectorsPerTrack)
		{
			if (allowRawFallback)
				continue;
			return false;
		}

		uint32_t computedTrackSize = 256;
		bool seen[256] = { false };
		bool invalidDescriptors = false;
		for (int n = 0; n < info->sectorCount; n++)
		{
			uint32_t sectorSize = disk->extended ? info->sectors[n].dataLength : (128U << info->sectors[n].size);
			if (sectorSize != disk->sectorSize)
			{
				invalidDescriptors = true;
				break;
			}

			uint8_t sectorId = info->sectors[n].id;
			if (sectorId < expectedFirstSectorId || sectorId >= expectedFirstSectorId + disk->sectorsPerTrack)
			{
				invalidDescriptors = true;
				break;
			}
			if (seen[sectorId])
			{
				invalidDescriptors = true;
				break;
			}
			seen[sectorId] = true;
			computedTrackSize += sectorSize;
		}
		if (invalidDescriptors)
		{
			if (allowRawFallback)
				continue;
			return false;
		}

		for (uint32_t sectorId = expectedFirstSectorId; sectorId < expectedFirstSectorId + disk->sectorsPerTrack; sectorId++)
			if (!seen[sectorId])
			{
				if (allowRawFallback)
				{
					invalidDescriptors = true;
					break;
				}
				return false;
			}
		if (invalidDescriptors)
			continue;

		if (!disk->extended && computedTrackSize != disk->header.trackSize)
			return false;
		if (disk->extended)
		{
			uint8_t declaredSize = disk->header.trackSizes[trackNumber];
			if ((computedTrackSize >> 8) != declaredSize)
				return false;
		}
	}

	return true;
}

static bool CPC_CanUseRawTrackFallback(CPC_Disk* disk, CPC_TrackInfoBlock* info)
{
	if (disk->extended)
		return false;
	if (MemComp(info->signature, CPC_TRACKINFO_SIGNATURE, 12) != 0)
		return false;
	return disk->header.trackSize == (uint16_t)(256 + (uint32_t)disk->sectorsPerTrack * disk->sectorSize);
}

static void CPC_BuildRawTrackLayout(CPC_Disk* disk, uint32_t* totalSize,
	uint32_t sectorOffsets[232], uint32_t sectorSizes[232], uint8_t* sectorCount)
{
	*sectorCount = disk->sectorsPerTrack;
	*totalSize = (uint32_t)disk->sectorsPerTrack * disk->sectorSize;
	for (uint32_t n = 0; n < *sectorCount; n++)
	{
		sectorOffsets[n] = n * disk->sectorSize;
		sectorSizes[n] = disk->sectorSize;
	}
}

static bool CPC_TryInferredFilesystemLayout(CPC_Disk* disk, CPC_DiskType diskType,
	uint16_t tracksPerSide, uint8_t sectorsPerTrack, uint16_t sectorSize,
	uint8_t reservedTracks, uint8_t directoryBlocks, uint16_t blockSize,
	uint8_t expectedFirstSectorId)
{
	disk->diskType = diskType;
	disk->tracksPerSide = tracksPerSide;
	disk->sectorsPerTrack = sectorsPerTrack;
	disk->sectorSize = sectorSize;
	disk->reservedTracks = reservedTracks;
	disk->directoryBlocks = directoryBlocks;
	disk->blockSize = blockSize;
	return CPC_ValidateInferredFilesystemLayout(disk, expectedFirstSectorId);
}

static bool CPC_ReadTrackLayout(CPC_Disk* disk, uint32_t trackNumber,
	uint64_t* dataPosition, uint32_t* totalSize,
	uint32_t sectorOffsets[232], uint32_t sectorSizes[232], uint8_t* sectorCount)
{
	uint8_t trackInfoBlock[256];
	CPC_TrackInfoBlock* info = (CPC_TrackInfoBlock*)&trackInfoBlock;

	uint32_t offsetInFile = CPC_GetTrackOffsetInFile(disk, trackNumber);
	if (!File_Seek(disk->file, offsetInFile) || File_Read(disk->file, trackInfoBlock, 256) != 256)
	{
		DIM_SetError(DIMError_ReadError);
		return false;
	}
	if (MemComp(info->signature, CPC_TRACKINFO_SIGNATURE, 12) != 0)
	{
		DIM_SetError(DIMError_InvalidDisk);
		return false;
	}

	*dataPosition = File_GetPosition(disk->file);
	*sectorCount = info->sectorCount;
	*totalSize = 0;
	if (info->sectorCount != disk->sectorsPerTrack && CPC_CanUseRawTrackFallback(disk, info))
	{
		CPC_BuildRawTrackLayout(disk, totalSize, sectorOffsets, sectorSizes, sectorCount);
		return true;
	}

	int previous = -1;
	for (int n = 0; n < info->sectorCount; n++)
	{
		int offset = 0;
		int current = 999;
		uint32_t currentSize = 0;
		int currentOffset = 0;
		for (int i = 0; i < info->sectorCount; i++)
		{
			int size = disk->extended ? info->sectors[i].dataLength : info->sectorSize * 256;
			if (info->sectors[i].id < current && info->sectors[i].id > previous)
			{
				currentOffset = offset;
				current = info->sectors[i].id;
				currentSize = size;
			}
			offset += size;
		}
		if (current == 999)
		{
			if (CPC_CanUseRawTrackFallback(disk, info))
			{
				CPC_BuildRawTrackLayout(disk, totalSize, sectorOffsets, sectorSizes, sectorCount);
				return true;
			}
			DIM_SetError(DIMError_InvalidDisk);
			return false;
		}
		previous = current;
		sectorOffsets[n] = currentOffset;
		sectorSizes[n] = currentSize;
		*totalSize += currentSize;
	}

	return true;
}

static uint32_t CPC_WriteTrack(CPC_Disk* disk, uint32_t trackNumber, uint32_t baseOffset, const uint8_t* buffer, uint32_t bufferSize)
{
	uint32_t sectorOffsets[232];
	uint32_t sectorSizes[232];
	uint8_t sectorCount = 0;
	uint32_t totalSize = 0;
	uint64_t dataPosition = 0;
	if (!CPC_ReadTrackLayout(disk, trackNumber, &dataPosition, &totalSize, sectorOffsets, sectorSizes, &sectorCount))
		return 0;
	if (baseOffset >= totalSize)
		return 0;

	uint32_t remaining = bufferSize;
	uint32_t written = 0;
	for (int n = 0; n < sectorCount; n++)
	{
		uint32_t currentSize = sectorSizes[n];
		uint32_t currentOffset = sectorOffsets[n];

		if (baseOffset >= currentSize)
		{
			baseOffset -= currentSize;
			continue;
		}

		currentOffset += baseOffset;
		currentSize -= baseOffset;
		baseOffset = 0;

		if (currentSize > remaining)
			currentSize = remaining;
		if (currentSize == 0)
			continue;

		if (!File_Seek(disk->file, dataPosition + currentOffset) || File_Write(disk->file, buffer, currentSize) != currentSize)
		{
			DIM_SetError(DIMError_WriteError);
			return 0;
		}

		buffer += currentSize;
		remaining -= currentSize;
		written += currentSize;
		if (remaining == 0)
			return written;
	}

	return written;
}

static uint32_t CPC_WriteBlock(CPC_Disk* disk, uint32_t blockIndex, const uint8_t* buffer, uint32_t bufferSize)
{
	uint32_t size = disk->blockSize > bufferSize ? bufferSize : disk->blockSize;
	uint32_t offset = blockIndex * disk->blockSize;
	uint32_t sector = offset / disk->sectorSize;
	uint32_t track = sector / disk->sectorsPerTrack;
	uint32_t trackSize = disk->sectorsPerTrack * disk->sectorSize;
	uint32_t trackOffset = offset - track * disk->sectorSize * disk->sectorsPerTrack;
	uint32_t result = CPC_WriteTrack(disk, disk->reservedTracks + track, trackOffset, buffer, size);

	if (trackSize - trackOffset < size && result < size)
	{
		uint32_t remaining = size - result;
		result += CPC_WriteTrack(disk, disk->reservedTracks + track + 1, 0, buffer + result, remaining);
	}
	return result;
}

static uint32_t CPC_GetDirectorySize(CPC_Disk* disk)
{
	return disk->directoryEntries * 32;
}

static bool CPC_FlushDirectory(CPC_Disk* disk)
{
	uint32_t directorySize = CPC_GetDirectorySize(disk);
	uint8_t* ptr = (uint8_t*)disk->directory;
	for (uint32_t block = 0; block < disk->directoryBlocks; block++)
	{
		uint32_t offset = block * disk->blockSize;
		uint32_t size = disk->blockSize;
		if (offset >= directorySize)
			break;
		if (offset + size > directorySize)
			size = directorySize - offset;
		if (CPC_WriteBlock(disk, block, ptr + offset, size) != size)
			return false;
	}
	return true;
}

static bool CPC_BuildBlockMap(CPC_Disk* disk, uint8_t* used, uint32_t blockCount)
{
	memset(used, 0, blockCount);
	for (uint32_t n = 0; n < disk->directoryBlocks && n < blockCount; n++)
		used[n] = 1;

	for (uint32_t n = 0; n < disk->directoryEntries; n++)
	{
		CPC_DirEntry* entry = disk->directory + n;
		if (!CPC_IsActiveEntry(entry))
			continue;
		for (int i = 0; i < 16; i++)
		{
			uint8_t block = entry->allocation[i];
			if (block == 0)
				continue;
			if (block >= blockCount)
			{
				DIM_SetError(DIMError_InvalidDisk);
				return false;
			}
			used[block] = 1;
		}
	}

	return true;
}

static uint32_t CPC_CountFreeDirectoryEntries(CPC_Disk* disk)
{
	uint32_t count = 0;
	for (uint32_t n = 0; n < disk->directoryEntries; n++)
	{
		CPC_DirEntry* entry = disk->directory + n;
		if (!CPC_IsActiveEntry(entry))
			count++;
	}
	return count;
}

static void CPC_GetFileStats(CPC_Disk* disk, const char* fileName, const char* extension, uint32_t* entries, uint32_t* blocks)
{
	*entries = 0;
	*blocks = 0;
	for (uint32_t n = 0; n < disk->directoryEntries; n++)
	{
		CPC_DirEntry* entry = disk->directory + n;
		if (!CPC_MatchesName(entry, fileName, extension))
			continue;
		(*entries)++;
		for (int i = 0; i < 16; i++)
			if (entry->allocation[i] != 0)
				(*blocks)++;
	}
}

static uint32_t CPC_FindFreeDirectoryEntry(CPC_Disk* disk)
{
	for (uint32_t n = 0; n < disk->directoryEntries; n++)
		if (!CPC_IsActiveEntry(disk->directory + n))
			return n;
	return disk->directoryEntries;
}

static bool CPC_DeleteMatchingEntries(CPC_Disk* disk, const char* fileName, const char* extension, bool flush)
{
	bool deleted = false;
	for (uint32_t n = 0; n < disk->directoryEntries; n++)
	{
		CPC_DirEntry* entry = disk->directory + n;
		if (!CPC_MatchesName(entry, fileName, extension))
			continue;
		memset(entry, 0, sizeof(CPC_DirEntry));
		entry->user = 0xE5;
		deleted = true;
	}
	if (!deleted)
	{
		DIM_SetError(DIMError_FileNotFound);
		return false;
	}
	if (flush && !CPC_FlushDirectory(disk))
		return false;
	return true;
}

uint32_t CPC_GetTrackOffsetInFile (CPC_Disk* disk, uint32_t trackNumber)
{
	if (!disk->extended)
		return 256 + disk->header.trackSize * trackNumber;

	uint32_t offset = 256;
	for (unsigned n = 0; n < trackNumber; n++)
		offset += disk->header.trackSizes[n] << 8;
	return offset;
}

uint32_t CPC_ReadTrack (CPC_Disk* disk, uint32_t trackNumber, uint32_t baseOffset, uint8_t* buffer, uint32_t bufferSize, uint32_t skip = 0)
{
	uint32_t sectorOffsets[232];
	uint32_t sectorSizes[232];
	uint8_t sectorCount = 0;
	uint32_t size = 0;
	uint64_t dataPosition = 0;
	if (!CPC_ReadTrackLayout(disk, trackNumber, &dataPosition, &size, sectorOffsets, sectorSizes, &sectorCount))
		return 0;

	if (baseOffset >= size)
		return 0;

	uint32_t remainingToRead = bufferSize;
	uint32_t remainingSkip = skip;

	uint32_t totalRead = 0;
	for (int n = 0; n < sectorCount; n++)
	{
		uint32_t currentOffset = sectorOffsets[n];
		uint32_t currentSize = sectorSizes[n];

		if (baseOffset >= currentSize)
		{
			baseOffset -= currentSize;
			continue;
		}

		currentOffset += baseOffset;
		currentSize -= baseOffset;
		baseOffset = 0;

		if (remainingSkip >= currentSize)
		{
			remainingSkip -= currentSize;
			continue;
		}

		currentOffset += remainingSkip;
		currentSize -= remainingSkip;
		remainingSkip = 0;

		if (currentSize > remainingToRead)
			currentSize = remainingToRead;

		if (currentSize == 0)
			continue;

		File_Seek(disk->file, dataPosition + currentOffset);
		if (File_Read(disk->file, buffer, currentSize) != currentSize)
		{
			DIM_SetError(DIMError_ReadError);
			return 0;
		}

		remainingToRead -= currentSize;
		totalRead += currentSize;
		buffer += currentSize;
		if (remainingToRead == 0)
			return totalRead;
	}

	return totalRead;
}

uint32_t CPC_ReadBlock (CPC_Disk* disk, uint32_t blockIndex, uint8_t* buffer, uint32_t bufferSize, uint32_t skip = 0)
{
	uint32_t size = disk->blockSize > bufferSize ? bufferSize : disk->blockSize;
	uint32_t offset = blockIndex * disk->blockSize;
	uint32_t sector = offset / disk->sectorSize;
	uint32_t track = sector / disk->sectorsPerTrack;
	uint32_t trackSize = disk->sectorsPerTrack * disk->sectorSize;
	uint32_t trackOffset = offset - track * disk->sectorSize * disk->sectorsPerTrack;
	uint32_t result = CPC_ReadTrack(disk, disk->reservedTracks + track, trackOffset, buffer, size, skip);

	if (trackSize - trackOffset < size && result < size)
	{
		uint32_t remaining = size - result;
		result += CPC_ReadTrack(disk, disk->reservedTracks + track + 1, 0, buffer + result, remaining);
	}
	return result;
}

CPC_Disk* CPC_OpenDisk (const char* fileName)
{
	uint8_t buffer[256];

	bool writable = true;
	File* file = File_Open(fileName, ReadWrite);
	if (file == NULL)
	{
		writable = false;
		file = File_Open(fileName, ReadOnly);
	}
	if (file == NULL)
	{
		DIM_SetError(DIMError_FileNotFound);
		return NULL;
	}
	if (File_Read(file, buffer, 256) != 256)
	{
		File_Close(file);
		DIM_SetError(DIMError_ReadError);
		return NULL;
	}
	if (MemComp(buffer, CPC_EXTENDED_SIGNATURE, 8) != 0 &&
        MemComp(buffer, CPC_INFO_SIGNATURE, 8) != 0)
	{
		File_Close(file);
		DIM_SetError(DIMError_InvalidFile);
		return NULL;

	}
	CPC_Disk* disk = Allocate<CPC_Disk>("CPC Disk");
	if (disk == NULL)
	{
		File_Close(file);
		DIM_SetError(DIMError_OutOfMemory);
		return NULL;
	}
	memset(disk, 0, sizeof(CPC_Disk));

	disk->size = (uint32_t)File_GetSize(file);
	disk->file = file;
	disk->writable = writable;
	disk->extended = buffer[0] == 'E';

	MemCopy(&disk->header, buffer, 256);

	bool ok = true;

	File_Seek(file, 256);
	if (File_Read(file, buffer, 256) != 256)
	{
		File_Close(file);
		Free(disk);
		DIM_SetError(DIMError_ReadError);
		return NULL;
	}

	CPC_TrackInfoBlock* firstTrackInfo = (CPC_TrackInfoBlock*)buffer;
	if (MemComp(firstTrackInfo->signature, CPC_TRACKINFO_SIGNATURE, 12) != 0)
	{
		DIM_SetError(DIMError_InvalidDisk);
		File_Close(disk->file);
		Free(disk);
		return NULL;
	}

	if (CPC_TryInferredFilesystemLayout(disk, CPC_FORMAT_CPC_SYSTEM, 40, 9, 512, 2, 2, 1024, 0x41))
	{
		ok = true;
	}
	else if (CPC_TryInferredFilesystemLayout(disk, CPC_FORMAT_CPC_DATA, 40, 9, 512, 0, 2, 1024, 0xC1))
	{
		ok = true;
	}
	else
	{
		File_Seek(file, 512);
		if (File_Read(file, buffer, 256) != 256)
		{
			File_Close(file);
			Free(disk);
			DIM_SetError(DIMError_ReadError);
			return NULL;
		}

		uint8_t firstByte = buffer[0];
		bool matches = true;
		for (int n = 1; matches && n < 10 ; n++)
			matches = buffer[n] == firstByte;

		if (matches)
		{
			// Assume this is a standard disk
			ok = CPC_TryInferredFilesystemLayout(disk, CPC_FORMAT_SPECTRUM, 40, 9, 512, 1, 2, 1024, 0x01);
		}
		else
		{
			// The disk actually contains a specs array
			CPC_DiskSpec* spec = (CPC_DiskSpec*)buffer;
			disk->diskType = (CPC_DiskType)spec->diskType;
			disk->doubleSide = (spec->sideness & 0x03) != 0;
			disk->doubleTracks = (spec->sideness & 0x80) != 0;
			disk->reservedTracks = spec->reservedTracks;
			disk->directoryBlocks = spec->directoryBlocks;
			disk->blockSize = (0x80 << spec->blockShift);
			disk->sectorSize = 128 << spec->sectorSizeLog;
			disk->sectorsPerTrack = spec->sectorsPerTrack;
			disk->tracksPerSide = spec->tracksPerSide;

			ok = spec->diskType < 4;
		}
	}

	if (ok)
	{
		disk->directoryEntries = disk->blockSize * disk->directoryBlocks / 32;
		disk->directory = Allocate<CPC_DirEntry>("CPC Disk directory", disk->directoryEntries);
		if (disk->directory != NULL)
		{
			CPC_ReadTrack(disk, disk->reservedTracks, 0,
			              (uint8_t*)disk->directory,
			              disk->directoryEntries * 32);
			return disk;
		}
		else
		{
			DIM_SetError(DIMError_OutOfMemory);
			File_Close(disk->file);
			Free(disk);
			return NULL;
		}
	}

	DIM_SetError(DIMError_InvalidDisk);
	File_Close(disk->file);
	Free(disk);
	return NULL;
}

bool CPC_FindFile (CPC_Disk* disk, CPC_FindResults* result, const char* name)
{
	return CPC_FindFirstFile(disk, result, name);
}

bool CPC_FindFirstFile (CPC_Disk* disk, CPC_FindResults* result, const char* pattern)
{
	DIM_CopyPatternTo8D3(pattern, result->matchName, result->matchExtension);
	result->offset = -1;
	result->entries = 0;
	return CPC_FindNextFile (disk, result);
}

CPC_OSHeaderType CPC_CheckOSHeader (CPC_FindResults* result, uint8_t* header)
{
	if (MemComp(header, "PLUS3DOS", 8) == 0)
	{
		uint8_t checksum = 0;
		for (int n = 0; n < 127; n++)
			checksum += header[n];
		if (header[127] != checksum)
			return CPC_HEADER_NONE;
		if (result == NULL)
			return CPC_HEADER_PLUS3DOS;

		uint32_t totalSize = header[11] + (header[12] << 8) + (header[13] << 16) + (header[14] << 24);
		result->fileSize = totalSize >= 128 ? totalSize - 128 : 0;
		uint16_t v0 = header[18] + (header[19] << 8);
		uint16_t v1 = header[20] + (header[21] << 8);
		switch (header[15])
		{
			case 0:
				if (v0 == 0x8000)
					snprintf(result->description, 32, "BASIC");
				else
					snprintf(result->description, 32, "BASIC LINE %d", v0);
				break;
			case 1:
				snprintf(result->description, 32, "DATA %c", header[19] - 64);
				break;
			case 2:
				snprintf(result->description, 32, "DATA %c$", header[19] - 64);
				break;
			case 3:
				if (v0 == 0)
					snprintf(result->description, 32, "CODE");
				else
					snprintf(result->description, 32, "CODE %d", v0);
				break;
		}
		return CPC_HEADER_PLUS3DOS;
	}

	uint32_t checksum = 0;
	for (int n = 0; n < 66; n++)
		checksum += header[n];
	if (checksum == 0 || (checksum & 0xFFFF) != (uint16_t)(header[67] + (header[68] << 8)))
		return CPC_HEADER_NONE;
	if (result == NULL)
		return CPC_HEADER_AMSDOS;

	result->fileSize = header[64] + (header[65] << 8) + (header[66] << 16);
	uint16_t v0 = header[21] + (header[22] << 8);
	uint16_t v1 = header[26] + (header[27] << 8);
	switch (header[18])
	{
		case 0:
			snprintf(result->description, 32, "BASIC");
			break;
		case 1:
			snprintf(result->description, 32, "BASIC(P)");
			break;
		case 2:
			if (v1)
				snprintf(result->description, 32, "BINARY %d EXEC %d", v0, v1);
			else
				snprintf(result->description, 32, "BINARY %d", v0);
			break;
		case 3:
			if (v1)
				snprintf(result->description, 32, "BINARY(P) %d EXEC %d", v0, v1);
			else
				snprintf(result->description, 32, "BINARY(P) %d", v0);
			break;
		case 4:
			snprintf(result->description, 32, "SCREEN");
			break;
		case 5:
			snprintf(result->description, 32, "SCREEN(P)");
			break;
		case 6:
			snprintf(result->description, 32, "ASCII");
			break;
		case 7:
			snprintf(result->description, 32, "ASCII [protected]");
			break;
	}
	return CPC_HEADER_AMSDOS;
}

static uint32_t CPC_GetExtentSize(const CPC_DirEntry* entry)
{
	return entry->recordCount * 128;
}

bool CPC_FindNextFile (CPC_Disk* disk, CPC_FindResults* result)
{
	bool found = false;
	CPC_OSHeaderType osHeaderType = CPC_HEADER_NONE;
	uint8_t extent;
	uint8_t header[128];
	uint8_t name[8];
	uint8_t extension[3];
	uint32_t logicalSize = 0;

	uint16_t offset = result->offset;

	for (;;)
	{
		offset++;
		if (offset >= disk->directoryEntries)
			break;

		CPC_DirEntry* entry = disk->directory + offset;

		if (entry->user >= 16 || entry->fileName[0] == 0)
			continue;

		if (!found)
		{
			if (entry->extentLow != 0)
				continue;
			if (!DIM_MatchWildcards(entry->fileName, 8, result->matchName, 8) ||
				!DIM_MatchWildcards(entry->extension, 3, result->matchExtension, 3))
			{
				continue;
			}

			char* out = result->fileName;
			char* in  = entry->fileName;
			for (int n = 0; n < 8 && *in != ' '; n++)
				*out++ = *in++;
			in = entry->extension;
			if ((*in & 0x7F) != ' ')
			{
				*out++ = '.';
				for (int n = 0; n < 3 && *in != ' '; n++)
					*out++ = *in++ & 0x7F;
			}
			*out = 0;
			result->fileSize = 0;

			MemCopy(name, entry->fileName, 8);
			MemCopy(extension, entry->extension, 3);

			result->osHeaderType = CPC_HEADER_NONE;
			result->description[0] = 0;
			if (entry->allocation[0] != 0 &&
				CPC_ReadBlock(disk, entry->allocation[0], header, 128) == 128)
			{
				osHeaderType = CPC_CheckOSHeader(result, header);
				if (osHeaderType == CPC_HEADER_AMSDOS)
				{
					if (header[0] >= 16 ||
						!CPC_MatchesHeader8D3(header + 1, entry->fileName, 8) ||
						!CPC_MatchesHeader8D3(header + 9, entry->extension, 3))
					{
						osHeaderType = CPC_HEADER_NONE;
						result->description[0] = 0;
						result->fileSize = 0;
					}
				}
				logicalSize = result->fileSize;
				result->fileSize = 0;
			}

			result->offset = offset;
			result->entries = 1;
			found = true;
		}
		else
		{
			if (MemComp(entry->fileName, name, 8) != 0 ||
				MemComp(entry->extension, extension, 3) != 0)
				continue;
			if (entry->extentLow != extent)
				continue;
			result->entries++;
		}

		uint32_t extentSize = CPC_GetExtentSize(entry);
		result->fileSize += extentSize;

		extent = entry->extentLow + 1;
	}

	if (osHeaderType != CPC_HEADER_NONE)
	{
		uint32_t physicalSize = result->fileSize;
		bool headerValid = false;
		if (osHeaderType == CPC_HEADER_PLUS3DOS)
		{
			headerValid = physicalSize >= logicalSize + 128;
		}
		else
		{
			headerValid =
				logicalSize > 0 &&
				physicalSize >= logicalSize;
		}

		if (headerValid)
		{
			result->fileSize = logicalSize;
			result->osHeaderType = osHeaderType;
		}
		else
		{
			result->fileSize = physicalSize;
			result->description[0] = 0;
			result->osHeaderType = CPC_HEADER_NONE;
		}
	}

	return found;
}

CPC_DirEntry* CPC_NextFileEntry (CPC_Disk* disk, CPC_DirEntry* current)
{
	CPC_DirEntry* end = disk->directory + disk->directoryEntries;
	for (CPC_DirEntry* entry = current + 1; entry < end; entry++)
	{
		if (MemComp(entry->fileName, current->fileName, 8) == 0 &&
			MemComp(entry->extension, current->extension, 3) == 0 &&
			entry->extentLow == current->extentLow + 1)
			return entry;
	}
	return NULL;
}

uint32_t CPC_ReadFile (CPC_Disk* disk, const char* name, uint8_t* buffer, uint32_t bufferSize)
{
	CPC_FindResults results;
	if (!CPC_FindFile(disk, &results, name))
		return 0;


	uint32_t totalRead = 0;
	uint32_t remaining = results.fileSize;

	CPC_DirEntry* entry = disk->directory + results.offset;
	for (int n = 0; n < results.entries; n++)
	{
		for (int i = 0; i < 16; i++)
		{
			if (entry->allocation[i] != 0 && remaining > 0)
			{
				uint32_t readSize = bufferSize < remaining ? bufferSize : remaining;
				uint32_t skip = 0;
				if (n == 0 && i == 0 && results.osHeaderType != CPC_HEADER_NONE)
				{
					skip = 128;
					uint32_t firstBlockPayload = disk->blockSize > skip ? disk->blockSize - skip : 0;
					if (readSize > firstBlockPayload)
						readSize = firstBlockPayload;
				}

				uint32_t read = CPC_ReadBlock(disk, entry->allocation[i], buffer, readSize, skip);
				if (read == 0)
					return 0;

				buffer += read;
				totalRead += read;
				bufferSize -= read;
				remaining -= read;
				if (bufferSize == 0 || remaining == 0)
					return totalRead;
			}
		}
		entry = CPC_NextFileEntry(disk, entry);
		if (entry == NULL)
		{
			if (n + 1 < results.entries)
				printf("File %s: truncated file, missing extent %d\n", name, n + 1);
			break;
		}
	}
	return totalRead;
}

bool CPC_DeleteFile(CPC_Disk* disk, const char* name)
{
	if (!disk->writable)
	{
		DIM_SetError(DIMError_WriteError);
		return false;
	}

	char fileName[8];
	char extension[3];
	if (!CPC_NormalizeFileName(name, fileName, extension))
	{
		DIM_SetError(DIMError_FileNotFound);
		return false;
	}

	return CPC_DeleteMatchingEntries(disk, fileName, extension, true);
}

uint32_t CPC_WriteFile(CPC_Disk* disk, const char* name, const uint8_t* buffer, uint32_t bufferSize)
{
	if (!disk->writable)
	{
		DIM_SetError(DIMError_WriteError);
		return 0;
	}

	char fileName[8];
	char extension[3];
	if (!CPC_NormalizeFileName(name, fileName, extension))
	{
		DIM_SetError(DIMError_CommandNotSupported);
		return 0;
	}

	if ((disk->blockSize & 0x7F) != 0)
	{
		DIM_SetError(DIMError_CommandNotSupported);
		return 0;
	}

	uint32_t recordsPerBlock = disk->blockSize >> 7;
	uint32_t blocksPerExtent = 16;
	uint32_t maxBlocksByRecordCount = 255 / recordsPerBlock;
	if (recordsPerBlock == 0 || maxBlocksByRecordCount == 0)
	{
		DIM_SetError(DIMError_CommandNotSupported);
		return 0;
	}
	if (blocksPerExtent > maxBlocksByRecordCount)
		blocksPerExtent = maxBlocksByRecordCount;

	uint32_t blockCount = CPC_GetBlockCount(disk);
	uint8_t* used = Allocate<uint8_t>("CPC block map", blockCount == 0 ? 1 : blockCount);
	if (used == NULL)
	{
		DIM_SetError(DIMError_OutOfMemory);
		return 0;
	}
	if (!CPC_BuildBlockMap(disk, used, blockCount))
	{
		Free(used);
		return 0;
	}

	uint32_t freeBlocks = 0;
	for (uint32_t n = disk->directoryBlocks; n < blockCount; n++)
		freeBlocks += !used[n];

	uint32_t existingEntries = 0;
	uint32_t existingBlocks = 0;
	CPC_GetFileStats(disk, fileName, extension, &existingEntries, &existingBlocks);

	uint64_t remaining = bufferSize;
	uint32_t totalBlocksNeeded = 0;
	uint32_t extentsNeeded = 0;
	for (;;)
	{
		uint64_t bytesThisExtent = remaining;
		uint64_t maxBytesThisExtent = (uint64_t)blocksPerExtent * disk->blockSize;
		if (bytesThisExtent > maxBytesThisExtent)
			bytesThisExtent = maxBytesThisExtent;
		uint32_t blocksThisExtent = bytesThisExtent == 0 ? 0 : (uint32_t)((bytesThisExtent + disk->blockSize - 1) / disk->blockSize);
		totalBlocksNeeded += blocksThisExtent;
		extentsNeeded++;
		if (remaining <= bytesThisExtent)
			break;
		remaining -= bytesThisExtent;
	}

	if (freeBlocks + existingBlocks < totalBlocksNeeded)
	{
		Free(used);
		DIM_SetError(DIMError_DiskFull);
		return 0;
	}
	if (CPC_CountFreeDirectoryEntries(disk) + existingEntries < extentsNeeded)
	{
		Free(used);
		DIM_SetError(DIMError_DirectoryFull);
		return 0;
	}

	if (existingEntries != 0)
	{
		if (!CPC_DeleteMatchingEntries(disk, fileName, extension, false))
		{
			Free(used);
			return 0;
		}
		if (!CPC_BuildBlockMap(disk, used, blockCount))
		{
			Free(used);
			return 0;
		}
	}

	uint8_t* allocatedBlocks = NULL;
	if (totalBlocksNeeded != 0)
	{
		allocatedBlocks = Allocate<uint8_t>("CPC allocated blocks", totalBlocksNeeded);
		if (allocatedBlocks == NULL)
		{
			Free(used);
			DIM_SetError(DIMError_OutOfMemory);
			return 0;
		}

		uint32_t allocated = 0;
		for (uint32_t n = disk->directoryBlocks; n < blockCount && allocated < totalBlocksNeeded; n++)
		{
			if (!used[n])
			{
				used[n] = 1;
				allocatedBlocks[allocated++] = (uint8_t)n;
			}
		}
		if (allocated < totalBlocksNeeded)
		{
			Free(allocatedBlocks);
			Free(used);
			DIM_SetError(DIMError_DiskFull);
			return 0;
		}
	}

	uint8_t* blockBuffer = NULL;
	if (totalBlocksNeeded != 0)
	{
		blockBuffer = Allocate<uint8_t>("CPC block buffer", disk->blockSize);
		if (blockBuffer == NULL)
		{
			Free(allocatedBlocks);
			Free(used);
			DIM_SetError(DIMError_OutOfMemory);
			return 0;
		}

		uint32_t dataOffset = 0;
		for (uint32_t block = 0; block < totalBlocksNeeded; block++)
		{
			uint32_t chunk = bufferSize - dataOffset;
			if (chunk > disk->blockSize)
				chunk = disk->blockSize;
			memset(blockBuffer, 0, disk->blockSize);
			if (chunk != 0)
				memcpy(blockBuffer, buffer + dataOffset, chunk);
			if (CPC_WriteBlock(disk, allocatedBlocks[block], blockBuffer, disk->blockSize) != disk->blockSize)
			{
				Free(blockBuffer);
				Free(allocatedBlocks);
				Free(used);
				return 0;
			}
			dataOffset += chunk;
		}
		Free(blockBuffer);
	}

	uint32_t blockCursor = 0;
	remaining = bufferSize;
	for (uint32_t extent = 0; extent < extentsNeeded; extent++)
	{
		uint64_t bytesThisExtent = remaining;
		uint64_t maxBytesThisExtent = (uint64_t)blocksPerExtent * disk->blockSize;
		if (bytesThisExtent > maxBytesThisExtent)
			bytesThisExtent = maxBytesThisExtent;
		uint32_t blocksThisExtent = bytesThisExtent == 0 ? 0 : (uint32_t)((bytesThisExtent + disk->blockSize - 1) / disk->blockSize);
		uint32_t recordsThisExtent = bytesThisExtent == 0 ? 0 : (uint32_t)((bytesThisExtent + 127) / 128);

		uint32_t entryIndex = CPC_FindFreeDirectoryEntry(disk);
		if (entryIndex >= disk->directoryEntries)
		{
			Free(allocatedBlocks);
			Free(used);
			DIM_SetError(DIMError_DirectoryFull);
			return 0;
		}

		CPC_DirEntry* entry = disk->directory + entryIndex;
		memset(entry, 0, sizeof(CPC_DirEntry));
		entry->user = 0;
		memcpy(entry->fileName, fileName, 8);
		memcpy(entry->extension, extension, 3);
		entry->extentLow = (uint8_t)extent;
		entry->recordCount = (uint8_t)recordsThisExtent;
		for (uint32_t i = 0; i < blocksThisExtent; i++)
			entry->allocation[i] = allocatedBlocks[blockCursor++];

		if (remaining > bytesThisExtent)
			remaining -= bytesThisExtent;
		else
			remaining = 0;
	}

	Free(allocatedBlocks);
	Free(used);

	if (!CPC_FlushDirectory(disk))
		return 0;

	return bufferSize;
}

uint32_t CPC_GetBlockCount (CPC_Disk* disk)
{
	uint32_t tracks = disk->tracksPerSide;
	if (disk->doubleSide) tracks *= 2;
	tracks -= disk->reservedTracks;
	return tracks * disk->sectorsPerTrack * disk->sectorSize / disk->blockSize;
}

bool CPC_IsSpectrumDisk (CPC_Disk* disk)
{
	return disk->diskType == CPC_FORMAT_SPECTRUM;
}

uint64_t CPC_GetFreeSpace (CPC_Disk* disk)
{
	uint32_t blocks = CPC_GetBlockCount(disk);
	uint8_t* used = Allocate<uint8_t>("CPC Free Space Map", blocks == 0 ? 1 : blocks);
	if (used == NULL)
		return 0;
	if (!CPC_BuildBlockMap(disk, used, blocks))
	{
		Free(used);
		return 0;
	}

	CPC_DirEntry* entry = disk->directory;

	for (unsigned n = 0; n < disk->directoryEntries; n++, entry++)
	{
		if (entry->user < 16 && entry->fileName[0] != 0)
		{
			for (int i = 0; i < 16; i++)
				if (entry->allocation[i] != 0 && entry->allocation[i] < blocks)
					used[entry->allocation[i]] = 1;
		}
	}
	uint32_t directoryBlocks = disk->directoryEntries * 32 / disk->blockSize;

	unsigned freeBlocks = 0;
	for (unsigned n = 0; n < blocks; n++)
		freeBlocks += !used[n];
	freeBlocks -= directoryBlocks;
	Free(used);
	return freeBlocks * disk->blockSize;
}

void CPC_DumpInfo (CPC_Disk* disk)
{
	uint32_t tracks = disk->tracksPerSide;
	if (disk->doubleSide) tracks *= 2;
	uint32_t totalSize = tracks * disk->sectorSize * disk->sectorsPerTrack;

	printf("Disk size:           %7dK\n", (totalSize + 1023)/1024);
	printf("Disk format:     %12s\n",
	       disk->diskType == CPC_FORMAT_SPECTRUM   ? "Spectrum+3" :
	       disk->diskType == CPC_FORMAT_CPC_DATA   ? "CPC Data" :
	       disk->diskType == CPC_FORMAT_CPC_SYSTEM ? "CPC System" : "PCW DD");
	printf("-----------------------------\n");
	printf("Tracks:              %8d%s\n",
	       disk->tracksPerSide * (disk->doubleSide ? 2 : 1),
	       disk->doubleSide ? " (Double sided)" : "");
	printf("Sectors per track:   %8d\n", disk->sectorsPerTrack);
	printf("Bytes per sector:    %8d\n", disk->sectorSize);
	printf("Bytes per block:     %8d\n", disk->blockSize);
	printf("Total blocks:        %8d\n", CPC_GetBlockCount(disk));
	printf("Directory blocks:    %8d\n", disk->directoryBlocks);
	printf("Free data blocks:    %8u\n", (uint32_t)(CPC_GetFreeSpace(disk) / disk->blockSize));
}

void CPC_CloseDisk (CPC_Disk* disk)
{
	File_Close(disk->file);
	Free(disk->directory);
	Free(disk);
}

#endif
