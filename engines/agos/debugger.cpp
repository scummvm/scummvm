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
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.
 *
 */

#include "common/config-manager.h"

#include "agos/debugger.h"
#include "agos/agos.h"
#include "agos/midi.h"
#include "agos/sound.h"

namespace AGOS {

Debugger::Debugger(AGOSEngine *vm)
	: GUI::Debugger() {
	_vm = vm;

	registerCmd("continue", WRAP_METHOD(Debugger, cmdExit));
	registerCmd("music",    WRAP_METHOD(Debugger, Cmd_PlayMusic));
	registerCmd("sound",    WRAP_METHOD(Debugger, Cmd_PlaySound));
	registerCmd("voice",    WRAP_METHOD(Debugger, Cmd_PlayVoice));
	registerCmd("bit",      WRAP_METHOD(Debugger, Cmd_SetBit));
	registerCmd("bit2",     WRAP_METHOD(Debugger, Cmd_SetBit2));
	registerCmd("bit3",     WRAP_METHOD(Debugger, Cmd_SetBit3));
	registerCmd("var",      WRAP_METHOD(Debugger, Cmd_SetVar));
	registerCmd("obj",      WRAP_METHOD(Debugger, Cmd_SetObjectFlag));
	registerCmd("sub",      WRAP_METHOD(Debugger, Cmd_StartSubroutine));
	registerCmd("dumpimage",      WRAP_METHOD(Debugger, Cmd_dumpImage));
	registerCmd("dumpscript",     WRAP_METHOD(Debugger, Cmd_dumpScript));

}

bool Debugger::Cmd_PlayMusic(int argc, const char **argv) {
	if (argc > 1) {
		uint music = atoi(argv[1]);
		if (music <= _vm->_numMusic) {
			if (_vm->getGameType() == GType_PP) {
				// TODO
			} else if (_vm->getGameType() == GType_SIMON2) {
				_vm->loadMusic(music);
				_vm->_midi->startTrack(0);
			} else {
				_vm->playMusic(music, 0);
			}
		} else
			debugPrintf("Music out of range (0 - %d)\n", _vm->_numMusic);
	} else
		debugPrintf("Syntax: music <musicnum>\n");

	return true;
}

bool Debugger::Cmd_PlaySound(int argc, const char **argv) {
	if (argc > 1) {
		uint sound = atoi(argv[1]);
		if (sound <= _vm->_numSFX)
			_vm->_sound->playEffects(sound);
		else
			debugPrintf("Sound out of range (0 - %d)\n", _vm->_numSFX);
	} else
		debugPrintf("Syntax: sound <soundnum>\n");

	return true;
}

bool Debugger::Cmd_PlayVoice(int argc, const char **argv) {
	if (argc > 1) {
		uint voice = atoi(argv[1]);
		if (voice <= _vm->_numSpeech)
			_vm->_sound->playVoice(voice);
		else
			debugPrintf("Voice out of range (0 - %d)\n", _vm->_numSpeech);
	} else
		debugPrintf("Syntax: voice <voicenum>\n");

	return true;
}

bool Debugger::Cmd_SetBit(int argc, const char **argv) {
	uint bit, value;
	if (argc > 2) {
		bit = atoi(argv[1]);
		value = atoi(argv[2]);
		if (value <= 1) {
			_vm->setBitFlag(bit, value != 0);
			debugPrintf("Set bit %d to %d\n", bit, value);
		} else
			debugPrintf("Bit value out of range (0 - 1)\n");
	} else if (argc > 1) {
		bit = atoi(argv[1]);
		value = _vm->getBitFlag(bit);
		debugPrintf("Bit %d is %d\n", bit, value);
	} else
		debugPrintf("Syntax: bit <bitnum> <value>\n");

	return true;
}

bool Debugger::Cmd_SetBit2(int argc, const char **argv) {
	uint bit, value;
	if (argc > 2) {
		bit = atoi(argv[1]);
		value = atoi(argv[2]);
		if (value == 0) {
			_vm->_bitArrayTwo[bit / 16] &= ~(1 << (bit & 15));
			debugPrintf("Set bit2 %d to %d\n", bit, value);
		} else if (value == 1) {
			_vm->_bitArrayTwo[bit / 16] |= (1 << (bit & 15));
			debugPrintf("Set bit2 %d to %d\n", bit, value);
		} else
			debugPrintf("Bit2 value out of range (0 - 1)\n");
	} else if (argc > 1) {
		bit = atoi(argv[1]);
		value = (_vm->_bitArrayTwo[bit / 16] & (1 << (bit & 15))) != 0;
		debugPrintf("Bit2 %d is %d\n", bit, value);
	} else
		debugPrintf("Syntax: bit2 <bitnum> <value>\n");

	return true;
}

bool Debugger::Cmd_SetBit3(int argc, const char **argv) {
	uint bit, value;
	if (argc > 2) {
		bit = atoi(argv[1]);
		value = atoi(argv[2]);
		if (value == 0) {
			_vm->_bitArrayThree[bit / 16] &= ~(1 << (bit & 15));
			debugPrintf("Set bit3 %d to %d\n", bit, value);
		} else if (value == 1) {
			_vm->_bitArrayThree[bit / 16] |= (1 << (bit & 15));
			debugPrintf("Set bit3 %d to %d\n", bit, value);
		} else
			debugPrintf("Bit3 value out of range (0 - 1)\n");
	} else if (argc > 1) {
		bit = atoi(argv[1]);
		value = (_vm->_bitArrayThree[bit / 16] & (1 << (bit & 15))) != 0;
		debugPrintf("Bit3 %d is %d\n", bit, value);
	} else
		debugPrintf("Syntax: bit3 <bitnum> <value>\n");

	return true;
}

bool Debugger::Cmd_SetVar(int argc, const char **argv) {
	uint var, value;
	if (argc > 1) {
		var = atoi(argv[1]);
		if (var < _vm->_numVars) {
			if (argc > 2) {
				value = atoi(argv[2]);
				_vm->writeVariable(var, value);
				debugPrintf("Set var %d to %d\n", var, value);
			} else {
				value = _vm->readVariable(var);
				debugPrintf("Var %d is %d\n", var, value);
			}
		} else
			debugPrintf("Var out of range (0 - %d)\n", _vm->_numVars - 1);
	} else
		debugPrintf("Syntax: var <varnum> <value>\n");

	return true;
}

bool Debugger::Cmd_SetObjectFlag(int argc, const char **argv) {
	uint obj, prop, value;
	if (argc > 2) {
		obj = atoi(argv[1]);
		prop = atoi(argv[2]);

		if (obj >= 1 && obj < _vm->_itemArraySize) {
			SubObject *o = (SubObject *)_vm->findChildOfType(_vm->derefItem(obj), kObjectType);
			if (o != NULL) {
				if (o->objectFlags & (1 << prop) && prop < 16) {
					uint offs = _vm->getOffsetOfChild2Param(o, 1 << prop);
					if (argc > 3) {
						value = atoi(argv[3]);
						o->objectFlagValue[offs] = value;
						debugPrintf("Object %d Flag %d set to %d\n", obj, prop, value);
					} else {
						value = o->objectFlagValue[offs];
						debugPrintf("Object %d Flag %d is %d\n", obj, prop, value);
					}
				} else {
					debugPrintf("Object flag out of range\n");
				}
			} else {
				debugPrintf("Item isn't an object\n");
			}
		} else {
			debugPrintf("Item out of range (1 - %d)\n", _vm->_itemArraySize - 1);
		}
	} else {
		debugPrintf("Syntax: obj <itemnum> <flag> <value>\n");
	}

	return true;
}

bool Debugger::Cmd_StartSubroutine(int argc, const char **argv) {
	if (argc > 1) {
		uint subroutine = atoi(argv[1]);
		Subroutine *sub;
		sub = _vm->getSubroutineByID(subroutine);
		if (sub != NULL)
			_vm->startSubroutine(sub);
	} else
		debugPrintf("Subroutine %d\n", _vm->_currentTable->id);

	return true;
}

bool Debugger::Cmd_dumpImage(int argc, const char **argv) {
	if (argc > 1) {
		uint16 zoneNum = atoi(argv[1]);
		_vm->loadZone(zoneNum, false);
		VgaPointersEntry *vpe = &_vm->_vgaBufferPointers[zoneNum];
		if (vpe->vgaFile2 != NULL) {
			_vm->dumpVgaBitmaps(zoneNum);
		} else {
			debugPrintf("Invalid Zone Number %d\n", zoneNum);

		}
	} else
		debugPrintf("Syntax: dumpimage <zonenum>\n");

	return true;
}

bool Debugger::Cmd_dumpScript(int argc, const char **argv) {
	if (argc > 1) {
		uint16 zoneNum = atoi(argv[1]);
		_vm->loadZone(zoneNum, false);
		VgaPointersEntry *vpe = &_vm->_vgaBufferPointers[zoneNum];
		if (vpe->vgaFile1 != NULL) {
			_vm->dumpVgaFile(vpe->vgaFile1);
		} else {
			debugPrintf("Invalid Zone Number %d\n", zoneNum);

		}
	} else
		debugPrintf("Syntax: dumpscript <zonenum>\n");

	return true;
}

} // End of namespace AGOS
