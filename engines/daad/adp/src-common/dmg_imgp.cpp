#include <dmg.h>
#include <ddb.h>
#include <ddb_pal.h>
#include <ddb_vid.h>
#include <os_mem.h>
#include <os_lib.h>
#include <os_bito.h>

#ifndef DEBUG_AMIGA_PICTURE_IO
#define DEBUG_AMIGA_PICTURE_IO 1
#endif

#if defined(_AMIGA) && HAS_ASM_RLE
extern "C" void DecompressRLE (const void* data, uint32_t dataSize,
	void* output, uint32_t width, uint32_t height, uint16_t rleMask);
#else
extern "C" void DecompressRLE (const void* data, uint32_t dataSize,
	void* output, uint32_t width, uint16_t rleMask);
#endif

#if (defined(_AMIGA) || defined(_ATARIST)) && HAS_ASM_RLE
extern "C" bool DecompressOldRLEToPlanar8Asm(const void* data, uint32_t dataSize,
	void* output, uint32_t pixels, uint16_t rleMask);
#endif

bool DMG_ConvertPlanarSTToPlanar(const DMG_Entry* entry, uint8_t* data, uint32_t dataSize, uint8_t* scratch, uint32_t scratchSize)
{
	if (entry == 0 || data == 0 || scratch == 0)
		return false;
	if (entry->bitDepth != 0 && entry->bitDepth != 4)
		return false;

	uint32_t widthWords = (uint32_t)(entry->width + 15) >> 4;
	uint32_t planeStrideWords = widthWords * entry->height;
	uint32_t planeStrideBytes = planeStrideWords * sizeof(uint16_t);
	uint32_t totalWords = planeStrideWords * 4;
	uint32_t totalBytes = totalWords * sizeof(uint16_t);
	if (totalBytes == 0 || totalBytes > dataSize || totalBytes > scratchSize)
		return false;

	const uint32_t* src = (const uint32_t*)data;
	uint8_t* dst0 = scratch;
	uint8_t* dst1 = dst0 + planeStrideBytes;
	uint8_t* dst2 = dst1 + planeStrideBytes;
	uint8_t* dst3 = dst2 + planeStrideBytes;

	for (uint32_t n = 0; n < planeStrideWords; n++)
	{
		uint32_t p0p1 = *src++;
		uint32_t p2p3 = *src++;
		uint32_t dstOffset = n << 1;
		dst0[dstOffset + 0] = (uint8_t)(p0p1 >> 24);
		dst0[dstOffset + 1] = (uint8_t)(p0p1 >> 16);
		dst1[dstOffset + 0] = (uint8_t)(p0p1 >> 8);
		dst1[dstOffset + 1] = (uint8_t)p0p1;
		dst2[dstOffset + 0] = (uint8_t)(p2p3 >> 24);
		dst2[dstOffset + 1] = (uint8_t)(p2p3 >> 16);
		dst3[dstOffset + 0] = (uint8_t)(p2p3 >> 8);
		dst3[dstOffset + 1] = (uint8_t)p2p3;
	}

	MemCopy(data, scratch, totalBytes);
	return true;
}

bool DMG_ConvertPlanar8ToPlanar(const DMG_Entry* entry, const uint8_t* data,
	uint32_t packedSize, uint8_t* output, uint32_t outputSize)
{
	if (entry == 0 || data == 0 || output == 0)
		return false;
	if (entry->bitDepth != 0 && entry->bitDepth != 4)
		return false;

	uint32_t widthWords = (uint32_t)(entry->width + 15) >> 4;
	uint32_t widthBlocks = (uint32_t)(entry->width + 7) >> 3;
	uint32_t planeStrideWords = widthWords * entry->height;
	uint32_t planeStrideBytes = planeStrideWords * sizeof(uint16_t);
	uint32_t totalBytes = planeStrideBytes * 4;
	uint32_t expectedPackedSize = widthBlocks * entry->height * 4;
	if (totalBytes == 0 || totalBytes > outputSize || packedSize != expectedPackedSize)
		return false;

	uint8_t* dst0 = output;
	uint8_t* dst1 = dst0 + planeStrideBytes;
	uint8_t* dst2 = dst1 + planeStrideBytes;
	uint8_t* dst3 = dst2 + planeStrideBytes;
	const uint8_t* src = data;

	for (uint32_t y = 0; y < entry->height; y++)
	{
		uint32_t rowWordOffset = y * widthWords * 2;
		for (uint32_t word = 0, block = 0; word < widthWords; word++, block += 2)
		{
			uint32_t dstOffset = rowWordOffset + word * 2;

			dst0[dstOffset] = *src++;
			dst1[dstOffset] = *src++;
			dst2[dstOffset] = *src++;
			dst3[dstOffset] = *src++;

			if (block + 1 < widthBlocks)
			{
				dst0[dstOffset + 1] = *src++;
				dst1[dstOffset + 1] = *src++;
				dst2[dstOffset + 1] = *src++;
				dst3[dstOffset + 1] = *src++;
			}
			else
			{
				dst0[dstOffset + 1] = 0;
				dst1[dstOffset + 1] = 0;
				dst2[dstOffset + 1] = 0;
				dst3[dstOffset + 1] = 0;
			}
		}
	}

	return true;
}

