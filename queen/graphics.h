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
#include "queen/input.h"
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
	int16 x, y;
	//! bounding box
	Box box;
	bool xflip;
	//! shrinking percentage
	uint16 scale;
	//! associated BobFrame
	uint16 frameNum;
	//! 'direction' for the next frame (-1, 1)
	int frameDir;

	//! animation stuff
	bool animating;
	struct {
		int16 speed, speedBak;	// Must be signed!

		//! string based animation
		struct {
			const AnimFrame *buffer;
			const AnimFrame *curPos;
		} string;

		//! normal moving animation
		struct {
			bool rebound;
			uint16 firstFrame, lastFrame;
		} normal;

	} anim;

	bool moving;
	//! moving speed
	int16 speed;
	//! move along x axis instead of y
	bool xmajor;
	//! moving direction
	int8 xdir, ydir;
	//! destination point
	int16 endx, endy;
	uint16 dx, dy;
	uint16 total;

	void curPos(int16 xx, int16 yy);
	void move(int16 dstx, int16 dsty, int16 spd);
	void moveOneStep();
	void animOneStep();

	void animString(const AnimFrame *animBuf);
	void animNormal(uint16 firstFrame, uint16 lastFrame, uint16 speed, bool rebound, bool xflip);
	void animReset();
};


struct TextSlot {
	uint16 x;
	uint8 color;
	Common::String text;
	bool outlined;
};


class Display;
class Input;

class Graphics {
public:

	Graphics(Display *display, Input *input, Resource *resource);
	~Graphics();

	void bankLoad(const char *bankname, uint32 bankslot); // loadbank()
	void bankUnpack(uint32 srcframe, uint32 dstframe, uint32 bankslot); // unpackbank()
	void bankOverpack(uint32 srcframe, uint32 dstframe, uint32 bankslot); // overpackbank()
	void bankErase(uint32 bankslot); // erase()

	void bobSetupControl();
	void bobAnimString(uint32 bobnum, const AnimFrame *buf); // stringanim()
	void bobAnimNormal(uint32 bobnum, uint16 firstFrame, uint16 lastFrame, uint16 speed, bool rebound, bool xflip); // makeanim()
	void bobMove(uint32 bobnum, int16 endx, int16 endy, int16 speed); // movebob()
	void bobDraw(uint32 bobnum, int16 x, int16 y, uint16 scale, bool xflip, const Box& box); // bob()
	void bobDrawInventoryItem(uint32 bobnum, uint16 x, uint16 y); // invbob()
	void bobPaste(uint32 frameNum, int16 x, int16 y); // bobpaste()
	void bobShrink(const BobFrame* pbf, uint16 percentage); // shrinkbob()
	void bobClear(uint32 bobnum); // clearbob()
	void bobSortAll(); // sortbobs()
	void bobDrawAll(); // drawbobs()
	void bobClearAll(); // clearallbobs()
	BobSlot *bob(int index);
	void bobCustomParallax(uint16 roomNum); // CHECK_PARALLAX()

	void bobSetText(
		BobSlot *bob, 
		const char *text, 
		int textX, int textY, 
		int color, int flags);	// MAKE_SPEAK_BOB

	void textCurrentColor(uint8 color); // ink()
	void textSet(uint16 x, uint16 y, const char *text, bool outlined = true); // text()
	void textSetCentered(uint16 y, const char *text, bool outlined = true);
	void textDrawAll(); // drawtext()
	void textClear(uint16 y1, uint16 y2); // blanktexts()
	uint16 textWidth(const char *text) const; // textlen()
	int textCenterX(const char *text) const; // MIDDLE()

	void frameErase(uint32 fslot);
	void frameEraseAll(bool joe); // freeframes, freeallframes
	BobFrame *frame(int index) { return _frames + index; }

	void loadBackdrop(const char *name, uint16 room);
	void loadPanel();

	void useJournal(GameSettings *settings);
	void journalBobSetup(uint32 bobnum, uint16 x, uint16 y, uint16 frame);
	void journalBobPreDraw(GameSettings *settings);

	void cameraBob(int bobNum) { _cameraBob = bobNum; }
	int cameraBob() const { return _cameraBob; }

	void update(uint16 room);


private:

	enum {
		MAX_STRING_LENGTH = 255,
		MAX_STRING_SIZE = (MAX_STRING_LENGTH + 1),
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

	//! number of bobs to display
	uint16 _sortedBobsCount;

	//! used to scale a BobFrame
	BobFrame _shrinkBuffer;

	TextSlot _texts[GAME_SCREEN_HEIGHT];
	uint8 _curTextColor;

	int _cameraBob; // cambob

	Display *_display;
	Input *_input;
	Resource *_resource;

};

} // End of namespace Queen

#endif
