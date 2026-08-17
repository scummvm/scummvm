#include <ddb_vid.h>
#include <ddb_pal.h>
#include <vid_screen.h>
#include <os_file.h>
#include <os_lib.h>
#include <dmg.h>

#if defined(_AMIGA) || defined(_ATARIST)
extern void VID_PresentDefaultScreen();
extern void VID_SetPaletteRangeFast(const uint32_t* palette, uint16_t count, uint16_t firstColor, bool clearOutside, bool waitForVBlank);
#endif

bool		waitingForKey = false;
bool 		buffering = false;
#if HAS_TESTMODE
static bool fastMode = false;
#endif
uint8_t 	lineHeight;
uint8_t 	columnWidth;
uint8_t 	screenCellWidth = 8;
uint16_t	screenWidth;
uint16_t	screenHeight;
uint8_t 	charWidth[256];

// Releases the graphics database and its underlying file handle. Used before
// prompting for a physical disk change, so no open file remains bound to the
// outgoing volume; the next part load reopens its database normally.
void VID_CloseDataFile()
{
	if (dmg != 0)
	{
		DMG_Close(dmg);
		dmg = 0;
	}
}

#if HAS_TESTMODE
void VID_SetFastMode(bool enabled)
{
	fastMode = enabled;
}

bool VID_IsFastMode()
{
	return fastMode;
}
#endif

static const VID_ScreenAdapter* activeScreenAdapter = 0;

void VID_ScreenRegisterAdapter(const VID_ScreenAdapter* adapter)
{
	activeScreenAdapter = adapter;
}

const VID_ScreenAdapter* VID_ScreenGetAdapter()
{
	return activeScreenAdapter;
}

const VID_ScreenAdapterInfo* VID_ScreenGetInfo()
{
	return activeScreenAdapter != 0 ? &activeScreenAdapter->info : 0;
}

uint32_t VID_ScreenGetNativeImageSize(int width, int height)
{
	const VID_ScreenAdapterInfo* info = VID_ScreenGetInfo();
	if (info == 0 || width <= 0 || height <= 0)
		return 0;

	switch (info->nativeImageMode)
	{
		case ImageMode_IndexedX:
			return ((uint32_t)(width + 3) & ~3u) * (uint32_t)height;

		case ImageMode_CGA:
			return (((uint32_t)width + 3u) >> 2) * (uint32_t)height;

		case ImageMode_Indexed:
			return (uint32_t)width * (uint32_t)height;

		case ImageMode_Packed:
			return ((uint32_t)width * (uint32_t)height + 1u) >> 1;

		case ImageMode_PlanarST:
		case ImageMode_Planar:
		case ImageMode_PlanarFalcon:
			return (((uint32_t)(width + 15) & ~15u) >> 3) * (uint32_t)height * info->colorDepth;

		default:
			return 0;
	}
}

bool VID_ScreenRequireAlignedX(int x)
{
	const VID_ScreenAdapterInfo* info = VID_ScreenGetInfo();
	uint8_t alignment = info != 0 ? info->alignmentPixels : 1;
	if (alignment <= 1 || (x % alignment) == 0)
		return true;

	DebugPrintf("FATAL: native picture draw requires %u-pixel aligned X, got %d\n", (unsigned)alignment, x);
	Abort();
	return false;
}

void VID_ScreenClear(int x, int y, int w, int h, uint8_t color, VID_ClearMode mode)
{
	if (activeScreenAdapter != 0 && activeScreenAdapter->ops.clear != 0)
		activeScreenAdapter->ops.clear(x, y, w, h, color, mode);
}

void VID_ScreenScroll(int x, int y, int w, int h, int lines, uint8_t paper)
{
	if (activeScreenAdapter != 0 && activeScreenAdapter->ops.scroll != 0)
		activeScreenAdapter->ops.scroll(x, y, w, h, lines, paper);
}

