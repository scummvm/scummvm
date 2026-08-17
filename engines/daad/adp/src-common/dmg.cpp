#include <dmg.h>
#include <ddb.h>
#include <ddb_pal.h>
#include <ddb_vid.h>
#include <os_mem.h>
#include <os_lib.h>
#include <os_bito.h>

#if defined(_STDCLIB) && !defined(NO_PRINTF)
#include <stdarg.h>
#include <stdio.h>
#include <stdlib.h>
#define PRINTF_WARNINGS
#endif

#ifndef DEBUG_IMAGE_CACHE
#define DEBUG_IMAGE_CACHE 0
#endif

#define DMG_BUFFER_SIZE 	(640 * 400)
#define DMG_MIN_FILE_SIZE 	16
#define DMG_MAX_FILE_SIZE 	0x1000000

DMG* dmg = 0;

static DMG_Error dmgError = DMG_ERROR_NONE;
static void    (*dmg_warningHandler)(const char* message) = 0;
static uint8_t*  dmgTemporaryBufferRaw = 0;
static uint8_t*  dmgTemporaryBuffer = 0;
static uint32_t  dmgTemporaryBufferSize = 0;
#if defined(_STDCLIB) && !defined(NO_PRINTF)
static char      dmgWarningBuffer[1024];
#endif

void DMG_InitializeOldRLETables();

static void DMG_GetTargetSize(const DMG* d, uint32_t* width, uint32_t* height)
{
	uint32_t resolvedWidth = 640;
	uint32_t resolvedHeight = 400;

	if (d != 0)
	{
		resolvedWidth = d->targetWidth;
		resolvedHeight = d->targetHeight;
		if (resolvedWidth == 0 || resolvedHeight == 0)
		{
			switch (d->screenMode)
			{
				case ScreenMode_HiRes:
					resolvedWidth = 640;
					resolvedHeight = 200;
					break;
				case ScreenMode_SHiRes:
					resolvedWidth = 640;
					resolvedHeight = 400;
					break;
				default:
					resolvedWidth = 320;
					resolvedHeight = 200;
					break;
			}
		}
	}

	if (width != 0)
		*width = resolvedWidth;
	if (height != 0)
		*height = resolvedHeight;
}

static uint8_t DMG_GetNativePlaneCount(const DMG* d, DMG_ImageMode mode)
{
	switch (mode)
	{
		case ImageMode_Planar:
		case ImageMode_PlanarST:
		case ImageMode_Planar8:
		case ImageMode_PlanarFalcon:
			if (d != 0 && d->version == DMG_Version5)
			{
				uint8_t planes = DMG_DAT5ModePlaneCount(d->colorMode);
				if (planes != 0)
					return planes;
			}
			return 4;
		default:
			return 0;
	}
}

uint32_t DMG_GetMinimumImageBufferSize(DMG* d, DMG_ImageMode mode)
{
	uint32_t width = 0;
	uint32_t height = 0;
	DMG_GetTargetSize(d, &width, &height);
	uint32_t pixels = width * height;

	switch (mode)
	{
		case ImageMode_Packed:
			return (pixels + 1) >> 1;
		case ImageMode_IndexedX:
			return ((width + 3u) & ~3u) * height;
		case ImageMode_CGA:
			return ((width + 3u) >> 2) * height;
		case ImageMode_RGBA32:
			return pixels * 4u;
		case ImageMode_Planar:
		case ImageMode_PlanarST:
		case ImageMode_Planar8:
		case ImageMode_PlanarFalcon:
		{
			uint8_t planes = DMG_GetNativePlaneCount(d, mode);
			return ((width + 15u) >> 4) * height * planes * 2u;
		}
		case ImageMode_Indexed:
		case ImageMode_Raw:
		default:
			return pixels;
	}
}

uint32_t DMG_GetMinimumImageCacheAllocationSize(DMG* d, DMG_ImageMode mode)
{
	uint32_t itemOverhead = ((sizeof(DMG_Cache) + 31u) & ~31u) + 32u;
	uint32_t minimum = DMG_GetMinimumImageBufferSize(d, mode) + itemOverhead;
	return (minimum + 0x0FFFu) & ~0x0FFFu;
}

static bool DMG_AllocatePaletteBlock(DMG* dmg, uint32_t colorCount, const char* name)
{
	if (dmg == 0 || colorCount == 0)
		return true;

	dmg->paletteBlock = Allocate<uint32_t>(name, colorCount);
	if (dmg->paletteBlock == 0)
	{
		DMG_SetError(DMG_ERROR_OUT_OF_MEMORY);
		return false;
	}
	MemClear(dmg->paletteBlock, sizeof(uint32_t) * colorCount);
	return true;
}

#if DMG_SUPPORT_CLASSIC_CONVERSION_PALETTES
static bool DMG_AllocateConversionPaletteBlock(DMG* dmg, uint32_t entryCount, const char* name)
{
	if (dmg == 0 || entryCount == 0)
		return true;

	dmg->conversionPaletteBlock = Allocate<DMG_CVPal>(name, entryCount);
	if (dmg->conversionPaletteBlock == 0)
	{
		DMG_SetError(DMG_ERROR_OUT_OF_MEMORY);
		return false;
	}
	MemClear(dmg->conversionPaletteBlock, sizeof(DMG_CVPal) * entryCount);
	return true;
}
#endif

bool DMG_EnsureEditableEntryData(DMG* dmg)
{
	if (dmg == 0)
		return false;
	if (dmg->editableEntries != 0)
		return true;

	dmg->editableEntries = Allocate<DMG_EditableEntryData>("DMG editable entries", 256);
	if (dmg->editableEntries == 0)
	{
		DMG_SetError(DMG_ERROR_OUT_OF_MEMORY);
		return false;
	}
	MemClear(dmg->editableEntries, sizeof(DMG_EditableEntryData) * 256);
	for (int i = 0; i < 256; i++)
		dmg->editableEntries[i].reusedFrom = 0xFF;
	return true;
}

static void DMG_ParseClassicEntryHeader(DMG* dmg, DMG_Entry* entry, const uint8_t* header)
{
	uint16_t v = read16(header, dmg->littleEndian);
	entry->flags &= ~DMG_FLAG_COMPRESSED;
	entry->width = v & 0x7FFF;
	if ((v & 0x8000) != 0)
		entry->flags |= DMG_FLAG_COMPRESSED;
	v = read16(header + 2, dmg->littleEndian);
	if ((v & 0x8000) != 0)
		entry->type = DMGEntry_Audio;
	entry->height = v & 0x7FFF;
	entry->length = read16(header + 4, dmg->littleEndian);
}

static bool DMG_PreallocateZX0Scratch(DMG* dmg)
{
	uint32_t maxCompressedSize = 0;

	if (dmg == 0 || dmg->version != DMG_Version5)
		return true;

	for (int n = dmg->firstEntry; n <= dmg->lastEntry; n++)
	{
		DMG_Entry* entry = dmg->entries[n];
		if (entry == 0 || entry->type != DMGEntry_Image)
			continue;
		if ((entry->flags & (DMG_FLAG_COMPRESSED | DMG_FLAG_ZX0)) != (DMG_FLAG_COMPRESSED | DMG_FLAG_ZX0))
			continue;
		if (entry->length > maxCompressedSize)
			maxCompressedSize = entry->length;
	}

	if (maxCompressedSize == 0)
		return true;

	if (!DMG_ReserveTemporaryBuffer(maxCompressedSize))
	{
		DMG_SetError(DMG_ERROR_OUT_OF_MEMORY);
		return false;
	}
	return true;
}

uint32_t DMG_GetTemporaryBufferSize()
{
	return dmgTemporaryBufferSize;
}

uint8_t* DMG_GetTemporaryBufferBase()
{
	return dmgTemporaryBuffer;
}

bool DMG_ReserveTemporaryBuffer(uint32_t size)
{
	if (dmgTemporaryBuffer != 0 && dmgTemporaryBufferSize >= size)
		return true;

	if (dmgTemporaryBufferRaw != 0)
	{
		#if DEBUG_IMAGE_CACHE
		DebugPrintf("DMG Temporary buffer reallocated (was too small: %u)\n", dmgTemporaryBufferSize);
		#endif
		Free(dmgTemporaryBufferRaw);
		dmgTemporaryBufferRaw = 0;
		dmgTemporaryBuffer = 0;
		dmgTemporaryBufferSize = 0;
	}

	uint32_t allocationSize = size + 255;
	dmgTemporaryBufferRaw = Allocate<uint8_t>("DMG Temporary buffer", allocationSize);
	if (dmgTemporaryBufferRaw == 0)
		return false;

	#if DEBUG_IMAGE_CACHE
	DebugPrintf("DMG Temporary buffer allocated (%u bytes)\n", size);
	#endif

	dmgTemporaryBuffer = dmgTemporaryBufferRaw;
	while ((((size_t)dmgTemporaryBuffer) & 255u) != 0)
		dmgTemporaryBuffer++;

	dmgTemporaryBufferSize = size;
	return true;
}

