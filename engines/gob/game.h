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
 * $URL$
 * $Id$
 *
 */

#ifndef GOB_GAME_H
#define GOB_GAME_H

#include "gob/variables.h"

namespace Gob {

class Game {
public:

#include "common/pack-start.h"	// START STRUCT PACKING

	struct Collision {
		int16 id;
		uint16 left;
		uint16 top;
		uint16 right;
		uint16 bottom;
		int16 flags;
		int16 key;
		uint16 funcEnter;
		uint16 funcLeave;
		uint16 funcSub;
		byte *totFileData;
	} PACKED_STRUCT;

#define szGame_TotTextItem (2 + 2)
	struct TotTextItem {
		uint16 offset;
		int16 size;
	} PACKED_STRUCT;

#define szGame_TotTextTable (2)
	struct TotTextTable {
		int16 itemsCount;
		TotTextItem *items;
		byte *dataPtr;
	} PACKED_STRUCT;

	struct InputDesc {
		int16 fontIndex;
		int16 backColor;
		int16 frontColor;
		byte *ptr;
	} PACKED_STRUCT;

#include "common/pack-end.h"	// END STRUCT PACKING

	Collision *_collisionAreas;
	Collision *_collStack[5];

	bool _foundTotLoc;
	TotTextTable *_totTextData;

	char _curTotFile[14];
	char _curExtFile[14];

	byte *_imFileData;
	byte *_totFileData;

	int16 _extHandle;
	int16 _lomHandle;

	char _totToLoad[20];

	int32 _startTimeKey;
	int16 _mouseButtons;

	bool _noScroll;
	bool _preventScroll;
	bool _scrollHandleMouse;

	bool _noCd;

	Game(GobEngine *vm);
	virtual ~Game();

	byte *loadExtData(int16 dataId, int16 *pResWidth, int16 *pResHeight, uint32 *dataSize = 0);
	byte *loadTotResource(int16 id, int16 *dataSize = 0, int16 *width = 0, int16 *height = 0);

	void capturePush(int16 left, int16 top, int16 width, int16 height);
	void capturePop(char doDraw);

	void freeSoundSlot(int16 slot);

	void evaluateScroll(int16 x, int16 y);

	int16 checkKeys(int16 *pMousex = 0, int16 *pMouseY = 0,
			int16 *pButtons = 0, char handleMouse = 0);
	void start(void);

	virtual void totSub(int8 flags, const char *newTotFile);
	virtual void switchTotSub(int16 index, int16 skipPlay);

	void freeCollision(int16 id);

	virtual void playTot(int16 skipPlay) = 0;

	virtual void clearCollisions(void) = 0;
	virtual int16 addNewCollision(int16 id, uint16 left, uint16 top,
			uint16 right, uint16 bottom, int16 flags, int16 key,
			uint16 funcEnter, uint16 funcLeave, uint16 funcSub = 0) = 0;
	virtual void collisionsBlock(void) = 0;
	virtual int16 multiEdit(int16 time, int16 index, int16 *pCurPos,
			InputDesc *inpDesc, int16 *collResId,
			int16 *collIndex, bool mono = true) = 0;
	virtual int16 inputArea(int16 xPos, int16 yPos, int16 width, int16 height,
			int16 backColor, int16 frontColor, char *str, int16 fontIndex,
			char inpType, int16 *pTotTime, int16 *collResId,
			int16 *collIndex, bool mono = true) = 0;
	virtual int16 checkCollisions(byte handleMouse, int16 deltaTime,
			int16 *pResId, int16 *pResIndex) = 0;

	virtual void prepareStart(void) = 0;

	virtual void pushCollisions(char all) = 0;
	virtual void popCollisions(void) = 0;

protected:
#include "common/pack-start.h"	// START STRUCT PACKING

#define szGame_TotResItem (4 + 2 + 2 + 2)
	struct TotResItem {
		int32 offset;	// if > 0, then offset from end of resource table.
						// If < 0, then -offset-1 is index in .IM file table
		int16 size;
		int16 width;
		int16 height;
	} PACKED_STRUCT;

#define szGame_TotResTable (2 + 1)
	struct TotResTable {
		int16 itemsCount;
		byte unknown;
		TotResItem *items;
		byte *dataPtr;
	} PACKED_STRUCT;

#define szGame_ExtItem (4 + 2 + 2 + 2)
	struct ExtItem {
		int32 offset;		// offset from the table end
		uint16 size;
		int16 width;		// width & 0x7FFF: width, width & 0x8000: pack flag
		int16 height;		// not zero
	} PACKED_STRUCT;

#define szGame_ExtTable (2 + 1)
	struct ExtTable {
		int16 itemsCount;
		byte unknown;
		ExtItem *items;
	} PACKED_STRUCT;

#include "common/pack-end.h"	// END STRUCT PACKING

	int16 _lastCollKey;
	int16 _lastCollAreaIndex;
	int16 _lastCollId;

	int16 _activeCollResId;
	int16 _activeCollIndex;
	byte _handleMouse;
	char _forceHandleMouse;
	uint32 _menuLevel;

	char _tempStr[256];

	TotResTable *_totResourceTable;
	ExtTable *_extTable;
	char _curImaFile[18];

	int16 _collStackSize;
	int16 _collStackElemSizes[5];