void VID_ScreenDrawTextSpan(int x, int y, const uint8_t* text, uint16_t length, uint8_t ink, uint8_t paper)
{
	if (activeScreenAdapter != 0 && activeScreenAdapter->ops.drawTextSpan != 0)
		activeScreenAdapter->ops.drawTextSpan(x, y, text, length, ink, paper);
}

void VID_ScreenBlitNativeImage(const uint8_t* pixels, int srcW, int srcH, int x, int y, int w, int h)
{
	if (activeScreenAdapter != 0 && activeScreenAdapter->ops.blitNativeImage != 0)
		activeScreenAdapter->ops.blitNativeImage(pixels, srcW, srcH, x, y, w, h);
}

void VID_ScreenBlitIndexedImage(const uint8_t* pixels, int srcW, int x, int y, int w, int h)
{
	if (activeScreenAdapter != 0 && activeScreenAdapter->ops.blitIndexedImage != 0)
		activeScreenAdapter->ops.blitIndexedImage(pixels, srcW, x, y, w, h);
}

void VID_ScreenClearBuffer(bool front)
{
	if (activeScreenAdapter != 0 && activeScreenAdapter->ops.clearBuffer != 0)
		activeScreenAdapter->ops.clearBuffer(front);
}

void VID_ScreenSaveScreen()
{
	if (activeScreenAdapter != 0 && activeScreenAdapter->ops.saveScreen != 0)
		activeScreenAdapter->ops.saveScreen();
}

void VID_ScreenRestoreScreen()
{
	if (activeScreenAdapter != 0 && activeScreenAdapter->ops.restoreScreen != 0)
		activeScreenAdapter->ops.restoreScreen();
}

void VID_ScreenSetTarget(SCR_Operation op, bool front)
{
	if (activeScreenAdapter != 0 && activeScreenAdapter->ops.setTarget != 0)
		activeScreenAdapter->ops.setTarget(op, front);
}

void VID_ScreenSwapScreen()
{
	if (activeScreenAdapter != 0 && activeScreenAdapter->ops.swapScreen != 0)
		activeScreenAdapter->ops.swapScreen();
}

void VID_ResetDisplay()
{
#ifdef _AMIGA
	VID_PresentDefaultScreen();
#else
	VID_Clear(0, 0, screenWidth, screenHeight, 0, Clear_All);
	VID_ClearBuffer(true);
	VID_ClearBuffer(false);
	VID_SetDefaultPalette();
	VID_ActivatePalette();
#endif
}

#if !defined(_AMIGA) && !defined(_ATARIST)
void VID_SetPaletteEntries(const uint32_t* palette, uint16_t count, uint16_t firstColor, bool clearOutside, bool waitForVBlank)
{
	uint16_t paletteSize = VID_GetPaletteSize();
	if (firstColor > paletteSize)
		return;
	if (count > paletteSize - firstColor)
		count = paletteSize - firstColor;

	if (clearOutside)
	{
		for (uint16_t i = 0; i < firstColor; i++)
			VID_SetPaletteColor(i, 0, 0, 0);
		for (uint16_t i = firstColor + count; i < paletteSize; i++)
			VID_SetPaletteColor(i, 0, 0, 0);
	}

	for (uint16_t i = 0; i < count; i++)
	{
		uint32_t color = palette[i];
		VID_SetPaletteColor(firstColor + i,
			(color >> 16) & 0xFF,
			(color >> 8) & 0xFF,
			color & 0xFF);
	}

	if (waitForVBlank)
		VID_VSync();
	VID_ActivatePalette();
}
#endif

#ifdef _ATARIST
void VID_SetPaletteEntries(const uint32_t* palette, uint16_t count, uint16_t firstColor, bool clearOutside, bool waitForVBlank)
{
	VID_SetPaletteRangeFast(palette, count, firstColor, clearOutside, waitForVBlank);
}
#endif

void VID_SetPaletteRange(const uint32_t* palette, uint16_t count, uint16_t firstColor, bool clearOutside, bool waitForVBlank)
{
#if defined(_AMIGA) || defined(_ATARIST)
	VID_SetPaletteRangeFast(palette, count, firstColor, clearOutside, waitForVBlank);
#else
	VID_SetPaletteEntries(palette, count, firstColor, clearOutside, waitForVBlank);
#endif
}

