/* ScummVM - Graphic Adventure Engine
 *
 * ScummVM is the legal property of its developers, whose names
 * are too numerous to list here. Please refer to the COPYRIGHT
 * file distributed with this source distribution.
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 *
 */

/*
 * ScummVM backend for the ADP interpreter, replacing src-sdl/video.cpp.
 *
 * ADP composes everything - pictures, text, the input line, the Spectrum
 * attribute grid - into an 8 bit paletted buffer of the emulated machine's
 * resolution, and the backend only has to present that buffer, feed it keys
 * and play samples. The drawing primitives below (VID_Clear, VID_Scroll,
 * VID_DrawCharacter, VID_DisplayPicture, ...) are carried over from the SDL
 * backend unchanged, because they encode the exact behaviour the games
 * expect.
 * The SDL specific parts (window, events, timer, audio callback) are
 * what had to be rewritten.
 *
 * Presentation runs in one of two modes depending on what the ScummVM
 * backend offers: 
 * An indexed mode where the buffer is handed to OSystem as is
 * or a True Color mode where it is expanded through the palette.
 * The latter is preferred because Amiga HAM6 pictures cannot be represented by a
 * 256 entry palette at all.
 */

#include "audio/audiostream.h"
#include "audio/decoders/raw.h"
#include "audio/mixer.h"
#include "common/events.h"
#include "common/keyboard.h"
#include "common/system.h"
#include "common/textconsole.h"
#include "engines/engine.h"
#include "engines/util.h"
#include "graphics/paletteman.h"
#include "graphics/pixelformat.h"

#include "daad/daad.h"

#include "daad/adp_prefix.h"
#include <ddb.h>
#include <ddb_data.h>
#include <ddb_pal.h>
#include <ddb_scr.h>
#include <ddb_vid.h>
#include <ddb_xmsg.h>
#include <vid_screen.h>
#include <dmg.h>
#include <dmg_font.h>
#include <vid_font.h>
#include <os_char.h>
#include <os_file.h>
#include <os_mem.h>

#include <string.h>

DDB_Machine    screenMachine = DDB_MACHINE_IBMPC;
DDB_ScreenMode screenMode = ScreenMode_VGA16;

uint8_t   *graphicsBuffer;
uint8_t   *textBuffer;

uint8_t   *frontBuffer = NULL;
uint8_t   *backBuffer = NULL;
static VID_ScreenAdapter screenAdapter;
uint8_t   *pictureData;
static uint8_t charset16[256 * 32];
static bool charset16Available = false;
#if HAS_PCX
uint8_t   *pcxPictureData = NULL;
#if HAS_SPECTRUM
uint8_t   *zxsPictureBitmap = NULL;
uint8_t   *zxsPictureAttributes = NULL;
bool       zxsPictureMirror = false;
#endif
#endif
DMG_Entry *bufferedEntry = NULL;
bool       bufferedHAM6 = false;
uint8_t    bufferedIndex;
bool       quit;
bool       exitGame = false;
bool       textInput;
bool       charsetInitialized = false;
bool       videoInitialized = false;
uint32_t   palette[256];
uint8_t    defaultCharWidth = 6;
#if HAS_PCX
uint32_t   pcxPalette[256];

uint32_t   pcxPictureSize = 0;
int        pcxPictureWidth = 0;
int        pcxPictureHeight = 0;
#if HAS_SPECTRUM
int        zxsPictureWidth = 0;
int        zxsPictureHeight = 0;
#endif
#endif

int        xCoordMultiplier = 1;
int        yCoordMultiplier = 1;
static bool screen2XMode = false;
static DDB_Version screenVersion = DDB_VERSION_1;

// Specific for Spectrum
uint8_t   *bitmap = NULL;
uint8_t   *attributes = NULL;
uint8_t    stride = 32;

// ScummVM has no native file dialog for the interpreter to call, so the
// "Enter file name" text prompt is used for SAVE/LOAD instead.
bool       supportsOpenFileDialog = false;

void (*mainLoopCallback)(int elapsed);

// Presentation state
static Graphics::PixelFormat presentFormat;
static uint8_t *presentBuffer = NULL;
static uint32_t presentBufferSize = 0;

// Ring buffer
static uint16_t inputBuffer[256];
static int inputBufferHead = 0;
static int inputBufferTail = 0;
static byte inputModifiers = 0;

// Audio
static Audio::SoundHandle audioHandle;
static const void *audioSource = NULL;
static uint32_t audioSourceSize = 0;
uint8_t   *audioData = NULL;
uint32_t   audioDataSize = 0;

static DDB_Interpreter *mainLoopInterpreter = NULL;

void VID_SetDisplayPlanesHint(uint8_t planes) {
	(void)planes;
}

void VID_SetWindowTitle(const char *title) {
	// The launcher owns the window title
	(void)title;
}

void VID_SetWindowIcon(const char *fileName) {
	(void)fileName;
}

static void VID_BlitScreenIndexedImage(const uint8_t *pixels, int srcW, int x, int y, int w, int h) {
	if (pixels == 0 || graphicsBuffer == 0 || srcW <= 0)
		return;

	if (x < 0) {
		pixels -= x;
		w += x;
		x = 0;
	}
	if (y < 0) {
		pixels -= y * srcW;
		h += y;
		y = 0;
	}
	if (x >= screenWidth || y >= screenHeight || w <= 0 || h <= 0)
		return;
	if (x + w > screenWidth)
		w = screenWidth - x;
	if (y + h > screenHeight)
		h = screenHeight - y;
	if (w <= 0 || h <= 0)
		return;

	for (int row = 0; row < h; row++)
		memcpy(graphicsBuffer + (y + row) * screenWidth + x, pixels + row * srcW, w);
}

static void VID_BlitScreenNativeImage(const uint8_t *pixels, int srcW, int srcH, int x, int y, int w, int h) {
	if (srcH > 0 && h > srcH)
		h = srcH;
	VID_BlitScreenIndexedImage(pixels, srcW, x, y, w, h);
}

static void VID_RegisterScreenAdapter() {
	screenAdapter.info.width = screenWidth;
	screenAdapter.info.height = screenHeight;
	screenAdapter.info.cellWidth = columnWidth;
	screenAdapter.info.cellHeight = lineHeight;
	screenAdapter.info.colorDepth = 8;
	screenAdapter.info.paletteSize = 256;
	screenAdapter.info.nativeImageMode = ImageMode_Indexed;
	screenAdapter.info.alignmentPixels = 1;
	screenAdapter.ops.clear = VID_Clear;
	screenAdapter.ops.scroll = VID_Scroll;
	screenAdapter.ops.drawTextSpan = VID_DrawTextSpan;
	screenAdapter.ops.blitNativeImage = VID_BlitScreenNativeImage;
	screenAdapter.ops.blitIndexedImage = VID_BlitScreenIndexedImage;
	screenAdapter.ops.clearBuffer = VID_ClearBuffer;
	screenAdapter.ops.saveScreen = VID_SaveScreen;
	screenAdapter.ops.restoreScreen = VID_RestoreScreen;
	screenAdapter.ops.setTarget = VID_SetOpBuffer;
	screenAdapter.ops.swapScreen = VID_SwapScreen;
	VID_ScreenRegisterAdapter(&screenAdapter);
}

static bool PaletteMatches(const uint32_t *candidate, int paletteCount, int firstColor, bool clearOutside) {
	if (candidate == NULL)
		return false;

	for (int n = 0; n < paletteCount && firstColor + n < 256; n++) {
		if (palette[firstColor + n] != candidate[n])
			return false;
	}

	if (clearOutside && firstColor == 0) {
		for (int n = paletteCount; n < 256; n++) {
			if (palette[n] != 0xFF000000)
				return false;
		}
	}

	return true;
}

// Set while an ILBM HAM loading screen occupies the graphics buffer: the
// buffer then holds HAM commands even though no game database is open yet
// (the loader phase; VID_LoadDataFile clears it when a game takes over).
static bool scrHAM6 = false;

static bool IsHAM6Display() {
	if (scrHAM6)
		return true;
	return dmg != NULL && dmg->version == DMG_Version5 && dmg->colorMode == DMG_DAT5_COLORMODE_HAM6;
}

static uint32_t DecodeHAM6Command(uint8_t command, uint32_t held) {
	command &= 0x3F;
	uint8_t data = command & 0x0F;
	switch (command >> 4) {
	case 0:
		return 0xFF000000u | (palette[data] & 0x00FFFFFFu);
	case 1:
		return (held & 0xFFFFFF00u) | (uint32_t)(data * 17);
	case 2:
		return (held & 0xFF00FFFFu) | ((uint32_t)(data * 17) << 16);
	default:
		return (held & 0xFFFF00FFu) | ((uint32_t)(data * 17) << 8);
	}
}

// ------------------------------------------------------------------ Charset

void VID_SetCharset(const uint8_t *newCharset) {
	MemCopy(charset, newCharset, 2048);
	charsetInitialized = true;
}

void VID_SetCharsetWidth(uint8_t w) {
	if (screen2XMode)
		w = (uint8_t)(w * 2);
	for (int n = 0; n < 256; n++)
		charWidth[n] = w;
}

