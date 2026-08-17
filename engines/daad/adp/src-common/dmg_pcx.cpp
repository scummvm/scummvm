#if HAS_PCX
#include <dmg.h>
#include <os_bito.h>
#include <os_file.h>
#include <os_mem.h>

static bool DecodePCXRow(const uint8_t*& source, const uint8_t* end, uint8_t* output, uint16_t bytesPerLine, uint16_t width)
{
    uint16_t x = 0;
    uint16_t visible = 0;

    while (x < bytesPerLine)
    {
        if (source >= end)
        {
            DMG_SetError(DMG_ERROR_TRUNCATED_DATA_STREAM);
            return false;
        }

        uint8_t value = *source++;
        uint16_t count = 1;
        if ((value & 0xC0) == 0xC0)
        {
            count = value & 0x3F;
            if (count == 0 || source >= end)
            {
                DMG_SetError(DMG_ERROR_CORRUPTED_DATA_STREAM);
                return false;
            }
            value = *source++;
        }

        if ((uint32_t)x + count > bytesPerLine)
        {
            DMG_SetError(DMG_ERROR_DATA_STREAM_TOO_LONG);
            return false;
        }

        while (count-- > 0)
        {
            if (visible < width)
                output[visible++] = value;
            x++;
        }
    }

    return true;
}

static bool ReadPCXPalette(File* file, uint64_t fileSize, uint32_t* palette)
{
    uint8_t paletteMarker;
    uint8_t paletteData[768];

    if (!File_Seek(file, fileSize - 769) || File_Read(file, &paletteMarker, 1) != 1)
    {
        DMG_SetError(DMG_ERROR_READING_FILE);
        return false;
    }

    if (paletteMarker != 0x0C)
    {
        DMG_SetError(DMG_ERROR_INVALID_IMAGE);
        return false;
    }

    if (File_Read(file, paletteData, sizeof(paletteData)) != sizeof(paletteData))
    {
        DMG_SetError(DMG_ERROR_READING_FILE);
        return false;
    }

    if (palette != 0)
    {
        for (int n = 0; n < 256; n++)
        {
            uint8_t r = paletteData[n * 3 + 0];
            uint8_t g = paletteData[n * 3 + 1];
            uint8_t b = paletteData[n * 3 + 2];
            palette[n] = 0xFF000000UL | ((uint32_t)r << 16) | ((uint32_t)g << 8) | (uint32_t)b;
        }
    }

    return true;
}

bool DMG_ReadPCXPalette (const char* fileName, uint32_t* palette)
{
    uint8_t header[128];
    File* file = File_Open(fileName);
    if (file == NULL)
    {
        DMG_SetError(DMG_ERROR_FILE_NOT_FOUND);
        return false;
    }

    DMG_SetError(DMG_ERROR_NONE);

    uint64_t fileSize = File_GetSize(file);
    if (fileSize < 128 + 769)
    {
        DMG_SetError(DMG_ERROR_INVALID_IMAGE);
        File_Close(file);
        return false;
    }

    if (File_Read(file, header, sizeof(header)) != sizeof(header))
    {
        DMG_SetError(DMG_ERROR_READING_FILE);
        File_Close(file);
        return false;
    }

    if (header[0] != 0x0A || header[2] != 1 || header[3] != 8 || header[65] != 1)
    {
        DMG_SetError(DMG_ERROR_INVALID_IMAGE);
        File_Close(file);
        return false;
    }

    bool ok = ReadPCXPalette(file, fileSize, palette);
    File_Close(file);
    return ok;
}

bool DMG_DecompressPCX (const char* fileName, uint8_t* buffer, uint32_t* bufferSize, int* width, int* height, uint32_t* palette)
{
    uint8_t header[128];
    uint8_t* compressed = 0;
    File* file = File_Open(fileName);
    if (file == NULL)
    {
        DMG_SetError(DMG_ERROR_FILE_NOT_FOUND);
        return false;
    }

    DMG_SetError(DMG_ERROR_NONE);

    uint64_t fileSize = File_GetSize(file);
    if (fileSize < 128 + 769)
    {
        DMG_SetError(DMG_ERROR_INVALID_IMAGE);
        File_Close(file);
        return false;
    }

    if (File_Read(file, header, sizeof(header)) != sizeof(header))
    {
        DMG_SetError(DMG_ERROR_READING_FILE);
        File_Close(file);
        return false;
    }

    if (header[0] != 0x0A || header[2] != 1 || header[3] != 8 || header[65] != 1)
    {
        DMG_SetError(DMG_ERROR_INVALID_IMAGE);
        File_Close(file);
        return false;
    }

    uint16_t xmin = read16LE(header + 4);
    uint16_t ymin = read16LE(header + 6);
    uint16_t xmax = read16LE(header + 8);
    uint16_t ymax = read16LE(header + 10);
    uint16_t bytesPerLine = read16LE(header + 66);

    if (xmax < xmin || ymax < ymin)
    {
        DMG_SetError(DMG_ERROR_INVALID_IMAGE);
        File_Close(file);
        return false;
    }

    int imageWidth = (int)(xmax - xmin + 1);
    int imageHeight = (int)(ymax - ymin + 1);

    if (imageWidth <= 0 || imageHeight <= 0 || imageWidth > DMG_MAX_IMAGE_WIDTH || imageHeight > DMG_MAX_IMAGE_HEIGHT)
    {
        DMG_SetError(DMG_ERROR_IMAGE_TOO_BIG);
        File_Close(file);
        return false;
    }

    if (bytesPerLine < imageWidth)
    {
        DMG_SetError(DMG_ERROR_INVALID_IMAGE);
        File_Close(file);
        return false;
    }

    if (!ReadPCXPalette(file, fileSize, palette))
    {
        File_Close(file);
        return false;
    }

    uint32_t requiredSize = (uint32_t)imageWidth * (uint32_t)imageHeight;
    uint32_t availableSize = bufferSize ? *bufferSize : 0;

    if (width)
        *width = imageWidth;
    if (height)
        *height = imageHeight;
    if (bufferSize)
        *bufferSize = requiredSize;

    if (buffer == NULL || bufferSize == NULL || availableSize < requiredSize)
    {
        DMG_SetError(DMG_ERROR_BUFFER_TOO_SMALL);
        File_Close(file);
        return false;
    }

    uint32_t compressedSize = (uint32_t)(fileSize - 128 - 769);
    compressed = Allocate<uint8_t>("PCX data", compressedSize, false);
    if (compressed == 0)
    {
        DMG_SetError(DMG_ERROR_OUT_OF_MEMORY);
        File_Close(file);
        return false;
    }

    if (!File_Seek(file, sizeof(header)) || File_Read(file, compressed, compressedSize) != compressedSize)
    {
        DMG_SetError(DMG_ERROR_READING_FILE);
        Free(compressed);
        File_Close(file);
        return false;
    }

    const uint8_t* source = compressed;
    const uint8_t* end = compressed + compressedSize;
    for (int y = 0; y < imageHeight; y++)
    {
        if (!DecodePCXRow(source, end, buffer + y * imageWidth, bytesPerLine, (uint16_t)imageWidth))
        {
            Free(compressed);
            File_Close(file);
            return false;
        }
    }

    Free(compressed);

    File_Close(file);
    return true;
}
#endif