uint8_t* DMG_GetTemporaryBuffer(DMG_ImageMode mode)
{
	uint32_t basePixels = DMG_BUFFER_SIZE;
	if (dmg != 0)
	{
		uint32_t width = dmg->targetWidth;
		uint32_t height = dmg->targetHeight;
		if (width == 0 || height == 0)
		{
			switch (dmg->screenMode)
			{
				case ScreenMode_HiRes:
					width = 640;
					height = 200;
					break;
				case ScreenMode_SHiRes:
					width = 640;
					height = 400;
					break;
				default:
					width = 320;
					height = 200;
					break;
			}
		}
		if (width != 0 && height != 0)
			basePixels = width * height;
	}

	uint32_t size = 
			DMG_IS_INDEXED(mode) ? 2 * basePixels : 
			DMG_IS_RGBA32(mode) ? 4 * basePixels : basePixels;
	if (!DMG_ReserveTemporaryBuffer(size))
		return 0;
	
	return dmgTemporaryBuffer;
}

bool DMG_IsTemporaryBufferPointer(const void* ptr)
{
	if (ptr == 0 || dmgTemporaryBuffer == 0 || dmgTemporaryBufferSize == 0)
		return false;

	const uint8_t* bytePtr = (const uint8_t*)ptr;
	return bytePtr >= dmgTemporaryBuffer && bytePtr < dmgTemporaryBuffer + dmgTemporaryBufferSize;
}

void DMG_FreeTemporaryBuffer()
{
	if (dmgTemporaryBufferRaw)
	{
		Free(dmgTemporaryBufferRaw);
		dmgTemporaryBufferRaw = 0;
		dmgTemporaryBuffer = 0;
		dmgTemporaryBufferSize = 0;
	}
}

void DMG_SetZX0ScratchBuffer(DMG* d, uint8_t* buffer, uint32_t size, bool owned)
{
	if (d == 0)
		return;

	if (d->zx0Scratch != 0 && d->zx0ScratchOwned)
		Free(d->zx0Scratch);

	d->zx0Scratch = buffer;
	d->zx0ScratchSize = size;
	d->zx0ScratchOwned = owned;
}

uint8_t* DMG_GetScratchBuffer(DMG* d, uint32_t size)
{
	if (d != 0 && d->zx0Scratch != 0 && d->zx0ScratchSize >= size)
		return d->zx0Scratch;

	if (!DMG_ReserveTemporaryBuffer(size))
		return 0;

	return DMG_GetTemporaryBufferBase();
}

uint32_t DMG_GetScratchBufferSize(DMG* d)
{
	if (d != 0 && d->zx0Scratch != 0)
		return d->zx0ScratchSize;
	return DMG_GetTemporaryBufferSize();
}

/* ───────────────────────────────────────────────────────────────────────── */
/*  Error handling															 */
/* ───────────────────────────────────────────────────────────────────────── */

void DMG_SetWarningHandler(void (*handler)(const char* message))
{
	dmg_warningHandler = handler;
}

void DMG_Warning(const char* format, ...)
{
#ifdef PRINTF_WARNINGS
	if (dmg_warningHandler != 0)
	{		
#ifdef _STDCLIB
		va_list args;
		va_start(args, format);
		vsnprintf(dmgWarningBuffer, sizeof(dmgWarningBuffer), format, args);
		va_end(args);
		dmg_warningHandler(dmgWarningBuffer);
#else
		dmg_warningHandler(format);
#endif
	}
#endif
}

void DMG_SetError(DMG_Error error)
{
	dmgError = error;
}

DMG_Error DMG_GetError()
{
	return dmgError;
}

static const char* DMG_GetFileErrorDetail(const char* fallback)
{
#if defined(HAS_VIRTUALFILESYSTEM) || defined(_STDCLIB)
	return File_GetError() != FileError_None ? File_GetErrorString() : fallback;
#else
	return fallback;
#endif
}

const char* DMG_GetErrorString()
{
	switch (dmgError)
	{
		case DMG_ERROR_NONE:
			return "No error";
		case DMG_ERROR_FILE_NOT_FOUND:
			return "File not found";
		case DMG_ERROR_CREATING_FILE:
			return DMG_GetFileErrorDetail("I/O Error creating file");
		case DMG_ERROR_OUT_OF_MEMORY:
			return "Out of memory";
		case DMG_ERROR_UNKNOWN_SIGNATURE:
			return "Unknown signature";
		case DMG_ERROR_READING_FILE:
			return DMG_GetFileErrorDetail("I/O Error reading file");
		case DMG_ERROR_SEEKING_FILE:
			return "I/O Error accessing file";
		case DMG_ERROR_WRITING_FILE:
			return DMG_GetFileErrorDetail("I/O Error writing file");
		case DMG_ERROR_INVALID_ENTRY_COUNT:
			return "Invalid entry count";
		case DMG_ERROR_FILE_TOO_SMALL:
			return "File too small";
		case DMG_ERROR_FILE_TOO_BIG:
			return "File too big";
		case DMG_ERROR_TRUNCATED_DATA_STREAM:
			return "Truncated data stream";
		case DMG_ERROR_DATA_STREAM_TOO_LONG:
			return "Data stream too long";
		case DMG_ERROR_DATA_OFFSET_OUT_OF_BOUNDS:
			return "Data offset out of bounds";
		case DMG_ERROR_CORRUPTED_DATA_STREAM:
			return "Corrupted data stream";
		case DMG_ERROR_IMAGE_TOO_BIG:
			return "Image too big";
		case DMG_ERROR_BUFFER_TOO_SMALL:
			return "Buffer too small";
		case DMG_ERROR_ENTRY_IS_EMPTY:
			return "Entry is empty";
		case DMG_ERROR_INVALID_IMAGE:
			return "Invalid image format";
		case DMG_ERROR_TEMPORARY_BUFFER_MISSING:
			return "Temporary buffer missing";
		default:
			DMG_Warning("Unknown error code %d", dmgError);
			return "Unknown error";
	}
}

bool DMG_CopyImageData(uint8_t* ptr, uint16_t length, uint8_t* output, int pixels)
{
	uint32_t outputSize = (pixels + 1) & ~1;
	if (length > outputSize)
		length = outputSize;
	MemMove(output, ptr, length);
	return true;
}

uint8_t* DMG_ConvertPlanar8ToPlanarST(uint8_t* data, uint8_t* buffer, int length, uint32_t width)
{
	uint32_t widthBlocks = (width + 7) >> 3;
	uint32_t widthWords = (width + 15) >> 4;
	uint32_t srcRowBytes = widthBlocks * 4;
	uint32_t dstRowBytes = widthWords * 8;
	if (srcRowBytes == 0)
		return buffer;

	uint32_t height = (uint32_t)length / srcRowBytes;
	for (uint32_t row = height; row-- > 0; )
	{
		uint8_t* src = data + row * srcRowBytes + srcRowBytes;
		uint8_t* dst = buffer + row * dstRowBytes + dstRowBytes;
		uint32_t remainingBlocks = widthBlocks;

		if ((remainingBlocks & 1) != 0)
		{
			src -= 4;
			dst -= 8;
			uint8_t p0 = src[0];
			uint8_t p1 = src[1];
			uint8_t p2 = src[2];
			uint8_t p3 = src[3];
			uint32_t a = ((uint32_t)p0 << 24) | ((uint32_t)p1 << 8);
			uint32_t b = ((uint32_t)p2 << 24) | ((uint32_t)p3 << 8);
			((uint32_t*)dst)[0] = a;
			((uint32_t*)dst)[1] = b;
			remainingBlocks--;
		}

		while (remainingBlocks > 0)
		{
			src -= 8;
			dst -= 8;
			uint8_t p0a = src[0];
			uint8_t p1a = src[1];
			uint8_t p2a = src[2];
			uint8_t p3a = src[3];
			uint8_t p0b = src[4];
			uint8_t p1b = src[5];
			uint8_t p2b = src[6];
			uint8_t p3b = src[7];
			uint32_t a = ((uint32_t)p0a << 24) | ((uint32_t)p0b << 16) |
				((uint32_t)p1a << 8) | (uint32_t)p1b;
			uint32_t b = ((uint32_t)p2a << 24) | ((uint32_t)p2b << 16) |
				((uint32_t)p3a << 8) | (uint32_t)p3b;
			((uint32_t*)dst)[0] = a;
			((uint32_t*)dst)[1] = b;
			remainingBlocks -= 2;
		}
	}
	return buffer;
}

uint8_t* DMG_ConvertPlanar8ToPlanarFalcon(uint8_t* data, uint8_t* buffer, int length, uint32_t width)
{
	uint32_t widthBlocks = (width + 7) >> 3;
	uint32_t widthWords = (width + 15) >> 4;
	uint32_t srcRowBytes = widthBlocks * 4;
	uint32_t dstRowBytes = widthWords * 16;
	if (srcRowBytes == 0)
		return buffer;

	uint32_t height = (uint32_t)length / srcRowBytes;
	for (uint32_t row = height; row-- > 0; )
	{
		uint8_t* src = data + row * srcRowBytes + srcRowBytes;
		uint8_t* dst = buffer + row * dstRowBytes + dstRowBytes;
		uint32_t remainingBlocks = widthBlocks;

		if ((remainingBlocks & 1) != 0)
		{
			src -= 4;
			dst -= 16;
			uint8_t p0 = src[0];
			uint8_t p1 = src[1];
			uint8_t p2 = src[2];
			uint8_t p3 = src[3];
			((uint32_t*)dst)[0] = ((uint32_t)p0 << 24) | ((uint32_t)p1 << 8);
			((uint32_t*)dst)[1] = ((uint32_t)p2 << 24) | ((uint32_t)p3 << 8);
			((uint32_t*)dst)[2] = 0;
			((uint32_t*)dst)[3] = 0;
			remainingBlocks--;
		}

		while (remainingBlocks > 0)
		{
			src -= 8;
			dst -= 16;
			uint8_t p0a = src[0];
			uint8_t p1a = src[1];
			uint8_t p2a = src[2];
			uint8_t p3a = src[3];
			uint8_t p0b = src[4];
			uint8_t p1b = src[5];
			uint8_t p2b = src[6];
			uint8_t p3b = src[7];
			((uint32_t*)dst)[0] = ((uint32_t)p0a << 24) | ((uint32_t)p0b << 16) |
				((uint32_t)p1a << 8) | (uint32_t)p1b;
			((uint32_t*)dst)[1] = ((uint32_t)p2a << 24) | ((uint32_t)p2b << 16) |
				((uint32_t)p3a << 8) | (uint32_t)p3b;
			((uint32_t*)dst)[2] = 0;
			((uint32_t*)dst)[3] = 0;
		}
	}
	return buffer;
}