	char _shouldPushColls;

	// Capture
	Common::Rect _captureStack[20];
	int16 _captureCount;

	char _collStr[256];

	// For totSub()
	int8 _backupedCount;
	int8 _curBackupPos;
	int16 _cursorHotspotXArray[5];
	int16 _cursorHotspotYArray[5];
	TotTextTable *_totTextDataArray[5];
	byte *_totFileDataArray[5];
	TotResTable *_totResourceTableArray[5];
	ExtTable *_extTableArray[5];
	int16 _extHandleArray[5];
	byte *_imFileDataArray[5];
	Variables *_variablesArray[5];
	char _curTotFileArray[5][14];

	GobEngine *_vm;

	virtual int16 adjustKey(int16 key);

	byte *loadLocTexts(int32 *dataSize = 0);
	int32 loadTotFile(const char *path);
	void loadExtTable(void);
	void loadImFile(void);

	void collAreaSub(int16 index, int8 enter);
	bool getLocTextFile(char *locTextFile, int language);

	virtual void setCollisions(byte arg_0 = 1);
	virtual void collSub(uint16 offset);

	virtual int16 checkMousePoint(int16 all, int16 *resId, int16 *resIndex) = 0;
};

class Game_v1 : public Game {
public:
	virtual void playTot(int16 skipPlay);

	virtual void clearCollisions(void);
	virtual int16 addNewCollision(int16 id, uint16 left, uint16 top,
			uint16 right, uint16 bottom, int16 flags, int16 key,
			uint16 funcEnter, uint16 funcLeave, uint16 funcSub = 0);
	virtual void collisionsBlock(void);
	virtual int16 multiEdit(int16 time, int16 index, int16 *pCurPos,
			InputDesc *inpDesc, int16 *collResId,
			int16 *collIndex, bool mono = true);
	virtual int16 inputArea(int16 xPos, int16 yPos, int16 width, int16 height,
			int16 backColor, int16 frontColor, char *str, int16 fontIndex,
			char inpType, int16 *pTotTime, int16 *collResId,
			int16 *collIndex, bool mono = true);
	virtual int16 checkCollisions(byte handleMouse, int16 deltaTime,
			int16 *pResId, int16 *pResIndex);

	virtual void prepareStart(void);

	virtual void pushCollisions(char all);
	virtual void popCollisions(void);

	Game_v1(GobEngine *vm);
	virtual ~Game_v1() {}

protected:
	virtual int16 checkMousePoint(int16 all, int16 *resId, int16 *resIndex);
};

class Game_v2 : public Game_v1 {
public:
	virtual void playTot(int16 skipPlay);

	virtual void clearCollisions(void);
	virtual int16 addNewCollision(int16 id, uint16 left, uint16 top,
			uint16 right, uint16 bottom, int16 flags, int16 key,
			uint16 funcEnter, uint16 funcLeave, uint16 funcSub = 0);
	virtual void collisionsBlock(void);
	virtual int16 multiEdit(int16 time, int16 index, int16 *pCurPos,
			InputDesc *inpDesc, int16 *collResId,
			int16 *collIndex, bool mono = true);
	virtual int16 inputArea(int16 xPos, int16 yPos, int16 width, int16 height,
			int16 backColor, int16 frontColor, char *str, int16 fontIndex,
			char inpType, int16 *pTotTime, int16 *collResId,
			int16 *collIndex, bool mono = true);
	virtual int16 checkCollisions(byte handleMouse, int16 deltaTime,
			int16 *pResId, int16 *pResIndex);

	virtual void prepareStart(void);

	virtual void pushCollisions(char all);
	virtual void popCollisions(void);

	Game_v2(GobEngine *vm);
	virtual ~Game_v2() {}

protected:
	struct CollLast {
		int16 key;
		int16 id;
		int16 areaIndex;
	};

	CollLast _collLasts[5];

	virtual int16 checkMousePoint(int16 all, int16 *resId, int16 *resIndex);
};

class Game_v6 : public Game_v2 {
public:
	virtual void totSub(int8 flags, const char *newTotFile);

	virtual int16 addNewCollision(int16 id, uint16 left, uint16 top,
			uint16 right, uint16 bottom, int16 flags, int16 key,
			uint16 funcEnter, uint16 funcLeave, uint16 funcSub = 0);

	virtual void prepareStart(void);

	virtual void pushCollisions(char all);

	virtual int16 checkCollisions(byte handleMouse, int16 deltaTime,
			int16 *pResId, int16 *pResIndex);
	virtual void collisionsBlock(void);

	Game_v6(GobEngine *vm);
	virtual ~Game_v6() {}

protected:
	uint32 _someTimeDly;

	virtual void setCollisions(byte arg_0 = 1);
	virtual void collSub(uint16 offset);

	virtual int16 adjustKey(int16 key);

	virtual int16 checkMousePoint(int16 all, int16 *resId, int16 *resIndex);

	void collSubReenter();
};

class Game_Fascination : public Game_v2 {
public:
	virtual int16 checkCollisions(byte handleMouse, int16 deltaTime,
			int16 *pResId, int16 *pResIndex);

	Game_Fascination(GobEngine *vm);
	virtual ~Game_Fascination() {}
};

} // End of namespace Gob

#endif // GOB_GAME_H
