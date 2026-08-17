#include <dmg.h>
#include <ddb.h>
#include <ddb_vid.h>
#include <vid_screen.h>
#include <ddb_pal.h>
#include <os_mem.h>
#include <os_lib.h>
#include <os_bito.h>

static DMG_ImageMode DMG_GetActiveNativeImageMode()
{
	#if defined(_DOS) || defined(_AMIGA)
	const VID_ScreenAdapterInfo* info = VID_ScreenGetInfo();
	if (info != 0)
		return info->nativeImageMode;
	#endif
	return DMG_NATIVE_IMAGE_MODE;
}

static void DMG_AbortOnInvalidCachedImageMode(uint8_t index, DMG_ImageMode cachedMode, DMG_ImageMode requestedMode)
{
	DebugPrintf("FATAL: image cache contains non-native image data for entry %u (cached=%d requested=%d native=%d)\n",
		(unsigned)index,
		(int)cachedMode,
		(int)requestedMode,
		(int)DMG_GetActiveNativeImageMode());
	Abort();
}

#if defined(_DOS)
static bool DMG_RangesOverlap(const uint8_t* a, uint32_t aSize, const uint8_t* b, uint32_t bSize)
{
	return a != 0 && b != 0 && aSize != 0 && bSize != 0 && a < b + bSize && b < a + aSize;
}

static bool DMG_ConvertIndexedToIndexedX(const uint8_t* input, uint16_t width, uint16_t height, uint8_t* output, uint32_t outputSize, uint8_t* scratch, uint32_t scratchSize)
{
	uint32_t bands = ((uint32_t)width + 3u) >> 2;
	uint32_t rowStride = bands * 4u;
	uint32_t indexedSize = (uint32_t)width * (uint32_t)height;
	uint32_t requiredSize = rowStride * (uint32_t)height;
	if (input == 0 || output == 0 || outputSize < requiredSize)
		return false;

	uint8_t* rowScratch = 0;
	if (scratch != 0 && scratchSize >= width)
	{
		uint8_t* candidate = scratch + scratchSize - width;
		if (!DMG_RangesOverlap(candidate, width, input, indexedSize) &&
			!DMG_RangesOverlap(candidate, width, output, requiredSize))
		{
			rowScratch = candidate;
		}
	}

	if (rowScratch == 0)
	{
		DMG_SetError(DMG_ERROR_BUFFER_TOO_SMALL);
		return false;
	}

	for (int32_t row = (int32_t)height - 1; row >= 0; row--)
	{
		const uint8_t* srcRow = input + (uint32_t)row * width;
		uint8_t* dstRow = output + (uint32_t)row * rowStride;
		MemCopy(rowScratch, srcRow, width);
		for (int32_t band = (int32_t)bands - 1; band >= 0; band--)
		{
			uint32_t pixelBase = (uint32_t)band << 2;
			uint8_t p0 = pixelBase + 0u < width ? rowScratch[pixelBase + 0u] : 0;
			uint8_t p1 = pixelBase + 1u < width ? rowScratch[pixelBase + 1u] : 0;
			uint8_t p2 = pixelBase + 2u < width ? rowScratch[pixelBase + 2u] : 0;
			uint8_t p3 = pixelBase + 3u < width ? rowScratch[pixelBase + 3u] : 0;
			dstRow[band + bands * 0u] = p0;
			dstRow[band + bands * 1u] = p1;
			dstRow[band + bands * 2u] = p2;
			dstRow[band + bands * 3u] = p3;
		}
	}

	return true;
}

static bool DMG_ConvertIndexedToCGA(const uint8_t* input, uint16_t width, uint16_t height, uint8_t* output, uint32_t outputSize)
{
	uint32_t rowBytes = ((uint32_t)width + 3u) >> 2;
	uint32_t requiredSize = rowBytes * (uint32_t)height;
	if (input == 0 || output == 0 || outputSize < requiredSize)
		return false;

	for (int32_t row = (int32_t)height - 1; row >= 0; row--)
	{
		const uint8_t* src = input + (uint32_t)row * width;
		uint8_t* dst = output + (uint32_t)row * rowBytes;
		for (int32_t byte = (int32_t)rowBytes - 1; byte >= 0; byte--)
		{
			uint32_t x = (uint32_t)byte << 2;
			uint8_t p0 = src[x] & 0x03;
			uint8_t p1 = x + 1u < width ? src[x + 1u] & 0x03 : 0;
			uint8_t p2 = x + 2u < width ? src[x + 2u] & 0x03 : 0;
			uint8_t p3 = x + 3u < width ? src[x + 3u] & 0x03 : 0;
			dst[byte] = (uint8_t)((p0 << 6) | (p1 << 4) | (p2 << 2) | p3);
		}
	}

	return true;
}

static bool DMG_ConvertPackedToCGA(const uint8_t* input, uint16_t width, uint16_t height, uint8_t* output, uint32_t outputSize, const uint8_t* paletteMap)
{
	uint32_t rowBytes = ((uint32_t)width + 3u) >> 2;
	uint32_t packedRowBytes = (((uint32_t)width + 1u) >> 1);
	uint32_t requiredSize = rowBytes * (uint32_t)height;
	if (input == 0 || output == 0 || outputSize < requiredSize)
		return false;

	for (int32_t row = (int32_t)height - 1; row >= 0; row--)
	{
		const uint8_t* srcRow = input + (uint32_t)row * packedRowBytes;
		uint8_t* dst = output + (uint32_t)row * rowBytes;
		for (uint32_t byte = 0; byte < rowBytes; byte++)
		{
			uint32_t x = byte << 2;
			uint8_t p0 = 0;
			uint8_t p1 = 0;
			uint8_t p2 = 0;
			uint8_t p3 = 0;

			if (x + 0u < width)
			{
				uint8_t v = srcRow[(x + 0u) >> 1];
				p0 = ((x + 0u) & 1u) == 0 ? (v >> 4) : (v & 0x0F);
				if (paletteMap != 0)
					p0 = paletteMap[p0 & 0x0F];
			}
			if (x + 1u < width)
			{
				uint8_t v = srcRow[(x + 1u) >> 1];
				p1 = ((x + 1u) & 1u) == 0 ? (v >> 4) : (v & 0x0F);
				if (paletteMap != 0)
					p1 = paletteMap[p1 & 0x0F];
			}
			if (x + 2u < width)
			{
				uint8_t v = srcRow[(x + 2u) >> 1];
				p2 = ((x + 2u) & 1u) == 0 ? (v >> 4) : (v & 0x0F);
				if (paletteMap != 0)
					p2 = paletteMap[p2 & 0x0F];
			}
			if (x + 3u < width)
			{
				uint8_t v = srcRow[(x + 3u) >> 1];
				p3 = ((x + 3u) & 1u) == 0 ? (v >> 4) : (v & 0x0F);
				if (paletteMap != 0)
					p3 = paletteMap[p3 & 0x0F];
			}

			dst[byte] = (uint8_t)(((p0 & 0x03) << 6) | ((p1 & 0x03) << 4) | ((p2 & 0x03) << 2) | (p3 & 0x03));
		}
	}

	return true;
}

