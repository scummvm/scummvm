/* ScummVM - Scumm Interpreter
 * Copyright (C) 2004 Ivan Dubrov
 * Copyright (C) 2004-2006 The ScummVM project
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

#pragma START_PACK_STRUCTS
#define szGame_ExtItem (4 + 2 + 2 + 2)
typedef struct Game_ExtItem {
	int32 offset;		// offset from the table end
	uint16 size;
	int16 width;		// width&0x7fff - width, width&0x8000 - pack flag
	int16 height;		// not zero
} GCC_PACK Game_ExtItem;

#define szGame_ExtTable (2 + 1)
typedef struct Game_ExtTable {
	int16 itemsCount;
	byte unknown;
	Game_ExtItem items[1];
} GCC_PACK Game_ExtTable;

#define szGame_TotResItem (4 + 2 + 2 + 2)
typedef struct Game_TotResItem {
	int32 offset;	// if > 0, then offset from end of resource table.
					// If < 0, then -offset-1 is index in .IM file table
	int16 size;
	int16 width;
	int16 height;
} GCC_PACK Game_TotResItem;

#define szGame_TotResTable (2 + 1)
typedef struct Game_TotResTable {
	int16 itemsCount;
	byte unknown;
	Game_TotResItem items[1];
} GCC_PACK Game_TotResTable;

#define szGame_TotTextItem (2 + 2)
typedef struct Game_TotTextItem {
	int16 offset;
	int16 size;
} GCC_PACK Game_TotTextItem;

#define szGame_TotTextTable (2)
typedef struct Game_TotTextTable {
	int16 itemsCount;
	Game_TotTextItem items[1];
} GCC_PACK Game_TotTextTable;

typedef struct Game_Collision {
	int16 id;
	int16 left;
	int16 top;
	int16 right;
	int16 bottom;
	int16 flags;
	int16 key;
	int16 funcEnter;
	int16 funcLeave;
} GCC_PACK Game_Collision;

typedef struct Game_InputDesc {
	int16 fontIndex;
	int16 backColor;
	int16 frontColor;
	char *ptr;
} GCC_PACK Game_InputDesc;
#pragma END_PACK_STRUCTS

extern Game_Collision *game_collisionAreas;

extern int16 game_lastCollKey;
extern int16 game_lastCollAreaIndex;
extern int16 game_lastCollId;

extern int16 game_activeCollResId;
extern int16 game_activeCollIndex;
extern char game_handleMouse;
extern char game_forceHandleMouse;

extern char game_tempStr[256];

extern Game_ExtTable *game_extTable;
extern char *game_totFileData;
extern Game_TotTextTable *game_totTextData;
extern Game_TotResTable *game_totResourceTable;
extern char *game_imFileData;
extern int16 game_extHandle;
extern char game_curExtFile[14];
extern char game_curTotFile[14];
extern char game_curImaFile[18];

extern int16 game_collStackSize;
extern Game_Collision *game_collStack[3];
extern int16 game_collStackElemSizes[3];

extern int16 game_mouseButtons;

extern Snd_SoundDesc *game_soundSamples[20];

extern char game_soundFromExt[20];
extern char game_totToLoad[20];

extern int32 game_startTimeKey;
extern char game_shouldPushColls;

// Functions

char *game_loadExtData(int16 dataId, int16 *pResWidth, int16 *pResHeight);
void game_clearCollisions(void);
void game_addNewCollision(int16 val_0, int16 left, int16 top, int16 right, int16 bottom,
    int16 flags, int16 key, int16 val_E, int16 val_10);
void game_freeCollision(int16 id);
char *game_loadTotResource(int16 id);
void game_capturePush(int16 left, int16 top, int16 width, int16 height);

void game_capturePush(int16 left, int16 top, int16 width, int16 height);
void game_capturePop(char doDraw);

void game_loadSound(int16 slot, char *dataPtr);
void game_interLoadSound(int16 slot);
void game_freeSoundSlot(int16 slot);
int16 game_checkKeys(int16 *pMousex, int16 *pMouseY, int16 *pButtons,
    char handleMouse);
int16 game_checkCollisions(char handleMouse, int16 deltaTime, int16 *pResId,
    int16 *pResIndex);
int16 game_inputArea(int16 xPos, int16 yPos, int16 width, int16 height, int16 backColor,
    int16 frontColor, char *str, int16 fontIndex, char inpType, int16 *pTotTime);
int16 game_multiEdit(int16 time, int16 index, int16 *pCurPos,
    Game_InputDesc * inpDesc);
int16 game_adjustKey(int16 key);
void game_collisionsBlock(void);
void game_prepareStart(void);
void game_loadTotFile(char *path);
void game_loadExtTable(void);
void game_loadImFile(void);
void game_playTot(int16 skipPlay);
void game_start(void);

}				// End of namespace Gob

#endif
