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

#include "prince/script.h"
#include "prince/prince.h"
#include "prince/flags.h"
#include "prince/variatxt.h"
#include "prince/font.h"
#include "prince/hero.h"
#include "prince/resource.h"
#include "prince/animation.h"

#include "common/debug.h"
#include "common/debug-channels.h"
#include "common/stream.h"
#include "common/archive.h"
#include "common/memstream.h"

namespace Prince {

static const uint16 kNumOpcodes = 144;

Room::Room() {}

bool Room::loadRoom(byte *roomData) {
	int roomSize = 64;
	Common::MemoryReadStream roomStream(roomData, roomSize);
	
	_mobs = roomStream.readSint32LE();
	_backAnim = roomStream.readSint32LE();
	_obj = roomStream.readSint32LE();
	_nak = roomStream.readSint32LE();
	_itemUse = roomStream.readSint32LE();
	_itemGive = roomStream.readSint32LE();
	_walkTo = roomStream.readSint32LE();
	_examine = roomStream.readSint32LE();
	_pickup = roomStream.readSint32LE();
	_use = roomStream.readSint32LE();
	_pushOpen = roomStream.readSint32LE();
	_pullClose = roomStream.readSint32LE();
	_talk = roomStream.readSint32LE();
	_give = roomStream.readSint32LE();

	return true;
}

int Room::getOptionOffset(int option) {
	switch (option) {
	case 0:
		return _walkTo;
	case 1:
		return _examine;
	case 2:
		return _pickup;
	case 3:
		return _use;
	case 4:
		return _pushOpen;
	case 5:
		return _pullClose;
	case 6:
		return _talk;
	case 7:
		return _give;
	default:
		error("Wrong option - nr %d", option);
	}
}

Script::Script(PrinceEngine *vm) : _vm(vm), _data(nullptr), _dataSize(0) {
}

Script::~Script() {
	delete[] _data;
	_dataSize = 0;
	_data = nullptr;
}

bool Script::loadFromStream(Common::SeekableReadStream &stream) {
	_dataSize = stream.size();
	if (!_dataSize) 
		return false;

	_data = new byte[_dataSize];

	if (!_data)
		return false;

	stream.read(_data, _dataSize);

	Common::MemoryReadStream scriptDataStream(_data, _dataSize);
	_scriptInfo.rooms = scriptDataStream.readSint32LE();
	_scriptInfo.startGame = scriptDataStream.readSint32LE();
	_scriptInfo.restoreGame = scriptDataStream.readSint32LE();
	_scriptInfo.stdExamine = scriptDataStream.readSint32LE();
	_scriptInfo.stdPickup = scriptDataStream.readSint32LE();
	_scriptInfo.stdUse = scriptDataStream.readSint32LE();
	_scriptInfo.stdOpen = scriptDataStream.readSint32LE();
	_scriptInfo.stdClose = scriptDataStream.readSint32LE();
	_scriptInfo.stdTalk = scriptDataStream.readSint32LE();
	_scriptInfo.stdGive = scriptDataStream.readSint32LE();
	_scriptInfo.usdCode = scriptDataStream.readSint32LE();
	_scriptInfo.invObjExam = scriptDataStream.readSint32LE();
	_scriptInfo.invObjUse = scriptDataStream.readSint32LE();
	_scriptInfo.invObjUU = scriptDataStream.readSint32LE();
	_scriptInfo.stdUseItem = scriptDataStream.readSint32LE();
	_scriptInfo.lightSources = scriptDataStream.readSint32LE();
	_scriptInfo.specRout = scriptDataStream.readSint32LE();
	_scriptInfo.invObjGive = scriptDataStream.readSint32LE();
	_scriptInfo.stdGiveItem = scriptDataStream.readSint32LE();
	_scriptInfo.goTester = scriptDataStream.readSint32LE();
	
	return true;
}

int16 Script::getLightX(int locationNr) {
	return (int)READ_UINT16(&_data[_scriptInfo.lightSources + locationNr * 8]);
}

int16 Script::getLightY(int locationNr) {
	return (int)READ_UINT16(&_data[_scriptInfo.lightSources + locationNr * 8 + 2]);
}

int32 Script::getShadowScale(int locationNr) {
	return (int)READ_UINT16(&_data[_scriptInfo.lightSources + locationNr * 8 + 4]);
}

uint32 Script::getStartGameOffset() {
	return _scriptInfo.startGame;
}

bool Script::getMobVisible(int mob) {
	return _data[_vm->_room->_mobs + mob];
}

void Script::setMobVisible(int mob, int value) {
	_data[_vm->_room->_mobs + mob] = value;
}

uint8 *Script::getRoomOffset(int locationNr) {
	return &_data[_scriptInfo.rooms + locationNr * 64];
}

int32 Script::getOptionStandardOffset(int option) {
	switch (option) {
	case 1:
		return _scriptInfo.stdExamine;
	case 2:
		return _scriptInfo.stdPickup;
	case 3:
		return _scriptInfo.stdUse;
	case 4:
		return _scriptInfo.stdOpen;
	case 5:
		return _scriptInfo.stdClose;
	case 6:
		return _scriptInfo.stdTalk;
	case 7:
		return _scriptInfo.stdGive;
	default:
		error("Wrong standard option - nr %d", option);
	}
}

uint8 *Script::getHeroAnimName(int offset) {
	return &_data[offset];
}

void Script::setBackAnimId(int offset, int animId) {
	WRITE_UINT32(&_data[offset], animId);
}

void Script::setObjId(int offset, int objId) {
	_data[offset] = objId;
}

int Script::scanMobEvents(int mobMask, int dataEventOffset) {
	debug("mobMask: %d", mobMask);
	int i = 0;
	int16 mob;
	int32 code;
	do {
		mob = (int)READ_UINT16(&_data[dataEventOffset + i * 6]);
		if (mob == mobMask) {
			code = (int)READ_UINT32(&_data[dataEventOffset + i * 6 + 2]);
			debug("code: %d", code);
			return code;
		}
		i++;
	} while (mob != -1);
	return -1;
}

int Script::scanMobEventsWithItem(int mobMask, int dataEventOffset, int itemMask) {
	debug("mobMask: %d", mobMask);
	int i = 0;
	int16 mob;
	int16 item;
	int32 code;
	do {
		mob = (int)READ_UINT16(&_data[dataEventOffset + i * 8]);
		if (mob == mobMask) {
			item = (int)READ_UINT16(&_data[dataEventOffset + i * 8 + 2]);
			if (item == itemMask) {
				code = (int)READ_UINT32(&_data[dataEventOffset + i * 8 + 4]);
				debug("itemMask: %d", item);
				debug("code: %d", code);
				return code;
			}
		}
		i++;
	} while (mob != -1);
	return -1;
}

void Script::installSingleBackAnim(Common::Array<BackgroundAnim> &backAnimList, int slot, int offset) {

	BackgroundAnim newBackgroundAnim;

	int animOffset = READ_UINT32(&_data[offset]);
	int anims = READ_UINT32(&_data[animOffset + 8]);

	if (anims == 0) {
		anims = 1;
	}

	if (animOffset != 0) {
		for (int i = 0; i < anims; i++) {
			Anim newAnim;
			newAnim._basaData._num = READ_UINT16(&_data[animOffset + 28 + i * 8]);
			newAnim._basaData._start = READ_UINT16(&_data[animOffset + 28 + i * 8 + 2]);
			newAnim._basaData._end = READ_UINT16(&_data[animOffset + 28 + i * 8 + 4]);
			int animNumber = newAnim._basaData._num;
			const Common::String animName = Common::String::format("AN%02d", animNumber);
			const Common::String shadowName = Common::String::format("AN%02dS", animNumber);
			newAnim._animData = new Animation();
			newAnim._shadowData = new Animation();
			Resource::loadResource(newAnim._animData, animName.c_str(), true);
			if (!Resource::loadResource(newAnim._shadowData, shadowName.c_str(), false)) {
				delete newAnim._shadowData;
				newAnim._shadowData = nullptr;
			}
			newAnim._usage = 0;
			newAnim._state = 0; // enabled
			if ((_vm->_animList[animNumber]._flags & 4) != 0) {
				newAnim._state = 1;
				newAnim._frame = _vm->_animList[animNumber]._endPhase;
				newAnim._showFrame = _vm->_animList[animNumber]._endPhase;
			} else {
				newAnim._frame = _vm->_animList[animNumber]._startPhase;
				newAnim._showFrame = _vm->_animList[animNumber]._startPhase;
			}
			newAnim._flags = _vm->_animList[animNumber]._flags;
			newAnim._lastFrame = _vm->_animList[animNumber]._endPhase;
			newAnim._loopFrame = _vm->_animList[animNumber]._loopPhase;
			newAnim._loopType = _vm->_animList[animNumber]._loopType;
			newAnim._nextAnim = _vm->_animList[animNumber]._nextAnim;
			newAnim._x = _vm->_animList[animNumber]._x;
			newAnim._y = _vm->_animList[animNumber]._y;
			newAnim._currFrame = 0;
			newAnim._currX = _vm->_animList[animNumber]._x;
			newAnim._currY = _vm->_animList[animNumber]._y;
			newAnim._currW = 0;
			newAnim._currH = 0;
			newAnim._packFlag = 0;
			newAnim._shadowBack = _vm->_animList[animNumber]._type;
			newBackgroundAnim.backAnims.push_back(newAnim);
		}

		newBackgroundAnim._seq._type = READ_UINT32(&_data[animOffset]);
		newBackgroundAnim._seq._data = READ_UINT32(&_data[animOffset + 4]);
		newBackgroundAnim._seq._anims = READ_UINT32(&_data[animOffset + 8]);
		newBackgroundAnim._seq._current = newBackgroundAnim.backAnims[0]._basaData._num;
		newBackgroundAnim._seq._counter = 0;
		newBackgroundAnim._seq._currRelative = 0;
		newBackgroundAnim._seq._data2 = READ_UINT32(&_data[animOffset + 24]);

		int start = newBackgroundAnim.backAnims[0]._basaData._start; // BASA_Start of first frame
		int end = newBackgroundAnim.backAnims[0]._basaData._end; //BASA_End of first frame

		if (start != -1) {
			newBackgroundAnim.backAnims[0]._frame = start;
			newBackgroundAnim.backAnims[0]._showFrame = start;
			newBackgroundAnim.backAnims[0]._loopFrame = start;
		}

		if (end != -1) {
			newBackgroundAnim.backAnims[0]._lastFrame = end;
		}

		backAnimList[slot] = newBackgroundAnim;
	}
}

void Script::installBackAnims(Common::Array<BackgroundAnim> &backAnimList, int offset) {
	for (int i = 0; i < _vm->kMaxBackAnims; i++) {
		installSingleBackAnim(backAnimList, i, offset);
		offset += 4;
	}
}

void Script::installObjects(int offset) {
	for (int i = 0; i < _vm->kMaxObjects; i++) {
		if (_data[offset] != 0xFF) {
			_vm->_objSlot[i] = i;
		} else {
			_vm->_objSlot[i] = -1;
		}
		offset++;
	}
}

bool Script::loadAllMasks(Common::Array<Mask> &maskList, int offset) {
	Mask tempMask;
	while (1) {
		tempMask._state = READ_UINT32(&_data[offset]);
		if (tempMask._state == -1) {
			break;
		}
		tempMask._flags = READ_UINT32(&_data[offset + 2]);
		tempMask._x1 = READ_UINT32(&_data[offset + 4]);
		tempMask._y1 = READ_UINT32(&_data[offset + 6]);
		tempMask._x2 = READ_UINT32(&_data[offset + 8]);
		tempMask._y2 = READ_UINT32(&_data[offset + 10]);
		tempMask._z = READ_UINT32(&_data[offset + 12]);
		tempMask._number = READ_UINT32(&_data[offset + 14]);

		const Common::String msStreamName = Common::String::format("MS%02d", tempMask._number);
		Common::SeekableReadStream *msStream = SearchMan.createReadStreamForMember(msStreamName);
		if (!msStream) {
			tempMask._width = 0;
			tempMask._height = 0;
			tempMask._data = nullptr;
			debug("Can't load %s", msStreamName.c_str());
			delete msStream;
		} else {
			uint32 dataSize = msStream->size();
			if (dataSize != -1) {
				tempMask._data = (byte *)malloc(dataSize);
				if (msStream->read(tempMask._data, dataSize) != dataSize) {
					free(tempMask._data);
					delete msStream;
					return false;
				}
				delete msStream;
			}
			tempMask._width = tempMask.getWidth();
			tempMask._height = tempMask.getHeight();
		}

		maskList.push_back(tempMask);
		offset += 16; // size of Mask (Nak) struct
	}
	return true;
}

InterpreterFlags::InterpreterFlags() {
	resetAllFlags();
}

void InterpreterFlags::resetAllFlags() {
	memset(_flags, 0, sizeof(_flags));
}

void InterpreterFlags::setFlagValue(Flags::Id flagId, uint32 value) {
	_flags[(uint32)flagId - FLAG_MASK] = value;
}

uint32 InterpreterFlags::getFlagValue(Flags::Id flagId) {
	return _flags[(uint32)flagId - FLAG_MASK];
}

Interpreter::Interpreter(PrinceEngine *vm, Script *script, InterpreterFlags *flags) : 
	_vm(vm), _script(script), _flags(flags),
	_stacktop(0), _opcodeNF(false),
	_waitFlag(0), _result(true) {

	// Initialize the script
	_mode = "fg";
	_fgOpcodePC = _script->getStartGameOffset();
	_bgOpcodePC = 0;
}

void Interpreter::debugInterpreter(const char *s, ...) {
	char buf[STRINGBUFLEN];
	va_list va;
	va_start(va, s);
	vsnprintf(buf, STRINGBUFLEN, s, va);
	va_end(va);

	Common::String str = Common::String::format("@0x%08X: ", _lastInstruction);
	str += Common::String::format("op %04d: ", _lastOpcode);
	//debugC(10, DebugChannel::kScript, "PrinceEngine::Script %s %s", str.c_str(), buf);
	if (!strcmp(_mode, "fg")) {
		debug(10, "PrinceEngine::Script %s %s", str.c_str(), buf);
	}
	//debug("Prince::Script frame %08ld mode %s %s %s", _vm->_frameNr, _mode, str.c_str(), buf);
}

void Interpreter::step() {
	if (_bgOpcodePC) {
		_mode = "bg";
		_bgOpcodePC = step(_bgOpcodePC);
	}
	if (_fgOpcodePC) {
		_mode = "fg";
		_fgOpcodePC = step(_fgOpcodePC);
	}
}

uint32 Interpreter::step(uint32 opcodePC) {
	_currentInstruction = opcodePC;

	while (!_opcodeNF) {
		_lastInstruction = _currentInstruction;

		// Get the current opcode
		_lastOpcode = readScript<uint16>();

		if (_lastOpcode > kNumOpcodes)
			error(
				"Trying to execute unknown opcode @0x%04X: %02d", 
				_currentInstruction, 
				_lastOpcode);

		// Execute the current opcode
		OpcodeFunc op = _opcodes[_lastOpcode];
		(this->*op)();
		if (_opcodeNF) {
			_opcodeNF = 0;
			break;
		}
	}

	return _currentInstruction;
}

void Interpreter::storeNewPC(int opcodePC) {
	if (_flags->getFlagValue(Flags::GETACTION) == 1) {
		_flags->setFlagValue(Flags::GETACTIONDATA, opcodePC);
		opcodePC = _flags->getFlagValue(Flags::GETACTIONBACK);
	}
	_fgOpcodePC = opcodePC;
}

int Interpreter::getLastOPCode() {
	return _lastOpcode;
}

int Interpreter::getFgOpcodePC() {
	return _fgOpcodePC;
}

uint32 Interpreter::getCurrentString() {
	return _currentString;
}

void Interpreter::setCurrentString(uint32 value) {
	_currentString = value;
}

byte *Interpreter::getString() {
	return _string;
}

void Interpreter::setString(byte *newString) {
	_string = newString;
}

void Interpreter::increaseString() {
	while (*_string) {
		_string++;
	}
	_string++;
}

void Interpreter::setResult(byte value) {
	_result = value;
}

template <typename T>
T Interpreter::readScript() {
	T data = _script->read<T>(_currentInstruction);
	_currentInstruction += sizeof(data);
	return data;
}

uint16 Interpreter::readScriptFlagValue() {
	uint16 value = readScript<uint16>();
	if (value & InterpreterFlags::FLAG_MASK) {
		return _flags->getFlagValue((Flags::Id)value);
	}
	return value;
}

Flags::Id Interpreter::readScriptFlagId() { 
	return (Flags::Id)readScript<uint16>(); 
}

void Interpreter::O_WAITFOREVER() {
	_vm->changeCursor(_vm->_currentPointerNumber);
	_opcodeNF = 1;
	_currentInstruction -= 2;
	//debugInterpreter("O_WAITFOREVER");
}

void Interpreter::O_BLACKPALETTE() {
	_vm->blackPalette();
	debugInterpreter("O_BLACKPALETTE");
}

void Interpreter::O_SETUPPALETTE() {
	_vm->setPalette();
	debugInterpreter("O_SETUPPALETTE");
}

void Interpreter::O_INITROOM() {
	uint16 roomId = readScriptFlagValue();
	debugInterpreter("O_INITROOM %d", roomId);
	_vm->loadLocation(roomId);
	_opcodeNF = 1;
}

void Interpreter::O_SETSAMPLE() {
	uint16 sampleId = readScriptFlagValue();
	int32 sampleNameOffset = readScript<uint32>();
	const char *sampleName = _script->getString(_currentInstruction + sampleNameOffset - 4);
	debugInterpreter("O_SETSAMPLE %d %s", sampleId, sampleName);
	_vm->loadSample(sampleId, sampleName);
}

void Interpreter::O_FREESAMPLE() {
	uint16 sample = readScriptFlagValue();
	debugInterpreter("O_FREESAMPLE %d", sample);
}

void Interpreter::O_PLAYSAMPLE() {
	uint16 sampleId = readScriptFlagValue();
	uint16 loopType = readScript<uint16>();
	debugInterpreter("O_PLAYSAMPLE sampleId %d loopType %d", sampleId, loopType);
	_vm->playSample(sampleId, loopType);
}

void Interpreter::O_PUTOBJECT() {
	uint16 roomId = readScriptFlagValue();
	uint16 slot = readScriptFlagValue();
	uint16 objectId = readScriptFlagValue();
	Room *room = new Room();
	room->loadRoom(_script->getRoomOffset(roomId));
	int offset = room->_obj + slot;
	_vm->_script->setObjId(offset, objectId);
	if (_vm->_locationNr == roomId) {
		_vm->_objSlot[slot] = objectId;
	}
	delete room;
	debugInterpreter("O_PUTOBJECT roomId %d, slot %d, objectId %d", roomId, slot, objectId);
}

void Interpreter::O_REMOBJECT() {
	uint16 roomId = readScriptFlagValue();
	uint16 slot = readScriptFlagValue();
	Room *room = new Room();
	room->loadRoom(_script->getRoomOffset(roomId));
	int offset = room->_obj + slot;
	_vm->_script->setObjId(offset, 0xFF);
	if (_vm->_locationNr == roomId) {
		_vm->_objSlot[slot] = -1;
	}
	delete room;
	debugInterpreter("O_REMOBJECT roomId %d slot %d", roomId, slot);
}

void Interpreter::O_SHOWANIM() {
	uint16 slot = readScriptFlagValue();
	uint16 animId = readScriptFlagValue();
	_vm->freeNormAnim(slot);
	Anim &anim = _vm->_normAnimList[slot];
	AnimListItem &animList = _vm->_animList[animId];
	anim._currFrame = 0;
	anim._packFlag = 0;
	anim._state = 0;
	anim._frame = animList._startPhase;
	anim._showFrame = animList._startPhase;
	anim._lastFrame = animList._endPhase;
	anim._loopFrame = animList._loopPhase;
	anim._x = animList._x;
	anim._y = animList._y;
	anim._loopType = animList._loopType;
	anim._shadowBack = animList._type;
	anim._flags = animList._flags;
	anim._nextAnim = animList._nextAnim;
	int fileNumber = animList._fileNumber;
	const Common::String animName = Common::String::format("AN%02d", fileNumber);
	const Common::String shadowName = Common::String::format("AN%02dS", fileNumber);
	anim._animData = new Animation();
	anim._shadowData = new Animation();
	Resource::loadResource(anim._animData, animName.c_str(), true);
	if (!Resource::loadResource(anim._shadowData, shadowName.c_str(), false)) {
		delete anim._shadowData;
		anim._shadowData = nullptr;
	}
	debugInterpreter("O_SHOWANIM slot %d, animId %d", slot, animId);
}

void Interpreter::O_CHECKANIMEND() {
	uint16 slot = readScriptFlagValue();
	if (_vm->_normAnimList[slot]._frame != _vm->_normAnimList[slot]._lastFrame - 1) {
		_currentInstruction -= 4;
		_opcodeNF = 1;
	}
	debugInterpreter("O_CHECKANIMEND slot %d", slot);
}

void Interpreter::O_FREEANIM() {
	uint16 slot = readScriptFlagValue();
	_vm->freeNormAnim(slot);
	debugInterpreter("O_FREEANIM slot %d", slot);
}

void Interpreter::O_CHECKANIMFRAME() {
	uint16 slot = readScriptFlagValue();
	uint16 frameNumber = readScriptFlagValue();
	if (_vm->_normAnimList[slot]._frame != frameNumber) {
		_currentInstruction -= 6;
		_opcodeNF = 1;
	}
	debugInterpreter("O_CHECKANIMFRAME slot %d, frameNumber %d", slot, frameNumber);
}

void Interpreter::O_PUTBACKANIM() {
	uint16 roomId = readScriptFlagValue();
	uint16 slot = readScriptFlagValue();
	int32 animId = readScript<uint32>();
	Room *room = new Room();
	room->loadRoom(_script->getRoomOffset(roomId));
	int offset = room->_backAnim + slot * 4;
	_vm->_script->setBackAnimId(offset, animId);
	if (_vm->_locationNr == roomId) {
		_vm->_script->installSingleBackAnim(_vm->_backAnimList, slot, offset);
	}
	delete room;
	debugInterpreter("O_PUTBACKANIM roomId %d, slot %d, animId %d", roomId, slot, animId);
}

void Interpreter::O_REMBACKANIM() {
	uint16 roomId = readScriptFlagValue();
	uint16 slot = readScriptFlagValue();
	if (_vm->_locationNr == roomId) {
		_vm->removeSingleBackAnim(slot);
	}
	Room *room = new Room();
	room->loadRoom(_script->getRoomOffset(roomId));
	int offset = room->_backAnim + slot * 4;
	_vm->_script->setBackAnimId(offset, 0);
	delete room;
	debugInterpreter("O_REMBACKANIM roomId %d, slot %d", roomId, slot);
}

void Interpreter::O_CHECKBACKANIMFRAME() {
	uint16 slotId = readScriptFlagValue();
	uint16 frameId = readScriptFlagValue();
	int currAnim = _vm->_backAnimList[slotId]._seq._currRelative;
	if (_vm->_backAnimList[slotId].backAnims[currAnim]._frame != frameId) {
		_currentInstruction -= 6;
		_opcodeNF = 1;
	}
	debugInterpreter("O_CHECKBACKANIMFRAME slotId %d, frameId %d", slotId, frameId);
}

void Interpreter::O_FREEALLSAMPLES() {
	debugInterpreter("O_FREEALLSAMPLES");
}

void Interpreter::O_SETMUSIC() {
	uint16 musicId = readScript<uint16>();

	debugInterpreter("O_SETMUSIC musicId %d", musicId);
}

void Interpreter::O_STOPMUSIC() {
	debugInterpreter("O_STOPMUSIC");
}

void Interpreter::O__WAIT() {
	uint16 pause = readScriptFlagValue();

	debugInterpreter("O__WAIT pause %d", pause);

	if (!_waitFlag) {
		// set new wait flag value and continue
		_waitFlag = pause;
		_opcodeNF = 1;
		_currentInstruction -= 4;
		return;
	}

	_waitFlag--;

	if (_waitFlag > 0) {
		_opcodeNF = 1;
		_currentInstruction -= 4;
		return;
	}
}

void Interpreter::O_UPDATEOFF() {
	debugInterpreter("O_UPDATEOFF");
	//_updateEnable = false;
}

void Interpreter::O_UPDATEON() {
	debugInterpreter("O_UPDATEON");
	//_updateEnable = true;
}

void Interpreter::O_UPDATE () {
	debugInterpreter("O_UPDATE");
	// Refresh screen
}

void Interpreter::O_CLS() {
	debugInterpreter("O_CLS");
	// do nothing
}

void Interpreter::O__CALL() {
	int32 address = readScript<uint32>();
	_stack[_stacktop] = _currentInstruction;
	_stacktop++;
	_currentInstruction += address - 4;
	debugInterpreter("O__CALL 0x%04X", _currentInstruction);
}

void Interpreter::O_RETURN() {
	// Get the return address
	if (_stacktop > 0) {
		_stacktop--;
		_currentInstruction = _stack[_stacktop];
		debugInterpreter("O_RETURN 0x%04X", _currentInstruction);
	} else {
		error("Return: Stack is empty");
	}
}

void Interpreter::O_GO() {
	int32 opPC = readScript<uint32>();
	debugInterpreter("O_GO 0x%04X", opPC);
	_currentInstruction += opPC - 4;
}

void Interpreter::O_BACKANIMUPDATEOFF() {
	uint16 slotId = readScriptFlagValue();
	int currAnim = _vm->_backAnimList[slotId]._seq._currRelative;
	_vm->_backAnimList[slotId].backAnims[currAnim]._state = 1;
	debugInterpreter("O_BACKANIMUPDATEOFF slotId %d", slotId);
}

void Interpreter::O_BACKANIMUPDATEON() {
	uint16 slotId = readScriptFlagValue();
	int currAnim = _vm->_backAnimList[slotId]._seq._currRelative;
	_vm->_backAnimList[slotId].backAnims[currAnim]._state = 0;
	debugInterpreter("O_BACKANIMUPDATEON %d", slotId);
}

void Interpreter::O_CHANGECURSOR() {
	uint16 cursorId = readScriptFlagValue();
	debugInterpreter("O_CHANGECURSOR %x", cursorId);
	_vm->changeCursor(cursorId);
}

void Interpreter::O_CHANGEANIMTYPE() {
	// NOT IMPLEMENTED
}

void Interpreter::O__SETFLAG() {
	Flags::Id flagId = readScriptFlagId();
	uint16 value = readScriptFlagValue();

	debugInterpreter("O__SETFLAG 0x%04X (%s) = %d", flagId, Flags::getFlagName(flagId), value);

	_flags->setFlagValue((Flags::Id)(flagId), value);
}

void Interpreter::O_COMPARE() {
	Flags::Id flagId = readScriptFlagId();
	uint16 value = readScriptFlagValue();

	_result = _flags->getFlagValue(flagId) != value;
	debugInterpreter("O_COMPARE flagId 0x%04X (%s), value %d == %d (%d)", flagId, Flags::getFlagName(flagId), value, _flags->getFlagValue(flagId), _result);
}

void Interpreter::O_JUMPZ() {
	int32 offset = readScript<uint32>();
	if (!_result) {
		_currentInstruction += offset - 4;
	}

	debugInterpreter("O_JUMPZ result = %d, next %08x, offset 0x%08X", _result, _currentInstruction, offset);
}

void Interpreter::O_JUMPNZ() {
	int32 offset = readScript<uint32>();
	if (_result) {
		_currentInstruction += offset - 4;
	}

	debugInterpreter("O_JUMPNZ result = %d, next %08x, offset 0x%08X", _result, _currentInstruction, offset);
}

void Interpreter::O_EXIT() {
	uint16 exitCode = readScriptFlagValue();
	debugInterpreter("O_EXIT exitCode %d", exitCode);
	// Set exit code and shows credits 
	// if exit code == 0x02EAD
}

void Interpreter::O_ADDFLAG() {
	Flags::Id flagId = readScriptFlagId();
	uint16 value = readScriptFlagValue();

	_flags->setFlagValue(flagId, _flags->getFlagValue(flagId) + value);
	if (_flags->getFlagValue(flagId))
		_result = 1;
	else
		_result = 0;

	debugInterpreter("O_ADDFLAG flagId %04x (%s), value %d", flagId, Flags::getFlagName(flagId), value);
}

void Interpreter::O_TALKANIM() {
	uint16 animNumber = readScriptFlagValue();
	uint16 slot = readScriptFlagValue();
	_vm->doTalkAnim(animNumber, slot, kNormalAnimation);
	debugInterpreter("O_TALKANIM animNumber %d, slot %d", animNumber, slot);
}

void Interpreter::O_SUBFLAG() {
	Flags::Id flagId = readScriptFlagId();
	uint16 value = readScriptFlagValue();

	_flags->setFlagValue(flagId, _flags->getFlagValue(flagId) - value);
	if (_flags->getFlagValue(flagId))
		_result = 1;
	else
		_result = 0;

	debugInterpreter("O_SUBFLAG flagId %d, value %d", flagId, value);
}

void Interpreter::O_SETSTRING() {
	int32 offset = readScript<uint32>();
	_currentString = offset;

	if (offset >= 80000) {
		_string = _vm->_variaTxt->getString(offset - 80000);
		debugInterpreter("GetVaria %s", _string);
	}
	else if (offset < 2000) {
		_vm->_dialogData = &_vm->_dialogDat[offset * 4 - 4];

		uint32 of = READ_LE_UINT32(_vm->_talkTxt + offset * 4);
		const char *txt = (const char *)&_vm->_talkTxt[of];
		_string = &_vm->_talkTxt[of];
		debugInterpreter("TalkTxt %d %s", of, txt);
	}

	debugInterpreter("O_SETSTRING %04d", offset);
}

void Interpreter::O_ANDFLAG() {
	Flags::Id flagId = readScriptFlagId();
	uint16 value = readScriptFlagValue();

	debugInterpreter("O_ANDFLAG flagId %d, value %d", flagId, value);

	_flags->setFlagValue(flagId, _flags->getFlagValue(flagId) & value);

	if (_flags->getFlagValue(flagId)) {
		_result = 1;
	} else {
		_result = 0;
	}
}

void Interpreter::O_GETMOBDATA() {
	Flags::Id flagId = readScriptFlagId();
	uint16 mobId = readScriptFlagValue();
	uint16 mobOffset = readScriptFlagValue();
	int16 value = _vm->_mobList[mobId].getData((Mob::AttrId)mobOffset);
	_flags->setFlagValue(flagId, value);
	debugInterpreter("O_GETMOBDATA flagId %d, modId %d, mobOffset %d", flagId, mobId, mobOffset);
}

void Interpreter::O_ORFLAG() {
	Flags::Id flagId = readScriptFlagId();
	uint16 value = readScriptFlagValue();
	
	debugInterpreter("O_ORFLAG flagId %d, value %d", flagId, value);
	
	_flags->setFlagValue(flagId, _flags->getFlagValue(flagId) | value);

	if (_flags->getFlagValue(flagId)) {
		_result = 1;
	} else {
		_result = 0;
	}
}

void Interpreter::O_SETMOBDATA() {
	uint16 mobId = readScriptFlagValue();
	uint16 mobOffset = readScriptFlagValue();
	uint16 value = readScriptFlagValue();
	_vm->_mobList[mobId].setData((Mob::AttrId)mobOffset, value);
	debugInterpreter("O_SETMOBDATA mobId %d, mobOffset %d, value %d", mobId, mobOffset, value);
}

void Interpreter::O_XORFLAG() {
	Flags::Id flagId = readScriptFlagId();
	uint16 value = readScriptFlagValue();

	debugInterpreter("O_XORFLAG flagId %d, value %d", flagId, value);

	_flags->setFlagValue(flagId, _flags->getFlagValue(flagId) ^ value);

	if (_flags->getFlagValue(flagId)) {
		_result = 1;
	} else {
		_result = 0;
	}
}

void Interpreter::O_GETMOBTEXT() {
	uint16 mob = readScriptFlagValue();
	_currentString = _vm->_locationNr * 100 + mob + 60001;
	_string = (byte *)_vm->_mobList[mob]._examText.c_str();
	debugInterpreter("O_GETMOBTEXT mob %d", mob);
}

void Interpreter::O_MOVEHERO() {
	uint16 heroId = readScriptFlagValue();
	uint16 x = readScriptFlagValue();
	uint16 y = readScriptFlagValue();
	uint16 dir = readScriptFlagValue();
	_vm->moveRunHero(heroId, x, y, dir, false);
	debugInterpreter("O_MOVEHERO heroId %d, x %d, y %d, dir %d", heroId, x, y, dir);
}

void Interpreter::O_WALKHERO() {
	uint16 heroId = readScriptFlagValue();
	Hero *hero = nullptr;
	if (!heroId) {
		hero = _vm->_mainHero;
	} else if (heroId == 1) {
		hero = _vm->_secondHero;
	}
	if (hero != nullptr) {
		if (hero->_state != Hero::STAY) {
			_currentInstruction -= 4;
			_opcodeNF = 1;
		}
	}
	debugInterpreter("O_WALKHERO %d", heroId);
}

void Interpreter::O_SETHERO() {
	uint16 hero = readScriptFlagValue();
	int16 x = readScriptFlagValue();
	int16 y = readScriptFlagValue();
	uint16 dir = readScriptFlagValue();
	if (hero == 0) {
		_vm->_mainHero->setPos(x, y);
		_vm->_mainHero->_lastDirection = dir;
		_vm->_mainHero->_state = _vm->_mainHero->STAY;
		_vm->_mainHero->_moveSetType = _vm->_mainHero->_lastDirection - 1; // for countDrawPosition
		_vm->_mainHero->countDrawPosition(); //setting drawX, drawY
		_vm->_mainHero->_visible = 1;
	} else if (hero == 1) {
		_vm->_secondHero->setPos(x, y);
		_vm->_secondHero->_lastDirection = dir;
		_vm->_secondHero->_state = _vm->_mainHero->STAY;
		_vm->_secondHero->_moveSetType = _vm->_mainHero->_lastDirection - 1; // for countDrawPosition
		_vm->_secondHero->countDrawPosition(); //setting drawX, drawY
		_vm->_secondHero->_visible = 1;
	}
	debugInterpreter("O_SETHERO hero %d, x %d, y %d, dir %d", hero, x, y, dir);
}

void Interpreter::O_HEROOFF() {
	uint16 heroId = readScriptFlagValue();
	debugInterpreter("O_HEROOFF %d", heroId);
	_vm->_mainHero->setVisible(false);
}

void Interpreter::O_HEROON() {
	uint16 heroId = readScriptFlagValue();
	debugInterpreter("O_HEROON %d", heroId);
	_vm->_mainHero->setVisible(true);
}

void Interpreter::O_CLSTEXT() {
	uint16 slot = readScriptFlagValue();
	debugInterpreter("O_CLSTEXT slot %d", slot);
	_vm->_textSlots[slot]._str = nullptr;
	_vm->_textSlots[slot]._time = 0;
}

void Interpreter::O_CALLTABLE() {
	uint16 flag = readScript<uint16>();
	int32 table = readScript<uint32>();

	debugInterpreter("O_CALLTABLE flag %d, table %d", flag, table);
	// makes a call from script function table
	// must read table pointer from _code and
	// use table entry as next opcode
}

void Interpreter::O_CHANGEMOB() {
	uint16 mob = readScriptFlagValue();
	uint16 value = readScriptFlagValue();
	value ^= 1;
	_vm->_script->setMobVisible(mob, value);
	_vm->_mobList[mob]._visible = value;
	debugInterpreter("O_CHANGEMOB mob %d, value %d", mob, value);
}

void Interpreter::O_ADDINV() {
	uint16 hero = readScriptFlagValue();
	uint16 item = readScriptFlagValue();
	_vm->addInv(hero, item, false);
	debugInterpreter("O_ADDINV hero %d, item %d", hero, item);
}

void Interpreter::O_REMINV() {
	uint16 hero = readScriptFlagValue();
	uint16 item = readScriptFlagValue();
	_vm->remInv(hero, item);
	debugInterpreter("O_REMINV hero %d, item %d", hero, item);
}

void Interpreter::O_REPINV() {
	uint16 hero = readScript<uint16>();
	uint16 item1 = readScript<uint16>();
	uint16 item2 = readScript<uint16>();
	// shouldn't be uses
	error("O_REPINV hero %d, item1 %d, item2 %d", hero, item1, item2);
}

void Interpreter::O_OBSOLETE_GETACTION() {
	// shouldn't be uses
	error("O_OBSOLETE_GETACTION");
}

void Interpreter::O_ADDWALKAREA() {
	uint16 x1 = readScript<uint16>();
	uint16 y1 = readScript<uint16>();
	uint16 x2 = readScript<uint16>();
	uint16 y2 = readScript<uint16>();
	// shouldn't be uses
	error("O_ADDWALKAREA x1 %d, y1 %d, x2 %d, y2 %d", x1, y1, x2, y2);
}

void Interpreter::O_REMWALKAREA() {
	uint16 x1 = readScript<uint16>();
	uint16 y1 = readScript<uint16>();
	uint16 x2 = readScript<uint16>();
	uint16 y2 = readScript<uint16>();

	// shouldn't be uses
	error("O_REMWALKAREA x1 %d, y1 %d, x2 %d, y2 %d", x1, y1, x2, y2);
}
 
void Interpreter::O_RESTOREWALKAREA() {
	debugInterpreter("O_RESTOREWALKAREA");
}

void Interpreter::O_WAITFRAME() {
	debugInterpreter("O_WAITFRAME");
	_opcodeNF = true;
}

void Interpreter::O_SETFRAME() {
	uint16 anim = readScriptFlagValue();
	uint16 frame = readScriptFlagValue();
	_vm->_normAnimList[anim]._frame = frame;
	debugInterpreter("O_SETFRAME anim %d, frame %d", anim, frame);
}

void Interpreter::O_RUNACTION() {
	// It's empty in original and never used in script
	// it's better to report error
	error("O_RUNACTION");
}

void Interpreter::O_COMPAREHI() {
	Flags::Id flag = readScriptFlagId();
	uint16 value = readScriptFlagValue();
	uint16 flagValue = _flags->getFlagValue(flag);
	if (flagValue > value) {
		_result = 0;
	} else {
		_result = 1;
	}
	debugInterpreter("O_COMPAREHI flag %04x - (%s), value %d, flagValue %d, result %d", flag, Flags::getFlagName(flag), value, flagValue, _result);
}

void Interpreter::O_COMPARELO() {
	Flags::Id flag = readScriptFlagId();
	uint16 value = readScriptFlagValue();
	uint16 flagValue = _flags->getFlagValue(flag);
	if (flagValue < value) {
		_result = 0;
	} else {
		_result = 1;
	}
	debugInterpreter("O_COMPARELO flag %04x - (%s), value %d, flagValue %d, result %d", flag, Flags::getFlagName(flag), value, flagValue, _result);
}

void Interpreter::O_PRELOADSET() {
	// not used in script
	int32 offset = readScript<uint32>();
	debugInterpreter("O_PRELOADSET offset %04x", offset);
}

void Interpreter::O_FREEPRELOAD() {
	// not used in script
	debugInterpreter("O_FREEPRELOAD");
}

void Interpreter::O_CHECKINV() {
	uint16 hero = readScriptFlagValue();
	uint16 item = readScriptFlagValue();
	_vm->checkInv(hero, item);
	debugInterpreter("O_CHECKINV hero %d, item %d", hero, item);
}

void Interpreter::O_TALKHERO() {
	uint16 hero = readScriptFlagValue();
	debugInterpreter("O_TALKHERO hero %d", hero);
	_vm->talkHero(hero);
}

void Interpreter::O_WAITTEXT() {
	uint16 slot = readScriptFlagValue();
	Text &text = _vm->_textSlots[slot];
	if (text._time && text._str) {
		if (_flags->getFlagValue(Flags::ESCAPED)) {
			text._time = 1;
			if (slot == 0) {
				_vm->_mainHero->_talkTime = 1;
			} else if (slot == 1) {
				_vm->_secondHero->_talkTime = 1;
			}
		} else {
			_opcodeNF = 1;
			_currentInstruction -= 4;
		}
	}
}

void Interpreter::O_SETHEROANIM() {
	uint16 heroId = readScriptFlagValue();
	int32 offset = readScript<uint32>();
	Hero *hero = nullptr;
	if (!heroId) {
		hero = _vm->_mainHero;
	} else {
		hero = _vm->_secondHero;
	}
	if (hero != nullptr) {
		hero->freeHeroAnim();
		if (hero ->_specAnim == nullptr) {
			hero->_specAnim = new Animation();
			if (offset < 100) {
				const Common::String animName = Common::String::format("AN%02d", offset);
				Resource::loadResource(hero->_specAnim, animName.c_str(), true);
			} else {
				const Common::String animName = Common::String((const char *)_script->getHeroAnimName(offset));
				Common::String normalizedPath = lastPathComponent(animName, '\\');
				Resource::loadResource(hero->_specAnim, normalizedPath.c_str(), true);
			}
			hero->_phase = 0;
			hero->_state = Hero::SPEC;
		}
	}
	debugInterpreter("O_SETHEROANIM hero %d, offset %d", hero, offset);
}

void Interpreter::O_WAITHEROANIM() {
	uint16 hero = readScriptFlagValue();
	if (hero == 0) {
		if (_vm->_mainHero->_state == _vm->_mainHero->SPEC) {
			_currentInstruction -= 4;
			_opcodeNF = 1;
		}
	} else if (hero == 1) {
		if (_vm->_secondHero->_state == _vm->_secondHero->SPEC) {
			_currentInstruction -= 4;
			_opcodeNF = 1;
		}
	}
	debugInterpreter("O_WAITHEROANIM hero %d", hero);
}

void Interpreter::O_GETHERODATA() {
	Flags::Id flagId = readScriptFlagId();
	uint16 hero = readScriptFlagValue();
	uint16 heroOffset = readScriptFlagValue();
	if (hero == 0) {
		_flags->setFlagValue(flagId, _vm->_mainHero->getData((Hero::AttrId)heroOffset));
	} else if (hero == 1) {
		_flags->setFlagValue(flagId, _vm->_secondHero->getData((Hero::AttrId)heroOffset));
	}
	debugInterpreter("O_GETHERODATA flag %04x - (%s), hero %d, heroOffset %d", flagId, Flags::getFlagName(flagId), hero, heroOffset);
}

void Interpreter::O_GETMOUSEBUTTON() {
	debugInterpreter("O_GETMOUSEBUTTON");
}

void Interpreter::O_CHANGEFRAMES() {
	uint16 anim = readScriptFlagValue();
	uint16 frame = readScriptFlagValue();
	uint16 lastFrame = readScriptFlagValue();
	uint16 loopFrame = readScriptFlagValue();
	_vm->_normAnimList[anim]._frame = frame;
	_vm->_normAnimList[anim]._lastFrame = lastFrame;
	_vm->_normAnimList[anim]._loopFrame = loopFrame;
	debugInterpreter("O_CHANGFRAMES anim %d, frame %d, lastFrame %d, loopFrame %d", anim, frame, lastFrame, loopFrame);
}

void Interpreter::O_CHANGEBACKFRAMES() {
	uint16 anim = readScriptFlagValue();
	uint16 frame = readScriptFlagValue();
	uint16 lastFrame = readScriptFlagValue();
	uint16 loopFrame = readScriptFlagValue();
	int currAnim = _vm->_backAnimList[anim]._seq._currRelative;
	Anim &backAnim = _vm->_backAnimList[anim].backAnims[currAnim];
	backAnim._frame = frame;
	backAnim._lastFrame = lastFrame;
	backAnim._loopFrame = loopFrame;
	debugInterpreter("O_CHANGEBACKFRAMES anim %d, frame %d, lastFrame %d, loopFrame %d", anim, frame, lastFrame, loopFrame);
}

void Interpreter::O_GETBACKANIMDATA() {
	Flags::Id flagId = readScriptFlagId();
	uint16 animNumber = readScriptFlagValue();
	uint16 animDataOffset = readScriptFlagValue();
	int currAnim = _vm->_backAnimList[animNumber]._seq._currRelative;
	int16 value = _vm->_backAnimList[animNumber].backAnims[currAnim].getAnimData((Anim::AnimOffsets)(animDataOffset));
	_flags->setFlagValue((Flags::Id)(flagId), value);
	debugInterpreter("O_GETBACKANIMDATA flag %04X (%s), animNumber %d, animDataOffset %d, value %d", flagId, Flags::getFlagName(flagId), animNumber, animDataOffset, value);
}

void Interpreter::O_GETANIMDATA() {
	Flags::Id flagId = readScriptFlagId();
	uint16 anim = readScriptFlagValue();
	uint16 animOffset = readScriptFlagValue();
	if (_vm->_normAnimList[anim]._animData != nullptr) {
		_flags->setFlagValue(flagId, _vm->_normAnimList[anim].getAnimData((Anim::AnimOffsets)(animOffset)));
	}
	debugInterpreter("O_GETANIMDATA flag %04X (%s), anim %d, animOffset %d", flagId, Flags::getFlagName(flagId), anim, animOffset);
}

void Interpreter::O_SETBGCODE() {
	int32 offset = readScript<uint32>();
	_bgOpcodePC = _currentInstruction + offset - 4;
	debugInterpreter("O_SETBGCODE next %08x, offset %08x", _bgOpcodePC, offset);
}

void Interpreter::O_SETBACKFRAME() {
	uint16 anim = readScriptFlagValue();
	uint16 frame = readScriptFlagValue();
	int currAnim = _vm->_backAnimList[anim]._seq._currRelative;
	if (_vm->_backAnimList[anim].backAnims[currAnim]._animData != nullptr) {
		_vm->_backAnimList[anim].backAnims[currAnim]._frame = frame;
	}
	debugInterpreter("O_SETBACKFRAME anim %d, frame %d", anim, frame);
}

void Interpreter::O_GETRND() {
	Flags::Id flag = readScriptFlagId();
	uint16 rndSeed = readScript<uint16>();
	int value = _vm->_randomSource.getRandomNumber(rndSeed - 1);
	_flags->setFlagValue(flag, value);
	debugInterpreter("O_GETRND flag %d, rndSeed %d, value %d", flag, rndSeed, value);
}

void Interpreter::O_TALKBACKANIM() {
	uint16 animNumber = readScriptFlagValue();
	uint16 slot = readScriptFlagValue();
	_vm->doTalkAnim(animNumber, slot, kBackgroundAnimation);
	debugInterpreter("O_TALKBACKANIM animNumber %d, slot %d", animNumber, slot);
}

void Interpreter::O_LOADPATH() {
	readScript<uint32>();
	//int32 offset = readScript<uint32>();
	// simplifying, because used only once in Location 20
	_vm->loadPath("path2");
	debugInterpreter("O_LOADPATH - path2");
}

void Interpreter::O_GETCHAR() {
	Flags::Id flagId = readScriptFlagId();
	_flags->setFlagValue(flagId, *_string);
	debugInterpreter("O_GETCHAR %04X (%s) %02x", flagId, Flags::getFlagName(flagId), _flags->getFlagValue(flagId));
	_string++;
}

void Interpreter::O_SETDFLAG() {
	Flags::Id flagId = readScriptFlagId();
	int32 address = readScript<uint32>();
	debugInterpreter("O_SETDFLAG 0x%04X (%s) = 0x%04X", flagId, Flags::getFlagName(flagId), _currentInstruction + address - 4);
	_flags->setFlagValue((Flags::Id)(flagId), _currentInstruction + address - 4);
}

void Interpreter::O_CALLDFLAG() {
	Flags::Id flagId = readScriptFlagId();
	_stack[_stacktop] = _currentInstruction;
	_stacktop++;
	_currentInstruction = _flags->getFlagValue(flagId);
	debugInterpreter("O_CALLDFLAG 0x%04X (%s) = 0x%04X", flagId, Flags::getFlagName(flagId), _currentInstruction);
}

void Interpreter::O_PRINTAT() {
	uint16 slot = readScriptFlagValue();
	uint16 x = readScriptFlagValue();
	uint16 y = readScriptFlagValue();
	debugInterpreter("O_PRINTAT slot %d, x %d, y %d", slot, x, y);
	uint8 color = _flags->getFlagValue(Flags::KOLOR);
	_vm->printAt(slot, color, (char *)_string, x, y);
	increaseString();
}

void Interpreter::O_ZOOMIN() {
	uint16 slot = readScriptFlagValue();
	_vm->initZoomIn(slot);
	debugInterpreter("O_ZOOMIN slot %04d", slot);
}

void Interpreter::O_ZOOMOUT() {
	uint16 slot = readScriptFlagValue();
	_vm->initZoomOut(slot);
	debugInterpreter("O_ZOOMOUT slot %d", slot);
}

// TODO - never used?
void Interpreter::O_SETSTRINGOFFSET() {
	int32 offset = readScript<uint32>();
	debugInterpreter("O_SETSTRINGOFFSET offset %04x", offset);
	_currentString = 0;
	_string = (byte *)_currentInstruction + offset; //FIXME
}

void Interpreter::O_GETOBJDATA() {
	Flags::Id flag = readScriptFlagId();
	uint16 obj = readScriptFlagValue();
	int16 objOffset = readScriptFlagValue();
	int16 value = _vm->_objList[obj]->getData((Object::AttrId)objOffset);
	_flags->setFlagValue(flag, value);
	debugInterpreter("O_GETOBJDATA flag %d, obj %d, objOffset %d", flag, obj, objOffset);
}

void Interpreter::O_SETOBJDATA() {
	uint16 obj = readScriptFlagValue();
	int16 objOffset = readScriptFlagValue();
	uint16 value = readScriptFlagValue();
	_vm->_objList[obj]->setData((Object::AttrId)objOffset, value);
	debugInterpreter("O_SETOBJDATA obj %d, objOffset %d, value %d", obj, objOffset, value);
}

// not used?
void Interpreter::O_SWAPOBJECTS() {
	uint16 obj1 = readScriptFlagValue();
	uint16 obj2 = readScriptFlagValue();
	debugInterpreter("O_SWAPOBJECTS obj1 %d, obj2 %d", obj1, obj2);
}

void Interpreter::O_CHANGEHEROSET() {
	uint16 heroId = readScriptFlagValue();
	uint16 heroSet = readScriptFlagValue();
	if (!heroId) {
		_vm->_mainHero->loadAnimSet(heroSet);
	} else if (heroId == 1) {
		_vm->_secondHero->loadAnimSet(heroSet);
	}
	debugInterpreter("O_CHANGEHEROSET hero %d, heroSet %d", heroId, heroSet);
}

void Interpreter::O_ADDSTRING() {
	uint16 value = readScriptFlagValue();
	debugInterpreter("O_ADDSTRING value %d", value);
	_string += value;
}

void Interpreter::O_SUBSTRING() {
	uint16 value = readScriptFlagValue();
	debugInterpreter("O_SUBSTRING value %d", value);
	_string -= value;
}

int Interpreter::checkSeq(byte *string) {
	int freeHSlotIncrease = 0;
	byte c;
	while ((c = string[0]) != 0xFF) {
		string++;
		if (c < 0xF0) {
			freeHSlotIncrease++;
			while ((c = string[0])) {
				string++;
			}
			string++;
		} else if (c != 0xFE) {
			string++;
		}
	}
	return freeHSlotIncrease;
}

void Interpreter::O_INITDIALOG() {
	debugInterpreter("O_INITDIALOG");
	if (_string[0] == 255) {
		byte *stringCurrOff = _string;
		byte *string = _string;
		stringCurrOff++;
		int32 adressOfFirstSequence = (int)READ_UINT16(stringCurrOff);
		stringCurrOff += 2;
		_string = string + adressOfFirstSequence;

		for (int i = 0; i < 32; i++) {
			_vm->_dialogBoxAddr[i] = 0;
			_vm->_dialogOptAddr[i] = 0;
		}

		for (int i = 0; i < 4 * 32; i++) {
			_vm->_dialogOptLines[i] = 0;
		}

		int16 off;
		byte *line = nullptr;

		int dialogBox = 0;
		while ((off = (int)READ_UINT16(stringCurrOff)) != -1) {
			stringCurrOff += 2;
			if (off) {
				line = string + off;
			}
			_vm->_dialogBoxAddr[dialogBox] = line;
			dialogBox++;
		}
		stringCurrOff += 2;

		int dialogOpt = 0;
		while ((off = (int)READ_UINT16(stringCurrOff)) != -1) {
			stringCurrOff += 2;
			if (off) {
				line = string + off;
			}
			_vm->_dialogOptAddr[dialogOpt] = line;
			dialogOpt++;
		}

		_flags->setFlagValue(Flags::VOICE_A_LINE, 0);
		_flags->setFlagValue(Flags::VOICE_B_LINE, 0); // bx in original?

		int freeHSlot = 0;
		for (int i = 31; i >= 0; i--) {
			if (_vm->_dialogOptAddr[i] != 0) {
				i++;
				freeHSlot = i;
				_flags->setFlagValue(Flags::VOICE_H_LINE, i);
				break;
			}
		}

		freeHSlot += checkSeq(_string);

		for (int i = 0; i < 32; i++) {
			_vm->_dialogOptLines[i * 4] = freeHSlot;
			_vm->_dialogOptLines[i * 4 + 1] = freeHSlot;
			_vm->_dialogOptLines[i * 4 + 2] = freeHSlot;
			if (_vm->_dialogOptAddr[i]) {
				freeHSlot += checkSeq(_vm->_dialogOptAddr[i]);
			}
		}
	}
}

void Interpreter::O_ENABLEDIALOGOPT() {
	uint16 opt = readScriptFlagValue();
	debugInterpreter("O_ENABLEDIALOGOPT opt %d", opt);
	int dialogDataValue = (int)READ_UINT32(_vm->_dialogData);
	dialogDataValue &= ~(1u << opt);
	WRITE_UINT32(_vm->_dialogData, dialogDataValue);
}

void Interpreter::O_DISABLEDIALOGOPT() {
	uint16 opt = readScriptFlagValue();
	debugInterpreter("O_DISABLEDIALOGOPT opt %d", opt);
	int dialogDataValue = (int)READ_UINT32(_vm->_dialogData);
	dialogDataValue |= (1u << opt);
	WRITE_UINT32(_vm->_dialogData, dialogDataValue);
}

void Interpreter::O_SHOWDIALOGBOX() {
	uint16 box = readScriptFlagValue();
	debugInterpreter("O_SHOWDIALOGBOX box %d", box);
	_vm->createDialogBox(box);
	_flags->setFlagValue(Flags::DIALINES, _vm->_dialogLines);
	if (_vm->_dialogLines != 0) {
		_vm->changeCursor(1);
		_vm->runDialog();
		_vm->changeCursor(0);
	}
}

void Interpreter::O_STOPSAMPLE() {
	uint16 slot = readScriptFlagValue();
	debugInterpreter("O_STOPSAMPLE slot %d", slot);
	_vm->stopSample(slot);
}

void Interpreter::O_BACKANIMRANGE() {
	uint16 slotId = readScriptFlagValue();
	uint16 animId = readScript<uint16>();
	uint16 low = readScriptFlagValue();
	uint16 high = readScriptFlagValue();

	if (animId != 0xFFFF) {
		if (animId & InterpreterFlags::FLAG_MASK) {
			animId = _flags->getFlagValue((Flags::Id)animId);
		}
	}

	_result = 1;
	if (!_vm->_backAnimList[slotId].backAnims.empty()) {
		int currAnim = _vm->_backAnimList[slotId]._seq._currRelative;
		if (_vm->_backAnimList[slotId].backAnims[currAnim]._animData != nullptr) {
			if (animId == 0xFFFF || _vm->_backAnimList[slotId]._seq._current == animId) {
				int currAnim = _vm->_backAnimList[slotId]._seq._currRelative;
				Anim &backAnim = _vm->_backAnimList[slotId].backAnims[currAnim];
				if (!backAnim._state) {
					if (backAnim._frame >= low) {
						if (backAnim._frame <= high) {
							_result = 0;
						}
					}
				}
			}
		}
	}
	debugInterpreter("O_BACKANIMRANGE slotId %d, animId %d, low %d, high %d, _result %d", slotId, animId, low, high, _result);
}

void Interpreter::O_CLEARPATH() {
	for (int i = 0; i < _vm->kPathBitmapLen; i++) {
		_vm->_roomPathBitmap[i] = 255;
	}
	debugInterpreter("O_CLEARPATH");
}

void Interpreter::O_SETPATH() {
	_vm->loadPath("path");
	debugInterpreter("O_SETPATH");
}

void Interpreter::O_GETHEROX() {
	uint16 heroId = readScriptFlagValue();
	Flags::Id flagId = readScriptFlagId();
	if (heroId == 0) {
		_flags->setFlagValue(flagId, _vm->_mainHero->_middleX);
	} else if (heroId == 1) {
		_flags->setFlagValue(flagId, _vm->_secondHero->_middleX);
	}
	debugInterpreter("O_GETHEROX heroId %d, flagId %d", heroId, flagId);
}

void Interpreter::O_GETHEROY() {
	uint16 heroId = readScriptFlagValue();
	Flags::Id flagId = readScriptFlagId();
	if (heroId == 0) {
		_flags->setFlagValue(flagId, _vm->_mainHero->_middleY);
	} else if (heroId == 1) {
		_flags->setFlagValue(flagId, _vm->_secondHero->_middleY);
	}
	debugInterpreter("O_GETHEROY heroId %d, flagId %d", heroId, flagId);
}

void Interpreter::O_GETHEROD() {
	uint16 heroId = readScriptFlagValue();
	Flags::Id flagId = readScriptFlagId();
	if (heroId == 0) {
		_flags->setFlagValue(flagId, _vm->_mainHero->_lastDirection);
	} else if (heroId == 1) {
		_flags->setFlagValue(flagId, _vm->_secondHero->_lastDirection);
	}
	debugInterpreter("O_GETHEROD heroId %d, flagId %d", heroId, flagId);
}

void Interpreter::O_PUSHSTRING() {
	_stringStack.string = _string;
	_stringStack.dialogData = _vm->_dialogData;
	_stringStack.currentString = _currentString;
	debugInterpreter("O_PUSHSTRING");
}

void Interpreter::O_POPSTRING() {
	_string = _stringStack.string;
	_vm->_dialogData = _stringStack.dialogData;
	_currentString = _stringStack.currentString;
	debugInterpreter("O_POPSTRING");
}

void Interpreter::O_SETFGCODE() {
	int32 offset = readScript<uint32>();
	_fgOpcodePC = _currentInstruction + offset - 4;	

	debugInterpreter("O_SETFGCODE next %08x, offset %08x", _fgOpcodePC, offset);
}

void Interpreter::O_STOPHERO() {
	uint16 heroId = readScriptFlagValue();
	if (!heroId) {
		_vm->_mainHero->freeOldMove();
	} else if (heroId == 1) {
		_vm->_secondHero->freeOldMove();
	}
	debugInterpreter("O_STOPHERO heroId %d", heroId);
}

void Interpreter::O_ANIMUPDATEOFF() {
	uint16 slotId = readScriptFlagValue();
	_vm->_normAnimList[slotId]._state = 1;
	debugInterpreter("O_ANIMUPDATEOFF slotId %d", slotId);
}

void Interpreter::O_ANIMUPDATEON() {
	uint16 slotId = readScriptFlagValue();
	_vm->_normAnimList[slotId]._state = 0;
	debugInterpreter("O_ANIMUPDATEON slotId %d", slotId);
}

void Interpreter::O_FREECURSOR() {
	_vm->changeCursor(0);
	_vm->_currentPointerNumber = 1;
	// free memory here?
	debugInterpreter("O_FREECURSOR");
}

void Interpreter::O_ADDINVQUIET() {
	uint16 hero = readScriptFlagValue();
	uint16 item = readScriptFlagValue();
	_vm->addInv(hero, item, true);
	debugInterpreter("O_ADDINVQUIET hero %d, item %d", hero, item);
}

void Interpreter::O_RUNHERO() {
	uint16 heroId = readScriptFlagValue();
	uint16 x = readScriptFlagValue();
	uint16 y = readScriptFlagValue();
	uint16 dir = readScriptFlagValue();
	_vm->moveRunHero(heroId, x, y, dir, true);
	debugInterpreter("O_RUNHERO heroId %d, x %d, y %d, dir %d", heroId, x, y, dir);
}

void Interpreter::O_SETBACKANIMDATA() {
	uint16 animNumber = readScript<uint16>();
	uint16 animDataOffset = readScript<uint16>();
	Flags::Id flagId = readScriptFlagId();
	uint16 value = _flags->getFlagValue((Flags::Id)(flagId));
	int currAnim = _vm->_backAnimList[animNumber]._seq._currRelative;
	_vm->_backAnimList[animNumber].backAnims[currAnim].setAnimData((Anim::AnimOffsets)(animDataOffset), value);
	debugInterpreter("O_SETBACKANIMDATA flag %04X (%s), animNumber %d, animDataOffset %d, value %d", flagId, Flags::getFlagName(flagId), animNumber, animDataOffset, value);
}

void Interpreter::O_VIEWFLC() {
	uint16 animNr = readScriptFlagValue();
	debugInterpreter("O_VIEWFLC animNr %d", animNr);
	_vm->loadAnim(animNr, false);
}

void Interpreter::O_CHECKFLCFRAME() {
	uint16 frameNr = readScriptFlagValue();

	debugInterpreter("O_CHECKFLCFRAME frame number %d", frameNr);

	if (_vm->_flicPlayer.getCurFrame() != frameNr) {
		// Move instruction pointer before current instruction
		// must do this check once again till it's false 
		_currentInstruction -= 2;
		_opcodeNF = 1;
	}
}

void Interpreter::O_CHECKFLCEND() {

	//debugInterpreter("O_CHECKFLCEND");

	const Video::FlicDecoder &flicPlayer = _vm->_flicPlayer;

	//debug("frameCount %d, currentFrame %d", flicPlayer.getFrameCount(), flicPlayer.getCurFrame());

	if (flicPlayer.getFrameCount() - flicPlayer.getCurFrame() > 1) {
		// Move instruction pointer before current instruciton
		// must do this check once again till it's false 
		_currentInstruction -= 2;
		_opcodeNF = 1;
	}
}

void Interpreter::O_FREEFLC() {
	debugInterpreter("O_FREEFLC");
}

void Interpreter::O_TALKHEROSTOP() {
	uint16 heroId = readScriptFlagValue();
	if (heroId == 0) {
		_vm->_mainHero->_state = _vm->_mainHero->STAY;
	} else if (heroId == 1) {
		_vm->_secondHero->_state = _vm->_secondHero->STAY;
	}
	debugInterpreter("O_TALKHEROSTOP %d", heroId);
}

// TODO - check this
void Interpreter::O_HEROCOLOR() {
	uint16 heroId = readScriptFlagValue();
	uint16 color = readScriptFlagValue();
	if (heroId == 0) {
		_vm->_mainHero->_color = color;
	} else if (heroId == 1) {
		_vm->_secondHero->_color = color;
	}
	debugInterpreter("O_HEROCOLOR heroId %d, color %d", heroId, color);
}

void Interpreter::O_GRABMAPA() {
	_vm->grabMap();
	debugInterpreter("O_GRABMAPA");
}

void Interpreter::O_ENABLENAK() {
	uint16 nakId = readScriptFlagValue();
	_vm->_maskList[nakId]._flags = 0;
	debugInterpreter("O_ENABLENAK nakId %d", nakId);
}

void Interpreter::O_DISABLENAK() {
	uint16 nakId = readScriptFlagValue();
	_vm->_maskList[nakId]._flags = 1;
	debugInterpreter("O_DISABLENAK nakId %d", nakId);
}

void Interpreter::O_GETMOBNAME() {
	uint16 modId = readScriptFlagValue();
	_string = (byte *)_vm->_mobList[modId]._name.c_str();
	debugInterpreter("O_GETMOBNAME modId %d", modId);
}

void Interpreter::O_SWAPINVENTORY() {
	uint16 hero = readScriptFlagValue();
	_vm->swapInv(hero);
	debugInterpreter("O_SWAPINVENTORY hero %d", hero);
}

void Interpreter::O_CLEARINVENTORY() {
	uint16 hero = readScriptFlagValue();
	_vm->clearInv(hero);
	debugInterpreter("O_CLEARINVENTORY hero %d", hero);
}

void Interpreter::O_SKIPTEXT() {
	increaseString();
	debugInterpreter("O_SKIPTEXT");
}

void Interpreter::O_SETVOICEH() {
	uint16 slot = readScriptFlagValue();
	static const uint32 VOICE_H_SLOT = 28;
	uint16 voiceLineH = _flags->getFlagValue(Flags::VOICE_H_LINE);
	_vm->setVoice(slot, VOICE_H_SLOT, voiceLineH);
}

void Interpreter::O_SETVOICEA() {
	uint16 slot = readScriptFlagValue();
	static const uint32 VOICE_A_SLOT = 29;
	uint16 voiceLineH = _flags->getFlagValue(Flags::VOICE_H_LINE);
	_vm->setVoice(slot, VOICE_A_SLOT, voiceLineH);
}

void Interpreter::O_SETVOICEB() {
	uint16 slot = readScriptFlagValue();
	static const uint32 VOICE_B_SLOT = 30;
	uint16 voiceLineH = _flags->getFlagValue(Flags::VOICE_H_LINE);
	_vm->setVoice(slot, VOICE_B_SLOT, voiceLineH);
}

void Interpreter::O_SETVOICEC() {
	uint16 slot = readScriptFlagValue();
	static const uint32 VOICE_C_SLOT = 31;
	uint16 voiceLineH = _flags->getFlagValue(Flags::VOICE_H_LINE);
	_vm->setVoice(slot, VOICE_C_SLOT, voiceLineH);
}

void Interpreter::O_SETVOICED() {
	uint16 slot = readScriptFlagValue();
	static const uint32 VOICE_D_SLOT = 32;
	uint16 voiceLineH = _flags->getFlagValue(Flags::VOICE_H_LINE);
	_vm->setVoice(slot, VOICE_D_SLOT, voiceLineH);
}

void Interpreter::O_VIEWFLCLOOP() {
	uint16 value = readScriptFlagValue();
	debugInterpreter("O_VIEWFLCLOOP animId %d", value);
	_vm->loadAnim(value, true);
}

void Interpreter::O_FLCSPEED() {
	uint16 speed = readScriptFlagValue();
	debugInterpreter("O_FLCSPEED speed %d", speed);
}

void Interpreter::O_OPENINVENTORY() {
	_vm->_showInventoryFlag = true;
	_opcodeNF = 1;
	debugInterpreter("O_OPENINVENTORY");
}

void Interpreter::O_KRZYWA() {
	debugInterpreter("O_KRZYWA");
}

void Interpreter::O_GETKRZYWA() {
	debugInterpreter("O_GETKRZYWA");
	// _flags->setFlagValue(Flags::TORX1, krzywa[_krzywaIndex++])
	// _flags->setFlagValue(Flags::TORY1, krzywa[_krzywaIndex++])
	// Check _krzywaIndex 
}

void Interpreter::O_GETMOB() {
	Flags::Id flagId = readScriptFlagId();
	uint16 mx = readScriptFlagValue();
	uint16 my = readScriptFlagValue();
	debugInterpreter("O_GETMOB flagId %d, mx %d, my %d", flagId, mx, my);
	// check if current mob pos = (mx, my)
}

void Interpreter::O_INPUTLINE() {
	debugInterpreter("O_INPUTLINE");
}


void Interpreter::O_BREAK_POINT() {
	debugInterpreter("O_BREAK_POINT");
}

Interpreter::OpcodeFunc Interpreter::_opcodes[kNumOpcodes] = {
	&Interpreter::O_WAITFOREVER,
	&Interpreter::O_BLACKPALETTE,
	&Interpreter::O_SETUPPALETTE,
	&Interpreter::O_INITROOM,
	&Interpreter::O_SETSAMPLE,
	&Interpreter::O_FREESAMPLE,
	&Interpreter::O_PLAYSAMPLE,
	&Interpreter::O_PUTOBJECT,
	&Interpreter::O_REMOBJECT,
	&Interpreter::O_SHOWANIM,
	&Interpreter::O_CHECKANIMEND,
	&Interpreter::O_FREEANIM,
	&Interpreter::O_CHECKANIMFRAME,
	&Interpreter::O_PUTBACKANIM,
	&Interpreter::O_REMBACKANIM,
	&Interpreter::O_CHECKBACKANIMFRAME,
	&Interpreter::O_FREEALLSAMPLES,
	&Interpreter::O_SETMUSIC,
	&Interpreter::O_STOPMUSIC,
	&Interpreter::O__WAIT,
	&Interpreter::O_UPDATEOFF,
	&Interpreter::O_UPDATEON,
	&Interpreter::O_UPDATE ,
	&Interpreter::O_CLS,
	&Interpreter::O__CALL,
	&Interpreter::O_RETURN,
	&Interpreter::O_GO,
	&Interpreter::O_BACKANIMUPDATEOFF,
	&Interpreter::O_BACKANIMUPDATEON,
	&Interpreter::O_CHANGECURSOR,
	&Interpreter::O_CHANGEANIMTYPE,
	&Interpreter::O__SETFLAG,
	&Interpreter::O_COMPARE,
	&Interpreter::O_JUMPZ,
	&Interpreter::O_JUMPNZ,
	&Interpreter::O_EXIT,
	&Interpreter::O_ADDFLAG,
	&Interpreter::O_TALKANIM,
	&Interpreter::O_SUBFLAG,
	&Interpreter::O_SETSTRING,
	&Interpreter::O_ANDFLAG,
	&Interpreter::O_GETMOBDATA,
	&Interpreter::O_ORFLAG,
	&Interpreter::O_SETMOBDATA,
	&Interpreter::O_XORFLAG,
	&Interpreter::O_GETMOBTEXT,
	&Interpreter::O_MOVEHERO,
	&Interpreter::O_WALKHERO,
	&Interpreter::O_SETHERO,
	&Interpreter::O_HEROOFF,
	&Interpreter::O_HEROON,
	&Interpreter::O_CLSTEXT,
	&Interpreter::O_CALLTABLE,
	&Interpreter::O_CHANGEMOB,
	&Interpreter::O_ADDINV,
	&Interpreter::O_REMINV,
	&Interpreter::O_REPINV,
	&Interpreter::O_OBSOLETE_GETACTION,
	&Interpreter::O_ADDWALKAREA,
	&Interpreter::O_REMWALKAREA,
	&Interpreter::O_RESTOREWALKAREA,
	&Interpreter::O_WAITFRAME,
	&Interpreter::O_SETFRAME,
	&Interpreter::O_RUNACTION,
	&Interpreter::O_COMPAREHI,
	&Interpreter::O_COMPARELO,
	&Interpreter::O_PRELOADSET,
	&Interpreter::O_FREEPRELOAD,
	&Interpreter::O_CHECKINV,
	&Interpreter::O_TALKHERO,
	&Interpreter::O_WAITTEXT,
	&Interpreter::O_SETHEROANIM,
	&Interpreter::O_WAITHEROANIM,
	&Interpreter::O_GETHERODATA,
	&Interpreter::O_GETMOUSEBUTTON,
	&Interpreter::O_CHANGEFRAMES,
	&Interpreter::O_CHANGEBACKFRAMES,
	&Interpreter::O_GETBACKANIMDATA,
	&Interpreter::O_GETANIMDATA,
	&Interpreter::O_SETBGCODE,
	&Interpreter::O_SETBACKFRAME,
	&Interpreter::O_GETRND,
	&Interpreter::O_TALKBACKANIM,
	&Interpreter::O_LOADPATH,
	&Interpreter::O_GETCHAR,
	&Interpreter::O_SETDFLAG,
	&Interpreter::O_CALLDFLAG,
	&Interpreter::O_PRINTAT,
	&Interpreter::O_ZOOMIN,
	&Interpreter::O_ZOOMOUT,
	&Interpreter::O_SETSTRINGOFFSET,
	&Interpreter::O_GETOBJDATA,
	&Interpreter::O_SETOBJDATA,
	&Interpreter::O_SWAPOBJECTS,
	&Interpreter::O_CHANGEHEROSET,
	&Interpreter::O_ADDSTRING,
	&Interpreter::O_SUBSTRING,
	&Interpreter::O_INITDIALOG,
	&Interpreter::O_ENABLEDIALOGOPT,
	&Interpreter::O_DISABLEDIALOGOPT,
	&Interpreter::O_SHOWDIALOGBOX,
	&Interpreter::O_STOPSAMPLE,
	&Interpreter::O_BACKANIMRANGE,
	&Interpreter::O_CLEARPATH,
	&Interpreter::O_SETPATH,
	&Interpreter::O_GETHEROX,
	&Interpreter::O_GETHEROY,
	&Interpreter::O_GETHEROD,
	&Interpreter::O_PUSHSTRING,
	&Interpreter::O_POPSTRING,
	&Interpreter::O_SETFGCODE,
	&Interpreter::O_STOPHERO,
	&Interpreter::O_ANIMUPDATEOFF,
	&Interpreter::O_ANIMUPDATEON,
	&Interpreter::O_FREECURSOR,
	&Interpreter::O_ADDINVQUIET,
	&Interpreter::O_RUNHERO,
	&Interpreter::O_SETBACKANIMDATA,
	&Interpreter::O_VIEWFLC,
	&Interpreter::O_CHECKFLCFRAME,
	&Interpreter::O_CHECKFLCEND,
	&Interpreter::O_FREEFLC,
	&Interpreter::O_TALKHEROSTOP,
	&Interpreter::O_HEROCOLOR,
	&Interpreter::O_GRABMAPA,
	&Interpreter::O_ENABLENAK,
	&Interpreter::O_DISABLENAK,
	&Interpreter::O_GETMOBNAME,
	&Interpreter::O_SWAPINVENTORY,
	&Interpreter::O_CLEARINVENTORY,
	&Interpreter::O_SKIPTEXT,
	&Interpreter::O_SETVOICEH,
	&Interpreter::O_SETVOICEA,
	&Interpreter::O_SETVOICEB,
	&Interpreter::O_SETVOICEC,
	&Interpreter::O_VIEWFLCLOOP,
	&Interpreter::O_FLCSPEED,
	&Interpreter::O_OPENINVENTORY,
	&Interpreter::O_KRZYWA,
	&Interpreter::O_GETKRZYWA,
	&Interpreter::O_GETMOB,
	&Interpreter::O_INPUTLINE,
	&Interpreter::O_SETVOICED,
	&Interpreter::O_BREAK_POINT,
};

}

/* vim: set tabstop=4 noexpandtab: */