static bool DMG_ConvertPackedToPlanar(const DMG_Entry* entry, const uint8_t* data,
	uint32_t packedSize, uint8_t* output, uint32_t outputSize, const uint8_t* paletteMap)
{
	if (entry == 0 || data == 0 || output == 0)
		return false;
	if (entry->bitDepth != 0 && entry->bitDepth != 4)
		return false;

	uint32_t width = entry->width;
	uint32_t height = entry->height;
	uint32_t expectedPackedSize = ((uint32_t)width * height + 1u) >> 1;
	uint32_t stride = ((width + 15u) >> 4) * 2u;
	uint32_t planeSize = stride * height;
	uint32_t totalBytes = planeSize * 4u;
	if (packedSize != expectedPackedSize || totalBytes == 0 || totalBytes > outputSize)
		return false;

	MemClear(output, totalBytes);
	for (uint32_t y = 0; y < height; y++)
	{
		uint8_t* dst0 = output + planeSize * 0u + y * stride;
		uint8_t* dst1 = output + planeSize * 1u + y * stride;
		uint8_t* dst2 = output + planeSize * 2u + y * stride;
		uint8_t* dst3 = output + planeSize * 3u + y * stride;
		for (uint32_t x = 0; x < width; x++)
		{
			uint32_t pixel = y * width + x;
			uint8_t packed = data[pixel >> 1];
			uint8_t color = (pixel & 1u) == 0 ? (packed >> 4) : (packed & 0x0F);
			if (paletteMap != 0)
				color = paletteMap[color & 0x0F];
			uint8_t mask = (uint8_t)(0x80 >> (x & 7u));
			uint32_t byteX = x >> 3;
			if ((color & 0x01) != 0) dst0[byteX] |= mask;
			if ((color & 0x02) != 0) dst1[byteX] |= mask;
			if ((color & 0x04) != 0) dst2[byteX] |= mask;
			if ((color & 0x08) != 0) dst3[byteX] |= mask;
		}
	}

	return true;
}

static bool DMG_EnsureDAT5PaletteFromPayload(DMG_Entry* entry, const uint8_t* payload)
{
	if (entry == 0 || entry->paletteColors == 0 || entry->paletteSize == 0 || DMG_IsPaletteDecoded(entry))
        return true;

	if (entry->RGB32Palette == 0)
    {
        DMG_SetError(DMG_ERROR_OUT_OF_MEMORY);
        return false;
    }

    for (uint16_t p = 0; p < entry->paletteColors; p++)
    {
		entry->RGB32Palette[p] =
            0xFF000000UL |
            ((uint32_t)payload[p * 3 + 0] << 16) |
            ((uint32_t)payload[p * 3 + 1] << 8) |
            (uint32_t)payload[p * 3 + 2];
    }
	DMG_SetPaletteDecoded(entry, true);
    return true;
}

static uint8_t dmgDAT5PaletteScratch[256 * 3];

