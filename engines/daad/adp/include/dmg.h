#pragma once

/* ───────────────────────────────────────────────────────────────────────── */
/*  DMG - .DAT Graphic file manager for DAAD                                 */
/* ───────────────────────────────────────────────────────────────────────── */

#include <os_file.h>

#define DMG_MAX_IMAGE_WIDTH		999
#define DMG_MAX_IMAGE_HEIGHT	999
#define DMG_CACHE_BLOCKS	    128

#ifndef DEBUG_ZX0
	#define DEBUG_ZX0 0
#endif

enum DMG_Error
{
	DMG_ERROR_NONE,
	DMG_ERROR_FILE_NOT_FOUND,
	DMG_ERROR_CREATING_FILE,
	DMG_ERROR_UNKNOWN_SIGNATURE,
	DMG_ERROR_READING_FILE,
	DMG_ERROR_SEEKING_FILE,
	DMG_ERROR_WRITING_FILE,
	DMG_ERROR_INVALID_ENTRY_COUNT,
	DMG_ERROR_FILE_TOO_SMALL,
	DMG_ERROR_FILE_TOO_BIG,
	DMG_ERROR_TRUNCATED_DATA_STREAM,
	DMG_ERROR_DATA_STREAM_TOO_LONG,
	DMG_ERROR_DATA_OFFSET_OUT_OF_BOUNDS,
	DMG_ERROR_CORRUPTED_DATA_STREAM,
	DMG_ERROR_IMAGE_TOO_BIG,
	DMG_ERROR_BUFFER_TOO_SMALL,
	DMG_ERROR_ENTRY_IS_EMPTY,
	DMG_ERROR_OUT_OF_MEMORY,
	DMG_ERROR_TEMPORARY_BUFFER_MISSING,
	DMG_ERROR_INVALID_IMAGE,
};

enum DMG_ImageMode
{
	ImageMode_Packed            = 0x00,		// Packed indexed pixels, 2 colors per byte
	ImageMode_IndexedX          = 0x01,		// Indexed pixels arranged in Mode X band/plane order
	ImageMode_CGA               = 0x02,		// CGA packed pixels, 4 colors per byte
	ImageMode_RGBA32            = 0x10,		// 4 bytes per pixel, native-endian 0xAARRGGBB
	ImageMode_PlanarST          = 0x20,     // Atari ST interleaved words (P0P0P1P1P2P2P3P3)
    ImageMode_Planar            = 0x24,     // One complete bitmap per plane
	ImageMode_Planar8           = 0x25,     // 8-pixel interleaved bitplanes (P0P1P2P3)
	ImageMode_PlanarFalcon      = 0x26,     // Atari Falcon interleaved words
	ImageMode_Indexed           = 0x40,		// 1 byte per pixel, regardless of color depth
	ImageMode_AudioConverted    = 0xFE,		// Cache tag: slot holds playback-ready audio (see dmg_audio.cpp)
	ImageMode_Raw               = 0xFF,
	ImageMode_Audio             = 0xFF,
};

enum DMG_ColorPaletteMode
{
	ColorPaletteMode_Native = 0,
	ColorPaletteMode_EGA    = 1,
	ColorPaletteMode_CGA    = 2,
};


#define DMG_IS_INDEXED(mode) (mode == ImageMode_Indexed)
#define DMG_IS_RGBA32(mode)  (mode == ImageMode_RGBA32)

#ifdef _BIG_ENDIAN
#define DMG_HOST_LITTLE_ENDIAN false
#else
#define DMG_HOST_LITTLE_ENDIAN true
#endif

static inline bool DMG_IsHostByteOrder(bool littleEndian)
{
	return littleEndian == DMG_HOST_LITTLE_ENDIAN;
}

static inline bool DMG_IsClassicNativeDATByteOrder(bool littleEndian)
{
	return littleEndian == false;
}

#ifndef DMG_NATIVE_IMAGE_MODE
#if defined(_AMIGA)
#define DMG_NATIVE_IMAGE_MODE ImageMode_Planar
#elif defined(_ATARIST)
extern DMG_ImageMode nativeImageMode;
#define DMG_NATIVE_IMAGE_MODE nativeImageMode
#elif defined(_DOS)
#define DMG_NATIVE_IMAGE_MODE ImageMode_IndexedX
#else
#define DMG_NATIVE_IMAGE_MODE ImageMode_Indexed
#endif
#endif