static bool DMG_LoadDAT5Palette(DMG* dmg, uint8_t index, DMG_Entry* entry);

bool DMG_Planar8ToPacked (const uint8_t* ptr, uint16_t length, uint8_t* output, int pixels, uint32_t width)
{
	const uint8_t* end = ptr + length;
	uint8_t* outputEnd = output + pixels;

		while (ptr < end-3 && output < outputEnd) {
			uint32_t word =
				((uint32_t)ptr[0] << 24) |
				((uint32_t)ptr[1] << 16) |
				((uint32_t)ptr[2] << 8) |
				(uint32_t)ptr[3];
		for (int n = 7; n >= 0; n--) {
			uint8_t color0 = 
				(((word >> 24) >> n) & 1) << 0 |
				(((word >> 16) >> n) & 1) << 1 |
				(((word >>  8) >> n) & 1) << 2 |
				(((word >>  0) >> n) & 1) << 3;
			n--;
			uint8_t color1 = 
				(((word >> 24) >> n) & 1) << 0 |
				(((word >> 16) >> n) & 1) << 1 |
				(((word >>  8) >> n) & 1) << 2 |
				(((word >>  0) >> n) & 1) << 3;
			*output++ = color1 | (color0 << 4);
			if (output == outputEnd) break;
		}
		ptr += 4;
	}
	return ptr == end && output == outputEnd;
}

void DMG_ConvertPackedToPlanarST(uint8_t *buffer, uint32_t bufferSize, uint32_t width)
{
	uint32_t *out = (uint32_t*)buffer;
	uint8_t *in = (uint8_t*)buffer;
	uint32_t x = 0;
	width >>= 1;
	for (unsigned n = 0; n < bufferSize; n += 8)
	{
		uint32_t p0    = 0;
		uint32_t p1    = 0;
		uint32_t mask0 = 0x00008000;
		uint32_t mask1 = 0x80000000;

		do
		{
			const uint8_t c = *in++;
			if (c & 0x10) p0 |= mask1;
			if (c & 0x20) p0 |= mask0;
			if (c & 0x40) p1 |= mask1;
			if (c & 0x80) p1 |= mask0;
			mask0 >>= 1;
			mask1 >>= 1;
			if (c & 0x01) p0 |= mask1;
			if (c & 0x02) p0 |= mask0;
			if (c & 0x04) p1 |= mask1;
			if (c & 0x08) p1 |= mask0;
			mask0 >>= 1;
			mask1 >>= 1;
		} while (mask0);

		*out++ = p0;
		*out++ = p1;
		if (++x == width)
			x = 0;
	}
}

void DMG_ConvertPackedToPlanarFalcon(uint8_t *buffer, uint32_t bufferSize, uint32_t width)
{
	uint32_t *out = (uint32_t*)buffer;
	uint8_t *in = (uint8_t*)buffer;
	uint32_t x = 0;
	width >>= 1;
	for (unsigned n = 0; n < bufferSize; n += 16)
	{
		uint32_t p0 = 0;
		uint32_t p1 = 0;
		uint32_t mask0 = 0x00008000;
		uint32_t mask1 = 0x80000000;

		do
		{
			const uint8_t c = *in++;
			if (c & 0x10) p0 |= mask1;
			if (c & 0x20) p0 |= mask0;
			if (c & 0x40) p1 |= mask1;
			if (c & 0x80) p1 |= mask0;
			mask0 >>= 1;
			mask1 >>= 1;
			if (c & 0x01) p0 |= mask1;
			if (c & 0x02) p0 |= mask0;
			if (c & 0x04) p1 |= mask1;
			if (c & 0x08) p1 |= mask0;
			mask0 >>= 1;
			mask1 >>= 1;
		} while (mask0);

		*out++ = p0;
		*out++ = p1;
		*out++ = 0;
		*out++ = 0;
		if (++x == width)
			x = 0;
	}
}

bool DMG_UnpackChunkyPixels(const uint8_t* input, uint16_t width, uint16_t height, uint8_t bitsPerPixel, uint8_t* output)
{
    if (bitsPerPixel != 2 && bitsPerPixel != 4)
        return false;

    const uint32_t pixels = (uint32_t)width * height;
    uint32_t shift = 8 - bitsPerPixel;
    uint8_t mask = (1u << bitsPerPixel) - 1;
    uint8_t value = 0;
    int bitsLeft = 0;

    for (uint32_t i = 0; i < pixels; i++)
    {
        if (bitsLeft == 0)
        {
            value = *input++;
            bitsLeft = 8;
        }
        output[i] = (value >> shift) & mask;
        value <<= bitsPerPixel;
        bitsLeft -= bitsPerPixel;
    }
    return true;
}

