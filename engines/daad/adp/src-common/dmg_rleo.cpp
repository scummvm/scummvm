#include <dmg.h>
#include <ddb_vid.h>
#include <os_lib.h>
#include <os_bito.h>

bool DMG_DecompressOldRLE (const uint8_t* data, uint16_t rleMask, uint16_t dataLength, uint8_t* buffer, int pixels, bool littleEndian)
{
	#if !DMG_SUPPORT_CROSS_ENDIAN_SOURCES
	(void)littleEndian;
	#endif
	uint32_t nibbles = 0;
	uint8_t color = 0;
	uint8_t repetitions;
	const uint8_t* start = data;
	const uint8_t* ptr = data;
	uint16_t remaining = dataLength;
	int nibbleCount = 0;

	uint8_t outc = 0;
	bool outcm = false;

	DMG_SetError(DMG_ERROR_NONE);
	
	#ifdef DEBUG_RLE
	DebugPrintf("Decompressing old RLE image (%u bytes)\n", (unsigned)dataLength);
	#endif

	#ifdef DEBUG
	uint32_t t0, t1;
	VID_GetMilliseconds(&t0);
	#endif

	uint8_t rle[16];
	for (int n = 0; n < 16; n++)
		rle[n] = (rleMask & (1 << n)) != 0;

	while (pixels > 0)
	{
		if (nibbleCount == 0)
		{
			if (remaining < 4)
			{
				if (remaining == 0)
				{
					while (pixels-- > 0)
					{
						if (outcm)
							*buffer++ = color | (outc << 4);
						else
							outc = color;
						outcm = !outcm;
					}
					break;
				}

				uint8_t temp[4] = { 0, 0, 0, 0 };
				for (uint16_t i = 0; i < remaining; i++)
					temp[i] = ptr[i];
				#if DMG_SUPPORT_CROSS_ENDIAN_SOURCES
				nibbles = read32(temp, littleEndian);
				#else
				nibbles = read32(temp, DMG_HOST_LITTLE_ENDIAN);
				#endif
				ptr += remaining;
				remaining = 0;
			}
			else
			{
				#if DMG_SUPPORT_CROSS_ENDIAN_SOURCES
				nibbles = read32(ptr, littleEndian);
				#else
				nibbles = read32(ptr, DMG_HOST_LITTLE_ENDIAN);
				#endif
				ptr += 4;
				remaining -= 4;
			}
			nibbleCount = 8;
		}
		
		nibbleCount--;
		color = 
			((nibbles & 0x00000080) >> 4) |
			((nibbles & 0x00008000) >> 13) |
			((nibbles & 0x00800000) >> 22) |
			((nibbles & 0x80000000) >> 31);
		nibbles <<= 1;
		
		if (outcm)
			*buffer++ = color | (outc << 4);
		else 
			outc = color;
		outcm = !outcm;

		pixels--;

		if (rle[color]) 
		{
			if (nibbleCount == 0)
			{
				if (remaining < 4)
				{
					while (pixels-- > 0)
					{
						if (outcm)
							*buffer++ = color | (outc << 4);
						else
							outc = color;
						outcm = !outcm;
					}
					break;
				}

				#if DMG_SUPPORT_CROSS_ENDIAN_SOURCES
				nibbles = read32(ptr, littleEndian);
				#else
				nibbles = read32(ptr, DMG_HOST_LITTLE_ENDIAN);
				#endif
				ptr += 4;
				remaining -= 4;
				nibbleCount = 8;
			}

			nibbleCount--;
			repetitions = 
				((nibbles & 0x00000080) >> 4) |
				((nibbles & 0x00008000) >> 13) |
				((nibbles & 0x00800000) >> 22) |
				((nibbles & 0x80000000) >> 31);
			nibbles <<= 1;

			pixels -= repetitions;
			while (repetitions-- > 0)
			{
				if (outcm)
					*buffer++ = color | (outc << 4);
				else 
					outc = color;
				outcm = !outcm;
			}
		}
	}

	if (outcm)
		*buffer++ = (outc << 4);

	if (remaining > 4)
		DMG_Warning("Data stream contains %u extra bytes (at offset %lu)",
			(unsigned)remaining, (unsigned long)(ptr - start));

	#ifdef DEBUG
	VID_GetMilliseconds(&t1);
	DebugPrintf("Decompressed old RLE image in %lu ms\n", (unsigned long)(t1 - t0));
	#endif

	return true;
}