static uint8_t* DMG_GetEntryDataPlanarV5(DMG* dmg, uint8_t index, DMG_Entry* entry, uint8_t* buffer, uint32_t bufferSize)
{
	#if defined(_AMIGA) && DEBUG_AMIGA_PICTURE_IO
	DebugPrintf("DMG_GetEntryDataPlanarV5(%u): begin len=%lu buf=%p bufSize=%lu mode=%u bitDepth=%u size=%ux%u flags=%u palette=%u\n",
		(unsigned)index,
		(unsigned long)entry->length,
		buffer,
		(unsigned long)bufferSize,
		(unsigned)dmg->colorMode,
		(unsigned)entry->bitDepth,
		(unsigned)entry->width,
		(unsigned)entry->height,
		(unsigned)entry->flags,
		(unsigned)entry->paletteColors);
	#endif

	if (entry->type != DMGEntry_Image)
		return 0;

	if (!DMG_DAT5ModeIsPlaneMajor(dmg->colorMode))
	{
		DMG_SetError(DMG_ERROR_INVALID_IMAGE);
		return 0;
	}

	uint32_t requiredSize = DMG_DAT5StoredImageSize(dmg->colorMode, entry->width, entry->height);
    uint32_t paletteBytes = entry->paletteColors * 3;
    uint32_t imageDataLength = entry->length >= paletteBytes ? entry->length - paletteBytes : 0;
	#if defined(_AMIGA) && DEBUG_AMIGA_PICTURE_IO
	DebugPrintf("DMG_GetEntryDataPlanarV5(%u): required=%lu paletteBytes=%lu imageBytes=%lu\n",
		(unsigned)index,
		(unsigned long)requiredSize,
		(unsigned long)paletteBytes,
		(unsigned long)imageDataLength);
	#endif
	if (requiredSize == 0)
	{
		DMG_SetError(DMG_ERROR_INVALID_IMAGE);
		return 0;
	}
	if (requiredSize > bufferSize)
	{
		DMG_SetError(DMG_ERROR_BUFFER_TOO_SMALL);
		return 0;
	}
    if (entry->length < paletteBytes)
    {
        DMG_SetError(DMG_ERROR_CORRUPTED_DATA_STREAM);
        return 0;
    }

	if ((entry->flags & DMG_FLAG_COMPRESSED) != 0)
	{
		if ((entry->flags & DMG_FLAG_ZX0) == 0)
		{
			DMG_SetError(DMG_ERROR_INVALID_IMAGE);
			return 0;
		}

		const uint8_t* payloadData = (const uint8_t*)DMG_GetFromFileCache(dmg, entry->fileOffset, entry->length);
		uint8_t* scratch = 0;
		#if defined(_AMIGA) && DEBUG_AMIGA_PICTURE_IO
		DebugPrintf("DMG_GetEntryDataPlanarV5(%u): compressed payload cache=%p offset=%lu len=%lu\n",
			(unsigned)index,
			payloadData,
			(unsigned long)entry->fileOffset,
			(unsigned long)entry->length);
		#endif
		if (payloadData == 0)
		{
			if (bufferSize >= requiredSize + entry->length)
				scratch = buffer + bufferSize - entry->length;
			else if (DMG_GetScratchBufferSize(dmg) >= entry->length)
				scratch = DMG_GetScratchBuffer(dmg, entry->length);
			else
			{
				#if defined(_AMIGA) && DEBUG_AMIGA_PICTURE_IO
				DebugPrintf("DMG_GetEntryDataPlanarV5(%u): no scratch buffer for compressed payload len=%lu bufSize=%lu scratchSize=%lu\n",
					(unsigned)index,
					(unsigned long)entry->length,
					(unsigned long)bufferSize,
					(unsigned long)DMG_GetScratchBufferSize(dmg));
				#endif
				DMG_SetError(DMG_ERROR_BUFFER_TOO_SMALL);
				return 0;
			}

			#if defined(_AMIGA) && DEBUG_AMIGA_PICTURE_IO
			DebugPrintf("DMG_GetEntryDataPlanarV5(%u): compressed scratch=%p source=%s\n",
				(unsigned)index,
				scratch,
				bufferSize >= requiredSize + entry->length ? "tail" : "zx0");
			#endif

			if (DMG_ReadFromFile(dmg, entry->fileOffset, scratch, entry->length) != entry->length)
			{
				#if defined(_AMIGA) && DEBUG_AMIGA_PICTURE_IO
				DebugPrintf("DMG_GetEntryDataPlanarV5(%u): compressed read failed offset=%lu len=%lu\n",
					(unsigned)index,
					(unsigned long)entry->fileOffset,
					(unsigned long)entry->length);
				#endif
				DMG_SetError(DMG_ERROR_READING_FILE);
				return 0;
			}
			payloadData = scratch;
			#if defined(_AMIGA) && DEBUG_AMIGA_PICTURE_IO
			DebugPrintf("DMG_GetEntryDataPlanarV5(%u): compressed payload read into %p\n",
				(unsigned)index,
				payloadData);
			#endif
		}

        if (!DMG_EnsureDAT5PaletteFromPayload(entry, payloadData))
            return 0;
		#if defined(_AMIGA) && DEBUG_AMIGA_PICTURE_IO
		DebugPrintf("DMG_GetEntryDataPlanarV5(%u): palette ok, decompressing ZX0 src=%p dst=%p bytes=%lu -> %lu\n",
			(unsigned)index,
			payloadData + paletteBytes,
			buffer,
			(unsigned long)imageDataLength,
			(unsigned long)requiredSize);
		#endif

		#if DEBUG_ZX0
		uint32_t t0;
		VID_GetMilliseconds(&t0);
		#endif

		if (!DMG_DecompressZX0(payloadData + paletteBytes, imageDataLength, buffer, requiredSize))
		{
			#if defined(_AMIGA) && DEBUG_AMIGA_PICTURE_IO
			DebugPrintf("DMG_GetEntryDataPlanarV5(%u): ZX0 decode failed error=%d (%s)\n",
				(unsigned)index,
				(int)DMG_GetError(),
				DMG_GetErrorString());
			#endif
			DMG_SetError(DMG_ERROR_CORRUPTED_DATA_STREAM);
			return 0;
		}
		#if DEBUG_ZX0
		uint32_t t1;
		VID_GetMilliseconds(&t1);
		DebugPrintf("DMG_GetEntryDataPlanarV5(%u): ZX0 decode %lu -> %lu bytes in %lu ms\n",
			(unsigned)index,
			(unsigned long)imageDataLength,
			(unsigned long)requiredSize,
			(unsigned long)(t1 - t0));
		#endif
		#if defined(_AMIGA) && DEBUG_AMIGA_PICTURE_IO
		DebugPrintf("DMG_GetEntryDataPlanarV5(%u): ZX0 decode returned buffer=%p\n",
			(unsigned)index,
			buffer);
		#endif
		return buffer;
	}

	if (imageDataLength != requiredSize)
	{
		DMG_SetError(DMG_ERROR_CORRUPTED_DATA_STREAM);
		return 0;
	}

	const uint8_t* payloadData = (const uint8_t*)DMG_GetFromFileCache(dmg, entry->fileOffset, entry->length);
	#if defined(_AMIGA) && DEBUG_AMIGA_PICTURE_IO
	DebugPrintf("DMG_GetEntryDataPlanarV5(%u): raw payload cache=%p offset=%lu len=%lu\n",
		(unsigned)index,
		payloadData,
		(unsigned long)entry->fileOffset,
		(unsigned long)entry->length);
	#endif
	if (payloadData != 0)
	{
        if (!DMG_EnsureDAT5PaletteFromPayload(entry, payloadData))
            return 0;
		#if defined(_AMIGA) && DEBUG_AMIGA_PICTURE_IO
		DebugPrintf("DMG_GetEntryDataPlanarV5(%u): copying raw payload src=%p dst=%p bytes=%lu\n",
			(unsigned)index,
			payloadData + paletteBytes,
			buffer,
			(unsigned long)requiredSize);
		#endif
		MemCopy(buffer, payloadData + paletteBytes, requiredSize);
		#if defined(_AMIGA) && DEBUG_AMIGA_PICTURE_IO
		DebugPrintf("DMG_GetEntryDataPlanarV5(%u): raw copy complete\n", (unsigned)index);
		#endif
		return buffer;
	}

	if (paletteBytes > sizeof(dmgDAT5PaletteScratch))
	{
		DMG_SetError(DMG_ERROR_INVALID_IMAGE);
		return 0;
	}
	if (paletteBytes != 0 && !DMG_IsPaletteDecoded(entry))
	{
		if (DMG_ReadFromFile(dmg, entry->fileOffset, dmgDAT5PaletteScratch, paletteBytes) != paletteBytes)
		{
			DMG_SetError(DMG_ERROR_READING_FILE);
			return 0;
		}
		if (!DMG_EnsureDAT5PaletteFromPayload(entry, dmgDAT5PaletteScratch))
			return 0;
	}
	if (DMG_ReadFromFile(dmg, entry->fileOffset + paletteBytes, buffer, requiredSize) != requiredSize)
	{
		DMG_SetError(DMG_ERROR_READING_FILE);
		return 0;
	}
	#if defined(_AMIGA) && DEBUG_AMIGA_PICTURE_IO
	DebugPrintf("DMG_GetEntryDataPlanarV5(%u): raw image read direct to %p (%lu bytes)\n",
		(unsigned)index,
		buffer,
		(unsigned long)requiredSize);
	#endif
	return buffer;
}