bool DMG_UnpackBitplaneBytes(const uint8_t* data, uint16_t width, uint16_t height, uint8_t bitsPerPixel, uint8_t* output)
{
    if (bitsPerPixel == 0 || bitsPerPixel > 8)
        return false;

    const uint32_t wordsPerRow = (width + 15) >> 4;
    const uint32_t planeStride = wordsPerRow * height * 2;

	if (bitsPerPixel == 8)
	{
		const uint32_t fullWordsPerRow = width >> 4;
		const uint8_t tailBits = (uint8_t)(width & 15u);

		for (uint16_t y = 0; y < height; y++)
		{
			uint8_t* dst = output + (uint32_t)y * width;
			uint32_t rowOffset = (uint32_t)y * wordsPerRow * 2;

			for (uint32_t wordIndex = 0; wordIndex < fullWordsPerRow; wordIndex++)
			{
				uint32_t wordOffset = rowOffset + wordIndex * 2;
				uint16_t p0 = (uint16_t)(data[planeStride * 0u + wordOffset + 0u] << 8) | data[planeStride * 0u + wordOffset + 1u];
				uint16_t p1 = (uint16_t)(data[planeStride * 1u + wordOffset + 0u] << 8) | data[planeStride * 1u + wordOffset + 1u];
				uint16_t p2 = (uint16_t)(data[planeStride * 2u + wordOffset + 0u] << 8) | data[planeStride * 2u + wordOffset + 1u];
				uint16_t p3 = (uint16_t)(data[planeStride * 3u + wordOffset + 0u] << 8) | data[planeStride * 3u + wordOffset + 1u];
				uint16_t p4 = (uint16_t)(data[planeStride * 4u + wordOffset + 0u] << 8) | data[planeStride * 4u + wordOffset + 1u];
				uint16_t p5 = (uint16_t)(data[planeStride * 5u + wordOffset + 0u] << 8) | data[planeStride * 5u + wordOffset + 1u];
				uint16_t p6 = (uint16_t)(data[planeStride * 6u + wordOffset + 0u] << 8) | data[planeStride * 6u + wordOffset + 1u];
				uint16_t p7 = (uint16_t)(data[planeStride * 7u + wordOffset + 0u] << 8) | data[planeStride * 7u + wordOffset + 1u];

				uint8_t* out = dst + (wordIndex << 4);
				for (uint8_t bit = 0; bit < 16; bit++)
				{
					uint16_t mask = (uint16_t)(0x8000u >> bit);
					out[bit] =
						((p0 & mask) ? 0x01u : 0u) |
						((p1 & mask) ? 0x02u : 0u) |
						((p2 & mask) ? 0x04u : 0u) |
						((p3 & mask) ? 0x08u : 0u) |
						((p4 & mask) ? 0x10u : 0u) |
						((p5 & mask) ? 0x20u : 0u) |
						((p6 & mask) ? 0x40u : 0u) |
						((p7 & mask) ? 0x80u : 0u);
				}
			}

			if (tailBits != 0)
			{
				uint32_t wordOffset = rowOffset + fullWordsPerRow * 2;
				uint16_t p0 = (uint16_t)(data[planeStride * 0u + wordOffset + 0u] << 8) | data[planeStride * 0u + wordOffset + 1u];
				uint16_t p1 = (uint16_t)(data[planeStride * 1u + wordOffset + 0u] << 8) | data[planeStride * 1u + wordOffset + 1u];
				uint16_t p2 = (uint16_t)(data[planeStride * 2u + wordOffset + 0u] << 8) | data[planeStride * 2u + wordOffset + 1u];
				uint16_t p3 = (uint16_t)(data[planeStride * 3u + wordOffset + 0u] << 8) | data[planeStride * 3u + wordOffset + 1u];
				uint16_t p4 = (uint16_t)(data[planeStride * 4u + wordOffset + 0u] << 8) | data[planeStride * 4u + wordOffset + 1u];
				uint16_t p5 = (uint16_t)(data[planeStride * 5u + wordOffset + 0u] << 8) | data[planeStride * 5u + wordOffset + 1u];
				uint16_t p6 = (uint16_t)(data[planeStride * 6u + wordOffset + 0u] << 8) | data[planeStride * 6u + wordOffset + 1u];
				uint16_t p7 = (uint16_t)(data[planeStride * 7u + wordOffset + 0u] << 8) | data[planeStride * 7u + wordOffset + 1u];

				uint8_t* out = dst + (fullWordsPerRow << 4);
				for (uint8_t bit = 0; bit < tailBits; bit++)
				{
					uint16_t mask = (uint16_t)(0x8000u >> bit);
					out[bit] =
						((p0 & mask) ? 0x01u : 0u) |
						((p1 & mask) ? 0x02u : 0u) |
						((p2 & mask) ? 0x04u : 0u) |
						((p3 & mask) ? 0x08u : 0u) |
						((p4 & mask) ? 0x10u : 0u) |
						((p5 & mask) ? 0x20u : 0u) |
						((p6 & mask) ? 0x40u : 0u) |
						((p7 & mask) ? 0x80u : 0u);
				}
			}
		}
		return true;
	}

	const uint32_t fullWordsPerRow = width >> 4;
	const uint8_t tailBits = (uint8_t)(width & 15u);
    MemClear(output, (uint32_t)width * height);

    for (uint8_t plane = 0; plane < bitsPerPixel; plane++)
    {
        const uint8_t* planePtr = data + plane * planeStride;
        uint8_t planeMask = (uint8_t)(1u << plane);
        for (uint16_t y = 0; y < height; y++)
        {
            const uint8_t* rowPtr = planePtr + y * wordsPerRow * 2;
            uint8_t* dst = output + y * width;
			for (uint32_t wordIndex = 0; wordIndex < fullWordsPerRow; wordIndex++)
            {
                const uint8_t* src = rowPtr + wordIndex * 2;
                uint16_t value = (uint16_t)(src[0] << 8) | src[1];
				uint8_t* out = dst + (wordIndex << 4);
				out[0]  |= (value & 0x8000u) ? planeMask : 0;
				out[1]  |= (value & 0x4000u) ? planeMask : 0;
				out[2]  |= (value & 0x2000u) ? planeMask : 0;
				out[3]  |= (value & 0x1000u) ? planeMask : 0;
				out[4]  |= (value & 0x0800u) ? planeMask : 0;
				out[5]  |= (value & 0x0400u) ? planeMask : 0;
				out[6]  |= (value & 0x0200u) ? planeMask : 0;
				out[7]  |= (value & 0x0100u) ? planeMask : 0;
				out[8]  |= (value & 0x0080u) ? planeMask : 0;
				out[9]  |= (value & 0x0040u) ? planeMask : 0;
				out[10] |= (value & 0x0020u) ? planeMask : 0;
				out[11] |= (value & 0x0010u) ? planeMask : 0;
				out[12] |= (value & 0x0008u) ? planeMask : 0;
				out[13] |= (value & 0x0004u) ? planeMask : 0;
				out[14] |= (value & 0x0002u) ? planeMask : 0;
				out[15] |= (value & 0x0001u) ? planeMask : 0;
			}

			if (tailBits != 0)
			{
				const uint8_t* src = rowPtr + fullWordsPerRow * 2;
				uint16_t value = (uint16_t)(src[0] << 8) | src[1];
				uint8_t* out = dst + (fullWordsPerRow << 4);
				for (uint8_t bit = 0; bit < tailBits; bit++)
				{
					if (value & (uint16_t)(0x8000u >> bit))
						out[bit] |= planeMask;
				}
			}
		}
	}
	return true;
}

bool DMG_UnpackBitplaneWords(const uint8_t* data, uint16_t width, uint16_t height, uint8_t bitsPerPixel, uint8_t* output)
{
	if (bitsPerPixel == 0 || bitsPerPixel > 8)
		return false;

	const uint32_t wordsPerRow = (width + 15) >> 4;
	const uint32_t rowStride = wordsPerRow * bitsPerPixel * 2;
	MemClear(output, (uint32_t)width * height);

	for (uint16_t y = 0; y < height; y++)
	{
		const uint8_t* rowPtr = data + y * rowStride;
		uint8_t* dst = output + y * width;
		for (uint32_t wordIndex = 0; wordIndex < wordsPerRow; wordIndex++)
		{
			uint16_t baseX = (uint16_t)(wordIndex << 4);
			for (uint8_t plane = 0; plane < bitsPerPixel; plane++)
			{
				const uint8_t* src = rowPtr + (wordIndex * bitsPerPixel + plane) * 2;
				uint16_t value = (uint16_t)(src[0] << 8) | src[1];
				uint8_t planeMask = (uint8_t)(1u << plane);
				for (int bit = 15; bit >= 0; bit--)
				{
					uint16_t x = (uint16_t)(baseX + (15 - bit));
					if (x >= width)
						break;
					if ((value >> bit) & 1)
						dst[x] |= planeMask;
				}
			}
		}
	}
	return true;
}

static bool DMG_ReadDAT5Entries(DMG* dmg)
{
    uint8_t header[16];
    uint32_t imageCount = 0;
    uint32_t audioCount = 0;
    uint32_t compressedCount = 0;
	uint32_t paletteColorCount = 0;

    if (DMG_ReadFromFile(dmg, 0, header, sizeof(header)) != sizeof(header))
    {
        DMG_SetError(DMG_ERROR_READING_FILE);
        return false;
    }

    dmg->version = DMG_Version5;
    dmg->littleEndian = false;
    dmg->targetWidth = read16BE(header + 0x06);
    dmg->targetHeight = read16BE(header + 0x08);
    dmg->firstEntry = (uint8_t)read16BE(header + 0x0A);
    dmg->lastEntry = (uint8_t)read16BE(header + 0x0C);
    dmg->colorMode = header[0x0E];
	dmg->dat5Flags = header[0x0F];

    if (dmg->lastEntry < dmg->firstEntry)
    {
        DMG_SetError(DMG_ERROR_INVALID_ENTRY_COUNT);
        return false;
    }

	if (dmg->targetWidth == 320 && dmg->targetHeight == 200)
		dmg->screenMode =
			(dmg->colorMode == DMG_DAT5_COLORMODE_CGA) ? ScreenMode_CGA :
			(dmg->colorMode == DMG_DAT5_COLORMODE_EGA) ? ScreenMode_EGA :
			(DMG_DAT5ModePlaneCount(dmg->colorMode) >= 8 || DMG_DAT5ModeIsIndexedX(dmg->colorMode) || DMG_DAT5ModeIsIndexed(dmg->colorMode)) ? ScreenMode_VGA :
			ScreenMode_VGA16;
	else if (dmg->targetWidth == 640 && dmg->targetHeight == 200)
		dmg->screenMode = ScreenMode_HiRes;
	else if (dmg->targetWidth == 640 && dmg->targetHeight == 400)
		dmg->screenMode = ScreenMode_SHiRes;

	const uint32_t entryCount = dmg->lastEntry - dmg->firstEntry + 1;
	uint8_t* buffer = Allocate<uint8_t>("DAT5 entry headers", entryCount * 32);
	if (buffer == 0)
	{
		DMG_SetError(DMG_ERROR_OUT_OF_MEMORY);
		return false;
	}
	if (DMG_ReadFromFile(dmg, 0x10, buffer, entryCount * 32) != entryCount * 32)
	{
		Free(buffer);
		DMG_SetError(DMG_ERROR_READING_FILE);
		return false;
	}

    for (uint32_t i = 0; i < entryCount; i++)
    {
        uint8_t* ptr = buffer + i * 32;
        uint32_t offset = read32BE(ptr + 0x00);
        uint32_t size = read32BE(ptr + 0x04);
        uint8_t type = ptr[0x08];

		if (offset == 0)
            continue;

		if (type == 1)
			audioCount++;
		else
			imageCount++;

        if (offset + size > dmg->fileSize)
        {
            Free(buffer);
            DMG_SetError(DMG_ERROR_DATA_OFFSET_OUT_OF_BOUNDS);
            return false;
        }

		if (type != 1)
        {
            uint16_t flags = read16BE(ptr + 0x0A);
            if ((flags & 0x0008) != 0)
                compressedCount++;

			uint8_t firstColor = ptr[0x14];
			uint8_t lastColor = ptr[0x15];
			uint16_t paletteColors = lastColor >= firstColor ? (uint16_t)(lastColor - firstColor + 1) : 0;
			uint32_t paletteSize = paletteColors * 3u;
			if (paletteSize != 0)
            {
				paletteColorCount += paletteColors;
				if (size < paletteSize)
                {
                    Free(buffer);
                    DMG_SetError(DMG_ERROR_DATA_OFFSET_OUT_OF_BOUNDS);
                    return false;
                }
            }
        }
    }

	dmg->entryBlock = Allocate<DMG_Entry>("DAT5 entries", entryCount);
	if (dmg->entryBlock == 0)
	{
		Free(buffer);
		DMG_SetError(DMG_ERROR_OUT_OF_MEMORY);
		return false;
	}
	MemClear(dmg->entryBlock, sizeof(DMG_Entry) * entryCount);
	if (!DMG_AllocatePaletteBlock(dmg, paletteColorCount, "DAT5 palettes"))
	{
		Free(buffer);
		return false;
	}

	uint32_t* paletteCursor = dmg->paletteBlock;

	for (uint32_t i = 0; i < entryCount; i++)
	{
		uint8_t index = (uint8_t)(dmg->firstEntry + i);
		uint8_t* ptr = buffer + i * 32;
		uint32_t offset = read32BE(ptr + 0x00);
		uint32_t size = read32BE(ptr + 0x04);
		uint8_t type = ptr[0x08];

		DMG_Entry* entry = dmg->entryBlock + i;
		dmg->entries[index] = entry;
		entry->type = (offset == 0) ? DMGEntry_Empty : (type == 1 ? DMGEntry_Audio : DMGEntry_Image);
		entry->fileOffset = offset;
		entry->length = size;
		entry->flags = DMG_FLAG_PROCESSED;

		if (entry->type == DMGEntry_Empty)
			continue;

		if (entry->type == DMGEntry_Image)
		{
			uint16_t flags = read16BE(ptr + 0x0A);
			entry->bitDepth = ptr[0x09];
			entry->x = (int16_t)read16BE(ptr + 0x0C);
			entry->y = (int16_t)read16BE(ptr + 0x0E);
			entry->width = read16BE(ptr + 0x10);
			entry->height = read16BE(ptr + 0x12);
			entry->firstColor = ptr[0x14];
			entry->lastColor = ptr[0x15];
			entry->paletteOffset = 0;
			entry->paletteColors = entry->lastColor >= entry->firstColor ? (uint16_t)(entry->lastColor - entry->firstColor + 1) : 0;
			entry->paletteSize = entry->paletteColors * 3;

			if ((flags & 0x0008) != 0)
			{
				entry->flags |= DMG_FLAG_COMPRESSED;
				entry->flags |= DMG_FLAG_ZX0;
			}
			if ((flags & 0x0010) == 0)
				entry->flags |= DMG_FLAG_FIXED;
			if ((flags & 0x0020) != 0)
				entry->flags |= DMG_FLAG_BUFFERED;
			if ((flags & 0x0040) != 0)
				DMG_SetCGAMode(entry, CGA_Blue);
			else
				DMG_SetCGAMode(entry, CGA_Red);

			if (entry->paletteColors != 0)
			{
				entry->RGB32Palette = paletteCursor;
				paletteCursor += entry->paletteColors;
			}
		}
		else
		{
			uint16_t flags = read16BE(ptr + 0x0A);
			entry->x = ptr[0x09];
			if ((flags & 0x0001) != 0)
				entry->bitDepth = 16;
			if ((flags & 0x0010) == 0)
				entry->flags |= DMG_FLAG_FIXED;
			if ((flags & 0x0020) != 0)
				entry->flags |= DMG_FLAG_BUFFERED;
		}
	}

    Free(buffer);
    return true;
}