#if HAS_PCX
static char externalPictureBase[FILE_MAX_PATH];

static bool FileExists(const char* fileName)
{
	File* file = File_Open(fileName, ReadOnly);
	if (file == 0)
		return false;
	File_Close(file);
	return true;
}

static const char* GetLastPathSeparator(const char* fileName)
{
	const char* slash = StrRChr(fileName, '/');
	const char* backslash = StrRChr(fileName, '\\');
	if (slash == 0)
		return backslash;
	if (backslash == 0)
		return slash;
	return slash > backslash ? slash : backslash;
}

static void BuildExternalPictureFileName(const char* fileName, uint8_t picno, const char* extension, char* output, size_t outputSize)
{
	if (output == 0 || outputSize == 0)
		return;

	output[0] = 0;
	if (fileName == 0 || fileName[0] == 0)
		return;

	const char* separator = GetLastPathSeparator(fileName);
	size_t prefixLength = separator == 0 ? 0 : (size_t)(separator - fileName + 1);
	if (prefixLength >= outputSize)
		prefixLength = outputSize - 1;

	if (prefixLength > 0)
	{
		MemCopy(output, fileName, prefixLength);
		output[prefixLength] = 0;
	}

	if (prefixLength + 4 >= outputSize)
		return;

	output[prefixLength + 0] = '0' + (picno / 100);
	output[prefixLength + 1] = '0' + ((picno / 10) % 10);
	output[prefixLength + 2] = '0' + (picno % 10);
	output[prefixLength + 3] = 0;
	StrCat(output, outputSize, extension);
}

void VID_SetExternalPictureBase(const char* fileName)
{
	if (fileName == 0)
		externalPictureBase[0] = 0;
	else
		StrCopy(externalPictureBase, sizeof(externalPictureBase), fileName);
}

bool VID_GetExternalPictureFileName(uint8_t picno, char* fileName, size_t fileNameSize)
{
	if (externalPictureBase[0] == 0 || fileName == 0 || fileNameSize == 0)
		return false;

	#if HAS_SPECTRUM
	if (screenMachine == DDB_MACHINE_SPECTRUM)
	{
		BuildExternalPictureFileName(externalPictureBase, picno, ".ZXS", fileName, fileNameSize);
		if (FileExists(fileName))
			return true;

		BuildExternalPictureFileName(externalPictureBase, picno, ".zxs", fileName, fileNameSize);
		if (FileExists(fileName))
			return true;
	}
	#endif

	BuildExternalPictureFileName(externalPictureBase, picno, ".VGA", fileName, fileNameSize);
	if (FileExists(fileName))
		return true;

	BuildExternalPictureFileName(externalPictureBase, picno, ".vga", fileName, fileNameSize);
	if (FileExists(fileName))
		return true;

	BuildExternalPictureFileName(externalPictureBase, picno, ".PCX", fileName, fileNameSize);
	if (FileExists(fileName))
		return true;

	BuildExternalPictureFileName(externalPictureBase, picno, ".pcx", fileName, fileNameSize);
	if (FileExists(fileName))
		return true;

	fileName[0] = 0;
	return false;
}

bool VID_HasExternalPictures()
{
	return externalPictureBase[0] != 0;
}
#endif

bool VID_PictureExists (uint8_t picno)
{
	#if HAS_PCX
	if (dmg == 0)
	{
		char fileName[FILE_MAX_PATH];
		return VID_GetExternalPictureFileName(picno, fileName, sizeof(fileName));
	}
	#endif
	
	DMG_Entry* entry = DMG_GetEntry(dmg, picno);
	return (entry != 0 && entry->type == DMGEntry_Image);
}

bool VID_SampleExists (uint8_t picno)
{
	if (dmg == 0) return false;

	DMG_Entry* entry = DMG_GetEntry(dmg, picno);
	return (entry != 0 && entry->type == DMGEntry_Audio);
}

