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

#include "dragons/actorresource.h"
#include "dragons/cursor.h"
#include "dragons/dragons.h"
#include "dragons/dragonflg.h"
#include "dragons/dragonini.h"
#include "dragons/dragonimg.h"
#include "dragons/dragonobd.h"
#include "dragons/inventory.h"
#include "dragons/scene.h"
#include "dragons/scriptopcodes.h"
#include "dragons/specialopcodes.h"
#include "dragons/actor.h"
#include "dragons/sound.h"
#include "dragons/talk.h"
#include "dragons/screen.h"


namespace Dragons {

// Convenience macros
#define ARG_SKIP(x) scriptOpCall.skip(x);
#define ARG_BYTE(name) byte name = scriptOpCall.readByte(); debug(5, "ARG_BYTE(" #name " = %d)", name);
#define ARG_INT8(name) int8 name = scriptOpCall.readByte(); debug(5, "ARG_INT8(" #name " = %d)", name);
#define ARG_INT16(name) int16 name = scriptOpCall.readSint16(); debug(5, "ARG_INT16(" #name " = %d)", name);
#define ARG_UINT32(name) uint32 name = scriptOpCall.readUint32(); debug(5, "ARG_UINT32(" #name " = %08X)", name);

// ScriptOpCall

void ScriptOpCall::skip(uint size) {
	_code += size;
}

byte ScriptOpCall::readByte() {
	return *_code++;
}

int16 ScriptOpCall::readSint16() {
	int16 value = READ_LE_UINT16(_code);
	_code += 2;
	return value;
}

uint32 ScriptOpCall::readUint32() {
	uint32 value = READ_LE_UINT32(_code);
	_code += 4;
	return value;
}

ScriptOpCall::ScriptOpCall(byte *start, uint32 length): _op(0), _result(0), _field8(0) {
	_code = _base = start;
	_codeEnd = _code + length;
}

// ScriptOpcodes

ScriptOpcodes::ScriptOpcodes(DragonsEngine *vm, DragonFLG *dragonFLG)
	: _vm(vm), _dragonFLG(dragonFLG), _numDialogStackFramesToPop(0) {
	_specialOpCodes = new SpecialOpcodes(_vm);
	initOpcodes();
	_scriptTargetINI = 0;
}

ScriptOpcodes::~ScriptOpcodes() {
	freeOpcodes();
	delete _specialOpCodes;
}

void ScriptOpcodes::execOpcode(ScriptOpCall &scriptOpCall) {
	if (!_opcodes[scriptOpCall._op])
		error("ScriptOpcodes::execOpcode() Unimplemented opcode %d (0x%X)", scriptOpCall._op, scriptOpCall._op);
	debug(1, "execScriptOpcode(0x%X) @%lX  %s", scriptOpCall._op, scriptOpCall._code - scriptOpCall._base, _opcodeNames[scriptOpCall._op].c_str());
	(*_opcodes[scriptOpCall._op])(scriptOpCall);
}

typedef Common::Functor1Mem<ScriptOpCall&, void, ScriptOpcodes> ScriptOpcodeI;
#define OPCODE(op, func) \
	_opcodes[op] = new ScriptOpcodeI(this, &ScriptOpcodes::func); \
	_opcodeNames[op] = #func;

void ScriptOpcodes::initOpcodes() {
	// First clear everything
	for (uint i = 0; i < DRAGONS_NUM_SCRIPT_OPCODES; ++i) {
		_opcodes[i] = 0;
	}
	// Register opcodes
	OPCODE(1, opUnk1);
	OPCODE(2, opAddDialogChoice);
	OPCODE(3, opPopDialogStack);
	OPCODE(4, opExecuteScript);
	OPCODE(5, opSetActorDirection);
	OPCODE(6, opPerformActionOnObject);
	OPCODE(7, opMoveObjectToScene);
	OPCODE(8, opActorLoadSequence);

	OPCODE(0xA, opSetVariable);
	OPCODE(0xB, opRunSpecialOpCode);
	OPCODE(0xC, opPlayOrStopSound);
	OPCODE(0xD, opDelay);
	OPCODE(0xE, opMoveActorToPoint);
	OPCODE(0xF, opMoveActorToXY);
	OPCODE(0x10, opMoveActorToObject);
	OPCODE(0x11, opUnk11FlickerTalk);
	OPCODE(0x12, opLoadScene);
	OPCODE(0x13, opIfStatement);
	OPCODE(0x14, opIfElseStatement);
	OPCODE(0x15, opUnk15PropertiesRelated); //Is this used?
	OPCODE(0x16, opUnk16);
	OPCODE(0x17, opWaitForActorSequenceToFinish);
	OPCODE(0x18, opDialogAtPoint);
	OPCODE(0x19, opExecuteObjectSceneScript);
	OPCODE(0x1A, opUpdatePaletteCycling);
	OPCODE(0x1B, opWaitForActorToFinishWalking);
	OPCODE(0x1C, opSetActorFlag0x1000);
	OPCODE(0x1D, opShowActor);
	OPCODE(0x1E, opHideActor);
	OPCODE(0x1F, opPlayMusic);
	OPCODE(0x20, opPreLoadSceneData);
	OPCODE(0x21, opPauseCurrentSpeechAndFetchNextDialog);
	OPCODE(0x22, opCodeActorTalk);
}

#undef OPCODE

void ScriptOpcodes::freeOpcodes() {
	for (uint i = 0; i < DRAGONS_NUM_SCRIPT_OPCODES; ++i) {
		delete _opcodes[i];
	}
}

void ScriptOpcodes::updateReturn(ScriptOpCall &scriptOpCall, uint16 size) {
	// scriptOpCall._deltaOfs = size * 2 + 2;
}


void ScriptOpcodes::runScript(ScriptOpCall &scriptOpCall) {
	scriptOpCall._field8 = 0;
	scriptOpCall._result = 0;
	_numDialogStackFramesToPop = 0;
	executeScriptLoop(scriptOpCall);
}


void ScriptOpcodes::runScript3(ScriptOpCall &scriptOpCall) {
	scriptOpCall._field8 = 3;
	scriptOpCall._result = 0;
	_numDialogStackFramesToPop = 0;
	executeScriptLoop(scriptOpCall);
}

bool ScriptOpcodes::runScript4(ScriptOpCall &scriptOpCall) {
	scriptOpCall._field8 = 4;
	scriptOpCall._result = 0;
	_numDialogStackFramesToPop = 0;
	executeScriptLoop(scriptOpCall);
	return scriptOpCall._result;
}

void ScriptOpcodes::executeScriptLoop(ScriptOpCall &scriptOpCall) {
//
//	if (scriptOpCall._code >= scriptOpCall._codeEnd || scriptOpCall._result & 1) {
//		return;
//	}
//
//	if (_vm->isFlagSet(ENGINE_FLAG_100000)) {
//		return;
//	}
//
//	if (_vm->isFlagSet(ENGINE_FLAG_80000)) {
//		//TODO
////		if (IsPressedStart(0)) {
////			getEngine()->setFlags(Dragons::ENGINE_FLAG_100000);
////		}
//	}
//
//	uint16 opcode = READ_LE_UINT16(scriptOpCall._code) & 0x7fff;
//
//	scriptOpCall._op = (byte) opcode;
//	if (opcode < DRAGONS_NUM_SCRIPT_OPCODES) {
//		execOpcode(scriptOpCall);
//	}
//
	while (scriptOpCall._code < scriptOpCall._codeEnd && !(scriptOpCall._result & 1) && !_vm->shouldQuit()) {

		if (_vm->isFlagSet(ENGINE_FLAG_100000)) {
			return;
		}

		if (_vm->isFlagSet(ENGINE_FLAG_80000)) {
			//TODO
//		if (IsPressedStart(0)) {
//			getEngine()->setFlags(Dragons::ENGINE_FLAG_100000);
//		}
		}

		uint16 opcode = READ_LE_UINT16(scriptOpCall._code) & 0x7fff;

		if (opcode >= DRAGONS_NUM_SCRIPT_OPCODES) {
			return; //TODO should continue here.
		}
		scriptOpCall._op = (byte) opcode;
		execOpcode(scriptOpCall);

		if (_numDialogStackFramesToPop != 0) {
			scriptOpCall._result |= 1;
			break;
		}
	}
}

// Opcodes

void ScriptOpcodes::opUnk1(ScriptOpCall &scriptOpCall) {
	ARG_SKIP(2);
	ARG_INT16(field2);
	ARG_INT16(field4);
	ARG_INT16(field6);

	if ((field2 >> _vm->_cursor->_data_800728b0_cursor_seqID) & 1
			&& (_vm->_cursor->_data_800728b0_cursor_seqID < 5 || field4 == _scriptTargetINI)
			&& scriptOpCall._field8 == 1) {
		scriptOpCall._code -= 8;
		scriptOpCall._result |= 1;
	} else {
		scriptOpCall._code += field6;
	}
}

void ScriptOpcodes::opAddDialogChoice(ScriptOpCall &scriptOpCall) {
	ARG_INT16(field0);
	ARG_UINT32(field2);
	ARG_UINT32(field6);
	ARG_INT16(fieldA);
	ARG_INT16(fieldC);
	ARG_INT16(fieldE);

	if (scriptOpCall._field8 == 2) {
		TalkDialogEntry *talkDialogEntry = new TalkDialogEntry();

		talkDialogEntry->hasText = _vm->_talk->loadText(field2, (uint16 *)(&talkDialogEntry->dialogText[10]), 295);

		talkDialogEntry->textIndex = field2;
		talkDialogEntry->textIndex1 = field6;
		talkDialogEntry->scriptCodeStartPtr = scriptOpCall._code;
		talkDialogEntry->flags = 0;
		talkDialogEntry->scriptCodeEndPtr = scriptOpCall._code + fieldA;
		if ((field0 & 0x8000U) != 0) {
			talkDialogEntry->flags = 2;
		}
		talkDialogEntry->field_26c = fieldC;
		talkDialogEntry->iniId = fieldE;
		_vm->_talk->addTalkDialogEntry(talkDialogEntry);
	}
	scriptOpCall._code += fieldA;
}

// The number of dialog frames to pop off the stack. this returns up from nested conversation trees.
void ScriptOpcodes::opPopDialogStack(ScriptOpCall &scriptOpCall) {
	ARG_INT16(field0);
	ARG_INT16(field2);
	if (scriptOpCall._field8 == 0) {
		_numDialogStackFramesToPop = field2;
	}
}

void ScriptOpcodes::opExecuteScript(ScriptOpCall &scriptOpCall) {
	ARG_SKIP(2);
	ARG_UINT32(obdOffset);
	byte *data =_vm->_dragonOBD->getObdAtOffset(obdOffset);

	ScriptOpCall newScriptOpCall(data + 4, READ_LE_UINT32(data));
	newScriptOpCall._field8 = scriptOpCall._field8;
	newScriptOpCall._result = 0;

	executeScriptLoop(newScriptOpCall);
}

void ScriptOpcodes::opSetActorDirection(ScriptOpCall &scriptOpCall) {
	ARG_SKIP(2);
	ARG_INT16(iniIndex);
	ARG_SKIP(2);
	ARG_INT16(direction);

	if (scriptOpCall._field8 == 0) {
		_vm->getINI(iniIndex - 1)->actor->_direction = direction;
	}
}

void ScriptOpcodes::opPerformActionOnObject(ScriptOpCall &scriptOpCall) {
	ARG_SKIP(2);
	ARG_INT16(verb);
	ARG_INT16(srcINI);
	ARG_INT16(targetINI);

	if (scriptOpCall._field8 != 0) {
		return;
	}

	int16 uVar6 = _scriptTargetINI;
	int16 uVar5 = _vm->_cursor->_data_800728b0_cursor_seqID;
	int16 uVar4 = _vm->_cursor->_performActionTargetINI;
	int16 uVar3 = _vm->_cursor->_iniUnderCursor;
	int32 uVar2 = _vm->_cursor->_sequenceID;
	bool isEngineFlag8Set = _vm->isFlagSet(ENGINE_FLAG_8);
	_vm->clearFlags(ENGINE_FLAG_8);
//	DisableVSyncEvent();
	_vm->_cursor->_iniUnderCursor = srcINI;
	_vm->_cursor->_sequenceID = 0;

	for (int16 i = verb >> 1; i != 0; i = i >> 1) {
		_vm->_cursor->_sequenceID++;
	}

	_scriptTargetINI = targetINI;
	_vm->_cursor->_data_800728b0_cursor_seqID = _vm->_cursor->_sequenceID;
	_vm->_cursor->_performActionTargetINI = _vm->_cursor->_iniUnderCursor;
//	EnableVSyncEvent();
	_vm->performAction();
	if (isEngineFlag8Set) {
		_vm->setFlags(ENGINE_FLAG_8);
	}
	_vm->_cursor->_sequenceID = uVar2;
	_vm->_cursor->_iniUnderCursor = uVar3;
	_vm->_cursor->_performActionTargetINI = uVar4;
	_vm->_cursor->_data_800728b0_cursor_seqID = uVar5;
	_scriptTargetINI = uVar6;
}

void ScriptOpcodes::opMoveObjectToScene(ScriptOpCall &scriptOpCall) {
	ARG_INT16(field0);
	ARG_INT16(field2);
	ARG_INT16(sceneId);

	if (scriptOpCall._field8 != 0) {
		return;
	}

	uint16 currentScene = _vm->getCurrentSceneId();
	DragonINI *ini = _vm->getINI(field2 - 1);
	if (!(field0 & 0x8000)) {

		if (ini->flags & 1) {
			if (ini->sceneId == currentScene) {
				assert(ini->actor);
				ini->actor->reset_maybe();
			}
			if (sceneId == currentScene) {
				ini->actor = _vm->_actorManager->loadActor(ini->actorResourceId, ini->sequenceId, ini->x, ini->y, 0);
				ini->actor->_direction = ini->direction2;
				if (ini->flags & 2) {
					ini->actor->_flags |= ACTOR_FLAG_80;
				} else {
					ini->actor->_flags &= ~ACTOR_FLAG_80;
				}

				if (ini->flags & 0x20) {
					ini->actor->_flags |= ACTOR_FLAG_100;
				} else {
					ini->actor->_flags &= ~ACTOR_FLAG_100;
				}

				if (ini->flags & 4) {
					ini->actor->_flags |= ACTOR_FLAG_8000;
				} else {
					ini->actor->_flags &= ~ACTOR_FLAG_8000;
				}

				if (ini->flags & 0x100) {
					ini->actor->_flags |= ACTOR_FLAG_4000;
				} else {
					ini->actor->_flags &= ~ACTOR_FLAG_4000;
				}
			}
		} else {
			if (ini->sceneId == currentScene && ini->iptIndex_maybe != -1) {
				_vm->_scene->removeImageOverlay(ini->iptIndex_maybe);
			}
			if (sceneId == currentScene && ini->iptIndex_maybe != -1) {
				_vm->_scene->loadImageOverlay(ini->iptIndex_maybe);
			}
		}

		if (ini->sceneId == 1) {
			if ((uint)_vm->_cursor->_iniItemInHand - 1 == ini->id) {
				_vm->_cursor->_data_800728b0_cursor_seqID = 0;
				_vm->_cursor->_sequenceID = 0;
				_vm->_cursor->_iniItemInHand = 0;
			} else {
				if (_vm->_inventory->hasItem(ini->id + 1)) {
					Actor *actor = _vm->_inventory->getInventoryItemActor(ini->id + 1);
					_vm->_inventory->clearItem(ini->id + 1);
					if (_vm->_inventory->getState() == InventoryOpen) {
						actor->clearFlag(ACTOR_FLAG_40);
					}
				}
			}
		}

		if (sceneId == 1) {
			if (_vm->_cursor->_iniItemInHand != 0) {
				_vm->_inventory->addItem(_vm->_cursor->_iniItemInHand);
				if (_vm->_inventory->getState() == InventoryOpen) {
					Actor *actor = _vm->_inventory->getInventoryItemActor(_vm->_cursor->_iniItemInHand);
					actor->_flags = 0;
					actor->_priorityLayer = 0;
					actor->_scale = DRAGONS_ENGINE_SPRITE_100_PERCENT_SCALE;
					actor->updateSequence((_vm->getINI(_vm->_cursor->_iniItemInHand - 1)->inventorySequenceId * 2 + 10) & 0xfffe);
					actor->setFlag(ACTOR_FLAG_40);
					actor->setFlag(ACTOR_FLAG_80);
					actor->setFlag(ACTOR_FLAG_100);
					actor->setFlag(ACTOR_FLAG_200);
					actor->_priorityLayer = 6;
				}
			}
			DragonINI *flicker = _vm->_dragonINIResource->getFlickerRecord();
			_vm->_cursor->updatePosition(flicker->actor->_x_pos - _vm->_scene->_camera.x,
										 flicker->actor->_y_pos - (_vm->_scene->_camera.y + 0x1e));
			_vm->_cursor->_data_800728b0_cursor_seqID = 5;
			_vm->_cursor->_sequenceID = 5;
			_vm->_cursor->_objectInHandSequenceID = _vm->getINI(field2 - 1)->inventorySequenceId * 2 + 10;
			_vm->_cursor->_iniItemInHand = field2;
		}
	}
	ini->sceneId = sceneId;
}

void ScriptOpcodes::opActorLoadSequence(ScriptOpCall &scriptOpCall) {
	ARG_INT16(field0);
	ARG_INT16(field2);
	ARG_INT16(sequenceId);

	if (scriptOpCall._field8 != 0) {
		return;
	}

	DragonINI *ini = _vm->getINI(field2 - 1);

	bool isFlicker = _vm->_dragonINIResource->isFlicker(field2 - 1);
	if (isFlicker) {
		ini->actor->_flags |= ACTOR_FLAG_2000;
	}

	if (!ini->actor->_actorResource || ini->actor->_actorResource->_id != (uint32)ini->actorResourceId) {
		ini->actor->_actorResource = _vm->_actorManager->getActorResource(ini->actorResourceId);
	}

	ini->actor->updateSequence(sequenceId);

	if (field0 & 0x8000) {
		ini->actor->waitUntilFlag8And4AreSet();
	}

	if (isFlicker) {
		ini->actor->_flags &= ~ACTOR_FLAG_2000;
	}
}

void ScriptOpcodes::opPlayMusic(ScriptOpCall &scriptOpCall) {
	ARG_SKIP(2);
	ARG_INT16(songNumber);
	if (scriptOpCall._field8 == 0) {
		_vm->_sound->playMusic(songNumber);
	}
}

void ScriptOpcodes::opIfStatement(ScriptOpCall &scriptOpCall) {
	if (evaluateExpression(scriptOpCall)) {
		scriptOpCall._code += 4;
	} else {
		scriptOpCall._code += 4 + READ_LE_UINT16(scriptOpCall._code);
	}
}

void ScriptOpcodes::opIfElseStatement(ScriptOpCall &scriptOpCall) {
	if (evaluateExpression(scriptOpCall)) {
		ScriptOpCall localScriptOpCall(scriptOpCall._code + 4, READ_LE_UINT16(scriptOpCall._code));
		localScriptOpCall._field8 = scriptOpCall._field8;
		localScriptOpCall._result = 0;

		executeScriptLoop(localScriptOpCall);

		if (scriptOpCall._field8 == 1) {
			scriptOpCall._result = localScriptOpCall._result;
			if ((localScriptOpCall._result & 1) != 0) {
				scriptOpCall._code = localScriptOpCall._code;
				return;
			}
		}

		scriptOpCall._code = localScriptOpCall._code + READ_LE_UINT16(scriptOpCall._code + 2);
	} else {
		scriptOpCall._code += 4 + READ_LE_UINT16(scriptOpCall._code);
	}
}

void ScriptOpcodes::opUnk15PropertiesRelated(ScriptOpCall &scriptOpCall) {
	while (true) {
		if (evaluateExpression(scriptOpCall)) {
			ScriptOpCall localScriptOpCall(scriptOpCall._code + 4, READ_LE_UINT32(scriptOpCall._code));

			runScript(localScriptOpCall);

			scriptOpCall._code = (scriptOpCall._code - ((uint)*(scriptOpCall._code + 2) + 2));
		} else {
			break;
		}
	}

	scriptOpCall._code += 4 + READ_LE_UINT16(scriptOpCall._code);
}

void ScriptOpcodes::opPreLoadSceneData(ScriptOpCall &scriptOpCall) {
	ARG_INT16(field0);
	ARG_INT16(sceneId);

	_vm->_sound->resumeMusic();
	_vm->_isLoadingDialogAudio = true;

	if (sceneId >= 2) {
		//TODO do we need this? It looks like it is pre-loading the next scene's data.
	}
}

void ScriptOpcodes::opPauseCurrentSpeechAndFetchNextDialog(ScriptOpCall &scriptOpCall) {
	ARG_INT16(field0);
	ARG_UINT32(textIndex);

	if (scriptOpCall._field8 == 0) {
		_vm->_sound->resumeMusic();
		//The original starts seeking the CD-ROM here for the `textIndex` dialog but we don't need to do that.
	}
}

bool ScriptOpcodes::evaluateExpression(ScriptOpCall &scriptOpCall) {
	byte *codePtrOffsetA = scriptOpCall._code + 0xA;
	byte *codePtrOffset2 = scriptOpCall._code + 2;

	uint16 status = 0;
	uint16 result = 0;

	int16 t2 = 0;
	int16 t0 = 0;

	for (;;) {
		byte value = 0;
		if (*codePtrOffsetA & 1) {
			uint32 propId = READ_LE_UINT16(codePtrOffset2) * 8 + READ_LE_UINT16(codePtrOffsetA - 6);
			value = _dragonFLG->get(propId) ? 1 : 0;
		} else {
			debug(3, "Op13 get here!!");
			if (*codePtrOffsetA & 2) {
				debug(3, "Op13 get here!! & 2");
				t2 = _vm->getVar(READ_LE_UINT16(codePtrOffset2));
			}

			if (*codePtrOffsetA & 4) {
				t2 = getINIField(READ_LE_INT16(codePtrOffsetA - 6) - 1, READ_LE_INT16(codePtrOffset2));
				debug(3, "Op13 get here!! & 4 read ini field ini: %X fieldOffset: %X value: %d", READ_LE_INT16(codePtrOffsetA - 6) - 1, READ_LE_INT16(codePtrOffset2), t2);

			}

			if (!(*codePtrOffsetA & 7)) {
				debug(3, "Op13 get here!! & 7");
				t2 = READ_LE_UINT16(codePtrOffsetA - 6);
			}

			if (*codePtrOffsetA & 8) {
				debug(3, "Op13 get here!! & 8");
				t0 = _vm->getVar(READ_LE_UINT16(codePtrOffsetA - 4));
			}

			if (*codePtrOffsetA & 0x10) {
				debug(3, "Op13 get here!! & 0x10");
				t0 = getINIField(READ_LE_INT16(codePtrOffsetA - 2) - 1, READ_LE_INT16(codePtrOffsetA - 4));
			}

			if (!(*codePtrOffsetA & 0x18)) {
				t0 = READ_LE_UINT16(codePtrOffsetA - 2);
				debug(3, "Op13 get here!! & 0x18 t0 == %d", t0);
			}

			if (*(codePtrOffsetA + 1) == 0 && t0 == t2) {
				value = 1;
			}

			if (*(codePtrOffsetA + 1) == 1 && t2 < t0) {
				value = 1;
			}

			if (*(codePtrOffsetA + 1) == 2 && t0 < t2) {
				value = 1;
			}

		}

		if (*codePtrOffsetA & 0x20) {
			value ^= 1;
		}

		if (!(status & 0xffff)) {
			result |= value;
		} else {
			result &= value;
		}

		status = (*codePtrOffsetA >> 6) & 1;

		if (!(*codePtrOffsetA & 0x80)) {
			break;
		}

		codePtrOffset2 += 0xa;
		codePtrOffsetA += 0xa;
	}

	scriptOpCall._code = codePtrOffset2 + 0xa;

	return (result & 0xffff) != 0;
}

void ScriptOpcodes::opSetVariable(ScriptOpCall &scriptOpCall) {
	if (scriptOpCall._field8 == 0) {
		setVariable(scriptOpCall);
	} else {
		scriptOpCall._code += 0xC;
	}
}

void ScriptOpcodes::opRunSpecialOpCode(ScriptOpCall &scriptOpCall) {
	ARG_SKIP(2);
	ARG_INT16(specialOpCode);

	if (scriptOpCall._field8 != 0) {
		return;
	}

	if (specialOpCode >= 140) {
		error("Invalid Special OpCode %d", specialOpCode);
	}

	debug(1, "Special opCode %X", specialOpCode);
	_specialOpCodes->run(specialOpCode);
}

void ScriptOpcodes::opPlayOrStopSound(ScriptOpCall &scriptOpCall) {
	ARG_SKIP(2);
	ARG_INT16(soundId);

	if (scriptOpCall._field8 == 0) {
		_vm->playOrStopSound((uint16) soundId);
	}
}

void ScriptOpcodes::opDelay(ScriptOpCall &scriptOpCall) {
	ARG_SKIP(2);
	ARG_INT16(delay);

	if (scriptOpCall._field8 != 0) {
		return;
	}

	_vm->waitForFrames((uint16)delay);
}

void ScriptOpcodes::opMoveActorToPoint(ScriptOpCall &scriptOpCall) {
	ARG_INT16(field0);
	ARG_INT16(iniId);
	ARG_INT16(walkSpeed);
	ARG_INT16(sequenceId);
	ARG_INT16(pointIndex);

	if (scriptOpCall._field8 != 0) {
		return;
	}

	bool waitForWalkToComplete = true;
	DragonINI *ini = _vm->getINI(iniId - 1);

	if (sequenceId & 0x8000) {
		waitForWalkToComplete = sequenceId == -1;
	}
	Common::Point point = _vm->_scene->getPoint(pointIndex);

	if (walkSpeed != -1) {
		if (sequenceId != -1) {
			if (!(field0 & 0x8000)) {
				assert(ini->actor);
				ini->actor->_flags |= ACTOR_FLAG_800;
				ini->actor->updateSequence(sequenceId & 0x7fff);
			}
			ini->actor->_walkSpeed = walkSpeed & 0x8000 ? (walkSpeed & 0x7fff) << 7 : walkSpeed << 0x10;
		}

		bool isFlicker = _vm->_dragonINIResource->isFlicker(ini);
		ini->actor->startWalk(point.x, point.y, isFlicker ? 0 : 1);

		if (waitForWalkToComplete) {
			ini->actor->waitForWalkToFinish();
		}
		ini->x = point.x;
		ini->y = point.y;
		ini->actor->clearFlag(ACTOR_FLAG_800);

	} else {
		ini->x = point.x;
		ini->actor->_x_pos = point.x;
		ini->y = point.y;
		ini->actor->_y_pos = point.y;

		if (sequenceId != -1) {
			ini->actor->_walkSpeed = -1;
			ini->actor->updateSequence(sequenceId & 0x7fff);
		}
	}
}

void ScriptOpcodes::opMoveActorToXY(ScriptOpCall &scriptOpCall) {
	ARG_INT16(field0);
	ARG_INT16(iniId);
	ARG_INT16(walkSpeed);
	ARG_INT16(sequenceId);
	ARG_INT16(destX);
	ARG_INT16(destY);

	if (scriptOpCall._field8 != 0) {
		return;
	}

	bool waitForWalkToComplete = true;
	DragonINI *ini = _vm->getINI(iniId - 1);

	if (sequenceId & 0x8000) {
		waitForWalkToComplete = sequenceId == -1;
	}

	if (walkSpeed != -1) {
		if (sequenceId != -1) {
			if (!(field0 & 0x8000)) {
				assert(ini->actor);
				ini->actor->setFlag(ACTOR_FLAG_800);
				ini->actor->updateSequence(sequenceId & 0x7fff);
			}
			ini->actor->_walkSpeed = walkSpeed & 0x8000 ? (walkSpeed & 0x7fff) << 7 : walkSpeed << 0x10;
		}
		bool isFlicker = _vm->_dragonINIResource->isFlicker(ini);
		ini->actor->startWalk(destX, destY, isFlicker ? 0 : 1);

		if (waitForWalkToComplete) {
			ini->actor->waitForWalkToFinish();
		}
		ini->x = destX;
		ini->y = destY;
		ini->actor->_flags &= ~ACTOR_FLAG_800;

	} else {
		assert(ini->actor);
		ini->x = destX;
		ini->actor->_x_pos = destX;
		ini->y = destY;
		ini->actor->_y_pos = destY;

		if (sequenceId != -1) {
			ini->actor->_walkSpeed = -1;
			ini->actor->updateSequence(sequenceId & 0x7fff);
		}
	}
}

void ScriptOpcodes::opMoveActorToObject(ScriptOpCall &scriptOpCall) {
	ARG_INT16(field0);
	ARG_INT16(field2);
	ARG_INT16(field4);
	ARG_INT16(field6);
	ARG_INT16(field8);

	//TODO these should be passed in as arguments and xparam should be set correctly.
	int16 someXParam = 0;
	int16 someYParam = _scriptTargetINI;
	uint someBooleanFlag = 0;
	if (scriptOpCall._field8 != 0) {
		return;
	}

	bool bVar1 = false;
	DragonINI *firstIni = _vm->getINI(field4 - 1);
	DragonINI *secondIni = _vm->getINI(field2 - 1);
//	secondIndexIntoINI = (uint)field2 - 1;
//	firstIndexIntoINI = (uint)field4 - 1;
	if ((field6 & 0x8000) != 0) {
		bVar1 = (field6 != -1);
	}
	if (field8 == -1) {
		if ((firstIni->flags & 1) == 0) {
			if ((int)firstIni->imgId != -1) {
				Img *firstDragonImg1 = _vm->_dragonImg->getImg(firstIni->imgId);
				int16 newXPos1 = firstDragonImg1->field_a + firstIni->baseXOffset;
				secondIni->x = newXPos1;
				secondIni->actor->_x_pos = newXPos1;
				int16 newYPos1 = firstDragonImg1->field_c + firstIni->baseYOffset;
				secondIni->y = newYPos1;
				secondIni->actor->_y_pos = newYPos1;
			}
		} else {
			int16 newYPos2 = firstIni->actor->_y_pos + firstIni->baseYOffset;
			firstIni->y = newYPos2;
			secondIni->actor->_y_pos = newYPos2;
			someXParam = firstIni->actor->_x_pos + firstIni->baseXOffset;
			secondIni->x = someXParam;
			secondIni->actor->_x_pos = someXParam;
		}
		if (field6 != -1) {
			secondIni->actor->_walkSpeed = -1;
			secondIni->actor->updateSequence(field6 & 0x7fff);
		}
		secondIni->x = someXParam;
		secondIni->y = someYParam;
		return;
	}
	if ((field8 & 0x8000) == 0) {
		someBooleanFlag = (uint)field8 << 0x10;
	} else {
		someBooleanFlag = ((uint)field8 & 0x7fff) << 7;
	}
	if (field6 != -1) {
		if (((int)(short)field0 & 0x8000) == 0) {
			secondIni->actor->setFlag(ACTOR_FLAG_800);
			secondIni->actor->updateSequence(field6 & 0x7fff);
		}
		secondIni->actor->_walkSpeed =
				someBooleanFlag;
	}
	int16 newXPosAgain = 0;
	int16 newYPosAgain = 0;
	someBooleanFlag = 1;
	if ((firstIni->flags & 1) == 0) {
		if ((int)firstIni->imgId == -1) {
			return;
		}
		Img *firstDragonImg2 = _vm->_dragonImg->getImg(firstIni->imgId);

		newXPosAgain = firstDragonImg2->field_a + firstIni->baseXOffset;
		newYPosAgain = firstDragonImg2->field_c + firstIni->baseYOffset;
		if (_vm->_dragonINIResource->isFlicker(secondIni)) {
			someBooleanFlag = 0;
		}
	} else {
		newXPosAgain = firstIni->actor->_x_pos + firstIni->baseXOffset;
		newYPosAgain = firstIni->actor->_y_pos + firstIni->baseYOffset;
		if (_vm->_dragonINIResource->isFlicker(secondIni)) {
			someBooleanFlag = 0;
		}
	}
	secondIni->actor->startWalk(newXPosAgain, newYPosAgain, someBooleanFlag);
	if (!bVar1) {
		secondIni->actor->waitForWalkToFinish();
	}

	secondIni->actor->_direction = firstIni->direction;

	secondIni->x = newXPosAgain;
	secondIni->y = newYPosAgain;
	secondIni->actor->clearFlag(ACTOR_FLAG_800);
}

void ScriptOpcodes::opUnk11FlickerTalk(ScriptOpCall &scriptOpCall) {
	ARG_SKIP(2);
	ARG_INT16(iniId)
	ARG_UINT32(textIndex)

	if (scriptOpCall._field8 != 0) {
		return;
	}

	_vm->_talk->talkFromIni(iniId, textIndex);
}

void ScriptOpcodes::opLoadScene(ScriptOpCall &scriptOpCall) {
	ARG_SKIP(2);
	ARG_INT16(newSceneID);
	ARG_INT16(cameraPointID);
	ARG_INT16(flickerDirection);

	if (scriptOpCall._field8 != 0) {
		return;
	}

	_vm->fadeToBlack();
	_vm->clearSceneUpdateFunction();
	_vm->_sound->resumeMusic();

	if (newSceneID != 0) {
		_vm->_scene->_mapTransitionEffectSceneID = _vm->_scene->getSceneId();
		_vm->_scene->setSceneId(newSceneID);
		_vm->_flickerInitialSceneDirection = flickerDirection;

		_vm->_scene->loadScene(newSceneID, cameraPointID);
	} else {
		_vm->setFlags(ENGINE_FLAG_100000);
	}
}

void ScriptOpcodes::opCodeActorTalk(ScriptOpCall &scriptOpCall) {
	ARG_SKIP(2);
	ARG_INT16(iniId);
	ARG_INT16(startSequenceId);
	ARG_INT16(endSequenceId);
	ARG_SKIP(2);
	ARG_UINT32(textIndex);

	if (scriptOpCall._field8 != 0) {
		return;
	}

	//TODO implement actor talk.

	uint16 dialog[2048];
	dialog[0] = 0;

	int sVar2 = -1; //TODO findTextToDtSpeechIndex(textIndex);

	if (!_vm->isUnkFlagSet(1) && (!_vm->isFlagSet(ENGINE_FLAG_1000_SUBTITLES_DISABLED) || sVar2 == -1)) {
		_vm->_talk->loadText(textIndex, dialog, 2048);
	}

	DragonINI *ini = iniId == 0 ? _vm->_dragonINIResource->getFlickerRecord() : _vm->getINI(iniId - 1);

	if ((ini->flags & 1) == 0) {
		Img *img = _vm->_dragonImg->getImg(ini->imgId);

		int y = img->field_e == 0 ? img->y : img->y << 3;

		_vm->_talk->FUN_8003239c(dialog,
				(int)(((uint)img->field_a - (uint)_vm->_scene->_camera.x) * 0x10000) >> 0x13,
				(int)(((y - 8) - (uint)_vm->_scene->_camera.y) * 0x10000) >> 0x13,
				READ_LE_INT16(_vm->_dragonOBD->getFromOpt(iniId) + 6),
				1,
				ini->actor, startSequenceId, endSequenceId, textIndex);
	} else {
		_vm->_talk->FUN_8003239c(dialog,
								 (int)(((uint)ini->actor->_x_pos - (uint)_vm->_scene->_camera.x) * 0x10000) >> 0x13,
								 (int)((((ini->actor->_y_pos - ini->actor->getFrameYOffset()) - (uint)_vm->_scene->_camera.y) * 0x10000) >> 0x13) - 3,
								 READ_LE_INT16(_vm->_dragonOBD->getFromOpt(iniId) + 6),
								 1,
								 ini->actor, startSequenceId, endSequenceId, textIndex);
	}
}

void ScriptOpcodes::setVariable(ScriptOpCall &scriptOpCall) {
	ARG_INT16(field0);
	ARG_INT16(field2);
	ARG_INT16(field4);
	ARG_INT16(field6);
	ARG_INT16(field8);
	ARG_INT8(fieldA);
	ARG_INT8(fieldB);

	enum S2Type {
		notSet,
		var,
		ini
	};

	S2Type s2Type = notSet;

	int16 s1 = 0;
	int16 s2 = 0;

	if (fieldA & 8) {
		// VAR
		s1 = _vm->getVar(field6 / 2);
	} else if (fieldA & 0x10) {
		// INI
		s1 = getINIField(field8 - 1, field6);
	} else {
		s1 = field8;
	}

	if (fieldA & 1) {
		debug(3, "fieldA & 1");
		bool flagValue = s1 != 0 ? true : false;
		_dragonFLG->set(field2 * 8 + field4, flagValue);
	}

	if (fieldA & 2) {
		debug(3, "fieldA & 2");
		s2Type = var;
		s2 = _vm->getVar(field2);
	}

	if (fieldA & 4) {
		debug(3, "fieldA & 4");
		s2Type = ini;
		s2 = getINIField(field4 - 1, field2);
		DragonINI *ini = _vm->getINI(field4 - 1);

		if (field2 == 0x1a && ini->flags & 1 && ini->sceneId == _vm->getCurrentSceneId()) {
			if (s1 & 2) {
				ini->actor->_flags |= ACTOR_FLAG_80;
				ini->actor->_scale = DRAGONS_ENGINE_SPRITE_100_PERCENT_SCALE;
			} else {
				ini->actor->_flags &= ~ACTOR_FLAG_80;
			}

			if (s1 & 4) {
				ini->actor->_flags |= ACTOR_FLAG_8000;
			} else {
				ini->actor->_flags &= ~ACTOR_FLAG_8000;
			}
		}

		if (fieldA & 4 && field2 == 0 && !(ini->flags & 1) && ini->sceneId == _vm->getCurrentSceneId()) {
			if (s1 == -1) {
				if (ini->iptIndex_maybe != -1) {
					_vm->_scene->removeImageOverlay(ini->iptIndex_maybe);
				}
			} else {
				_vm->_scene->loadImageOverlay(s1);
			}
		}
	}

	if (s2Type != notSet) {
		if (fieldB == 1) {
			s2 += s1;
		} else {
			if (fieldB < 2) {
				if (fieldB == 0) {
					s2 = s1;
				}
			} else {
				if (fieldB == 2) {
					s2 -= s1;
				} else {
					if (fieldB == 3) {
						s2 = _vm->getRand(MAX<int16>(1, s1));
					}
				}
			}
		}

		if (s2Type == ini) {
			setINIField(field4 - 1, field2, s2);
		} else  { //var type
			_vm->setVar(field2, s2);
		}
	}
}

uint16 ScriptOpcodes::getINIField(uint32 iniIndex, uint16 fieldOffset) {
	DragonINI *ini = _vm->getINI(iniIndex);

	switch (fieldOffset) {
	case 0 : return ini->iptIndex_maybe;
	case 4 : return ini->actorResourceId;
	case 6 : return ini->sequenceId;
	case 0xC  : return ini->sceneId;
	case 0xE  : return ini->direction;
	case 0x10 : return ini->counter;
	case 0x12  : return ini->objectState;
	case 0x14 : return ini->objectState2;
	case 0x16 : return ini->x;
	case 0x18 : return ini->y;
	case 0x1A : return ini->flags;
	case 0x1C : return ini->baseXOffset;
	case 0x1E : return ini->baseYOffset;
	case 0x20 : return ini->direction2;
	default: error("getINIField() Invalid fieldOffset 0x%X", fieldOffset);
	}
}

void ScriptOpcodes::setINIField(uint32 iniIndex, uint16 fieldOffset, uint16 value) {
	DragonINI *ini = _vm->getINI(iniIndex);

	switch (fieldOffset) {
	case 0 : ini->iptIndex_maybe = value; break;
	case 4 : ini->actorResourceId = value; break;
	case 6 : ini->sequenceId = value; break;
	case 0xc : ini->sceneId = value; break;
	case 0xe : ini->direction = value; break;
	case 0x10 : ini->counter = value; break;
	case 0x12 : ini->objectState = value; break;
	case 0x14 : ini->objectState2 = value; break;
	case 0x16 : ini->x = value; break;
	case 0x18 : ini->y = value; break;
	case 0x1A : ini->flags = value; break;
	case 0x1C : ini->baseXOffset = value; break;
	case 0x1E : ini->baseYOffset = value; break;
	case 0x20 : ini->direction2 = value; break;
	default: error("setINIField() Invalid fieldOffset 0x%X", fieldOffset);
	}

}

void ScriptOpcodes::opUnk16(ScriptOpCall &scriptOpCall) {
	if (scriptOpCall._field8 == 4) {
		scriptOpCall._result |= 1;
	} else {
		scriptOpCall._code += 4;
	}
}

void ScriptOpcodes::opWaitForActorSequenceToFinish(ScriptOpCall &scriptOpCall) {
	ARG_SKIP(2);
	ARG_INT16(iniId);

	DragonINI *ini = _vm->getINI(iniId - 1);
	if (ini->flags & 1) {
		while (!(ini->actor->_flags & ACTOR_FLAG_4)) {
			_vm->waitForFrames(1);
		}
	}

}


void ScriptOpcodes::opDialogAtPoint(ScriptOpCall &scriptOpCall) {
	ARG_INT16(field0);
	ARG_UINT32(field2);
	ARG_INT16(x);
	ARG_INT16(y);
	ARG_INT16(fieldA);

	if (scriptOpCall._field8 != 0) {
		return;
	}

	uint16 dialog[2000];
	dialog[0] = 0;
	_vm->_talk->loadText(field2, dialog, 2000);

//	if (((unkFlags1 & 1) == 0) && (((engine_flags_maybe & 0x1000) == 0 || (sVar1 == -1)))) {
//		dialogText = (uint8 *)load_string_from_dragon_txt(offset, acStack2016);
//	}

	if (fieldA != 0) {
		fieldA = READ_LE_INT16(_vm->_dragonOBD->getFromOpt(fieldA - 1) + 6);
	}
	_vm->_talk->displayDialogAroundPoint(dialog, x, y, fieldA, 1, field2);
}

void ScriptOpcodes::opExecuteObjectSceneScript(ScriptOpCall &scriptOpCall) {
	ARG_INT16(field0);
	ARG_INT16(size);

	if (scriptOpCall._field8 == 3) {
		ScriptOpCall newCall(scriptOpCall._code, size);
		_vm->_scriptOpcodes->runScript(newCall);
	}
	scriptOpCall._code += size;

}

void ScriptOpcodes::opUpdatePaletteCycling(ScriptOpCall &scriptOpCall) {
	ARG_SKIP(2);
	ARG_INT16(index);
	ARG_INT16(field4);
	ARG_INT16(field6);
	ARG_INT16(field8);
	ARG_INT16(fieldA);

	if (scriptOpCall._field8 != 0) {
		return;
	}

	assert(index < 8);

	_vm->_paletteCyclingTbl[index].paletteType = field4;
	_vm->_paletteCyclingTbl[index].startOffset = field6;
	_vm->_paletteCyclingTbl[index].endOffset = field8;
	_vm->_paletteCyclingTbl[index].updateInterval = fieldA;
	_vm->_paletteCyclingTbl[index].updateCounter = 0;
}

void ScriptOpcodes::opWaitForActorToFinishWalking(ScriptOpCall &scriptOpCall) {
	ARG_SKIP(2);
	ARG_INT16(iniId);

	if (scriptOpCall._field8 != 0) {
		return;
	}

	DragonINI *ini = _vm->getINI(iniId - 1);
	if (ini->flags & 1) {
		ini->actor->waitForWalkToFinish();
	}
}

void ScriptOpcodes::opShowActor(ScriptOpCall &scriptOpCall) {
	ARG_SKIP(2);
	ARG_INT16(iniId);

	if (scriptOpCall._field8 != 0) {
		return;
	}

	DragonINI *ini = _vm->getINI(iniId - 1);
	ini->actor->clearFlag(ACTOR_FLAG_400);
}

void ScriptOpcodes::opHideActor(ScriptOpCall &scriptOpCall) {
	ARG_SKIP(2);
	ARG_INT16(iniId);

	if (scriptOpCall._field8 != 0) {
		return;
	}

	DragonINI *ini = _vm->getINI(iniId - 1);
	ini->actor->setFlag(ACTOR_FLAG_400);
}

void ScriptOpcodes::opSetActorFlag0x1000(ScriptOpCall &scriptOpCall) {
	ARG_SKIP(2);
	ARG_INT16(iniId);

	if (scriptOpCall._field8 != 0) {
		return;
	}

	DragonINI *ini = _vm->getINI(iniId - 1);
	ini->actor->setFlag(ACTOR_FLAG_1000);
}

void ScriptOpcodes::opCode_Unk7(ScriptOpCall &scriptOpCall) {

}

void ScriptOpcodes::loadTalkDialogEntries(ScriptOpCall &scriptOpCall) {
	scriptOpCall._field8 = 2;
	scriptOpCall._result = 0;
	_vm->_talk->clearDialogEntries();
	_numDialogStackFramesToPop = 0;
	executeScriptLoop(scriptOpCall);

}

} // End of namespace Dragons
