/* ScummVM - Graphic Adventure Engine
 *
 * ScummVM is the legal property of its developers, whose names
 * are too numerous to list here. Please refer to the COPYRIGHT
 * file distributed with this source distribution.
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
 */

/*
 * This code is based on original Tony Tough source code
 *
 * Copyright (c) 1997-2003 Nayma Software
 */

#ifndef TONY_LOC_H
#define TONY_LOC_H

#include "common/scummsys.h"
#include "common/system.h"
#include "common/file.h"
#include "tony/sound.h"
#include "tony/utils.h"

namespace Tony {

/****************************************************************************\
*       Various defines
\****************************************************************************/

/**
 * Valid colour modes
 */
typedef enum {
	CM_256,
	CM_65K
} RMColorMode;


/****************************************************************************\
*       Class declarations
\****************************************************************************/

/**
 * Generic palette
 */
class RMPalette {
public:
	byte _data[1024];

public:
	friend RMDataStream &operator>>(RMDataStream &ds, RMPalette &pal);
};


/**
 * Sound effect of an object
 */
class RMSfx {
public:
	RMString _name;
	FPSFX *_fx;
	bool _bPlayingLoop;

public:
	RMSfx();
	virtual ~RMSfx();

	friend RMDataStream &operator>>(RMDataStream &ds, RMSfx &sfx);

	void play(bool bLoop = false);
	void setVolume(int vol);
	void pause(bool bPause);
	void stop(void);

	void readFromStream(RMDataStream &ds, bool bLOX = false);
};


/**
 * Object pattern
 */
class RMPattern {
public:
	// Type of slot
	enum RMSlotType {
		DUMMY1 = 0,
		DUMMY2,
		SPRITE,
		SOUND,
		COMMAND,
		SPECIAL
	};

	// Class slot
	class RMSlot {
	private:
		RMPoint _pos; // Child co-ordinates

	public:
		RMSlotType _type;
		int _data;
		byte _flag;

	public:
		friend RMDataStream &operator>>(RMDataStream &ds, RMSlot &slot);

		RMPoint pos() {
			return _pos;
		}

		void readFromStream(RMDataStream &ds, bool bLOX = false);
	};

public:
	RMString _name;

private:
	int _speed;
	RMPoint _pos;      // Parent coordinates
	RMPoint _curPos;   // Parent + child coordinates
	int _bLoop;
	int _nSlots;
	int _nCurSlot;
	int _nCurSprite;

	RMSlot *_slots;

	uint32 _nStartTime;

public:
	RMPattern();
	virtual ~RMPattern();

	friend RMDataStream &operator>>(RMDataStream &ds, RMPattern &pat);

	// A warning that the pattern now and the current
	int init(RMSfx *sfx, bool bPlayP0 = false, byte *bFlag = NULL);

	// Update the pattern, checking to see if it's time to change slot and executing
	// any associated commands
	int update(uint32 hEndPattern, byte &bFlag, RMSfx *sfx);

	// Stop a sound effect
	void stopSfx(RMSfx *sfx);

	// Reads the position of the pattern
	RMPoint pos() {
		return _curPos;
	}

	void readFromStream(RMDataStream &ds, bool bLOX = false);

private:
	void updateCoord(void);
};


/**
 * Sprite (frame) animation of an item
 */
class RMSprite : public RMGfxTask {
public:
	RMString _name;
	RMRect _rcBox;

protected:
	RMGfxSourceBuffer *_buf;

public:
	RMSprite();
	virtual ~RMSprite();

	void init(RMGfxSourceBuffer *buf);
	friend RMDataStream &operator>>(RMDataStream &ds, RMSprite &sprite);
	virtual void draw(CORO_PARAM, RMGfxTargetBuffer &bigBuf, RMGfxPrimitive *prim);
	void setPalette(byte *lpBuf);
	void getSizeFromStream(RMDataStream &ds, int *dimx, int *dimy);
	void LOXGetSizeFromStream(RMDataStream &ds, int *dimx, int *dimy);

