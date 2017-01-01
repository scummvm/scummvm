/* ScummVM - Graphic Adventure Engine
 *
 * ScummVM is the legal property of its developers, whose names
 * are too numerous to list here. Please refer to the COPYRIGHT
 * file distributed with this source distribution.
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License
 * as published by the Free Software Foundation; either version 2
 * of the License, or (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.
 *
 */

#ifndef AGI_GRAPHICS_H
#define AGI_GRAPHICS_H

#include "agi/font.h"

namespace Agi {

#define SCRIPT_WIDTH    160
#define SCRIPT_HEIGHT   168
#define VISUAL_WIDTH    160
#define VISUAL_HEIGHT   200
#define DISPLAY_DEFAULT_WIDTH   320
#define DISPLAY_DEFAULT_HEIGHT  200

enum GfxScreenUpscaledMode {
	DISPLAY_UPSCALED_DISABLED = 0,
	DISPLAY_UPSCALED_640x400  = 1
};

class AgiEngine;

enum GfxScreenMasks {
	GFX_SCREEN_MASK_VISUAL      = 1,
	GFX_SCREEN_MASK_PRIORITY    = 2,
	GFX_SCREEN_MASK_ALL         = GFX_SCREEN_MASK_VISUAL | GFX_SCREEN_MASK_PRIORITY
};

struct MouseCursorData {
	const byte *bitmapData;
	byte *bitmapDataAllocated;
	uint16 width;
	uint16 height;
	int hotspotX;
	int hotspotY;
};

class GfxMgr {
private:
	AgiBase *_vm;
	GfxFont *_font;

	uint8 _paletteGfxMode[256 * 3];
	uint8 _paletteTextMode[256 * 3];

	uint8 _agipalPalette[16 * 3];
	int _agipalFileNum;

public:
	GfxMgr(AgiBase *vm, GfxFont *font);

	int initVideo();
	int deinitVideo();
	void initPalette(uint8 *destPalette, const uint8 *paletteData, uint colorCount = 16, uint fromBits = 6, uint toBits = 8);
	void initPaletteCLUT(uint8 *destPalette, const uint16 *paletteCLUTData, uint colorCount = 16);
	void setAGIPal(int);
	int getAGIPalFileNum();
	void setPalette(bool GfxModePalette);

	void initMouseCursor(MouseCursorData *mouseCursor, const byte *bitmapData, uint16 width, uint16 height, int hotspotX, int hotspotY);
	void setMouseCursor(bool busy = false);

	void setRenderStartOffset(uint16 offsetY);
	uint16 getRenderStartDisplayOffsetY();

	void translateGamePosToDisplayScreen(int16 &x, int16 &y);
	void translateVisualPosToDisplayScreen(int16 &x, int16 &y);
	void translateDisplayPosToGameScreen(int16 &x, int16 &y);

	void translateVisualDimensionToDisplayScreen(int16 &width, int16 &height);
	void translateDisplayDimensionToVisualScreen(int16 &width, int16 &height);

	void translateGameRectToDisplayScreen(int16 &x, int16 &y, int16 &width, int16 &height);
	void translateVisualRectToDisplayScreen(int16 &x, int16 &y, int16 &width, int16 &height);
	void translateDisplayRectToVisualScreen(int16 &x, int16 &y, int16 &width, int16 &height);

	uint32 getDisplayOffsetToGameScreenPos(int16 x, int16 y);
	uint32 getDisplayOffsetToVisualScreenPos(int16 x, int16 y);

	void copyDisplayRectToScreen(int16 x, int16 y, int16 width, int16 height);
	void copyDisplayRectToScreen(int16 x, int16 adjX, int16 y, int16 adjY, int16 width, int16 adjWidth, int16 height, int16 adjHeight);
	void copyDisplayRectToScreenUsingGamePos(int16 x, int16 y, int16 width, int16 height);
	void copyDisplayRectToScreenUsingVisualPos(int16 x, int16 y, int16 width, int16 height);
	void copyDisplayToScreen();

	void translateFontPosToDisplayScreen(int16 &x, int16 &y);
	void translateDisplayPosToFontScreen(int16 &x, int16 &y);
	void translateFontDimensionToDisplayScreen(int16 &width, int16 &height);
	void translateFontRectToDisplayScreen(int16 &x, int16 &y, int16 &width, int16 &height);
	Common::Rect getFontRectForDisplayScreen(int16 column, int16 row, int16 width, int16 height);

private:
	uint _pixels;
	uint _displayPixels;

	byte *_activeScreen;
	byte *_gameScreen;     // 160x168 - screen, where the actual game content is drawn to (actual graphics, not including status line, prompt, etc.)
	byte *_priorityScreen; // 160x168 - screen contains priority information of the game screen
	// the term "visual screen" is effectively the display screen, but at 160x200 resolution. Used for coordinate translation
	byte *_displayScreen;  // 320x200 or 640x400 - screen, that the game is rendered to and which is then copied to framebuffer

