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
#include "dragons/dragons.h"
#include "dragons/dragonflg.h"
#include "dragons/dragonini.h"
#include "dragons/dragonobd.h"
#include "dragons/scene.h"
#include "dragons/scriptopcodes.h"
#include "dragons/specialopcodes.h"
#include "dragons/actor.h"
#include "scriptopcodes.h"


namespace Dragons {
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

// ScriptOpcodes

ScriptOpcodes::ScriptOpcodes(DragonsEngine *vm, DragonFLG *dragonFLG)
	: _vm(vm), _dragonFLG(dragonFLG), _data_80071f5c(0) {
	_specialOpCodes = new SpecialOpcodes(_vm);
	initOpcodes();
}

ScriptOpcodes::~ScriptOpcodes() {
	freeOpcodes();
	delete _specialOpCodes;
}

void ScriptOpcodes::execOpcode(ScriptOpCall &scriptOpCall) {
	if (!_opcodes[scriptOpCall._op])
		error("ScriptOpcodes::execOpcode() Unimplemented opcode %d (0x%X)", scriptOpCall._op, scriptOpCall._op);
	debug(3, "execScriptOpcode(%d) %s", scriptOpCall._op, _opcodeNames[scriptOpCall._op].c_str());
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
	// OPCODE(1, opUnk1);

	OPCODE(4,  opExecuteScript);
	OPCODE(5,  opActorSetSequenceID2);

	OPCODE(7,  opUnk7);
	OPCODE(8,  opActorLoadSequence);

	OPCODE(0xA, opUnkA);
	OPCODE(0xB, opRunSpecialOpCode);
	OPCODE(0xC, opUnkCSoundRelatedMaybe);
	OPCODE(0xD, opDelay);
	OPCODE(0xE, opUnkE);
	OPCODE(0xF, opUnkF);

	OPCODE(0x11, opUnk11FlickerTalk);

	OPCODE(0x13, opUnk13PropertiesRelated);
	OPCODE(0x1F, opPlayMusic);

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
	_data_80071f5c = 0;
	executeScriptLoop(scriptOpCall);
}


void ScriptOpcodes::runScript3(ScriptOpCall &scriptOpCall) {
	scriptOpCall._field8 = 3;
	scriptOpCall._result = 0;
	_data_80071f5c = 0;
	executeScriptLoop(scriptOpCall);
}

void ScriptOpcodes::executeScriptLoop(ScriptOpCall &scriptOpCall) {

	if (scriptOpCall._code >= scriptOpCall._codeEnd || scriptOpCall._result & 1) {
		return;
	}

	if (_vm->isFlagSet(Dragons::ENGINE_FLAG_100000)) {
		return;
	}

	if (_vm->isFlagSet(Dragons::ENGINE_FLAG_80000)) {
		//TODO
//		if (IsPressedStart(0)) {
//			Dragons::getEngine()->setFlags(Dragons::ENGINE_FLAG_100000);
//		}
	}

	uint16 opcode = READ_LE_UINT16(scriptOpCall._code) & 0x7fff;

	scriptOpCall._op = (byte) opcode;
	if (opcode < DRAGONS_NUM_SCRIPT_OPCODES) {
		execOpcode(scriptOpCall);
	}

	while (scriptOpCall._code < scriptOpCall._codeEnd && !(scriptOpCall._result & 1) && _data_80071f5c == 0) {

		if (_vm->isFlagSet(Dragons::ENGINE_FLAG_100000)) {
			return;
		}

		if (_vm->isFlagSet(Dragons::ENGINE_FLAG_80000)) {
			//TODO
//		if (IsPressedStart(0)) {
//			Dragons::getEngine()->setFlags(Dragons::ENGINE_FLAG_100000);
//		}
		}

		opcode = READ_LE_UINT16(scriptOpCall._code) & 0x7fff;

		if (opcode >= DRAGONS_NUM_SCRIPT_OPCODES) {
			return; //TODO should continue here.
		}
		scriptOpCall._op = (byte) opcode;
		execOpcode(scriptOpCall);
	}
}

// Opcodes

void ScriptOpcodes::opUnk1(ScriptOpCall &scriptOpCall) {
//	ARG_INT16(framePointer);
//	debug(3, "set frame pointer %X", framePointer);
//	actor->loadFrame((uint16)framePointer);
//	actor->flags |= Dragons::ACTOR_FLAG_2;
//	actor->sequenceTimer = actor->field_c;
//	updateReturn(scriptOpCall, 1);
}

void ScriptOpcodes::opExecuteScript(ScriptOpCall &scriptOpCall) {
	ARG_SKIP(2);
	ARG_UINT32(obdOffset);
	ScriptOpCall newScriptOpCall;
	byte *data =_vm->_dragonOBD->getObdAtOffset(obdOffset);

	newScriptOpCall._code = data + 4;
	newScriptOpCall._codeEnd = newScriptOpCall._code + READ_LE_UINT32(data);
	newScriptOpCall._field8 = scriptOpCall._field8;
	newScriptOpCall._result = 0;

	executeScriptLoop(newScriptOpCall);
}

void ScriptOpcodes::opActorSetSequenceID2(ScriptOpCall &scriptOpCall) {
	ARG_SKIP(2);
	ARG_INT16(iniIndex);
	ARG_SKIP(2);
	ARG_INT16(sequenceId);

	if (scriptOpCall._field8 == 0) {
		_vm->getINI(iniIndex - 1)->actor->_sequenceID2 = sequenceId;
	}
}


void ScriptOpcodes::opUnk7(ScriptOpCall &scriptOpCall) {
	if (scriptOpCall._field8 == 0) {
		opCode_Unk7(scriptOpCall);
	} else {
		scriptOpCall._code += 6;
	}

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
		ini->actor->flags |= Dragons::ACTOR_FLAG_2000;
	}

