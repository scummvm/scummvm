/* Copyright (C) 1994-2003 Revolution Software Ltd
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
 * Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.
 *
 * $Header$
 */

#ifndef D_DRAW_H
#define D_DRAW_H

#include "sword2/driver/driver96.h"

namespace Sword2 {

// This is the maximum mouse cursor size in the SDL backend

#define MAX_MOUSE_W		80
#define MAX_MOUSE_H		80

#define RENDERAVERAGETOTAL	4

#define BLOCKWIDTH		64
#define BLOCKHEIGHT		64
#define MAXLAYERS		5

#define PALTABLESIZE		64 * 64 * 64

// Maximum scaled size of a sprite
#define SCALE_MAXWIDTH		512
#define SCALE_MAXHEIGHT		512

#if !defined(__GNUC__)
	#pragma START_PACK_STRUCTS
#endif

struct _mouseAnim {
	uint8 runTimeComp;	// type of runtime compression used for the
				// frame data
	uint8 noAnimFrames;	// number of frames in the anim
	int8 xHotSpot;		
	int8 yHotSpot;
	uint8 mousew;
	uint8 mouseh;
} GCC_PACK;

#if !defined(__GNUC__)
	#pragma END_PACK_STRUCTS
#endif

class MoviePlayer {
private:
	uint8 *_textSurface;
	void openTextObject(_movieTextObject *obj);
	void closeTextObject(_movieTextObject *obj);
	void drawTextObject(_movieTextObject *obj);

public:
	MoviePlayer() : _textSurface(NULL) {}
	int32 play(char *filename, _movieTextObject *text[], uint8 *musicOut);
};

typedef struct {
	byte data[BLOCKWIDTH * BLOCKHEIGHT];
	bool transparent;
} BlockSurface;

class Display {
	friend class MoviePlayer;

private:
	byte *_buffer;

	int32 _renderCaps;
	int8 _renderLevel;

	uint8 _menuStatus[2];
	byte *_icons[2][RDMENU_MAXPOCKETS];
	uint8 _pocketStatus[2][RDMENU_MAXPOCKETS];

	uint8 _iconCount;

	bool _needFullRedraw;

	uint8 _paletteMatch[PALTABLESIZE];

	uint8 _fadePalette[256][4];
	uint8 _fadeStatus;

	int32 _fadeStartTime;
	int32 _fadeTotalTime;

	byte _mouseData[MAX_MOUSE_W * MAX_MOUSE_H];

	uint8 _mouseFrame;
	uint8 *_mouseSprite;
	struct _mouseAnim *_mouseAnim;
	struct _mouseAnim *_luggageAnim;
	int32 *_mouseOffsets;
	int32 *_luggageOffset;

	// Scroll variables.  _scrollX and _scrollY hold the current scroll
	// position, and _scrollXTarget and _scrollYTarget are the target
	// position for the end of the game cycle.

	int16 _scrollX;
	int16 _scrollY;

	int16 _scrollXTarget;
	int16 _scrollYTarget;
	int16 _scrollXOld;
	int16 _scrollYOld;

	int16 _parallaxScrollX;	// current x offset to link a sprite to the
				// parallax layer
	int16 _parallaxScrollY;	// current y offset to link a sprite to the
				// parallax layer
	int16 _locationWide;
	int16 _locationDeep;

	uint16 _layer;

	int32 _initialTime;
	int32 _startTime;
	int32 _totalTime;
	int32 _renderAverageTime;
	int32 _framesPerGameCycle;
	bool _renderTooSlow;

	uint8 _xBlocks[MAXLAYERS];
	uint8 _yBlocks[MAXLAYERS];

	// An array of sub-blocks, one for each of the parallax layers.

	BlockSurface **_blockSurfaces[MAXLAYERS];

	uint16 _xScale[SCALE_MAXWIDTH];
	uint16 _yScale[SCALE_MAXHEIGHT];

	uint8 *_lightMask;

	void clearIconArea(int menu, int pocket, Common::Rect *r);

	void decompressMouse(uint8 *decomp, uint8 *comp, int width, int height, int pitch, int xOff = 0, int yOff = 0);

	uint8 getMatch(uint8 r, uint8 g, uint8 b);
	void fadeServer(void);

	void squashImage(byte *dst, uint16 dstPitch, uint16 dstWidth,
		uint16 dstHeight, byte *src, uint16 srcPitch, uint16 srcWidth,
		uint16 srcHeight, byte *backbuf);
	void stretchImage(byte *dst, uint16 dstPitch, uint16 dstWidth,
		uint16 dstHeight, byte *src, uint16 srcPitch, uint16 srcWidth,
		uint16 srcHeight, byte *backbuf);

	void updateRect(Common::Rect *r);

	void blitBlockSurface(BlockSurface *s, Common::Rect *r, Common::Rect *clip_rect);

	void mirrorSprite(uint8 *dst, uint8 *src, int16 w, int16 h);
	int32 decompressRLE256(uint8 *dest, uint8 *source, int32 decompSize);
	void unwindRaw16(uint8 *dest, uint8 *source, uint8 blockSize, uint8 *colTable);
	int32 decompressRLE16(uint8 *dest, uint8 *source, int32 decompSize, uint8 *colTable);

public:
	Display(int16 width, int16 height);

	// Game screen metrics
	int16 _screenWide;
	int16 _screenDeep;

	int16 _mouseX;
	int16 _mouseY;

	uint8 _palCopy[256][4];

	int8 getRenderLevel(void);
	void setRenderLevel(int8 level);

	void clearScene(void);

	void processMenu(void);
	int32 showMenu(uint8 menu);
	int32 hideMenu(uint8 menu);
	int32 setMenuIcon(uint8 menu, uint8 pocket, uint8 *icon);
	void closeMenuImmediately(void);

	void updateDisplay(void);
	void setWindowName(const char *windowName);
	void setNeedFullRedraw(void);

	void setPalette(int16 startEntry, int16 noEntries, uint8 *palette, uint8 setNow);
	void updatePaletteMatchTable(uint8 *data);
	uint8 quickMatch(uint8 r, uint8 g, uint8 b);
	int32 fadeUp(float time = 0.75);
	int32 fadeDown(float time = 0.75);
	uint8 getFadeStatus(void);
	void dimPalette(void);
	void waitForFade(void);

	int32 setMouseAnim(uint8 *ma, int32 size, int32 mouseFlash);
	int32 setLuggageAnim(uint8 *la, int32 size);
	int32 animateMouse(void);

	void drawMouse(void);

	void resetRenderEngine(void);

	void setScrollTarget(int16 sx, int16 sy);
	void initialiseRenderCycle(void);
	void startRenderCycle(void);
	bool endRenderCycle(void);
	void renderParallax(_parallax *p, int16 layer);
	void setLocationMetrics(uint16 w, uint16 h);
	int32 initialiseBackgroundLayer(_parallax *p);
	void closeBackgroundLayer(void);

	void plotPoint(uint16 x, uint16 y, uint8 colour);
	void drawLine(int16 x1, int16 y1, int16 x2, int16 y2, uint8 colour);

	int32 createSurface(_spriteInfo *s, uint8 **surface);
	void drawSurface(_spriteInfo *s, uint8 *surface, Common::Rect *clipRect = NULL);
	void deleteSurface(uint8 *surface);
	int32 drawSprite(_spriteInfo *s);
	int32 openLightMask(_spriteInfo *s);
	int32 closeLightMask(void);
};

} // End of namespace Sword2

#endif
