#ifdef HAS_VIRTUALFILESYSTEM

#include <dim_adf.h>
#include <dim.h>
#include <os_char.h>
#include <os_file.h>
#include <os_bito.h>
#include <os_mem.h>

#include <stdio.h>
#include <string.h>
#include <time.h>

const int64_t AMIGA_EPOCH_OFFSET = 252460800;

static void ADF_FreeBlock(ADF_Disk* disk, uint32_t blockIndex);
static void ADF_UpdateChecksum(uint8_t* block, uint32_t checksumOffset);
static void ADF_UpdateChecksumSized(uint8_t* block, uint32_t blockSize, uint32_t checksumOffset);
static void ADF_UpdateBootBlockChecksum(uint8_t* block, uint32_t blockSize, uint32_t checksumOffset);
static uint32_t ADF_GetFloppyRootBlock(uint32_t numBlocks);

static void fix32(ADF_RootBlock* root)
{
	uint8_t* ptr = (uint8_t*) root;
	uint8_t* nameStart = &root->nameLength;
	uint8_t* nameEnd = nameStart + 31;
	uint8_t* end = ptr + 512;

	while (ptr < end)
	{
		if (!(ptr >= nameStart && ptr < nameEnd))
			*(uint32_t*)ptr = read32(ptr, false);
		ptr += 4;
	}
}

static void fix32(ADF_HeaderBlock* header)
{
	uint8_t* ptr = (uint8_t*) header;
	uint8_t* commentStart = &header->commentLength;
	uint8_t* commentEnd = commentStart + 23;
	uint8_t* nameStart = &header->nameLength;
	uint8_t* nameEnd = nameStart + 31;
	uint8_t* end = ptr + 512;

	while (ptr < end)
	{
		if (!(ptr >= nameStart && ptr < nameEnd) &&
			!(ptr >= commentStart && ptr < commentEnd))
			*(uint32_t*)ptr = read32(ptr, false);
		ptr += 4;
	}
}

static uint32_t ADF_GetFloppyRootBlock(uint32_t numBlocks)
{
    return numBlocks / 2;
}

bool ADF_LoadBitmapFromDisk (ADF_Disk* disk)
{
	if (disk->root.bitmapFlag != -1)
		return false;

	uint32_t data[128];
	uint32_t size = (disk->numBlocks - 2 + 31) / 32;
	uint32_t current = 0;

	if (disk->bitmap == 0)
		disk->bitmap = Allocate<uint32_t>("ADF Disk", size);

	for (int n = 0; n < 25 && current < size && disk->root.bitmapPage[n]; n++)
	{
		uint32_t blockIndex = disk->root.bitmapPage[n];
		if (!File_Seek(disk->file, blockIndex*512) ||
			 File_Read(disk->file, &data, 512) != 512)
			return false;
		
		uint32_t checksum = 0;
		data[0] = fix32(data[0]);
		for (int i = 1; i < 128; i++)
		{
			data[i] = fix32(data[i]);
			checksum -= data[i];
		}
		if (checksum != data[0])
			return false;

		for (int i = 1; i < 128 && current < size; i++)
			disk->bitmap[current++] = data[i];
	}
	return true;
}

uint64_t ADF_GetFreeSpace (ADF_Disk* disk)
{
	if (disk->bitmap == 0 && !ADF_LoadBitmapFromDisk(disk))
		return 0;

	uint64_t count = 0;
	uint32_t mask = 0x00000001;
	uint32_t offset = 0;
    uint32_t totalBitmapBits = (disk->numBlocks - 2);
	for (unsigned i = 0; i < totalBitmapBits; i++)
	{
		if (disk->bitmap[offset] & mask)
			count++;
		mask <<= 1;
		if (mask == 0)
		{
			mask = 0x00000001;
			offset++;
		}
	}
	return count * 512;
}

ADF_Disk* ADF_OpenDisk (const char* filename)
{
	ADF_Disk* disk = Allocate<ADF_Disk>("ADF Disk");
	if (disk == 0)
	{
		DIM_SetError(DIMError_OutOfMemory);
		return 0;
	}

	File* file = File_Open(filename, ReadWrite);
	if (file == 0)
	{
		DIM_SetError(DIMError_FileNotFound);
		Free(disk);
		return 0;
	}
	if (File_Read(file, disk->boot, 1024) != 1024)
	{
		DIM_SetError(DIMError_ReadError);
		File_Close(file);
		Free(disk);
		return 0;
	}

	disk->file = file;
	disk->size = (uint32_t)File_GetSize(file);
	disk->numBlocks = disk->size / 512;
	disk->rootBlock = ADF_GetFloppyRootBlock(disk->numBlocks);
	
	bool ok = false;

	if (memcmp(disk->boot, "DOS", 3) == 0 && disk->boot[3] < 2 &&
		disk->rootBlock < disk->numBlocks)
	{
        uint8_t dosType = disk->boot[3];
        switch (dosType) {
            case 0: disk->fs = ADF_OFS; break;
            case 1: disk->fs = ADF_FFS; break;
            case 2: disk->fs = ADF_OFS; break;  // International OFS
            case 3: disk->fs = ADF_FFS; break;  // International FFS
            default:
                DIM_SetError(DIMError_FormatNotSupported);
    			ADF_CloseDisk(disk);
                return 0;
        }
		File_Seek(file, 512*disk->rootBlock);
		if (File_Read(file, &disk->root, 512) != 512)
		{
			DIM_SetError(DIMError_ReadError);
			ADF_CloseDisk(disk);
			return 0;
		}

		fix32(&disk->root);

		memcpy(disk->cwd[0].hashTable, disk->root.hashTable, 72*4);
		disk->cwd[0].hashTableEntries = disk->root.hashTableEntries;
		strcpy(disk->cwd[0].name, "");
		disk->cwd[0].block = disk->rootBlock;
		disk->cwdIndex = 0;

		disk->hashTableSize = disk->root.hashTableEntries;

		ok = disk->hashTableSize <= 72;
	}

	if (!ok)
	{
		DIM_SetError(DIMError_InvalidDisk);
		ADF_CloseDisk(disk);
		return 0;
	}
	return disk;
}