bool DMG_DecompressNewRLEToPlanarST (const uint8_t* d, uint16_t rleMask, 
	uint16_t dataLength, uint8_t* buffer, uint32_t width, uint32_t height, bool littleEndian)
{
#if defined(_AMIGA) && HAS_ASM_RLE
	DecompressRLE(d, dataLength, buffer, width, height, rleMask);
	return true;
#elif HAS_ASM_RLE
	(void)height;
	DecompressRLE(d, dataLength, buffer, width, rleMask);
	return true;
#else
	(void)height;
	// WARNING! Non-ASM RLE decompression is buggy, please use ASM implementaiton

	uint32_t nibbles;
	uint8_t repetitions;
	const uint32_t* data = (const uint32_t*) d;
	const uint32_t* end = (const uint32_t*)(d + ((dataLength+3)&~3));
	int nibbleCount = 0;

	uint32_t* out = (uint32_t*)buffer;
	uint32_t p0 = 0x00000000;
	uint32_t p1 = 0x00000000;
	uint32_t x0 = 0x80000000;
	uint32_t x1 = 0x00008000;
	uint32_t xc = 0;

	while (data < end)
	{
		if (nibbleCount == 0)
		{
			nibbles = fix32(*data++);
			nibbleCount = 7;
		}
		else
		{
			nibbles >>= 4;
			nibbleCount--;
		}
		
		if (nibbles & 0x01) p0 |= x0;
		if (nibbles & 0x02) p0 |= x1;
		if (nibbles & 0x04) p1 |= x0;
		if (nibbles & 0x08) p1 |= x1;
		x0 >>= 1;
		x1 >>= 1;
		xc++;
		if (!x1 || xc == width)
		{
			*out++ = p0;
			*out++ = p1;
			p0 = p1 = 0;
			x0 = 0x80000000;
			x1 = 0x00008000;
			if (xc == width)
				xc = 0;
		}

		if (rleMask & (1 << (nibbles & 0x0F))) 
		{
			uint32_t m0 = nibbles & 0x01 ? 0xFFFF0000 : 0;
			uint32_t m1 = nibbles & 0x04 ? 0xFFFF0000 : 0;
			if (nibbles & 0x02) m0 |= 0xFFFF;
			if (nibbles & 0x08) m1 |= 0xFFFF;

			if (nibbleCount == 0)
			{
				nibbles = fix32(*data++);
				nibbleCount = 7;
			}
			else
			{
				nibbles >>= 4;
				nibbleCount--;
			}

			repetitions = nibbles & 0x0F;

			while (repetitions--)
			{
				p0 |= m0 & (x0 | x1);
				p1 |= m1 & (x0 | x1);
				x0 >>= 1;
				x1 >>= 1;
				xc++;
				if (!x1 || xc == width)
				{
					*out++ = p0;
					*out++ = p1;
					p0 = p1 = 0;
					x0 = 0x80000000;
					x1 = 0x00008000;
					if (xc == width)
						xc = 0;
				}
			}
		}
	}
	if (x1 != 0x8000)
	{
		*out++ = p0;
		*out++ = p1;
	}
	return true;
#endif
}