static bool DMG_DecodeDAT5PaletteBytes(DMG_Entry* entry, const uint8_t* palData)
{
	if (entry == 0 || entry->paletteColors == 0 || entry->paletteSize == 0)
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
            ((uint32_t)palData[p * 3 + 0] << 16) |
            ((uint32_t)palData[p * 3 + 1] << 8) |
            (uint32_t)palData[p * 3 + 2];
    }
	DMG_SetPaletteDecoded(entry, true);
    return true;
}

static bool DMG_LoadDAT5Palette(DMG* dmg, uint8_t index, DMG_Entry* entry)
{
	if (dmg == 0 || entry == 0 || entry->paletteColors == 0 || entry->paletteSize == 0)
		return true;
	if (DMG_IsPaletteDecoded(entry))
		return true;

	const uint8_t* fileData = (const uint8_t*)DMG_GetFromFileCache(dmg, entry->fileOffset, entry->paletteColors * 3);
	if (fileData != 0)
		return DMG_DecodeDAT5PaletteBytes(entry, fileData);

	uint8_t palData[256 * 3];
	if (entry->paletteColors > 256)
	{
		DMG_SetError(DMG_ERROR_INVALID_IMAGE);
		return false;
	}
	if (DMG_ReadFromFile(dmg, entry->fileOffset, palData, entry->paletteColors * 3) != entry->paletteColors * 3)
	{
		DMG_SetError(DMG_ERROR_READING_FILE);
		return false;
	}
	return DMG_DecodeDAT5PaletteBytes(entry, palData);
}

/* ───────────────────────────────────────────────────────────────────────── */
/*  Entry management														 */
/* ───────────────────────────────────────────────────────────────────────── */

DMG_Entry* DMG_GetEntry (DMG* dmg, uint8_t n)
{
	uint8_t header[6];

	DMG_SetError(DMG_ERROR_NONE);
	if (dmg == 0)
		return 0;
	if (dmg->entries[n] == 0)
		return 0;
	if (dmg->entries[n]->type == DMGEntry_Empty)
		return dmg->entries[n];
	if (dmg->version == DMG_Version5)
		return dmg->entries[n];
	if ((dmg->entries[n]->flags & DMG_FLAG_PROCESSED) != 0)
		return dmg->entries[n];

	if (dmg->entries[n]->fileOffset + sizeof(header) > dmg->fileSize)
	{
		DMG_Warning("Entry %d: Data offset %04X-%04X out of bounds (0-%04X)", n,
			dmg->entries[n]->fileOffset,
			dmg->entries[n]->fileOffset + (uint32_t)sizeof(header) - 1,
			dmg->fileSize);
		DMG_SetError(DMG_ERROR_DATA_OFFSET_OUT_OF_BOUNDS);
		return 0;
	}

	const uint8_t* cachedHeader = (const uint8_t*)DMG_GetFromFileCache(dmg, dmg->entries[n]->fileOffset, sizeof(header));
	if (cachedHeader != 0)
		MemCopy(header, cachedHeader, sizeof(header));
	else if (DMG_ReadFromFile(dmg, dmg->entries[n]->fileOffset, header, sizeof(header)) != sizeof(header))
	{
		DMG_SetError(DMG_ERROR_READING_FILE);
		return 0;
	}

	DMG_ParseClassicEntryHeader(dmg, dmg->entries[n], header);

	if (dmg->entries[n]->type == DMGEntry_Image &&
		(dmg->entries[n]->width == 0 || dmg->entries[n]->height == 0 || dmg->entries[n]->length == 0))
	{
		DMG_SetError(DMG_ERROR_INVALID_IMAGE);
		return 0;
	}

	// Actual limits should be related to screen mode
	if (dmg->entries[n]->type == DMGEntry_Image && (dmg->entries[n]->width > 1024 || dmg->entries[n]->height > 1024))
	{
		DMG_SetError(DMG_ERROR_IMAGE_TOO_BIG);
		return 0;
	}

	uint32_t entryHeaderSize = 6u;
	if (dmg->entries[n]->length + dmg->entries[n]->fileOffset + entryHeaderSize > dmg->fileSize)
	{
		DMG_Warning("Entry %d: Data offset %04X-%04X out of bounds (0-%04X)", n,
			dmg->entries[n]->fileOffset,
			dmg->entries[n]->fileOffset + entryHeaderSize + dmg->entries[n]->length - 1,
			dmg->fileSize);
		DMG_SetError(DMG_ERROR_DATA_OFFSET_OUT_OF_BOUNDS);
		return 0;
	}

	dmg->entries[n]->flags |= DMG_FLAG_PROCESSED;
	return dmg->entries[n];
}

/* ───────────────────────────────────────────────────────────────────────── */
/*  File management														     */
/* ───────────────────────────────────────────────────────────────────────── */