void ADF_CloseDisk (ADF_Disk* disk)
{
	if (disk->bitmap)
		Free(disk->bitmap);
	File_Close(disk->file);
	Free(disk);
}

bool ADF_FindFile (ADF_Disk* disk, ADF_FindResults* result, const char* name)
{
	return ADF_FindFirstFile(disk, result, name);
}

bool ADF_FindFirstFile (ADF_Disk* disk, ADF_FindResults* result, const char* pattern)
{
	result->nextBlock = ADF_INVALID_BLOCK;
	result->hashIndex = -1;
	result->block     = ADF_INVALID_BLOCK;
	
	if (pattern == NULL)
		pattern = "*";
	strncpy(result->pattern, pattern, 30);
	result->pattern[30] = 0;
	result->patternLen = (uint8_t)strlen(result->pattern);
	
	return ADF_FindNextFile(disk, result);
}

bool ADF_FindNextFile (ADF_Disk* disk, ADF_FindResults* result)
{
	while(true)
	{
		if (result->nextBlock == ADF_INVALID_BLOCK)
		{
			do
			{
				result->hashIndex++;
				if (result->hashIndex >= (int)disk->cwd[disk->cwdIndex].hashTableEntries)
				{
					DIM_SetError(DIMError_FileNotFound);
					return false;
				}
				result->nextBlock = disk->cwd[disk->cwdIndex].hashTable[result->hashIndex];
			}
			while (result->nextBlock == 0);
		}
		
		uint32_t blockIndex = result->nextBlock;
		if (blockIndex == 0)
			continue;
		if (blockIndex >= disk->numBlocks)
		{
			DIM_SetError(DIMError_InvalidDisk);
			return false;
		}

		File_Seek(disk->file, 512*blockIndex);
		if (File_Read(disk->file, &disk->block, 512) != 512)
		{
			DIM_SetError(DIMError_ReadError);
			return false;
		}
		fix32(&disk->block);

		if (disk->block.type != 2 || disk->block.headerKey != blockIndex)
		{
			DIM_SetError(DIMError_InvalidDisk);
			return false;
		}

		uint8_t len = disk->block.nameLength;
		if (len > 30) len = 30;
		strncpy(result->fileName, disk->block.name, len);
		result->fileName[len] = 0;
		result->fileSize = disk->block.fileSize;
		result->block = blockIndex;
		result->nextBlock = disk->block.hashChain;
		result->directory = (disk->block.secondaryType == 2);
		result->days = disk->block.aDays;
		result->minutes = disk->block.aMinutes;
		result->ticks = disk->block.aTicks;

		if (!DIM_MatchWildcards(result->fileName, len, result->pattern, result->patternLen))
			continue;

		return true;
	}
}

uint32_t ADF_ReadFile (ADF_Disk* disk, const char* path, uint8_t* buffer, uint32_t size)
{
	ADF_FindResults r;
	if (!ADF_FindFile(disk, &r, path))
		return false;

	uint8_t  data[512];
	uint8_t  headerSize = disk->fs == ADF_OFS ? 24 : 0;
	uint16_t dataSize = 512 - headerSize;

    if (disk->fs == ADF_OFS)
    {
        uint32_t block = disk->block.firstData;
        uint32_t total = 0;
        while (total < size)
        {
            if (block == 0 || block >= disk->numBlocks)
            {
                DIM_SetError(DIMError_ReadError);
                return total;
            }
            if (!File_Seek(disk->file, block * 512))
                return total;
            if (File_Read(disk->file, &data, 512) != 512)
                return total;

            if (read32(data, false) != ADF_DATA_TYPE)
            {
                DIM_SetError(DIMError_InvalidDisk);
                return total;
            }

            uint32_t blockDataSize = read32(data + 12, false);
            if (blockDataSize > dataSize)
            {
                DIM_SetError(DIMError_InvalidDisk);
                return total;
            }

            uint32_t remaining = size - total;
            uint32_t length = remaining > blockDataSize ? blockDataSize : remaining;
            memcpy(buffer, data + headerSize, length);
            total += length;
            buffer += length;

            if (total >= size)
                break;

            block = read32(data + 16, false);
        }
        return total;
    }
    else
    {
        uint32_t block = disk->block.firstData;
        uint32_t index = disk->block.highSeq;
        uint32_t total = 0;
        if (disk->block.dataBlocks[index] != block)
        {
            DIM_SetError(DIMError_ReadError);
            return total;
        }
        while (total < size)
        {
            if (!File_Seek(disk->file, block*512))
                return total;
            if (File_Read(disk->file, &data, 512) != 512)
                return total;

            uint32_t remaining = size-total;
            uint32_t length = remaining > dataSize ? dataSize : remaining;
            memcpy(buffer, data+headerSize, length);
            total += length;
            buffer += length;
            if (total >= size)
                break;

            if (index-- == 0)
            {
                index = 71;
                if (disk->block.extension == 0)
                {
                    DIM_SetError(DIMError_ReadError);
                    return total;
                }
                if (!File_Seek(disk->file, disk->block.extension*512))
                    return total;
                if (File_Read(disk->file, &disk->block, 512) != 512)
                    return total;
                fix32(&disk->block);
            }
            block = disk->block.dataBlocks[index];
        }
        return total;
    }
}

uint32_t ADF_GetVolumeLabel(ADF_Disk* disk, char* buffer, uint32_t bufferSize)
{
	if (bufferSize == 0)
		return 0;
	if (disk->root.nameLength == 0 ||
		disk->root.name[0] == ' ' ||
		disk->root.name[0] == 0)
		return 0;

	uint32_t len = disk->root.nameLength;
	if (len > 30) len = 30;
	if (len > bufferSize-1) len = bufferSize-1;
	memcpy(buffer, disk->root.name, len);
	buffer[len] = 0;
	return len;
}