	uint16 _displayScreenWidth;
	uint16 _displayScreenHeight;

	uint16 _displayFontWidth;
	uint16 _displayFontHeight;

	uint16 _displayWidthMulAdjust;
	uint16 _displayHeightMulAdjust;

	/**
	 * This variable defines, if upscaled hires is active and what upscaled mode
	 * is used.
	 */
	GfxScreenUpscaledMode _upscaledHires;

	bool  _priorityTableSet;
	uint8 _priorityTable[SCRIPT_HEIGHT]; /**< priority table */

	MouseCursorData _mouseCursor;
	MouseCursorData _mouseCursorBusy;

	uint16 _renderStartVisualOffsetY;
	uint16 _renderStartDisplayOffsetY;

public:
	uint16 getDisplayScreenWidth() {
		return _displayScreenWidth;
	}
	uint16 getDisplayFontWidth() {
		return _displayFontWidth;
	}
	uint16 getDisplayFontHeight() {
		return _displayFontHeight;
	}

	GfxScreenUpscaledMode getUpscaledHires() {
		return _upscaledHires;
	}

	void debugShowMap(int mapNr);

	void clear(byte color, byte priority);
	void clearDisplay(byte color, bool copyToScreen = true);
	void putPixel(int16 x, int16 y, byte drawMask, byte color, byte priority);
	void putPixelOnDisplay(int16 x, int16 y, byte color);
	void putPixelOnDisplay(int16 x, int16 adjX, int16 y, int16 adjY, byte color);
	void putFontPixelOnDisplay(int16 baseX, int16 baseY, int16 addX, int16 addY, byte color, bool isHires);

	byte getColor(int16 x, int16 y);
	byte getPriority(int16 x, int16 y);
	bool checkControlPixel(int16 x, int16 y, byte newPriority);

	byte getCGAMixtureColor(byte color);

	void render_Block(int16 x, int16 y, int16 width, int16 height, bool copyToScreen = true);
	bool render_Clip(int16 &x, int16 &y, int16 &width, int16 &height, int16 clipAgainstWidth = SCRIPT_WIDTH, int16 clipAgainstHeight = SCRIPT_HEIGHT);

private:
	void render_BlockEGA(int16 x, int16 y, int16 width, int16 height, bool copyToScreen);
	void render_BlockCGA(int16 x, int16 y, int16 width, int16 height, bool copyToScreen);
	void render_BlockHercules(int16 x, int16 y, int16 width, int16 height, bool copyToScreen);

public:
	void transition_Amiga();
	void transition_AtariSt();

	void block_save(int16 x, int16 y, int16 width, int16 height, byte *bufferPtr);
	void block_restore(int16 x, int16 y, int16 width, int16 height, byte *bufferPtr);

	void drawBox(int16 x, int16 y, int16 width, int16 height, byte backgroundColor, byte lineColor);
	void drawDisplayRect(int16 x, int16 y, int16 width, int16 height, byte color, bool copyToScreen = true);
	void drawDisplayRect(int16 x, int16 adjX, int16 y, int16 adjY, int16 width, int16 adjWidth, int16 height, int16 adjHeight, byte color, bool copyToScreen = true);
private:
	void drawDisplayRectEGA(int16 x, int16 y, int16 width, int16 height, byte color);
	void drawDisplayRectCGA(int16 x, int16 y, int16 width, int16 height, byte color);

public:
	void drawCharacter(int16 row, int16 column, byte character, byte foreground, byte background, bool disabledLook);
	void drawStringOnDisplay(int16 x, int16 y, const char *text, byte foreground, byte background);
	void drawStringOnDisplay(int16 x, int16 adjX, int16 y, int16 adjY, const char *text, byte foregroundColor, byte backgroundColor);
	void drawCharacterOnDisplay(int16 x, int16 y, byte character, byte foreground, byte background, byte transformXOR = 0, byte transformOR = 0);

	void shakeScreen(int16 repeatCount);
	void updateScreen();

	void initPriorityTable();
	void createDefaultPriorityTable(uint8 *priorityTable);
	void setPriorityTable(int16 priorityBase);
	bool saveLoadWasPriorityTableModified();
	int16 saveLoadGetPriority(int16 yPos);
	void saveLoadSetPriorityTableModifiedBool(bool wasModified);
	void saveLoadSetPriority(int16 yPos, int16 priority);
	void saveLoadFigureOutPriorityTableModifiedBool();

	int16 priorityToY(int16 priority);
	int16 priorityFromY(int16 yPos);
};

} // End of namespace Agi

#endif /* AGI_GRAPHICS_H */
