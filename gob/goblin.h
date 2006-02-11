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
 * $URL$
 * $Id$
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

class Goblin {
public:
#pragma START_PACK_STRUCTS
	struct Gob_State {
		int16 animation;// +0h
		int16 layer;	// +2h
		int16 unk0;		// +4h
		int16 unk1;		// +6h
		int16 sndItem;	// +8h, high/low byte - sound sample index
		int16 freq;		// +Ah, high/low byte * 100 - frequency
		int16 repCount;	// +Ch high/low byte - repeat count
		int16 sndFrame;		// +Eh
	} GCC_PACK;

	typedef Gob_State *Gob_PState;

	typedef Gob_PState Gob_StateLine[6];

	struct Gob_Object {
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
	} GCC_PACK;

	struct Gob_Pos {
		char x;
		char y;
	} GCC_PACK;
#pragma END_PACK_STRUCTS

	Util::List *_objList;
	Gob_Object *_goblins[4];
	int16 _currentGoblin;
	Snd::SoundDesc *_soundData[16];
	int16 _gobStateLayer;
	char _goesAtTarget;
	char _readyToAct;
	int16 _gobAction;	// 0 - move, 3 - do action, 4 - pick
						// goblins  0 - picker, 1 - fighter, 2 - mage
	Gob_Pos _gobPositions[3];
	int16 _gobDestX;
	int16 _gobDestY;
	int16 _pressedMapX;
	int16 _pressedMapY;
	char _pathExistence;

	// Pointers to interpreter variables
	int32 *_some0ValPtr;

	int32 *_gobRetVarPtr;
	int32 *_curGobVarPtr;
	int32 *_curGobXPosVarPtr;
	int32 *_curGobYPosVarPtr;
	int32 *_itemInPocketVarPtr;

	int32 *_curGobStateVarPtr;
	int32 *_curGobFrameVarPtr;
	int32 *_curGobMultStateVarPtr;
	int32 *_curGobNextStateVarPtr;
	int32 *_curGobScrXVarPtr;
	int32 *_curGobScrYVarPtr;
	int32 *_curGobLeftVarPtr;
	int32 *_curGobTopVarPtr;
	int32 *_curGobRightVarPtr;
	int32 *_curGobBottomVarPtr;
	int32 *_curGobDoAnimVarPtr;
	int32 *_curGobOrderVarPtr;
	int32 *_curGobNoTickVarPtr;
	int32 *_curGobTypeVarPtr;
	int32 *_curGobMaxTickVarPtr;
	int32 *_curGobTickVarPtr;
	int32 *_curGobActStartStateVarPtr;
	int32 *_curGobLookDirVarPtr;
	int32 *_curGobPickableVarPtr;
	int32 *_curGobRelaxVarPtr;
	int32 *_curGobMaxFrameVarPtr;

	int32 *_destItemStateVarPtr;
	int32 *_destItemFrameVarPtr;
	int32 *_destItemMultStateVarPtr;
	int32 *_destItemNextStateVarPtr;
	int32 *_destItemScrXVarPtr;
	int32 *_destItemScrYVarPtr;
	int32 *_destItemLeftVarPtr;
	int32 *_destItemTopVarPtr;
	int32 *_destItemRightVarPtr;
	int32 *_destItemBottomVarPtr;
	int32 *_destItemDoAnimVarPtr;
	int32 *_destItemOrderVarPtr;
	int32 *_destItemNoTickVarPtr;
	int32 *_destItemTypeVarPtr;
	int32 *_destItemMaxTickVarPtr;
	int32 *_destItemTickVarPtr;
	int32 *_destItemActStartStVarPtr;
	int32 *_destItemLookDirVarPtr;
	int32 *_destItemPickableVarPtr;
	int32 *_destItemRelaxVarPtr;
	int32 *_destItemMaxFrameVarPtr;

	int16 _destItemType;
	int16 _destItemState;
	int16 _itemToObject[20];
	Gob_Object *_objects[20];
	int16 _objCount;
	int16 _gobsCount;
	int16 _itemIndInPocket;
	int16 _itemIdInPocket;
	char _itemByteFlag;
	int16 _destItemId;
	int16 _destActionItem;
	Gob_Object *_actDestItemDesc;
	int16 _forceNextState[10];
	char _boreCounter;
	int16 _positionedGob;
	char _noPick;

	// Functions
	char rotateState(int16 from, int16 to);
	void playSound(Snd::SoundDesc * snd, int16 repCount, int16 freq);
	void drawObjects(void);
	void animateObjects(void);
	void placeObject(Gob_Object * objDesc, char animated);
	int16 getObjMaxFrame(Gob_Object * obj);
	int16 objIntersected(Gob_Object * obj1, Gob_Object * obj2);
	void setMultStates(Gob_Object * gobDesc);
	int16 nextLayer(Gob_Object * gobDesc);
	void showBoredom(int16 gobIndex);
	void switchGoblin(int16 index);
	void freeObjects(void);
	void zeroObjects(void);
	void freeAllObjects(void);
	void loadObjects(char *source);
	void initVarPointers(void);
	void saveGobDataToVars(int16 xPos, int16 yPos, int16 someVal);
	void loadGobDataFromVars(void);
	void pickItem(int16 indexToPocket, int16 idToPocket);
	void placeItem(int16 indexInPocket, int16 idInPocket);
	void swapItems(int16 indexToPick, int16 idToPick);
	void treatItemPick(int16 itemId);
	int16 treatItem(int16 action);
	int16 doMove(Gob_Object *gobDesc, int16 cont, int16 action);

	Goblin(GobEngine *vm);

protected:
	int16 _rotStates[4][4];
	GobEngine *_vm;

	int16 peekGoblin(Gob_Object *curGob);
	void initList(void);
	void sortByOrder(Util::List *list);
	void adjustDest(int16 posX, int16 posY);
	void adjustTarget(void);
	void targetDummyItem(Gob_Object *gobDesc);
	void targetItem(void);
	void initiateMove(void);
	void moveFindItem(int16 posX, int16 posY);
	void moveCheckSelect(int16 framesCount, Gob_Object * gobDesc, int16 *pGobIndex, int16 *nextAct);
	void moveInitStep(int16 framesCount, int16 action, int16 cont,
					  Gob_Object *gobDesc, int16 *pGobIndex, int16 *pNextAct);
	void moveTreatRopeStairs(Gob_Object *gobDesc);
	void movePathFind(Gob_Object *gobDesc, int16 nextAct);
	void moveAdvance(Gob_Object *gobDesc, int16 nextAct, int16 framesCount);
};

}				// End of namespace Gob

#endif	/* __GOBLIN_H */
