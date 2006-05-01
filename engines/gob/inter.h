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
#ifndef GOB_INTERPRET_H
#define GOB_INTERPRET_H

#include "gob/goblin.h"

namespace Gob {

// This is to help devices with small memory (PDA, smartphones, ...)
// to save abit of memory used by opcode names in the Scumm engine.
#ifndef REDUCE_MEMORY_USAGE
# define _OPCODE(ver, x)  { &ver::x, #x }
#else
# define _OPCODE(ver, x)  { &ver::x, "" }
#endif

class Inter {
public:
	int16 _animPalLowIndex;
	int16 _animPalHighIndex;
	int16 _animPalDir;
	uint32 _soundEndTimeKey;
	int16 _soundStopVal;
	char _terminate;
	char _breakFlag;
	int16 *_breakFromLevel;
	int16 *_nestLevel;

	int16 load16(void);
	int16 peek16(char *ptr);
	int32 peek32(char *ptr);

	char evalExpr(int16 *pRes);
	char evalBoolResult(void);
	void animPalette(void);
	void funcBlock(int16 retFlag);
	void storeKey(int16 key);
	void checkSwitchTable(char **ppExec);
	void callSub(int16 retFlag);
	void initControlVars(void);
	void renewTimeInVars(void);
	void manipulateMap(int16 xPos, int16 yPos, int16 item);
	virtual int16 loadSound(int16 slot) = 0;
	virtual void storeMouse(void) = 0;

	Inter(GobEngine *vm);
	virtual ~Inter() {};

protected:
	GobEngine *_vm;

	virtual void setupOpcodes(void) = 0;
	virtual void executeDrawOpcode(byte i) = 0;
	virtual bool executeFuncOpcode(byte i, byte j, char &cmdCount, int16 &counter, int16 &retFlag) = 0;
	virtual void executeGoblinOpcode(int i, int16 &extraData, int32 *retVarPtr, Goblin::Gob_Object *objDesc) = 0;
	virtual const char *getOpcodeDrawDesc(byte i) = 0;
	virtual const char *getOpcodeFuncDesc(byte i, byte j) = 0;
	virtual const char *getOpcodeGoblinDesc(int i) = 0;
	virtual void loadMult(void) = 0;
};

class Inter_v1 : public Inter {
public:
	Inter_v1(GobEngine *vm);
	virtual ~Inter_v1() {};
	virtual int16 loadSound(int16 slot);
	virtual void storeMouse(void);

protected:
	typedef void (Inter_v1::*OpcodeDrawProcV1)(void);
	typedef bool (Inter_v1::*OpcodeFuncProcV1)(char &, int16 &, int16 &);
	typedef void (Inter_v1::*OpcodeGoblinProcV1)(int16 &, int32 *, Goblin::Gob_Object *);
  struct OpcodeDrawEntryV1 {
		OpcodeDrawProcV1 proc;
		const char *desc;
		};
  struct OpcodeFuncEntryV1 {
		OpcodeFuncProcV1 proc;
		const char *desc;
		};
  struct OpcodeGoblinEntryV1 {
		OpcodeGoblinProcV1 proc;
		const char *desc;
		};
	const OpcodeDrawEntryV1 *_opcodesDrawV1;
	const OpcodeFuncEntryV1 *_opcodesFuncV1;
	const OpcodeGoblinEntryV1 *_opcodesGoblinV1;
	static const int _goblinFuncLookUp[][2];

	virtual void setupOpcodes(void);
	virtual void executeDrawOpcode(byte i);
	virtual bool executeFuncOpcode(byte i, byte j, char &cmdCount, int16 &counter, int16 &retFlag);
	virtual void executeGoblinOpcode(int i, int16 &extraData, int32 *retVarPtr, Goblin::Gob_Object *objDesc);
	virtual const char *getOpcodeDrawDesc(byte i);
	virtual const char *getOpcodeFuncDesc(byte i, byte j);
	virtual const char *getOpcodeGoblinDesc(int i);
	virtual void loadMult(void);

