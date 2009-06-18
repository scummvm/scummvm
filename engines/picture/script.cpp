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
 * $URL$
 * $Id$
 *
 */

// TODO: Clean up game variable handling and move it to PictureEngine

#include "common/events.h"
#include "common/keyboard.h"
#include "common/file.h"
#include "common/savefile.h"
#include "common/config-manager.h"

#include "base/plugins.h"
#include "base/version.h"

#include "sound/mixer.h"

#include "picture/picture.h"
#include "picture/animation.h"
#include "picture/movie.h"
#include "picture/palette.h"
#include "picture/resource.h"
#include "picture/script.h"
#include "picture/screen.h"
#include "picture/segmap.h"
#include "picture/sound.h"

namespace Picture {

ScriptInterpreter::ScriptInterpreter(PictureEngine *vm) : _vm(vm) {

	_stack = new byte[kScriptStackSize];

	memset(_slots, 0, sizeof(_slots));

	_savedSp = 0;

	_slots[kMaxScriptSlots - 1].size = 1024;
 	_slots[kMaxScriptSlots - 1].data = new byte[_slots[kMaxScriptSlots - 1].size];

	setupScriptFunctions();

}

ScriptInterpreter::~ScriptInterpreter() {
	delete[] _stack;
	for (int i = 0; i < kMaxScriptSlots; i++)
		delete[] _slots[i].data;
	for (uint i = 0; i < _scriptFuncs.size(); ++i)
		delete _scriptFuncs[i];
}

typedef Common::Functor0Mem<void, ScriptInterpreter> ScriptFunctionF;
#define RegisterScriptFunction(x) \
	_scriptFuncs.push_back(new ScriptFunctionF(this, &ScriptInterpreter::x));  \
	_scriptFuncNames.push_back(#x);
void ScriptInterpreter::setupScriptFunctions() {

	RegisterScriptFunction(sfNop);
	RegisterScriptFunction(sfNop);
	RegisterScriptFunction(sfGetGameVar);
	RegisterScriptFunction(sfSetGameVar);
	RegisterScriptFunction(sfUpdateScreen);
	RegisterScriptFunction(sfGetRandomNumber);
	RegisterScriptFunction(sfDrawGuiTextMulti);
	RegisterScriptFunction(sfUpdateVerbLine);
	RegisterScriptFunction(sfSetFontColor);
	RegisterScriptFunction(sfGetTalkTextDuration);
	RegisterScriptFunction(sfTalk);
	RegisterScriptFunction(sfFindPaletteFragment);
	RegisterScriptFunction(sfClearPaletteFragments);
	RegisterScriptFunction(sfAddPaletteFragment);
	RegisterScriptFunction(sfSetDeltaAnimPalette);
	RegisterScriptFunction(sfNop); // TODO
	RegisterScriptFunction(sfBuildColorTransTable);
	RegisterScriptFunction(sfSetDeltaMainPalette);
	RegisterScriptFunction(sfLoadScript);
	RegisterScriptFunction(sfRegisterFont);
	RegisterScriptFunction(sfLoadAddPalette);
	RegisterScriptFunction(sfLoadScene);
	RegisterScriptFunction(sfSetGuiHeight);
	RegisterScriptFunction(sfFindMouseInRectIndex1);
	RegisterScriptFunction(sfFindMouseInRectIndex2);
	RegisterScriptFunction(sfDrawGuiImage);
	RegisterScriptFunction(sfAddAnimatedSpriteNoLoop);
	RegisterScriptFunction(sfAddAnimatedSprite);
	RegisterScriptFunction(sfAddStaticSprite);
	RegisterScriptFunction(sfAddAnimatedSpriteScaled);
	RegisterScriptFunction(sfFindPath);
	RegisterScriptFunction(sfWalk);
	RegisterScriptFunction(sfScrollCameraUp);
	RegisterScriptFunction(sfScrollCameraDown);
	RegisterScriptFunction(sfScrollCameraLeft);
	RegisterScriptFunction(sfScrollCameraRight);
	RegisterScriptFunction(sfScrollCameraUpEx);
	RegisterScriptFunction(sfScrollCameraDownEx);
	RegisterScriptFunction(sfScrollCameraLeftEx);
	RegisterScriptFunction(sfScrollCameraRightEx);
	RegisterScriptFunction(sfSetCamera);
	RegisterScriptFunction(sfNop); // TODO
	RegisterScriptFunction(sfGetRgbModifiertAtPoint);
	RegisterScriptFunction(sfStartAnim);
	RegisterScriptFunction(sfAnimNextFrame);
	RegisterScriptFunction(sfNop);
	RegisterScriptFunction(sfGetAnimFrameNumber);
	RegisterScriptFunction(sfGetAnimStatus);
	RegisterScriptFunction(sfStartShakeScreen);
	RegisterScriptFunction(sfStopShakeScreen);
	RegisterScriptFunction(sfStartSequence);
	RegisterScriptFunction(sfEndSequence);
	RegisterScriptFunction(sfSetSequenceVolume);
	RegisterScriptFunction(sfPlayPositionalSound);
	RegisterScriptFunction(sfPlaySound2);
	RegisterScriptFunction(sfClearScreen);
	RegisterScriptFunction(sfNop);
	RegisterScriptFunction(sfHandleInput);
	RegisterScriptFunction(sfRunOptionsScreen);
	RegisterScriptFunction(sfPrecacheSprites);
	RegisterScriptFunction(sfPrecacheSounds1);
	RegisterScriptFunction(sfDeleteAllPbfFilesByExternalArray);
	RegisterScriptFunction(sfPrecacheSounds2);
	RegisterScriptFunction(sfRestoreStackPtr);
	RegisterScriptFunction(sfSaveStackPtr);
	RegisterScriptFunction(sfPlayMovie);
	RegisterScriptFunction(sfNop);

}

void ScriptInterpreter::loadScript(uint resIndex, uint slotIndex) {

	delete[] _slots[slotIndex].data;

 	_slots[slotIndex].resIndex = resIndex;
	Resource *scriptResource = _vm->_res->load(resIndex);
	_slots[slotIndex].size = scriptResource->size;
 	_slots[slotIndex].data = new byte[_slots[slotIndex].size];
 	memcpy(_slots[slotIndex].data, scriptResource->data, _slots[slotIndex].size);

}

void ScriptInterpreter::runScript(uint slotIndex) {

	_switchLocalDataNear = true;
	_switchLocalDataFar = false;
	_switchLocalDataToStack = false;
	_cmpBitTest = false;

	_regs.reg0 = 0;
	_regs.reg1 = 0;
	_regs.reg2 = 0;
	_regs.reg3 = 0;
	_regs.reg4 = slotIndex;
	_regs.reg5 = 0;
	_regs.reg6 = 0;
	_regs.sp = 4096;
	_regs.reg8 = 0;

	_code = getSlotData(_regs.reg4);

	while (!_vm->shouldQuit()) {

		if (_vm->_movieSceneFlag)
			_vm->_mouseButton = 0;
			
		if (_switchLocalDataNear) {
			_switchLocalDataNear = false;
			_localData = getSlotData(_regs.reg4);
		}

		if (_switchLocalDataFar) {
			_switchLocalDataFar = false;
			_localData = getSlotData(_regs.reg5);
			_switchLocalDataNear = true;
		}

		if (_switchLocalDataToStack) {
			_switchLocalDataToStack = false;
			_localData = _stack + 2;
			_switchLocalDataNear = true;
		}
		
		byte opcode = readByte();
		execOpcode(opcode);

		// Call updateScreen roughly every 10ms else the mouse cursor will be jerky
		if (_vm->_system->getMillis() % 10 == 0)
			_vm->_system->updateScreen();

	}

}

byte ScriptInterpreter::readByte() {
	return *_code++;
}

int16 ScriptInterpreter::readInt16() {
	int16 value = READ_LE_UINT16(_code);
	_code += 2;
	return value;
}

void ScriptInterpreter::execOpcode(byte opcode) {

	int16 ofs;

	debug(1, "opcode = %d", opcode);

	switch (opcode) {
	case 0:
	{
		// ok
		_subCode = _code;
		byte length = readByte();
		debug(1, "length = %d", length);
		uint16 index = readInt16();
		debug(1, "callScriptFunction %d", index);
		execScriptFunction(index);
		_code += length - 2;
		break;
	}
	case 1:
		// ok
		_regs.reg0 = readInt16();
		break;
	case 2:
		// ok
		_regs.reg1 = readInt16();
		break;
	case 3:
		// ok
		_regs.reg3 = readInt16();
		break;
	case 4:
		// ok
		_regs.reg5 = _regs.reg0;
		break;
	case 5:
		// ok
		_regs.reg3 = _regs.reg0;
		break;
	case 6:
		// ok
		_regs.reg1 = _regs.reg0;
		break;
	case 7:
		_regs.reg1 = localRead16(_regs.reg3);
		break;
	case 8:
		localWrite16(_regs.reg3, _regs.reg0);
		break;
	case 9:
		localWrite16(readInt16(), _regs.reg0);
		break;
	case 10:
		localWrite8(readInt16(), _regs.reg0);
		break;
	case 11:
		localWrite16(readInt16(), _regs.reg5);
		break;
	case 12:
		localWrite16(readInt16(), _regs.reg4);
		break;
	case 13:
		localWrite16(readInt16(), _regs.reg3);
		break;
	case 14:
		_regs.reg3 = localRead16(readInt16());
		break;
	case 15:
		_regs.reg2 = localRead16(_regs.reg1);
		break;
	case 16:
		_regs.reg2 = localRead16(_regs.reg1 + readInt16());
		break;
	case 17:
		_regs.reg2 = _regs.reg0;
		break;
	case 18:
		_regs.reg0 += readInt16();
		break;
	case 19:
		localWrite16(_regs.reg3, localRead16(_regs.reg3) + _regs.reg0);
		break;
	case 20:
		_regs.reg0 += _regs.reg2;
		break;
	case 21:
		_regs.reg3 += _regs.sp;
		break;
	case 22:
		_regs.reg1 += _regs.sp;
		break;
	case 23:
		localWrite16(_regs.reg3, localRead16(_regs.reg3) - _regs.reg0);
		break;
	case 24:
		_regs.reg0 /= readInt16();
		break;
	case 25:
		localWrite16(_regs.reg3, localRead16(_regs.reg3) / _regs.reg0);
		break;
	case 26:
		// NOP
		break;
	case 27:
		_regs.reg0 *= readInt16();
		break;
	case 28:
		localWrite16(_regs.reg3, localRead16(_regs.reg3) * _regs.reg0);
		break;
	case 29:
		_regs.reg0 *= _regs.reg2;
		break;
	case 30:
		localWrite16(_regs.reg3, localRead16(_regs.reg3) + 1);
		break;
	case 31:
		localWrite16(_regs.reg3, localRead16(_regs.reg3) - 1);
		break;
	case 32:
		_switchLocalDataFar = true;
		break;
	case 33:
		_switchLocalDataToStack = true;
		break;
	case 34:
		pushInt16(_regs.reg0);
		break;
	case 35:
		pushInt16(_regs.reg1);
		break;
	case 36:
		_regs.reg1 = popInt16();
		break;
	case 37:
		_regs.reg0 = popInt16();
		break;
	case 38:
		_regs.reg2 = -_regs.reg2;
		break;
	case 39:
		_regs.reg8 = readInt16();
		_cmpBitTest = false;
		break;
	case 40:
		_regs.reg8 = _regs.reg0;
		_cmpBitTest = false;
		break;
	case 41:
		_regs.reg8 = readInt16();
		_cmpBitTest = true;
		break;
	case 42:
		_regs.reg8 = _regs.reg0;
		_cmpBitTest = true;
		break;
	case 43:
		_code = getSlotData(_regs.reg4) + _regs.reg0;
		break;
	case 44:
		_code = getSlotData(_regs.reg5) + _regs.reg0;
		_regs.reg4 = _regs.reg5;
		_switchLocalDataNear = true;
		break;
	case 45:
		pushInt16(_code - getSlotData(_regs.reg4));
		pushInt16(_regs.reg4);
		_code = getSlotData(_regs.reg4) + _regs.reg0;
		break;
	case 46:
		pushInt16(_code - getSlotData(_regs.reg4));
		pushInt16(_regs.reg4);
		_code = getSlotData(_regs.reg5) + _regs.reg0;
		_regs.reg4 = _regs.reg5;
		_switchLocalDataNear = true;
		break;
	case 47:
		_regs.reg4 = popInt16();
		ofs = popInt16();
		_code = getSlotData(_regs.reg4) + ofs;
		_switchLocalDataNear = true;
		break;
	case 48:
		_regs.reg4 = popInt16();
		ofs = popInt16();
		_code = getSlotData(_regs.reg4) + ofs;
		_regs.sp += _regs.reg0;
		_switchLocalDataNear = true;
		break;
	case 49:
		ofs = readByte();
		_code += ofs;
		break;
	case 50:
		if (_cmpBitTest) {
			_regs.reg1 &= _regs.reg8;
			if (_regs.reg1 == 0)
				_code += 4;
		} else {
			if (_regs.reg1 == _regs.reg8)
				_code += 4;
		}
		_code++;
		break;
	case 51:
		if (_cmpBitTest) {
			_regs.reg1 &= _regs.reg8;
			if (_regs.reg1 != 0)
				_code += 4;
		} else {
			if (_regs.reg1 != _regs.reg8)
				_code += 4;
		}
		_code++;
		break;
	case 52:
		if ((uint16)_regs.reg1 >= (uint16)_regs.reg8)
			_code += 4;
		_code++;
		break;
	case 53:
		if ((uint16)_regs.reg1 <= (uint16)_regs.reg8)
			_code += 4;
		_code++;
		break;
	case 54:
		if ((uint16)_regs.reg1 < (uint16)_regs.reg8)
			_code += 4;
		_code++;
		break;
	case 55:
		if ((uint16)_regs.reg1 > (uint16)_regs.reg8)
			_code += 4;
		_code++;
		break;
	default:
		error("Invalid opcode %d", opcode);
	}

}

void ScriptInterpreter::execScriptFunction(uint16 index) {
	debug(4, "execScriptFunction(%d)", index);
	if (index >= _scriptFuncs.size())
		error("ScriptInterpreter::execScriptFunction() Invalid script function index %d", index);
	debug(4, "%s", _scriptFuncNames[index]);
	(*_scriptFuncs[index])();
}

VarType ScriptInterpreter::getGameVarType(uint variable) {
	switch (variable) {
		case 0:	 return vtByte;
		case 1:	 return vtWord;
		case 2:	 return vtWord;
		case 3:	 return vtByte;
		case 4:	 return vtWord;
		case 5:	 return vtWord;
		case 6:	 return vtWord;
		case 7:	 return vtWord;
		case 8:	 return vtWord;
		case 9:	 return vtWord;
		case 10:	 return vtWord;
		case 11:	 return vtWord;
		case 12:	 return vtByte;
		case 13:	 return vtWord;
		case 14:	 return vtWord;
		case 15:	 return vtWord;
		case 16:	 return vtWord;
		case 17:	 return vtWord;
		case 18:	 return vtWord;
		case 19:	 return vtWord;
		case 20:	 return vtWord;
		case 21:	 return vtWord;
		default:
			error("Invalid game variable");
	}
}

const char *getVarName(uint variable) {
	switch (variable) {
		case 0: return "mouseDisabled";
		case 1: return "mouseY";
		case 2: return "mouseX";
		case 3: return "mouseButton";
		case 4: return "verbLineY";
		case 5: return "verbLineX";
		case 6: return "verbLineWidth";
		case 7: return "verbLineCount";
		case 8: return "verbLineNum";
		case 9: return "talkTextItemNum";
		case 10: return "talkTextY";
		case 11: return "talkTextX";
		case 12: return "talkTextFontColor";
		case 13: return "cameraY";
		case 14: return "cameraX";
		case 15: return "walkSpeedY";
		case 16: return "walkSpeedX";
		case 17: return "flag01";
		case 18: return "sceneResIndex";
		case 19: return "guiHeight";
		case 20: return "sceneHeight";
		case 21: return "sceneWidth";
	}
	return "(invalid)";
}

int16 ScriptInterpreter::getGameVar(uint variable) {
	debug(0, "ScriptInterpreter::getGameVar(%d{%s})", variable, getVarName(variable));
	
	int16 value = 0;
	
	switch (variable) {
		case 0:
			value = _vm->_mouseDisabled;
			break;
		case 1:
			value = _vm->_mouseY;
			break;
		case 2:
			value = _vm->_mouseX;
			break;
		case 3:
			value = _vm->_mouseButton;
			break;
		case 4:
			value = _vm->_screen->_verbLineY;
			break;
		case 5:
			value = _vm->_screen->_verbLineX;
			break;
		case 6:
			value = _vm->_screen->_verbLineWidth;
			break;
		case 7:
			value = _vm->_screen->_verbLineCount;
			break;
		case 8:
			value = _vm->_screen->_verbLineNum;
			break;
		case 9:
			value = _vm->_screen->_talkTextItemNum;
			break;
		case 10:
			value = _vm->_screen->_talkTextY;
			break;
		case 11:
			value = _vm->_screen->_talkTextX;
			break;
		case 12:
			value = _vm->_screen->_talkTextFontColor;
			break;
		case 13:
			value = _vm->_cameraY;
			break;
		case 14:
			value = _vm->_cameraX;
			break;
		case 15:
			value = _vm->_walkSpeedY;
			break;
		case 16:
			value = _vm->_walkSpeedX;
			break;
		case 17:
			value = _vm->_flag01;
			break;
		case 18:
			value = _vm->_sceneResIndex;
			break;
		case 19:
			value = _vm->_guiHeight;
			break;
		case 20:
			value = _vm->_sceneHeight;
			break;
		case 21:
			value = _vm->_sceneWidth;
			break;
		default:
			warning("Getting unimplemented game variable %s (%d)", getVarName(variable), variable);
			break;
	}

	
	return value;

}

void ScriptInterpreter::setGameVar(uint variable, int16 value) {
	debug(0, "ScriptInterpreter::setGameVar(%d{%s}, %d)", variable, getVarName(variable), value);
	
	switch (variable) {
		case 0:
			_vm->_mouseDisabled = value;
			_vm->_system->showMouse(value == 0);
			break;
		case 3:
			_vm->_mouseButton = value;
			break;
		case 4:
			_vm->_screen->_verbLineY = value;
			break;
		case 5:
			_vm->_screen->_verbLineX = value;
			break;
		case 6:
			_vm->_screen->_verbLineWidth = value;
			break;
		case 7:
			_vm->_screen->_verbLineCount = value;
			break;
		case 8:
			_vm->_screen->_verbLineNum = value;
			break;
		case 9:
			_vm->_screen->_talkTextItemNum = value;
			break;
		case 10:
			_vm->_screen->_talkTextY = value;
			break;
		case 11:
			_vm->_screen->_talkTextX = value;
			break;
		case 12:
			_vm->_screen->_talkTextFontColor = value;
			break;
		case 13:
			_vm->_cameraY = value;
			break;
		case 14:
			_vm->_cameraX = value;
			break;
		case 15:
			_vm->_walkSpeedY = value;
			break;
		case 16:
			_vm->_walkSpeedX = value;
			break;
		case 17:
			_vm->_flag01 = value != 0;
			break;
		case 18:
			_vm->_sceneResIndex = value;
			break;
		case 19:
			_vm->_guiHeight = value;
			break;
		case 20:
			_vm->_sceneHeight = value;
			break;
		case 21:
			_vm->_sceneWidth = value;
			break;
		case 1:
		case 2:
		default:
			warning("Setting unimplemented game variable %s (%d) to %d", getVarName(variable), variable, value);
			break;
	}


}

byte ScriptInterpreter::arg8(int16 offset) {
	return _subCode[offset];
}

int16 ScriptInterpreter::arg16(int16 offset) {
	return READ_LE_UINT16(&_subCode[offset]);
}

void ScriptInterpreter::pushInt16(int16 value) {
	WRITE_LE_UINT16(_stack + _regs.sp, value);
	_regs.sp -= 2;
}

int16 ScriptInterpreter::popInt16() {
	_regs.sp += 2;
	return READ_LE_UINT16(_stack + _regs.sp);
}

void ScriptInterpreter::localWrite8(int16 offset, byte value) {
	debug(1, "localWrite8(%d, %d)", offset, value);
	_localData[offset] = value;
}

byte ScriptInterpreter::localRead8(int16 offset) {
	debug(1, "localRead8(%d) -> %d", offset, _localData[offset]);
	return _localData[offset];
}

void ScriptInterpreter::localWrite16(int16 offset, int16 value) {
	debug(1, "localWrite16(%d, %d)", offset, value);
	WRITE_LE_UINT16(&_localData[offset], value);
}

int16 ScriptInterpreter::localRead16(int16 offset) {
	debug(1, "localRead16(%d) -> %d", offset, (int16)READ_LE_UINT16(&_localData[offset]));
	return (int16)READ_LE_UINT16(&_localData[offset]);
}

byte *ScriptInterpreter::localPtr(int16 offset) {
	debug(1, "localPtr(%d)", offset);
	return &_localData[offset];
}

void ScriptInterpreter::saveState(Common::WriteStream *out) {

	// Save registers
	out->writeUint16LE(_regs.reg0);
	out->writeUint16LE(_regs.reg1);
	out->writeUint16LE(_regs.reg2);
	out->writeUint16LE(_regs.reg3);
	out->writeUint16LE(_regs.reg4);
	out->writeUint16LE(_regs.reg5);
	out->writeUint16LE(_regs.reg6);
	out->writeUint16LE(_regs.sp);
	out->writeUint16LE(_regs.reg8);

	// Save slots
	for (int slot = 0; slot < kMaxScriptSlots; slot++) {
		out->writeUint32LE(_slots[slot].size);
		out->writeUint16LE(_slots[slot].resIndex);
		if (_slots[slot].size > 0)
			out->write(_slots[slot].data, _slots[slot].size);
	}

	// Save stack
	out->write(_stack, kScriptStackSize);
	out->writeUint16LE(_savedSp);
	
	// Save IP
	out->writeUint16LE((int16)(_code - getSlotData(_regs.reg4)));

}

void ScriptInterpreter::loadState(Common::ReadStream *in) {

	// Load registers
	_regs.reg0 = in->readUint16LE();
	_regs.reg1 = in->readUint16LE();
	_regs.reg2 = in->readUint16LE();
	_regs.reg3 = in->readUint16LE();
	_regs.reg4 = in->readUint16LE();
	_regs.reg5 = in->readUint16LE();
	_regs.reg6 = in->readUint16LE();
	_regs.sp = in->readUint16LE();
	_regs.reg8 = in->readUint16LE();

	// Load slots
	for (int slot = 0; slot < kMaxScriptSlots; slot++) {
		_slots[slot].size = in->readUint32LE();
		_slots[slot].resIndex = in->readUint16LE();
		_slots[slot].data = NULL;
		if (_slots[slot].size > 0) {
			_slots[slot].data = new byte[_slots[slot].size];
			in->read(_slots[slot].data, _slots[slot].size);
		}
	}

	// Load stack
	in->read(_stack, kScriptStackSize);
	_savedSp = in->readUint16LE();

	// Load IP
	_code = getSlotData(_regs.reg4) + in->readUint16LE();

}

void ScriptInterpreter::sfNop() {
	// NOP
}

void ScriptInterpreter::sfGetGameVar() {
	int16 value = getGameVar(arg16(3));
	localWrite16(arg16(5), value);
}

void ScriptInterpreter::sfSetGameVar() {
	int16 varIndex = arg16(3);
	VarType varType = getGameVarType(varIndex);
	int16 value = 0;
	if (varType == vtByte)
		value = arg8(5);
	else if (varType == vtWord)
		value = arg16(5);
	setGameVar(varIndex, value);
}

void ScriptInterpreter::sfUpdateScreen() {

	_vm->_sound->updateSpeech();

	_vm->_screen->updateShakeScreen();

	// TODO: Set quit flag
	if (_vm->shouldQuit())
		return;

	if (!_vm->_movieSceneFlag)
		_vm->updateInput();
	else
		_vm->_mouseButton = 0;

	// TODO? Check keyb

	_vm->_counter01--;
	if (_vm->_counter01 <= 0) {
		_vm->_counter01 = MIN(_vm->_counter02, 30);
		_vm->_counter02 = 0;
		_vm->updateScreen();
		_vm->_flag01 = 1;
		_vm->_system->delayMillis(5);
		_vm->_counter02 = 1; // ?
	} else {
		_vm->_screen->clearSprites();
		_vm->_flag01 = 0;
		//_vm->_system->updateScreen();
	}

	// TODO

}

void ScriptInterpreter::sfGetRandomNumber() {
	localWrite16(arg16(5), _vm->_rnd->getRandomNumber(arg16(3) - 1));
}

void ScriptInterpreter::sfDrawGuiTextMulti() {
	_vm->_screen->drawGuiTextMulti((byte*)localPtr(arg16(3)));
}

void ScriptInterpreter::sfUpdateVerbLine() {
	_vm->_screen->updateVerbLine(arg16(5), arg16(3));
}

void ScriptInterpreter::sfSetFontColor() {
	_vm->_screen->_fontColor1 = 0;
	_vm->_screen->_fontColor2 = arg8(3);
}

void ScriptInterpreter::sfGetTalkTextDuration() {
	localWrite16(arg16(3), _vm->_screen->getTalkTextDuration());
}

void ScriptInterpreter::sfTalk() {
	_vm->talk(arg16(5), arg16(3));
}

void ScriptInterpreter::sfFindPaletteFragment() {
	localWrite16(arg16(5), _vm->_palette->findFragment(arg16(3)));
}

void ScriptInterpreter::sfClearPaletteFragments() {
	_vm->_palette->clearFragments();
}

void ScriptInterpreter::sfAddPaletteFragment() {
	_vm->_palette->addFragment(arg16(3), arg16(5));
}

void ScriptInterpreter::sfSetDeltaAnimPalette() {
	_vm->_palette->setDeltaPalette(_vm->_palette->getAnimPalette(), arg8(6), (char)arg8(5), arg8(4), arg8(3));
}

void ScriptInterpreter::sfBuildColorTransTable() {
	_vm->_palette->buildColorTransTable(arg8(4), (char)arg8(3), arg8(5));
}

void ScriptInterpreter::sfSetDeltaMainPalette() {
	_vm->_palette->setDeltaPalette(_vm->_palette->getMainPalette(), arg8(6), (char)arg8(5), arg8(4), arg8(3));
}

void ScriptInterpreter::sfLoadScript() {
	int16 codeOfs = _code - getSlotData(_regs.reg4);
	loadScript(arg16(4), arg8(3));
	_code = getSlotData(_regs.reg4) + codeOfs;
	_switchLocalDataNear = true;
}

void ScriptInterpreter::sfRegisterFont() {
	_vm->_screen->registerFont(arg8(3), arg16(4));
}

void ScriptInterpreter::sfLoadAddPalette() {
	_vm->_palette->loadAddPalette(arg16(4), arg8(3));
}

void ScriptInterpreter::sfLoadScene() {
	if (arg8(3) == 0) {
		_vm->_sound->stopSpeech();
		_vm->loadScene(arg16(4));
	} else {
		_vm->_screen->loadMouseCursor(arg16(4));
	}
}

void ScriptInterpreter::sfSetGuiHeight() {
	_vm->setGuiHeight(arg8(3));
}

void ScriptInterpreter::sfFindMouseInRectIndex1() {
	int16 index = -1;
	if (_vm->_mouseY < _vm->_cameraHeight) {
		index = _vm->findRectAtPoint(getSlotData(arg16(5)) + arg16(3),
			_vm->_mouseX + _vm->_cameraX,
			_vm->_mouseY + _vm->_cameraY,
			arg16(11) + 1, arg16(7));
	}
	localWrite16(arg16(9), index);
}

void ScriptInterpreter::sfFindMouseInRectIndex2() {
	int16 index = -1;
	if (_vm->_sceneResIndex != 0) {
		if (_vm->_mouseY < _vm->_cameraHeight) {
			index = _vm->findRectAtPoint(getSlotData(arg16(5)) + arg16(3),
				_vm->_mouseX + _vm->_cameraX,
				_vm->_mouseY + _vm->_cameraY,
				0, arg16(7));
		}
	}
	localWrite16(arg16(9), index);
}

void ScriptInterpreter::sfDrawGuiImage() {
  	_vm->_screen->drawGuiImage(arg16(5), arg16(3), arg16(7));
}

void ScriptInterpreter::sfAddAnimatedSpriteNoLoop() {
	_vm->_screen->addAnimatedSprite(arg16(5), arg16(3), arg16(7), (byte*)localPtr(0), (int16*)localPtr(arg16(9)), false, 2);
}

void ScriptInterpreter::sfAddAnimatedSprite() {
	_vm->_screen->addAnimatedSprite(arg16(5), arg16(3), arg16(7), (byte*)localPtr(0), (int16*)localPtr(arg16(9)), true, 2);
}

void ScriptInterpreter::sfAddStaticSprite() {
	_vm->_screen->addStaticSprite(_subCode + 3);
}

void ScriptInterpreter::sfAddAnimatedSpriteScaled() {
	_vm->_screen->addAnimatedSprite(arg16(5), arg16(3), arg16(7), (byte*)localPtr(0), (int16*)localPtr(arg16(9)), true, 1);
}

void ScriptInterpreter::sfFindPath() {
	_vm->_segmap->findPath((int16*)(getSlotData(arg16(13)) + arg16(11)), arg16(9), arg16(7), arg16(5), arg16(3));
}

void ScriptInterpreter::sfWalk() {
	_vm->walk(getSlotData(arg16(5)) + arg16(3));
}

void ScriptInterpreter::sfScrollCameraUp() {
	_vm->scrollCameraUp(4);
}

void ScriptInterpreter::sfScrollCameraDown() {
	_vm->scrollCameraDown(4);
}

void ScriptInterpreter::sfScrollCameraLeft() {
	_vm->scrollCameraLeft(4);
}

void ScriptInterpreter::sfScrollCameraRight() {
	_vm->scrollCameraRight(4);
}

void ScriptInterpreter::sfScrollCameraUpEx() {
	_vm->scrollCameraUp(arg16(3));
}

void ScriptInterpreter::sfScrollCameraDownEx() {
	_vm->scrollCameraDown(arg16(3));
}

void ScriptInterpreter::sfScrollCameraLeftEx() {
	_vm->scrollCameraLeft(arg16(3));
}

void ScriptInterpreter::sfScrollCameraRightEx() {
	_vm->scrollCameraRight(arg16(3));
}

void ScriptInterpreter::sfSetCamera() {
	_vm->setCamera(arg16(5), arg16(3));
}

void ScriptInterpreter::sfGetRgbModifiertAtPoint() {
	byte *rgb = getSlotData(arg16(11)) + arg16(9);
	_vm->_segmap->getRgbModifiertAtPoint(arg16(5), arg16(3), arg16(7), rgb[0], rgb[1], rgb[2]);
}

void ScriptInterpreter::sfStartAnim() {
	_vm->_anim->start(arg16(3));
}

void ScriptInterpreter::sfAnimNextFrame() {
	_vm->_anim->nextFrame();
}

void ScriptInterpreter::sfGetAnimFrameNumber() {
	localWrite16(arg16(3), _vm->_anim->getFrameNumber());
}

void ScriptInterpreter::sfGetAnimStatus() {
	int16 status = _vm->_anim->getStatus();
	if (status == 0 || status == 1) {
		// TODO mov screenFlag01, 0
	}
	localWrite16(arg16(3), status);
}

void ScriptInterpreter::sfStartShakeScreen() {
	_vm->_screen->startShakeScreen(arg16(3));
}

void ScriptInterpreter::sfStopShakeScreen() {
	_vm->_screen->stopShakeScreen();
}

void ScriptInterpreter::sfStartSequence() {
	// TODO
	//_vm->_arc->dump(arg16(3));
}

void ScriptInterpreter::sfEndSequence() {
	// TODO
}

void ScriptInterpreter::sfSetSequenceVolume() {
	// TODO
}

void ScriptInterpreter::sfPlayPositionalSound() {
	// TODO
	debug("ScriptInterpreter::sfPlayPositionalSound()");
}

void ScriptInterpreter::sfPlaySound2() {
	_vm->_sound->playSound(arg16(3), arg16(5), arg16(7));
}

void ScriptInterpreter::sfClearScreen() {
	// TODO
}

void ScriptInterpreter::sfHandleInput() {
	// TODO: Recheck what this does, I don't remember
	int16 varOfs = arg16(3);
	localWrite16(varOfs, 0);
}

void ScriptInterpreter::sfRunOptionsScreen() {
	// TODO
}

void ScriptInterpreter::sfPrecacheSprites() {
#if 0
	// DEBUG
	int16 *resArray = (int16*)localPtr(arg16(3));
	while (*resArray != -1) {
		debug("ScriptInterpreter::sfPrecacheSprites() index = %d", *resArray);
		_vm->_arc->dump(*resArray, "pc1");
		resArray++;
	}
#endif
}

void ScriptInterpreter::sfPrecacheSounds1() {
#if 0
	// DEBUG
	int16 *resArray = (int16*)localPtr(arg16(3));
	while (*resArray != -1) {
		debug("ScriptInterpreter::sfPrecacheSounds1() index = %d", *resArray);
		_vm->_arc->dump(*resArray, "pc2");
		resArray++;
	}
#endif
}

void ScriptInterpreter::sfDeleteAllPbfFilesByExternalArray() {
}

void ScriptInterpreter::sfPrecacheSounds2() {
#if 0
	// DEBUG
	int16 *resArray = (int16*)localPtr(arg16(3));
	while (*resArray != -1) {
		debug("ScriptInterpreter::sfPrecacheSounds2() index = %d", *resArray);
		_vm->_arc->dump(*resArray, "pc3");
		resArray++;
	}
#endif
}

void ScriptInterpreter::sfRestoreStackPtr() {
	_regs.sp = _savedSp;
}

void ScriptInterpreter::sfSaveStackPtr() {
	_savedSp = _regs.sp;
}

void ScriptInterpreter::sfPlayMovie() {
	_vm->_moviePlayer->playMovie(arg16(3));
}

} // End of namespace Picture