bool ADF_SetVolumeLabel(ADF_Disk* disk, const char* label)
{
	// Calculate the label length (max 30 characters)
	uint32_t len = 0;
	if (label != NULL)
	{
		while (len < 30 && label[len] != 0)
			len++;
	}
	
	// Update the root block in memory
	disk->root.nameLength = (uint8_t)len;
	memset(disk->root.name, 0, 30);
	if (len > 0)
		memcpy(disk->root.name, label, len);
	
	// Create a temporary copy for writing
	ADF_RootBlock temp = disk->root;
	fix32(&temp); // Convert to Big Endian

	// Update checksum on the Big Endian block
	ADF_UpdateChecksum((uint8_t*)&temp, 20);
	
	// Write the root block back to disk
	File_Seek(disk->file, disk->rootBlock * 512);
	if (File_Write(disk->file, &temp, 512) != 512)
	{
		DIM_SetError(DIMError_WriteError);
		return false;
	}
	
	return true;
}

static void ADF_GetCurrentAmigaTime(uint32_t* days, uint32_t* minutes, uint32_t* ticks)
{
    time_t   t = time(NULL);
    int64_t  amigaSeconds = (int64_t)t < AMIGA_EPOCH_OFFSET ? 0 : ((int64_t)t - AMIGA_EPOCH_OFFSET);
    uint32_t secondsInDay = (uint32_t)(amigaSeconds % 86400);
    uint32_t seconds = secondsInDay % 60;

    *days    = (uint32_t)(amigaSeconds / 86400);
    *minutes = secondsInDay / 60;
    *ticks   = seconds * 50;
}

static time_t ADF_AmigaDateToUnix(uint32_t days, uint32_t minutes, uint32_t ticks)
{
    int64_t seconds =
        days * 86400LL +
        minutes * 60LL +
        ticks / 50;

    return seconds + AMIGA_EPOCH_OFFSET;
}

void ADF_DumpInfo (ADF_Disk* disk)
{
	time_t     tc, ta;
	struct tm  ts;
	char       creation[80];
	char       access[80];

	// Get current time
    tc = ADF_AmigaDateToUnix(disk->root.cDays, disk->root.cMinutes, disk->root.cTicks);
	ts = *localtime(&tc);
	strftime(creation, sizeof(creation), "%Y-%m-%d", &ts);
	ta = ADF_AmigaDateToUnix(disk->root.aDays, disk->root.aMinutes, disk->root.aTicks);
	ts = *localtime(&ta);
	strftime(access, sizeof(access), "%Y-%m-%d", &ts);

	printf("%.*s\n\n", disk->root.nameLength, disk->root.name);
	printf("Disk size:           %7dK\n", (disk->size + 1023) / 1024);
	printf("Creation:    %16s\n", creation);
	printf("Access:      %16s\n", access);
	printf("-----------------------------\n");
	printf("Total blocks:        %8d\n", disk->numBlocks);
	printf("Free blocks:         %8d\n", (uint32_t)(ADF_GetFreeSpace(disk) / 512));
	printf("Root block index:    %8d\n", disk->rootBlock);
}

uint32_t ADF_GetCWD (ADF_Disk* disk, char* buffer, uint32_t bufferSize)
{
	if (bufferSize == 0)
		return 0;

	char* end = buffer + bufferSize - 1;
	char* out = buffer;
	*out++ = '\\';

	for (int n = 1; n <= disk->cwdIndex && out < end; n++)
	{
		const char* in = disk->cwd[n].name;
		while (out < end && *in)
			*out++ = *in++;
		if (out < end && n < disk->cwdIndex-1)
			*out++ = '\\';
	}
	*out = 0;
	return (uint32_t)(out - buffer);
}

bool ADF_ChangeDirectory (ADF_Disk* disk, const char* name)
{
    if (name == NULL || name[0] == 0)
        return true;

    const char* ptr = name;
    char component[64];

    if (*ptr == '/' || *ptr == '\\')
    {
        disk->cwdIndex = 0;
        ptr++;
    }

    while (*ptr)
    {
        const char* sep = ptr;
        while (*sep && *sep != '/' && *sep != '\\')
            sep++;
        size_t len = sep - ptr;
        if (len >= sizeof(component))
        {
            DIM_SetError(DIMError_CommandNotSupported);
            return false;
        }
        memcpy(component, ptr, len);
        component[len] = 0;

        if (strcmp(component, "") == 0 || strcmp(component, ".") == 0)
        {
            // no-op
        }
        else if (strcmp(component, "..") == 0)
        {
            if (disk->cwdIndex > 0)
                disk->cwdIndex--;
        }
        else
        {
            ADF_FindResults r;
            if (!ADF_FindFile(disk, &r, component))
                return false;
            if (!r.directory)
            {
                DIM_SetError(DIMError_NotADirectory);
                return false;
            }
            if (disk->cwdIndex >= ADF_MAX_DEPTH-1)
            {
                DIM_SetError(DIMError_CommandNotSupported);
                return false;
            }
            int index = ++disk->cwdIndex;
            memcpy(disk->cwd[index].name, r.fileName, 32);
            disk->cwd[index].block = r.block;
            disk->cwd[index].hashTableEntries = 72;
            memcpy(disk->cwd[index].hashTable, disk->block.dataBlocks, 72*4);
        }

        ptr = (*sep) ? sep + 1 : sep;
    }

    return true;
}

// ---------------------------------------------------------------------------
//  Write support helpers
// ---------------------------------------------------------------------------

static void ADF_UpdateChecksumSized(uint8_t* block, uint32_t blockSize, uint32_t checksumOffset)
{
    uint32_t sum = 0;
    uint32_t* ptr = (uint32_t*)block;
    uint32_t count = blockSize / 4;
    ptr[checksumOffset / 4] = 0;
    for (uint32_t i = 0; i < count; i++)
        sum += fix32(ptr[i]);
    ptr[checksumOffset / 4] = fix32(-sum);
}