static void ApplyIBMPCTextMetrics(bool enable2X) {
	uint8_t baseColumnWidth = (screenVersion == DDB_VERSION_PAWS || screenMachine == DDB_MACHINE_PCW) ? 8 : 6;
	lineHeight = enable2X ? 16 : 8;
	screenCellWidth = enable2X ? 16 : 8;
	columnWidth = enable2X ? (uint8_t)(baseColumnWidth * 2) : baseColumnWidth;
	defaultCharWidth = columnWidth;
	screen2XMode = enable2X;
	charset16Available = false;
	for (int n = 0; n < 256; n++)
		charWidth[n] = defaultCharWidth;
}

// Charset/font loading lives in the shared src-common/vid_font.cpp. This
// renderer marks the charset live after a load, and - being a 2x-capable
// target (HAS_HIRES_FONT) - stores the high-res glyphs when a 2x mode is
// active.
void VID_ActivateCharset() {
	charsetInitialized = true;
}

bool VID_StoreFont2X(const DMG_Font *font, const char *filename) {
	if (!screen2XMode)
		return false;

	if (DMG_IsSINTACFontV4(filename)) {
		MemCopy(charset16, font->bitmap16, sizeof(font->bitmap16));
		MemCopy(charWidth, font->width16, sizeof(font->width16));
		charset16Available = true;
	} else {
		MemCopy(charset, font->bitmap8, sizeof(font->bitmap8));
		for (int n = 0; n < 256; n++)
			charWidth[n] = (uint8_t)(font->width8[n] * 2);
	}
	return true;
}

// -------------------------------------------------------------- PCX support

#if HAS_PCX
static bool FileExists(const char *fileName) {
	File *file = File_Open(fileName, ReadOnly);
	if (file == 0)
		return false;
	File_Close(file);
	return true;
}

static void BuildFileNameWithExtension(const char *fileName, const char *extension, char *output, size_t outputSize) {
	StrCopy(output, outputSize, fileName);
	char *dot = (char *)StrRChr(output, '.');
	if (dot == 0)
		dot = output + StrLen(output);
	StrCopy(dot, output + outputSize - dot, extension);
}

static bool FindNamedPCXFile(const char *fileName, char *output, size_t outputSize) {
	BuildFileNameWithExtension(fileName, ".VGA", output, outputSize);
	if (FileExists(output))
		return true;

	BuildFileNameWithExtension(fileName, ".vga", output, outputSize);
	if (FileExists(output))
		return true;

	BuildFileNameWithExtension(fileName, ".PCX", output, outputSize);
	if (FileExists(output))
		return true;

	BuildFileNameWithExtension(fileName, ".pcx", output, outputSize);
	return FileExists(output);
}

static void FreeBufferedPCXPicture() {
	if (pcxPictureData != NULL) {
		Free(pcxPictureData);
		pcxPictureData = NULL;
	}
	#if HAS_SPECTRUM
	if (zxsPictureBitmap != NULL) {
		Free(zxsPictureBitmap);
		zxsPictureBitmap = NULL;
	}
	if (zxsPictureAttributes != NULL) {
		Free(zxsPictureAttributes);
		zxsPictureAttributes = NULL;
	}
	pcxPictureSize = 0;
	pcxPictureWidth = 0;
	pcxPictureHeight = 0;
	zxsPictureWidth = 0;
	zxsPictureHeight = 0;
	zxsPictureMirror = false;
	#endif
}

#if HAS_SPECTRUM
static uint8_t ReverseBits(uint8_t value) {
	value = (uint8_t)(((value & 0xF0) >> 4) | ((value & 0x0F) << 4));
	value = (uint8_t)(((value & 0xCC) >> 2) | ((value & 0x33) << 2));
	value = (uint8_t)(((value & 0xAA) >> 1) | ((value & 0x55) << 1));
	return value;
}

static bool HasOnlyPaddingBytes(const uint8_t *data, size_t offset, size_t size) {
	for (size_t i = offset; i < size; i++) {
		if (data[i] != 0x00 && data[i] != 0xE5)
			return false;
	}
	return true;
}

static uint32_t GetSpectrumBitmapOffset(uint32_t y) {
	return ((y & 0xC0u) << 5) |
		((y & 0x07u) << 8) |
		((y & 0x38u) << 2);
}

static void DecodeLegacyZXSBitmap(const uint8_t *source, uint8_t numLines, uint8_t *destination) {
	uint8_t scrBitmap[6144];
	memset(scrBitmap, 0, sizeof(scrBitmap));

	size_t sourceOffset = 0;
	uint8_t remainingLines = numLines;
	uint32_t scrOffset = 0;
	while (remainingLines >= 64) {
		memcpy(scrBitmap + scrOffset, source + sourceOffset, 2048);
		sourceOffset += 2048;
		scrOffset += 2048;
		remainingLines -= 64;
	}

	if (remainingLines != 0) {
		size_t rows = (size_t)remainingLines / 8u;
		size_t bytesToRead = rows * 32u;
		for (size_t block = 0; block < 8; block++) {
			memcpy(scrBitmap + scrOffset + block * 256u, source + sourceOffset, bytesToRead);
			sourceOffset += bytesToRead;
		}
	}

	for (uint32_t y = 0; y < numLines; y++)
		memcpy(destination + y * 32u, scrBitmap + GetSpectrumBitmapOffset(y), 32);
}

static bool LoadExternalZXSPicture(const char *fileName) {
	File *file = File_Open(fileName, ReadOnly);
	if (file == 0) {
		DDB_SetError(DDB_ERROR_FILE_NOT_FOUND);
		return false;
	}

	uint64_t size = File_GetSize(file);
	if (size < 2 || size > 65535) {
		DDB_SetError(DDB_ERROR_INVALID_FILE);
		File_Close(file);
		return false;
	}

	uint8_t *compressed = Allocate<uint8_t>("ZXS picture", (size_t)size);
	if (compressed == NULL) {
		DDB_SetError(DDB_ERROR_OUT_OF_MEMORY);
		File_Close(file);
		return false;
	}

	if (File_Read(file, compressed, size) != size) {
		DDB_SetError(DDB_ERROR_READING_FILE);
		File_Close(file);
		Free(compressed);
		return false;
	}
	File_Close(file);

	if (zxsPictureBitmap == NULL)
		zxsPictureBitmap = Allocate<uint8_t>("ZXS picture bitmap", 32 * 192);
	if (zxsPictureAttributes == NULL)
		zxsPictureAttributes = Allocate<uint8_t>("ZXS picture attributes", 32 * 24);
	if (zxsPictureBitmap == NULL || zxsPictureAttributes == NULL) {
		DDB_SetError(DDB_ERROR_OUT_OF_MEMORY);
		Free(compressed);
		return false;
	}

	memset(zxsPictureBitmap, 0, 32 * 192);
	memset(zxsPictureAttributes, 0, 32 * 24);

	uint8_t legacyNumLines = compressed[0];
	if ((legacyNumLines & 7) == 0 && legacyNumLines <= 192) {
		size_t legacyBitmapBytes = 0;
		uint8_t remainingLines = legacyNumLines;
		if (remainingLines >= 64) {
			legacyBitmapBytes += 2048;
			remainingLines -= 64;
		}
		if (remainingLines >= 64) {
			legacyBitmapBytes += 2048;
			remainingLines -= 64;
		}
		if (remainingLines >= 64) {
			legacyBitmapBytes += 2048;
			remainingLines -= 64;
		}
		if (remainingLines != 0)
			legacyBitmapBytes += ((size_t)remainingLines / 8u) * 32u * 8u;

		size_t legacyAttributeBytes = ((size_t)legacyNumLines / 8u) * 32u;
		size_t legacySize = 1 + legacyBitmapBytes + legacyAttributeBytes;
		if (legacySize <= size && HasOnlyPaddingBytes(compressed, legacySize, (size_t)size)) {
			DecodeLegacyZXSBitmap(compressed + 1, legacyNumLines, zxsPictureBitmap);
			memcpy(zxsPictureAttributes, compressed + 1 + legacyBitmapBytes, legacyAttributeBytes);
			zxsPictureWidth = 256;
			zxsPictureHeight = legacyNumLines;
			zxsPictureMirror = false;
			Free(compressed);
			return true;
		}
	}

	if (size >= 6) {
		uint16_t bitmapSize = (uint16_t)(compressed[0] | (compressed[1] << 8));
		uint16_t attributeSize = (uint16_t)(compressed[2] | (compressed[3] << 8));
		uint8_t numLinesBitmap = compressed[4];
		uint8_t numLinesAttributes = compressed[5] & 0x7F;
		bool mirror = (compressed[5] & 0x80) != 0;

		uint32_t expectedBitmapBytes = (uint32_t)numLinesBitmap * 32;
		uint32_t expectedAttributeBytes = (uint32_t)numLinesAttributes * 32;
		if (numLinesBitmap <= 192 && numLinesAttributes <= 24 &&
			numLinesAttributes >= ((numLinesBitmap + 7) >> 3) &&
			6u + bitmapSize + attributeSize == size &&
			expectedBitmapBytes <= 6144 && expectedAttributeBytes <= 768) {
			uint8_t *bitmapRows = Allocate<uint8_t>("ZXS bitmap rows", expectedBitmapBytes);
			uint8_t *attributeRows = Allocate<uint8_t>("ZXS attribute rows", expectedAttributeBytes);
			if ((expectedBitmapBytes != 0 && bitmapRows == NULL) ||
				(expectedAttributeBytes != 0 && attributeRows == NULL)) {
				DDB_SetError(DDB_ERROR_OUT_OF_MEMORY);
				Free(bitmapRows);
				Free(attributeRows);
				Free(compressed);
				return false;
			}

			bool ok = DMG_DecompressZX0(compressed + 6, bitmapSize, bitmapRows, expectedBitmapBytes) &&
				DMG_DecompressZX0(compressed + 6 + bitmapSize, attributeSize, attributeRows, expectedAttributeBytes);
			if (!ok) {
				DDB_SetError(DDB_ERROR_INVALID_FILE);
				Free(bitmapRows);
				Free(attributeRows);
				Free(compressed);
				return false;
			}

			for (uint32_t y = 0; y < numLinesBitmap; y++) {
				uint8_t *dst = zxsPictureBitmap + y * 32;
				uint8_t *src = bitmapRows + y * 32;
				if (mirror) {
					memcpy(dst, src, 16);
					for (int x = 0; x < 16; x++)
						dst[16 + x] = ReverseBits(src[15 - x]);
				} else {
					memcpy(dst, src, 32);
				}
			}
			for (uint32_t y = 0; y < numLinesAttributes; y++) {
				uint8_t *dst = zxsPictureAttributes + y * 32;
				uint8_t *src = attributeRows + y * 32;
				if (mirror) {
					memcpy(dst, src, 16);
					for (int x = 0; x < 16; x++)
						dst[16 + x] = src[15 - x];
				} else {
					memcpy(dst, src, 32);
				}
			}

			Free(bitmapRows);
			Free(attributeRows);
			zxsPictureWidth = 256;
			zxsPictureHeight = numLinesBitmap;
			zxsPictureMirror = mirror;
			Free(compressed);
			return true;
		}
	}

	DDB_SetError(DDB_ERROR_INVALID_FILE);
	Free(compressed);
	return false;
}
#endif

