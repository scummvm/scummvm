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

namespace Queen {

#define MAX_BANK_SIZE      110
#define MAX_FRAMES_NUMBER  256
#define MAX_BANKS_NUMBER    18
#define MAX_BOBS_NUMBER     64

#define BOB_SHRINK_BUF_SIZE  60000


struct BobFrame {
	uint16 width, height;
	uint16 xhotspot, yhotspot;
	uint8 *data;
};

// FIXME: share that with logic.h (ObjectData) ?
struct Box {
	uint16 x1, y1, x2, y2;

	bool intersects(uint16 x, uint16 y, uint16 w, uint16 h) const {
		return (x + w > x1) && (y + h > y1) && (x <= x2) && (y <= y2);
	}
};


struct BobSlot {
	bool active; 
	uint16 x, y;     //! current position
	Box box;         //! bounding box
	bool xflip;
	uint16 scale;    //! shrinking percentage
	uint16 frameNum; //! associated BobFrame
	uint8 frameDir;  //! 'direction' for the next frame (-1, 1)

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
	uint16 speed;      //! moving speed
	bool xmajor;       //! move along x axis instead of y
	int8 xdir, ydir;   //! moving direction
	uint16 endx, endy; //! destination point
	uint16 dx, dy;
	uint16 total;

	void moveOneStep();
	void animOneStep();
};


class QueenGraphics {
public:

	QueenGraphics(QueenResource *resource);
	~QueenGraphics();

	void bankLoad(const char *bankname, uint32 bankslot); // loadbank()
	void bankUnpack(uint32 srcframe, uint32 dstframe, uint32 bankslot); // unpackbank()
	void bankOverpack(uint32 srcframe, uint32 dstframe, uint32 bankslot); // overpackbank()
	void bankErase(uint32 bankslot); // erase()

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

	void frameErase(uint32 fslot);

private:
	
	struct PackedBank {
		uint32 indexes[MAX_BANK_SIZE];
		uint8 *data;
	};

	BobFrame _frames[MAX_FRAMES_NUMBER]; //! unbanked bob frames
	PackedBank _banks[MAX_BANKS_NUMBER]; //! banked bob frames
	BobSlot _bobs[MAX_BOBS_NUMBER];
	BobSlot *_sortedBobs[MAX_BOBS_NUMBER + 1]; //! bobs displayed
	BobFrame _shrinkBuffer;

	QueenResource *_resource;
	
};

} // End of namespace Queen

#endif