#ifndef DMG_NATIVE_COLOR_PALETTE_MODE
#define DMG_NATIVE_COLOR_PALETTE_MODE ColorPaletteMode_Native
#endif

#ifndef DMG_SUPPORT_NATIVE_DAT_ONLY
#if defined(_AMIGA) || defined(_ATARIST)
#define DMG_SUPPORT_NATIVE_DAT_ONLY 1
#else
#define DMG_SUPPORT_NATIVE_DAT_ONLY 0
#endif
#endif

#ifndef DMG_SUPPORT_CGA_SOURCES
#if defined(_AMIGA) || defined(_ATARIST)
#define DMG_SUPPORT_CGA_SOURCES 0
#else
#define DMG_SUPPORT_CGA_SOURCES 1
#endif
#endif

#ifndef DMG_SUPPORT_EGA_SOURCES
#if defined(_AMIGA) || defined(_ATARIST)
#define DMG_SUPPORT_EGA_SOURCES 0
#else
#define DMG_SUPPORT_EGA_SOURCES 1
#endif
#endif

#if DMG_SUPPORT_CGA_SOURCES || DMG_SUPPORT_EGA_SOURCES
#define DMG_SUPPORT_CLASSIC_CONVERSION_PALETTES 1
#else
#define DMG_SUPPORT_CLASSIC_CONVERSION_PALETTES 0
#endif

#ifndef DMG_SUPPORT_DAT5_PLANAR_SOURCES
#if defined(_DOS)
#define DMG_SUPPORT_DAT5_PLANAR_SOURCES 0
#else
#define DMG_SUPPORT_DAT5_PLANAR_SOURCES 1
#endif
#endif

#ifndef DMG_SUPPORT_DAT5_ST_SOURCES
#if defined(_DOS)
#define DMG_SUPPORT_DAT5_ST_SOURCES 0
#else
#define DMG_SUPPORT_DAT5_ST_SOURCES 1
#endif
#endif

#ifndef DMG_SUPPORT_DAT5_INDEXEDX_SOURCES
#if defined(_DOS)
#define DMG_SUPPORT_DAT5_INDEXEDX_SOURCES 0
#else
#define DMG_SUPPORT_DAT5_INDEXEDX_SOURCES 1
#endif
#endif

#ifndef DMG_SUPPORT_CROSS_ENDIAN_SOURCES
#if defined(_AMIGA) || defined(_ATARIST)
#define DMG_SUPPORT_CROSS_ENDIAN_SOURCES 0
#else
#define DMG_SUPPORT_CROSS_ENDIAN_SOURCES 1
#endif
#endif

enum DMG_EntryType
{
	DMGEntry_Empty,
	DMGEntry_Image,
	DMGEntry_Audio,
};

typedef enum
{
	CGA_Blue,
	CGA_Red
}
DMG_CGAMode;

typedef enum
{
	DMG_Version1_CGA,
	DMG_Version1_EGA,
	DMG_Version1_PCW,
	DMG_Version1,
	DMG_Version2,
    DMG_Version5
}
DMG_Version;

typedef enum
{
    DMG_DAT5_COLORMODE_NONE = 0,
    DMG_DAT5_COLORMODE_CGA       = 1,
    DMG_DAT5_COLORMODE_EGA       = 2,
    DMG_DAT5_COLORMODE_PLANAR4   = 3,
    DMG_DAT5_COLORMODE_PLANAR5   = 4,
    DMG_DAT5_COLORMODE_PLANAR8   = 5,
    DMG_DAT5_COLORMODE_PLANAR4ST = 6,
    DMG_DAT5_COLORMODE_PLANAR8ST = 7,
    DMG_DAT5_COLORMODE_EHB6      = 8,
    DMG_DAT5_COLORMODE_HAM6      = 9,
    DMG_DAT5_COLORMODE_INDEXEDX  = 10,
    DMG_DAT5_COLORMODE_INDEXED   = 11,

    DMG_DAT5_COLORMODE_I16  = DMG_DAT5_COLORMODE_PLANAR4,
    DMG_DAT5_COLORMODE_I32  = DMG_DAT5_COLORMODE_PLANAR5,
    DMG_DAT5_COLORMODE_I256 = DMG_DAT5_COLORMODE_PLANAR8,
    DMG_DAT5_COLORMODE_VESA = DMG_DAT5_COLORMODE_INDEXED,
}
DMG_DAT5ColorMode;