static bool HasExternalPCXGraphics(const char *fileName) {
	char introScreen[FILE_MAX_PATH];
	if (FindNamedPCXFile(fileName, introScreen, sizeof(introScreen)))
		return true;

	for (int picno = 0; picno < 256; picno++) {
		char pictureFileName[FILE_MAX_PATH];
		if (VID_GetExternalPictureFileName((uint8_t)picno, pictureFileName, sizeof(pictureFileName)))
			return true;
	}

	return false;
}

static bool IsPCXScreenFile(const char *fileName) {
	const char *dot = StrRChr(fileName, '.');
	if (dot != 0 && (StrIComp(dot, ".vga") == 0 || StrIComp(dot, ".pcx") == 0))
		return true;

	uint8_t header[128];
	File *file = File_Open(fileName, ReadOnly);
	if (file == 0)
		return false;
	bool ok = File_Read(file, header, sizeof(header)) == sizeof(header);
	File_Close(file);
	return ok && header[0] == 0x0A && header[2] == 1 && header[3] == 8 && header[65] == 1;
}
#endif

// ---------------------------------------------------------- Drawing surface

bool VID_IsBackBufferEnabled() {
	return true;
}

void VID_EnableBackBuffer() {
	// Always enabled
}

void VID_Clear(int x, int y, int w, int h, uint8_t color, VID_ClearMode mode) {
	if (mode == Clear_All && attributes && x <= 0 && y <= 0 && w >= screenWidth && h >= screenHeight) {
		uint8_t attrValue = VID_GetAttributes();
		VID_SetPaper(color);
		uint8_t clearAttributes = VID_GetAttributes();
		VID_SetAttributes(attrValue);
		memset(bitmap, 0, stride * screenHeight);
		memset(attributes, clearAttributes, stride * (screenHeight >> 3));
		if (frontBuffer)
			memset(frontBuffer, color, screenWidth * screenHeight);
		if (backBuffer)
			memset(backBuffer, color, screenWidth * screenHeight);
		return;
	}

	if (y < 0) {
		h += y;
		y = 0;
	}
	if (x < 0) {
		w += x;
		x = 0;
	}
	if (y >= screenHeight)
		return;
	if (x >= screenWidth)
		return;
	if (x + w > screenWidth)
		w = screenWidth - x;
	if (y + h > screenHeight)
		h = screenHeight - y;
	if (w <= 0)
		return;
	if (h <= 0)
		return;

	if (attributes) {
		uint8_t maskLeft = 0xFF00 >> (x & 7);
		uint8_t maskRight = (0x00FF << ((x + w) & 7)) >> 8;
		w = ((x + w - 1) >> 3) - (x >> 3);
		for (int dy = 0; dy < h; dy++) {
			uint8_t *ptr = bitmap + (y + dy) * stride + (x >> 3);
			if (w == 0)
				*ptr &= (maskLeft | maskRight);
			else {
				ptr[0] &= maskLeft;
				for (int dx = 1; dx < w; dx++)
					ptr[dx] = 0;
				ptr[w] &= maskRight;
			}
		}

#if HAS_DRAWSTRING
		uint8_t *attr = attributes + (y >> 3) * stride + (x >> 3);
		uint8_t attrValue = VID_GetAttributes();
		VID_SetPaper(color);
		uint8_t attrSet = VID_GetAttributes();
		for (int dy = 0; dy < h; dy += 8) {
			for (int dx = 0; dx <= w; dx++)
				attr[dx] = attrSet;
			attr += stride;
		}
		VID_SetAttributes(attrValue);
#endif
	} else {
		for (int dy = 0; dy < h; dy++) {
			uint8_t *ptr = textBuffer + (y + dy) * screenWidth + x;
			for (int dx = 0; dx < w; dx++)
				ptr[dx] = color;
		}
	}
}

void VID_Scroll(int x, int y, int w, int h, int lines, uint8_t paper) {
	int dy = 0;

	if (lines < h) {
		if (attributes) {
			w >>= 3;
			for (; dy < h - lines; dy++) {
				uint8_t *ptr = bitmap + (y + dy) * stride + (x >> 3);
				uint8_t *next = ptr + lines * stride;

				for (int dx = 0; dx < w; dx++)
					ptr[dx] = next[dx];
			}

			if (lines >= 8 && w >= 1) {
				int row0 = y >> 3;
				int row1 = (y + h - lines) >> 3;
				int rows = h >> 3;
				int cols = w;
				int col0 = x >> 3;
				int inc  = stride * (lines >> 3);
				int attv = (paper << 3);
				for (int row = row0; row < row1; row++) {
					uint8_t *attr = attributes + row * stride + col0;
					for (int col = 0; col < cols; col++)
						attr[col] = attr[col + inc];
				}
				rows = lines >> 3;
				for (int row = row1; row < row1 + rows; row++) {
					uint8_t *attr = attributes + row * stride + col0;
					for (int col = 0; col < cols; col++)
						attr[col] = attv;
				}
			}
		} else {
			for (; dy < h - lines; dy++) {
				uint8_t *ptr = textBuffer + (y + dy) * screenWidth + x;
				uint8_t *next = ptr + lines * screenWidth;

				for (int dx = 0; dx < w; dx++)
					ptr[dx] = next[dx];
			}
		}
	}

	VID_Clear(x, y + dy, w, lines, paper);
}

void VID_ClearBuffer(bool front) {
	uint8_t *buffer = front ? frontBuffer : backBuffer;
	if (buffer == NULL)
		return;
	memset(buffer, 0, screenWidth * screenHeight);
}

void VID_SetOpBuffer(SCR_Operation op, bool front) {
	switch (op) {
	case SCR_OP_DRAWPICTURE:
		graphicsBuffer = front ? frontBuffer : backBuffer;
		break;
	case SCR_OP_DRAWTEXT:
		textBuffer = front ? frontBuffer : backBuffer;
		break;
	}
}

void VID_SaveScreen() {
	uint8_t *pixels = frontBuffer;

	for (int y = 0; y < screenHeight; y++) {
		memcpy(backBuffer + y * screenWidth, pixels, screenWidth);
		pixels += screenWidth;
	}
}

void VID_RestoreScreen() {
	uint8_t *pixels = frontBuffer;

	for (int y = 0; y < screenHeight; y++) {
		memcpy(pixels, backBuffer + y * screenWidth, screenWidth);
		pixels += screenWidth;
	}
}

void VID_SwapScreen() {
	uint8_t *pixels = frontBuffer;
	uint8_t *src = backBuffer;

	for (int y = 0; y < screenHeight; y++) {
		uint8_t *dst = pixels;
		for (int x = 0; x < screenWidth; x++) {
			uint8_t c = *src++;
			*src++ = *dst;
			*dst++ = c;
		}
		pixels += screenWidth;
	}
}