	assert(ini->actor->_actorResource);
	assert(ini->actor->_actorResource->_id == ini->actorResourceId); // TODO need to rework selecting resourceId for an actor.
	ini->actor->updateSequence(sequenceId);

	if (field0 & 0x8000) {
		ini->actor->waitUntilFlag8And4AreSet();
	}

	if (isFlicker) {
		ini->actor->flags &= ~Dragons::ACTOR_FLAG_2000;
	}
}

void ScriptOpcodes::opPlayMusic(ScriptOpCall &scriptOpCall) {
	byte *code = scriptOpCall._code;
	scriptOpCall._code += 4;
	if (scriptOpCall._field8 == 0) {
		//TODO play music here.
	}
}

void ScriptOpcodes::opUnk13PropertiesRelated(ScriptOpCall &scriptOpCall) {
	if (checkPropertyFlag(scriptOpCall)) {
		scriptOpCall._code += 4;
	} else {
		scriptOpCall._code += 4 + READ_LE_UINT16(scriptOpCall._code);
	}
}

bool ScriptOpcodes::checkPropertyFlag(ScriptOpCall &scriptOpCall) {
	byte *codePtrOffsetA = scriptOpCall._code + 0xA;
	byte *codePtrOffset2 = scriptOpCall._code + 2;

	uint16 status = 0;
	uint16 result = 0;

	uint16 t2 = 0;
	uint16 t0 = 0;

	for(;;) {
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
				debug(3, "Op13 get here!! & 4 %d, %d", READ_LE_INT16(codePtrOffsetA - 6), t2);

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
				debug(3, "Op13 get here!! & 0x18");
				t0 = READ_LE_UINT16(codePtrOffsetA - 2);
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

void ScriptOpcodes::opUnkA(ScriptOpCall &scriptOpCall) {
	if (scriptOpCall._field8 == 0) {
		opCode_UnkA_setsProperty(scriptOpCall);
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

	debug("Special opCode %X", specialOpCode);
	_specialOpCodes->run(specialOpCode);
}

void ScriptOpcodes::opUnkCSoundRelatedMaybe(ScriptOpCall &scriptOpCall) {
	ARG_SKIP(2);
	ARG_INT16(soundId);

	if (scriptOpCall._field8 == 0) {
		_vm->playSound((uint16)soundId);
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

void ScriptOpcodes::opUnkE(ScriptOpCall &scriptOpCall) {
	ARG_INT16(field0);
	ARG_INT16(field2);
	ARG_INT16(field4);
	ARG_INT16(field6);
	ARG_INT16(field8);

	if (scriptOpCall._field8 != 0) {
		return;
	}

	int32 s3 = 0;
	DragonINI *ini = _vm->getINI(field2 - 1);

	if (field6 & 0x8000) {
		s3 = 0 > (field6 ^ 0xffff) ? 1 : 0;
	}
	Common::Point point = _vm->_scene->getPoint(field8);

	if (field4 != -1) {
		if (field6 != -1) {
			if (!(field0 & 0x8000)) {
				assert(ini->actor);
				ini->actor->flags |= Dragons::ACTOR_FLAG_800;
				ini->actor->updateSequence(field6 & 0x7fff);
			}
			ini->actor->field_7c = field4 & 0x8000 ? (field4 & 0x7fff) << 0x10 : (field4 & 0x7fff) << 7;
		}

		bool isFlicker = _vm->_dragonINIResource->isFlicker(ini);
		ini->actor->pathfinding_maybe(point.x, point.y, isFlicker ? 0 : 1);

		if(s3 == 0) {
			while (ini->actor->flags & Dragons::ACTOR_FLAG_10) {
				_vm->waitForFrames(1);
			}
		}
		ini->x = point.x;
		ini->y = point.y;
		ini->actor->flags &= ~Dragons::ACTOR_FLAG_800;

	} else {
		ini->x = point.x;
		ini->actor->x_pos = point.x;
		ini->y = point.y;
		ini->actor->y_pos = point.y;

		if (field4 != field6) {
			ini->actor->field_7c = field4;
			ini->actor->updateSequence(field6 & 0x7fff);
		}
	}
}

void ScriptOpcodes::opUnkF(ScriptOpCall &scriptOpCall) {
	ARG_INT16(field0);
	ARG_INT16(field2);
	ARG_INT16(field4);
	ARG_INT16(field6);
	ARG_INT16(field8);
	ARG_INT16(fieldA);

	if (scriptOpCall._field8 != 0) {
		return;
	}

	int32 s3 = 0;
	DragonINI *ini = _vm->getINI(field2 - 1);

	if (field6 & 0x8000) {
		s3 = 0 > (field6 ^ 0xffff) ? 1 : 0;
	}

	if (field4 != -1) {
		if (field6 != -1) {
			if (!(field0 & 0x8000)) {
				assert(ini->actor);
				ini->actor->flags |= Dragons::ACTOR_FLAG_800;
				ini->actor->updateSequence(field6 & 0x7fff);
			}
			ini->actor->field_7c = field4 & 0x8000 ? (field4 & 0x7fff) << 0x10 : (field4 & 0x7fff) << 7;
		}
		bool isFlicker = _vm->_dragonINIResource->isFlicker(ini);
		ini->actor->pathfinding_maybe(field8, fieldA, isFlicker ? 0 : 1);

		if(s3 == 0) {
			while (ini->actor->flags & Dragons::ACTOR_FLAG_10) {
				_vm->waitForFrames(1);
			}
		}
		ini->x = field8;
		ini->y = fieldA;
		ini->actor->flags &= ~Dragons::ACTOR_FLAG_800;

	} else {
		assert(ini->actor);
		ini->x = field8;
		ini->actor->x_pos = field8;
		ini->y = fieldA;
		ini->actor->y_pos = fieldA;

		if (field4 != field6) {
			ini->actor->field_7c = field4;
		}
		ini->actor->updateSequence(field6 & 0x7fff);
	}
}

void ScriptOpcodes::opUnk11FlickerTalk(ScriptOpCall &scriptOpCall) {
	ARG_SKIP(8);
	// TODO implement me!
}

void ScriptOpcodes::opCodeActorTalk(ScriptOpCall &scriptOpCall) {
	ARG_SKIP(2);
	ARG_INT16(field2);
	ARG_SKIP(6);
	ARG_UINT32(fieldA);

	if (scriptOpCall._field8 != 0) {
		return;
	}

	//TODO implement actor talk.
}

void ScriptOpcodes::opCode_UnkA_setsProperty(ScriptOpCall &scriptOpCall) {
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

		if (field2 == 0x1a && ini->field_1a_flags_maybe & 1 && ini->sceneId == _vm->getCurrentSceneId()) {
			if (s1 & 2) {
				ini->actor->flags |= Dragons::ACTOR_FLAG_80;
				ini->actor->field_e = 0x100;
			} else {
				ini->actor->flags &= ~Dragons::ACTOR_FLAG_80;
			}

			if (s1 & 4) {
				ini->actor->flags |= Dragons::ACTOR_FLAG_8000;
			} else {
				ini->actor->flags &= ~Dragons::ACTOR_FLAG_8000;
			}
		}

		if (fieldA & 4 && field2 == 0 && !(ini->field_1a_flags_maybe & 1) && ini->sceneId == _vm->getCurrentSceneId()) {
			if (s1 == -1) {
				if (ini->iptIndex_maybe != -1) {
					// TODO ipt_img_file_related_3(ini->iptIndex_maybe);
					//error("TODO ipt_img_file_related_3(ini->iptIndex_maybe);");
				}
			} else {
				// TODO ipt_img_file_related_2(s1);
				//error("TODO ipt_img_file_related_2(s1);");
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
						//TODO s2 = sub_80023830(s1);
						error("TODO s2 = sub_80023830(s1);");
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
		case 6 : return ini->sequenceId;
		case 0xC  : return ini->sceneId;
		case 0x12  : return ini->field_12;
		case 0x14 : return ini->field_14;
		case 0x16 : return ini->x;
		case 0x18 : return ini->y;
		case 0x1A : return ini->field_1a_flags_maybe;
		case 0x20 : return ini->field_20_actor_field_14;
		default: error("getINIField() Invalid fieldOffset 0x%X", fieldOffset);
	}
}

void ScriptOpcodes::setINIField(uint32 iniIndex, uint16 fieldOffset, uint16 value) {
	DragonINI *ini = _vm->getINI(iniIndex);

	switch (fieldOffset) {
		case 0 : ini->iptIndex_maybe = value; break;
		case 6 : ini->sequenceId = value; break;
		case 0xc  : ini->sceneId = value; break;
		case 0x12 : ini->field_12 = value; break;
		case 0x14 : ini->field_14 = value; break;
		case 0x16 : ini->x = value; break;
		case 0x18 : ini->y = value; break;
		case 0x1A : ini->field_1a_flags_maybe = value; break;
		case 0x20 : ini->field_20_actor_field_14 = value; break;
		default: error("setINIField() Invalid fieldOffset 0x%X", fieldOffset);
	}

}

void ScriptOpcodes::opUnk14(ScriptOpCall &scriptOpCall) {

}

void ScriptOpcodes::opUnk15(ScriptOpCall &scriptOpCall) {

}

void ScriptOpcodes::opCode_Unk7(ScriptOpCall &scriptOpCall) {
	ARG_INT16(field0);
	ARG_INT16(field2);
	ARG_INT16(sceneId); //sceneId

	uint16 currentScene = _vm->getCurrentSceneId();
	DragonINI *ini = _vm->getINI(field2 - 1);
	if (!(field0 & 0x8000)) {

		if (ini->field_1a_flags_maybe & 1) {
			if (ini->sceneId == currentScene) {
				assert(ini->actor);
				ini->actor->reset_maybe();
			}
			if (sceneId == currentScene) {
				ini->actor = _vm->_actorManager->loadActor(ini->actorResourceId, ini->sequenceId, ini->x, ini->y, 0);
				ini->actor->_sequenceID2 = ini->field_20_actor_field_14;
				if (ini->field_1a_flags_maybe & 2) {
					ini->actor->flags |= Dragons::ACTOR_FLAG_80;
				} else {
					ini->actor->flags &= ~Dragons::ACTOR_FLAG_80;
				}

				if (ini->field_1a_flags_maybe & 0x20) {
					ini->actor->flags |= Dragons::ACTOR_FLAG_100;
				} else {
					ini->actor->flags &= ~Dragons::ACTOR_FLAG_100;
				}

				if (ini->field_1a_flags_maybe & 4) {
					ini->actor->flags |= Dragons::ACTOR_FLAG_8000;
				} else {
					ini->actor->flags &= ~Dragons::ACTOR_FLAG_8000;
				}

				if (ini->field_1a_flags_maybe & 0x100) {
					ini->actor->flags |= Dragons::ACTOR_FLAG_4000;
				} else {
					ini->actor->flags &= ~Dragons::ACTOR_FLAG_4000;
				}
			}
		} else {
			if (ini->sceneId == currentScene && ini->iptIndex_maybe != -1) {
				//TODO ipt_img_file_related_3(ini->iptIndex_maybe);
				error("//TODO ipt_img_file_related_3(ini->iptIndex_maybe);");
			}
			if (sceneId == currentScene && ini->iptIndex_maybe != -1) {
				// TODO ipt_img_file_related_2(ini->iptIndex_maybe);
				error("// TODO ipt_img_file_related_2(ini->iptIndex_maybe);");
			}
		}

		if (ini->sceneId == 1) {
			//TODO 0x8002d218
		}

		if (sceneId == 1) {
			// TODO 0x8002d2f4
		}
	}
	ini->sceneId = sceneId;
}

} // End of namespace Dragons