static bool DMG_ConvertPackedToIndexedX(const uint8_t* input, uint16_t width, uint16_t height, uint8_t* output, uint32_t outputSize, const uint8_t* paletteMap, uint8_t* scratch, uint32_t scratchSize)
{
	uint32_t bands = ((uint32_t)width + 3u) >> 2;
	uint32_t rowStride = bands * 4u;
	uint32_t packedRowBytes = (((uint32_t)width + 1u) >> 1);
	uint32_t packedSize = packedRowBytes * (uint32_t)height;
	uint32_t requiredSize = rowStride * (uint32_t)height;
	if (input == 0 || output == 0 || outputSize < requiredSize)
		return false;

	if ((width & 1u) == 0)
	{
		uint8_t* rowScratch = 0;
		if (scratch != 0 && scratchSize >= packedRowBytes)
		{
			uint8_t* candidate = scratch + scratchSize - packedRowBytes;
			if (!DMG_RangesOverlap(candidate, packedRowBytes, input, packedSize) &&
				!DMG_RangesOverlap(candidate, packedRowBytes, output, requiredSize))
			{
				rowScratch = candidate;
			}
		}

		if (rowScratch == 0)
		{
			DMG_SetError(DMG_ERROR_BUFFER_TOO_SMALL);
			return false;
		}

		for (int32_t row = (int32_t)height - 1; row >= 0; row--)
		{
			const uint8_t* srcRow = input + (uint32_t)row * packedRowBytes;
			uint8_t* dstRow = output + (uint32_t)row * rowStride;
			MemCopy(rowScratch, srcRow, packedRowBytes);
			for (uint32_t band = 0; band < bands; band++)
			{
				uint32_t pixelBase = band << 2;
				uint8_t p0 = 0;
				uint8_t p1 = 0;
				uint8_t p2 = 0;
				uint8_t p3 = 0;

				if (pixelBase + 0u < width)
				{
					uint8_t value = rowScratch[(pixelBase + 0u) >> 1];
					p0 = ((pixelBase + 0u) & 1u) == 0 ? (value >> 4) : (value & 0x0F);
					if (paletteMap != 0)
						p0 = paletteMap[p0 & 0x0F];
				}
				if (pixelBase + 1u < width)
				{
					uint8_t value = rowScratch[(pixelBase + 1u) >> 1];
					p1 = ((pixelBase + 1u) & 1u) == 0 ? (value >> 4) : (value & 0x0F);
					if (paletteMap != 0)
						p1 = paletteMap[p1 & 0x0F];
				}
				if (pixelBase + 2u < width)
				{
					uint8_t value = rowScratch[(pixelBase + 2u) >> 1];
					p2 = ((pixelBase + 2u) & 1u) == 0 ? (value >> 4) : (value & 0x0F);
					if (paletteMap != 0)
						p2 = paletteMap[p2 & 0x0F];
				}
				if (pixelBase + 3u < width)
				{
					uint8_t value = rowScratch[(pixelBase + 3u) >> 1];
					p3 = ((pixelBase + 3u) & 1u) == 0 ? (value >> 4) : (value & 0x0F);
					if (paletteMap != 0)
						p3 = paletteMap[p3 & 0x0F];
				}

				dstRow[band + bands * 0u] = p0;
				dstRow[band + bands * 1u] = p1;
				dstRow[band + bands * 2u] = p2;
				dstRow[band + bands * 3u] = p3;
			}
		}

		return true;
	}

	const uint8_t* src = input;
	bool highNibble = true;
	for (uint16_t row = 0; row < height; row++)
	{
		uint8_t* dstRow = output + (uint32_t)row * rowStride;
		for (uint32_t band = 0; band < bands; band++)
		{
			uint32_t pixelBase = band << 2;
			uint8_t p0 = 0;
			uint8_t p1 = 0;
			uint8_t p2 = 0;
			uint8_t p3 = 0;

			if (pixelBase + 0u < width)
			{
				uint8_t value = *src;
				p0 = highNibble ? (value >> 4) : (value & 0x0F);
				if (paletteMap != 0)
					p0 = paletteMap[p0 & 0x0F];
				if (!highNibble)
					src++;
				highNibble = !highNibble;
			}
			if (pixelBase + 1u < width)
			{
				uint8_t value = *src;
				p1 = highNibble ? (value >> 4) : (value & 0x0F);
				if (paletteMap != 0)
					p1 = paletteMap[p1 & 0x0F];
				if (!highNibble)
					src++;
				highNibble = !highNibble;
			}
			if (pixelBase + 2u < width)
			{
				uint8_t value = *src;
				p2 = highNibble ? (value >> 4) : (value & 0x0F);
				if (paletteMap != 0)
					p2 = paletteMap[p2 & 0x0F];
				if (!highNibble)
					src++;
				highNibble = !highNibble;
			}
			if (pixelBase + 3u < width)
			{
				uint8_t value = *src;
				p3 = highNibble ? (value >> 4) : (value & 0x0F);
				if (paletteMap != 0)
					p3 = paletteMap[p3 & 0x0F];
				if (!highNibble)
					src++;
				highNibble = !highNibble;
			}

			dstRow[band + bands * 0u] = p0;
			dstRow[band + bands * 1u] = p1;
			dstRow[band + bands * 2u] = p2;
			dstRow[band + bands * 3u] = p3;
		}
	}

	return true;
}

static bool DMG_CanConvertPackedToIndexedX(const DMG* dmg)
{
	if (dmg == 0)
		return false;
	return dmg->version != DMG_Version5 && dmg->version != DMG_Version1_PCW;
}

