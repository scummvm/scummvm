#include <dmg.h>
#include <ddb_vid.h>
#include <os_mem.h>
#include <os_lib.h>
#include <vid_screen.h>

#ifndef DEBUG_IMAGE_CACHE
#define DEBUG_IMAGE_CACHE 0
#endif

#ifndef DEBUG_FILE_CACHE
#define DEBUG_FILE_CACHE 0
#endif

static uint32_t useCounter = 0;
static const uint32_t kMinImageCacheSize = 4096;
#if defined(_DOS) && !defined(__386__)
static const uint32_t kMaxSafe16BitImageCacheSize = 63UL * 1024UL;
#endif

static uint32_t DMG_GetCacheItemAllocationSize(uint32_t payloadSize)
{
	return ((payloadSize + 63u) & ~31u) + sizeof(DMG_Cache);
}

static DMG_ImageMode DMG_GetActiveNativeImageModeForCache()
{
	const VID_ScreenAdapterInfo* info = VID_ScreenGetInfo();
	if (info != 0)
		return info->nativeImageMode;
	return DMG_NATIVE_IMAGE_MODE;
}

void DMG_SetupFileCache (DMG* dmg, uint32_t freeMemory, void(*progressFunction)(uint16_t))
{
	uint32_t totalBufferedBytes = 0;

#if DEBUG_FILE_CACHE
	DebugPrintf("Setting up file cache\n");
#endif

	if (dmg->fileCacheData != 0)
	{
		Free(dmg->fileCacheData);
		dmg->fileCacheData = 0;
	}

	dmg->fileCacheSize = 0;
	bool haveBufferedEntries = false;

	for (int n = 0; n < 256; n++)
	{
		if (dmg->entries[n] != 0)
		{
			dmg->entries[n]->cachedFileData = 0;
			dmg->entries[n]->cachedFileSize = 0;
		}
		if (dmg->entries[n] == 0 || 
			dmg->entries[n]->type == DMGEntry_Empty)
			continue;
		DMG_Entry* entry = dmg->entries[n];
		if ((entry->flags & DMG_FLAG_BUFFERED) == 0)
			continue;
		entry = DMG_GetEntry(dmg, (uint8_t)n);
		if (entry == 0 || entry->type == DMGEntry_Empty)
			continue;

		uint32_t bytes = entry->length;
		if (dmg->version != DMG_Version5)
			bytes += 6;
		totalBufferedBytes += bytes;
		haveBufferedEntries = true;
	}

	if (!haveBufferedEntries || totalBufferedBytes == 0)
	{
		if (progressFunction)
			progressFunction(255);
		return;
	}

	if (freeMemory != 0 && totalBufferedBytes > freeMemory)
	{
		#if DEBUG_FILE_CACHE
		DebugPrintf("File cache skipped: %lu bytes required, %lu available\n",
			(unsigned long)totalBufferedBytes,
			(unsigned long)freeMemory);
		#endif
		if (progressFunction)
			progressFunction(255);
		return;
	}

#if DEBUG_FILE_CACHE
	DebugPrintf("File cache buffered payload bytes: %lu\n", (unsigned long)totalBufferedBytes);
#endif

	if (progressFunction)
		progressFunction(64);

	dmg->fileCacheData = Allocate<uint8_t>("DAT Cache", totalBufferedBytes, false);
	if (dmg->fileCacheData == 0)
	{
		#if DEBUG_FILE_CACHE
		DebugPrintf("File cache allocation failed\n");
		#endif
		if (progressFunction)
			progressFunction(255);
		return;
	}

	uint32_t cachedBytes = 0;
	for (int n = 0; n < 256; n++)
	{
		if (dmg->entries[n] == 0 || dmg->entries[n]->type == DMGEntry_Empty)
			continue;
		DMG_Entry* entry = dmg->entries[n];
		if ((entry->flags & DMG_FLAG_BUFFERED) == 0)
			continue;
		entry = DMG_GetEntry(dmg, (uint8_t)n);
		if (entry == 0 || entry->type == DMGEntry_Empty)
			continue;

		uint32_t entryOffset = entry->fileOffset;
		uint32_t entrySize = entry->length;
		if (dmg->version != DMG_Version5)
			entrySize += 6;
		if (entrySize == 0)
			continue;

		uint8_t* ptr = dmg->fileCacheData + cachedBytes;

		if (!File_Seek(dmg->file, entryOffset))
		{
			Free(dmg->fileCacheData);
			dmg->fileCacheData = 0;
			dmg->fileCacheSize = 0;
			for (int i = 0; i < 256; i++)
			{
				if (dmg->entries[i] != 0)
				{
					dmg->entries[i]->cachedFileData = 0;
					dmg->entries[i]->cachedFileSize = 0;
				}
			}
			break;
		}

		uint32_t amount = entrySize;
#if DEBUG_FILE_CACHE
		DebugPrintf("File cache entry %d: reading %lu bytes from file offset %lu\n",
			n, (unsigned long)amount, (unsigned long)entryOffset);
#endif

		entry->cachedFileData = ptr;
		entry->cachedFileSize = entrySize;

		while (amount > 0)
		{
			uint32_t part = amount < 32768 ? amount : 32768;
			uint32_t read = File_Read(dmg->file, ptr, part);

			if (read < part) 	// Read error
			{
				Free(dmg->fileCacheData);
				dmg->fileCacheData = 0;
				dmg->fileCacheSize = 0;
				for (int i = 0; i < 256; i++)
				{
					if (dmg->entries[i] != 0)
					{
						dmg->entries[i]->cachedFileData = 0;
						dmg->entries[i]->cachedFileSize = 0;
					}
				}

				if (progressFunction)
					progressFunction(256);

				DebugPrintf("File cache allocation partial (read error!)\n");
				return;
			}
			amount -= read;
			ptr += read;
			cachedBytes += read;
			
			if (progressFunction)
			{
				uint32_t progress = 64 + (totalBufferedBytes == 0 ? 192 : cachedBytes * 192 / totalBufferedBytes);
				if (progress > 255)
					progress = 255;
				progressFunction(progress);
			}
		}
	}
	dmg->fileCacheSize = cachedBytes;

#if DEBUG_FILE_CACHE
	DebugPrintf("File cache allocated: %lu bytes in one block\n",
		(unsigned long)cachedBytes);
#endif

	if (progressFunction)
		progressFunction(255);
}