void VID_DrawCharacter(int x, int y, uint8_t ch, uint8_t ink, uint8_t paper) {
	uint8_t width = charWidth[ch];

#if HAS_DRAWSTRING
	if (ink != 255)
		VID_SetInk(ink);
	if (paper != 255)
		VID_SetPaper(paper);
#endif

	if (attributes) {
		uint8_t *ptr = charset + (ch << 3);
		uint8_t *out = bitmap + y * stride + (x >> 3);
		uint8_t rot = x & 7;
		uint8_t *attr = attributes + (y >> 3) * stride + (x >> 3);
		uint8_t xattr = 0;
		uint8_t paperShift = 4;

		// TODO: Use DRAWSTRING routines to handle attributes,
		// attributes shouldn't be supported without DRAWSTRING

		if (screenMachine == DDB_MACHINE_SPECTRUM) {
			xattr = (ink & 0x18) << 3;
			ink &= 7;
			if (paper != 255)
				paper &= 7;
			paperShift = 3;
		}

		if (paper == 255) {
			// MSX and C64 colors are a full nibble; keep the paper nibble intact
			uint8_t keep = (screenMachine == DDB_MACHINE_MSX ||
			                screenMachine == DDB_MACHINE_C64) ? 0xF0 : 0x37;
			*attr = (*attr & keep) | ink | xattr;
			if (rot > 8 - width)
				attr[1] = (attr[1] & keep) | ink | xattr;
		} else {
			if (screenMachine != DDB_MACHINE_MSX && screenMachine != DDB_MACHINE_C64)
				paper &= 7;
			*attr = ink | (paper << paperShift) | xattr;
			if (rot > 8 - width)
				attr[1] = *attr;
		}

		for (int line = 0; line < 8; line++) {
			uint8_t *sav = out;
			uint8_t mask = 0x80 >> rot;
			for (int col = 0; col < width; col++) {
				if ((ptr[line] & (0x80 >> col)))
					*out |= mask;
				else if (paper != 255)
					*out &= ~mask;
				mask >>= 1;
				if (mask == 0) {
					mask = 0x80;
					out++;
				}
			}
			out = sav + stride;
		}
		return;
	}

	uint8_t *ptr = charset + (ch << 3);
	uint8_t *pixels = textBuffer + y * screenWidth + x;
	if (screen2XMode) {
		if (charset16Available) {
			const uint8_t *ptr16 = charset16 + ch * 32;
			if (paper == 255) {
				for (int line = 0; line < 16; line++) {
					uint16_t bits = (uint16_t)(ptr16[line * 2] << 8) | ptr16[line * 2 + 1];
					uint8_t *row = pixels + line * screenWidth;
					for (int col = 0; col < width; col++)
						if ((bits & (0x8000 >> col)) != 0)
							row[col] = ink;
				}
			} else {
				for (int line = 0; line < 16; line++) {
					uint16_t bits = (uint16_t)(ptr16[line * 2] << 8) | ptr16[line * 2 + 1];
					uint8_t *row = pixels + line * screenWidth;
					for (int col = 0; col < width; col++)
						row[col] = (bits & (0x8000 >> col)) ? ink : paper;
				}
			}
			return;
		}

		uint8_t sourceWidth = (uint8_t)((width + 1) >> 1);
		if (paper == 255) {
			for (int line = 0; line < 8; line++) {
				uint8_t *row0 = pixels + (line * 2) * screenWidth;
				uint8_t *row1 = row0 + screenWidth;
				for (int col = 0; col < sourceWidth; col++) {
					if ((ptr[line] & (0x80 >> col)) == 0)
						continue;
					int dx = col * 2;
					row0[dx] = ink;
					row0[dx + 1] = ink;
					row1[dx] = ink;
					row1[dx + 1] = ink;
				}
			}
		} else {
			for (int line = 0; line < 8; line++) {
				uint8_t *row0 = pixels + (line * 2) * screenWidth;
				uint8_t *row1 = row0 + screenWidth;
				for (int col = 0; col < sourceWidth; col++) {
					uint8_t color = (ptr[line] & (0x80 >> col)) ? ink : paper;
					int dx = col * 2;
					row0[dx] = color;
					row0[dx + 1] = color;
					row1[dx] = color;
					row1[dx + 1] = color;
				}
			}
		}
		return;
	}

	if (paper == 255) {
		for (int line = 0; line < 8; line++, pixels += screenWidth) {
			for (int col = 0; col < width; col++)
				if ((ptr[line] & (0x80 >> col)))
					pixels[col] = ink;
		}
	} else {
		for (int line = 0; line < 8; line++, pixels += screenWidth) {
			for (int col = 0; col < width; col++)
				pixels[col] = (ptr[line] & (0x80 >> col)) ? ink : paper;
		}
	}
}

void VID_GetPaletteColor(uint8_t color, uint8_t *r, uint8_t *g, uint8_t *b) {
	uint32_t c = palette[color];
	*r = (c >> 16) & 0xFF;
	*g = (c >> 8) & 0xFF;
	*b = (c >> 0) & 0xFF;
}

uint16_t VID_GetPaletteSize() {
	return 256;
}

void VID_SetPaletteColor(uint8_t color, uint8_t r, uint8_t g, uint8_t b) {
	palette[color] = (r << 16) | (g << 8) | b;
}

static uint8_t *screenBackupBuffer = 0;

bool VID_BackupScreen() {
	if (graphicsBuffer == 0 || screenWidth == 0 || screenHeight == 0)
		return false;
	if (screenBackupBuffer == 0)
		screenBackupBuffer = Allocate<uint8_t>("Screen backup", screenWidth * screenHeight);
	if (screenBackupBuffer == 0)
		return false;
	memcpy(screenBackupBuffer, graphicsBuffer, screenWidth * screenHeight);
	return true;
}

bool VID_RestoreBackupScreen() {
	if (screenBackupBuffer == 0 || graphicsBuffer == 0)
		return false;
	memcpy(graphicsBuffer, screenBackupBuffer, screenWidth * screenHeight);
	return true;
}

bool VID_DisplaySCRFile(const char *fileName, DDB_Machine target, bool fadeIn) {
	#if HAS_PCX
	if (IsPCXScreenFile(fileName)) {
		uint32_t bufferSize = (uint32_t)screenWidth * (uint32_t)screenHeight;
		uint8_t *output = Allocate<uint8_t>("PCX screen", bufferSize);
		if (output == 0) {
			DDB_SetError(DDB_ERROR_OUT_OF_MEMORY);
			return false;
		}

		int width = 0;
		int height = 0;
		if (!DMG_DecompressPCX(fileName, output, &bufferSize, &width, &height, palette)) {
			DDB_SetError(DDB_ERROR_INVALID_FILE);
			Free(output);
			return false;
		}

		memset(graphicsBuffer, 0, screenWidth * screenHeight);
		int copyWidth = width > screenWidth ? screenWidth : width;
		int copyHeight = height > screenHeight ? screenHeight : height;
		for (int y = 0; y < copyHeight; y++)
			memcpy(graphicsBuffer + y * screenWidth, output + y * width, copyWidth);

		if (fadeIn)
			VID_ActivatePalette();
		Free(output);
		return true;
	}
	#endif

	#if HAS_SPECTRUM
	if (target == DDB_MACHINE_SPECTRUM && bitmap != NULL && attributes != NULL) {
		File *file = File_Open(fileName, ReadOnly);
		if (file == 0) {
			DDB_SetError(DDB_ERROR_FILE_NOT_FOUND);
			return false;
		}

		uint64_t size = File_GetSize(file);
		if (size < 6912 || size > 7040) {
			DDB_SetError(DDB_ERROR_INVALID_FILE);
			File_Close(file);
			return false;
		}

		uint8_t *buffer = Allocate<uint8_t>("SCR Temporary buffer", 7040);
		if (buffer == 0) {
			DDB_SetError(DDB_ERROR_OUT_OF_MEMORY);
			File_Close(file);
			return false;
		}

		if (File_Read(file, buffer, size) != size) {
			DDB_SetError(DDB_ERROR_READING_FILE);
			File_Close(file);
			Free(buffer);
			return false;
		}
		File_Close(file);

		while (size > 6912 && (buffer[size - 1] == 0x00 || buffer[size - 1] == 0xE5))
			size--;
		if (size != 6912) {
			DDB_SetError(DDB_ERROR_INVALID_FILE);
			Free(buffer);
			return false;
		}

		memset(bitmap, 0, 32 * 192);
		for (int y = 0; y < 192; y++) {
			uint32_t sourceOffset = ((uint32_t)(y & 0xC0) << 5) |
			                      ((uint32_t)(y & 0x07) << 8) |
			                      ((uint32_t)(y & 0x38) << 2);
			memcpy(bitmap + y * stride, buffer + sourceOffset, 32);
		}
		memcpy(attributes, buffer + 6144, 32 * 24);
		memcpy(palette, ZXSpectrumPalette, sizeof(ZXSpectrumPalette));

		if (fadeIn)
			VID_ActivatePalette();

		Free(buffer);
		return true;
	}
	#endif

	uint8_t *buffer = Allocate<uint8_t>("SCR Temporary buffer", 65536);
	bool fileIsHAM = false;
	bool ok = SCR_GetScreen(fileName, target, buffer, 65536,
		graphicsBuffer, screenWidth, screenHeight, palette, &fileIsHAM);
	scrHAM6 = ok && fileIsHAM;
	Free(buffer);
	return ok;
}

