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

#include "queen/queen.h"
#include "queen/defs.h"
#include "queen/structs.h"

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

	Language lang;
	uint8 charWidth[256];
	static const uint8 FONT[];
};


class Display {
public:

	Display(Language language, OSystem *system, Input *input);
	~Display();

	void dynalumInit(Resource *resource, const char *roomName, uint16 roomNum);
	void dynalumUpdate(int16 x, int16 y);

	void palConvert(uint8 *outPal, const uint8 *inPal, int start, int end);
	void palSet(const uint8 *pal, int start, int end, bool updateScreen = false);
	void palSetJoe(JoePalette pal); // changejoepal
	void palFadeIn(int start, int end, uint16 roomNum);
	void palFadeOut(int start, int end, uint16 roomNum);
	void palFadePanel();
	void palScroll(int start, int end);
	void palCustomColors(uint16 roomNum); // check_colors
	void palCustomScroll(uint16 roomNum); // check_pal_scroll
	void palCustomFlash(); // flashspecial()
	void palSetAllDirty() { _pals.dirtyMin = 0; _pals.dirtyMax = 255; }

	void screenMode(int comPanel, bool inCutaway);

	void prepareUpdate();
	void update(bool dynalum, int16 dynaX, int16 dynaY);

	void blit(RenderingBuffer dstBuf, uint16 dstX, uint16 dstY, const uint8 *srcBuf, uint16 srcW, uint16 srcH, uint16 srcPitch, bool xflip, bool masked);
	void fill(RenderingBuffer dstBuf, uint16 x, uint16 y, uint16 w, uint16 h, uint8 color);

	void pcxRead(uint8 *dst, uint16 dstPitch, const uint8 *src, uint16 w, uint16 h);
	void pcxReadBackdrop(const uint8 *pcxBuf, uint32 size, bool useFullPal);
	void pcxReadPanel(const uint8 *pcxBuf, uint32 size);

	void textDraw(uint16 x, uint16 y, uint8 color, const char *text, bool outlined = true);
	uint16 textWidth(const char *text) const;

	void horizontalScrollUpdate(int16 xCamera); // calc_screen_scroll
	void horizontalScroll(int16 scroll);
	int16 horizontalScroll() const { return _horizontalScroll; }

	void fullscreen(bool fs) { _fullscreen = fs; }
	bool fullscreen() const { return _fullscreen; }

	void panel(bool on) { _panel = on; }

	void handleTimer();
	void waitForTimer();

	void mouseCursorInit(uint8 *buf, uint16 w, uint16 h, uint16 xhs, uint16 yhs);
	void mouseCursorShow(bool show);

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
	} _pals;

	uint8 *_buffers[3];
	uint16 _bufPitch[3];

	bool _fullscreen;
	bool _panel;

	uint16 _horizontalScroll;
	uint16 _bdWidth, _bdHeight;

	bool _gotTick;

	Dynalum _dynalum;
	OSystem *_system;
	Input *_input;

	static const uint8 PAL_JOE_CLOTHES[];
	static const uint8 PAL_JOE_DRESS[];
};


} // End of namespace Queen

#endif