static void ADF_UpdateChecksum(uint8_t* block, uint32_t checksumOffset)
{
    ADF_UpdateChecksumSized(block, 512, checksumOffset);
}

static void ADF_UpdateBootBlockChecksum(uint8_t* block, uint32_t blockSize, uint32_t checksumOffset)
{
    uint32_t sum = 0;
    uint32_t* ptr = (uint32_t*)block;
    uint32_t count = blockSize / 4;

    ptr[checksumOffset / 4] = 0;
    for (uint32_t i = 0; i < count; i++)
    {
        uint32_t value = fix32(ptr[i]);
        uint32_t previous = sum;
        sum += value;
        if (sum < previous)
            sum++;
    }
    ptr[checksumOffset / 4] = fix32(~sum);
}

// Standard Amiga Int'l Hash function
static uint32_t ADF_HashString(const char* name, int hashTableSize)
{
    uint32_t hash = strlen(name);
    for(int i=0; name[i]; i++)
    {
        hash = hash * 13 + ToUpper(name[i]);
        hash &= 0x7ff;
    }
    return hash % hashTableSize;
}

// Allocates the first free block searching upward from preferredBlock and
// wrapping around. File and directory headers are allocated near the root
// block: OFS directory scans and hash lookups read only the root and header
// blocks, so keeping them on the root cylinder turns a startup full of
// long floppy seeks into a couple of track reads. Data allocations avoid a
// small window after the root so late-added files still get nearby headers
// (the window is released when the rest of the disk is full).
#define ADF_HEADER_ZONE_BLOCKS 32

static uint32_t ADF_AllocBlockInternal(ADF_Disk* disk, uint32_t preferredBlock, bool avoidHeaderZone)
{
    if (disk->bitmap == 0 && !ADF_LoadBitmapFromDisk(disk))
        return ADF_INVALID_BLOCK;

    uint32_t totalBlocks = disk->numBlocks - 2; // Exclude bootblocks
    uint32_t mapSize = (totalBlocks + 31) / 32;
    uint32_t zoneStart = disk->rootBlock + 1;
    uint32_t zoneEnd = zoneStart + ADF_HEADER_ZONE_BLOCKS - 1;
    uint32_t startWord = 0;
    if (preferredBlock >= 2 && preferredBlock < disk->numBlocks)
        startWord = (preferredBlock - 2) / 32;

    for (uint32_t n = 0; n < mapSize; n++)
    {
        uint32_t i = (startWord + n) % mapSize;
        if (disk->bitmap[i] != 0)
        {
            uint32_t mask = 1;
            for (int bit = 0; bit < 32; bit++)
            {
                if (disk->bitmap[i] & mask)
                {
                    uint32_t blockIndex = 2 + (i * 32) + bit; // +2 for bootblocks
                    if (avoidHeaderZone && blockIndex >= zoneStart && blockIndex <= zoneEnd)
                    {
                        mask <<= 1;
                        continue;
                    }
                    disk->bitmap[i] &= ~mask;
                    
                    // We must write the modified bitmap block back to disk immediately
                    // to prevent corruption if we crash later.
                    // Calculate which bitmap page this belongs to.
                    // (Assuming standard root block bitmap pointers for simplicity)
                    // A bitmap block covers 127 longs * 32 = 4064 blocks.
                    
                    // Note: This simplified write-back assumes standard floppy layout
                    // where bitmap pages are stored in the root block list.
                    
                    uint32_t wordsPerBlock = 127;
                    uint32_t bitmapBlockIndex = i / wordsPerBlock;
                    uint32_t offsetInBlock = i % wordsPerBlock;
                    
                    if (bitmapBlockIndex < 25)
                    {
                        uint32_t physBlock = disk->root.bitmapPage[bitmapBlockIndex];
                        uint32_t buffer[128];
                        
                        // Read, Modify, Write
                        if (File_Seek(disk->file, physBlock * 512) && 
                            File_Read(disk->file, buffer, 512) == 512)
                        {
                            // Fix endianness for processing
                            for(int k=0; k<128; k++) buffer[k] = fix32(buffer[k]);
                            
                            // Update the specific word
                            buffer[offsetInBlock + 1] = disk->bitmap[i]; // +1 because first long is checksum
                            
                            // Re-checksum
                            buffer[0] = 0; // Clear checksum
                            uint32_t sum = 0;
                            for(int k=0; k<128; k++) {
                                // Convert back to BE for calculation/writing
                                buffer[k] = fix32(buffer[k]); 
                                sum += fix32(buffer[k]); // Sum logical values
                            }
                            // The sum loop above converted everything to BE. 
                            // Now set checksum.
                            buffer[0] = fix32(-sum); 
                            
                            File_Seek(disk->file, physBlock * 512);
                            File_Write(disk->file, buffer, 512);
                        }
                    }
                    return blockIndex;
                }
                mask <<= 1;
            }
        }
    }
    DIM_SetError(DIMError_DiskFull);
    return ADF_INVALID_BLOCK;
}

static uint32_t ADF_AllocBlockFrom(ADF_Disk* disk, uint32_t preferredBlock)
{
    return ADF_AllocBlockInternal(disk, preferredBlock, false);
}

static uint32_t ADF_AllocBlock(ADF_Disk* disk)
{
    uint32_t block = ADF_AllocBlockInternal(disk, 2, true);
    if (block == ADF_INVALID_BLOCK)
        block = ADF_AllocBlockInternal(disk, 2, false);
    return block;
}

