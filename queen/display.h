/* ScummVM - Scumm Interpreter
 * Copyright (C) 2003 The ScummVM project
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

#include "common/util.h"
#include "queen/defs.h"

namespace Queen {


enum RenderingBuffer {
	RB_BACKDROP = 0,
	RB_PANEL    = 1,
	RB_SCREEN   = 2
};

enum JoePalette {
	JP_CLOTHES = 0,
	JP_DRESS   = 1
};


struct Dynalum {
	bool valid;
	uint8 msk[50 * 160];
	int8 lum[8 * 3];
	uint8 prevColMask;
};


struct TextRenderer {
	void init();
	void drawString(uint8 *dstBuf, uint16 dstPitch, uint16 x, uint16 y, uint8 color, const char *text, bool outlined = true);
	void drawChar(uint8 *dstBuf, uint16 dstPitch, uint16 x, uint16 y, uint8 color, const uint8 *chr);

	Language _lang;
	uint8 _charWidth[256];
	static const uint8 _font[];
};

class Input;
class Resource;

class Display {
public:

	Display(Language language, OSystem *system, Input *input);
	~Display();

	void dynalumInit(Resource *resource, const char *roomName, uint16 roomNum);
	void dynalumUpdate(int16 x, int16 y);

	void palConvert(uint8 *outPal, const uint8 *inPal, int start, int end);
	void palSet(const uint8 *pal, int start, int end, bool updateScreen = false);
	void palSetJoe(JoePalette pal);
	void palFadeIn(int start, int end, uint16 roomNum, bool dynalum = false, int16 dynaX = 0, int16 dynaY = 0);
	void palFadeOut(int start, int end, uint16 roomNum);
	void palFadePanel();
	void palScroll(int start, int end);
	void palCustomColors(uint16 roomNum);
	void palCustomScroll(uint16 roomNum);
	void palCustomFlash();
	void palSetAllDirty() { _pal.dirtyMin = 0; _pal.dirtyMax = 255; }

	void screenMode(int comPanel, bool inCutaway);

	void prepareUpdate();
	void update(bool dynalum = false, int16 dynaX = 0, int16 dynaY = 0);

	void blit(RenderingBuffer dstBuf, uint16 dstX, uint16 dstY, const uint8 *srcBuf, uint16 srcW, uint16 srcH, uint16 srcPitch, bool xflip, bool masked);
	void fill(RenderingBuffer dstBuf, uint16 x, uint16 y, uint16 w, uint16 h, uint8 color);

	void pcxRead(uint8 *dst, uint16 dstPitch, const uint8 *src, uint16 w, uint16 h);
	void pcxReadBackdrop(const uint8 *pcxBuf, uint32 size, bool useFullPal);
	void pcxReadPanel(const uint8 *pcxBuf, uint32 size);

	void textDraw(uint16 x, uint16 y, uint8 color, const char *text, bool outlined = true);
	uint16 textWidth(const char *text) const;

	void horizontalScrollUpdate(int16 xCamera);
	void horizontalScroll(int16 scroll);
	int16 horizontalScroll() const { return _horizontalScroll; }

	void fullscreen(bool fs) { debug(0, "Display::fullscreen(%d)", fs); _fullscreen = fs; }
	bool fullscreen() const { return _fullscreen; }

	void handleTimer();
	void waitForTimer();

	void setMouseCursor(uint8 *buf, uint16 w, uint16 h, uint16 xhs, uint16 yhs);
	void showMouseCursor(bool show);

	void drawBox(int16 x1, int16 y1, int16 x2, int16 y2, uint8 col);

private:

	enum {
		FADE_SPEED = 16
	};

	enum BufferDimension {
		BACKDROP_W = 640,
		BACKDROP_H = 200,
		SCREEN_W   = 320,
		SCREEN_H   = 200,
		PANEL_W    = 320,
		PANEL_H    = 50
	};

	TextRenderer _textRenderer;

	struct {
		uint8 *room;
		uint8 *screen;
		int dirtyMin, dirtyMax;
		bool scrollable;
	} _pal;

	uint8 *_buffer[3];
	uint16 _bufPitch[3];

	bool _fullscreen;

	uint16 _horizontalScroll;
	uint16 _bdWidth, _bdHeight;

	bool _gotTick;

	Dynalum _dynalum;
	OSystem *_system;
	Input *_input;

	static const uint8 _palJoeClothes[];
	static const uint8 _palJoeDress[];
};


} // End of namespace Queen

#endif