static uint8_t* DMG_GetEntryDataCGA(DMG* dmg, uint8_t index)
{
	DMG_Entry* entry = DMG_GetEntry(dmg, index);
	if (entry == 0 || entry->type == DMGEntry_Empty)
		return 0;
	if (entry->type == DMGEntry_Audio)
		return DMG_GetEntryData(dmg, index, ImageMode_Audio);
	if (dmg != 0 && dmg->version == DMG_Version5 && dmg->colorMode == DMG_DAT5_COLORMODE_CGA)
		return DMG_GetEntryData(dmg, index, ImageMode_CGA);

	bool cacheNativeOnly = entry->type == DMGEntry_Image;
	DMG_ImageMode nativeImageMode = DMG_GetActiveNativeImageMode();
	bool allowCache = !cacheNativeOnly || nativeImageMode == ImageMode_CGA;
	uint32_t nativeSize = DMG_CalculateRequiredSize(entry, ImageMode_CGA);
	DMG_Cache* cache = allowCache ? DMG_GetImageCache(dmg, index, entry, nativeSize) : 0;
	uint8_t* output = cache != 0 ? (uint8_t*)(cache + 1) : DMG_GetTemporaryBuffer(ImageMode_CGA);
	uint32_t outputSize = cache != 0 ? cache->size : DMG_GetTemporaryBufferSize();
	if (cache != 0 && cache->populated && cache->imageMode == ImageMode_CGA)
		return output;
	if (output == 0 || outputSize < nativeSize)
	{
		DMG_SetError(DMG_ERROR_BUFFER_TOO_SMALL);
		if (cache != 0)
			cache->populated = false;
		return 0;
	}

	const uint8_t* cgaPalette = DMG_GetEntryCGAPalette(entry);
	if (cgaPalette != 0)
	{
		uint8_t* packedData = DMG_GetEntryData(dmg, index, ImageMode_Packed);
		if (packedData != 0 && DMG_ConvertPackedToCGA(packedData, entry->width, entry->height, output, outputSize, cgaPalette))
		{
			if (cache != 0)
			{
				cache->imageMode = ImageMode_CGA;
				cache->populated = true;
			}
			return output;
		}
		if (DMG_GetError() != DMG_ERROR_NONE)
		{
			if (cache != 0)
				cache->populated = false;
			return 0;
		}
	}

	uint8_t* indexedData = DMG_GetEntryData(dmg, index, ImageMode_Indexed);
	if (indexedData == 0)
	{
		if (cache != 0)
			cache->populated = false;
		return 0;
	}
	if (!DMG_ConvertIndexedToCGA(indexedData, entry->width, entry->height, output, outputSize))
	{
		if (cache != 0)
			cache->populated = false;
		DMG_SetError(DMG_ERROR_INVALID_IMAGE);
		return 0;
	}

	if (cache != 0)
	{
		cache->imageMode = ImageMode_CGA;
		cache->populated = true;
	}
	return output;
}

static uint8_t* DMG_GetEntryDataIndexedX(DMG* dmg, uint8_t index)
{
	DMG_Entry* entry = DMG_GetEntry(dmg, index);
	if (entry == 0 || entry->type == DMGEntry_Empty)
		return 0;
	if (entry->type == DMGEntry_Audio)
		return DMG_GetEntryData(dmg, index, ImageMode_Audio);
	if (dmg != 0 && dmg->version == DMG_Version5 && DMG_DAT5ModeIsIndexedX(dmg->colorMode))
		return DMG_GetEntryData(dmg, index, ImageMode_IndexedX);

	bool cacheNativeOnly = entry->type == DMGEntry_Image;
	DMG_ImageMode nativeImageMode = DMG_GetActiveNativeImageMode();
	bool allowCache = !cacheNativeOnly || nativeImageMode == ImageMode_IndexedX;
	uint32_t nativeSize = DMG_CalculateRequiredSize(entry, ImageMode_IndexedX);
	DMG_Cache* cache = allowCache ? DMG_GetImageCache(dmg, index, entry, nativeSize) : 0;
	uint8_t* output = cache != 0 ? (uint8_t*)(cache + 1) : DMG_GetTemporaryBuffer(ImageMode_IndexedX);
	uint32_t outputSize = cache != 0 ? cache->size : DMG_GetTemporaryBufferSize();
	if (cache != 0 && cache->populated && cache->imageMode == ImageMode_IndexedX)
		return output;
	if (output == 0 || outputSize < nativeSize)
	{
		DMG_SetError(DMG_ERROR_BUFFER_TOO_SMALL);
		if (cache != 0)
			cache->populated = false;
		return 0;
	}

	if (DMG_CanConvertPackedToIndexedX(dmg) && (entry->flags & DMG_FLAG_FIXED) == 0)
	{
		DMG_ColorPaletteMode paletteMode = ColorPaletteMode_Native;
		const uint8_t* paletteMap = 0;
		if (dmg->screenMode == ScreenMode_CGA || dmg->colorMode == DAT5_COLORMODE_CGA)
			paletteMode = ColorPaletteMode_CGA;
		if (dmg->screenMode == ScreenMode_EGA || dmg->colorMode == DAT5_COLORMODE_EGA)
			paletteMode = ColorPaletteMode_EGA;
		if (paletteMode == ColorPaletteMode_CGA)
			paletteMap = DMG_GetEntryCGAPalette(entry);
		else if (paletteMode == ColorPaletteMode_EGA)
			paletteMap = DMG_GetEntryEGAPalette(entry);
		if ((paletteMode == ColorPaletteMode_CGA || paletteMode == ColorPaletteMode_EGA) && paletteMap == 0)
		{
			if (cache != 0)
				cache->populated = false;
			DMG_SetError(DMG_ERROR_INVALID_IMAGE);
			return 0;
		}

		uint8_t* packedData = DMG_GetEntryData(dmg, index, ImageMode_Packed);
		if (packedData == 0)
		{
			if (cache != 0)
				cache->populated = false;
			return 0;
		}

		uint8_t* scratch = DMG_GetScratchBuffer(dmg, ((uint32_t)entry->width + 1u) >> 1);
		uint32_t scratchSize = DMG_GetScratchBufferSize(dmg);
		if (!DMG_ConvertPackedToIndexedX(packedData, entry->width, entry->height, output, nativeSize, paletteMap, scratch, scratchSize))
		{
			if (cache != 0)
				cache->populated = false;
			return 0;
		}
	}
	else
	{
		uint8_t* indexedData = DMG_GetEntryData(dmg, index, ImageMode_Indexed);
		if (indexedData == 0)
		{
			if (cache != 0)
				cache->populated = false;
			return 0;
		}
		uint8_t* scratch = DMG_GetScratchBuffer(dmg, entry->width);
		uint32_t scratchSize = DMG_GetScratchBufferSize(dmg);
		if (!DMG_ConvertIndexedToIndexedX(indexedData, entry->width, entry->height, output, nativeSize, scratch, scratchSize))
		{
			if (cache != 0)
				cache->populated = false;
			return 0;
		}
	}

	if (cache != 0)
	{
		cache->imageMode = ImageMode_IndexedX;
		cache->populated = true;
	}

	return output;
}
#endif

static bool DMG_ConvertIndexedXToIndexed(const uint8_t* input, uint16_t width, uint16_t height, uint8_t* output, uint32_t outputSize)
{
	uint32_t bands = ((uint32_t)width + 3u) >> 2;
	uint32_t rowStride = bands * 4u;
	uint32_t indexedSize = (uint32_t)width * (uint32_t)height;
	if (input == 0 || output == 0 || outputSize < indexedSize)
		return false;

	for (uint16_t row = 0; row < height; row++)
	{
		const uint8_t* srcRow = input + (uint32_t)row * rowStride;
		uint8_t* dstRow = output + (uint32_t)row * width;
		const uint8_t* src0 = srcRow;
		const uint8_t* src1 = srcRow + bands;
		const uint8_t* src2 = srcRow + bands * 2u;
		const uint8_t* src3 = srcRow + bands * 3u;
		uint32_t fullBands = ((uint32_t)width) >> 2;
		uint32_t band = 0;
		uint32_t* dst32 = (uint32_t*)dstRow;
		for (; band < fullBands; band++)
		{
			dst32[band] =
				((uint32_t)src0[band]) |
				((uint32_t)src1[band] << 8) |
				((uint32_t)src2[band] << 16) |
				((uint32_t)src3[band] << 24);
		}
		dstRow += fullBands << 2;
		switch (width & 3u)
		{
			case 3:
				*dstRow++ = src0[band];
				*dstRow++ = src1[band];
				*dstRow++ = src2[band];
				break;
			case 2:
				*dstRow++ = src0[band];
				*dstRow++ = src1[band];
				break;
			case 1:
				*dstRow++ = src0[band];
				break;
			default:
				break;
		}
	}

	return true;
}