#if defined(_AMIGA) && HAS_ASM_RLE
static bool DMG_DecompressOldRLEToNativeAmiga(const DMG_Entry* entry, DMG* dmg,
	const uint8_t* fileData, bool fileDataSharesOutputBuffer,
	uint8_t* buffer, uint32_t bufferSize, uint32_t requiredSize,
	uint16_t mask, DMG_ImageMode* outputMode)
{
	const uint8_t* oldRLEData = fileData + 2;
	uint32_t prototypePackedSize = ((uint32_t)(entry->width + 7) >> 3) * entry->height * 4;
	uint32_t prototypePixels = prototypePackedSize * 2;
	if (prototypePackedSize > requiredSize)
	{
		DMG_SetError(DMG_ERROR_INVALID_IMAGE);
		return false;
	}

	uint8_t* oldRLEOutput = 0;
	if (!fileDataSharesOutputBuffer && bufferSize >= requiredSize + prototypePackedSize)
		oldRLEOutput = buffer + requiredSize;
	else if (fileDataSharesOutputBuffer && bufferSize >= requiredSize + entry->length + prototypePackedSize)
		oldRLEOutput = buffer + requiredSize;
	else if (DMG_GetScratchBufferSize(dmg) >= prototypePackedSize)
		oldRLEOutput = DMG_GetScratchBuffer(dmg, prototypePackedSize);
	else
	{
		DMG_SetError(DMG_ERROR_BUFFER_TOO_SMALL);
		return false;
	}

	if (!DecompressOldRLEToPlanar8Asm(oldRLEData, entry->length - 2, oldRLEOutput, prototypePixels, mask))
	{
		DMG_SetError(DMG_ERROR_CORRUPTED_DATA_STREAM);
		return false;
	}

	if (!DMG_ConvertPlanar8ToPlanar(entry, oldRLEOutput, prototypePackedSize, buffer, requiredSize))
	{
		DMG_SetError(DMG_ERROR_BUFFER_TOO_SMALL);
		return false;
	}

	*outputMode = ImageMode_Planar;
	return true;
}
#endif

#if defined(_ATARIST) && HAS_ASM_RLE
static bool DMG_DecompressOldRLEToPlanarSTAsm(const DMG_Entry* entry, DMG* dmg,
	const uint8_t* fileData, bool fileDataSharesOutputBuffer,
	uint8_t* buffer, uint32_t bufferSize, uint32_t requiredSize,
	uint16_t mask, DMG_ImageMode* outputMode)
{
	(void)dmg;
	(void)fileDataSharesOutputBuffer;
	(void)bufferSize;
	(void)requiredSize;

	const uint8_t* oldRLEData = fileData + 2;
	uint32_t prototypePackedSize = ((uint32_t)(entry->width + 7) >> 3) * entry->height * 4;
	uint32_t prototypePixels = prototypePackedSize * 2;

	if (!DecompressOldRLEToPlanar8Asm(oldRLEData, entry->length - 2, buffer, prototypePixels, mask))
	{
		DMG_SetError(DMG_ERROR_CORRUPTED_DATA_STREAM);
		return false;
	}

	DMG_ConvertPlanar8ToPlanarST(buffer, buffer, prototypePackedSize, entry->width);
	*outputMode = ImageMode_PlanarST;
	return true;
}
#endif