void* DMG_GetFromFileCache(DMG* dmg, uint32_t offset, uint32_t size)
{
	if (dmg->fileCacheData == 0)
		return 0;
	for (int n = 0; n < 256; n++)
	{
		DMG_Entry* entry = dmg->entries[n];
		if (entry == 0 || entry->cachedFileData == 0 || entry->cachedFileSize == 0)
			continue;
		uint32_t entryOffset = entry->fileOffset;
		uint32_t entrySize = entry->cachedFileSize;
		if (offset < entryOffset)
			continue;
		uint32_t skip = offset - entryOffset;
		if (skip > entrySize)
			continue;
		if (size > entrySize - skip)
			continue;
		return entry->cachedFileData + skip;
	}
	return 0;
}

void DMG_FreeImageCache(DMG* dmg)
{
	if (dmg->cache != 0)
	{
		VID_StopSampleIfOverlaps(dmg->cache, dmg->cacheSize);
		Free(dmg->cache);
		dmg->cache = 0;
	}
}

bool DMG_SetupImageCache (DMG* dmg, uint32_t bytes)
{
	uint32_t minimumBytes = DMG_GetMinimumImageCacheAllocationSize(dmg, DMG_NATIVE_IMAGE_MODE);
	if (minimumBytes < kMinImageCacheSize)
		minimumBytes = kMinImageCacheSize;
	#if defined(_DOS) && !defined(__386__)
	if (minimumBytes > kMaxSafe16BitImageCacheSize)
		minimumBytes = kMaxSafe16BitImageCacheSize;
	if (bytes > kMaxSafe16BitImageCacheSize)
		bytes = kMaxSafe16BitImageCacheSize;
	#endif
	if (bytes < minimumBytes)
		bytes = minimumBytes;

	if (dmg->cache != 0)
	{
		VID_StopSampleIfOverlaps(dmg->cache, dmg->cacheSize);
		Free(dmg->cache);
		dmg->cache = 0;
	}

	bytes &= ~31u;
	if (bytes < minimumBytes) {
		#if DEBUG_IMAGE_CACHE
		DebugPrintf("WARNING: No image cache allocated, only %lu bytes of RAM available\n", (unsigned long)bytes);
		#endif
		return false;
	}

	while (true)
	{
		dmg->cache = (DMG_Cache*)AllocateInPool<uint8_t>("DMG Image Cache", OSMemoryPool_Chip, bytes, false);
		if (dmg->cache != 0)
			break;
		#if DEBUG_IMAGE_CACHE
		DebugPrintf("Image cache allocation retry: %lu bytes failed\n", (unsigned long)bytes);
		#endif
		if (bytes <= minimumBytes)
		{
			DMG_SetError(DMG_ERROR_OUT_OF_MEMORY);
			dmg->cacheSize = 0;
			DebugPrintf("Image cache allocation failed\n");
			return false;
		}
		bytes >>= 1;
		bytes &= ~31u;
		if (bytes < minimumBytes)
			bytes = minimumBytes;
		#if DEBUG_IMAGE_CACHE
		DebugPrintf("Image cache allocation retry: shrinking request to %lu bytes\n", (unsigned long)bytes);
		#endif
	}
	dmg->cacheSize = bytes;
	dmg->cacheFree = bytes;
	dmg->cacheTail = dmg->cache;
	MemClear(dmg->cacheBitmap, sizeof(dmg->cacheBitmap));

	DebugPrintf("Image cache allocated (%ld bytes)\n", (long)bytes);
	return true;
}

