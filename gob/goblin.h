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
#ifndef GOB_GOBLIN_H
#define GOB_GOBLIN_H

#include "gob/util.h"
#include "gob/sound.h"

namespace Gob {

#define TYPE_USUAL		0
#define TYPE_AMORPHOUS	1
#define TYPE_MOBILE		3

#pragma START_PACK_STRUCTS
typedef struct Gob_State {
	int16 animation;// +0h
	int16 layer;	// +2h
	int16 unk0;		// +4h
	int16 unk1;		// +6h
	int16 sndItem;	// +8h, high/low byte - sound sample index
	int16 freq;		// +Ah, high/low byte * 100 - frequency
	int16 repCount;	// +Ch high/low byte - repeat count
	int16 sndFrame;		// +Eh
} GCC_PACK Gob_State;

typedef struct Gob_State *Gob_PState;

#define szGob_StateLine 24
typedef Gob_PState Gob_StateLine[6];

typedef struct Gob_Object {
	int16 animation;	// +0h
	int16 state;		// +2h
	int16 stateColumn;	// +4h
	int16 curFrame;		// +6h
	int16 xPos;			// +8h
	int16 yPos;			// +Ah
	int16 dirtyLeft;	// +Ch
	int16 dirtyTop;		// +Eh
	int16 dirtyRight;	// +10h
	int16 dirtyBottom;	// +12h
	int16 left;			// +14h
	int16 top;			// +16h
	int16 right;		// +18h
	int16 bottom;		// +1ah
	int16 nextState;	// +1ch
	int16 multState;	// +1eh
	int16 actionStartState;	// +20h
	int16 curLookDir;	// +22h
	int16 pickable;		// +24h
	int16 relaxTime;	// +26h
	Gob_StateLine *stateMach;	// +28h
	Gob_StateLine *realStateMach;	// +2ch
	char doAnim;		// +30h
	char order;			// +31h
	char noTick;		// +32h
	char toRedraw;		// +33h
	char type;			// +34h
	char maxTick;		// +35h
	char tick;			// +36h
	char multObjIndex;	// +37h, from which play mult animations
	char unk14;			// +38h
	char visible;		// +39h
} GCC_PACK Gob_Object;

typedef struct Gob_Pos {
	char x;
	char y;
} GCC_PACK Gob_Pos;
#pragma END_PACK_STRUCTS

extern Util_List *gob_objList;
extern Gob_Object *gob_goblins[4];
extern int16 gob_currentGoblin;
extern Snd_SoundDesc *gob_soundData[16];
extern int16 gob_gobStateLayer;
extern char gob_goesAtTarget;
extern char gob_readyToAct;
extern int16 gob_gobAction;	// 0 - move, 3 - do action, 4 - pick
						  // goblins  0 - picker, 1 - fighter, 2 - mage
extern Gob_Pos gob_gobPositions[3];
extern int16 gob_gobDestX;
extern int16 gob_gobDestY;
extern int16 gob_pressedMapX;
extern int16 gob_pressedMapY;
extern char gob_pathExistence;

// Pointers to interpreter variables
extern int32 *gob_some0ValPtr;

extern int32 *gob_gobRetVarPtr;
extern int32 *gob_curGobVarPtr;
extern int32 *gob_curGobXPosVarPtr;
extern int32 *gob_curGobYPosVarPtr;
extern int32 *gob_itemInPocketVarPtr;

extern int32 *gob_curGobStateVarPtr;
extern int32 *gob_curGobFrameVarPtr;
extern int32 *gob_curGobMultStateVarPtr;
extern int32 *gob_curGobNextStateVarPtr;
extern int32 *gob_curGobScrXVarPtr;
extern int32 *gob_curGobScrYVarPtr;
extern int32 *gob_curGobLeftVarPtr;
extern int32 *gob_curGobTopVarPtr;
extern int32 *gob_curGobRightVarPtr;
extern int32 *gob_curGobBottomVarPtr;
extern int32 *gob_curGobDoAnimVarPtr;
extern int32 *gob_curGobOrderVarPtr;
extern int32 *gob_curGobNoTickVarPtr;
extern int32 *gob_curGobTypeVarPtr;
extern int32 *gob_curGobMaxTickVarPtr;
extern int32 *gob_curGobTickVarPtr;
extern int32 *gob_curGobActStartStateVarPtr;
extern int32 *gob_curGobLookDirVarPtr;
extern int32 *gob_curGobPickableVarPtr;
extern int32 *gob_curGobRelaxVarPtr;
extern int32 *gob_curGobMaxFrameVarPtr;

extern int32 *gob_destItemStateVarPtr;
extern int32 *gob_destItemFrameVarPtr;
extern int32 *gob_destItemMultStateVarPtr;
extern int32 *gob_destItemNextStateVarPtr;
extern int32 *gob_destItemScrXVarPtr;
extern int32 *gob_destItemScrYVarPtr;
extern int32 *gob_destItemLeftVarPtr;
extern int32 *gob_destItemTopVarPtr;
extern int32 *gob_destItemRightVarPtr;
extern int32 *gob_destItemBottomVarPtr;
extern int32 *gob_destItemDoAnimVarPtr;
extern int32 *gob_destItemOrderVarPtr;
extern int32 *gob_destItemNoTickVarPtr;
extern int32 *gob_destItemTypeVarPtr;
extern int32 *gob_destItemMaxTickVarPtr;
extern int32 *gob_destItemTickVarPtr;
extern int32 *gob_destItemActStartStVarPtr;
extern int32 *gob_destItemLookDirVarPtr;
extern int32 *gob_destItemPickableVarPtr;
extern int32 *gob_destItemRelaxVarPtr;
extern int32 *gob_destItemMaxFrameVarPtr;

extern int16 gob_destItemType;
extern int16 gob_destItemState;
extern int16 gob_itemToObject[20];
extern Gob_Object *gob_objects[20];
extern int16 gob_objCount;
extern int16 gob_gobsCount;
extern int16 gob_itemIndInPocket;
extern int16 gob_itemIdInPocket;
extern char gob_itemByteFlag;
extern int16 gob_destItemId;
extern int16 gob_destActionItem;
extern Gob_Object *gob_actDestItemDesc;
extern int16 gob_forceNextState[10];
extern char gob_boreCounter;
extern int16 gob_positionedGob;
extern char gob_noPick;

// Functions
char gob_rotateState(int16 from, int16 to);
void gob_playSound(Snd_SoundDesc * snd, int16 repCount, int16 freq);
void gob_drawObjects(void);
void gob_animateObjects(void);
void gob_placeObject(Gob_Object * objDesc, char animated);
int16 gob_getObjMaxFrame(Gob_Object * obj);
int16 gob_objIntersected(Gob_Object * obj1, Gob_Object * obj2);
void gob_setMultStates(Gob_Object * gobDesc);
int16 gob_nextLayer(Gob_Object * gobDesc);
void gob_showBoredom(int16 gobIndex);
void gob_switchGoblin(int16 index);
void gob_freeObjects(void);
void gob_zeroObjects(void);
void gob_freeAllObjects(void);
void gob_loadObjects(char *source);
void gob_initVarPointers(void);
void gob_saveGobDataToVars(int16 xPos, int16 yPos, int16 someVal);
void gob_loadGobDataFromVars(void);
void gob_pickItem(int16 indexToPocket, int16 idToPocket);
void gob_placeItem(int16 indexInPocket, int16 idInPocket);
void gob_swapItems(int16 indexToPick, int16 idToPick);
void gob_treatItemPick(int16 itemId);
int16 gob_treatItem(int16 action);
void gob_interFunc(void);

}				// End of namespace Gob

#endif	/* __GOBLIN_H */