typedef enum
{
	DMG_DAT5_FLAG_2X = 0x01,
}
DMG_DAT5Flags;

static inline bool DMG_DAT5ModeUsesPalette(uint8_t mode)
{
	switch ((DMG_DAT5ColorMode)mode)
	{
		case DMG_DAT5_COLORMODE_PLANAR4:
		case DMG_DAT5_COLORMODE_PLANAR5:
		case DMG_DAT5_COLORMODE_PLANAR8:
		case DMG_DAT5_COLORMODE_PLANAR4ST:
		case DMG_DAT5_COLORMODE_PLANAR8ST:
		case DMG_DAT5_COLORMODE_EHB6:
		case DMG_DAT5_COLORMODE_HAM6:
		case DMG_DAT5_COLORMODE_INDEXEDX:
		case DMG_DAT5_COLORMODE_INDEXED:
			return true;
		default:
			return false;
	}
}

static inline bool DMG_DAT5ModeIsIndexed(uint8_t mode)
{
	return (DMG_DAT5ColorMode)mode == DMG_DAT5_COLORMODE_INDEXED;
}

static inline bool DMG_DAT5ModeIsIndexedX(uint8_t mode)
{
	return (DMG_DAT5ColorMode)mode == DMG_DAT5_COLORMODE_INDEXEDX;
}

static inline bool DMG_DAT5ModeIsDOSSupported(uint8_t mode)
{
	switch ((DMG_DAT5ColorMode)mode)
	{
		case DMG_DAT5_COLORMODE_CGA:
		case DMG_DAT5_COLORMODE_EGA:
		case DMG_DAT5_COLORMODE_INDEXED:
			return true;
		default:
			return false;
	}
}

static inline bool DMG_DAT5ModeIsPlaneMajor(uint8_t mode)
{
	switch ((DMG_DAT5ColorMode)mode)
	{
		case DMG_DAT5_COLORMODE_PLANAR4:
		case DMG_DAT5_COLORMODE_PLANAR5:
		case DMG_DAT5_COLORMODE_PLANAR8:
		case DMG_DAT5_COLORMODE_EHB6:
		case DMG_DAT5_COLORMODE_HAM6:
			return true;
		default:
			return false;
	}
}

static inline bool DMG_DAT5ModeIsSTInterleaved(uint8_t mode)
{
	switch ((DMG_DAT5ColorMode)mode)
	{
		case DMG_DAT5_COLORMODE_PLANAR4ST:
		case DMG_DAT5_COLORMODE_PLANAR8ST:
			return true;
		default:
			return false;
	}
}

static inline bool DMG_DAT5ModeIsAmigaSpecial(uint8_t mode)
{
	switch ((DMG_DAT5ColorMode)mode)
	{
		case DMG_DAT5_COLORMODE_EHB6:
		case DMG_DAT5_COLORMODE_HAM6:
			return true;
		default:
			return false;
	}
}

static inline bool DMG_DAT5ModeIsHAM(uint8_t mode)
{
	return (DMG_DAT5ColorMode)mode == DMG_DAT5_COLORMODE_HAM6;
}

static inline uint8_t DMG_DAT5ModePlaneCount(uint8_t mode)
{
	switch ((DMG_DAT5ColorMode)mode)
	{
		case DMG_DAT5_COLORMODE_PLANAR4:
		case DMG_DAT5_COLORMODE_PLANAR4ST:
			return 4;
		case DMG_DAT5_COLORMODE_PLANAR5:
			return 5;
		case DMG_DAT5_COLORMODE_EHB6:
		case DMG_DAT5_COLORMODE_HAM6:
			return 6;
		case DMG_DAT5_COLORMODE_PLANAR8:
		case DMG_DAT5_COLORMODE_PLANAR8ST:
		case DMG_DAT5_COLORMODE_INDEXEDX:
		case DMG_DAT5_COLORMODE_INDEXED:
			return 8;
		default:
			return 0;
	}
}