void VID_LoadPicture(uint8_t picno, DDB_ScreenMode mode) {
	bufferedHAM6 = false;
	#if HAS_PCX
	FreeBufferedPCXPicture();
	bufferedEntry = NULL;
	pictureData = NULL;
	bufferedIndex = picno;
	(void)mode;

	if (dmg == NULL) {
		char pictureFileName[FILE_MAX_PATH];
		if (!VID_GetExternalPictureFileName(picno, pictureFileName, sizeof(pictureFileName)))
			return;

		#if HAS_SPECTRUM
		const char *dot = StrRChr(pictureFileName, '.');
		if (dot != NULL && StrIComp(dot, ".zxs") == 0) {
			LoadExternalZXSPicture(pictureFileName);
			return;
		}
		#endif

		pcxPictureSize = (uint32_t)DMG_MAX_IMAGE_WIDTH * (uint32_t)DMG_MAX_IMAGE_HEIGHT;
		pcxPictureData = Allocate<uint8_t>("PCX picture", pcxPictureSize);
		if (pcxPictureData == NULL)
			return;

		if (!DMG_DecompressPCX(pictureFileName, pcxPictureData, &pcxPictureSize, &pcxPictureWidth, &pcxPictureHeight, pcxPalette))
			FreeBufferedPCXPicture();
		return;
	}
	#else
	if (dmg == NULL)
		return;
	#endif

	DMG_Entry *entry = DMG_GetEntry(dmg, picno);
	if (entry == NULL || entry->type != DMGEntry_Image)
		return;

	bufferedEntry = entry;
	bufferedIndex = picno;
	if (dmg->version == DMG_Version1_PCW)
		pictureData = DMG_GetEntryDataChunky(dmg, picno);
	else {
		DMG_ImageMode imageMode = ImageMode_Packed;
		if (dmg->version == DMG_Version5 ||
			dmg->screenMode == ScreenMode_CGA ||
			dmg->screenMode == ScreenMode_EGA)
			imageMode = ImageMode_Indexed;
		pictureData = DMG_GetEntryData(dmg, picno, imageMode);
	}
	bufferedHAM6 = pictureData != 0 && IsHAM6Display();
	if (pictureData == 0) {
		bufferedEntry = NULL;
	}
}

void VID_GetPictureInfo(bool *fixed, int16_t *x, int16_t *y, int16_t *w, int16_t *h) {
	#if HAS_PCX
	#if HAS_SPECTRUM
	if (zxsPictureBitmap != NULL && zxsPictureAttributes != NULL) {
		if (fixed != NULL)
			*fixed = true;
		if (x != NULL)
			*x = 0;
		if (y != NULL)
			*y = 0;
		if (w != NULL)
			*w = zxsPictureWidth;
		if (h != NULL)
			*h = zxsPictureHeight;
		return;
	}
	#endif

	if (pcxPictureData != NULL) {
		if (fixed != NULL)
			*fixed = true;
		if (x != NULL)
			*x = 0;
		if (y != NULL)
			*y = 0;
		if (w != NULL)
			*w = pcxPictureWidth;
		if (h != NULL)
			*h = pcxPictureHeight;
		return;
	}
	#endif

	if (bufferedEntry == NULL) {
		if (fixed != NULL)
			*fixed = false;
		if (x != NULL)
			*x = 0;
		if (y != NULL)
			*y = 0;
		if (w != NULL)
			*w = 0;
		if (h != NULL)
			*h = 0;
	} else {
		if (fixed != NULL)
			*fixed = (bufferedEntry->flags & DMG_FLAG_FIXED) != 0;
		if (x != NULL)
			*x = bufferedEntry->x;
		if (y != NULL)
			*y = bufferedEntry->y;
		if (w != NULL)
			*w = bufferedEntry->width;
		if (h != NULL)
			*h = bufferedEntry->height;
	}
}

void VID_DisplayPicture(int x, int y, int w, int h, DDB_ScreenMode mode) {
	#if HAS_PCX
	#if HAS_SPECTRUM
	if (zxsPictureBitmap != NULL && zxsPictureAttributes != NULL && bitmap != NULL && attributes != NULL) {
		(void)x;
		(void)y;
		(void)w;
		(void)h;
		(void)mode;
		VID_Clear(0, 0, screenWidth, screenHeight, 0, Clear_All);
		memcpy(bitmap, zxsPictureBitmap, 32 * 192);
		memcpy(attributes, zxsPictureAttributes, 32 * 24);
		memcpy(palette, ZXSpectrumPalette, sizeof(ZXSpectrumPalette));
		VID_ActivatePalette();
		return;
	}
	#endif

	if (pcxPictureData != NULL) {
		int srcX = 0;
		int srcY = 0;

		if (x < 0) {
			srcX = -x;
			w += x;
			x = 0;
		}
		if (y < 0) {
			srcY = -y;
			h += y;
			y = 0;
		}
		if (x >= screenWidth || y >= screenHeight || w <= 0 || h <= 0)
			return;

		if (w > pcxPictureWidth - srcX)
			w = pcxPictureWidth - srcX;
		if (h > pcxPictureHeight - srcY)
			h = pcxPictureHeight - srcY;
		if (w > screenWidth - x)
			w = screenWidth - x;
		if (h > screenHeight - y)
			h = screenHeight - y;
		if (w <= 0 || h <= 0)
			return;

		uint8_t *srcPtr = pcxPictureData + srcY * pcxPictureWidth + srcX;
		uint8_t *dstPtr = graphicsBuffer + y * screenWidth + x;
		for (int dy = 0; dy < h; dy++, srcPtr += pcxPictureWidth, dstPtr += screenWidth)
			memcpy(dstPtr, srcPtr, w);

		memcpy(palette, pcxPalette, sizeof(pcxPalette));
		VID_ActivatePalette();
		return;
	}
	#endif

	DMG_Entry *entry = bufferedEntry;
	if (pictureData == NULL)
		return;
	if (entry == NULL)
		return;

	if (x < 0) {
		w += x;
		x = 0;
	}
	if (y < 0) {
		h += y;
		y = 0;
	}
	if (w <= 0 || h <= 0)
		return;

	if (h > entry->height)
		h = entry->height;
	if (w > entry->width)
		w = entry->width;

	uint8_t *srcPtr = (uint8_t *)pictureData;
	uint8_t *dstPtr = graphicsBuffer + y * screenWidth + x;
	uint32_t *filePalette = (uint32_t *)DMG_GetEntryPalette(dmg, bufferedIndex);
	bool indexedPicture =
		dmg->version == DMG_Version5 ||
		dmg->screenMode == ScreenMode_CGA ||
		dmg->screenMode == ScreenMode_EGA;
	if (indexedPicture) {
		for (int dy = 0; dy < h; dy++, srcPtr += entry->width, dstPtr += screenWidth)
			memcpy(dstPtr, srcPtr, w);
	} else {
		for (int dy = 0; dy < h; dy++, srcPtr += entry->width / 2, dstPtr += screenWidth) {
			for (int dx = 0, ix = 0; dx < w; dx += 2, ix++) {
				dstPtr[dx] = srcPtr[ix] >> 4;
				dstPtr[dx + 1] = srcPtr[ix] & 0x0F;
			}
		}
	}

	switch (mode) {
	default:
	case ScreenMode_VGA16:
		if (entry->flags & DMG_FLAG_FIXED) {
			if (dmg->version == DMG_Version1)
				filePalette[15] = 0xFFFFFFFF;
			int paletteCount = bufferedHAM6 ? 16 : DMG_GetEntryPaletteSize(dmg, bufferedIndex);
			int firstColor = bufferedHAM6 ? 0 : DMG_GetEntryFirstColor(dmg, bufferedIndex);
			if (!PaletteMatches(filePalette, paletteCount, firstColor, firstColor == 0)) {
				if (firstColor == 0) {
					for (int n = 0; n < 256; n++)
						palette[n] = 0xFF000000;
				}
				for (int n = 0; n < paletteCount && firstColor + n < 256; n++)
					palette[firstColor + n] = filePalette[n];
			}
		}
		break;

	case ScreenMode_EGA:
		break;

	case ScreenMode_CGA: {
			uint32_t *cgaPalette = DMG_GetCGAMode(entry) == CGA_Red ? CGAPaletteRed : CGAPaletteCyan;
			if (!PaletteMatches(cgaPalette, 16, 0, false)) {
				for (int n = 0; n < 16; n++)
					palette[n] = cgaPalette[n];
			}
		}
		break;
	}
}

// -------------------------------------------------------------- Presentation

static void RenderSpectrumScreen(uint8_t *attrs, int fixedFlashPhase = -1) {
	bool flashOn = fixedFlashPhase < 0 ? ((g_system->getMillis() / 500) & 1) : fixedFlashPhase != 0;

	uint8_t *attrPtr = attrs;
	uint8_t cols = screenWidth / 8;
	uint8_t rows = screenHeight / 8;
	bool spectrum = (screenMachine == DDB_MACHINE_SPECTRUM);
	for (int y = 0; y < rows; y++) {
		uint8_t *ptr = bitmap + stride * (y * 8);
		uint8_t *out = frontBuffer + y * 8 * screenWidth;

		for (int x = 0; x < cols; x++, ptr++) {
			uint8_t attr = *attrPtr++;
			uint8_t ink, paper;
			if (spectrum) {
				ink = (attr & 0x07);
				paper = ((attr >> 3) & 0x07);
				if (attr & 0x40) {
					// Bright On
					ink |= 0x8;
					if (paper != 0)
						paper |= 0x08;
				}
				if ((attr & 0x80) && flashOn) {
					// Flash On
					uint8_t tmp = ink;
					ink = paper;
					paper = tmp;
				}
			} else {
				ink = (attr & 0x0F);
				paper = ((attr >> 4) & 0x0F);
			}

			uint8_t *outPtr = out;

			for (int cy = 0; cy < 8; cy++) {
				uint8_t pixels = ptr[cy * stride];

				for (int cx = 0; cx < 8; cx++) {
					out[cx] = (pixels & 0x80) ? ink : paper;
					pixels <<= 1;
				}
				out += screenWidth;
			}

			out = outPtr + 8;
		}
	}
}

