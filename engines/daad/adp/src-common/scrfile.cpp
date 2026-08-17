#include <ddb.h>
#include <ddb_pal.h>
#include <os_file.h>

static bool SCR_HasPI1Header(const uint8_t* data, uint64_t size)
{
	(void)data;
	return size == 32034 || size == 32066;
}

static bool SCR_HasFalconRawHeader(const uint8_t* data, uint64_t size)
{
	return size == (uint64_t)(4 + 256 * 3 + 64000) &&
		data[0] == 'F' && data[1] == 'C' && data[2] == 'R' && data[3] == '1';
}

static void SCR_DecodePlanarST(const uint8_t* data, uint8_t* output, int width, int height, uint32_t* palette)
{
	uint8_t* filePalette = (uint8_t*)data;
	for (int n = 0; n < 16; n++)
		palette[n] = Pal2RGB((filePalette[n * 2] << 8) + filePalette[n * 2 + 1], false);

	for (int y = 0; y < 200 && y < height; y++)
	{
		uint8_t* row = (uint8_t*)data + 32 + y * 160;
		uint8_t* ptr = output + y * width;
		for (int x = 0; x < 160; x += 8)
		{
			uint16_t bits0 = (row[x+0] << 8) | row[x+1];
			uint16_t bits1 = (row[x+2] << 8) | row[x+3];
			uint16_t bits2 = (row[x+4] << 8) | row[x+5];
			uint16_t bits3 = (row[x+6] << 8) | row[x+7];

			int mask = 0x8000;
			do
			{
				int color = ((bits0 & mask) ? 0x01 : 0x00);
				if (bits1 & mask) color |= 0x02;
				if (bits2 & mask) color |= 0x04;
				if (bits3 & mask) color |= 0x08;
				*ptr++ = color;
				mask >>= 1;
			}
			while (mask);
		}
	}
}

static void SCR_DecodePlanarFalcon(const uint8_t* data, uint8_t* output, int width, int height, uint32_t* palette)
{
	const uint8_t* filePalette = data + 4;
	for (int n = 0; n < 256; n++)
	{
		palette[n] = 0xFF000000UL |
			((uint32_t)filePalette[n * 3 + 0] << 16) |
			((uint32_t)filePalette[n * 3 + 1] << 8) |
			(uint32_t)filePalette[n * 3 + 2];
	}

	const uint8_t* imageData = data + 4 + 256 * 3;
	for (int y = 0; y < 200 && y < height; y++)
	{
		const uint8_t* row = imageData + y * 320;
		uint8_t* ptr = output + y * width;
		for (int x = 0; x < 320; x += 16)
		{
			uint16_t planes[8];
			for (int plane = 0; plane < 8; plane++)
				planes[plane] = (uint16_t)(row[plane * 2] << 8) | row[plane * 2 + 1];

			int mask = 0x8000;
			do
			{
				uint8_t color = 0;
				for (int plane = 0; plane < 8; plane++)
					if (planes[plane] & mask)
						color |= (uint8_t)(1u << plane);
				*ptr++ = color;
				mask >>= 1;
			}
			while (mask != 0);

			row += 16;
		}
	}
}

static uint32_t SCR_ReadBE32(const uint8_t* data)
{
	return ((uint32_t)data[0] << 24) | ((uint32_t)data[1] << 16) |
	       ((uint32_t)data[2] << 8) | (uint32_t)data[3];
}

static bool SCR_HasILBMHeader(const uint8_t* data, uint64_t size)
{
	return size >= 12 &&
		data[0] == 'F' && data[1] == 'O' && data[2] == 'R' && data[3] == 'M' &&
		data[8] == 'I' && data[9] == 'L' && data[10] == 'B' && data[11] == 'M';
}