static void ADF_FreeBlock(ADF_Disk* disk, uint32_t blockIndex)
{
    // 1. Validate
    if (blockIndex < 2 || blockIndex >= disk->numBlocks) return;

    // 2. Calculate Bitmap Position
    // The boot blocks (0,1) are not in the bitmap usually, but the bitmap index 
    // is 0-based relative to the disk start.
    uint32_t mapIndex = blockIndex - 2;
    uint32_t wordIndex = mapIndex / 32;
    uint32_t bitIndex  = mapIndex % 32;

    // 3. Update Memory Copy (if loaded)
    if (disk->bitmap)
    {
        disk->bitmap[wordIndex] |= (1 << bitIndex);
    }

    // 4. Update Disk (Read-Modify-Write)
    // We must find which physical block holds this bitmap word.
    // Standard OFS/FFS stores bitmap pointers in the Root Block (pages).
    // Each bitmap block holds 127 uint32s of data (minus checksum).
    
    uint32_t longsPerBlock = 127;
    uint32_t pageIndex = wordIndex / longsPerBlock;
    uint32_t offsetInPage = wordIndex % longsPerBlock;

    if (pageIndex < 25 && disk->root.bitmapPage[pageIndex] != 0)
    {
        uint32_t bitmapBlockNum = disk->root.bitmapPage[pageIndex];
        uint32_t buffer[128];

        if (File_Seek(disk->file, bitmapBlockNum * 512) &&
            File_Read(disk->file, buffer, 512) == 512)
        {
            // The buffer is Big Endian. We need to handle it carefully.
            // Data starts at index 1 (index 0 is checksum).
            uint32_t* dataPtr = &buffer[1];
            
            // Read the specific word, fix endianness, set bit
            uint32_t val = fix32(dataPtr[offsetInPage]);
            val |= (1 << bitIndex);
            
            // Write back fixed
            dataPtr[offsetInPage] = fix32(val);

            // Recalculate Checksum for the whole block
            ADF_UpdateChecksum((uint8_t*)buffer, 0);

            File_Seek(disk->file, bitmapBlockNum * 512);
            File_Write(disk->file, buffer, 512);
        }
    }
}

// Detaches a block (file or dir) from its parent's hash chain
static bool ADF_UnlinkFromParent(ADF_Disk* disk, uint32_t parentBlock, uint32_t targetBlock, const char* name)
{
    // 1. Read Parent
    ADF_HeaderBlock parent;
    if (!File_Seek(disk->file, parentBlock * 512) || 
        File_Read(disk->file, &parent, 512) != 512) return false;
    
    // 2. Calculate Hash
    uint32_t hash = ADF_HashString(name, 72);
    
    // 3. Check Head of Chain
    uint32_t headBlock = fix32(parent.dataBlocks[hash]);

    if (headBlock == targetBlock)
    {
        // Target is the first link.
        // We need to fetch the target to find *its* next link (hashChain).
        ADF_HeaderBlock target;
        File_Seek(disk->file, targetBlock * 512);
        File_Read(disk->file, &target, 512);
        
        // Update Parent: parent.hashTable[hash] = target.hashChain
        parent.dataBlocks[hash] = target.hashChain; // target.hashChain is already BE, safe to copy directly
        
        // Write Parent
        ADF_UpdateChecksum((uint8_t*)&parent, 20);
        File_Seek(disk->file, parentBlock * 512);
        File_Write(disk->file, &parent, 512);

        // Update Cache
        for (int i = 0; i <= disk->cwdIndex; i++)
        {
            if (disk->cwd[i].block == parentBlock)
            {
                disk->cwd[i].hashTable[hash] = fix32(target.hashChain); // BE -> Native
            }
        }
        return true;
    }

    // 4. Traverse Chain (Collision)
    uint32_t currentBlockIdx = headBlock;
    while(currentBlockIdx != 0)
    {
        ADF_HeaderBlock current;
        if (!File_Seek(disk->file, currentBlockIdx * 512) ||
            File_Read(disk->file, &current, 512) != 512) return false;
        
        uint32_t nextBlockIdx = fix32(current.hashChain);
        
        if (nextBlockIdx == targetBlock)
        {
            ADF_HeaderBlock target;
            File_Seek(disk->file, targetBlock * 512);
            File_Read(disk->file, &target, 512);

            // Link Predecessor -> Successor
            current.hashChain = target.hashChain; // Copy BE to BE
            
            // Write Predecessor
            ADF_UpdateChecksum((uint8_t*)&current, 20);
            File_Seek(disk->file, currentBlockIdx * 512);
            File_Write(disk->file, &current, 512);
            return true;
        }
        
        currentBlockIdx = nextBlockIdx;
    }

    return false; // Not found in chain
}

static void ADF_MarkBlockUsed(uint32_t* bitmapData, uint32_t physBlock)
{
    if (physBlock < 2) return; // Bootblocks not in bitmap
    uint32_t mapIndex = physBlock - 2;
    uint32_t wordIdx  = 1 + (mapIndex / 32); // +1 because word 0 is checksum
    uint32_t bitIdx   = mapIndex % 32;
    bitmapData[wordIdx] &= ~(1 << bitIdx);
}

static bool ADF_UpdateOFSNextData(ADF_Disk* disk, uint32_t blockIndex, uint32_t nextBlock)
{
    uint8_t buffer[512];
    if (!File_Seek(disk->file, blockIndex * 512) ||
        File_Read(disk->file, buffer, 512) != 512)
    {
        DIM_SetError(DIMError_ReadError);
        return false;
    }

    write32(buffer + 16, nextBlock, false);
    ADF_UpdateChecksum(buffer, 20);

    if (!File_Seek(disk->file, blockIndex * 512) ||
        File_Write(disk->file, buffer, 512) != 512)
    {
        DIM_SetError(DIMError_WriteError);
        return false;
    }
    return true;
}

// ---------------------------------------------------------------------------
//  Write support API
// ---------------------------------------------------------------------------

