#pragma once

#include <ddb_vid.h>
#include <dmg.h>

struct VID_ScreenAdapterInfo
{
	uint16_t width;
	uint16_t height;
	uint8_t cellWidth;
	uint8_t cellHeight;
	uint8_t colorDepth;
	uint16_t paletteSize;
	DMG_ImageMode nativeImageMode;
	uint8_t alignmentPixels;
};

struct VID_ScreenAdapterOps
{
	void (*clear)(int x, int y, int w, int h, uint8_t color, VID_ClearMode mode);
	void (*scroll)(int x, int y, int w, int h, int lines, uint8_t paper);
	void (*drawTextSpan)(int x, int y, const uint8_t* text, uint16_t length, uint8_t ink, uint8_t paper);
	void (*blitNativeImage)(const uint8_t* pixels, int srcW, int srcH, int x, int y, int w, int h);
	void (*blitIndexedImage)(const uint8_t* pixels, int srcW, int x, int y, int w, int h);
	void (*clearBuffer)(bool front);
	void (*saveScreen)();
	void (*restoreScreen)();
	void (*setTarget)(SCR_Operation op, bool front);
	void (*swapScreen)();
};

struct VID_ScreenAdapter
{
	VID_ScreenAdapterInfo info;
	VID_ScreenAdapterOps ops;
};

void VID_ScreenRegisterAdapter(const VID_ScreenAdapter* adapter);
const VID_ScreenAdapter* VID_ScreenGetAdapter();
const VID_ScreenAdapterInfo* VID_ScreenGetInfo();
uint32_t VID_ScreenGetNativeImageSize(int width, int height);
bool VID_ScreenRequireAlignedX(int x);

void VID_ScreenClear(int x, int y, int w, int h, uint8_t color, VID_ClearMode mode);
void VID_ScreenScroll(int x, int y, int w, int h, int lines, uint8_t paper);
void VID_ScreenDrawTextSpan(int x, int y, const uint8_t* text, uint16_t length, uint8_t ink, uint8_t paper);
void VID_ScreenBlitNativeImage(const uint8_t* pixels, int srcW, int srcH, int x, int y, int w, int h);
void VID_ScreenBlitIndexedImage(const uint8_t* pixels, int srcW, int x, int y, int w, int h);
void VID_ScreenClearBuffer(bool front);
void VID_ScreenSaveScreen();
void VID_ScreenRestoreScreen();
void VID_ScreenSetTarget(SCR_Operation op, bool front);
void VID_ScreenSwapScreen();