bool DMG_ReadV1DOSEntries(DMG* dmg, DMG_Version version)
{
	int n, p;
	uint16_t entryCount = 256;
	uint32_t directorySize = 2560;
	uint32_t imageCount = 0;

	if (version == DMG_Version1_PCW)
	{
		uint8_t header[6];
		if (!File_Seek(dmg->file, 0) || File_Read(dmg->file, header, sizeof(header)) != sizeof(header))
		{
			DMG_SetError(DMG_ERROR_READING_FILE);
			return false;
		}

		uint16_t pictureCount = read16(header + 4, true);
		if (pictureCount > 256)
		{
			DMG_SetError(DMG_ERROR_INVALID_ENTRY_COUNT);
			return false;
		}
	}

	if (!DMG_ReserveTemporaryBuffer(directorySize))
	{
		DMG_SetError(DMG_ERROR_OUT_OF_MEMORY);
		return false;
	}

	uint8_t* buffer = DMG_GetTemporaryBufferBase();
	uint32_t usedEntryCount = 0;

	File_Seek(dmg->file, 0x06);
	if (File_Read(dmg->file, buffer, directorySize) != directorySize)
	{
		DMG_SetError(DMG_ERROR_READING_FILE);
		return false;
	}

	for (n = 0; n < entryCount; n++)
	{
		uint8_t* ptr = buffer + n * 10;
		uint32_t offset = read32(ptr, dmg->littleEndian);
		if (offset != 0 && offset < 0xA06)
		{
			DMG_Warning("Entry %d: Data offset %06X out of bounds (0-%06X)", n, offset, dmg->fileSize);
			DMG_SetError(DMG_ERROR_DATA_OFFSET_OUT_OF_BOUNDS);
			return false;
		}
		if (offset != 0)
		{
			usedEntryCount++;
			imageCount++;
		}
	}

	if (usedEntryCount != 0)
	{
		dmg->entryBlock = Allocate<DMG_Entry>(version == DMG_Version1_PCW ? "PCW DAT entries" : "DMG entries", usedEntryCount);
		if (dmg->entryBlock == 0)
		{
			DMG_SetError(DMG_ERROR_OUT_OF_MEMORY);
			return false;
		}
		MemClear(dmg->entryBlock, sizeof(DMG_Entry) * usedEntryCount);
	}
	if (!DMG_AllocatePaletteBlock(dmg, imageCount * 16u, version == DMG_Version1_PCW ? "PCW palettes" : "Classic palettes"))
		return false;
	#if DMG_SUPPORT_CLASSIC_CONVERSION_PALETTES
	if (version != DMG_Version1_PCW && !DMG_AllocateConversionPaletteBlock(dmg, imageCount, "Classic conversion palettes"))
		return false;
	#endif

	uint32_t nextEntry = 0;
	uint32_t* paletteCursor = dmg->paletteBlock;
	#if DMG_SUPPORT_CLASSIC_CONVERSION_PALETTES
	DMG_CVPal* conversionCursor = dmg->conversionPaletteBlock;
	#endif

	for (n = 0; n < entryCount; n++)
	{
		uint8_t* ptr     = buffer + n * 10;
		uint32_t offset  = read32(ptr, dmg->littleEndian);
		uint16_t flags   = read16(ptr + 4, dmg->littleEndian);
		int16_t  x       = (int16_t)read16(ptr + 6, dmg->littleEndian);
		int16_t  y       = (int16_t)read16(ptr + 8, dmg->littleEndian);

		if (offset != 0 && offset < 0xA06)
		{
			DMG_Warning("Entry %d: Data offset %06X out of bounds (0-%06X)", n, offset, dmg->fileSize);
			DMG_SetError(DMG_ERROR_DATA_OFFSET_OUT_OF_BOUNDS);
			return false;
		}
		if (offset == 0)
			continue;
		
		dmg->entries[n] = dmg->entryBlock + nextEntry++;
		dmg->entries[n]->type = DMGEntry_Image;
		dmg->entries[n]->RGB32Palette = paletteCursor;
		paletteCursor += 16;
		#if DMG_SUPPORT_CLASSIC_CONVERSION_PALETTES
		if (version != DMG_Version1_PCW)
			dmg->entries[n]->conversionPalette = conversionCursor++;
		#endif
		dmg->entries[n]->bitDepth = version == DMG_Version1_CGA ? 2 :
			version == DMG_Version1_PCW ? 1 : 4;
		dmg->entries[n]->flags = 0;
        if ((flags & 0x0002) != 0)
		    dmg->entries[n]->flags |= DMG_FLAG_BUFFERED;
        if ((flags & 0x0001) == 0)
            dmg->entries[n]->flags |= DMG_FLAG_FIXED;
		dmg->entries[n]->x = x;
		dmg->entries[n]->y = y;
		dmg->entries[n]->fileOffset = offset;
		dmg->entries[n]->flags &= ~DMG_FLAG_PROCESSED;

		for (p = 0; p < 16; p++) {
			if (version == DMG_Version1_EGA)
				dmg->entries[n]->RGB32Palette[p] = EGAPalette[p];
			else if (version == DMG_Version1_PCW)
				dmg->entries[n]->RGB32Palette[p] = PCWDefaultPalette[p];
			else if (flags & 0x00800)
				dmg->entries[n]->RGB32Palette[p] = CGAPaletteRed[p];
			else
				dmg->entries[n]->RGB32Palette[p] = CGAPaletteCyan[p];
			uint8_t* egaPalette = DMG_GetEntryEGAPalette(dmg->entries[n]);
			uint8_t* cgaPalette = DMG_GetEntryCGAPalette(dmg->entries[n]);
			if (egaPalette != 0)
				egaPalette[p] = p;
			if (cgaPalette != 0)
				cgaPalette[p] = p & 0x03;
		}
	}
	return true;
}

bool DMG_LooksLikePCWDataFile(const uint8_t* header, size_t fileSize, const char* extension)
{
	if (fileSize < 6)
		return false;

	if (extension != NULL && StrIComp(extension, ".dat") != 0)
		return false;

	uint16_t machine = read16(header + 0, true);
	uint16_t mode = read16(header + 2, true);
	uint16_t pictureCount = read16(header + 4, true);
	uint32_t directorySize = 6u + 256u * 10u;

	if (machine != 0)
		return false;
	if (mode != 4)
		return false;
	if (pictureCount > 256)
		return false;
	if (directorySize > fileSize)
		return false;

	return true;
}

bool DMG_ReadV1Entries(DMG* dmg)
{
	int n, p;

	if (!DMG_ReserveTemporaryBuffer(44 * 256))
	{
		DMG_SetError(DMG_ERROR_OUT_OF_MEMORY);
		return false;
	}

	uint8_t* buffer = DMG_GetTemporaryBufferBase();
	uint32_t entryCount = 0;

	File_Seek(dmg->file, 0x06);
	if (File_Read(dmg->file, buffer, 44 * 256) != 44 * 256)
	{
		DMG_SetError(DMG_ERROR_READING_FILE);
		return false;
	}

	for (n = 0; n < 256; n++)
	{
		uint8_t* ptr = buffer + n * 44;
		uint32_t offset = read32(ptr, dmg->littleEndian);
		if (offset != 0 && (offset < 0x2C06 || offset >= dmg->fileSize))
		{
			DMG_Warning("Entry %d: Data offset %06X out of bounds (0-%06X)", n, offset, dmg->fileSize);
			DMG_SetError(DMG_ERROR_DATA_OFFSET_OUT_OF_BOUNDS);
			return false;
		}
		if (offset != 0)
			entryCount++;
	}

	if (entryCount != 0)
	{
		dmg->entryBlock = Allocate<DMG_Entry>("DMG entries", entryCount);
		if (dmg->entryBlock == 0)
		{
			DMG_SetError(DMG_ERROR_OUT_OF_MEMORY);
			return false;
		}
		MemClear(dmg->entryBlock, sizeof(DMG_Entry) * entryCount);
	}
	if (!DMG_AllocatePaletteBlock(dmg, entryCount * 16u, "Classic palettes"))
		return false;

	uint32_t nextEntry = 0;
	uint32_t* paletteCursor = dmg->paletteBlock;

	for (n = 0; n < 256; n++)
	{
		uint8_t* ptr = buffer + n * 44;

		uint32_t offset     = read32(ptr, dmg->littleEndian);
		uint16_t flags      = read16(ptr + 4, dmg->littleEndian);
		int16_t  x          = (int16_t)read16(ptr + 6, dmg->littleEndian);
		int16_t  y          = (int16_t)read16(ptr + 8, dmg->littleEndian);
		uint8_t  firstColor = ptr[10];
		uint8_t  lastColor  = ptr[11];

		if (offset != 0 && (offset < 0x2C06 || offset >= dmg->fileSize))
		{
			DMG_Warning("Entry %d: Data offset %06X out of bounds (0-%06X)", n, offset, dmg->fileSize);
			DMG_SetError(DMG_ERROR_DATA_OFFSET_OUT_OF_BOUNDS);
			return false;
		}
		if (offset == 0)
			continue;

		dmg->entries[n] = dmg->entryBlock + nextEntry++;
		dmg->entries[n]->type = DMGEntry_Image;
		dmg->entries[n]->RGB32Palette = paletteCursor;
		paletteCursor += 16;
		dmg->entries[n]->bitDepth = 4;
		dmg->entries[n]->flags = 0;
        if ((flags & 0x0002) != 0)
		    dmg->entries[n]->flags |= DMG_FLAG_BUFFERED;
        if ((flags & 0x0001) == 0)
            dmg->entries[n]->flags |= DMG_FLAG_FIXED;
		dmg->entries[n]->firstColor = firstColor;
		dmg->entries[n]->lastColor = lastColor;
		dmg->entries[n]->x = x;
		dmg->entries[n]->y = y;
		dmg->entries[n]->fileOffset = offset;
		dmg->entries[n]->flags &= ~DMG_FLAG_PROCESSED;

		if (offset == 0)
			dmg->entries[n]->type = DMGEntry_Empty;

		for (p = 0; p < 16; p++) 
		{
			uint16_t color = read16BE(ptr + 12 + p * 2);
			dmg->entries[n]->RGB32Palette[p] = Pal2RGB(color, false);
		}
	}
	return true;
}

