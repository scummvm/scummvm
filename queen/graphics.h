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

#ifndef QUEENGRAPHICS_H
#define QUEENGRAPHICS_H

#include "common/str.h"
#include "common/util.h"
#include "queen/defs.h"
#include "queen/structs.h"

namespace Queen {

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
		int16 speed, speedBak;

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

	void clear();
};

struct TextSlot {
	uint16 x;
	uint8 color;
	Common::String text;
	bool outlined;
};

class QueenEngine;

class Graphics {
public:

	Graphics(QueenEngine *vm);
	~Graphics();

	void bobSetupControl();
	void bobDraw(const BobSlot *bs, int16 x, int16 y);
	void bobDrawInventoryItem(uint32 frameNum, uint16 x, uint16 y);
	void bobPaste(uint16 objNum, uint16 image);
	void bobShrink(const BobFrame *bf, uint16 percentage);
	void bobClear(uint32 bobNum);
	void bobSortAll();
	void bobDrawAll();
	void bobClearAll();
	void bobStopAll();
	BobSlot *bob(int index);

	void bobCustomParallax(uint16 roomNum); // CHECK_PARALLAX()

	void bobSetText(BobSlot *bob, const char *text, int textX, int textY, int color, int flags);

	void textCurrentColor(uint8 color) { _curTextColor = color; }
	void textSet(uint16 x, uint16 y, const char *text, bool outlined = true); // text()
	void textSetCentered(uint16 y, const char *text, bool outlined = true);
	void textDrawAll(); // drawtext()
	void textClear(uint16 y1, uint16 y2); // blanktexts()
	uint16 textWidth(const char *text) const; // textlen()
	int textCenterX(const char *text) const; // MIDDLE()
	void textColor(uint16 y, uint8 color) { _texts[y].color = color; }

	void setupNewRoom(const char *room, uint16 roomNum, int16 *furniture, uint16 furnitureCount);

	void fillAnimBuffer(const char *anim, AnimFrame *af);
	uint16 countAnimFrames(const char *anim);
	void setupObjectAnim(const GraphicData *gd, uint16 firstImage, uint16 bobNum, bool visible);
	uint16 setupPersonAnim(const ActorData *ad, const char *anim, uint16 curImage);
	void resetPersonAnim(uint16 bobNum);
	void erasePersonAnim(uint16 bobNum);
	void eraseAllAnims();

	uint16 refreshObject(uint16 obj);

	void setupRoomFurniture(int16 *furniture, uint16 furnitureCount);
	void setupRoomObjects();

	uint16 setupPerson(uint16 noun, uint16 curImage);
	uint16 allocPerson(uint16 noun, uint16 curImage);

	uint16 personFrames(uint16 bobNum) const { return _personFrames[bobNum]; }
	void clearPersonFrames() { memset(_personFrames, 0, sizeof(_personFrames)); }
	uint16 numFrames() const { return _numFrames; }
	uint16 numStaticFurniture() const { return _numFurnitureStatic; }
	uint16 numAnimatedFurniture() const { return _numFurnitureAnimated; }
	uint16 numFurnitureFrames() const { return _numFurnitureStatic + _numFurnitureAnimatedLen; }

	void putCameraOnBob(int bobNum) { _cameraBob = bobNum; }

	void update(uint16 room);


	enum {
		MAX_BOBS_NUMBER     =  64,
		MAX_STRING_LENGTH   = 255,
		MAX_STRING_SIZE     = (MAX_STRING_LENGTH + 1),
		BOB_SHRINK_BUF_SIZE = 60000
	};


private:

	BobSlot _bobs[MAX_BOBS_NUMBER];

	//! bobs to display
	BobSlot *_sortedBobs[MAX_BOBS_NUMBER];

	//! number of bobs to display
	uint16 _sortedBobsCount;

	//! used to scale a BobFrame
	BobFrame _shrinkBuffer;

	TextSlot _texts[GAME_SCREEN_HEIGHT];
	uint8 _curTextColor;

	AnimFrame _newAnim[17][30];

	uint16 _personFrames[4];

	//! Number of animated furniture in current room
	uint16 _numFurnitureAnimated;

	//! Number of static furniture in current room
	uint16 _numFurnitureStatic;

	//! Total number of frames for the animated furniture
	uint16 _numFurnitureAnimatedLen;

	//! Current number of frames unpacked
	uint16 _numFrames;

	int _cameraBob;

	QueenEngine *_vm;
};


class BamScene {
public:

	BamScene(QueenEngine *vm);
	
	void prepareAnimation();
	void updateCarAnimation();
	void updateFightAnimation();

	enum {
		BOB_OBJ1 = 5,
		BOB_OBJ2 = 6,
		BOB_FX   = 7
	};

	enum {
		F_STOP     = 0,
		F_PLAY     = 1,
		F_REQ_STOP = 2
	};

	uint16 _flag, _index;

private:

	struct BamDataObj {
		int16 x, y;
		int16 frame;
	};

	struct BamDataBlock {
		BamDataObj obj1; // truck / Frank
		BamDataObj obj2; // Rico  / robot
		BamDataObj fx;
		int16 sfx;
	};

	BobSlot *_obj1;
	BobSlot *_obj2;
	BobSlot *_objfx;
	bool _screenShaked;
	const BamDataBlock *_fightData;

	QueenEngine *_vm;

	static const BamDataBlock _carData[];
	static const BamDataBlock _fight1Data[];
	static const BamDataBlock _fight2Data[];
	static const BamDataBlock _fight3Data[];
};

} // End of namespace Queen

#endif