/**
 * Hand the composed 8 bit buffer to the backend.
 *
 * In indexed mode the indices go across untouched. 
 * In True Color mode they are expanded here,
 * which is also the only way HAM6 pictures
 * (whose "indices" are per pixel color modification commands)
 * can be shown at all.
 */
static void VID_Present() {
	if (frontBuffer == NULL)
		return;

	if (attributes)
		RenderSpectrumScreen(attributes);

	if (presentFormat.bytesPerPixel == 1) {
		byte pal[256 * 3];
		for (int n = 0; n < 256; n++) {
			pal[n * 3 + 0] = (palette[n] >> 16) & 0xFF;
			pal[n * 3 + 1] = (palette[n] >> 8) & 0xFF;
			pal[n * 3 + 2] = palette[n] & 0xFF;
		}
		g_system->getPaletteManager()->setPalette(pal, 0, 256);
		g_system->copyRectToScreen(frontBuffer, screenWidth, 0, 0, screenWidth, screenHeight);
	} else if (presentBuffer != NULL) {
		bool ham6Display = IsHAM6Display();
		uint32 lookup[256];
		if (!ham6Display) {
			for (int n = 0; n < 256; n++)
				lookup[n] = presentFormat.RGBToColor((palette[n] >> 16) & 0xFF,
					(palette[n] >> 8) & 0xFF, palette[n] & 0xFF);
		}

		const uint8_t *src = frontBuffer;
		for (int y = 0; y < screenHeight; y++) {
			uint32_t held = 0xFF000000u | (palette[0] & 0x00FFFFFFu);
			if (presentFormat.bytesPerPixel == 2) {
				uint16 *dst = (uint16 *)(presentBuffer + y * screenWidth * 2);
				for (int x = 0; x < screenWidth; x++, src++) {
					if (ham6Display) {
						held = DecodeHAM6Command(*src, held);
						*dst++ = (uint16)presentFormat.RGBToColor((held >> 16) & 0xFF,
							(held >> 8) & 0xFF, held & 0xFF);
					} else {
						*dst++ = (uint16)lookup[*src];
					}
				}
			} else {
				uint32 *dst = (uint32 *)(presentBuffer + y * screenWidth * 4);
				for (int x = 0; x < screenWidth; x++, src++) {
					if (ham6Display) {
						held = DecodeHAM6Command(*src, held);
						*dst++ = presentFormat.RGBToColor((held >> 16) & 0xFF,
							(held >> 8) & 0xFF, held & 0xFF);
					} else {
						*dst++ = lookup[*src];
					}
				}
			}
		}

		g_system->copyRectToScreen(presentBuffer, screenWidth * presentFormat.bytesPerPixel,
			0, 0, screenWidth, screenHeight);
	}

	g_system->updateScreen();
}

// -------------------------------------------------------------------- Input

static const struct {
	Common::KeyCode key;
	uint8_t         code;
	uint8_t         ext;
	bool            special;
}
keyMapping[] = {
	{ Common::KEYCODE_KP_ENTER,  0x00, 0x1C, true  },
	{ Common::KEYCODE_HOME,      0x00, 0x47, true  },
	{ Common::KEYCODE_UP,        0x00, 0x48, true  },
	{ Common::KEYCODE_PAGEUP,    0x00, 0x49, true  },
	{ Common::KEYCODE_LEFT,      0x00, 0x4B, true  },
	{ Common::KEYCODE_RIGHT,     0x00, 0x4D, true  },
	{ Common::KEYCODE_END,       0x00, 0x4F, true  },
	{ Common::KEYCODE_DOWN,      0x00, 0x50, true  },
	{ Common::KEYCODE_PAGEDOWN,  0x00, 0x51, true  },
	{ Common::KEYCODE_INSERT,    0x00, 0x52, true  },
	{ Common::KEYCODE_DELETE,    0x00, 0x53, true  },
	{ Common::KEYCODE_F1,        0x00, 0x3B, true  },
	{ Common::KEYCODE_F2,        0x00, 0x3C, true  },
	{ Common::KEYCODE_F3,        0x00, 0x3D, true  },
	{ Common::KEYCODE_F4,        0x00, 0x3E, true  },
	{ Common::KEYCODE_F5,        0x00, 0x3F, true  },
	{ Common::KEYCODE_F6,        0x00, 0x40, true  },
	{ Common::KEYCODE_F7,        0x00, 0x41, true  },
	{ Common::KEYCODE_F8,        0x00, 0x42, true  },
	{ Common::KEYCODE_F9,        0x00, 0x43, true  },
	{ Common::KEYCODE_F10,       0x00, 0x44, true  },
	// It could also be 0x6D, which allows ENTER to work in Espacial
	{ Common::KEYCODE_RETURN,    0x0D, 0x00, true  },
	{ Common::KEYCODE_BACKSPACE, 0x08, 0x00, true  },
	{ Common::KEYCODE_ESCAPE,    0x1B, 0x00, true  },
	{ Common::KEYCODE_INVALID,   0x00, 0x00, false }
};

static void PushKey(uint16_t value) {
	if (((inputBufferHead + 1) & 0xFF) == inputBufferTail)
		return;
	inputBuffer[inputBufferHead] = value;
	inputBufferHead = (inputBufferHead + 1) & 255;
}

static void VID_PumpEvents() {
	Common::Event event;
	Common::EventManager *eventMan = g_system->getEventManager();

	while (eventMan->pollEvent(event)) {
		switch (event.type) {
		case Common::EVENT_QUIT:
		case Common::EVENT_RETURN_TO_LAUNCHER:
			quit = true;
			exitGame = true;
			return;

		case Common::EVENT_KEYDOWN: {
			inputModifiers = event.kbd.flags;

			bool handled = false;
			for (int n = 0; keyMapping[n].key != Common::KEYCODE_INVALID; n++) {
				if (keyMapping[n].key != event.kbd.keycode)
					continue;
				PushKey((uint16_t)(keyMapping[n].code | (keyMapping[n].ext << 8)));
				handled = true;
				break;
			}
			if (handled)
				break;

			// Printable keys reach the interpreter as DAAD character codes.
			// Outside text input mode the uppercased ASCII value is what the
			// original interpreters reported for INKEY and "press any key".
			uint16 ascii = event.kbd.ascii;
			if (ascii >= 32 && ascii < 256) {
				if (textInput)
					PushKey(DDB_ISO2Char[ascii & 0xFF]);
				else
					PushKey((uint16_t)ToUpper((uint8_t)ascii));
			}
			break;
		}

		case Common::EVENT_KEYUP:
			inputModifiers = event.kbd.flags;
			break;

		default:
			break;
		}
	}

	if (g_engine != nullptr && g_engine->shouldQuit()) {
		quit = true;
		exitGame = true;
	}
}

bool VID_AnyKey() {
	return inputBufferHead != inputBufferTail;
}

void VID_GetKey(uint8_t *key, uint8_t *ext, uint8_t *mod) {
	if (inputBufferHead != inputBufferTail) {
		int keyCode = inputBuffer[inputBufferTail] & 0xFF;
		int extCode = inputBuffer[inputBufferTail] >> 8;

		if (key != NULL)
			*key = keyCode;
		if (ext != NULL)
			*ext = extCode;
		if (mod != NULL) {
			*mod = 0;
			if (inputModifiers & Common::KBD_SHIFT)
				*mod |= SCR_KEYMOD_SHIFT;
			if (inputModifiers & Common::KBD_CTRL)
				*mod |= SCR_KEYMOD_CTRL;
			if (inputModifiers & Common::KBD_ALT)
				*mod |= SCR_KEYMOD_ALT;
		}
		inputBufferTail = (inputBufferTail + 1) & 255;
	} else {
		if (key != NULL)
			*key = 0;
		if (ext != NULL)
			*ext = 0;
	}
}

void VID_GetMilliseconds(uint32_t *ms) {
	*ms = g_system->getMillis();
}

void VID_SetTextInputMode(bool enabled) {
	textInput = enabled;
	g_system->setFeatureState(OSystem::kFeatureVirtualKeyboard, enabled);
}

void VID_OpenFileDialog(bool existing, char *buffer, size_t bufferSize) {
	// No native dialog: the interpreter falls back to its own text prompt
	(void)existing;
	if (buffer != NULL && bufferSize > 0)
		*buffer = 0;
}

void VID_WaitForKey() {
	// Unsupported
}

// -------------------------------------------------------------------- Audio

static void VID_StopSample() {
	if (g_engine != nullptr && g_engine->_mixer != nullptr)
		g_engine->_mixer->stopHandle(audioHandle);
	audioSource = NULL;
	audioSourceSize = 0;
}