bool DMG_RemoveOlderCacheItem (DMG* dmg, bool force)
{
	if (dmg->cache == 0)
		return false;

	uint32_t olderTime = 0;
	DMG_Cache* older = 0;
	DMG_Cache* item = dmg->cache;
	while (item != dmg->cacheTail)
	{
		if ((force || !item->buffer) && 
			(older == 0 || olderTime > item->time))
		{
			olderTime = item->time;
			older = item;
		}
		item = item->next;
	}
	if (older == 0)
	{
#if DEBUG_IMAGE_CACHE
		DebugPrintf("Image cache evict: no removable item (force=%d free=%lu)\n",
			force ? 1 : 0, (unsigned long)dmg->cacheFree);
#endif
		return false;
	}

	uint32_t space = DMG_GetCacheItemAllocationSize(older->size);
	uint8_t removedIndex = older->index;
#if DEBUG_IMAGE_CACHE
	DebugPrintf("Image cache evict: index=%u size=%lu buffered=%d free=%lu gain=%lu force=%d\n",
		(unsigned)removedIndex,
		(unsigned long)older->size,
		older->buffer ? 1 : 0,
		(unsigned long)dmg->cacheFree,
		(unsigned long)space,
		force ? 1 : 0);
#endif
	uint32_t offsetNext = (uint8_t*)older->next - (uint8_t*)dmg->cache;
	uint32_t spaceAfter = dmg->cacheSize - dmg->cacheFree - offsetNext;
	VID_StopSampleIfOverlaps(older, (uint32_t)((uint8_t*)dmg->cacheTail - (uint8_t*)older));
	if (spaceAfter > 0)
		MemMove(older, older->next, spaceAfter);

	dmg->cacheTail = (DMG_Cache*)((uint8_t*)dmg->cacheTail - space);
	dmg->cacheFree += space;

	dmg->cacheBitmap[removedIndex >> 3] &= ~(1 << (removedIndex & 7));

	item = older;
	while (item != dmg->cacheTail)
	{
		item->next = (DMG_Cache*)((uint8_t*)item->next - space);
		item = item->next;		            
	}
	return true;
}

