/* ScummVM - Scumm Interpreter
 * Copyright (C) 2003-2004 The ScummVM project
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License
 * as published by the Free Software Foundation; either version 2
 * of the License, or (at your option) any later version.

 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.	 See the
 * GNU General Public License for more details.

 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.
 *
 * $Header$
 *
 */

#ifndef QUEENDISPLAY_H
#define QUEENDISPLAY_H

#include "common/str.h"
#include "common/util.h"
#include "queen/defs.h"

namespace Queen {

class QueenEngine;

class Display {
public:

	Display(QueenEngine *vm, OSystem *system);
	~Display();

	void dynalumInit(const char *roomName, uint16 roomNum);
	void dynalumUpdate(int16 x, int16 y);

	void palConvert(uint8 *outPal, const uint8 *inPal, int start, int end);
	void palSet(const uint8 *pal, int start, int end, bool updateScreen = false);
	void palSetJoeDress();
	void palSetJoeNormal();
	void palSetPanel();
	void palFadeIn(int start, int end, uint16 roomNum, bool dynalum = false, int16 dynaX = 0, int16 dynaY = 0);
	void palFadeOut(int start, int end, uint16 roomNum);
	void palFadePanel();
	void palScroll(int start, int end);
	void palCustomColors(uint16 roomNum);
	void palCustomScroll(uint16 roomNum);
	void palCustomFlash();
	void palCustomLightsOff(uint16 roomNum);
	void palCustomLightsOn(uint16 roomNum);
	void palSetAllDirty() { _pal.dirtyMin = 0; _pal.dirtyMax = 255; }

	void screenMode(int comPanel, bool inCutaway);

	void prepareUpdate();
	void update(bool dynalum = false, int16 dynaX = 0, int16 dynaY = 0);

	void setupPanel();
	void setupNewRoom(const char *name, uint16 room);

	void drawBobSprite(const uint8 *data, uint16 x, uint16 y, uint16 w, uint16 h, uint16 pitch, bool xflip);
	void drawBobPasteDown(const uint8 *data, uint16 x, uint16 y, uint16 w, uint16 h);
	void drawInventoryItem(const uint8 *data, uint16 x, uint16 y, uint16 w, uint16 h);

	void blit(uint8 *dstBuf, uint16 dstPitch, uint16 x, uint16 y, const uint8 *srcBuf, uint16 srcPitch, uint16 w, uint16 h, bool xflip, bool masked);
	void fill(uint8 *dstBuf, uint16 dstPitch, uint16 x, uint16 y, uint16 w, uint16 h, uint8 color);

	void readPCX(uint8 *dst, uint16 dstPitch, const uint8 *src, uint16 w, uint16 h);

	void horizontalScrollUpdate(int16 xCamera);
	void horizontalScroll(int16 scroll);
	int16 horizontalScroll() const { return _horizontalScroll; }

	void fullscreen(bool fs) { _fullRefresh = 2; _fullscreen = fs; }
	bool fullscreen() const { return _fullscreen; }

	void setDirtyBlock(uint16 x, uint16 y, uint16 w, uint16 h);
	void forceFullRefresh() { _fullRefresh = 2; }

	void handleTimer();
	void waitForTimer();

	void setMouseCursor(uint8 *buf, uint16 w, uint16 h);
	void showMouseCursor(bool show);

	void initFont();

	void setText(uint16 x, uint16 y, const char *text, bool outlined = true);
	void setTextCentered(uint16 y, const char *text, bool outlined = true);
	void drawTexts();
	void clearTexts(uint16 y1, uint16 y2);
	void textCurrentColor(uint8 color) { _curTextColor = color; }
	void textColor(uint16 y, uint8 color) { _texts[y].color = color; }
	int textCenterX(const char *text) const;
	uint16 textWidth(const char *text) const;
	uint16 textWidth(const char *text, uint16 len) const;
	void drawChar(uint16 x, uint16 y, uint8 color, const uint8 *chr);
	void drawText(uint16 x, uint16 y, uint8 color, const char *text, bool outlined = true);
	void drawBox(int16 x1, int16 y1, int16 x2, int16 y2, uint8 col);
	
	void shake(bool reset);

	void blankScreen();
	void blankScreenEffect1();
	void blankScreenEffect2();
	void blankScreenEffect3();

private:

	enum {
		FADE_SPEED = 16,
		D_BLOCK_W  =  8,
		D_BLOCK_H  =  8
	};

	enum BufferDimension {
		BACKDROP_W = 640,
		BACKDROP_H = 200,
		SCREEN_W   = 320,
		SCREEN_H   = 200,
		PANEL_W    = 320,
		PANEL_H    =  50
	};

	struct {
		uint8 *room;
		uint8 *screen;
		uint8 *panel;
		int dirtyMin, dirtyMax;
		bool scrollable;
	} _pal;

	struct Dynalum {
		bool valid;
		uint8 msk[50 * 160];
		int8 lum[8 * 3];
		uint8 prevColMask;
	};

	struct TextSlot {
		uint16 x;
		uint8 color;
		Common::String text;
		bool outlined;
	};

	uint8 *_screenBuf;
	uint8 *_panelBuf;
	uint8 *_backdropBuf;

	uint8 _mousePtr[14 * 14];

	uint8 _fullRefresh;
	uint8 *_dirtyBlocks;
	uint16 _dirtyBlocksWidth, _dirtyBlocksHeight;

	bool _fullscreen;

	uint16 _horizontalScroll;
	uint16 _bdWidth, _bdHeight;

	TextSlot _texts[GAME_SCREEN_HEIGHT];
	uint8 _curTextColor;

	uint8 _charWidth[256];

	bool _gotTick;	

	Dynalum _dynalum;
	OSystem *_system;
	QueenEngine *_vm;

	static const uint8 _font[];
	static const uint8 _palJoeClothes[];
	static const uint8 _palJoeDress[];
};


} // End of namespace Queen

#endif