static uint8_t* DMG_GetEntryDataV5(DMG* dmg, uint8_t index, DMG_ImageMode mode, DMG_Entry* entry, uint8_t* buffer, uint32_t bufferSize)
{
	#if DEBUG_ZX0
	uint32_t tV5Start = 0;
	VID_GetMilliseconds(&tV5Start);
	#endif

	uint32_t indexedSize = entry->width * entry->height;
	uint32_t paletteBytes = entry->paletteColors * 3;
	uint32_t imageDataLength = entry->length >= paletteBytes ? entry->length - paletteBytes : 0;
	uint8_t* fileData = imageDataLength <= bufferSize ? buffer + bufferSize - imageDataLength : 0;
	uint8_t* tempFileData = 0;
	if (entry->length < paletteBytes)
	{
		DMG_SetError(DMG_ERROR_BUFFER_TOO_SMALL);
		return 0;
	}

	if (mode == ImageMode_Audio)
	{
		if (imageDataLength > bufferSize)
		{
			DMG_SetError(DMG_ERROR_BUFFER_TOO_SMALL);
			return 0;
		}
		const uint8_t* storedData = DMG_GetEntryStoredData(dmg, index);
		uint32_t storedDataSize = DMG_GetEntryStoredDataSize(dmg, index);
		if (storedData != 0)
		{
			if (storedDataSize > bufferSize)
			{
				DMG_SetError(DMG_ERROR_BUFFER_TOO_SMALL);
				return 0;
			}
			MemCopy(fileData, storedData, storedDataSize);
			return fileData;
		}
		if (DMG_ReadFromFile(dmg, entry->fileOffset, fileData, entry->length) != entry->length)
		{
			DMG_SetError(DMG_ERROR_READING_FILE);
			return 0;
		}
		return fileData;
	}

	if ((entry->flags & DMG_FLAG_COMPRESSED) != 0)
	{
		if ((entry->flags & DMG_FLAG_ZX0) == 0)
		{
			DMG_SetError(DMG_ERROR_INVALID_IMAGE);
			return 0;
		}

		uint32_t decompressedSize = 0;
		switch (dmg->colorMode)
		{
			case DMG_DAT5_COLORMODE_CGA:
			case DMG_DAT5_COLORMODE_EGA:
			#if DMG_SUPPORT_DAT5_PLANAR_SOURCES
			case DMG_DAT5_COLORMODE_PLANAR4:
			case DMG_DAT5_COLORMODE_PLANAR5:
			case DMG_DAT5_COLORMODE_PLANAR8:
			case DMG_DAT5_COLORMODE_EHB6:
			case DMG_DAT5_COLORMODE_HAM6:
			#endif
			#if DMG_SUPPORT_DAT5_ST_SOURCES
			case DMG_DAT5_COLORMODE_PLANAR4ST:
			case DMG_DAT5_COLORMODE_PLANAR8ST:
			#endif
			#if DMG_SUPPORT_DAT5_INDEXEDX_SOURCES
			case DMG_DAT5_COLORMODE_INDEXEDX:
			#endif
			case DMG_DAT5_COLORMODE_INDEXED:
				decompressedSize = DMG_DAT5StoredImageSize(dmg->colorMode, entry->width, entry->height);
				break;
			default:
				DMG_SetError(DMG_ERROR_INVALID_IMAGE);
				return 0;
		}

		bool directOutput =
			(mode == ImageMode_CGA && dmg->colorMode == DMG_DAT5_COLORMODE_CGA) ||
			(mode == ImageMode_Indexed && DMG_DAT5ModeIsIndexed(dmg->colorMode)) ||
			#if DMG_SUPPORT_DAT5_INDEXEDX_SOURCES
			(mode == ImageMode_IndexedX && DMG_DAT5ModeIsIndexedX(dmg->colorMode)) ||
			#endif
			#if DMG_SUPPORT_DAT5_PLANAR_SOURCES
			(mode == ImageMode_Planar && DMG_DAT5ModeIsPlaneMajor(dmg->colorMode)) ||
			#endif
			#if DMG_SUPPORT_DAT5_ST_SOURCES
			(mode == ImageMode_PlanarST && DMG_DAT5ModeIsSTInterleaved(dmg->colorMode) && entry->bitDepth <= 4) ||
			(mode == ImageMode_PlanarFalcon && DMG_DAT5ModeIsSTInterleaved(dmg->colorMode) && entry->bitDepth == 8) ||
			#endif
			false;

		if (directOutput && decompressedSize > bufferSize)
		{
			DMG_SetError(DMG_ERROR_BUFFER_TOO_SMALL);
			return 0;
		}
		uint8_t* decompressedOutput = buffer;
		bool freeDecompressedOutput = false;
		if (!directOutput)
		{
			decompressedOutput = Allocate<uint8_t>("DAT5 decompressed", decompressedSize, false);
			if (decompressedOutput == 0)
			{
				DMG_SetError(DMG_ERROR_OUT_OF_MEMORY);
				return 0;
			}
			freeDecompressedOutput = true;
		}
		uint8_t* compressedData = 0;
		const uint8_t* compressedInput = (const uint8_t*)DMG_GetFromFileCache(dmg, entry->fileOffset + paletteBytes, imageDataLength);
		bool freeCompressedData = false;

		#if DEBUG_ZX0
		uint32_t tFetch0 = 0;
		uint32_t tFetch1 = 0;
		VID_GetMilliseconds(&tFetch0);
		#endif

		if (compressedInput == 0)
		{
			if (!freeDecompressedOutput && bufferSize >= decompressedSize + imageDataLength)
			{
				compressedData = buffer + bufferSize - imageDataLength;
			}
			else if (DMG_GetScratchBufferSize(dmg) >= imageDataLength)
			{
				compressedData = DMG_GetScratchBuffer(dmg, imageDataLength);
			}
			else
			{
				compressedData = Allocate<uint8_t>("ZX0 input", imageDataLength, false);
				if (compressedData == 0)
				{
					if (freeDecompressedOutput)
						Free(decompressedOutput);
					DMG_SetError(DMG_ERROR_OUT_OF_MEMORY);
					return 0;
				}
				freeCompressedData = true;
			}
			if (DMG_ReadFromFile(dmg, entry->fileOffset + paletteBytes, compressedData, imageDataLength) != imageDataLength)
			{
				if (freeCompressedData)
					Free(compressedData);
				if (freeDecompressedOutput)
					Free(decompressedOutput);
				DMG_SetError(DMG_ERROR_READING_FILE);
				return 0;
			}
			compressedInput = compressedData;
		}

		#if DEBUG_ZX0
		VID_GetMilliseconds(&tFetch1);
		DebugPrintf("DAT5 ZX0 fetch image %u: %s in %lu ms (%lu bytes)\n",
			(unsigned)index,
			compressedData == 0 ? "cache" : "file",
			(unsigned long)(tFetch1 - tFetch0),
			(unsigned long)imageDataLength);
		#endif

		#if DEBUG_ZX0
		uint32_t t0, t1;
		VID_GetMilliseconds(&t0);
		#endif

		if (!DMG_DecompressZX0(compressedInput, imageDataLength, decompressedOutput, decompressedSize))
		{
			if (freeCompressedData)
				Free(compressedData);
			if (freeDecompressedOutput)
				Free(decompressedOutput);
			DMG_SetError(DMG_ERROR_CORRUPTED_DATA_STREAM);
			return 0;
		}

		#if DEBUG_ZX0
		VID_GetMilliseconds(&t1);
		uint32_t dt = t1 - t0;
		dmg->zx0ProfileCount++;
		dmg->zx0ProfileInputBytes += imageDataLength;
		dmg->zx0ProfileOutputBytes += decompressedSize;
		dmg->zx0ProfileTotalMs += dt;
		if (dt >= dmg->zx0ProfileMaxMs)
		{
			dmg->zx0ProfileMaxMs = dt;
			dmg->zx0ProfileMaxIndex = index;
		}
		DebugPrintf("Decompressed DAT5 ZX0 image %u: %lu -> %lu bytes in %lu ms\n",
			(unsigned)index,
			(unsigned long)imageDataLength,
			(unsigned long)decompressedSize,
			(unsigned long)dt);
		#endif

		if (directOutput)
		{
			if (freeCompressedData)
				Free(compressedData);
			if (decompressedOutput != buffer)
			{
				if (decompressedSize > bufferSize)
				{
					if (freeDecompressedOutput)
						Free(decompressedOutput);
					DMG_SetError(DMG_ERROR_BUFFER_TOO_SMALL);
					return 0;
				}
				MemCopy(buffer, decompressedOutput, decompressedSize);
				if (freeDecompressedOutput)
					Free(decompressedOutput);
			}
			return buffer;
		}

		tempFileData = freeDecompressedOutput ? decompressedOutput : 0;
		if (freeCompressedData)
			Free(compressedData);
		fileData = decompressedOutput;
		imageDataLength = decompressedSize;
	}
	else
	{
		const uint8_t* storedData = DMG_GetEntryStoredData(dmg, index);
		if (imageDataLength > bufferSize ||
			#if DMG_SUPPORT_DAT5_INDEXEDX_SOURCES
			(DMG_DAT5ModeIsIndexedX(dmg->colorMode) && mode != ImageMode_IndexedX) ||
			#endif
			(DMG_DAT5ModeIsIndexed(dmg->colorMode) && mode != ImageMode_Indexed))
		{
			tempFileData = Allocate<uint8_t>("DAT5 image", imageDataLength, false);
			if (tempFileData == 0)
			{
				DMG_SetError(DMG_ERROR_OUT_OF_MEMORY);
				return 0;
			}
			fileData = tempFileData;
		}
		if (storedData != 0)
		{
			if (DMG_GetEntryStoredDataSize(dmg, index) != imageDataLength)
			{
				DMG_SetError(DMG_ERROR_INVALID_IMAGE);
				return 0;
			}
			MemCopy(fileData, storedData, imageDataLength);
		}
		else
		if (DMG_ReadFromFile(dmg, entry->fileOffset + paletteBytes, fileData, imageDataLength) != imageDataLength)
		{
			DMG_SetError(DMG_ERROR_READING_FILE);
			return 0;
		}
	}

	if (entry->type != DMGEntry_Image)
	{
		return 0;
	}

	if (mode == ImageMode_Planar || mode == ImageMode_PlanarST || mode == ImageMode_PlanarFalcon)
	{
		if (
			#if DMG_SUPPORT_DAT5_PLANAR_SOURCES
			(mode == ImageMode_Planar && DMG_DAT5ModeIsPlaneMajor(dmg->colorMode)) ||
			#endif
			#if DMG_SUPPORT_DAT5_ST_SOURCES
			(mode == ImageMode_PlanarST && DMG_DAT5ModeIsSTInterleaved(dmg->colorMode) && entry->bitDepth <= 4) ||
			(mode == ImageMode_PlanarFalcon && DMG_DAT5ModeIsSTInterleaved(dmg->colorMode) && entry->bitDepth == 8) ||
			#endif
			false)
		{
			if (fileData != buffer)
				MemMove(buffer, fileData, imageDataLength);
			if (tempFileData)
				Free(tempFileData);
			return buffer;
		}
	}

	#if DMG_SUPPORT_DAT5_INDEXEDX_SOURCES
	if (mode == ImageMode_IndexedX && DMG_DAT5ModeIsIndexedX(dmg->colorMode))
	{
		if (fileData != buffer)
			MemMove(buffer, fileData, imageDataLength);
		if (tempFileData)
			Free(tempFileData);
		return buffer;
	}
	#endif

	if (mode == ImageMode_Indexed && DMG_DAT5ModeIsIndexed(dmg->colorMode))
	{
		if (fileData != buffer)
			MemMove(buffer, fileData, imageDataLength);
		if (tempFileData)
			Free(tempFileData);
		return buffer;
	}

	if (mode == ImageMode_CGA && dmg->colorMode == DMG_DAT5_COLORMODE_CGA)
	{
		if (fileData != buffer)
			MemMove(buffer, fileData, imageDataLength);
		if (tempFileData)
			Free(tempFileData);
		return buffer;
	}

	DMG_ColorPaletteMode paletteMode = ColorPaletteMode_Native;

	switch (dmg->screenMode)
	{
		case ScreenMode_CGA:
			paletteMode = ColorPaletteMode_CGA;
			break;
		case ScreenMode_EGA:
			paletteMode = ColorPaletteMode_EGA;
			break;
		default:
			break;
	}

	switch (dmg->colorMode)
	{
		case DMG_DAT5_COLORMODE_CGA:
			if (!DMG_UnpackChunkyPixels(fileData, entry->width, entry->height, 2, buffer))
			{
				if (tempFileData)
					Free(tempFileData);
				DMG_SetError(DMG_ERROR_INVALID_IMAGE);
				return 0;
			}
			paletteMode = ColorPaletteMode_CGA;
			break;

		case DMG_DAT5_COLORMODE_EGA:
			if (!DMG_UnpackChunkyPixels(fileData, entry->width, entry->height, 4, buffer))
			{
				if (tempFileData)
					Free(tempFileData);
				DMG_SetError(DMG_ERROR_INVALID_IMAGE);
				return 0;
			}
			paletteMode = ColorPaletteMode_EGA;
			break;

		#if DMG_SUPPORT_DAT5_INDEXEDX_SOURCES
		case DMG_DAT5_COLORMODE_INDEXEDX:
			if (!DMG_ConvertIndexedXToIndexed(fileData, entry->width, entry->height, buffer, bufferSize))
			{
				if (tempFileData)
					Free(tempFileData);
				DMG_SetError(DMG_ERROR_INVALID_IMAGE);
				return 0;
			}
			break;
		#endif

		case DMG_DAT5_COLORMODE_INDEXED:
			if (imageDataLength > bufferSize)
			{
				if (tempFileData)
					Free(tempFileData);
				DMG_SetError(DMG_ERROR_BUFFER_TOO_SMALL);
				return 0;
			}
			if (fileData != buffer)
				MemMove(buffer, fileData, imageDataLength);
			break;

		#if DMG_SUPPORT_DAT5_PLANAR_SOURCES
		case DMG_DAT5_COLORMODE_PLANAR4:
		case DMG_DAT5_COLORMODE_PLANAR5:
		case DMG_DAT5_COLORMODE_PLANAR8:
		case DMG_DAT5_COLORMODE_EHB6:
		case DMG_DAT5_COLORMODE_HAM6:
			if (!DMG_UnpackBitplaneBytes(fileData, entry->width, entry->height, entry->bitDepth, buffer))
			{
				if (tempFileData)
					Free(tempFileData);
				DMG_SetError(DMG_ERROR_INVALID_IMAGE);
				return 0;
			}
			break;
		#endif

		#if DMG_SUPPORT_DAT5_ST_SOURCES
		case DMG_DAT5_COLORMODE_PLANAR4ST:
		case DMG_DAT5_COLORMODE_PLANAR8ST:
			if (!DMG_UnpackBitplaneWords(fileData, entry->width, entry->height, entry->bitDepth, buffer))
			{
				if (tempFileData)
					Free(tempFileData);
				DMG_SetError(DMG_ERROR_INVALID_IMAGE);
				return 0;
			}
			break;
		#endif

		default:
			if (tempFileData)
				Free(tempFileData);
			DMG_SetError(DMG_ERROR_INVALID_IMAGE);
			return 0;
	}

	#if DEBUG_ZX0
	uint32_t tAfterConvert = 0;
	VID_GetMilliseconds(&tAfterConvert);
	DebugPrintf("DAT5 V5 convert image %u: mode=%u colorMode=%u in %lu ms\n",
		(unsigned)index,
		(unsigned)mode,
		(unsigned)dmg->colorMode,
		(unsigned long)(tAfterConvert - tV5Start));
	#endif

	if (mode == ImageMode_Indexed)
	{
		if (paletteMode == ColorPaletteMode_CGA)
		{
			for (uint32_t i = 0; i < indexedSize; i++)
				buffer[i] &= 0x03;
		}
		else if (paletteMode == ColorPaletteMode_EGA)
		{
			for (uint32_t i = 0; i < indexedSize; i++)
				buffer[i] &= 0x0F;
		}
		if (tempFileData)
			Free(tempFileData);
		#if DEBUG_ZX0
		uint32_t tIndexedDone = 0;
		VID_GetMilliseconds(&tIndexedDone);
		DebugPrintf("DAT5 V5 indexed finalize image %u: paletteMode=%u in %lu ms (total %lu ms)\n",
			(unsigned)index,
			(unsigned)paletteMode,
			(unsigned long)(tIndexedDone - tAfterConvert),
			(unsigned long)(tIndexedDone - tV5Start));
		#endif
		return buffer;
	}

	if (mode == ImageMode_Packed)
	{
		if (paletteMode == ColorPaletteMode_CGA)
		{
			MemCopy(buffer, fileData, imageDataLength);
			if (tempFileData)
				Free(tempFileData);
			return buffer;
		}
		if (paletteMode == ColorPaletteMode_EGA)
		{
			MemCopy(buffer, fileData, imageDataLength);
			if (tempFileData)
				Free(tempFileData);
			return buffer;
		}
		if (tempFileData)
			Free(tempFileData);
		DMG_SetError(DMG_ERROR_INVALID_IMAGE);
		return 0;
	}

	if (mode == ImageMode_RGBA32)
	{
		uint32_t* dst = (uint32_t*)buffer;
		uint32_t* colors = entry->RGB32Palette;
		uint8_t firstColor = entry->firstColor;
		if (paletteMode == ColorPaletteMode_CGA)
			colors = DMG_GetCGAMode(entry) == CGA_Red ? CGAPaletteRed : CGAPaletteCyan;
		else if (paletteMode == ColorPaletteMode_EGA)
			colors = EGAPalette;
		else if (colors == 0)
			colors = DMG_GetEntryPalette(dmg, index);

		for (int32_t i = (int32_t)indexedSize - 1; i >= 0; i--)
		{
			uint8_t color = buffer[i];
			if (paletteMode == ColorPaletteMode_Native && color < firstColor)
				dst[i] = color < 16 ? DefaultPalette[color] : 0xFF000000;
			else if (paletteMode == ColorPaletteMode_Native && color > entry->lastColor)
				dst[i] = 0xFF000000;
			else
				dst[i] = colors[paletteMode == ColorPaletteMode_Native ? (color - firstColor) : color];
		}
		if (tempFileData)
			Free(tempFileData);
		return buffer;
	}

	if (mode == ImageMode_Planar || mode == ImageMode_PlanarST || mode == ImageMode_PlanarFalcon)
	{
		if (tempFileData)
			Free(tempFileData);
		DMG_SetError(DMG_ERROR_INVALID_IMAGE);
		return 0;
	}

	if (tempFileData)
		Free(tempFileData);
	DMG_SetError(DMG_ERROR_INVALID_IMAGE);
	return 0;
}

