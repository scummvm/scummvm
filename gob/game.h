/* ScummVM - Scumm Interpreter
 * Copyright (C) 2004 Ivan Dubrov
 * Copyright (C) 2004-2005 The ScummVM project
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
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.
 *
 * $Header$
 *
 */
#ifndef GOB_GAME_H
#define GOB_GAME_H

#include "gob/sound.h"

namespace Gob {

class Game {
public:

#pragma START_PACK_STRUCTS
#define szGame_TotResItem (4 + 2 + 2 + 2)
	typedef struct Collision {
		int16 id;
		int16 left;
		int16 top;
		int16 right;
		int16 bottom;
		int16 flags;
		int16 key;
		int16 funcEnter;
		int16 funcLeave;
	} GCC_PACK Collision;

	typedef struct TotResItem {
		int32 offset;	// if > 0, then offset from end of resource table.
						// If < 0, then -offset-1 is index in .IM file table
		int16 size;
		int16 width;
		int16 height;
	} GCC_PACK TotResItem;

#define szGame_TotResTable (2 + 1)
	typedef struct TotResTable {
		int16 itemsCount;
		byte unknown;
		TotResItem items[1];
	} GCC_PACK TotResTable;

#define szGame_ExtItem (4 + 2 + 2 + 2)
	typedef struct ExtItem {
		int32 offset;		// offset from the table end
		uint16 size;
		int16 width;		// width&0x7fff - width, width&0x8000 - pack flag
		int16 height;		// not zero
	} GCC_PACK ExtItem;

#define szGame_ExtTable (2 + 1)
	typedef struct ExtTable {
		int16 itemsCount;
		byte unknown;
		ExtItem items[1];
	} GCC_PACK ExtTable;

#define szGame_TotTextItem (2 + 2)
	typedef struct TotTextItem {
		int16 offset;
		int16 size;
	} GCC_PACK TotTextItem;

#define szGame_TotTextTable (2)
	typedef struct TotTextTable {
		int16 itemsCount;
		TotTextItem items[1];
	} GCC_PACK TotTextTable;

	typedef struct InputDesc {
		int16 fontIndex;
		int16 backColor;
		int16 frontColor;
		char *ptr;
	} GCC_PACK InputDesc;
#pragma END_PACK_STRUCTS

	TotResTable *_totResourceTable;
	Collision *_collisionAreas;
	Collision *_collStack[3];

	TotTextTable *_totTextData;

	char _curTotFile[14];
	char _curExtFile[14];

	char *_imFileData;
	char *_totFileData;

	int16 _extHandle;

	Snd::SoundDesc *_soundSamples[20];

	char _totToLoad[20];

	int32 _startTimeKey;
	int16 _mouseButtons;

	Game(GobEngine *vm);

	char *loadExtData(int16 dataId, int16 *pResWidth, int16 *pResHeight);
	char *loadTotResource(int16 id);

	void capturePush(int16 left, int16 top, int16 width, int16 height);

	void capturePop(char doDraw);
	void interLoadSound(int16 slot);
	void freeSoundSlot(int16 slot);
	int16 checkKeys(int16 *pMousex, int16 *pMouseY, int16 *pButtons,
					char handleMouse);
	int16 checkCollisions(char handleMouse, int16 deltaTime, int16 *pResId,
						  int16 *pResIndex);
	void clearCollisions(void);
	void addNewCollision(int16 val_0, int16 left, int16 top, int16 right, int16 bottom,
						 int16 flags, int16 key, int16 val_E, int16 val_10);
	void freeCollision(int16 id);

	void loadSound(int16 slot, char *dataPtr);
	int16 inputArea(int16 xPos, int16 yPos, int16 width, int16 height, int16 backColor,
			int16 frontColor, char *str, int16 fontIndex, char inpType, int16 *pTotTime);
	int16 multiEdit(int16 time, int16 index, int16 *pCurPos,
					InputDesc * inpDesc);
	int16 adjustKey(int16 key);
	void collisionsBlock(void);
	void prepareStart(void);
	void loadTotFile(char *path);
	void loadExtTable(void);
	void loadImFile(void);
	void playTot(int16 skipPlay);
	void start(void);

protected:

	int16 _lastCollKey;
	int16 _lastCollAreaIndex;
	int16 _lastCollId;

	int16 _activeCollResId;
	int16 _activeCollIndex;
	char _handleMouse;
	char _forceHandleMouse;

	char _tempStr[256];

	ExtTable *_extTable;
	char _curImaFile[18];

	int16 _collStackSize;
	int16 _collStackElemSizes[3];

	char _soundFromExt[20];

	char _shouldPushColls;

	// Capture
	static Common::Rect _captureStack[20];
	static int16 _captureCount;

	char _collStr[256];

	GobEngine *_vm;
		
	void pushCollisions(char all);
	void popCollisions(void);
	int16 checkMousePoint(int16 all, int16 *resId, int16 *resIndex);
};

}				// End of namespace Gob

#endif
