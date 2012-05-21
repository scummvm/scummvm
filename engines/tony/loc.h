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
	byte m_data[1024];

public:
	friend RMDataStream &operator>>(RMDataStream &ds, RMPalette &pal);
};


/**
 * Sound effect of an object
 */
class RMSfx {
public:
	RMString m_name;
	FPSFX *m_fx;
	bool m_bPlayingLoop;

public:
	RMSfx();
	virtual ~RMSfx();

	friend RMDataStream &operator>>(RMDataStream &ds, RMSfx &sfx);

	void Play(bool bLoop = false);
	void SetVolume(int vol);
	void Pause(bool bPause);
	void Stop(void);

	void ReadFromStream(RMDataStream &ds, bool bLOX = false);
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
		RMPoint m_pos; // Child co-ordinates

	public:
		RMSlotType m_type;
		int m_data;
		byte m_flag;

	public:
		friend RMDataStream &operator>>(RMDataStream &ds, RMSlot &slot);

		RMPoint Pos() {
			return m_pos;
		}

		void ReadFromStream(RMDataStream &ds, bool bLOX = false);
	};

public:
	RMString m_name;

private:
	int m_speed;
	RMPoint m_pos;      // Parent coordinates
	RMPoint m_curPos;   // Parent + child coordinates
	int m_bLoop;
	int m_nSlots;
	int m_nCurSlot;
	int m_nCurSprite;

	RMSlot *m_slots;

	uint32 m_nStartTime;

public:
	RMPattern();
	virtual ~RMPattern();

	friend RMDataStream &operator>>(RMDataStream &ds, RMPattern &pat);

	// A warning that the pattern now and the current
	int Init(RMSfx *sfx, bool bPlayP0 = false, byte *bFlag = NULL);

	// Update the pattern, checking to see if it's time to change slot and executing
	// any associated commands
	int Update(uint32 hEndPattern, byte &bFlag, RMSfx *sfx);

	// Stop a sound effect
	void StopSfx(RMSfx *sfx);

	// Reads the position of the pattern
	RMPoint Pos() {
		return m_curPos;
	}

	void ReadFromStream(RMDataStream &ds, bool bLOX = false);

private:
	void UpdateCoord(void);
};


/**
 * Sprite (frame) animation of an item
 */
class RMSprite : public RMGfxTask {
public:
	RMString m_name;
	RMRect m_rcBox;

protected:
	RMGfxSourceBuffer *m_buf;

public:
	RMSprite();
	virtual ~RMSprite();

	void Init(RMGfxSourceBuffer *buf);
	friend RMDataStream &operator>>(RMDataStream &ds, RMSprite &sprite);
	virtual void Draw(CORO_PARAM, RMGfxTargetBuffer &bigBuf, RMGfxPrimitive *prim);
	void SetPalette(byte *lpBuf);
	void GetSizeFromStream(RMDataStream &ds, int *dimx, int *dimy);
	void LOXGetSizeFromStream(RMDataStream &ds, int *dimx, int *dimy);

	void ReadFromStream(RMDataStream &ds, bool bLOX = false);
};


/**
 * Data on an item
 */
class RMItem : public RMGfxTask {
public:
	RMString m_name;

protected:
	int m_z;
	RMPoint m_pos;  // Coordinate nonno
	RMColorMode m_cm;
	RMPoint m_curScroll;

	byte m_FX;
	byte m_FXparm;

	virtual int GetCurPattern() {
		return m_nCurPattern;
	}

private:
	int m_nCurPattern;
	int m_mpalCode;
	RMPoint m_hot;
	RMRect m_rcBox;
	int m_nSprites, m_nSfx, m_nPatterns;
	byte m_bPal;
	RMPalette m_pal;

	RMSprite *m_sprites;
	RMSfx *m_sfx;
	RMPattern *m_patterns;

	byte m_bCurFlag;
	int m_nCurSprite;
	bool m_bIsActive;
	uint32 m_hEndPattern;
	bool m_bInitCurPattern;

public:
	RMPoint CalculatePos(void);

public:
	RMItem();
	virtual ~RMItem();

	friend RMDataStream &operator>>(RMDataStream &ds, RMItem &item);

	// Process to make the object move on any animations.
	// Returns TRUE if it should be redrawn on the next frame
	bool DoFrame(RMGfxTargetBuffer *bigBuf, bool bAddToList = true);

	// Sets the current scrolling position
	void SetScrollPosition(const RMPoint &scroll);

	// Overloading of check whether to remove from active list
	virtual void RemoveThis(CORO_PARAM, bool &result);

	// Overloaded Draw
	virtual void Draw(CORO_PARAM, RMGfxTargetBuffer &bigBuf, RMGfxPrimitive *prim);