static inline uint32_t DMG_DAT5StoredImageSize(uint8_t mode, uint16_t width, uint16_t height)
{
	switch ((DMG_DAT5ColorMode)mode)
	{
		case DMG_DAT5_COLORMODE_CGA:
			return ((uint32_t)width * height + 3) / 4;
		case DMG_DAT5_COLORMODE_EGA:
			return ((uint32_t)width * height + 1) / 2;
		case DMG_DAT5_COLORMODE_PLANAR4:
		case DMG_DAT5_COLORMODE_PLANAR5:
		case DMG_DAT5_COLORMODE_PLANAR8:
		case DMG_DAT5_COLORMODE_PLANAR4ST:
		case DMG_DAT5_COLORMODE_PLANAR8ST:
		case DMG_DAT5_COLORMODE_EHB6:
		case DMG_DAT5_COLORMODE_HAM6:
			return (((uint32_t)width + 15) >> 4) * height * DMG_DAT5ModePlaneCount(mode) * 2u;
		case DMG_DAT5_COLORMODE_INDEXEDX:
			return (((uint32_t)width + 3) & ~3u) * height;
		case DMG_DAT5_COLORMODE_INDEXED:
			return (uint32_t)width * height;
		default:
			return 0;
	}
}

typedef enum
{
	DMG_5KHZ 			= 0,
	DMG_7KHZ			= 1,
	DMG_9_5KHZ			= 2,
	DMG_15KHZ			= 3,
	DMG_20KHZ			= 4,
	DMG_30KHZ			= 5,
    DMG_44_1KHZ         = 6,        // Version 5+
    DMG_48KHZ           = 7,        // Version 5+
}
DMG_KHZ;

typedef enum
{
    DMG_FLAG_COMPRESSED    = 0x0001,
    DMG_FLAG_BUFFERED      = 0x0002,
    DMG_FLAG_FIXED         = 0x0004,
    DMG_FLAG_ZX0           = 0x0008,
    DMG_FLAG_PROCESSED     = 0x0010,
    DMG_FLAG_AMIPALHACK    = 0x0020,
    DMG_FLAG_CGAMODE       = 0x0040,
    DMG_FLAG_PALETTE_DECODED = 0x0080,
}
DMG_FLAGS;

struct DMG_CVPal
{
	uint8_t			cga[16];
	uint8_t			ega[16];
};

typedef enum
{
	DMG_EDITDATA_OWNS_STORED_DATA = 0x01,
	DMG_EDITDATA_OWNS_PALETTE = 0x02,
	DMG_EDITDATA_OWNS_CONVERSION_PALETTE = 0x04,
	DMG_EDITDATA_REUSES_STORED_DATA = 0x08,
}
DMG_EditableEntryFlags;

struct DMG_EditableEntryData
{
	uint8_t*		storedData;
	uint32_t		storedDataSize;
	uint8_t			flags;
	uint8_t			reusedFrom;
};

struct DMG_Entry
{
	DMG_EntryType	type;

	uint32_t*		RGB32Palette;
	DMG_CVPal*      conversionPalette;

	uint8_t		    flags;
    uint8_t         bitDepth;       // Number of planes, only used if ImageMode_Planar
	int16_t			x;
	int16_t			y;
	uint16_t		width;
	uint16_t		height;
	uint8_t         firstColor;
	uint8_t         lastColor;

	uint32_t		fileOffset;
	uint32_t		length;
    uint32_t        paletteOffset;
    uint32_t        paletteSize;
    uint16_t        paletteColors;

	uint8_t*        cachedFileData;
	uint32_t        cachedFileSize;
};

struct DMG_Slot
{
	uint8_t         firstEntry;
	uint8_t         usageCount;
	uint32_t        startOffset;
	uint32_t        length;
};

struct DMG_Cache
{
	DMG_Cache*  next;
	uint32_t    size;
	uint32_t    time;
	uint8_t     index;
	uint8_t     imageMode;
	bool        buffer;
	bool        populated;
};

struct DMG
{
	bool		littleEndian;
	DMG_Version version;
	uint8_t		screenMode;
	bool        dirty;
    uint8_t     firstEntry;
    uint8_t     lastEntry;
    uint8_t     colorMode;
	uint8_t     dat5Flags;
    uint16_t    targetWidth;
    uint16_t    targetHeight;

	DMG_Entry*  entries[256];
    DMG_Entry*  entryBlock;