ADF_Disk* ADF_CreateDisk(const char* filename, uint32_t size)
{
    if (size == 0) size = 901120; // Default to 880KB (standard DD)
    
    File* f = File_Create(filename); 
    if (!f)
    { 
        DIM_SetError(DIMError_CreatingFile);
        return 0;
    }

    // 1. Setup Layout
    uint32_t numBlocks   = size / 512;
    uint32_t rootBlock   = ADF_GetFloppyRootBlock(numBlocks);
    uint32_t bitmapBlock = rootBlock + 1; // 881

    // 2. Erase Disk (Write Zeros)
    // Writing a blank track buffer is faster than single sectors
    uint8_t buffer[512];
    memset(buffer, 0, 512);
    for (uint32_t i = 0; i < numBlocks; i++)
    {
        File_Write(f, buffer, 512);
    }

    // 3. Create Boot Block (Block 0)
    memset(buffer, 0, 512);
    buffer[0] = 'D'; buffer[1] = 'O'; buffer[2] = 'S'; buffer[3] = 0; // DOS0 (OFS)

    // CRITICAL FIX: Write the Root Block pointer at offset 8 (Big Endian)
    // ADF_OpenDisk reads this to find the filesystem.
    uint32_t* bootVals = (uint32_t*)buffer;
    bootVals[2] = fix32(880); // Floppy bootblock field is 880 for both DD and HD

    uint8_t bootBlock[1024];
    memset(bootBlock, 0, sizeof(bootBlock));
    memcpy(bootBlock, buffer, 512);
    ADF_UpdateBootBlockChecksum(bootBlock, sizeof(bootBlock), 4);

    File_Seek(f, 0);
    File_Write(f, bootBlock, sizeof(bootBlock));

    // 4. Create Root Block
    ADF_RootBlock root;
    memset(&root, 0, sizeof(root));

    root.type = fix32(ADF_ROOT_TYPE);
    root.headerKey = 0;
    root.highSeq = 0;
    root.hashTableEntries = fix32(72);
    root.bitmapFlag = fix32(-1); // VALID (-1)
    root.bitmapPage[0] = fix32(bitmapBlock); // Pointer to bitmap block
    
    // Dates
    uint32_t d, m, t;
    ADF_GetCurrentAmigaTime(&d, &m, &t);

    // Set Creation Date
    root.cDays    = fix32(d);
    root.cMinutes = fix32(m);
    root.cTicks   = fix32(t);

    // Set Last Access Date (same as creation for new disk)
    root.aDays    = fix32(d);
    root.aMinutes = fix32(m);
    root.aTicks   = fix32(t);

    root.nameLength = 5;
    strcpy(root.name, "EMPTY");

    // Set Secondary Type (Must be ST_ROOT for validity)
    root.blockSecondaryType = fix32(ADF_ST_ROOT);

    // Write Root
    ADF_UpdateChecksum((uint8_t*)&root, 20);
    File_Seek(f, rootBlock * 512);
    File_Write(f, &root, 512);

    // 5. Create Bitmap Block
    uint32_t bitmapData[128];
    
    // Fill with 1s (FREE)
    // Note: We fill 128 longs. 128*32 = 4096 bits. 
    // Disk is 1760 blocks. We have plenty of spare bits (ignored).
    for(int i=0; i<128; i++) bitmapData[i] = 0xFFFFFFFF;
    
    // Helper to Mark Block as USED (0)
    // Amiga Bitmap: Bit 0 of Long 0 is Block 2 (since Block 0/1 are Boot)
    ADF_MarkBlockUsed(bitmapData, rootBlock);
    ADF_MarkBlockUsed(bitmapData, bitmapBlock);

    // Swap to Big Endian for Disk Storage
    for(int i=1; i<128; i++) 
    {
        bitmapData[i] = fix32(bitmapData[i]);
    }

    // Checksum (Calculates over the Big Endian data)
    ADF_UpdateChecksum((uint8_t*)bitmapData, 0);

    File_Seek(f, bitmapBlock * 512);
    File_Write(f, bitmapData, 512);

    File_Close(f);

    // 6. Return handle
    return ADF_OpenDisk(filename);
}

bool ADF_ReadBootBlock(ADF_Disk* disk, void* buffer, uint32_t size)
{
    if (size < sizeof(disk->boot))
    {
        DIM_SetError(DIMError_InvalidFile);
        return false;
    }
    memcpy(buffer, disk->boot, sizeof(disk->boot));
    return true;
}

bool ADF_WriteBootBlock(ADF_Disk* disk, const void* buffer, uint32_t size)
{
    if (size < sizeof(disk->boot))
    {
        DIM_SetError(DIMError_InvalidFile);
        return false;
    }

    uint8_t boot[1024];
    memcpy(boot, buffer, sizeof(boot));
    write32(&boot[8], 880, false);
    ADF_UpdateBootBlockChecksum(boot, sizeof(boot), 4);

    if (!File_Seek(disk->file, 0) || File_Write(disk->file, boot, sizeof(boot)) != sizeof(boot))
    {
        DIM_SetError(DIMError_WriteError);
        return false;
    }

    memcpy(disk->boot, boot, sizeof(disk->boot));
    return true;
}

static bool ADF_LinkToParent(ADF_Disk* disk, uint32_t parentBlock, uint32_t childBlock, const char* name)
{
    ADF_HeaderBlock parent;
    if (!File_Seek(disk->file, parentBlock * 512) || 
        File_Read(disk->file, &parent, 512) != 512) return false;
    
    uint32_t hash = ADF_HashString(name, 72);
    uint32_t currentPtr = fix32(parent.dataBlocks[hash]); 

    if (currentPtr == 0)
    {
        // Empty slot, insert here
        parent.dataBlocks[hash] = fix32(childBlock);
        
        // Update Cache
        for (int i = 0; i <= disk->cwdIndex; i++)
        {
            if (disk->cwd[i].block == parentBlock)
            {
                disk->cwd[i].hashTable[hash] = childBlock; // Native
            }
        }
    }
    else
    {
        // Collision: Traverse to end of chain
        uint32_t currentBlockIdx = currentPtr;
        while(true)
        {
            ADF_HeaderBlock currBlock;
            File_Seek(disk->file, currentBlockIdx * 512);
            File_Read(disk->file, &currBlock, 512);
            
            // The 'hashChain' field is at the same offset for Files and Dirs
            // so we can safely read it from ADF_HeaderBlock
            uint32_t nextBlockIdx = fix32(currBlock.hashChain);
            
            if (nextBlockIdx == 0)
            {
                // Found end of chain
                currBlock.hashChain = fix32(childBlock);
                
                // Write back the modified link
                ADF_UpdateChecksum((uint8_t*)&currBlock, 20);
                File_Seek(disk->file, currentBlockIdx * 512);
                File_Write(disk->file, &currBlock, 512);
                break;
            }
            currentBlockIdx = nextBlockIdx;
        }
    }

    // Write Parent
    ADF_UpdateChecksum((uint8_t*)&parent, 20);
    File_Seek(disk->file, parentBlock * 512);
    File_Write(disk->file, &parent, 512);    
    return true;
}