	void o1_loadMult(void);
	void o1_playMult(void);
	void o1_freeMult(void);
	void o1_initCursor(void);
	void o1_initCursorAnim(void);
	void o1_clearCursorAnim(void);
	void o1_setRenderFlags(void);
	void o1_loadAnim(void);
	void o1_freeAnim(void);
	void o1_updateAnim(void);
	void o1_initMult(void);
	void o1_multFreeMult(void);
	void o1_animate(void);
	void o1_multLoadMult(void);
	void o1_storeParams(void);
	void o1_getObjAnimSize(void);
	void o1_loadStatic(void);
	void o1_freeStatic(void);
	void o1_renderStatic(void);
	void o1_loadCurLayer(void);
	void o1_playCDTrack(void);
	void o1_getCDTrackPos(void);
	void o1_stopCD(void);
	void o1_loadFontToSprite(void);
	void o1_freeFontToSprite(void);
	bool o1_callSub(char &cmdCount, int16 &counter, int16 &retFlag);
	bool o1_drawPrintText(char &cmdCount, int16 &counter, int16 &retFlag);
	bool o1_call(char &cmdCount, int16 &counter, int16 &retFlag);
	bool o1_callBool(char &cmdCount, int16 &counter, int16 &retFlag);
	bool o1_loadCursor(char &cmdCount, int16 &counter, int16 &retFlag);
	bool o1_repeatUntil(char &cmdCount, int16 &counter, int16 &retFlag);
	bool o1_whileDo(char &cmdCount, int16 &counter, int16 &retFlag);
	bool o1_evaluateStore(char &cmdCount, int16 &counter, int16 &retFlag);
	bool o1_loadSpriteToPos(char &cmdCount, int16 &counter, int16 &retFlag);
	bool o1_printText(char &cmdCount, int16 &counter, int16 &retFlag);
	bool o1_loadTot(char &cmdCount, int16 &counter, int16 &retFlag);
	bool o1_palLoad(char &cmdCount, int16 &counter, int16 &retFlag);
	bool o1_keyFunc(char &cmdCount, int16 &counter, int16 &retFlag);
	bool o1_capturePush(char &cmdCount, int16 &counter, int16 &retFlag);
	bool o1_capturePop(char &cmdCount, int16 &counter, int16 &retFlag);
	bool o1_animPalInit(char &cmdCount, int16 &counter, int16 &retFlag);
	bool o1_drawOperations(char &cmdCount, int16 &counter, int16 &retFlag);
	bool o1_renewTimeInVars(char &cmdCount, int16 &counter, int16 &retFlag);
	bool o1_putPixel(char &cmdCount, int16 &counter, int16 &retFlag);
	bool o1_createSprite(char &cmdCount, int16 &counter, int16 &retFlag);
	bool o1_freeSprite(char &cmdCount, int16 &counter, int16 &retFlag);
	bool o1_loadSpriteContent(char &cmdCount, int16 &counter, int16 &retFlag);
	bool o1_copySprite(char &cmdCount, int16 &counter, int16 &retFlag);
	bool o1_fillRect(char &cmdCount, int16 &counter, int16 &retFlag);
	bool o1_drawLine(char &cmdCount, int16 &counter, int16 &retFlag);
	bool o1_strToLong(char &cmdCount, int16 &counter, int16 &retFlag);
	bool o1_invalidate(char &cmdCount, int16 &counter, int16 &retFlag);
	bool o1_playSound(char &cmdCount, int16 &counter, int16 &retFlag);
	bool o1_stopSound(char &cmdCount, int16 &counter, int16 &retFlag);
	bool o1_playComposition(char &cmdCount, int16 &counter, int16 &retFlag);
	bool o1_getFreeMem(char &cmdCount, int16 &counter, int16 &retFlag);
	bool o1_checkData(char &cmdCount, int16 &counter, int16 &retFlag);
	bool o1_prepareStr(char &cmdCount, int16 &counter, int16 &retFlag);
	bool o1_insertStr(char &cmdCount, int16 &counter, int16 &retFlag);
	bool o1_cutStr(char &cmdCount, int16 &counter, int16 &retFlag);
	bool o1_strstr(char &cmdCount, int16 &counter, int16 &retFlag);
	bool o1_istrlen(char &cmdCount, int16 &counter, int16 &retFlag);
	bool o1_setMousePos(char &cmdCount, int16 &counter, int16 &retFlag);
	bool o1_setFrameRate(char &cmdCount, int16 &counter, int16 &retFlag);
	bool o1_loadFont(char &cmdCount, int16 &counter, int16 &retFlag);
	bool o1_freeFont(char &cmdCount, int16 &counter, int16 &retFlag);
	bool o1_readData(char &cmdCount, int16 &counter, int16 &retFlag);
	bool o1_writeData(char &cmdCount, int16 &counter, int16 &retFlag);
	bool o1_manageDataFile(char &cmdCount, int16 &counter, int16 &retFlag);
	bool o1_setcmdCount(char &cmdCount, int16 &counter, int16 &retFlag);
	bool o1_return(char &cmdCount, int16 &counter, int16 &retFlag);
	bool o1_speakerOn(char &cmdCount, int16 &counter, int16 &retFlag);
	bool o1_speakerOff(char &cmdCount, int16 &counter, int16 &retFlag);
	bool o1_goblinFunc(char &cmdCount, int16 &counter, int16 &retFlag);
	bool o1_returnTo(char &cmdCount, int16 &counter, int16 &retFlag);
	bool o1_setBackDelta(char &cmdCount, int16 &counter, int16 &retFlag);
	bool o1_loadSound(char &cmdCount, int16 &counter, int16 &retFlag);
	bool o1_freeSoundSlot(char &cmdCount, int16 &counter, int16 &retFlag);
	bool o1_waitEndPlay(char &cmdCount, int16 &counter, int16 &retFlag);
	bool o1_animatePalette(char &cmdCount, int16 &counter, int16 &retFlag);
	bool o1_animateCursor(char &cmdCount, int16 &counter, int16 &retFlag);
	bool o1_blitCursor(char &cmdCount, int16 &counter, int16 &retFlag);
	void o1_setState(int16 &extraData, int32 *retVarPtr, Goblin::Gob_Object *objDesc);
	void o1_setCurFrame(int16 &extraData, int32 *retVarPtr, Goblin::Gob_Object *objDesc);
	void o1_setNextState(int16 &extraData, int32 *retVarPtr, Goblin::Gob_Object *objDesc);
	void o1_setMultState(int16 &extraData, int32 *retVarPtr, Goblin::Gob_Object *objDesc);
	void o1_setOrder(int16 &extraData, int32 *retVarPtr, Goblin::Gob_Object *objDesc);
	void o1_setActionStartState(int16 &extraData, int32 *retVarPtr, Goblin::Gob_Object *objDesc);
	void o1_setCurLookDir(int16 &extraData, int32 *retVarPtr, Goblin::Gob_Object *objDesc);
	void o1_setType(int16 &extraData, int32 *retVarPtr, Goblin::Gob_Object *objDesc);
	void o1_setNoTick(int16 &extraData, int32 *retVarPtr, Goblin::Gob_Object *objDesc);
	void o1_setPickable(int16 &extraData, int32 *retVarPtr, Goblin::Gob_Object *objDesc);
	void o1_setXPos(int16 &extraData, int32 *retVarPtr, Goblin::Gob_Object *objDesc);
	void o1_setYPos(int16 &extraData, int32 *retVarPtr, Goblin::Gob_Object *objDesc);
	void o1_setDoAnim(int16 &extraData, int32 *retVarPtr, Goblin::Gob_Object *objDesc);
	void o1_setRelaxTime(int16 &extraData, int32 *retVarPtr, Goblin::Gob_Object *objDesc);
	void o1_setMaxTick(int16 &extraData, int32 *retVarPtr, Goblin::Gob_Object *objDesc);
	void o1_getState(int16 &extraData, int32 *retVarPtr, Goblin::Gob_Object *objDesc);
	void o1_getCurFrame(int16 &extraData, int32 *retVarPtr, Goblin::Gob_Object *objDesc);
	void o1_getNextState(int16 &extraData, int32 *retVarPtr, Goblin::Gob_Object *objDesc);
	void o1_getMultState(int16 &extraData, int32 *retVarPtr, Goblin::Gob_Object *objDesc);
	void o1_getOrder(int16 &extraData, int32 *retVarPtr, Goblin::Gob_Object *objDesc);
	void o1_getActionStartState(int16 &extraData, int32 *retVarPtr, Goblin::Gob_Object *objDesc);
	void o1_getCurLookDir(int16 &extraData, int32 *retVarPtr, Goblin::Gob_Object *objDesc);
	void o1_getType(int16 &extraData, int32 *retVarPtr, Goblin::Gob_Object *objDesc);
	void o1_getNoTick(int16 &extraData, int32 *retVarPtr, Goblin::Gob_Object *objDesc);
	void o1_getPickable(int16 &extraData, int32 *retVarPtr, Goblin::Gob_Object *objDesc);
	void o1_getObjMaxFrame(int16 &extraData, int32 *retVarPtr, Goblin::Gob_Object *objDesc);
	void o1_getXPos(int16 &extraData, int32 *retVarPtr, Goblin::Gob_Object *objDesc);
	void o1_getYPos(int16 &extraData, int32 *retVarPtr, Goblin::Gob_Object *objDesc);
	void o1_getDoAnim(int16 &extraData, int32 *retVarPtr, Goblin::Gob_Object *objDesc);
	void o1_getRelaxTime(int16 &extraData, int32 *retVarPtr, Goblin::Gob_Object *objDesc);
	void o1_getMaxTick(int16 &extraData, int32 *retVarPtr, Goblin::Gob_Object *objDesc);
	void o1_manipulateMap(int16 &extraData, int32 *retVarPtr, Goblin::Gob_Object *objDesc);
	void o1_getItem(int16 &extraData, int32 *retVarPtr, Goblin::Gob_Object *objDesc);
	void o1_manipulateMapIndirect(int16 &extraData, int32 *retVarPtr, Goblin::Gob_Object *objDesc);
	void o1_getItemIndirect(int16 &extraData, int32 *retVarPtr, Goblin::Gob_Object *objDesc);
	void o1_setPassMap(int16 &extraData, int32 *retVarPtr, Goblin::Gob_Object *objDesc);
	void o1_setGoblinPosH(int16 &extraData, int32 *retVarPtr, Goblin::Gob_Object *objDesc);
	void o1_getGoblinPosXH(int16 &extraData, int32 *retVarPtr, Goblin::Gob_Object *objDesc);
	void o1_getGoblinPosYH(int16 &extraData, int32 *retVarPtr, Goblin::Gob_Object *objDesc);
	void o1_setGoblinMultState(int16 &extraData, int32 *retVarPtr, Goblin::Gob_Object *objDesc);
	void o1_setGoblinPos(int16 &extraData, int32 *retVarPtr, Goblin::Gob_Object *objDesc);
	void o1_setGoblinState(int16 &extraData, int32 *retVarPtr, Goblin::Gob_Object *objDesc);
	void o1_setGoblinStateRedraw(int16 &extraData, int32 *retVarPtr, Goblin::Gob_Object *objDesc);
	void o1_setGoblinUnk14(int16 &extraData, int32 *retVarPtr, Goblin::Gob_Object *objDesc);
	void o1_setItemIdInPocket(int16 &extraData, int32 *retVarPtr, Goblin::Gob_Object *objDesc);
	void o1_setItemIndInPocket(int16 &extraData, int32 *retVarPtr, Goblin::Gob_Object *objDesc);
	void o1_getItemIdInPocket(int16 &extraData, int32 *retVarPtr, Goblin::Gob_Object *objDesc);
	void o1_getItemIndInPocket(int16 &extraData, int32 *retVarPtr, Goblin::Gob_Object *objDesc);
	void o1_setItemPos(int16 &extraData, int32 *retVarPtr, Goblin::Gob_Object *objDesc);
	void o1_decRelaxTime(int16 &extraData, int32 *retVarPtr, Goblin::Gob_Object *objDesc);
	void o1_getGoblinPosX(int16 &extraData, int32 *retVarPtr, Goblin::Gob_Object *objDesc);
	void o1_getGoblinPosY(int16 &extraData, int32 *retVarPtr, Goblin::Gob_Object *objDesc);
	void o1_clearPathExistence(int16 &extraData, int32 *retVarPtr, Goblin::Gob_Object *objDesc);
	void o1_setGoblinVisible(int16 &extraData, int32 *retVarPtr, Goblin::Gob_Object *objDesc);
	void o1_setGoblinInvisible(int16 &extraData, int32 *retVarPtr, Goblin::Gob_Object *objDesc);
	void o1_getObjectIntersect(int16 &extraData, int32 *retVarPtr, Goblin::Gob_Object *objDesc);
	void o1_getGoblinIntersect(int16 &extraData, int32 *retVarPtr, Goblin::Gob_Object *objDesc);
	void o1_loadObjects(int16 &extraData, int32 *retVarPtr, Goblin::Gob_Object *objDesc);
	void o1_freeObjects(int16 &extraData, int32 *retVarPtr, Goblin::Gob_Object *objDesc);
	void o1_animateObjects(int16 &extraData, int32 *retVarPtr, Goblin::Gob_Object *objDesc);
	void o1_drawObjects(int16 &extraData, int32 *retVarPtr, Goblin::Gob_Object *objDesc);
	void o1_loadMap(int16 &extraData, int32 *retVarPtr, Goblin::Gob_Object *objDesc);
	void o1_moveGoblin(int16 &extraData, int32 *retVarPtr, Goblin::Gob_Object *objDesc);
	void o1_switchGoblin(int16 &extraData, int32 *retVarPtr, Goblin::Gob_Object *objDesc);
	void o1_loadGoblin(int16 &extraData, int32 *retVarPtr, Goblin::Gob_Object *objDesc);
	void o1_writeTreatItem(int16 &extraData, int32 *retVarPtr, Goblin::Gob_Object *objDesc);
	void o1_moveGoblin0(int16 &extraData, int32 *retVarPtr, Goblin::Gob_Object *objDesc);
	void o1_setGoblinTarget(int16 &extraData, int32 *retVarPtr, Goblin::Gob_Object *objDesc);
	void o1_setGoblinObjectsPos(int16 &extraData, int32 *retVarPtr, Goblin::Gob_Object *objDesc);
	void o1_initGoblin(int16 &extraData, int32 *retVarPtr, Goblin::Gob_Object *objDesc);
};

class Inter_v2 : public Inter_v1 {
public:
	Inter_v2(GobEngine *vm);
	virtual ~Inter_v2() {};
	virtual int16 loadSound(int16 search);
	virtual void storeMouse(void);

protected:
	typedef void (Inter_v2::*OpcodeDrawProcV2)(void);
	typedef bool (Inter_v2::*OpcodeFuncProcV2)(char &, int16 &, int16 &);
	typedef void (Inter_v2::*OpcodeGoblinProcV2)(int16 &, int32 *, Goblin::Gob_Object *);
  struct OpcodeDrawEntryV2 {
		OpcodeDrawProcV2 proc;
		const char *desc;
		};
  struct OpcodeFuncEntryV2 {
		OpcodeFuncProcV2 proc;
		const char *desc;
		};
  struct OpcodeGoblinEntryV2 {
		OpcodeGoblinProcV2 proc;
		const char *desc;
		};
	const OpcodeDrawEntryV2 *_opcodesDrawV2;
	const OpcodeFuncEntryV2 *_opcodesFuncV2;
	const OpcodeGoblinEntryV2 *_opcodesGoblinV2;
	static const int _goblinFuncLookUp[][2];