	DMG_Cache*  cache;
	DMG_Cache*  cacheTail;
	uint32_t    cacheSize;
	uint32_t    cacheFree;
	uint8_t     cacheBitmap[32];
	uint32_t*   paletteBlock;
	DMG_CVPal* conversionPaletteBlock;
	DMG_EditableEntryData* editableEntries;
	uint8_t     savePriority[256];
	uint16_t    savePriorityCount;

	uint8_t*    fileCacheData;
	uint32_t    fileCacheSize;
    uint8_t*    zx0Scratch;
    uint32_t    zx0ScratchSize;
    bool        zx0ScratchOwned;

	#if DEBUG_ZX0
	uint32_t    zx0ProfileCount;
	uint32_t    zx0ProfileInputBytes;
	uint32_t    zx0ProfileOutputBytes;
	uint32_t    zx0ProfileTotalMs;
	uint32_t    zx0ProfileMaxMs;
	uint8_t     zx0ProfileMaxIndex;
	#endif

	File*		file;
	uint32_t    fileSize;
};

extern DMG* dmg;

DMG*		DMG_Open			   (const char* filename, bool readOnly);
DMG*		DMG_OpenFromFile	   (File* file);
DMG*        DMG_Create             (const char* filename);
DMG*        DMG_CreateFormat       (const char* filename, DMG_Version version);
DMG*        DMG_CreateDAT5         (const char* filename, DMG_DAT5ColorMode colorMode, uint16_t width, uint16_t height, uint8_t firstEntry = 0, uint8_t lastEntry = 255, uint8_t dat5Flags = 0);
DMG_Entry*	DMG_GetEntry		   (DMG* dmg, uint8_t index);
bool        DMG_UpdateEntry        (DMG* dmg, uint8_t index);
bool        DMG_UpdateFileHeader   (DMG* dmg);
bool        DMG_Save               (DMG* dmg);
uint8_t*    DMG_GetEntryData	   (DMG* dmg, uint8_t index, DMG_ImageMode mode);
uint8_t*    DMG_GetEntryDataNative (DMG* dmg, uint8_t index);
uint8_t*    DMG_GetEntryDataChunky (DMG* dmg, uint8_t index);

// Playback sink description for DMG_GetEntryAudioConverted. Describes the
// audio format a given platform's mixer can consume directly.
struct DMG_AudioTarget
{
	uint32_t maxRate;       // Highest sample rate the sink can play; 0 = no cap
	uint8_t  bitDepth;      // Desired output depth: 8 to downconvert, 16 to accept as-is
	bool     signedOutput;  // true = signed 8-bit (e.g. Amiga Paula), false = unsigned (SB/ST)
};

// Returns a pointer to sample data converted to the sink's format, doing the
// conversion in place inside the entry's reusable cache slot (no per-play
// allocation). Rate is capped to sink->maxRate by decimation and 16-bit
// samples are folded to 8-bit as needed. *outLength/*outRate receive the
// converted byte count and sample rate. Returns 0 on error.
uint32_t    DMG_AudioRateHz        (int16_t rateCode);
uint8_t*    DMG_GetEntryAudioConverted (DMG* dmg, uint8_t index,
                                        const DMG_AudioTarget* sink,
                                        uint32_t* outLength, uint32_t* outRate);

#if defined(_AMIGA) || defined(_ATARIST) || defined(_DOS)
uint8_t*    DMG_GetEntryDataPlanar (DMG* dmg, uint8_t index);
#endif