bool ADF_MakeDirectory(ADF_Disk* disk, const char* path)
{
    // 1. Check if exists
    ADF_FindResults r;
    if (ADF_FindFile(disk, &r, path)) 
    {
        DIM_SetError(DIMError_FileExists);
        return false;
    }

    // 2. Resolve Parent
    uint32_t parentBlock = disk->cwd[disk->cwdIndex].block;
    const char* newName = path;

    // 3. Allocate Block
    uint32_t newBlock = ADF_AllocBlockFrom(disk, disk->rootBlock + 1);
    if (newBlock == ADF_INVALID_BLOCK)
    {
        DIM_SetError(DIMError_DiskFull);
        return false;
    }

    // 4. Create Header (in NATIVE endianness)
    ADF_HeaderBlock dir;
    memset(&dir, 0, 512);
    
    dir.type = ADF_DIR_TYPE;                      
    dir.headerKey = newBlock;                     
    dir.protectionFlags = 0;
    dir.nameLength = strlen(newName) > 30 ? 30 : strlen(newName);
    strncpy(dir.name, newName, dir.nameLength);
    dir.parent = parentBlock;                     
    dir.secondaryType = ADF_ST_DIR;               
    ADF_GetCurrentAmigaTime(&dir.aDays, &dir.aMinutes, &dir.aTicks);

    fix32(&dir);
    ADF_UpdateChecksum((uint8_t*)&dir, 20);
    
    // Write to disk
    File_Seek(disk->file, newBlock * 512);
    File_Write(disk->file, &dir, 512);

    // 5. Link to Parent
    return ADF_LinkToParent(disk, parentBlock, newBlock, newName);
}

bool ADF_WriteFile(ADF_Disk* disk, const char* path, const void* data, uint32_t dataSize)
{
    ADF_FindResults r;
    if (ADF_FindFile(disk, &r, path))
    {
        ADF_RemoveFile(disk, path);
    }

    uint32_t headerBlockIndex = ADF_AllocBlockFrom(disk, disk->rootBlock + 1);
    if (headerBlockIndex == ADF_INVALID_BLOCK)
    {
        DIM_SetError(DIMError_DiskFull);
        return false;
    }

    ADF_HeaderBlock mainHeader;
    ADF_HeaderBlock extBlock;
    memset(&mainHeader, 0, sizeof(mainHeader));
    memset(&extBlock, 0, sizeof(extBlock));

    ADF_HeaderBlock* currentBlock = &mainHeader;
    uint32_t currentBlockIndex = headerBlockIndex;
    int currentEntries = 0;

    const uint8_t* byteData = (const uint8_t*)data;
    uint32_t remaining = dataSize;
    int tableIndex = 71; 
    
    uint32_t dataPayloadSize = (disk->fs == ADF_OFS) ? 488 : 512;
    uint32_t headerSize      = (disk->fs == ADF_OFS) ? 24  : 0;
    uint32_t ofsSequence     = 1;
    uint32_t previousOfsDataBlock = 0;

    while (remaining > 0)
    {
        uint32_t dataBlockIndex = ADF_AllocBlock(disk);
        if (dataBlockIndex == ADF_INVALID_BLOCK) 
        {
            DIM_SetError(DIMError_DiskFull);
            return false; 
        }

        if (disk->fs == ADF_OFS && previousOfsDataBlock != 0)
        {
            if (!ADF_UpdateOFSNextData(disk, previousOfsDataBlock, dataBlockIndex))
            {
                ADF_FreeBlock(disk, dataBlockIndex);
                return false;
            }
        }

        if (currentBlock->firstData == 0) 
            currentBlock->firstData = dataBlockIndex; 
        
        currentBlock->dataBlocks[tableIndex] = dataBlockIndex;
        tableIndex--;
        currentEntries++;

        if (tableIndex < 0)
        {
            currentBlock->highSeq = currentEntries;
            uint32_t newExtIndex = ADF_AllocBlock(disk);
            if (newExtIndex == ADF_INVALID_BLOCK)
            {
                DIM_SetError(DIMError_DiskFull);
                return false;
            }

            currentBlock->extension = newExtIndex;
            if (currentBlock != &mainHeader)
            {
                currentBlock->type = ADF_EXT_TYPE; 
                currentBlock->headerKey = headerBlockIndex;
                currentBlock->parent = headerBlockIndex;
                currentBlock->secondaryType = ADF_ST_FILE;

                ADF_HeaderBlock tempWrite = *currentBlock;
                fix32(&tempWrite); 
                ADF_UpdateChecksum((uint8_t*)&tempWrite, 20);
                File_Seek(disk->file, currentBlockIndex * 512);
                File_Write(disk->file, &tempWrite, 512);
            }

            memset(&extBlock, 0, sizeof(extBlock));
            currentBlock = &extBlock;
            currentBlockIndex = newExtIndex;
            tableIndex = 71; 
            currentEntries = 0;
        }

        uint32_t writeSize = remaining > dataPayloadSize ? dataPayloadSize : remaining;
        uint8_t buffer[512];
        memset(buffer, 0, 512); 

        if (disk->fs == ADF_OFS)
        {
            write32(buffer + 0, ADF_DATA_TYPE, false);
            write32(buffer + 4, headerBlockIndex, false);
            write32(buffer + 8, ofsSequence, false);
            write32(buffer + 12, writeSize, false);
            write32(buffer + 16, 0, false); // next data filled when we know the block
            write32(buffer + 20, 0, false);
        }
        memcpy(buffer + headerSize, byteData, writeSize);
        if (disk->fs == ADF_OFS)
            ADF_UpdateChecksum(buffer, 20);
        
        File_Seek(disk->file, dataBlockIndex * 512);
        File_Write(disk->file, buffer, 512);
        
        byteData += writeSize;
        remaining -= writeSize;

        if (disk->fs == ADF_OFS)
        {
            previousOfsDataBlock = dataBlockIndex;
            ofsSequence++;
        }
    }

    if (currentBlock != &mainHeader)
    {
        currentBlock->type = ADF_EXT_TYPE;
        currentBlock->headerKey = headerBlockIndex; 
        currentBlock->highSeq = currentEntries;
        currentBlock->secondaryType = ADF_ST_FILE;
        
        fix32(currentBlock); 
        ADF_UpdateChecksum((uint8_t*)currentBlock, 20);
        File_Seek(disk->file, currentBlockIndex * 512);
        File_Write(disk->file, currentBlock, 512);
    }

    mainHeader.type = 2;
    mainHeader.secondaryType = ADF_ST_FILE;
    mainHeader.headerKey = headerBlockIndex;
    mainHeader.highSeq = currentBlock == &mainHeader ? currentEntries : 72;
    mainHeader.dataSize = 0; 
    mainHeader.protectionFlags = 0;
    mainHeader.fileSize = dataSize;
    mainHeader.parent = disk->cwd[disk->cwdIndex].block;
    
    const char* filename = path;
    mainHeader.nameLength = strlen(filename) > 30 ? 30 : strlen(filename);
    strncpy(mainHeader.name, filename, mainHeader.nameLength);
    
    uint32_t d, m, t;
    ADF_GetCurrentAmigaTime(&d, &m, &t);
    mainHeader.aDays    = d; 
    mainHeader.aMinutes = m; 
    mainHeader.aTicks   = t;

    fix32(&mainHeader);
    ADF_UpdateChecksum((uint8_t*)&mainHeader, 20);
    
    File_Seek(disk->file, headerBlockIndex * 512);
    File_Write(disk->file, &mainHeader, 512);
    return ADF_LinkToParent(disk, disk->cwd[disk->cwdIndex].block, headerBlockIndex, filename);
}