uint8_t* DMG_GetEntryDataPlanar (DMG* dmg, uint8_t index)
{
	bool success;
	#if defined(_AMIGA) && DEBUG_AMIGA_PICTURE_IO
	uint32_t planarStart, planarStep;
	VID_GetMilliseconds(&planarStart);
	planarStep = planarStart;
	#endif
	DMG_Entry* entry = DMG_GetEntry(dmg, index);
	#if defined(_AMIGA) && DEBUG_AMIGA_PICTURE_IO
	uint32_t entryLookupMs;
	VID_GetMilliseconds(&entryLookupMs);
	DebugPrintf("DMG_GetEntryDataPlanar(%u): entry lookup %lu ms entry=%p\n",
		(unsigned)index,
		(unsigned long)(entryLookupMs - planarStep),
		entry);
	planarStep = entryLookupMs;
	#endif
	if (entry == 0)
		return 0;
	if (entry->type == DMGEntry_Empty)
		return 0;

	uint8_t* buffer = 0;
	uint8_t* fileData;
	uint32_t bufferSize = 0;
	DMG_Cache* cache = 0;

	bool destinationIsImageCache = false;
	bool fileDataSharesOutputBuffer = false;

	#if defined(_AMIGA) || defined(_DOS)
	DMG_ImageMode nativePlanarMode = ImageMode_Planar;
	#else
	DMG_ImageMode nativePlanarMode =
		(dmg != 0 && dmg->version == DMG_Version5 && DMG_DAT5ModeIsPlaneMajor(dmg->colorMode)) ?
		ImageMode_Planar :
		ImageMode_PlanarST;
	#endif
	unsigned requiredSize = DMG_CalculateRequiredSize(entry, nativePlanarMode);
	#if defined(_AMIGA) && DEBUG_AMIGA_PICTURE_IO
	DebugPrintf("DMG_GetEntryDataPlanar(%u): required=%u mode=%u version=%u colorMode=%u\n",
		(unsigned)index,
		requiredSize,
		(unsigned)nativePlanarMode,
		(unsigned)(dmg != 0 ? dmg->version : 0),
		(unsigned)(dmg != 0 ? dmg->colorMode : 0));
	#endif

	#if defined(_AMIGA) && DEBUG_AMIGA_PICTURE_IO
	VID_GetMilliseconds(&planarStep);
	#endif
	cache = DMG_GetImageCache (dmg, index, entry, requiredSize);
	#if defined(_AMIGA) && DEBUG_AMIGA_PICTURE_IO
	uint32_t cacheMs;
	VID_GetMilliseconds(&cacheMs);
	DebugPrintf("DMG_GetEntryDataPlanar(%u): cache=%p lookup=%lu ms\n",
		(unsigned)index,
		cache,
		(unsigned long)(cacheMs - planarStep));
	planarStep = cacheMs;
	#endif
	if (cache)
	{
		buffer = (uint8_t*)(cache + 1);
		if (cache->populated && cache->imageMode == nativePlanarMode)
		{
			#if defined(_AMIGA) && DEBUG_AMIGA_PICTURE_IO
			uint32_t cacheHitMs;
			VID_GetMilliseconds(&cacheHitMs);
			DebugPrintf("DMG_GetEntryDataPlanar(%u): cache hit buffer=%p total=%lu ms\n",
				(unsigned)index,
				buffer,
				(unsigned long)(cacheHitMs - planarStart));
			#endif
			return buffer;
		}
		bufferSize = cache->size;
		destinationIsImageCache = true;
	}

	if (buffer == 0 || bufferSize < requiredSize)
	{
		buffer = DMG_GetTemporaryBuffer(nativePlanarMode);
		bufferSize = DMG_GetTemporaryBufferSize();
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
		DMG_SetError(DMG_ERROR_TEMPORARY_BUFFER_MISSING);
		return 0;
	}

	if (dmg->version == DMG_Version5)
	{
		#if defined(_AMIGA) && DEBUG_AMIGA_PICTURE_IO
		DebugPrintf("DMG_GetEntryDataPlanar(%u): entering V5 decoder buffer=%p size=%lu\n",
			(unsigned)index,
			buffer,
			(unsigned long)bufferSize);
		#endif
		uint8_t* result = DMG_GetEntryDataPlanarV5(dmg, index, entry, buffer, bufferSize);
		#if defined(_AMIGA) && DEBUG_AMIGA_PICTURE_IO
		DebugPrintf("DMG_GetEntryDataPlanar(%u): V5 decoder result=%p error=%d\n",
			(unsigned)index,
			result,
			(int)DMG_GetError());
		#endif
		if (result != 0 && cache != 0)
		{
			cache->populated = true;
			cache->imageMode = ImageMode_Planar;
		}
		return result;
	}

	#if defined(_AMIGA) && DEBUG_AMIGA_PICTURE_IO
	VID_GetMilliseconds(&planarStep);
	#endif
	fileData = (uint8_t*)DMG_GetFromFileCache(dmg, entry->fileOffset+6, entry->length);
	#if defined(_AMIGA) && DEBUG_AMIGA_PICTURE_IO
	uint32_t fileCacheMs;
	VID_GetMilliseconds(&fileCacheMs);
	DebugPrintf("DMG_GetEntryDataPlanar(%u): file cache %s offset=%lu length=%lu in %lu ms\n",
		(unsigned)index,
		fileData != 0 ? "hit" : "miss",
		(unsigned long)(entry->fileOffset + 6),
		(unsigned long)entry->length,
		(unsigned long)(fileCacheMs - planarStep));
	planarStep = fileCacheMs;
	#endif
	if (fileData == 0)
	{
		if (entry->length > bufferSize)
		{
			DMG_Warning("Entry %d: Invalid entry data (size too big for image dimensions)", index);
			DMG_SetError(DMG_ERROR_CORRUPTED_DATA_STREAM);
			return 0;
		}

		if (destinationIsImageCache)
		{
			if (!DMG_ReserveTemporaryBuffer(entry->length))
			{
				DMG_Warning("Entry %d: Internal buffer too small for entry (%d bytes required)", index, requiredSize);
				DMG_SetError(DMG_ERROR_BUFFER_TOO_SMALL);
				return 0;
			}
			fileData = DMG_GetTemporaryBufferBase();
		}
		else
		{
			DebugPrintf("WARNING: file data will be read into image output buffer (!)\n");
			fileData = buffer + bufferSize - entry->length;
			fileDataSharesOutputBuffer = true;
		}
		#if defined(_AMIGA) && DEBUG_AMIGA_PICTURE_IO
		uint32_t readStart;
		VID_GetMilliseconds(&readStart);
		#endif
		if (DMG_ReadFromFile(dmg, entry->fileOffset + 6, fileData, entry->length) != entry->length)
		{
			DMG_SetError(DMG_ERROR_READING_FILE);
			return 0;
		}
		#if defined(_AMIGA) && DEBUG_AMIGA_PICTURE_IO
		uint32_t readEnd;
		VID_GetMilliseconds(&readEnd);
		DebugPrintf("DMG_GetEntryDataPlanar(%u): file read length=%lu in %lu ms into %p\n",
			(unsigned)index,
			(unsigned long)entry->length,
			(unsigned long)(readEnd - readStart),
			fileData);
		planarStep = readEnd;
		#endif
	}

	DMG_ImageMode outputMode = ImageMode_PlanarST;
	uint8_t* scratch = 0;
	uint32_t scratchSize = 0;

	if ((entry->flags & DMG_FLAG_COMPRESSED) != 0)
	{
		if (false)
		{
		}
		#if DMG_SUPPORT_EGA_SOURCES
		else if (dmg->version == DMG_Version1_EGA)
		{
			success = DMG_DecompressEGA(fileData, entry->length, buffer, entry->width, entry->height);
			outputMode = ImageMode_Packed;
		}
		#endif
		#if DMG_SUPPORT_CGA_SOURCES
		else if (dmg->version == DMG_Version1_CGA)
		{
			success = DMG_DecompressCGA(fileData, entry->length, buffer, entry->width, entry->height);
			outputMode = ImageMode_Packed;
		}
		#endif
		else
		{
			#if !DMG_SUPPORT_CROSS_ENDIAN_SOURCES
			if (!DMG_IsClassicNativeDATByteOrder(dmg->littleEndian))
			{
				DMG_SetError(DMG_ERROR_INVALID_IMAGE);
				if (cache != 0)
					cache->populated = false;
				return 0;
			}
			#endif
			uint16_t mask = read16(fileData, dmg->littleEndian);
			if (dmg->version != DMG_Version2)
			{
				#ifdef DEBUG_RLE
				uint32_t t0, t1;
				VID_GetMilliseconds(&t0);
				#endif

				#if defined(_AMIGA) && HAS_ASM_RLE
				success = DMG_DecompressOldRLEToNativeAmiga(entry, dmg, fileData,
					fileDataSharesOutputBuffer, buffer, bufferSize, requiredSize, mask, &outputMode);
				#elif defined(_ATARIST) && HAS_ASM_RLE
				success = DMG_DecompressOldRLEToPlanarSTAsm(entry, dmg, fileData,
					fileDataSharesOutputBuffer, buffer, bufferSize, requiredSize, mask, &outputMode);
				#else
				success = DMG_DecompressOldRLEToPlanarST(fileData+2, mask,
					entry->length-2, buffer, requiredSize*2, dmg->littleEndian);
				#endif

				#ifdef DEBUG_RLE
				VID_GetMilliseconds(&t1);
				if (outputMode == ImageMode_Planar)
				{
					DebugPrintf("Decompressed old RLE image %u directly to Planar in %lu ms\n",
						(unsigned)index, (unsigned long)(t1 - t0));
				}
				else
				{
					DebugPrintf("Decompressed old RLE image %u in %lu ms\n",
						(unsigned)index, (unsigned long)(t1 - t0));
				}
				#endif
			}
			else if (DMG_IsClassicNativeDATByteOrder(dmg->littleEndian))
			{
				#if defined(_AMIGA) && DEBUG_AMIGA_PICTURE_IO
				uint32_t t0, t1;
				VID_GetMilliseconds(&t0);
				#endif

				#if defined(_DOS)
				success = DMG_DecompressNewRLE(fileData+2, mask,
					entry->length-2, buffer, entry->width * entry->height, dmg->littleEndian);
				outputMode = ImageMode_Packed;
				#else
				success = DMG_DecompressNewRLEToPlanarST(fileData+2, mask, 
					entry->length-2, buffer, entry->width, entry->height, dmg->littleEndian);
				#if defined(_AMIGA) && HAS_ASM_RLE
				outputMode = ImageMode_Planar;
				#endif
				#endif
				
				#if defined(_AMIGA) && DEBUG_AMIGA_PICTURE_IO
				VID_GetMilliseconds(&t1);
				DebugPrintf("DMG_GetEntryDataPlanar(%u): DAT2 RLE decode success=%d outputMode=%u in %lu ms\n",
					(unsigned)index,
					success ? 1 : 0,
					(unsigned)outputMode,
					(unsigned long)(t1 - t0));
				planarStep = t1;
				#endif
			}
			else
			{
				#ifdef DEBUG_RLE
				uint32_t t0, t1;
				VID_GetMilliseconds(&t0);
				#endif

				success = DMG_DecompressNewRLE(fileData+2, mask, 
					entry->length-2, buffer, entry->width * entry->height, dmg->littleEndian);
				outputMode = ImageMode_Packed;

				#ifdef DEBUG_RLE
				VID_GetMilliseconds(&t1);
				DebugPrintf("Decompressed (non-native) new RLE image %u in %lu ms\n",
					(unsigned)index, (unsigned long)(t1 - t0));
				#endif
			}
		}
	}
	else
	{
		uint32_t expectedSize = DMG_CalculateRequiredSize(entry, ImageMode_Raw);
		uint32_t packedSize = DMG_CalculateRequiredSize(entry, ImageMode_Packed);
		if (entry->length < expectedSize)
		{
			DebugPrintf("WARNING: expectedSize(%lu) != packedSize(%lu) in image %u\n",
				(unsigned long)expectedSize, (unsigned long)packedSize, (unsigned)index);
			success = false;
		}
		#if DMG_SUPPORT_EGA_SOURCES
		else if (dmg->version == DMG_Version1_EGA)
		{
			scratch = DMG_GetScratchBuffer(dmg, packedSize);
			scratchSize = DMG_GetScratchBufferSize(dmg);
			success = DMG_UncEGAToPacked(fileData, entry->width, entry->height, buffer, packedSize, scratch, scratchSize);
			outputMode = ImageMode_Packed;
		}
		#endif
		#if DMG_SUPPORT_CGA_SOURCES
		else if (dmg->version == DMG_Version1_CGA)
		{
			scratch = DMG_GetScratchBuffer(dmg, entry->length);
			scratchSize = DMG_GetScratchBufferSize(dmg);
			success = DMG_UncCGAToPacked(fileData, entry->width, entry->height, buffer, packedSize, scratch, scratchSize);
			outputMode = ImageMode_Packed;
		}
		#endif
		#if DMG_SUPPORT_CROSS_ENDIAN_SOURCES
		else if (dmg->littleEndian)
		{
			success = DMG_CopyImageData(fileData, entry->length, buffer, packedSize);
			outputMode = ImageMode_Packed;
		}
		#endif
		else
		{
			#ifdef DEBUG_RLE
			uint32_t t0, t1;
			VID_GetMilliseconds(&t0);
			#endif

			DMG_ConvertPlanar8ToPlanarST(fileData, buffer, packedSize, entry->width);
			success = true;
			
			#ifdef DEBUG_RLE
			VID_GetMilliseconds(&t1);
			DebugPrintf("Converted raw planar image %u to PlanarST in %lu ms\n",
				(unsigned)index, (unsigned long)(t1 - t0));
			#endif
		}
	}

	if (success)
	{
		if (outputMode == ImageMode_Packed)
		{
			#ifdef DEBUG_RLE
			uint32_t t0, t1;
			VID_GetMilliseconds(&t0);
			#endif

			#if defined(_DOS)
			uint32_t packedSize = DMG_CalculateRequiredSize(entry, ImageMode_Packed);
			uint8_t* packedScratch = DMG_GetScratchBuffer(dmg, packedSize);
			if (packedScratch == 0 || DMG_GetScratchBufferSize(dmg) < packedSize)
			{
				DMG_SetError(DMG_ERROR_BUFFER_TOO_SMALL);
				if (cache != 0)
					cache->populated = false;
				return 0;
			}
			MemCopy(packedScratch, buffer, packedSize);
			if (!DMG_ConvertPackedToPlanar(entry, packedScratch, packedSize, buffer,
				requiredSize, DMG_GetEntryEGAPalette(entry)))
			{
				DMG_SetError(DMG_ERROR_BUFFER_TOO_SMALL);
				if (cache != 0)
					cache->populated = false;
				return 0;
			}
			outputMode = ImageMode_Planar;
			#else
			DMG_ConvertPackedToPlanarST(buffer, requiredSize, entry->width);
			outputMode = ImageMode_PlanarST;
			#endif
			
			#ifdef DEBUG_RLE
			VID_GetMilliseconds(&t1);
			DebugPrintf("Converted chunky image %u to planar in %lu ms\n",
				(unsigned)index, (unsigned long)(t1 - t0));
			#endif
		}

		#if defined(_AMIGA) || defined(_DOS)
		if (outputMode == ImageMode_PlanarST)
		{
			uint32_t t0, t1;
			VID_GetMilliseconds(&t0);
			uint8_t* scratch = DMG_GetScratchBuffer(dmg, requiredSize);
			if (scratch == 0 || !DMG_ConvertPlanarSTToPlanar(entry, buffer, requiredSize, scratch, DMG_GetScratchBufferSize(dmg)))
			{
				DMG_SetError(DMG_ERROR_BUFFER_TOO_SMALL);
				if (cache != 0)
					cache->populated = false;
				return 0;
			}
			VID_GetMilliseconds(&t1);
			DebugPrintf("Converted PlanarST image %u to Planar in %lu ms\n",
				(unsigned)index, (unsigned long)(t1 - t0));
			outputMode = ImageMode_Planar;
		}
		if (cache != 0)
			cache->imageMode = outputMode;
		#endif

		if (cache != 0)
			cache->populated = true;

		#if defined(_AMIGA) && DEBUG_AMIGA_PICTURE_IO
		uint32_t doneMs;
		VID_GetMilliseconds(&doneMs);
		DebugPrintf("DMG_GetEntryDataPlanar(%u): success outputMode=%u cache=%p populated=%d total=%lu ms sinceLast=%lu ms buffer=%p\n",
			(unsigned)index,
			(unsigned)outputMode,
			cache,
			cache != 0 && cache->populated ? 1 : 0,
			(unsigned long)(doneMs - planarStart),
			(unsigned long)(doneMs - planarStep),
			buffer);
		#endif
	}
	else
	{
		if (cache != 0)
			cache->populated = false;
		if (DMG_GetError() == DMG_ERROR_NONE)
			DMG_SetError(DMG_ERROR_CORRUPTED_DATA_STREAM);
		return 0;
	}

	return buffer;
}