uint32_t*   DMG_GetEntryPalette    (DMG* dmg, uint8_t index);
uint32_t*   DMG_GetEntryStoredPalette(DMG* dmg, uint8_t index);
uint16_t    DMG_GetEntryPaletteSize(DMG* dmg, uint8_t index);
uint8_t     DMG_GetEntryFirstColor (DMG* dmg, uint8_t index);
bool        DMG_RemoveEntry	 	   (DMG* dmg, uint8_t index);
bool        DMG_SetEntryPalette    (DMG* dmg, uint8_t index, uint32_t* palette);
bool        DMG_SetEntryPaletteEx  (DMG* dmg, uint8_t index, uint32_t* palette, uint16_t paletteSize);
bool        DMG_SetEntryPaletteRange(DMG* dmg, uint8_t index, uint32_t* palette, uint16_t paletteSize, uint8_t firstColor, uint8_t lastColor);
bool        DMG_SetEntryConversionPalette(DMG* dmg, uint8_t index, const DMG_CVPal* palette);
bool        DMG_EnsureEditableEntryData(DMG* dmg);
bool		DMG_SetImageData       (DMG* dmg, uint8_t index, uint8_t* buffer, uint16_t width, uint16_t height, uint16_t size, bool compressed);
bool        DMG_SetImageDataEx     (DMG* dmg, uint8_t index, uint8_t* buffer, uint16_t width, uint16_t height, uint32_t size, bool compressed, uint8_t bitDepth);
bool        DMG_SetAudioData       (DMG* dmg, uint8_t index, uint8_t* buffer, uint32_t size, DMG_KHZ freq, uint8_t bitDepth);
bool        DMG_ReuseEntryData     (DMG* dmg, uint8_t index, uint8_t originalIndex);
void        DMG_ClearSavePriority  (DMG* dmg);
bool        DMG_AddSavePriority    (DMG* dmg, uint8_t index);
int         DMG_GetEntryCount      (DMG* dmg);
uint8_t     DMG_FindFreeEntry      (DMG* dmg);
void		DMG_Close			   (DMG* dmg);

bool        DMG_SetupImageCache    (DMG* dmg, uint32_t bytes);
DMG_Cache*  DMG_GetImageCache      (DMG* dmg, uint8_t index, DMG_Entry* entry, uint32_t size);
void        DMG_FreeImageCache     (DMG* dmg);
uint32_t    DMG_GetMinimumImageBufferSize(DMG* dmg, DMG_ImageMode mode);
uint32_t    DMG_GetMinimumImageCacheAllocationSize(DMG* dmg, DMG_ImageMode mode);

void        DMG_SetupFileCache     (DMG* dmg, uint32_t blockSize = 0, void(*progressFunction)(uint16_t) = 0);
uint32_t    DMG_ReadFromFile       (DMG* dmg, uint32_t offset, void* buffer, uint32_t size);
void*       DMG_GetFromFileCache   (DMG* dmg, uint32_t offset, uint32_t size);
void        DMG_FreeFileImageCache (DMG* dmg);
void        DMG_SetZX0ScratchBuffer(DMG* dmg, uint8_t* buffer, uint32_t size, bool owned = false);
uint8_t*    DMG_GetScratchBuffer   (DMG* dmg, uint32_t size);
uint32_t    DMG_GetScratchBufferSize(DMG* dmg);

void        DMG_Warning            (const char* format, ...);
void        DMG_SetError           (DMG_Error error);
DMG_Error   DMG_GetError           ();
const char* DMG_GetErrorString     ();
void        DMG_SetWarningHandler  (void (*handler)(const char* message));

const char* DMG_DescribeFreq       (DMG_KHZ freq);

uint32_t    DMG_GetTemporaryBufferSize ();
uint8_t*    DMG_GetTemporaryBufferBase ();
uint8_t*    DMG_GetTemporaryBuffer     (DMG_ImageMode mode);
bool        DMG_ReserveTemporaryBuffer (uint32_t size);
bool        DMG_IsTemporaryBufferPointer(const void* ptr);
void        DMG_FreeTemporaryBuffer    ();

bool        DMG_UncCGAToPacked         (const uint8_t* input, uint16_t width, uint16_t height, uint8_t* output, int pixels, uint8_t* scratch, uint32_t scratchSize);
bool        DMG_UncEGAToPacked         (const uint8_t* input, uint16_t width, uint16_t height, uint8_t* output, int pixels, uint8_t* scratch, uint32_t scratchSize);
bool        DMG_CopyImageData          (uint8_t* ptr, uint16_t length, uint8_t* output, int pixels);
uint8_t*    DMG_ConvertPlanar8ToPlanarST(uint8_t* data, uint8_t* buffer, int length, uint32_t width);
uint8_t*    DMG_ConvertPlanar8ToPlanarFalcon(uint8_t* data, uint8_t* buffer, int length, uint32_t width);
bool 		DMG_ConvertPlanar8ToPlanar (const DMG_Entry* entry, const uint8_t* data, uint32_t packedSize, uint8_t* output, uint32_t outputSize);
bool        DMG_UnpackBitplaneBytes    (const uint8_t* data, uint16_t width, uint16_t height, uint8_t bitsPerPixel, uint8_t* output);
bool        DMG_UnpackBitplaneWords    (const uint8_t* data, uint16_t width, uint16_t height, uint8_t bitsPerPixel, uint8_t* output);
bool        DMG_UnpackChunkyPixels     (const uint8_t* input, uint16_t width, uint16_t height, uint8_t bitsPerPixel, uint8_t* output);