DMG_Cache* DMG_GetImageCache (DMG* dmg, uint8_t index, DMG_Entry* entry, uint32_t size)
{
	if (dmg->cache == 0)
	{
		#if DEBUG_IMAGE_CACHE
		DebugPrintf("Image cache unavailable: index=%u required=%lu cacheSize=0\n",
			(unsigned)index,
			(unsigned long)size);
		#endif
		return 0;
	}

	if (dmg->cacheBitmap[index >> 3] & (1 << (index & 7)))
	{
		DMG_Cache* item = dmg->cache;
		while (item != dmg->cacheTail)
		{
			if (item->index == index)
			{
				DMG_ImageMode nativeImageMode = DMG_GetActiveNativeImageModeForCache();
				if (entry != 0 && entry->type == DMGEntry_Image && item->populated && item->imageMode != nativeImageMode)
				{
					DebugPrintf("FATAL: image cache contains non-native image data for entry %u (cached=%d native=%d)\n",
						(unsigned)index,
						(int)item->imageMode,
						(int)nativeImageMode);
					Abort();
				}
				if (item->size < size)
				{
					DebugPrintf("Image cache grow: cached block too small (cached=%lu required=%lu), evicting index %u\n",
						(unsigned long)item->size,
						(unsigned long)size,
						(unsigned)index);
					uint32_t oldTime = item->time;
					item->time = 0;
					if (!DMG_RemoveOlderCacheItem(dmg, true))
					{
						item->time = oldTime;
						DebugPrintf("Image cache grow: failed to evict existing block for index %u\n", (unsigned)index);
						return 0;
					}
					break;
				}
				item->time = useCounter++;
#if DEBUG_IMAGE_CACHE
				DebugPrintf("Image cache lookup: index=%u hit size=%lu required=%lu free=%lu\n",
					(unsigned)index,
					(unsigned long)item->size,
					(unsigned long)size,
					(unsigned long)dmg->cacheFree);
#endif
				return item;
			}
			item = item->next;
		}
		DebugPrintf("Image cache warning: bitmap mismatch for index %u, clearing stale bit\n", (unsigned)index);
		dmg->cacheBitmap[index >> 3] &= ~(1 << (index & 7));
	}

	// We're adding 32 bytes of extra pdding to help decompression
	// inside the same buffer as read

	uint32_t required = DMG_GetCacheItemAllocationSize(size);
#if DEBUG_IMAGE_CACHE
	DebugPrintf("Image cache lookup: index=%u miss required=%lu payload=%lu free=%lu total=%lu\n",
		(unsigned)index,
		(unsigned long)required,
		(unsigned long)size,
		(unsigned long)dmg->cacheFree,
		(unsigned long)dmg->cacheSize);
#endif
	if (required > dmg->cacheSize)
	{
		DebugPrintf("Image cache miss: required block too large for cache (need=%lu total=%lu)\n",
			(unsigned long)required,
			(unsigned long)dmg->cacheSize);
		return 0;
	}
	if (dmg->cacheFree < required)
	{
		bool force = dmg->cacheSize < 96UL * 1024UL;
#if DEBUG_IMAGE_CACHE
		DebugPrintf("Image cache compact: need=%lu free=%lu force=%d\n",
			(unsigned long)required, (unsigned long)dmg->cacheFree, force ? 1 : 0);
#endif
		while (DMG_RemoveOlderCacheItem(dmg, force)) 
		{
			if (dmg->cacheFree >= required)
				break;
		}
		if (dmg->cacheFree < required && !force && (entry->flags & DMG_FLAG_BUFFERED))
		{
			while (DMG_RemoveOlderCacheItem(dmg, true))
			{
				if (dmg->cacheFree >= required)
					break;
			}
		}
		if (dmg->cacheFree < required)
		{
			DebugPrintf("Image cache miss: insufficient space after compaction (need=%lu free=%lu)\n",
				(unsigned long)required, (unsigned long)dmg->cacheFree);
			return 0;
		}
	}

	DMG_Cache* item = dmg->cacheTail;
	dmg->cacheTail = (DMG_Cache*)((uint8_t*)dmg->cacheTail + required);
	item->next = dmg->cacheTail;
	item->size = size;
	item->time = useCounter++;
	item->index = index;
	item->imageMode = ImageMode_Raw;
	item->buffer = (entry->flags & DMG_FLAG_BUFFERED) != 0;
	item->populated = false;
	dmg->cacheBitmap[index >> 3] |= (1 << (index & 7));

	dmg->cacheFree -= required;
#if DEBUG_IMAGE_CACHE
	DebugPrintf("Image cache insert: index=%u required=%lu remaining=%lu size=%d -> %p\n",
		(unsigned)index,
		(unsigned long)required,
		(unsigned long)dmg->cacheFree,
		size,
		item);
#endif
	return item;
}
