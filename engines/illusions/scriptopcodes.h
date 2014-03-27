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

#ifndef ILLUSIONS_SCRIPTOPCODES_H
#define ILLUSIONS_SCRIPTOPCODES_H

#include "common/func.h"

namespace Illusions {

class IllusionsEngine;
class ScriptThread;

struct OpCall {
	byte _op;
	byte _opSize;
	uint32 _threadId;
	uint32 _callerThreadId;
	int16 _deltaOfs;
	byte *_code;
	int _result;
	void skip(uint size);
	byte readByte();
	int16 readSint16();
	uint32 readUint32();
};

typedef Common::Functor2<ScriptThread*, OpCall&, void> ScriptOpcode;

class ScriptOpcodes {
public:
	ScriptOpcodes(IllusionsEngine *vm);
	~ScriptOpcodes();
	void execOpcode(ScriptThread *scriptThread, OpCall &opCall);
protected:
	IllusionsEngine *_vm;
	ScriptOpcode *_opcodes[256];
	Common::String _opcodeNames[256];
	void initOpcodes();
	void freeOpcodes();

	// Opcodes
	void opSuspend(ScriptThread *scriptThread, OpCall &opCall);
	void opYield(ScriptThread *scriptThread, OpCall &opCall);
	void opTerminate(ScriptThread *scriptThread, OpCall &opCall);
	void opJump(ScriptThread *scriptThread, OpCall &opCall);
	void opStartScriptThread(ScriptThread *scriptThread, OpCall &opCall);
	void opStartTempScriptThread(ScriptThread *scriptThread, OpCall &opCall);
	void opStartTimerThread(ScriptThread *scriptThread, OpCall &opCall);
	void opSetThreadSceneId(ScriptThread *scriptThread, OpCall &opCall);
	void opEndTalkThreads(ScriptThread *scriptThread, OpCall &opCall);
	void opLoadResource(ScriptThread *scriptThread, OpCall &opCall);
	void opUnloadResource(ScriptThread *scriptThread, OpCall &opCall);
	void opEnterScene(ScriptThread *scriptThread, OpCall &opCall);
	void opChangeScene(ScriptThread *scriptThread, OpCall &opCall);
	void opStartModalScene(ScriptThread *scriptThread, OpCall &opCall);
	void opEnterCloseUpScene(ScriptThread *scriptThread, OpCall &opCall);
	void opExitCloseUpScene(ScriptThread *scriptThread, OpCall &opCall);
	void opPanCenterObject(ScriptThread *scriptThread, OpCall &opCall);
	void opPanToNamedPoint(ScriptThread *scriptThread, OpCall &opCall);
	void opPanStop(ScriptThread *scriptThread, OpCall &opCall);
	void opSetDisplay(ScriptThread *scriptThread, OpCall &opCall);	
	void opIncBlockCounter(ScriptThread *scriptThread, OpCall &opCall);
	void opSetProperty(ScriptThread *scriptThread, OpCall &opCall);
	void opPlaceActor(ScriptThread *scriptThread, OpCall &opCall);
	void opFaceActor(ScriptThread *scriptThread, OpCall &opCall);
	void opFaceActorToObject(ScriptThread *scriptThread, OpCall &opCall);
	void opStartSequenceActor(ScriptThread *scriptThread, OpCall &opCall);
	void opStartMoveActor(ScriptThread *scriptThread, OpCall &opCall);
	void opSetActorToNamedPoint(ScriptThread *scriptThread, OpCall &opCall);
	void opStartTalkThread(ScriptThread *scriptThread, OpCall &opCall);
	void opAppearActor(ScriptThread *scriptThread, OpCall &opCall);
	void opDisappearActor(ScriptThread *scriptThread, OpCall &opCall);
	void opActivateObject(ScriptThread *scriptThread, OpCall &opCall);
	void opDeactivateObject(ScriptThread *scriptThread, OpCall &opCall);
	void opSetDefaultSequence(ScriptThread *scriptThread, OpCall &opCall);
	void opSetSelectSfx(ScriptThread *scriptThread, OpCall &opCall);
	void opSetMoveSfx(ScriptThread *scriptThread, OpCall &opCall);
	void opSetDenySfx(ScriptThread *scriptThread, OpCall &opCall);
	void opSetAdjustUpSfx(ScriptThread *scriptThread, OpCall &opCall);
	void opSetAdjustDnSfx(ScriptThread *scriptThread, OpCall &opCall);
	void opStartSound(ScriptThread *scriptThread, OpCall &opCall);
	void opStopSound(ScriptThread *scriptThread, OpCall &opCall);
	void opStartMusic(ScriptThread *scriptThread, OpCall &opCall);
	void opStopMusic(ScriptThread *scriptThread, OpCall &opCall);
	void opStackPushRandom(ScriptThread *scriptThread, OpCall &opCall);
	void opIfLte(ScriptThread *scriptThread, OpCall &opCall);
	void opAddMenuChoice(ScriptThread *scriptThread, OpCall &opCall);
	void opDisplayMenu(ScriptThread *scriptThread, OpCall &opCall);
	void opSwitchMenuChoice(ScriptThread *scriptThread, OpCall &opCall);
	void opResetGame(ScriptThread *scriptThread, OpCall &opCall);
	void opDeactivateButton(ScriptThread *scriptThread, OpCall &opCall);
	void opActivateButton(ScriptThread *scriptThread, OpCall &opCall);
	void opJumpIf(ScriptThread *scriptThread, OpCall &opCall);
	void opNot(ScriptThread *scriptThread, OpCall &opCall);
	void opAnd(ScriptThread *scriptThread, OpCall &opCall);
	void opOr(ScriptThread *scriptThread, OpCall &opCall);
	void opGetProperty(ScriptThread *scriptThread, OpCall &opCall);
	void opCompareBlockCounter(ScriptThread *scriptThread, OpCall &opCall);
	void opDebug126(ScriptThread *scriptThread, OpCall &opCall);
	void opPlayVideo(ScriptThread *scriptThread, OpCall &opCall);
	void opStackPop(ScriptThread *scriptThread, OpCall &opCall);
	void opStackDup(ScriptThread *scriptThread, OpCall &opCall);
	void opLoadSpecialCodeModule(ScriptThread *scriptThread, OpCall &opCall);
	void opRunSpecialCode(ScriptThread *scriptThread, OpCall &opCall);
	void opStopActor(ScriptThread *scriptThread, OpCall &opCall);
	void opSetActorUsePan(ScriptThread *scriptThread, OpCall &opCall);
	void opStartAbortableThread(ScriptThread *scriptThread, OpCall &opCall);
	void opKillThread(ScriptThread *scriptThread, OpCall &opCall);
	void opSetSceneIdThreadId(ScriptThread *scriptThread, OpCall &opCall);
	void opStackPush0(ScriptThread *scriptThread, OpCall &opCall);
	void opSetFontId(ScriptThread *scriptThread, OpCall &opCall);
	void opAddMenuKey(ScriptThread *scriptThread, OpCall &opCall);
	
};

} // End of namespace Illusions

#endif // ILLUSIONS_SCRIPTOPCODES_H