// Amiga IFF ILBM loading screens, including HAM6. The output receives one
// byte per pixel holding that pixel's plane bits; for a HAM image those
// are HAM commands to run against the 16-color base palette, and *isHAM
// tells the caller to render them that way.
static bool SCR_DecodeILBM(const uint8_t* data, uint64_t size,
	uint8_t* output, int width, int height, uint32_t* palette, bool* isHAM)
{
	uint16_t imgWidth = 0, imgHeight = 0;
	uint8_t planes = 0, masking = 0, compression = 0;
	const uint8_t* body = 0;
	uint32_t bodySize = 0;
	bool ham = false;

	uint64_t offset = 12;
	while (offset + 8 <= size)
	{
		const uint8_t* chunk = data + offset;
		uint32_t chunkSize = SCR_ReadBE32(chunk + 4);
		if (offset + 8 + chunkSize > size)
			break;
		const uint8_t* content = chunk + 8;
		if (chunk[0] == 'B' && chunk[1] == 'M' && chunk[2] == 'H' && chunk[3] == 'D' && chunkSize >= 11)
		{
			imgWidth = (uint16_t)((content[0] << 8) | content[1]);
			imgHeight = (uint16_t)((content[2] << 8) | content[3]);
			planes = content[8];
			masking = content[9];
			compression = content[10];
		}
		else if (chunk[0] == 'C' && chunk[1] == 'M' && chunk[2] == 'A' && chunk[3] == 'P')
		{
			uint32_t count = chunkSize / 3;
			for (uint32_t n = 0; n < count && n < 32; n++)
			{
				palette[n] = 0xFF000000UL |
					((uint32_t)content[n * 3] << 16) |
					((uint32_t)content[n * 3 + 1] << 8) |
					(uint32_t)content[n * 3 + 2];
			}
		}
		else if (chunk[0] == 'C' && chunk[1] == 'A' && chunk[2] == 'M' && chunk[3] == 'G' && chunkSize >= 4)
		{
			ham = (SCR_ReadBE32(content) & 0x0800) != 0;
		}
		else if (chunk[0] == 'B' && chunk[1] == 'O' && chunk[2] == 'D' && chunk[3] == 'Y')
		{
			body = content;
			bodySize = chunkSize;
		}
		offset += 8 + chunkSize + (chunkSize & 1);
	}

	if (body == 0 || planes < 1 || planes > 6 || imgWidth < 16 || imgHeight == 0)
		return false;
	if (compression > 1)
		return false;

	uint16_t rowBytes = (uint16_t)(((imgWidth + 15) >> 4) * 2);
	uint8_t rowBuffer[80];
	if (rowBytes > sizeof(rowBuffer))
		return false;

	const uint8_t* src = body;
	const uint8_t* srcEnd = body + bodySize;
	int rowPlanes = planes + (masking == 1 ? 1 : 0);
	for (int y = 0; y < imgHeight; y++)
	{
		uint8_t* out = (y < height) ? output + y * width : 0;
		if (out != 0)
		{
			for (int x = 0; x < imgWidth && x < width; x++)
				out[x] = 0;
		}
		for (int p = 0; p < rowPlanes; p++)
		{
			if (compression == 0)
			{
				if (src + rowBytes > srcEnd)
					return false;
				for (int n = 0; n < rowBytes; n++)
					rowBuffer[n] = src[n];
				src += rowBytes;
			}
			else
			{
				int filled = 0;
				while (filled < rowBytes)
				{
					if (src >= srcEnd)
						return false;
					int8_t control = (int8_t)*src++;
					if (control >= 0)
					{
						int run = control + 1;
						if (src + run > srcEnd || filled + run > rowBytes)
							return false;
						for (int n = 0; n < run; n++)
							rowBuffer[filled++] = *src++;
					}
					else if (control != -128)
					{
						int run = 1 - control;
						if (src >= srcEnd || filled + run > rowBytes)
							return false;
						uint8_t value = *src++;
						for (int n = 0; n < run; n++)
							rowBuffer[filled++] = value;
					}
				}
			}
			if (out == 0 || p >= planes)
				continue;
			for (int x = 0; x < imgWidth && x < width; x++)
			{
				if (rowBuffer[x >> 3] & (0x80 >> (x & 7)))
					out[x] |= (uint8_t)(1 << p);
			}
		}
	}

	if (isHAM != 0)
		*isHAM = ham && planes == 6;
	return true;
}