bool DMG_ReadV2Entries(DMG* dmg)
{
	int n, p;
	uint8_t sizeBuffer[4];
	uint32_t size;
	uint32_t imageCount = 0;
	uint32_t conversionCount = 0;

	if (!DMG_ReserveTemporaryBuffer(48 * 256))
	{
		DMG_SetError(DMG_ERROR_OUT_OF_MEMORY);
		return false;
	}

	uint8_t* buffer = DMG_GetTemporaryBufferBase();
	uint32_t entryCount = 0;

	File_Seek(dmg->file, 0x06);
	if (File_Read(dmg->file, sizeBuffer, 4) != 4)
	{
		DMG_SetError(DMG_ERROR_READING_FILE);
		return false;
	}
	if (File_Read(dmg->file, buffer, 48 * 256) != 48 * 256)
	{
		DMG_SetError(DMG_ERROR_READING_FILE);
		return false;
	}
	size = read32(sizeBuffer, dmg->littleEndian);
	if (size != dmg->fileSize)
		DMG_Warning("Invalid file size %d on header (expected %d)", size, dmg->fileSize);

	for (n = 0; n < 256; n++)
	{
		uint8_t* ptr = buffer + n * 48;
		uint32_t offset = read32(ptr, dmg->littleEndian);
		if (offset != 0 && (offset < 0x300A || offset >= dmg->fileSize))
		{
			DMG_SetError(DMG_ERROR_DATA_OFFSET_OUT_OF_BOUNDS);
			return false;
		}
		if (offset != 0)
		{
			entryCount++;
			if ((read16(ptr + 4, dmg->littleEndian) & 0x0010) == 0)
			{
				imageCount++;
				uint32_t CGAColors = read32BE(ptr + 44);
				if (CGAColors != 0xDAADDAAD)
					conversionCount++;
			}
		}
	}

	if (entryCount != 0)
	{
		dmg->entryBlock = Allocate<DMG_Entry>("DMG entries", entryCount);
		if (dmg->entryBlock == 0)
		{
			DMG_SetError(DMG_ERROR_OUT_OF_MEMORY);
			return false;
		}
		MemClear(dmg->entryBlock, sizeof(DMG_Entry) * entryCount);
	}
	if (!DMG_AllocatePaletteBlock(dmg, imageCount * 16u, "Classic palettes"))
		return false;
	#if DMG_SUPPORT_CLASSIC_CONVERSION_PALETTES
	if (!DMG_AllocateConversionPaletteBlock(dmg, conversionCount, "Classic conversion palettes"))
		return false;
	DMG_CVPal* conversionCursor = dmg->conversionPaletteBlock;
	#endif

	uint32_t nextEntry = 0;
	uint32_t* paletteCursor = dmg->paletteBlock;

	for (n = 0; n < 256; n++)
	{
		uint8_t* ptr          = buffer + n * 48;
		uint32_t offset       = read32(ptr, dmg->littleEndian);
		uint16_t flags        = read16(ptr + 4, dmg->littleEndian);
		int16_t  x            = (int16_t)read16(ptr + 6, dmg->littleEndian);
		int16_t  y            = (int16_t)read16(ptr + 8, dmg->littleEndian);
		uint8_t  firstColor   = ptr[10];
		uint8_t  lastColor    = ptr[11];
		uint32_t CGAColors    = read32BE(ptr + 44);
		bool amigaPaletteHack = CGAColors == 0xDAADDAAD;

		if (offset != 0 && (offset < 0x300A || offset >= dmg->fileSize))
		{
			DMG_SetError(DMG_ERROR_DATA_OFFSET_OUT_OF_BOUNDS);
			return false;
		}
		if (offset == 0)
			continue;
		dmg->entries[n] = dmg->entryBlock + nextEntry++;
		dmg->entries[n]->type = (flags & 0x0010) ? DMGEntry_Audio : DMGEntry_Image;
		if (dmg->entries[n]->type == DMGEntry_Image)
		{
			dmg->entries[n]->RGB32Palette = paletteCursor;
			paletteCursor += 16;
		}
		dmg->entries[n]->bitDepth = 4;
		dmg->entries[n]->flags = 0;
        if ((flags & 0x0002) != 0)
            dmg->entries[n]->flags |= DMG_FLAG_BUFFERED;
        if ((flags & 0x0001) == 0)
            dmg->entries[n]->flags |= DMG_FLAG_FIXED;
		dmg->entries[n]->firstColor = firstColor;
		dmg->entries[n]->lastColor = lastColor;
		dmg->entries[n]->x = x;
		dmg->entries[n]->y = y;
		dmg->entries[n]->fileOffset = offset;
		dmg->entries[n]->flags &= ~DMG_FLAG_PROCESSED;

		if (offset == 0)
			dmg->entries[n]->type = DMGEntry_Empty;

		#if DMG_SUPPORT_CLASSIC_CONVERSION_PALETTES
		if (dmg->entries[n]->type == DMGEntry_Image && !amigaPaletteHack) {
			dmg->entries[n]->conversionPalette = conversionCursor++;
		}
		if (dmg->entries[n]->type == DMGEntry_Image)
		{
			uint8_t* egaPalette = DMG_GetEntryEGAPalette(dmg->entries[n]);
			uint8_t* cgaPalette = DMG_GetEntryCGAPalette(dmg->entries[n]);
			for (p = 0; p < 16; p++) 
			{
				uint16_t color = read16BE(ptr + 12 + p * 2);
				dmg->entries[n]->RGB32Palette[p] = Pal2RGB(color, amigaPaletteHack);
				if (egaPalette != 0)
					egaPalette[p] = (color >> 12) & 0x0F;
				if (cgaPalette != 0)
					cgaPalette[p] = (CGAColors >> (2*p)) & 0x03;
			}
		}
		#else
		if (dmg->entries[n]->type == DMGEntry_Image)
		{
			for (p = 0; p < 16; p++) 
			{
				uint16_t color = read16BE(ptr + 12 + p * 2);
				dmg->entries[n]->RGB32Palette[p] = Pal2RGB(color, amigaPaletteHack);
			}
		}
		#endif

        if (amigaPaletteHack)
            dmg->entries[n]->flags |= DMG_FLAG_AMIPALHACK;
        else
            dmg->entries[n]->flags &= ~DMG_FLAG_AMIPALHACK;
        DMG_SetCGAMode(dmg->entries[n], (flags & 0x0001) ? CGA_Red : CGA_Blue);
	}
	return true;
}

int DMG_GetEntryCount(DMG* dmg)
{
	int n;
	int count = 0;

	if (dmg == 0)
		return 0;
	for (n = 0; n < 256; n++)
	{
		if (dmg->entries[n] != 0 && dmg->entries[n]->type != DMGEntry_Empty)
            count++;
	}
	return count;
}

DMG* DMG_Open(const char* filename, bool readOnly)
{
	DMG* d;
	File* file;
	uint8_t header[16];
	uint16_t signature;
	uint16_t entryCount;
	uint16_t realEntryCount;
	bool success;
	const char* extension = StrRChr(filename, '.');
	uint64_t rawFileSize;

	DMG_SetError(DMG_ERROR_NONE);
	
	file = File_Open(filename, readOnly ? ReadOnly : ReadWrite);
	if (file == 0)
	{
		DMG_SetError(DMG_ERROR_FILE_NOT_FOUND);
		return 0;
	}

	rawFileSize = File_GetSize(file);
	if (rawFileSize < DMG_MIN_FILE_SIZE)
	{
		DMG_SetError(DMG_ERROR_FILE_TOO_SMALL);
		File_Close(file);
		return 0;
	}
	if (rawFileSize > DMG_MAX_FILE_SIZE)
	{
		DMG_SetError(DMG_ERROR_FILE_TOO_BIG);
		File_Close(file);
		return 0;
	}

	d = Allocate<DMG>("DMG");
	if (d == 0)
	{
		DMG_SetError(DMG_ERROR_OUT_OF_MEMORY);
		File_Close(file);
		return 0;
	}
    MemClear(d, sizeof(DMG));
	d->file = file;
	d->fileSize = (uint32_t)rawFileSize;
    d->dirty = false;

	// Read file header
	MemClear(header, sizeof(header));
	if (File_Read(file, header, 10) != 10)
	{
		DMG_SetError(DMG_ERROR_READING_FILE);
		DMG_Close(d);
		return 0;
	}
	signature = read16BE(header);
	if (DMG_LooksLikePCWDataFile(header, rawFileSize, extension))
	{
		d->version = DMG_Version1_PCW;
		d->littleEndian = true;
		d->screenMode = ScreenMode_HiRes;
		success = DMG_ReadV1DOSEntries(d, DMG_Version1_PCW);
	}

	else if (header[0] == 'D' && header[1] == 'A' && header[2] == 'T' && header[3] == 0 && header[4] == 0 && header[5] == 5)
	{
		success = DMG_ReadDAT5Entries(d);
		entryCount = d->lastEntry >= d->firstEntry ? (uint16_t)(d->lastEntry - d->firstEntry + 1) : 0;
	}
	else switch (signature)
	{
		case 0x0004:
			// Old version DAT file, big endian
			d->version = DMG_Version1;
			success = DMG_ReadV1Entries(d);
			break;

		case 0x0300:
			// New version DAT file, big endian
			d->version = DMG_Version2;
			d->littleEndian = false;
			d->screenMode = read16BE(header + 2);
			success = DMG_ReadV2Entries(d);
			break;

		case 0xFFFF:
			// New version DAT file, little endian
			d->version = DMG_Version2;
			d->littleEndian = true;
			d->screenMode = read16BE(header + 2);
			success = DMG_ReadV2Entries(d);
			break;

		case 0x0000:
			if (StrIComp(extension, ".ega") == 0)
			{
				d->version = DMG_Version1_EGA;
				d->littleEndian = true;
				d->screenMode = ScreenMode_EGA;
				success = DMG_ReadV1DOSEntries(d, DMG_Version1_EGA);
				break;
			}
			else if (StrIComp(extension, ".cga") == 0)
			{
				d->version = DMG_Version1_CGA;
				d->littleEndian = true;
				d->screenMode = ScreenMode_CGA;
				success = DMG_ReadV1DOSEntries(d, DMG_Version1_CGA);
				break;
			}
			// Fall through

		default:
			DMG_Warning("Unknown signature: %04X", signature);
			DMG_SetError(DMG_ERROR_UNKNOWN_SIGNATURE);
			success = false;
			break;
	}

	if (success == false)
	{
		DMG_Close(d);
		return 0;
	}

	#if !defined(_AMIGA)
	if (d->version == DMG_Version1)
		DMG_InitializeOldRLETables();
	#endif

	uint32_t minimumBuffer = DMG_GetMinimumImageBufferSize(d, DMG_NATIVE_IMAGE_MODE);
	if (minimumBuffer != 0 && DMG_GetTemporaryBufferSize() < minimumBuffer)
	{
		if (!DMG_ReserveTemporaryBuffer(minimumBuffer))
		{
			DMG_SetError(DMG_ERROR_OUT_OF_MEMORY);
			DMG_Close(d);
			return 0;
		}
	}

	if (!DMG_PreallocateZX0Scratch(d))
	{
		DMG_Close(d);
		return 0;
	}
	if (d->version != DMG_Version5 && d->version != DMG_Version1_PCW)
    {
		entryCount = read16(header + 4, d->littleEndian);
    	realEntryCount = DMG_GetEntryCount(d);
	    if (entryCount != realEntryCount)
		    DMG_Warning("Invalid entry count %d on file header (expected %d)", entryCount, realEntryCount);
    }

	if (dmg == 0)
		dmg = d;
	return d;
}