uint8_t* DMG_GetEntryData(DMG* dmg, uint8_t index, DMG_ImageMode mode)
{
	bool success;
	DMG_Entry* entry = DMG_GetEntry(dmg, index);
	if (entry == 0)
		return 0;
	if (entry->type == DMGEntry_Empty)
		return 0;

	DMG_ColorPaletteMode paletteMode = ColorPaletteMode_Native;
	if (dmg->screenMode == ScreenMode_CGA || dmg->colorMode == DAT5_COLORMODE_CGA)
		paletteMode = ColorPaletteMode_CGA;
	if (dmg->screenMode == ScreenMode_EGA || dmg->colorMode == DAT5_COLORMODE_EGA)
		paletteMode = ColorPaletteMode_EGA;

	uint8_t* buffer = 0;
	uint8_t* fileData;
	uint32_t bufferSize = 0;
	bool cacheNativeOnly = entry->type == DMGEntry_Image;
	DMG_ImageMode nativeImageMode = DMG_GetActiveNativeImageMode();
	bool allowCache = !cacheNativeOnly || mode == nativeImageMode;

	uint32_t requiredSize = DMG_CalculateRequiredSize(entry, mode);

	DMG_Cache* cache = allowCache ? DMG_GetImageCache(dmg, index, entry, requiredSize) : 0;
	if (cache)
	{
		if (cacheNativeOnly && cache->populated && cache->imageMode != nativeImageMode)
			DMG_AbortOnInvalidCachedImageMode(index, (DMG_ImageMode)cache->imageMode, mode);
		buffer = (uint8_t*)(cache + 1);
		if (cache->populated && cache->imageMode == mode)
			return buffer;
		bufferSize = cache->size;
	}

	if (buffer == 0 || bufferSize < requiredSize)
	{
		buffer = DMG_GetTemporaryBuffer(mode);
		bufferSize = DMG_GetTemporaryBufferSize();
		if (bufferSize < requiredSize)
		{
			if (mode == ImageMode_Audio && DMG_ReserveTemporaryBuffer(requiredSize))
			{
				buffer = DMG_GetTemporaryBuffer(mode);
				bufferSize = DMG_GetTemporaryBufferSize();
			}
			#if defined(_DOS) && !defined(__386__)
			else if (mode != ImageMode_RGBA32 && DMG_ReserveTemporaryBuffer(requiredSize))
			{
				buffer = DMG_GetTemporaryBufferBase();
				bufferSize = DMG_GetTemporaryBufferSize();
			}
			#endif
		}
		if (bufferSize < requiredSize)
		{
			DMG_Warning("Entry %d: Internal buffer too small for entry (%d bytes required)", index, requiredSize);
			DMG_SetError(DMG_ERROR_BUFFER_TOO_SMALL);
			return 0;
		}
	}
	if (buffer == 0)
	{
		// PANIC: No temporary buffer
		DMG_SetError(DMG_ERROR_BUFFER_TOO_SMALL);
		return 0;
	}

	if (dmg->version == DMG_Version5)
	{
		uint8_t* result = DMG_GetEntryDataV5(dmg, index, mode, entry, buffer, bufferSize);
		if (cache != 0)
		{
			if (cacheNativeOnly && mode != nativeImageMode)
				DMG_AbortOnInvalidCachedImageMode(index, mode, mode);
			cache->populated = result != 0;
			if (result != 0)
				cache->imageMode = mode;
		}
		return result;
	}

	uint32_t dataOffset = entry->fileOffset + 6;

	fileData = DMG_GetEntryStoredData(dmg, index) != 0 ? 0 : (uint8_t*)DMG_GetFromFileCache(dmg, dataOffset, entry->length);
	if (fileData == 0)
	{
		if (entry->length > bufferSize)
		{
			DMG_Warning("Entry %d: Invalid entry data (size too big for image dimensions)", index);
			DMG_SetError(DMG_ERROR_BUFFER_TOO_SMALL);
			return 0;
		}

		fileData = buffer + bufferSize - entry->length;
		if (DMG_GetEntryStoredData(dmg, index) != 0)
			MemCopy(fileData, DMG_GetEntryStoredData(dmg, index), entry->length);
		else
		if (DMG_ReadFromFile(dmg, dataOffset, fileData, entry->length) != entry->length)
		{
			DMG_SetError(DMG_ERROR_READING_FILE);
			return 0;
		}
	}

	if (mode == ImageMode_Audio)
		return fileData;

	if ((entry->flags & DMG_FLAG_COMPRESSED) != 0)
	{
		if (false)
		{
		}
		else if (dmg->version == DMG_Version1_PCW)
		{
			success = DMG_DecodePCWCompressedToPacked(fileData, entry->length, entry, buffer, DMG_CalculateRequiredSize(entry, ImageMode_Packed));
		}
		#if DMG_SUPPORT_EGA_SOURCES
		else if (dmg->version == DMG_Version1_EGA)
		{
			success = DMG_DecompressEGA(fileData, entry->length, buffer, entry->width, entry->height);
		}
		#endif
		#if DMG_SUPPORT_CGA_SOURCES
		else if (dmg->version == DMG_Version1_CGA)
		{
			success = DMG_DecompressCGA(fileData, entry->length, buffer, entry->width, entry->height);
		}
		#endif
		else
		{
			#if !DMG_SUPPORT_CROSS_ENDIAN_SOURCES
			if (!DMG_IsClassicNativeDATByteOrder(dmg->littleEndian))
			{
				if (cache != 0)
					cache->populated = false;
				DMG_SetError(DMG_ERROR_INVALID_IMAGE);
				return 0;
			}
			#endif
			uint16_t mask = read16(fileData, dmg->littleEndian);
			uint32_t pixels = entry->width * entry->height;
			if (dmg->version != DMG_Version2)
			{
				success = DMG_DecompressOldRLE(fileData+2, mask, 
					entry->length-2, buffer, pixels, dmg->littleEndian);
			}
			else if (DMG_IsClassicNativeDATByteOrder(dmg->littleEndian) && mode == ImageMode_PlanarST)
			{
				success = DMG_DecompressNewRLEToPlanarST(fileData+2, mask, 
					entry->length-2, buffer, entry->width, entry->height, dmg->littleEndian);
				if (success)
				{
					if (cache != 0)
					{
						cache->populated = true;
						cache->imageMode = mode;
					}
					return buffer;
				}
			}
			else
			{
				success = DMG_DecompressNewRLE(fileData+2, mask, 
					entry->length-2, buffer, pixels, dmg->littleEndian);
			}
		}
	}
	else
	{
		// TODO: Handle ImageMode_Planar
		uint32_t expectedSize = DMG_CalculateRequiredSize(entry, ImageMode_Raw);
		uint32_t packedSize = DMG_CalculateRequiredSize(entry, ImageMode_Packed);
		if (entry->length < expectedSize)
			success = false;
		else if (dmg->version == DMG_Version1_PCW)
			success = DMG_ExpandPCWStoredLayoutToPacked(fileData, entry->width, entry->height, buffer, packedSize);
		#if DMG_SUPPORT_EGA_SOURCES
		else if (dmg->version == DMG_Version1_EGA)
		{
			uint8_t* scratch = DMG_GetScratchBuffer(dmg, packedSize);
			uint32_t scratchSize = DMG_GetScratchBufferSize(dmg);
			success = DMG_UncEGAToPacked(fileData, entry->width, entry->height, buffer, packedSize, scratch, scratchSize);
		}
		#endif
		#if DMG_SUPPORT_CGA_SOURCES
		else if (dmg->version == DMG_Version1_CGA)
		{
			uint8_t* scratch = DMG_GetScratchBuffer(dmg, entry->length);
			uint32_t scratchSize = DMG_GetScratchBufferSize(dmg);
			success = DMG_UncCGAToPacked(fileData, entry->width, entry->height, buffer, packedSize, scratch, scratchSize);
		}
		#endif
		#if DMG_SUPPORT_CROSS_ENDIAN_SOURCES
		else if (dmg->littleEndian)
			success = DMG_CopyImageData(fileData, entry->length, buffer, packedSize);
		#endif
		else if (mode == ImageMode_PlanarST)
			return DMG_ConvertPlanar8ToPlanarST(fileData, buffer, packedSize, entry->width);
		else if (mode == ImageMode_PlanarFalcon)
			return DMG_ConvertPlanar8ToPlanarFalcon(fileData, buffer, packedSize, entry->width);
		else
			success = DMG_Planar8ToPacked(fileData, expectedSize, buffer, packedSize, entry->width);
	}
	if (success && mode != ImageMode_Packed)
	{
		uint32_t *ptr;
		uint32_t *colors;
		uint32_t n;
			
		switch (mode)
		{
			case ImageMode_Raw:
			case ImageMode_Packed:
				break;
			case ImageMode_Planar8:
				DMG_SetError(DMG_ERROR_INVALID_IMAGE);
				break;
			case ImageMode_PlanarFalcon:
				DMG_ConvertPackedToPlanarFalcon(buffer, requiredSize, entry->width);
				break;
			case ImageMode_PlanarST:
				DMG_ConvertPackedToPlanarST(buffer, requiredSize, entry->width);
				break;
			case ImageMode_Planar:
				DMG_SetError(DMG_ERROR_INVALID_IMAGE);
				break;
			case ImageMode_IndexedX:
				DMG_SetError(DMG_ERROR_INVALID_IMAGE);
				break;
			case ImageMode_CGA:
				DMG_SetError(DMG_ERROR_INVALID_IMAGE);
				break;
			case ImageMode_RGBA32:
			{
				if (bufferSize < requiredSize * 8)
				{
					DMG_Warning("Entry %d: Buffer too small for RGBA32 image (%d bytes required)", index, requiredSize * 4);
					if (cache != 0)
						cache->populated = false;
					DMG_SetError(DMG_ERROR_BUFFER_TOO_SMALL);
					return 0;
				}
				ptr = (uint32_t *)buffer + 2*requiredSize;
				if (paletteMode == ColorPaletteMode_CGA)
				{
					colors = DMG_GetCGAMode(entry) == CGA_Red ? CGAPaletteRed : CGAPaletteCyan;
					for (n = requiredSize - 1; n > 0; n--)
					{
						*--ptr = colors[buffer[n] & 0x0F];
						*--ptr = colors[buffer[n] >> 4];
					}
				}
				else if (paletteMode == ColorPaletteMode_EGA)
				{
					const uint8_t* egaPalette = DMG_GetEntryEGAPalette(entry);
					if (egaPalette == 0)
					{
						if (cache != 0)
							cache->populated = false;
						DMG_SetError(DMG_ERROR_INVALID_IMAGE);
						return 0;
					}
					for (n = requiredSize - 1; n > 0; n--)
					{
						*--ptr = egaPalette[buffer[n] & 0x0F];
						*--ptr = egaPalette[buffer[n] >> 4];
					}
				}
				else
				{
					colors = entry->RGB32Palette;
					for (n = requiredSize - 1; n > 0; n--)
					{
						*--ptr = colors[buffer[n] & 0x0F];
						*--ptr = colors[buffer[n] >> 4];
					}
				}
				break;
			}
			case ImageMode_Indexed:
			{
				const uint8_t* cgaPalette = DMG_GetEntryCGAPalette(entry);
				const uint8_t* egaPalette = DMG_GetEntryEGAPalette(entry);
				uint32_t pixelCount = (uint32_t)entry->width * (uint32_t)entry->height;
				if ((paletteMode == ColorPaletteMode_CGA && cgaPalette == 0) ||
					(paletteMode == ColorPaletteMode_EGA && egaPalette == 0))
				{
					if (cache != 0)
						cache->populated = false;
					DMG_SetError(DMG_ERROR_INVALID_IMAGE);
					return 0;
				}
				n = pixelCount;
				if ((n & 1u) != 0)
				{
					uint32_t pixel = n - 1;
					uint8_t packed = buffer[pixel >> 1];
					if (paletteMode == ColorPaletteMode_CGA)
						buffer[pixel] = cgaPalette[packed >> 4];
					else if (paletteMode == ColorPaletteMode_EGA)
						buffer[pixel] = egaPalette[packed >> 4];
					else
						buffer[pixel] = packed >> 4;
					n--;
				}
				uint8_t* ptr = n > 1 ? buffer + (n >> 1) - 1 : buffer;
				if (paletteMode == ColorPaletteMode_CGA)
				{
					while (n > 1)
					{
						uint8_t packed = *ptr--;
						buffer[--n] = cgaPalette[packed & 0x0F];
						buffer[--n] = cgaPalette[packed >> 4];
					}
				}
				else if (paletteMode == ColorPaletteMode_EGA)
				{
					while (n > 1)
					{
						uint8_t packed = *ptr--;
						buffer[--n] = egaPalette[packed & 0x0F];
						buffer[--n] = egaPalette[packed >> 4];
					}
				}
				else
				{
					while (n > 1)
					{
						uint8_t packed = *ptr--;
						buffer[--n] = packed & 0x0F;
						buffer[--n] = packed >> 4;
					}
				}
				break;
			}
		}
	}
	if (!success)
	{
		if (cache != 0)
			cache->populated = false;
		if (DMG_GetError() == DMG_ERROR_NONE)
			DMG_SetError(DMG_ERROR_CORRUPTED_DATA_STREAM);
		return 0;
	}

	if (cache != 0)
	{
		if (cacheNativeOnly && mode != nativeImageMode)
			DMG_AbortOnInvalidCachedImageMode(index, mode, mode);
		cache->imageMode = mode;
		cache->populated = true;
	}

	return buffer;
}

uint8_t* DMG_GetEntryDataNative(DMG* dmg, uint8_t index)
{
	const VID_ScreenAdapterInfo* info = VID_ScreenGetInfo();
	if (info != 0)
	{
		#if defined(_DOS)
		if (info->nativeImageMode == ImageMode_CGA)
			return DMG_GetEntryDataCGA(dmg, index);
		if (info->nativeImageMode == ImageMode_IndexedX)
			return DMG_GetEntryDataIndexedX(dmg, index);
		if (info->nativeImageMode == ImageMode_Planar)
			return DMG_GetEntryDataPlanar(dmg, index);
		#endif
		#if defined(_AMIGA)
		if (info->nativeImageMode == ImageMode_Planar)
			return DMG_GetEntryDataPlanar(dmg, index);
		#endif
		return DMG_GetEntryData(dmg, index, info->nativeImageMode);
	}

	#if defined(_AMIGA)
	return DMG_GetEntryDataPlanar(dmg, index);
	#elif defined(_DOS)
	return DMG_GetEntryDataIndexedX(dmg, index);
	#else
	return DMG_GetEntryData(dmg, index, DMG_NATIVE_IMAGE_MODE);
	#endif
}