	void readFromStream(RMDataStream &ds, bool bLOX = false);
};


/**
 * Data on an item
 */
class RMItem : public RMGfxTask {
public:
	RMString _name;

protected:
	int _z;
	RMPoint _pos;  // Coordinate nonno
	RMColorMode _cm;
	RMPoint _curScroll;

	byte _FX;
	byte _FXparm;

	virtual int getCurPattern() {
		return _nCurPattern;
	}

private:
	int _nCurPattern;
	int _mpalCode;
	RMPoint _hot;
	RMRect _rcBox;
	int _nSprites, _nSfx, _nPatterns;
	byte _bPal;
	RMPalette _pal;

	RMSprite *_sprites;
	RMSfx *_sfx;
	RMPattern *_patterns;

	byte _bCurFlag;
	int _nCurSprite;
	bool _bIsActive;
	uint32 _hEndPattern;
	bool _bInitCurPattern;

public:
	RMPoint calculatePos(void);

public:
	RMItem();
	virtual ~RMItem();

	friend RMDataStream &operator>>(RMDataStream &ds, RMItem &item);

	// Process to make the object move on any animations.
	// Returns TRUE if it should be redrawn on the next frame
	bool doFrame(RMGfxTargetBuffer *bigBuf, bool bAddToList = true);

	// Sets the current scrolling position
	void setScrollPosition(const RMPoint &scroll);

	// Overloading of check whether to remove from active list
	virtual void removeThis(CORO_PARAM, bool &result);

	// Overloaded Draw
	virtual void draw(CORO_PARAM, RMGfxTargetBuffer &bigBuf, RMGfxPrimitive *prim);

	// Overloaded priority: it's based on Z ordering
	virtual int priority() {
		return _z;
	}

	// Pattern number
	int numPattern() {
		return _nPatterns;
	}

	// Set anew animation pattern, changing abruptly from the current
	virtual void setPattern(int nPattern, bool bPlayP0 = false);

	// Set a new status
	void setStatus(int nStatus);

	bool isIn(const RMPoint &pt, int *size = NULL);
	RMPoint hotspot() {
		return _hot;
	}
	bool getName(RMString &name);
	int mpalCode() {
		return _mpalCode;
	}

	// Unload
	void unload(void);

	// Wait for the end of the current pattern
	void waitForEndPattern(CORO_PARAM, uint32 hCustomSkip = CORO_INVALID_PID_VALUE);

	// Sets a new hotspot fro the object
	void changeHotspot(const RMPoint &pt);

	void setInitCurPattern(bool status) {
		_bInitCurPattern = status;
	}

	void playSfx(int nSfx);

	void readFromStream(RMDataStream &ds, bool bLOX = false);

	void pauseSound(bool bPause);

protected:
	// Create a primitive that has as it's task this item
	virtual RMGfxPrimitive *newItemPrimitive();

	// Allocate memory for the sprites
	virtual RMGfxSourceBuffer *newItemSpriteBuffer(int dimx, int dimy, bool bPreRLE);
};


#define MAXBOXES    50      // Maximum number of allowed boxes
#define MAXHOTSPOT 20       // Maximum nimber of allowed hotspots

class RMBox {
public:
	struct Hotspot {
		int _hotx, _hoty;        // Hotspot coordinates
		int _destination;        // Hotspot destination
	};

public:
	int _left, _top, _right, _bottom; // Vertici bounding boxes
	int _adj[MAXBOXES];               // List of adjacent bounding boxes
	int _numHotspot;                  // Hotspot number
	uint8 _destZ;                     // Z value for the bounding box
	Hotspot _hotspot[MAXHOTSPOT];     // List of hotspots