bool ADF_RemoveFile(ADF_Disk* disk, const char* path)
{
    ADF_FindResults r;
    if (!ADF_FindFile(disk, &r, path))
    {
        DIM_SetError(DIMError_FileNotFound);
        return false;
    }

    if (r.directory)
    {
        DIM_SetError(DIMError_FileNotFound); // Must use RemoveDirectory
        return false;
    }

    ADF_HeaderBlock header;
    if (!File_Seek(disk->file, r.block * 512) ||
        File_Read(disk->file, &header, 512) != 512)
    {
        DIM_SetError(DIMError_ReadError);
        return false;
    }

    uint32_t parentBlock = fix32(header.parent);
    if (!ADF_UnlinkFromParent(disk, parentBlock, r.block, r.fileName))
    {
        DIM_SetError(DIMError_WriteError);
        return false;
    }

    uint32_t currentBlockIdx = r.block;
    while (currentBlockIdx != 0)
    {
        ADF_HeaderBlock currentBlock;
        if (!File_Seek(disk->file, currentBlockIdx * 512) ||
            File_Read(disk->file, &currentBlock, 512) != 512)
        {
            // Blocks are leaked, but we can't safely continue.
            DIM_SetError(DIMError_ReadError);
            return false;
        }

        fix32(&currentBlock);

        // AmigaDOS fills from the top (71) down, but we check all slots to be safe.
        for (int i = 0; i < 72; i++)
        {
            if (currentBlock.dataBlocks[i] != 0)
            {
                ADF_FreeBlock(disk, currentBlock.dataBlocks[i]);
            }
        }

        uint32_t nextExtension = currentBlock.extension;
        ADF_FreeBlock(disk, currentBlockIdx);
        currentBlockIdx = nextExtension;
    }

    return true;
}

bool ADF_RemoveDirectory(ADF_Disk* disk, const char* path)
{
    // 1. Locate the directory
    ADF_FindResults r;
    if (!ADF_FindFile(disk, &r, path)) 
    {
        DIM_SetError(DIMError_FileNotFound);
        return false;
    }

    if (!r.directory)
    {
        DIM_SetError(DIMError_NotADirectory); // Cannot use RemoveDir on a file
        return false;
    }

    // 2. Read the Directory Header Block
    ADF_HeaderBlock dirBlock;
    if (!File_Seek(disk->file, r.block * 512) ||
        File_Read(disk->file, &dirBlock, 512) != 512)
    {
        DIM_SetError(DIMError_ReadError);
        return false;
    }

    // 3. Check if Empty
    // In AmigaDOS, the 'dataBlocks' array in the header is used as the HashTable for directories.
    // We must ensure every entry is 0.
    for (int i = 0; i < 72; i++)
    {
        if (dirBlock.dataBlocks[i] != 0)
        {
            DIM_SetError(DIMError_DirectoryNotEmpty);
            return false;
        }
    }

    // 4. Unlink from Parent
    // We need the parent block ID (stored in the directory header)
    uint32_t parentID = fix32(dirBlock.parent);
    
    // Note: r.fileName contains the "Leaf" name (e.g., "SubDir"), which is required for hashing
    if (!ADF_UnlinkFromParent(disk, parentID, r.block, r.fileName))
    {
        DIM_SetError(DIMError_WriteError); // Should not happen if filesystem is consistent
        return false;
    }

    // 5. Free the Header Block in Bitmap
    ADF_FreeBlock(disk, r.block);

    return true;
}

#endif