bool SCR_GetScreen (const char* fileName, DDB_Machine target, 
                    uint8_t* buffer, size_t bufferSize, 
                    uint8_t* output, int width, int height, uint32_t* palette,
                    bool* isHAM)
{
	if (isHAM != 0)
		*isHAM = false;
	File* file = File_Open(fileName, ReadOnly);
	if (file == 0)
	{
		DDB_SetError(DDB_ERROR_FILE_NOT_FOUND);
		return false;
	}

	uint64_t size = File_GetSize(file);
	bool amigaSized = target == DDB_MACHINE_AMIGA && size >= 128 && size <= (uint64_t)bufferSize;
	#if HAS_SPECTRUM
	if (target == DDB_MACHINE_SPECTRUM)
	{
		if (size < 6912 || size > 7040)
		{
			DDB_SetError(DDB_ERROR_INVALID_FILE);
			File_Close(file);
			return false;
		}
	}
	else if (!amigaSized && size != (uint64_t)(4 + 256 * 3 + 64000) && (size < 16384 || size > 32768))
	#else
	if (!amigaSized && size != (uint64_t)(4 + 256 * 3 + 64000) && (size < 16384 || size > 32768))
	#endif
	{
		DDB_SetError(DDB_ERROR_INVALID_FILE);
		File_Close(file);
		return false;
	}

	if (bufferSize < size)
	{
		DDB_SetError(DDB_ERROR_OUT_OF_MEMORY);
		File_Close(file);
		return false;
	}

	File_Read(file, buffer, size);
	File_Close(file);

	#if HAS_SPECTRUM
	if (target == DDB_MACHINE_SPECTRUM)
	{
		uint64_t logicalSize = size;
		while (logicalSize > 6912 && (buffer[logicalSize - 1] == 0x00 || buffer[logicalSize - 1] == 0xE5))
			logicalSize--;
		if (logicalSize != 6912)
		{
			DDB_SetError(DDB_ERROR_INVALID_FILE);
			return false;
		}

		for (int n = 0; n < 16; n++)
			palette[n] = ZXSpectrumPalette[n];

		for (int y = 0; y < 192 && y < height; y++)
		{
			uint32_t bitmapOffset = ((uint32_t)(y & 0xC0) << 5) |
			                      ((uint32_t)(y & 0x07) << 8) |
			                      ((uint32_t)(y & 0x38) << 2);
			uint32_t attrOffset = 6144 + ((uint32_t)(y >> 3) * 32);
			uint8_t* ptr = output + y * width;

			for (int x = 0; x < 32 && x * 8 < width; x++)
			{
				uint8_t bits = buffer[bitmapOffset + x];
				uint8_t attr = buffer[attrOffset + x];
				uint8_t bright = (attr & 0x40) ? 8 : 0;
				uint8_t ink = bright | (attr & 0x07);
				uint8_t paper = bright | ((attr >> 3) & 0x07);

				for (int bit = 0; bit < 8 && x * 8 + bit < width; bit++)
					*ptr++ = (bits & (0x80 >> bit)) ? ink : paper;
			}
		}
		return true;
	}
	#endif

	if (target == DDB_MACHINE_AMIGA && SCR_HasILBMHeader(buffer, size))
	{
		if (SCR_DecodeILBM(buffer, size, output, width, height, palette, isHAM))
			return true;
		DDB_SetError(DDB_ERROR_INVALID_FILE);
		return false;
	}

	// PI1 (Degas) detection is size-only, and an Amiga SCR is coincidentally the
	// same 32034 bytes, so exclude Amiga here or its screens decode as Atari ST.
	if (target != DDB_MACHINE_AMIGA && SCR_HasPI1Header(buffer, size))
	{
		SCR_DecodePlanarST(buffer + 2, output, width, height, palette);
		return true;
	}

	if (SCR_HasFalconRawHeader(buffer, size))
	{
		SCR_DecodePlanarFalcon(buffer, output, width, height, palette);
		return true;
	}

	if (target == DDB_MACHINE_ATARIST)
	{
		SCR_DecodePlanarST(buffer, output, width, height, palette);
	}
	else if (target == DDB_MACHINE_AMIGA)
	{
		// Amiga raw SCR file: 16-color palette + four contiguous planes
		if (size < 32034)
		{
			DDB_SetError(DDB_ERROR_INVALID_FILE);
			return false;
		}
		uint8_t* filePalette = buffer + 2;
		for (int n = 0; n < 16; n++) {
			uint16_t c = (filePalette[n * 2] << 8) + filePalette[n * 2 + 1];
			palette[n] = 0xFF000000UL |
				((uint32_t)(c & 0xF00) << 12) |
				((uint32_t)(c & 0x0F0) << 8) |
				((uint32_t)(c & 0x00F) << 4);
		}
		for (int y = 0 ; y < 200 && y < height; y++)
		{
			uint8_t* row = buffer + 34 + y * 40;
			uint8_t* ptr = output + y * width;
			for (int x = 0; x < 40; x++)
			{
				uint16_t bits0 = row[x+0];
				uint16_t bits1 = row[x+8000];
				uint16_t bits2 = row[x+16000];
				uint16_t bits3 = row[x+24000];
				
				for (int b = 0; b < 8; b++)
				{
					int mask = 0x80 >> b;
					int color = ((bits0 & mask) ? 0x01 : 0x00) |
								((bits1 & mask) ? 0x02 : 0x00) |
								((bits2 & mask) ? 0x04 : 0x00) |
								((bits3 & mask) ? 0x08 : 0x00);
					*ptr++ = color;
				}
			}
		}
	}
	else if (target == DDB_MACHINE_IBMPC)
	{
		if (size <= 16384)
		{
			// CGA
			for (int n = 0; n < 16; n++) {
				palette[n] = CGAPaletteCyan[n & 3];
			}

			for (int y = 0 ; y < 200 && y < height; y++)
			{
				uint8_t* row = buffer + 80*(y >> 1) + 8192*(y & 1);
				uint8_t* ptr = output + y * width;
				for (int x = 0; x < 320; x++)
				{
					uint8_t c = row[x >> 2];
					int rot = 6 - ((x & 3) << 1);
					*ptr++ = (c >> rot) & 0x03;
				}
			}
		}
		else
		{
			uint8_t* data = buffer + 1;
			if (size <= 32048)
			{
				// EGA
				for (int n = 0; n < 16; n++) {	
					palette[n] = EGAPalette[n];
				}
			}
			else
			{
				// VGA
				uint8_t* filePalette = buffer + 1;
				data = buffer + 49;
				for (int n = 0; n < 16; n++) {
					uint16_t r = filePalette[n * 3];
					uint16_t g = filePalette[n * 3 + 1];
					uint16_t b = filePalette[n * 3 + 2];
					r = (r << 2) | (r & 0x03);
					g = (g << 2) | (g & 0x03);
					b = (b << 2) | (b & 0x03);
					palette[n] = 0xFF000000UL |
						((uint32_t)r << 16) |
						((uint32_t)g << 8) |
						(uint32_t)b;
				}
			}

			for (int y = 0 ; y < 200 && y < height; y++)
			{
				uint8_t* row = data + 40*y;
				int bitPlaneStride = 8000;
				for (int x = 0; x < 320; x++)
				{
					int offset = x >> 3;
					int mask = 0x80 >> (x & 7);
					uint8_t color = 
						((row[offset                   ] & mask) != 0 ? 0x01 : 0) |
						((row[offset + bitPlaneStride  ] & mask) != 0 ? 0x02 : 0) |
						((row[offset + bitPlaneStride*2] & mask) != 0 ? 0x04 : 0) |
						((row[offset + bitPlaneStride*3] & mask) != 0 ? 0x08 : 0);
					*output++ = color;
				}
			}
		}
	}

	return true;
}