uint32_t* DMG_GetEntryPalette(DMG* dmg, uint8_t index)
{
	DMG_Entry* entry = DMG_GetEntry(dmg, index);
	if (entry == 0)
		return 0;

	switch (dmg->screenMode)
	{
		case ScreenMode_CGA:
			return DMG_GetCGAMode(entry) == CGA_Red ? CGAPaletteRed : CGAPaletteCyan;
		case ScreenMode_EGA:
			return EGAPalette;
		default:
			if (dmg->version == DMG_Version5)
			{
				if (!DMG_LoadDAT5Palette(dmg, index, entry))
					return 0;
				return entry->RGB32Palette;
			}
			else
				return entry->RGB32Palette;
	}
}

uint32_t* DMG_GetEntryStoredPalette(DMG* dmg, uint8_t index)
{
	DMG_Entry* entry = DMG_GetEntry(dmg, index);
	if (entry == 0)
		return 0;

	if (dmg->version == DMG_Version5)
	{
		if (!DMG_LoadDAT5Palette(dmg, index, entry))
			return 0;
		return entry->RGB32Palette;
	}

	return entry->RGB32Palette;
}

uint16_t DMG_GetEntryPaletteSize(DMG* dmg, uint8_t index)
{
    DMG_Entry* entry = DMG_GetEntry(dmg, index);
    if (entry == 0)
        return 0;
    if (dmg->version == DMG_Version5)
    {
        if (dmg->colorMode == DMG_DAT5_COLORMODE_CGA)
            return 4;
        if (dmg->colorMode == DMG_DAT5_COLORMODE_EGA)
            return 16;
        return entry->paletteColors;
    }
    return 16;
}

uint8_t DMG_GetEntryFirstColor(DMG* dmg, uint8_t index)
{
    DMG_Entry* entry = DMG_GetEntry(dmg, index);
    if (entry == 0)
        return 0;
    return entry->firstColor;
}

void DMG_Close(DMG* d)
{
	if (d == 0)
		return;

	#if DEBUG_ZX0
	if (d->zx0ProfileCount != 0)
	{
		uint32_t averageMs = d->zx0ProfileTotalMs / d->zx0ProfileCount;
		DebugPrintf("DAT5 ZX0 summary: %lu images, %lu -> %lu bytes, %lu ms total, %lu ms avg, slowest image %u (%lu ms)\n",
			(unsigned long)d->zx0ProfileCount,
			(unsigned long)d->zx0ProfileInputBytes,
			(unsigned long)d->zx0ProfileOutputBytes,
			(unsigned long)d->zx0ProfileTotalMs,
			(unsigned long)averageMs,
			(unsigned)d->zx0ProfileMaxIndex,
			(unsigned long)d->zx0ProfileMaxMs);
	}
	#endif

	DMG_FreeImageCache(d);
	
	if (d->paletteBlock != 0)
		Free(d->paletteBlock);
	if (d->conversionPaletteBlock != 0)
		Free(d->conversionPaletteBlock);
	if (d->fileCacheData != 0)
		Free(d->fileCacheData);

    for (int i = 0; i < 256; i++)
    {
		if (d->entries[i] != 0 && d->entries[i]->RGB32Palette != 0 && DMG_EditableEntryOwnsPalette(d, (uint8_t)i))
			Free(d->entries[i]->RGB32Palette);
		if (d->entries[i] != 0 && d->entries[i]->conversionPalette != 0 && DMG_EditableEntryOwnsConversionPalette(d, (uint8_t)i))
			Free(d->entries[i]->conversionPalette);
		if (DMG_GetEntryStoredData(d, (uint8_t)i) != 0 && DMG_EditableEntryOwnsStoredData(d, (uint8_t)i))
			Free(DMG_GetEntryStoredData(d, (uint8_t)i));
    }
	if (d->editableEntries != 0)
		Free(d->editableEntries);
	if (d->entryBlock != 0)
	{
		Free(d->entryBlock);
	}
	else
	{
		for (int i = 0; i < 256; i++)
		{
			if (d->entries[i] != 0)
				Free(d->entries[i]);
		}
    }

	if (d->zx0Scratch != 0 && d->zx0ScratchOwned)
		Free(d->zx0Scratch);

	File_Close(d->file);
	Free(d);

	if (dmg == d)
		dmg = 0;
}

uint32_t DMG_ReadFromFile (DMG* dmg, uint32_t offset, void* buffer, uint32_t size)
{
	void* cached = DMG_GetFromFileCache(dmg, offset, size);
	if (cached != 0)
	{
		MemCopy(buffer, cached, size);
		return size;
	}

	if (!File_Seek(dmg->file, offset))
		return 0;
	return File_Read(dmg->file, buffer, size);
}

uint32_t DMG_CalculateRequiredSize (DMG_Entry* entry, DMG_ImageMode mode)
{
	uint16_t width  = entry->width;
	uint16_t height = entry->height;

	if (mode == ImageMode_Audio && entry->type == DMGEntry_Audio)
	{
		return entry->length;
	}

	if (mode == ImageMode_Raw)
	{
		if (dmg != 0 && dmg->version == DMG_Version5)
		{
			uint32_t storedSize = DMG_DAT5StoredImageSize(dmg->colorMode, width, height);
			if (storedSize != 0)
				return storedSize;
			if (entry->bitDepth <= 4)
				return (width * height + 1) / 2;
			return width * height;
		}
		if (dmg != 0 && dmg->version == DMG_Version1_PCW)
			return ((width + 7) >> 3) * height;
		if (dmg != 0 && dmg->version == DMG_Version1_CGA)
			return width * height / 4;
		return ((uint32_t)width * (uint32_t)height + 1u) >> 1;
	}

	switch (mode)
	{
		case ImageMode_Packed:
            if (dmg != 0 && dmg->version == DMG_Version5)
            {
				if (DMG_DAT5ModeIsIndexedX(dmg->colorMode))
					return DMG_DAT5StoredImageSize(dmg->colorMode, width, height);
				if (DMG_DAT5ModeIsIndexed(dmg->colorMode))
					return DMG_DAT5StoredImageSize(dmg->colorMode, width, height);
                if (entry->bitDepth <= 4)
                    return (width * height + 1) / 2;
                return width * height;
            }
			return ((uint32_t)width * (uint32_t)height + 1u) >> 1;

		case ImageMode_IndexedX:
			return ((width + 3u) & ~3u) * height;

		case ImageMode_CGA:
			return (((uint32_t)width + 3u) >> 2) * (uint32_t)height;

		case ImageMode_Planar:
			if (dmg != 0 && dmg->version == DMG_Version5 && DMG_DAT5ModeIsPlaneMajor(dmg->colorMode))
				return DMG_DAT5StoredImageSize(dmg->colorMode, width, height);
            return ((width + 7) >> 3) * height * entry->bitDepth;

		case ImageMode_PlanarST:
			if (dmg != 0 && dmg->version == DMG_Version5 && DMG_DAT5ModeIsSTInterleaved(dmg->colorMode) && entry->bitDepth <= 4)
				return DMG_DAT5StoredImageSize(dmg->colorMode, width, height);
			return ((width + 15) & ~15) * height / 2;

		case ImageMode_PlanarFalcon:
			if (dmg != 0 && dmg->version == DMG_Version5 && DMG_DAT5ModeIsSTInterleaved(dmg->colorMode) && entry->bitDepth == 8)
				return DMG_DAT5StoredImageSize(dmg->colorMode, width, height);
			return ((width + 15) & ~15) * height;

		case ImageMode_RGBA32:
			return width * height * 4;

		case ImageMode_Indexed:
			return width * height;

		default:
			return width * height / 2;
	}
}
