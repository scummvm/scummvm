/* ScummVM - Scumm Interpreter
 * Copyright (C) 2001  Ludvig Strigeus
 * Copyright (C) 2001-2005 The ScummVM project
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License
 * as published by the Free Software Foundation; either version 2
 * of the License, or (at your option) any later version.

 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.

 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.
 *
 * $Header$
 *
 */


#ifndef ACTOR_H
#define ACTOR_H

#include "common/scummsys.h"
#include "scumm/scumm.h"


namespace Scumm {

enum MoveFlags {
	MF_NEW_LEG = 1,
	MF_IN_LEG = 2,
	MF_TURN = 4,
	MF_LAST_LEG = 8,
	MF_FROZEN = 0x80
};

struct ActorWalkData {
	Common::Point dest;           // Final destination point
	byte destbox;                 // Final destination box
	int16 destdir;                // Final destination, direction to face at

	Common::Point cur;            // Last position
	byte curbox;                  // Last box

	Common::Point next;           // Next position on our way to the destination, i.e. our intermediate destination

	Common::Point point3;
	int32 deltaXFactor, deltaYFactor;
	uint16 xfrac, yfrac;
};

struct CostumeData {
	byte active[16];
	uint16 animCounter;
	byte soundCounter;
	uint16 stopped;
	uint16 curpos[16];
	uint16 start[16];
	uint16 end[16];
	uint16 frame[16];

	uint16 heJumpOffsetTable[16];
	uint16 heJumpCountTable[16];
	uint32 heCondMaskTable[16];

	void reset() {
		stopped = 0;
		for (int i = 0; i < 16; i++) {
			active[i] = 0;
			curpos[i] = start[i] = end[i] = frame[i] = 0xFFFF;
		}
	}
};

struct AdjustBoxResult {	/* Result type of AdjustBox functions */
	int16 x, y;
	byte box;
};

struct SaveLoadEntry;

class Actor {

public:
	static byte kInvalidBox;

	static void initActorClass(ScummEngine *scumm);

public:
	/** The position of the actor inside the virtual screen. */
	Common::Point _pos;

	/** HE specific: This rect is used to clip actor drawing. */
	Common::Rect _clipOverride;

	int _offsX, _offsY;
	int _top, _bottom;
	uint _width;
	byte _number;
	uint16 _costume;
	byte _room;
	byte _talkColor;
	int _talkFrequency;
	byte _talkPan;
	byte _talkVolume;
	uint16 _boxscale;
	byte _scalex, _scaley;
	byte _charset;
	byte _moving;
	bool _ignoreBoxes;
	byte _forceClip;

	byte _initFrame;
	byte _walkFrame;
	byte _standFrame;
	byte _talkStartFrame;
	byte _talkStopFrame;

	bool _needRedraw, _needBgReset, _visible;
	byte _shadowMode;
	bool _flip;
	byte _frame;
	byte _walkbox;
	int16 _talkPosX, _talkPosY;
	uint16 _talkScript, _walkScript;
	bool _ignoreTurns;
	bool _drawToBackBuf;
	int8 _layer;
	uint16 _sound[32];
	CostumeData _cost;

	/* HE specific */
	int32 _heXmapNum;
	byte _hePaletteNum;
	bool _heNoTalkAnimation;
	bool _heSkipLimbs;
	bool _heTalking;
	uint32 _heCondMask;

	AuxBlock _auxBlock;

	struct {
		int16 posX;
		int16 posY;
		int16 color;
		byte sentence[128];
	} _heTalkQueue[16];

protected:
	byte _palette[256];
	int _elevation;
	uint16 _facing;
	uint16 _targetFacing;
	uint _speedx, _speedy;
	byte _animProgress, _animSpeed;
	bool _costumeNeedsInit;
	ActorWalkData _walkdata;
	int16 _animVariable[27];

	static ScummEngine *_vm;

public:

	Actor();

//protected:
	void hideActor();
	void showActor();

	void initActor(int mode);
	void putActor(int x, int y, byte room);
	void setActorWalkSpeed(uint newSpeedX, uint newSpeedY);
protected:
	int calcMovementFactor(const Common::Point& next);
	int actorWalkStep();
	int remapDirection(int dir, bool is_walking);
	void setupActorScale();

	void setBox(int box);
	int updateActorDirection(bool is_walking);

public:
	void adjustActorPos();
	AdjustBoxResult adjustXYToBeInBox(int dstX, int dstY);

	void setDirection(int direction);
	void faceToObject(int obj);
	void turnToDirection(int newdir);
	void walkActor();
	void walkActorOld();
	void drawActorToBackBuf(int x, int y);
	void drawActorCostume(bool hitTestMode = false);
	void animateCostume();
	void setActorCostume(int c);

	void animateLimb(int limb, int f);

	bool actorHitTest(int x, int y);

	byte *getActorName();
	void startWalkActor(int x, int y, int dir);
	void stopActorMoving();
protected:
	void startWalkAnim(int cmd, int angle);
public:
	void runActorTalkScript(int f);
	void startAnimActor(int frame);

	void remapActorPalette(int r_fact, int g_fact, int b_fact, int threshold);
	void remapActorPaletteColor(int slot, int color);

	void animateActor(int anim);

	bool isInCurrentRoom() const {
		return _room == _vm->_currentRoom;
	}

	int getActorXYPos(int &x, int &y) const;

	int getRoom() const {
		return _room;
	}

	int getFacing() const {
		return _facing;
	}

	int getAnimVar(byte var) const;
	void setAnimVar(byte var, int value);

	void setAnimSpeed(byte newAnimSpeed) {
		_animSpeed = newAnimSpeed;
		_animProgress = 0;
	}

	int getAnimSpeed() const {
		return _animSpeed;
	}

	int getAnimProgress() const {
		return _animProgress;
	}

	int getElevation() const {
		return _elevation;
	}

	void setElevation(int newElevation) {
		if (_elevation != newElevation) {
			_elevation = newElevation;
			_needRedraw = true;
		}
	}

	void setPalette(int idx, int val) {
		_palette[idx] = val;
		_needRedraw = true;
	}

	void setScale(int sx, int sy) {
		if (sx != -1)
			_scalex = sx;
		if (sy != -1)
			_scaley = sy;
		_needRedraw = true;
	}

	void classChanged(int cls, bool value);

	void setUserCondition(int slot, int set);
	bool isUserConditionSet(int slot) const;

	void setTalkCondition(int slot);
	bool isTalkConditionSet(int slot) const;

	// Used by the save/load syste:
	static const SaveLoadEntry *getSaveLoadEntries();

protected:
	bool isInClass(int cls);

	bool isPlayer();

	bool findPathTowards(byte box, byte box2, byte box3, Common::Point &foundPath);
	void findPathTowardsOld(byte box, byte box2, byte box3, Common::Point &p2, Common::Point &p3);
};

} // End of namespace Scumm

#endif