static void VID_QueueSample(const void *data, uint32_t length, int hz, int bitsPerSample, int volume) {
	if (g_engine == nullptr || g_engine->_mixer == nullptr || data == NULL || length == 0)
		return;

	byte *copy = (byte *)malloc(length);
	if (copy == NULL)
		return;
	memcpy(copy, data, length);

	// 8 bit DMG samples are unsigned, 16 bit ones are signed little endian
	// (see the mixing loop in the reference SDL backend)
	byte flags = (bitsPerSample == 16) ? (Audio::FLAG_16BITS | Audio::FLAG_LITTLE_ENDIAN)
	                                   : Audio::FLAG_UNSIGNED;

	Audio::SeekableAudioStream *stream = Audio::makeRawStream(copy, length, hz, flags,
		DisposeAfterUse::YES);
	if (stream == NULL) {
		free(copy);
		return;
	}

	g_engine->_mixer->stopHandle(audioHandle);
	g_engine->_mixer->playStream(Audio::Mixer::kSFXSoundType, &audioHandle, stream, -1,
		volume >= 256 ? Audio::Mixer::kMaxChannelVolume
		              : (volume * Audio::Mixer::kMaxChannelVolume) / 256);
}

void VID_PlaySampleBuffer(void *buffer, int samples, int hz, int v) {
	audioSource = buffer;
	audioSourceSize = (uint32_t)samples;
	VID_QueueSample(buffer, (uint32_t)samples, hz, 8, v);
}

void VID_StopSampleIfOverlaps(const void *buffer, uint32_t size) {
	// The sample data was copied when it was queued, so the only thing that
	// matters is whether the buffer the caller is about to free is the one
	// that is still playing.
	if (audioSource == NULL)
		return;

	size_t start = (size_t)buffer;
	size_t end = start + size;
	if ((size_t)audioSource < end && (size_t)audioSource + audioSourceSize > start)
		VID_StopSample();
}

void VID_PlaySample(uint8_t picno, int *duration) {
	DMG_Entry *entry = DMG_GetEntry(dmg, picno);
	if (entry == NULL || entry->type != DMGEntry_Audio)
		return;

	uint8_t *sourceData = DMG_GetEntryData(dmg, picno, ImageMode_Audio);
	if (sourceData == 0)
		return;

	int bitsPerSample = entry->bitDepth == 16 ? 16 : 8;
	int inputHz;
	switch (entry->x) {
	case DMG_5KHZ:    inputHz =  5000; break;
	case DMG_7KHZ:    inputHz =  7000; break;
	case DMG_9_5KHZ:  inputHz =  9500; break;
	case DMG_15KHZ:   inputHz = 15000; break;
	case DMG_20KHZ:   inputHz = 20000; break;
	case DMG_30KHZ:   inputHz = 30000; break;
	case DMG_44_1KHZ: inputHz = 44100; break;
	case DMG_48KHZ:   inputHz = 48000; break;
	default:          inputHz = 11025; break;
	}

	audioSource = sourceData;
	audioSourceSize = entry->length;
	VID_QueueSample(sourceData, entry->length, inputHz, bitsPerSample, 256);

	if (duration != NULL)
		*duration = (entry->length / (bitsPerSample == 16 ? 2 : 1)) * 1000 / inputHz;
}

// ---------------------------------------------------------------- Main loop

void VID_InnerLoop() {
	static uint32 ticks = 0;

	uint32 now = g_system->getMillis();
	if (ticks == 0)
		ticks = now;
	int elapsed = now - ticks;
	ticks = now;

	VID_PumpEvents();

	if (mainLoopCallback)
		mainLoopCallback(elapsed);

	VID_Present();
}

void VID_MainLoopAsync(DDB_Interpreter *i, void (*callback)(int elapsed)) {
	mainLoopInterpreter = i;
	mainLoopCallback = callback;
	quit = false;
}

void VID_MainLoop(DDB_Interpreter *i, void (*callback)(int elapsed)) {
	void (*previousCallback)(int elapsed) = mainLoopCallback;
	DDB_Interpreter *previousInterpreter = mainLoopInterpreter;

	mainLoopInterpreter = i;
	mainLoopCallback = callback;
	quit = false;

	uint32 ticks = g_system->getMillis();
	while (!quit) {
		// Delay for smooth scrolling
		uint32 now = g_system->getMillis();
		if (buffering || (mainLoopInterpreter != NULL &&
			(mainLoopInterpreter->state == DDB_VSYNC ||
			// INKEY polls cost one frame on the original interpreters; without
			// this, INKEY-driven game loops (the Espacial arcade) run at CPU
			// speed instead of frame rate
			 mainLoopInterpreter->state == DDB_CHECKING_KEY))) {
			if (now - ticks < 20)
				g_system->delayMillis(now + 20 - ticks);
		}
		ticks = now;

		VID_InnerLoop();
	}
	quit = false;

	// The loader nests loops (VID_MainLoop for the disk prompt while the game
	// loop is running), so restore whatever was driving before us.
	mainLoopCallback = previousCallback;
	mainLoopInterpreter = previousInterpreter;
}

void VID_Quit() {
	quit = true;
}

void VID_ActivatePalette() {
	// The palette is uploaded as part of every presented frame
}

void VID_VSync() {
	VID_InnerLoop();
	g_system->delayMillis(16);
}

// ------------------------------------------------------------- Clipboard

#if HAS_CLIPBOARD
static uint8_t *clipboardText = NULL;
static uint32_t clipboardTextSize = 0;

bool VID_HasClipboardText(uint32_t *size) {
	if (clipboardText != NULL) {
		Free(clipboardText);
		clipboardText = NULL;
		clipboardTextSize = 0;
	}

	if (!g_system->hasTextInClipboard())
		return false;

	Common::String text = g_system->getTextFromClipboard().encode(Common::kISO8859_1);
	if (text.empty())
		return false;

	clipboardText = Allocate<uint8_t>("Clipboard text", text.size());
	if (clipboardText == NULL)
		return false;

	// The interpreter works in the DAAD character set, not in ISO-8859-1
	for (uint32 n = 0; n < text.size(); n++)
		clipboardText[n] = DDB_ISO2Char[(uint8)text[n]];
	clipboardTextSize = text.size();

	if (size != NULL)
		*size = clipboardTextSize;
	return true;
}

void VID_GetClipboardText(uint8_t *buffer, uint32_t bufferSize) {
	if (clipboardText == NULL || buffer == NULL)
		return;

	uint32_t count = bufferSize < clipboardTextSize ? bufferSize : clipboardTextSize;
	MemCopy(buffer, clipboardText, count);
}

void VID_SetClipboardText(uint8_t *buffer, uint32_t bufferSize) {
	if (buffer == NULL || bufferSize == 0)
		return;

	// DDB_Char2ISO only covers the 128 printable DAAD codes
	Common::String text;
	for (uint32_t n = 0; n < bufferSize && buffer[n] != 0; n++)
		text += (char)(buffer[n] < 128 ? DDB_Char2ISO[buffer[n]] : '?');

	g_system->setTextInClipboard(Common::U32String(text, Common::kISO8859_1));
}
#endif

// ------------------------------------------------------------ Data file

bool VID_LoadDataFile(const char *fileName) {
	// The game database now governs HAM decoding (see IsHAM6Display).
	scrHAM6 = false;

	#if HAS_XMSG
	DDB_InitializeXMessageCache(65536);
	#endif

	#if HAS_PCX
	FreeBufferedPCXPicture();
	VID_SetExternalPictureBase(0);
	#endif
	charset16Available = false;
	for (int n = 0; n < 256; n++)
		charWidth[n] = defaultCharWidth;

	if (dmg != NULL) {
		DMG_Close(dmg);
		dmg = NULL;
	}

	char resolvedDataFile[FILE_MAX_PATH];
	DDB_ScreenMode resolvedDataMode = screenMode;
	if (DDB_ResolveDataFile(fileName, screenMachine, screenMode, resolvedDataFile, sizeof(resolvedDataFile), &resolvedDataMode, 0)) {
		dmg = DMG_Open(resolvedDataFile, true);
		if (dmg != NULL && screenMachine == DDB_MACHINE_IBMPC)
			dmg->screenMode = resolvedDataMode;
		screenMode = resolvedDataMode;
	}
	if (dmg == NULL) {
		#if HAS_PCX
		VID_SetExternalPictureBase(fileName);
		if (!HasExternalPCXGraphics(fileName)) {
			VID_SetExternalPictureBase(0);
			DDB_SetError(DDB_ERROR_FILE_NOT_FOUND);
			return false;
		}

		if (!screen2XMode) {
			screenMode = ScreenMode_VGA;
			lineHeight = 8;
			columnWidth = 8;
			screenCellWidth = 8;
		}
		#else
		DDB_SetError(DDB_ERROR_FILE_NOT_FOUND);
		return false;
		#endif
	} else if (screenMachine == DDB_MACHINE_IBMPC) {
		ApplyIBMPCTextMetrics(
			dmg->version == DMG_Version5 &&
			dmg->targetWidth == 640 &&
			dmg->targetHeight == 400 &&
			(dmg->dat5Flags & DMG_DAT5_FLAG_2X) != 0);
	}

	// Only reset the charset once a data file is confirmed, so a snapshot's
	// custom charset is kept when no data file exists.
	memcpy(charset, DefaultCharset, 1024);
	memcpy(charset + 1024, DefaultCharset, 1024);
	charsetInitialized = true;

	bool fontLoaded = false;
	fontLoaded = SCR_LoadSINTACFont(ChangeExtension(fileName, ".FNT")) ||
		SCR_LoadSINTACFont(ChangeExtension(fileName, ".fnt"));

	if (!fontLoaded &&
		!SCR_LoadCharset(charset, ChangeExtension(fileName, ".CH0")) &&
		!SCR_LoadCharset(charset, ChangeExtension(fileName, ".ch0")) &&
		!SCR_LoadCharset(charset, ChangeExtension(fileName, ".CHR")) &&
		!SCR_LoadCharset(charset, ChangeExtension(fileName, ".chr"))) {
		// Keep the default fixed-width charset restored above.
	}
	charsetInitialized = true;

	if (dmg != NULL && screenMachine == DDB_MACHINE_IBMPC) {
		screenMode = (DDB_ScreenMode)dmg->screenMode;
		if (dmg->screenMode == ScreenMode_CGA) {
			memcpy(palette, CGAPaletteCyan, sizeof(CGAPaletteCyan));
			memcpy(DefaultPalette, palette, sizeof(DefaultPalette));
		} else if (dmg->screenMode == ScreenMode_EGA) {
			memcpy(palette, EGAPalette, sizeof(EGAPalette));
			memcpy(DefaultPalette, palette, sizeof(DefaultPalette));
		}
	}

	#if HAS_PSG
	if ((screenMachine == DDB_MACHINE_ATARIST || screenMachine == DDB_MACHINE_AMIGA) && !DDB_InitializePSGPlayback()) {
		if (dmg != NULL) {
			DMG_Close(dmg);
			dmg = NULL;
		}
		DDB_SetError(DDB_ERROR_OUT_OF_MEMORY);
		return false;
	}
	#endif

	return true;
}