void VID_SetDefaultPalette()
{
	for (int n = 0; n < 16; n++)
	{
		uint32_t color = DefaultPalette[n];
		VID_SetPaletteColor(n, (color >> 16) & 0xff, (color >> 8) & 0xff, color & 0xff);
	}
}

void VID_ShowError(const char* msg)
{
	if (msg == 0)
		return;

	int bg = 0, cr = 0;
	for (int n = 0; n < 16; n++)
	{
		uint8_t r, g, b;
		VID_GetPaletteColor(n, &r, &g, &b);
		if (r > cr)
		{
			bg = n;
			cr = r;
		}
	}

	// It may be the case globals such as screenWidth has not been initialized yet

	int viewportWidth = screenWidth > 0 ? screenWidth : 320;
	int textHeight = lineHeight > 0 ? lineHeight : 8;
	int boxHeight = textHeight + 2;
	if (boxHeight < 10)
		boxHeight = 10;

	VID_Clear(0, 0, viewportWidth, boxHeight, bg);

	int textWidth = 0;
	for (int n = 0; msg[n] != 0; n++)
	{
		uint8_t ch = (uint8_t)msg[n];
		int width = charWidth[ch];
		if (width <= 0)
			width = columnWidth > 0 ? columnWidth : 6;
		textWidth += width;
	}

	int x = (viewportWidth - textWidth) / 2;
	if (x < 0)
		x = 0;

	for (int n = 0; msg[n] != 0; n++)
	{
		uint8_t ch = (uint8_t)msg[n];
		int width = charWidth[ch];
		if (width <= 0)
			width = columnWidth > 0 ? columnWidth : 6;
		VID_DrawCharacter(x, 1, ch, 0, 255);
		x += width;
	}

	VID_WaitForKey();
}

static bool progressBarVisible = false;
static bool progressBarEnabled = true;
static bool fadeEnabled = true;

void VID_SetFadeEnabled(bool enabled)
{
	fadeEnabled = enabled;
}

bool VID_IsFadeEnabled()
{
	return fadeEnabled;
}

void VID_SetProgressBarEnabled(bool enabled)
{
	progressBarEnabled = enabled;
	if (!enabled)
		progressBarVisible = false;
}

void VID_ShowProgressBar(uint16_t amount)
{
	#if !HAS_PROGRESS_BAR
	(void)amount;
	return;
	#else
	if (!progressBarEnabled)
		return;
	#ifdef _AMIGA
	const VID_ClearMode clearMode = Clear_All;
	#else
	const VID_ClearMode clearMode = Clear_Text;
	#endif
	uint16_t x = screenWidth/2 - 32;
	uint16_t y = screenHeight - 32;
	
	if (!progressBarVisible)
	{
		VID_Clear(x, y, 64, 1, 15, clearMode);
		VID_Clear(x, y, 1, 12, 15, clearMode);
		VID_Clear(x+63, y, 1, 12, 15, clearMode);
		VID_Clear(x, y+11, 64, 1, 15, clearMode);
		VID_Clear(x+1, y+1, 62, 10, 0, clearMode);
		progressBarVisible = true;
	}
	VID_Clear(x+2, y+2, amount*60/255, 8, 15, clearMode);
	#endif
}

#if !defined(HAS_DRAWTEXT) && !defined(_AMIGA) && !defined(_DOS)

void VID_DrawTextSpan(int x, int y, const uint8_t* text, uint16_t length, uint8_t ink, uint8_t paper)
{
	for (uint16_t n = 0; n < length && x < screenWidth; n++)
	{
		uint8_t ch = text[n];
		VID_DrawCharacter(x, y, ch, ink, paper);
		x += charWidth[ch];
	}
}

void VID_DrawText(int x, int y, const char* text, uint8_t ink, uint8_t paper)
{
	VID_DrawTextSpan(x, y, (const uint8_t*)text, (uint16_t)StrLen(text), ink, paper);
}

#endif