	// Overloaded priority: it's based on Z ordering
	virtual int Priority() {
		return m_z;
	}

	// Pattern number
	int NumPattern() {
		return m_nPatterns;
	}

	// Set anew animation pattern, changing abruptly from the current
	virtual void SetPattern(int nPattern, bool bPlayP0 = false);

	// Set a new status
	void SetStatus(int nStatus);

	bool IsIn(const RMPoint &pt, int *size = NULL);
	RMPoint Hotspot() {
		return m_hot;
	}
	bool GetName(RMString &name);
	int MpalCode() {
		return m_mpalCode;
	}

	// Unload
	void Unload(void);

	// Wait for the end of the current pattern
	void WaitForEndPattern(CORO_PARAM, uint32 hCustomSkip = CORO_INVALID_PID_VALUE);

	// Sets a new hotspot fro the object
	void ChangeHotspot(const RMPoint &pt);

	void SetInitCurPattern(bool status) {
		m_bInitCurPattern = status;
	}

	void PlaySfx(int nSfx);

	void ReadFromStream(RMDataStream &ds, bool bLOX = false);

	void PauseSound(bool bPause);

protected:
	// Create a primitive that has as it's task this item
	virtual RMGfxPrimitive *NewItemPrimitive();

	// Allocate memory for the sprites
	virtual RMGfxSourceBuffer *NewItemSpriteBuffer(int dimx, int dimy, bool bPreRLE);
};


#define MAXBOXES    50      // Maximum number of allowed boxes
#define MAXHOTSPOT 20       // Maximum nimber of allowed hotspots

class RMBox {
public:
	struct T_HOTSPOT {
		int hotx, hoty;          // Hotspot coordinates
		int destination;         // Hotspot destination
	};

public:
	int left, top, right, bottom;   // Vertici bounding boxes
	int adj[MAXBOXES];              // List of adjacent bounding boxes
	int numhotspot;                 // Hotspot number
	uint8 Zvalue;                   // Z value for the bounding box
	T_HOTSPOT hotspot[MAXHOTSPOT];  // List of hotspots

	bool attivo;
	bool bReversed;

private:
	void ReadFromStream(RMDataStream &ds);

public:
	friend RMDataStream &operator>>(RMDataStream &ds, RMBox &box);
};


class RMBoxLoc {
public:
	int numbbox;
	RMBox *boxes;

private:
	void ReadFromStream(RMDataStream &ds);

public:
	RMBoxLoc();
	virtual ~RMBoxLoc();

	friend RMDataStream &operator >>(RMDataStream &ds, RMBoxLoc &bl);
	void RecalcAllAdj(void);
};

#define GAME_BOXES_SIZE 200

class RMGameBoxes {
protected:
	RMBoxLoc *m_allBoxes[GAME_BOXES_SIZE];
	int m_nLocBoxes;

public:
	RMGameBoxes();
	~RMGameBoxes();

	void Init(void);
	void Close(void);

	// Get binding boxes for a given location
	RMBoxLoc *GetBoxes(int nLoc);
	int GetLocBoxesCount() const { return m_nLocBoxes; }

	// Return the box which contains a given point
	int WhichBox(int nLoc, const RMPoint &pt);

	// Check whether a point is inside a given box
	bool IsInBox(int nLoc, int nBox, const RMPoint &pt);

	// Change the status of a box
	void ChangeBoxStatus(int nLoc, int nBox, int status);

	// Save state handling
	int GetSaveStateSize(void);
	void SaveState(byte *buf);
	void LoadState(byte *buf);
};

class RMCharacter : protected RMItem {
public:
	enum PATTERNS {
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
	enum STATUS {
		STAND,
		WALK
	};

	signed short walkcount;
	int dx, dy, olddx, olddy;
	float fx, fy, slope;
	RMPoint linestart, lineend, pathend;
	signed char walkspeed, walkstatus;
	char minpath;
	short nextbox;
	short path[MAXBOXES];
	short pathlenght, pathcount;
	int curbox;

	STATUS status;
	int curSpeed;
	bool bEndOfPath;
	uint32 hEndOfPath;
	OSystem::MutexRef csMove;
	int curLocation;
	bool bRemoveFromOT;
	bool bMovingWithoutMinpath;
	RMGameBoxes *theBoxes;

	RMPoint m_fixedScroll;

private:
	int InWhichBox(const RMPoint &pt);

	short FindPath(short source, short destination);
	RMPoint Searching(char UP, char DOWN, char RIGHT, char LEFT, RMPoint punto);
	RMPoint NearestPoint(const RMPoint &punto);

