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

#ifndef QUEENGRAPHICS_H
#define QUEENGRAPHICS_H

#include "queen/queen.h"
#include "queen/defs.h"
#include "queen/structs.h"

namespace Queen {


#define MAX_BANK_SIZE      110
#define MAX_FRAMES_NUMBER  256
#define MAX_BANKS_NUMBER    18
#define MAX_BOBS_NUMBER     64



struct BobFrame {
	uint16 width, height;
	uint16 xhotspot, yhotspot;
	uint8 *data;
};


struct BobSlot {
	bool active;
	//! current position
	uint16 x, y;
	//! bounding box
	Box box;
	bool xflip;
	//! shrinking percentage
	uint16 scale;
	//! associated BobFrame
	uint16 frameNum;
	//! 'direction' for the next frame (-1, 1)
	uint8 frameDir;

	//! animation stuff
	bool animating;
	struct {
		uint16 speed, speedBak;

		//! string based animation
		struct {
			uint8* buffer;
			uint8* curPos;
		} string;

		//! normal moving animation
		struct {
			bool rebound;
			uint16 firstFrame, lastFrame;
		} normal;

	} anim;

	bool moving;
	//! moving speed
	uint16 speed;
	//! move along x axis instead of y
	bool xmajor;
	//! moving direction
	int8 xdir, ydir;
	//! destination point
	uint16 endx, endy;
	uint16 dx, dy;
	uint16 total;

	void moveOneStep();
	void animOneStep();
};


struct TextSlot {
	uint16 x;
	uint8 color;
	Common::String text;
	bool outlined;
};


//class Display;

class Graphics {
public:

	Graphics(Resource *resource);
	~Graphics();

	void bankLoad(const char *bankname, uint32 bankslot); // loadbank()
	void bankUnpack(uint32 srcframe, uint32 dstframe, uint32 bankslot); // unpackbank()
	void bankOverpack(uint32 srcframe, uint32 dstframe, uint32 bankslot); // overpackbank()
	void bankErase(uint32 bankslot); // erase()

	void bobSetupControl();
	void bobAnimString(uint32 bobnum, uint8* animBuf); // stringanim()
	void bobAnimNormal(uint32 bobnum, uint16 firstFrame, uint16 lastFrame, uint16 speed, bool rebound, bool xflip); // makeanim()
	void bobMove(uint32 bobnum, uint16 endx, uint16 endy, int16 speed); // movebob()
	void bobDraw(uint32 bobnum, uint16 x, uint16 y, uint16 scale, bool xflip, const Box& box); // bob()
	void bobDrawInventoryItem(uint32 bobnum, uint16 x, uint16 y); // invbob()
	void bobPaste(uint32 bobnum, uint16 x, uint16 y); // bobpaste()
	void bobShrink(const BobFrame* pbf, uint16 percentage); // shrinkbob()
	void bobClear(uint32 bobnum); // clearbob()
	void bobSortAll(); // sortbobs()
	void bobDrawAll(); // drawbobs()
	void bobClearAll(); // clearallbobs()
	BobSlot *bob(int index);

	void textCurrentColor(uint8 color); // ink()
	void textSet(uint16 x, uint16 y, const char *text, bool outlined = true); // text()
	void textDrawAll(); // drawtext()
	void textClear(uint16 y1, uint16 y2); // blanktexts()
	uint16 textWidth(const char* text) const; // textlen()

	void frameErase(uint32 fslot);
	void frameEraseAll(bool joe); // freeframes, freeallframes

	void backdropLoad(const char *name, uint16 room); // loadbackdrop
	void backdropDraw();

	void panelLoad(); // loadpanel
	void panelDraw();
	void panelClear();

	void boxDraw(const Box &b, uint8 color);

	void useJournal();
	void journalBobSetup(uint32 bobnum, uint16 x, uint16 y, uint16 frame);
	void journalBobPreDraw();

	void update();
 
	void displayText(const TextSlot *pts, uint16 y); // FIXME: move to Display class
	void displayChar(uint16 x, uint16 y, uint8 color, const uint8 *chr); // FIXME: move to Display class
	static void displayBlit(uint8 *dst_buf, uint16 dst_x, uint16 dst_y, uint16 dst_pitch, const uint8 *src_buf, uint16 src_w, uint16 src_h, uint16 src_pitch, bool xflip, bool masked); // FIXME: move to Display class
	void displaySetPal(uint8 *pal, int start, int end);
	void displayScreen();

private:

	enum {
		BACKDROP_W = 640,
		BACKDROP_H = 200,
		SCREEN_W = 320,
		SCREEN_H = 200,
		PANEL_W = 320,
		PANEL_H = 50,
		BOB_SHRINK_BUF_SIZE = 60000
	};
	
	struct PackedBank {
		uint32 indexes[MAX_BANK_SIZE];
		uint8 *data;
	};

	//! unbanked bob frames
	BobFrame _frames[MAX_FRAMES_NUMBER];

	 //! banked bob frames
	PackedBank _banks[MAX_BANKS_NUMBER];

	BobSlot _bobs[MAX_BOBS_NUMBER];

	//! bobs to display
	BobSlot *_sortedBobs[MAX_BOBS_NUMBER];

	uint16 _sortedBobsCount;

	//! used to scale a BobFrame
	BobFrame _shrinkBuffer;

	TextSlot _texts[GAME_SCREEN_HEIGHT];
	uint8 _curTextColor;

	uint16 _cameraBob; // cambob

	//! current room dimensions
	uint16 _backdropWidth, _backdropHeight; // BDxres, BDyres

	 //! current room bitmap
	uint8 *_backdrop;

	uint8 *_screen;

	bool _fullscreen;

	uint16 _horizontalScroll;

	uint8 *_palette;

	//! panel storage area
	uint8 *_panel;

	Resource *_resource;
//	Display *_display;

	//! font used to render the text
	static const uint8 FONT[]; // FIXME: move to Display class

	//! font justification values
	static const uint8 FONT_SIZES[]; // FIXME: move to Display class

	void readPCX(const uint8 *src, uint8 *dst, uint16 dstPitch, uint16 w, uint16 h);

};

} // End of namespace Queen

#endif
