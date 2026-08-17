#include <dmg.h>
#include <ddb_vid.h>
#include <os_lib.h>
#include <os_bito.h>
#include <os_mem.h>
#include <os_types.h>

#if defined(_AMIGA)

__attribute__((noinline, used))
bool DMG_DecompressOldRLEToPlanarST (const uint8_t* data, uint16_t rleMask, uint16_t dataLength, uint8_t* buffer, int pixels, bool littleEndian)
{
	(void)data;
	(void)rleMask;
	(void)dataLength;
	(void)buffer;
	(void)pixels;
	(void)littleEndian;
	DMG_SetError(DMG_ERROR_INVALID_IMAGE);
	return false;
}

#else

static const uint16_t prefixMask16[17] =
{
	0x0000u,
	0x8000u,
	0xC000u,
	0xE000u,
	0xF000u,
	0xF800u,
	0xFC00u,
	0xFE00u,
	0xFF00u,
	0xFF80u,
	0xFFC0u,
	0xFFE0u,
	0xFFF0u,
	0xFFF8u,
	0xFFFCu,
	0xFFFEu,
	0xFFFFu
};

uint16_t DMGOldRLEFullWordPlanes16[16][4];
uint16_t DMGOldRLESpanMask16[17][17];
uint32_t DMGOldRLEPackedColorBits0[256];
uint32_t DMGOldRLEPackedColorBits1[256];
uint32_t DMGOldRLEPackedColorBits2[256];
uint32_t DMGOldRLEPackedColorBits3[256];

static bool packedColorBitsInitialized = false;

void DMG_InitializeOldRLETables()
{
	if (packedColorBitsInitialized)
		return;

	for (uint32_t value = 0; value < 256; value++)
	{
		uint32_t packed = 0;
		for (uint32_t bit = 0; bit < 8; bit++)
		{
			if ((value & (0x80u >> bit)) != 0)
				packed |= 1u << (28 - bit * 4);
		}
			DMGOldRLEPackedColorBits0[value] = packed;
			DMGOldRLEPackedColorBits1[value] = packed << 1;
			DMGOldRLEPackedColorBits2[value] = packed << 2;
			DMGOldRLEPackedColorBits3[value] = packed << 3;
	}

	for (uint32_t bitsUsed = 0; bitsUsed <= 16; bitsUsed++)
	{
		for (uint32_t span = 0; span <= 16; span++)
			DMGOldRLESpanMask16[bitsUsed][span] = (uint16_t)(prefixMask16[span] >> bitsUsed);
	}

	for (uint32_t color = 0; color < 16; color++)
	{
		DMGOldRLEFullWordPlanes16[color][0] = (color & 0x1) ? 0xFFFFu : 0x0000u;
		DMGOldRLEFullWordPlanes16[color][1] = (color & 0x2) ? 0xFFFFu : 0x0000u;
		DMGOldRLEFullWordPlanes16[color][2] = (color & 0x4) ? 0xFFFFu : 0x0000u;
		DMGOldRLEFullWordPlanes16[color][3] = (color & 0x8) ? 0xFFFFu : 0x0000u;
	}

	packedColorBitsInitialized = true;
}

static bool DMG_DecompressOldRLEToST_C(const uint8_t* data, uint16_t rleMask, uint16_t dataLength, uint8_t* buffer, int pixels)
{
	Abort();
	
    const uint8_t* src = data;
    uint32_t packedColors = 0;
    uint8_t packedColorCount = 0;

    uint16_t* out = (uint16_t*)buffer;

    uint16_t plane0 = 0;
    uint16_t plane1 = 0;
    uint16_t plane2 = 0;
    uint16_t plane3 = 0;
    uint8_t bitsUsed = 0;

    while (pixels > 0)
    {
		if (packedColorCount == 0)
		{
			packedColors =
					 DMGOldRLEPackedColorBits0[src[0]] |
					 DMGOldRLEPackedColorBits1[src[1]] |
					 DMGOldRLEPackedColorBits2[src[2]] |
					 DMGOldRLEPackedColorBits3[src[3]];
			src += 4;
			packedColorCount = 8;
		}

		uint8_t color = (uint8_t)(packedColors >> 28);
		packedColors <<= 4;
		packedColorCount--;
		uint8_t count = 1;
		if ((rleMask & (1u << color)) != 0)
		{
			if (packedColorCount == 0)
			{
				packedColors =
						 DMGOldRLEPackedColorBits0[src[0]] |
						 DMGOldRLEPackedColorBits1[src[1]] |
						 DMGOldRLEPackedColorBits2[src[2]] |
						 DMGOldRLEPackedColorBits3[src[3]];
				src += 4;
				packedColorCount = 8;
			}

			count += (uint8_t)(packedColors >> 28);
			packedColors <<= 4;
			packedColorCount--;
		}
		if (count > pixels)
			count = (uint8_t)pixels;
		pixels -= count;

		if (bitsUsed == 0 && count == 16)
		{
			const uint16_t* fullWord = DMGOldRLEFullWordPlanes16[color];
			*out++ = fullWord[0];
			*out++ = fullWord[1];
			*out++ = fullWord[2];
			*out++ = fullWord[3];
			continue;
		}

		while (count > 0)
		{
			uint8_t span = (uint8_t)(16 - bitsUsed);
			if (span > count)
				span = count;

			uint16_t mask = DMGOldRLESpanMask16[bitsUsed][span];
			if (color & 0x1) plane0 |= mask;
			if (color & 0x2) plane1 |= mask;
			if (color & 0x4) plane2 |= mask;
			if (color & 0x8) plane3 |= mask;

			bitsUsed += span;
			count -= span;

			if (bitsUsed == 16)
			{
				*out++ = plane0;
				*out++ = plane1;
				*out++ = plane2;
				*out++ = plane3;

				plane0 = 0;
				plane1 = 0;
				plane2 = 0;
				plane3 = 0;
				bitsUsed = 0;
			}
		}
    }

	if (bitsUsed != 0)
	{
		*out++ = plane0;
		*out++ = plane1;
		*out++ = plane2;
		*out++ = plane3;
	}

	DebugPrintf("Decompress old RLE finished at %p (%lu bytes written, tail has %d pixels)\n",
		out, (unsigned long)((uint8_t*)out - (uint8_t*)buffer), bitsUsed);

    return true;
}

bool DMG_DecompressOldRLEToPlanarST (const uint8_t* data, uint16_t rleMask, uint16_t dataLength, uint8_t* buffer, int pixels, bool littleEndian)
{
	#if DMG_SUPPORT_CROSS_ENDIAN_SOURCES
	bool shouldSwap = !DMG_IsHostByteOrder(littleEndian);
	if (shouldSwap)
	{
		uint32_t* src = (uint32_t*)data;
		int wordCount = (dataLength + 3) >> 2;

		DebugPrintf("WARNING: swapping order of old RLE image");
		for (int n = 0; n < wordCount; n++)
			src[n] = fix32(src[n], littleEndian);
	}
	#endif

	return DMG_DecompressOldRLEToST_C(data, rleMask, dataLength, buffer, pixels);
}

#endif
