/* ScummVM - Scumm Interpreter
 * Copyright (C) 2006 The ScummVM project
 *
 * cinE Engine is (C) 2004-2005 by CinE Team
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

#ifndef CINE_SCRIPT_H_
#define CINE_SCRIPT_H_

namespace Cine {

#define SCRIPT_STACK_SIZE 50

struct scriptStruct {
	byte *ptr;
	uint16 size;
	int16 stack[SCRIPT_STACK_SIZE];
};

#define NUM_MAX_SCRIPT 50

extern scriptStruct scriptTable[NUM_MAX_SCRIPT];

 void setupOpcodes(); 

void computeScriptStack(byte *scriptPtr, int16 *stackPtr, uint16 scriptSize);
void decompileScript(byte *scriptPtr, int16 *stackPtr, uint16 scriptSize, uint16 scriptIdx);
void dumpScript(char *dumpName);

#define OP_loadPart                     0x3F
#define OP_loadNewPrcName               0x41
#define OP_requestCheckPendingDataLoad  0x42
#define OP_endScript                    0x50
#define OP_changeDataDisk               0x6B
#define OP_79                           0x79

void addScriptToList0(uint16 idx);
int16 checkCollision(int16 objIdx, int16 x, int16 y, int16 numZones, int16 zoneIdx);

void runObjectScript(int16 entryIdx);
int16 stopObjectScript(int16 entryIdx);

void executeList1(void);
void executeList0(void);

void purgeList1(void);
void purgeList0(void);

void o1_modifyObjectParam();
void o1_getObjectParam();
void o1_addObjectParam();
void o1_subObjectParam();
void o1_add2ObjectParam();
void o1_sub2ObjectParam();
void o1_compareObjectParam();
void o1_setupObject();
void o1_checkCollision();
void o1_loadVar();
void o1_addVar();
void o1_subVar();
void o1_mulVar();
void o1_divVar();
void o1_compareVar();
void o1_modifyObjectParam2();
void o1_loadMask0();
void o1_unloadMask0();
void o1_addToBgList();
void o1_loadMask1();
void o1_unloadMask1();
void o1_loadMask4();
void o1_unloadMask4();
void o1_addSpriteFilledToBgList();
void o1_op1B();
void o1_label();
void o1_goto();
void o1_gotoIfSup();
void o1_gotoIfSupEqu();
void o1_gotoIfInf();
void o1_gotoIfInfEqu();
void o1_gotoIfEqu();
void o1_gotoIfDiff();
void o1_removeLabel();
void o1_loop();
void o1_startGlobalScript();
void o1_endGlobalScript();
void o1_loadAnim();
void o1_loadBg();
void o1_loadCt();
void o1_loadPart();
void o1_closePart();
void o1_loadNewPrcName();
void o1_requestCheckPendingDataLoad();
void o1_blitAndFade();
void o1_fadeToBlack();
void o1_transformPaletteRange();
void o1_setDefaultMenuColor2();
void o1_palRotate();
void o1_break();
void o1_endScript();
void o1_message();
void o1_loadGlobalVar();
void o1_compareGlobalVar();
void o1_declareFunctionName();
void o1_freePartRange();
void o1_unloadAllMasks();
void o1_op63();
void o1_op64();
void o1_initializeZoneData();
void o1_setZoneDataEntry();
void o1_getZoneDataEntry();
void o1_setDefaultMenuColor();
void o1_allowPlayerInput();
void o1_disallowPlayerInput();
void o1_changeDataDisk();
void o1_loadMusic();
void o1_playMusic();
void o1_fadeOutMusic();
void o1_stopSample();
void o1_op71();
void o1_op72();
void o1_op73();
void o1_playSample();
void o1_playSample();
void o1_allowSystemMenu();
void o1_loadMask5();
void o1_unloadMask5();

void o2_loadPart();
void o2_addSeqListElement();
void o2_removeSeq();
void o2_op78(); 
void o2_op81();
void o2_op82();
void o2_isSeqRunning();
void o2_gotoIfSupNearest();
void o2_gotoIfSupEquNearest();
void o2_gotoIfInfNearest();
void o2_gotoIfInfEquNearest();
void o2_gotoIfEquNearest();
void o2_gotoIfDiffNearest();
void o2_startObjectScript();
void o2_stopObjectScript();
void o2_op8D();
void o2_addBackground();
void o2_removeBackground();
void o2_loadAbs();
void o2_loadBg();
void o2_wasZoneChecked();
void o2_op9B();
void o2_op9C();
void o2_useBgScroll();
void o2_setAdditionalBgVScroll();
void o2_op9F();
void o2_addGfxElementA0();
void o2_opA1();
void o2_opA2();
void o2_opA3();
void o2_loadMask22();
void o2_unloadMask22();

} // End of namespace Cine

#endif
