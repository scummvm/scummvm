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

#ifndef XPLIB_XPLIB_H
#define XPLIB_XPLIB_H

#include "bolt/bolt.h"

namespace Bolt {

struct DisplaySpecs;
class BoltEngine;

typedef struct XPCycleSpec {
	short startIndex; // +0x00: first palette index
	short endIndex;   // +0x02: last palette index
	short delay;      // +0x04: cycle period in ms
} XPCycleSpec;        // 6 bytes

typedef struct XPSurface {
	byte *pixelData;     // +0x00: pixel buffer (width * height bytes, 8bpp)
	int16 width;         // +0x04: surface width
	int16 height;        // +0x06: surface height
	byte *palette;       // +0x08: RGB palette data (127 * 3 = 381 bytes)
	int16 paletteStart;  // +0x0C: first palette index (1)
	int16 paletteCount;  // +0x0E: number of palette entries (127)
	int16 flags;         // +0x10
	long overlayData;    // +0x12
	char _pad[0x10];     // +0x16..+0x23 (unknown)
	int16 clipX;         // +0x24
	int16 clipY;         // +0x26
} XPSurface;             // total: 0x28 = 40 bytes

typedef struct XPPicDesc {
	byte *pixelData;     // +0x00: pixel buffer (8bpp)
	int16 width;         // +0x04: image width
	int16 height;        // +0x06: image height
	byte *palette;       // +0x08: RGB palette data
	int16 paletteStart;  // +0x0C: first palette index
	int16 paletteCount;  // +0x0E: number of palette entries
	int16 flags;         // +0x10: bit 0 = transparent, bit 1 = RLE
} XPPicDesc;             // 0x12 = 18 bytes

typedef void (*BlitFunc)(void *src, int16 srcStride, void *dest, int16 destStride, int16 width, int16 height);

typedef struct ClipRect {
	short left;
	short top;
	short right;
	short bottom;
} ClipRect;

class XpLib {
public:
	XpLib(BoltEngine *bolt);
	~XpLib();

	void initialize();
	void terminate();

	// Blit
	void blit(void *src, uint16 srcStride, void *dst, uint16 dstStride, uint16 width, uint16 height);
	void maskBlit(void *src, uint16 srcStride, void *dst, uint16 dstStride, uint16 width, uint16 height);

	// Palette
	void getPalette(uint16 startIndex, uint16 count, void *destBuf);
	void setPalette(uint16 count, uint16 startIndex, void *srcBuf);
	bool startCycle(XPCycleSpec *specs);
	void cycleColors();
	void stopCycle();
	void setScreenBrightness(uint8 percent);

	// Cursor
	bool readCursor(uint16 *outButtons, int16 *outX, int16 *outY);
	void setCursorPos(int16 x, int16 y);
	void setCursorImage(void *bitmap, int16 hotspotX, int16 hotspotY);
	void setCursorColor(byte r, byte g, byte b);
	bool showCursor();
	void hideCursor();
	void updateCursorPosition();

	// Events
	int16 getEvent(int16 filter, uint32 *outData);
	int16 peekEvent(int16 filter, uint32 *outData);
	void postEvent(int16 type, uint32 data);
	int16 setInactivityTimer(int16 seconds);
	int16 setScreenSaverTimer(int16 time);
	bool enableController();
	void disableController();

	// Display
	bool chooseDisplaySpec(int *outMode, int numSpecs, DisplaySpecs *specs);
	void setCoordSpec(int16 x, int16 y, int16 width, int16 height);
	void displayPic(XPPicDesc *pic, int16 x, int16 y, int16 page);
	void setFrameRate(int16 fps);
	void updateDisplay();
	void setTransparency(bool toggle);
	void fillDisplay(byte color, int16 page);

	// Random
	int16 getRandom(int16 range);
	void randomize();

	// File
	int32 createFile(const char *fileName);
	void deleteFile(const char *fileName);
	int32 openFile(const char *fileName, short flags);
	void closeFile(int32 handle);
	bool readFile(int32 handle, void *buffer, uint32 *size);
	bool setFilePos(int32 handle, uint32 offset, int16 origin);
	void *allocMem(uint32 size);
	void *tryAllocMem(uint32 size);
	void freeMem(void *mem);

	// Sound
	void waveCb();
	bool playSound(void *data, uint32 size, int16 sampleRate);
	bool pauseSound();
	bool resumeSound();
	bool stopSound();

	// Timer
	int32 startTimer(int16 delay);
	void timeCb();
	bool killTimer(int32 timerId);

protected:
	BoltEngine *_bolt;

	// Blit
	void dirtyBlit(void *src, void *dst, void *dirtyFlags, uint16 width, uint16 height);
	uint32 compositeBlit(void *src, void *background, void *dst, uint16 stride, uint16 width, uint16 height);
	void rleBlit(void *src, void *dst, uint16 dstStride, uint16 width, uint16 height);
	void rleMaskBlit(void *src, void *dst, uint16 dstStride, uint16 width, uint16 height);
	uint32 rleCompositeBlit(void *rle, void *background, void *dst, uint16 width, uint16 height, void *dirtyFlags);
	uint16 rleDataSize(void *rleData, uint16 height);
	void markCursorPixels(void *buffer, uint32 count);

	// Cursor
	bool initCoords();
	void shutdownCoords();
	void readJoystick(int16 *outX, int16 *outY);

	// Events
	bool initWindow();
	void shutdownWindow();
	void unlinkEvent(int16 node);
	void enqueueEvent(int16 node);
	void pumpMessages();
	void handleTimer(uint32 timerId);
	void handleMouseMove(bool *mouseMoved);
	void handleMouseButton(int16 down, int16 button);
	void handleKey(int16 vkey, int16 down);
	void postJoystickEvent(int16 source, int16 dx, int16 dy);
	bool canDropEvent(int16 type);
	void activateScreenSaver();
	void resetInactivity();
	void enableMouse();
	void disableMouse();
	int16 getButtonState();

	// Display
	int16 switchDisplayMode(int16 mode);
	bool initDisplay();
	void shutdownDisplay();
	bool createSurface(XPSurface *surf);
	void freeSurface(XPSurface *surf);
	void virtualToScreen(int16 *x, int16 *y);
	void screenToVirtual(int16 *x, int16 *y);
	bool clipAndBlit(XPPicDesc *src, XPSurface *dest, int16 x, int16 y, ClipRect *outClip);
	void addDirtyRect(ClipRect *rect);
	void waitForFrameRate();
	void handlePaletteTransitions();
	void flushPalette();
	void overlayComposite();
	void compositeToScreen();
	void mergeDirtyRects();
	void blitDirtyRects(ClipRect *rects, int16 count);
	void compositeDirtyRects(ClipRect *rects, int16 count);
	void applyCursorPalette(bool enable);
	void prepareBackSurface();

	// File
	void fileError(const char *message);

	// Sound
	bool pollSound(void *outData);
	bool initSound();
	void shutdownSound();

	// Timer
	bool initTimer();
	void shutdownTimer();
};

} // End of namespace Bolt

#endif // XPLIB_XPLIB_H