	void GoTo(CORO_PARAM, RMPoint destcoord, bool bReversed = false);
	short ScanLine(const RMPoint &punto);
	RMPoint InvScanLine(const RMPoint &punto);
	RMPoint NearestHotSpot(int sourcebox, int destbox);

	void NewBoxEntered(int nBox);

protected:
	bool bMoving;
	bool bDrawNow;
	bool bNeedToStop;
//		virtual RMGfxPrimitive *NewItemPrimitive();

public:
	RMCharacter();
	virtual ~RMCharacter();

	void LinkToBoxes(RMGameBoxes *theBoxes);

	virtual void RemoveThis(CORO_PARAM, bool &result);

	// Update the position of a character
	void DoFrame(CORO_PARAM, RMGfxTargetBuffer *bigBuf, int loc);

	// Overloaded draw
	virtual void Draw(CORO_PARAM, RMGfxTargetBuffer &bigBuf, RMGfxPrimitive *prim);

	// TRUE if you just stopped
	bool EndOfPath() {
		return bEndOfPath;
	}

	// Change the pattern of a character to STOP
	virtual void Stop(CORO_PARAM);

	// Check if the character is moving
	bool IsMoving() {
		return bMoving;
	}

	// Move the character to a certain position
	void Move(CORO_PARAM, RMPoint pt, bool *result = NULL);

	// Place the character in a certain position WITHOUT moving
	void SetPosition(const RMPoint &pt, int newloc = -1);

	// Wait for the end of movement
	void WaitForEndMovement(CORO_PARAM);

	void SetFixedScroll(const RMPoint &fix) {
		m_fixedScroll = fix;
	}
	void SetSpeed(int speed) {
		curSpeed = speed;
	}
};


class RMWipe : public RMGfxTask {
private:
	bool m_bFading;
	bool m_bEndFade;
	bool m_bUnregister;
	uint32 m_hUnregistered;
	int m_nFadeStep;
	uint32 m_hEndOfFade;
	bool m_bMustRegister;

	RMItem m_wip0r;

public:
	RMWipe();
	virtual ~RMWipe();

	void DoFrame(RMGfxTargetBuffer &bigBuf);
	virtual void Draw(CORO_PARAM, RMGfxTargetBuffer &bigBuf, RMGfxPrimitive *prim);

	void InitFade(int type);
	void CloseFade(void);
	void WaitForFadeEnd(CORO_PARAM);

	virtual void Unregister(void);
	virtual void RemoveThis(CORO_PARAM, bool &result);
	virtual int Priority(void);
};


/**
 * Location
 */
class RMLocation : public RMGfxTaskSetPrior {
public:
	RMString m_name;                // Name

private:
	RMColorMode m_cmode;            // Color mode
	RMGfxSourceBuffer *m_buf;       // Location picture

	int m_nItems;                   // Number of objects
	RMItem *m_items;                // Objects

	RMPoint m_curScroll;            // Current scroll position
	RMPoint m_fixedScroll;

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
	bool Load(const char *lpszFileName);
	bool Load(Common::File &file);
	bool Load(const byte *buf);
	bool Load(RMDataStream &ds);
	bool LoadLOX(RMDataStream &ds);

	// Unload
	void Unload(void);

	// Overloaded draw
	virtual void Draw(CORO_PARAM, RMGfxTargetBuffer &bigBuf, RMGfxPrimitive *prim);

	// Prepare a frame by drawing the location and all it's items
	void DoFrame(RMGfxTargetBuffer *bigBuf);

	// Return the item at a given point
	RMItem *WhichItemIsIn(const RMPoint &pt);

	// Return the item based on it's MPAL code
	RMItem *GetItemFromCode(uint32 dwCode);

	// Set the current scroll position
	void SetScrollPosition(const RMPoint &scroll);

	// Sets an additinal offset for scrolling
	void SetFixedScroll(const RMPoint &scroll);

	// Update the scrolling coordinates to display the specified point
	void UpdateScrolling(const RMPoint &ptShowThis);

	// Read the current scroll position
	RMPoint ScrollPosition() {
		return m_curScroll;
	}

	// Pause sound
	void PauseSound(bool bPause);
};


/**
 * MPAL message, composed of more ASCIIZ
 */
class RMMessage {
private:
	char *lpMessage;
	char *lpPeriods[256];
	int nPeriods;

private:
	void ParseMessage(void);

public:
	RMMessage();
	RMMessage(uint32 dwId);
	virtual ~RMMessage();

	void Load(uint32 dwId);
	bool IsValid() {
		return lpMessage != NULL;
	}
	int NumPeriods() {
		return nPeriods;
	}
	char *Period(int num) {
		return lpPeriods[num];
	}
	char *operator[](int num) {
		return lpPeriods[num];
	}
};

} // End of namespace Tony

#endif /* TONY_H */
