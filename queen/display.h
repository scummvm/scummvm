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


struct Dynalum {
	uint8 msk[50 * 160];
	int8 lum[8 * 3];
	uint8 prevColMask;
};


struct TextRenderer {
	void init();
	void drawString(uint8 *dstBuf, uint16 dstPitch, uint16 x, uint16 y, uint8 color, const char *text, bool outlined = true);
	void drawChar(uint8 *dstBuf, uint16 dstPitch, uint16 x, uint16 y, uint8 color, const uint8 *chr);

	uint8 charWidth[256];
	static const uint8 FONT[];
};


class Display {
public:

	Display(OSystem *system);
	~Display();

	void dynalumInit(Resource *resource, const char *roomName, uint16 roomNum);
	void dynalumUpdate(int x, int y);

	void palSetRoom(const uint8 *pal, int start, int end);
	void palFadeIn(uint8 start, uint8 end, uint16 roomNum);
	void palFadeOut(uint8 start, uint8 end, uint16 roomNum);
	void palFadePanel();
	void palCustomColors(uint16 roomNum); // check_colors
	void palCustomScroll(uint16 roomNum); // check_pal_scroll

	void screenMode(int comPanel, bool inCutaway);

	void prepareUpdate();
	void update(bool dynalum, int dynaX, int dynaY);

	void blit(RenderingBuffer dstBuf, uint16 dstX, uint16 dstY, const uint8 *srcBuf, uint16 srcW, uint16 srcH, uint16 srcPitch, bool xflip, bool masked);
	void fill(RenderingBuffer dstBuf, uint16 x, uint16 y, uint16 w, uint16 h, uint8 color);

	void pcxRead(uint8 *dst, uint16 dstPitch, const uint8 *src, uint16 w, uint16 h);
	void pcxReadBackdrop(const uint8 *pcxBuf, uint32 size, bool useFullPal);
	void pcxReadPanel(const uint8 *pcxBuf, uint32 size);

	void textDraw(uint16 x, uint16 y, uint8 color, const char *text, bool outlined = true);
	uint16 textWidth(const char *text) const;

	void horizontalScroll(uint16 scroll);

	bool fullscreen() const { return _fullscreen; }


private:

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
		uint8 *screen;
		uint8 *room;
		int dirtyMin, dirtyMax;
	} _pals;

	uint8 *_buffers[3];
	uint16 _bufPitch[3];

	bool _fullscreen;
	bool _panel;

	uint16 _horizontalScroll;
	uint16 _bdWidth, _bdHeight;

	Dynalum _dynalum;
	OSystem *_system;

};


} // End of namespace Queen

#endif