	virtual void setupOpcodes(void);
	virtual void executeDrawOpcode(byte i);
	virtual bool executeFuncOpcode(byte i, byte j, char &cmdCount, int16 &counter, int16 &retFlag);
	virtual void executeGoblinOpcode(int i, int16 &extraData, int32 *retVarPtr, Goblin::Gob_Object *objDesc);
	virtual const char *getOpcodeDrawDesc(byte i);
	virtual const char *getOpcodeFuncDesc(byte i, byte j);
	virtual const char *getOpcodeGoblinDesc(int i);
	virtual void loadMult(void);

	void o2_drawStub(void) { warning("Gob2 stub"); }
	void o2_totSub(void);
	void o2_stub0x56(void);
	void o2_stub0x80(void);
	void o2_renderStatic(void);
	bool o2_evaluateStore(char &cmdCount, int16 &counter, int16 &retFlag);
	bool o2_palLoad(char &cmdCount, int16 &counter, int16 &retFlag);
	bool o2_loadTot(char &cmdCount, int16 &counter, int16 &retFlag);
	bool o2_freeSprite(char &cmdCount, int16 &counter, int16 &retFlag);
	bool o2_loadSound(char &cmdCount, int16 &counter, int16 &retFlag);
	void o2_setRenderFlags(void);
	void o2_initMult(void);
	void o2_loadCurLayer(void);
	void o2_playCDTrack(void);
	void o2_stopCD(void);
	void o2_readLIC(void);
	void o2_freeLIC(void);
	void o2_getCDTrackPos(void);
	void o2_playMult(void);
	void o2_initCursor(void);
};

}				// End of namespace Gob

#endif