bool        DMG_DecompressCGA          (const uint8_t* data, uint16_t dataLength, uint8_t* buffer, int width, int height);
bool        DMG_DecompressEGA          (const uint8_t* data, uint16_t dataLength, uint8_t* buffer, int width, int height);
bool        DMG_ExpandPCWStoredLayoutToPacked(const uint8_t* input, uint16_t width, uint16_t height, uint8_t* output, uint32_t outputSize);
bool        DMG_DecodePCWCompressedToPacked(const uint8_t* input, uint32_t inputSize, const DMG_Entry* entry, uint8_t* output, uint32_t outputSize);
bool        DMG_LooksLikePCWDataFile     (const uint8_t* header, size_t fileSize, const char* extension);

#if HAS_PCX
bool        DMG_ReadPCXPalette         (const char* fileName, uint32_t* palette);
bool        DMG_DecompressPCX          (const char* fileName, uint8_t* buffer, uint32_t* bufferSize, int* width, int* height, uint32_t* palette);
#endif

bool        DMG_DecompressOldRLE       (const uint8_t* data, uint16_t rleMask, uint16_t dataLength, uint8_t* buffer, int pixels, bool littleEndian);
bool        DMG_DecompressOldRLEToPlanarST(const uint8_t* data, uint16_t rleMask, uint16_t dataLength, uint8_t* buffer, int pixels, bool littleEndian);
bool        DMG_DecompressNewRLE       (const uint8_t* data, uint16_t rleMask, uint16_t dataLength, uint8_t* buffer, int pixels, bool littleEndian);
bool        DMG_DecompressNewRLEToPlanarST(const uint8_t* data, uint16_t rleMask, uint16_t dataLength, uint8_t* buffer, uint32_t width, uint32_t height, bool littleEndian);
bool        DMG_DecompressZX0          (const uint8_t* data, uint32_t dataLength, uint8_t* buffer, uint32_t outputSize);
uint8_t*    DMG_CompressZX0            (const uint8_t* data, uint32_t dataLength, uint32_t* outputSize);
bool        DMG_Planar8ToPacked        (const uint8_t* data, uint16_t length, uint8_t* output, int pixels, uint32_t width);
void        DMG_ConvertPackedToPlanarST(uint8_t *buffer, uint32_t bufferSize, uint32_t width);
void        DMG_ConvertPackedToPlanarFalcon(uint8_t *buffer, uint32_t bufferSize, uint32_t width);
bool        DMG_ConvertPlanarSTToPlanar(const DMG_Entry* entry, uint8_t* data, uint32_t dataSize, uint8_t* scratch, uint32_t scratchSize);

uint32_t    DMG_CalculateRequiredSize  (DMG_Entry* entry, DMG_ImageMode mode);

static inline bool DMG_IsPaletteDecoded(const DMG_Entry* entry)
{
	return entry != 0 && (entry->flags & DMG_FLAG_PALETTE_DECODED) != 0;
}

static inline void DMG_SetPaletteDecoded(DMG_Entry* entry, bool decoded)
{
	if (decoded)
		entry->flags |= DMG_FLAG_PALETTE_DECODED;
	else
		entry->flags &= ~DMG_FLAG_PALETTE_DECODED;
}

static inline DMG_EditableEntryData* DMG_GetEditableEntryData(DMG* dmg, uint8_t index)
{
	return dmg != 0 && dmg->editableEntries != 0 ? dmg->editableEntries + index : 0;
}

static inline const DMG_EditableEntryData* DMG_GetEditableEntryData(const DMG* dmg, uint8_t index)
{
	return dmg != 0 && dmg->editableEntries != 0 ? dmg->editableEntries + index : 0;
}