	bool _bActive;
	bool _bReversed;

private:
	void readFromStream(RMDataStream &ds);

public:
	friend RMDataStream &operator>>(RMDataStream &ds, RMBox &box);
};


class RMBoxLoc {
public:
	int _numbBox;
	RMBox *_boxes;

private:
	void readFromStream(RMDataStream &ds);

public:
	RMBoxLoc();
	virtual ~RMBoxLoc();

	friend RMDataStream &operator >>(RMDataStream &ds, RMBoxLoc &bl);
	void recalcAllAdj(void);
};

#define GAME_BOXES_SIZE 200

class RMGameBoxes {
protected:
	RMBoxLoc *_allBoxes[GAME_BOXES_SIZE];
	int _nLocBoxes;

public:
	RMGameBoxes();
	~RMGameBoxes();

	void init(void);
	void close(void);

	// Get binding boxes for a given location
	RMBoxLoc *getBoxes(int nLoc);
	int getLocBoxesCount() const { return _nLocBoxes; }

	// Return the box which contains a given point
	int whichBox(int nLoc, const RMPoint &pt);

	// Check whether a point is inside a given box
	bool isInBox(int nLoc, int nBox, const RMPoint &pt);

	// Change the status of a box
	void changeBoxStatus(int nLoc, int nBox, int status);

	// Save state handling
	int getSaveStateSize(void);
	void saveState(byte *buf);
	void loadState(byte *buf);
};

class RMCharacter : protected RMItem {
public:
	enum Patterns {
		PAT_STANDUP = 1,
		PAT_STANDDOWN,
		PAT_STANDLEFT,
		PAT_STANDRIGHT,
		PAT_WALKUP,
		PAT_WALKDOWN,
		PAT_WALKLEFT,
		PAT_WALKRIGHT
	};

private:
	enum CharacterStatus {
		STAND,
		WALK
	};

	signed short _walkCount;
	int _dx, _dy, _olddx, _olddy;
	float _fx, _fy, _slope;
	RMPoint _lineStart, _lineEnd, _pathEnd;
	signed char _walkSpeed, _walkStatus;
	char _minPath;
	short _nextBox;
	short _path[MAXBOXES];
	short _pathLength, _pathCount;
	int _curBox;

	CharacterStatus _status;
	int _curSpeed;
	bool _bEndOfPath;
	uint32 _hEndOfPath;
	OSystem::MutexRef _csMove;
	int _curLocation;
	bool _bRemoveFromOT;
	bool _bMovingWithoutMinpath;
	RMGameBoxes *_theBoxes;

	RMPoint _fixedScroll;

private:
	int inWhichBox(const RMPoint &pt);

	short findPath(short source, short destination);
	RMPoint searching(char UP, char DOWN, char RIGHT, char LEFT, RMPoint punto);
	RMPoint nearestPoint(const RMPoint &punto);

	void goTo(CORO_PARAM, RMPoint destcoord, bool bReversed = false);
	short scanLine(const RMPoint &punto);
	RMPoint invScanLine(const RMPoint &punto);
	RMPoint nearestHotSpot(int sourcebox, int destbox);

	void newBoxEntered(int nBox);

protected:
	bool _bMoving;
	bool _bDrawNow;
	bool _bNeedToStop;
//		virtual RMGfxPrimitive *NewItemPrimitive();

public:
	RMCharacter();
	virtual ~RMCharacter();

	void linkToBoxes(RMGameBoxes *theBoxes);

	virtual void removeThis(CORO_PARAM, bool &result);

	// Update the position of a character
	void doFrame(CORO_PARAM, RMGfxTargetBuffer *bigBuf, int loc);

	// Overloaded draw
	virtual void draw(CORO_PARAM, RMGfxTargetBuffer &bigBuf, RMGfxPrimitive *prim);

	// TRUE if you just stopped
	bool endOfPath() {
		return _bEndOfPath;
	}

	// Change the pattern of a character to STOP
	virtual void stop(CORO_PARAM);

	// Check if the character is moving
	bool isMoving() {
		return _bMoving;
	}