// ------------------------------------------------------------ Init / teardown

static void VID_SetupScreen() {
	// Prefer a True Color mode
	// HAM6 pictures cannot be shown through a 256 entry palette,
	// and the conversion cost of one 320x200 frame is trivial.
	Common::List<Graphics::PixelFormat> formats = g_system->getSupportedFormats();
	initGraphics(screenWidth, screenHeight, formats);
	presentFormat = g_system->getScreenFormat();

	if (presentBuffer != NULL) {
		Free(presentBuffer);
		presentBuffer = NULL;
		presentBufferSize = 0;
	}

	if (presentFormat.bytesPerPixel != 1) {
		presentBufferSize = (uint32_t)screenWidth * screenHeight * presentFormat.bytesPerPixel;
		presentBuffer = Allocate<uint8_t>("Present buffer", presentBufferSize);
	}
}

bool VID_Initialize(DDB_Machine machine, DDB_Version version, DDB_ScreenMode mode) {
	videoInitialized = true;

	lineHeight  = 8;
	columnWidth = (version == DDB_VERSION_PAWS || machine == DDB_MACHINE_PCW) ? 8 : 6;
	defaultCharWidth = columnWidth;
	screenCellWidth = 8;
	screenMode = mode;
	screenVersion = version;
	xCoordMultiplier = 1;
	yCoordMultiplier = 1;
	screen2XMode = false;
	charset16Available = false;
	for (int n = 0; n < 256; n++)
		charWidth[n] = columnWidth;

	switch (machine) {
	case DDB_MACHINE_MSX:
		memcpy(DefaultPalette, MSXPalette, sizeof(MSXPalette));
		screenMachine = machine;
		screenWidth   = 256;
		screenHeight  = 192;
		bitmap        = Allocate<uint8_t>("MSX Screen Data", 256 * 192 / 8);
		attributes    = Allocate<uint8_t>("MSX Attributes", 32 * 24);
		stride        = 32;
		break;
	case DDB_MACHINE_SPECTRUM:
		memcpy(DefaultPalette, ZXSpectrumPalette, sizeof(ZXSpectrumPalette));
		screenMachine = machine;
		screenWidth   = 256;
		screenHeight  = 192;
		bitmap        = Allocate<uint8_t>("Spectrum Screen Data", 256 * 192 / 8);
		attributes    = Allocate<uint8_t>("Spectrum Attributes", 32 * 24);
		stride        = 32;
		break;
	case DDB_MACHINE_CPC:
		memcpy(DefaultPalette, CPCPalette, sizeof(CPCPalette));
		screenMachine = machine;
		screenWidth   = 320;
		screenHeight  = 200;
		columnWidth   = 8;
		defaultCharWidth = 8;
		for (int n = 0; n < 256; n++)
			charWidth[n] = 8;
		break;
	case DDB_MACHINE_C64:
		memcpy(DefaultPalette, Commodore64Palette, sizeof(Commodore64Palette));
		screenMachine = machine;
		screenWidth   = 320;
		screenHeight  = 200;
		bitmap        = Allocate<uint8_t>("C64 Screen Data", 320 * 200 / 8);
		attributes    = Allocate<uint8_t>("C64 Attributes", 40 * 25);
		stride        = 40;
		columnWidth   = 8;
		defaultCharWidth = 8;
		for (int n = 0; n < 256; n++)
			charWidth[n] = 8;
		break;
	case DDB_MACHINE_PCW:
		screenMachine = machine;
		screenWidth   = 720;
		screenHeight  = 256;
		stride        = 40;
		columnWidth   = 8;
		defaultCharWidth = 8;
		memcpy(DefaultPalette, PCWDefaultPalette, sizeof(PCWDefaultPalette));
		for (int n = 0; n < 256; n++)
			charWidth[n] = 8;
		break;
	default:
		memcpy(DefaultPalette, EGAPalette, sizeof(EGAPalette));
		screenMachine = DDB_MACHINE_IBMPC;
		switch (mode) {
		case ScreenMode_HiRes:
			screenWidth  = 640;
			screenHeight = 200;
			xCoordMultiplier = 2;
			break;
		case ScreenMode_SHiRes:
			screenWidth  = 640;
			screenHeight = 400;
			xCoordMultiplier = 2;
			yCoordMultiplier = 2;
			break;
		default:
			screenWidth  = 320;
			screenHeight = 200;
			break;
		}
		ApplyIBMPCTextMetrics(mode == ScreenMode_SHiRes);
		break;
	}

	for (int n = 0; n < 256; n++)
		charWidth[n] = defaultCharWidth;

	memcpy(palette, DefaultPalette, sizeof(DefaultPalette));
	VID_RegisterScreenAdapter();

	if (frontBuffer != NULL) {
		Free(frontBuffer);
		frontBuffer = NULL;
	}
	if (backBuffer != NULL) {
		Free(backBuffer);
		backBuffer = NULL;
	}

	frontBuffer = Allocate<uint8_t>("Graphics front buffer", screenWidth * screenHeight);
	backBuffer  = Allocate<uint8_t>("Graphics back buffer", screenWidth * screenHeight);
	if (frontBuffer == NULL || backBuffer == NULL) {
		DDB_SetError(DDB_ERROR_OUT_OF_MEMORY);
		return false;
	}
	memset(frontBuffer, 0, screenWidth * screenHeight);
	memset(backBuffer, 0, screenWidth * screenHeight);

	textBuffer = frontBuffer;
	graphicsBuffer = frontBuffer;

	if (charsetInitialized == false) {
		memcpy(charset, DefaultCharset, 1024);
		memcpy(charset + 1024, DefaultCharset, 1024);
		charsetInitialized = true;
	}

	// Must go through initGraphics(); OSystem::initSize() asserts outside of a
	// graphics transaction.
	VID_SetupScreen();
	return true;
}

uint32_t VID_GetSupportedDataFileModes() {
	// Everything the desktop player supports; the buffer is machine sized and
	// paletted either way.
	return DDB_DataFileMode_CGA | DDB_DataFileMode_EGA | DDB_DataFileMode_VGA16 |
		DDB_DataFileMode_VGA | DDB_DataFileMode_HiRes | DDB_DataFileMode_SHiRes;
}

void VID_Finish() {
	if (videoInitialized)
		videoInitialized = false;

	VID_StopSample();
	VID_ScreenRegisterAdapter(0);

	if (frontBuffer != NULL) {
		Free(frontBuffer);
		frontBuffer = NULL;
	}
	if (backBuffer != NULL) {
		Free(backBuffer);
		backBuffer = NULL;
	}
	if (screenBackupBuffer != NULL) {
		Free(screenBackupBuffer);
		screenBackupBuffer = NULL;
	}
	if (presentBuffer != NULL) {
		Free(presentBuffer);
		presentBuffer = NULL;
		presentBufferSize = 0;
	}
	if (audioData != NULL) {
		Free(audioData);
		audioData = NULL;
		audioDataSize = 0;
	}
	#if HAS_PCX
	FreeBufferedPCXPicture();
	#endif
	#if HAS_CLIPBOARD
	if (clipboardText != NULL) {
		Free(clipboardText);
		clipboardText = NULL;
		clipboardTextSize = 0;
	}
	#endif

	quit = true;
	exitGame = true;
}

void VID_SaveDebugBitmap() {
	// Debug only helper, not supported here
}

bool VID_SaveGraphicsSnapshot(const char *fileName) {
	(void)fileName;
	return false;
}