static inline uint8_t* DMG_GetEntryStoredData(DMG* dmg, uint8_t index)
{
	DMG_EditableEntryData* edit = DMG_GetEditableEntryData(dmg, index);
	for (int depth = 0; edit != 0 && depth < 256; depth++)
	{
		if ((edit->flags & DMG_EDITDATA_REUSES_STORED_DATA) == 0)
			return edit->storedData;
		edit = DMG_GetEditableEntryData(dmg, edit->reusedFrom);
	}
	return 0;
}

static inline const uint8_t* DMG_GetEntryStoredData(const DMG* dmg, uint8_t index)
{
	const DMG_EditableEntryData* edit = DMG_GetEditableEntryData(dmg, index);
	for (int depth = 0; edit != 0 && depth < 256; depth++)
	{
		if ((edit->flags & DMG_EDITDATA_REUSES_STORED_DATA) == 0)
			return edit->storedData;
		edit = DMG_GetEditableEntryData(dmg, edit->reusedFrom);
	}
	return 0;
}

static inline uint32_t DMG_GetEntryStoredDataSize(const DMG* dmg, uint8_t index)
{
	const DMG_EditableEntryData* edit = DMG_GetEditableEntryData(dmg, index);
	for (int depth = 0; edit != 0 && depth < 256; depth++)
	{
		if ((edit->flags & DMG_EDITDATA_REUSES_STORED_DATA) == 0)
			return edit->storedDataSize;
		edit = DMG_GetEditableEntryData(dmg, edit->reusedFrom);
	}
	return 0;
}

static inline bool DMG_EditableEntryOwnsStoredData(const DMG* dmg, uint8_t index)
{
	const DMG_EditableEntryData* edit = DMG_GetEditableEntryData(dmg, index);
	return edit != 0 && (edit->flags & DMG_EDITDATA_OWNS_STORED_DATA) != 0;
}

static inline bool DMG_EditableEntryReusesStoredData(const DMG* dmg, uint8_t index)
{
	const DMG_EditableEntryData* edit = DMG_GetEditableEntryData(dmg, index);
	return edit != 0 && (edit->flags & DMG_EDITDATA_REUSES_STORED_DATA) != 0;
}

static inline uint8_t DMG_GetEntryStoredDataSource(const DMG* dmg, uint8_t index)
{
	const DMG_EditableEntryData* edit = DMG_GetEditableEntryData(dmg, index);
	return edit != 0 ? edit->reusedFrom : 0xFF;
}

static inline bool DMG_EditableEntryOwnsPalette(const DMG* dmg, uint8_t index)
{
	const DMG_EditableEntryData* edit = DMG_GetEditableEntryData(dmg, index);
	return edit != 0 && (edit->flags & DMG_EDITDATA_OWNS_PALETTE) != 0;
}

static inline bool DMG_EditableEntryOwnsConversionPalette(const DMG* dmg, uint8_t index)
{
	const DMG_EditableEntryData* edit = DMG_GetEditableEntryData(dmg, index);
	return edit != 0 && (edit->flags & DMG_EDITDATA_OWNS_CONVERSION_PALETTE) != 0;
}

static inline uint8_t* DMG_GetEntryCGAPalette(DMG_Entry* entry)
{
	return entry != 0 && entry->conversionPalette != 0 ? entry->conversionPalette->cga : 0;
}

static inline const uint8_t* DMG_GetEntryCGAPalette(const DMG_Entry* entry)
{
	return entry != 0 && entry->conversionPalette != 0 ? entry->conversionPalette->cga : 0;
}

static inline uint8_t* DMG_GetEntryEGAPalette(DMG_Entry* entry)
{
	return entry != 0 && entry->conversionPalette != 0 ? entry->conversionPalette->ega : 0;
}

static inline const uint8_t* DMG_GetEntryEGAPalette(const DMG_Entry* entry)
{
	return entry != 0 && entry->conversionPalette != 0 ? entry->conversionPalette->ega : 0;
}

static inline DMG_CGAMode DMG_GetCGAMode(DMG_Entry* entry)
{
    return (entry->flags & DMG_FLAG_CGAMODE) ? CGA_Red : CGA_Blue;
}

static inline void DMG_SetCGAMode(DMG_Entry* entry, DMG_CGAMode mode)
{
    if (mode == CGA_Red)
        entry->flags |= DMG_FLAG_CGAMODE;
    else
        entry->flags &= ~DMG_FLAG_CGAMODE;
}