	// Move the character to a certain position
	void move(CORO_PARAM, RMPoint pt, bool *result = NULL);

	// Place the character in a certain position WITHOUT moving
	void setPosition(const RMPoint &pt, int newloc = -1);

	// Wait for the end of movement
	void waitForEndMovement(CORO_PARAM);

	void setFixedScroll(const RMPoint &fix) {
		_fixedScroll = fix;
	}
	void setSpeed(int speed) {
		_curSpeed = speed;
	}
};


class RMWipe : public RMGfxTask {
private:
	bool _bFading;
	bool _bEndFade;
	bool _bUnregister;
	uint32 _hUnregistered;
	int _nFadeStep;
	uint32 _hEndOfFade;
	bool _bMustRegister;

	RMItem _wip0r;

public:
	RMWipe();
	virtual ~RMWipe();

	void doFrame(RMGfxTargetBuffer &bigBuf);
	virtual void draw(CORO_PARAM, RMGfxTargetBuffer &bigBuf, RMGfxPrimitive *prim);

	void initFade(int type);
	void closeFade(void);
	void waitForFadeEnd(CORO_PARAM);

	virtual void Unregister(void);
	virtual void removeThis(CORO_PARAM, bool &result);
	virtual int priority(void);
};


/**
 * Location
 */
class RMLocation : public RMGfxTaskSetPrior {
public:
	RMString _name;                // Name

private:
	RMColorMode _cmode;            // Color mode
	RMGfxSourceBuffer *_buf;       // Location picture

	int _nItems;                   // Number of objects
	RMItem *_items;                // Objects

	RMPoint _curScroll;            // Current scroll position
	RMPoint _fixedScroll;

public:
	// @@@@@@@@@@@@@@@@@@@@@@@
	RMPoint TEMPTonyStart;
	RMPoint TEMPGetTonyStart() {
		return TEMPTonyStart;
	}

	int TEMPNumLoc;
	int TEMPGetNumLoc() {
		return TEMPNumLoc;
	}

public:
	RMLocation();
	virtual ~RMLocation();

	// Load variations
	bool load(const char *lpszFileName);
	bool load(Common::File &file);
	bool load(const byte *buf);
	bool load(RMDataStream &ds);
	bool loadLOX(RMDataStream &ds);

	// Unload
	void unload(void);

	// Overloaded draw
	virtual void draw(CORO_PARAM, RMGfxTargetBuffer &bigBuf, RMGfxPrimitive *prim);

	// Prepare a frame by drawing the location and all it's items
	void doFrame(RMGfxTargetBuffer *bigBuf);

	// Return the item at a given point
	RMItem *whichItemIsIn(const RMPoint &pt);

	// Return the item based on it's MPAL code
	RMItem *getItemFromCode(uint32 dwCode);

	// Set the current scroll position
	void setScrollPosition(const RMPoint &scroll);

	// Sets an additinal offset for scrolling
	void setFixedScroll(const RMPoint &scroll);

	// Update the scrolling coordinates to display the specified point
	void updateScrolling(const RMPoint &ptShowThis);

	// Read the current scroll position
	RMPoint scrollPosition() {
		return _curScroll;
	}

	// Pause sound
	void pauseSound(bool bPause);
};


/**
 * MPAL message, composed of more ASCIIZ
 */
class RMMessage {
private:
	char *_lpMessage;
	char *_lpPeriods[256];
	int _nPeriods;

private:
	void parseMessage(void);

public:
	RMMessage();
	RMMessage(uint32 dwId);
	virtual ~RMMessage();

	void load(uint32 dwId);
	bool isValid() {
		return _lpMessage != NULL;
	}
	int numPeriods() {
		return _nPeriods;
	}
	char *period(int num) {
		return _lpPeriods[num];
	}
	char *operator[](int num) {
		return _lpPeriods[num];
	}
};

} // End of namespace Tony

#endif /* TONY_H */
