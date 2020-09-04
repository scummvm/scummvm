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
	if (_data != nullptr) {
		free(_data);
		_dataSize = 0;
		_data = nullptr;
	}
}

bool Script::loadStream(Common::SeekableReadStream &stream) {
	_dataSize = stream.size();
	if (!_dataSize) {
		return false;
	}

	_data = (byte *)malloc(_dataSize);

	if (!_data) {
		return false;
	}

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

uint16 Script::readScript16(uint32 address) {
	assert((_data + address + sizeof(uint16)) <= (_data + _dataSize));
	uint16 data = READ_LE_UINT16(&_data[address]);
	return data;
}

uint32 Script::readScript32(uint32 address) {
	assert((_data + address + sizeof(uint32)) <= (_data + _dataSize));
	uint32 data = READ_LE_UINT32(&_data[address]);
	return data;
}

int16 Script::getLightX(int locationNr) {
	return (int)READ_LE_UINT16(&_data[_scriptInfo.lightSources + locationNr * 8]);
}

int16 Script::getLightY(int locationNr) {
	return (int)READ_LE_UINT16(&_data[_scriptInfo.lightSources + locationNr * 8 + 2]);
}

int32 Script::getShadowScale(int locationNr) {
	return (int)READ_LE_UINT16(&_data[_scriptInfo.lightSources + locationNr * 8 + 4]);
}

uint32 Script::getStartGameOffset() {
	return _scriptInfo.startGame;
}

uint32 Script::getLocationInitScript(int initRoomTableOffset, int roomNr) {
	return (uint32)READ_LE_UINT32(&_data[initRoomTableOffset + roomNr * 4]);
}

byte Script::getMobVisible(int roomMobOffset, uint16 mob) {
	return _data[roomMobOffset + mob];
}

void Script::setMobVisible(int roomMobOffset, uint16 mob, byte value) {
	_data[roomMobOffset + mob] = value;
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

uint32 Script::getBackAnimId(int roomBackAnimOffset, int slot) {
	uint32 animId = READ_LE_UINT32(&_data[roomBackAnimOffset + slot * 4]);
	return animId;
}

void Script::setBackAnimId(int roomBackAnimOffset, int slot, int animId) {
	WRITE_LE_UINT32(&_data[roomBackAnimOffset + slot * 4], animId);
}

byte Script::getObjId(int roomObjOffset, int slot) {
	return _data[roomObjOffset + slot];
}

void Script::setObjId(int roomObjOffset, int slot, byte objectId) {
	_data[roomObjOffset + slot] = objectId;
}

int Script::scanMobEvents(int mobMask, int dataEventOffset) {
	debug(3, "scanMobEvents: mobMask: %d", mobMask);
	int i = 0;
	int16 mob;
	int32 code;
	do {
		mob = (int)READ_LE_UINT16(&_data[dataEventOffset + i * 6]);
		if (mob == mobMask) {
			code = (int)READ_LE_UINT32(&_data[dataEventOffset + i * 6 + 2]);
			debug(3, "scanMobEvents: code: %d", code);
			return code;
		}
		i++;
	} while (mob != -1);
	return -1;
}

int Script::scanMobEventsWithItem(int mobMask, int dataEventOffset, int itemMask) {
	debug(3, "scanMobEventsWithItem: mobMask: %d", mobMask);
	int i = 0;
	int16 mob;
	int16 item;
	int32 code;
	do {
		mob = (int)READ_LE_UINT16(&_data[dataEventOffset + i * 8]);
		if (mob == mobMask) {
			item = (int)READ_LE_UINT16(&_data[dataEventOffset + i * 8 + 2]);
			if (item == itemMask) {
				code = (int)READ_LE_UINT32(&_data[dataEventOffset + i * 8 + 4]);
				debug(3, "scanMobEventsWithItem: itemMask: %d", item);
				debug(3, "scanMobEventsWithItem: code: %d", code);
				return code;
			}
		}
		i++;
	} while (mob != -1);
	return -1;
}

void Script::installSingleBackAnim(Common::Array<BackgroundAnim> &backAnimList, int slot, int roomBackAnimOffset) {

	_vm->removeSingleBackAnim(slot); // free slot before loading

	int offset = roomBackAnimOffset + slot * 4; // BackgroundAnim offset for selected slot number

	BackgroundAnim newBackgroundAnim; // BackgroundAnim seq data and its array of Anim

	int animOffset = READ_LE_UINT32(&_data[offset]); // pos of BackgroundAnim data in script
	int anims = READ_LE_UINT32(&_data[animOffset + 8]); // amount of Anim in BackgroundAnim

	if (anims == 0) {
		anims = 1; // anims with 0 as amount in game data has just 1 animation
	}

	if (animOffset != 0) {
		Common::MemoryReadStream stream(_data, _dataSize); // stream from script data
		for (int i = 0; i < anims; i++) {
			Anim newAnim;
			stream.seek(animOffset + kStructSizeBAS + kStructSizeBASA * i);
			// Anim BASA data
			newAnim._basaData._num = stream.readUint16LE();
			newAnim._basaData._start = stream.readUint16LE();
			newAnim._basaData._end = stream.readUint16LE();

			// Anim number in game files
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
			if ((_vm->_animList[animNumber]._flags & 4)) {
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

		// Anim BAS data
		stream.seek(animOffset);
		newBackgroundAnim._seq._type = stream.readUint32LE();
		newBackgroundAnim._seq._data = stream.readUint32LE();
		newBackgroundAnim._seq._anims = stream.readUint32LE();
		stream.skip(12);
		newBackgroundAnim._seq._current = newBackgroundAnim.backAnims[0]._basaData._num;
		newBackgroundAnim._seq._counter = 0;
		newBackgroundAnim._seq._currRelative = 0;
		newBackgroundAnim._seq._data2 = stream.readUint32LE();

		int start = newBackgroundAnim.backAnims[0]._basaData._start; // BASA_Start of first frame
		int end = newBackgroundAnim.backAnims[0]._basaData._end; // BASA_End of first frame

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

void Script::installBackAnims(Common::Array<BackgroundAnim> &backAnimList, int roomBackAnimOffset) {
	for (int i = 0; i < _vm->kMaxBackAnims; i++) {
		installSingleBackAnim(backAnimList, i, roomBackAnimOffset);
	}
}

void Script::installObjects(int offset) {
	for (int i = 0; i < _vm->kMaxObjects; i++) {
		_vm->_objSlot[i] = _data[offset];
		offset++;
	}
}

bool Script::loadAllMasks(Common::Array<Mask> &maskList, int offset) {
	Mask tempMask;
	while (1) {
		Common::MemoryReadStream maskStream(_data, _dataSize);
		maskStream.seek(offset);
		tempMask._state = maskStream.readUint16LE();
		if (tempMask._state == 0xffff) {
			break;
		}
		tempMask._flags = maskStream.readUint16LE();
		tempMask._x1 = maskStream.readUint16LE();
		tempMask._y1 = maskStream.readUint16LE();
		tempMask._x2 = maskStream.readUint16LE();
		tempMask._y2 = maskStream.readUint16LE();
		tempMask._z = maskStream.readUint16LE();
		tempMask._number = maskStream.readUint16LE();

		const Common::String msStreamName = Common::String::format("MS%02d", tempMask._number);
		Common::SeekableReadStream *msStream = SearchMan.createReadStreamForMember(msStreamName);
		if (!msStream) {
			tempMask._width = 0;
			tempMask._height = 0;
			tempMask._data = nullptr;
			warning("loadAllMasks: Can't load %s", msStreamName.c_str());
			delete msStream;
		} else {
			msStream = Resource::getDecompressedStream(msStream);

			int32 dataSize = msStream->size();
			if (dataSize != -1) {
				tempMask._data = (byte *)malloc(dataSize);
				if (msStream->read(tempMask._data, dataSize) != (uint32)dataSize) {
					free(tempMask._data);
					delete msStream;
					return false;
				}
				delete msStream;

				tempMask._width = tempMask.getWidth();
				tempMask._height = tempMask.getHeight();
			} else {
				return false;
			}
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

void InterpreterFlags::setFlagValue(Flags::Id flagId, int32 value) {
	_flags[(uint32)flagId - kFlagMask] = value;
}

int32 InterpreterFlags::getFlagValue(Flags::Id flagId) {
	return _flags[(uint32)flagId - kFlagMask];
}

Interpreter::Interpreter(PrinceEngine *vm, Script *script, InterpreterFlags *flags) :
	_vm(vm), _script(script), _flags(flags),
	_stacktop(0), _opcodeNF(false), _opcodeEnd(false),
	_waitFlag(0), _result(true) {

	// Initialize the script
	_mode = "fg";
	_fgOpcodePC = _script->getStartGameOffset();
	_bgOpcodePC = 0;

	_currentInstruction = 0;
	_lastOpcode = 0;
	_lastInstruction = 0;

	_string = nullptr;
	_currentString = 0;

	_stringStack.string = nullptr;
	_stringStack.dialogData = nullptr;
	_stringStack.currentString = 0;

	memset(_stringBuf, 1, 1024);
}

void Interpreter::debugInterpreter(const char *s, ...) {
	char buf[STRINGBUFLEN];
	va_list va;
	va_start(va, s);
	vsnprintf(buf, STRINGBUFLEN, s, va);
	va_end(va);

	Common::String str = Common::String::format("@0x%08X: ", _lastInstruction);
	str += Common::String::format("op %04d: ", _lastOpcode);
	if (!strcmp(_mode, "fg")) {
		debug(10, "PrinceEngine::Script %s %s", str.c_str(), buf);
	}
}

void Interpreter::stepBg() {
	if (_bgOpcodePC) {
		_mode = "bg";
		_bgOpcodePC = step(_bgOpcodePC);
	}
}

void Interpreter::stepFg() {
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
		_lastOpcode = readScript16();

		if (_lastOpcode >= kNumOpcodes)
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

	if (_opcodeEnd) {
		_vm->quitGame();
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

void Interpreter::setBgOpcodePC(uint32 value) {
	_bgOpcodePC = value;
}

void Interpreter::setFgOpcodePC(uint32 value) {
	_fgOpcodePC = value;
}

uint16 Interpreter::readScript16() {
	uint16 data = _script->readScript16(_currentInstruction);
	_currentInstruction += sizeof(uint16);
	return data;
}

uint32 Interpreter::readScript32() {
	uint32 data = _script->readScript32(_currentInstruction);
	_currentInstruction += sizeof(uint32);
	return data;
}

int32 Interpreter::readScriptFlagValue() {
	uint16 value = readScript16();
	if (value & InterpreterFlags::kFlagMask) {
		return _flags->getFlagValue((Flags::Id)value);
	}
	return value;
}

Flags::Id Interpreter::readScriptFlagId() {
	return (Flags::Id)readScript16();
}

void Interpreter::O_WAITFOREVER() {
	debugInterpreter("O_WAITFOREVER");
	_vm->changeCursor(_vm->_currentPointerNumber);
	_opcodeNF = 1;
	_currentInstruction -= 2;
}

void Interpreter::O_BLACKPALETTE() {
	debugInterpreter("O_BLACKPALETTE");
	_vm->blackPalette();
}

void Interpreter::O_SETUPPALETTE() {
	debugInterpreter("O_SETUPPALETTE");
	_vm->setPalette(_vm->_roomBmp->getPalette());
}

void Interpreter::O_INITROOM() {
	int32 roomId = readScriptFlagValue();
	debugInterpreter("O_INITROOM %d", roomId);
	_vm->loadLocation(roomId);
	_opcodeNF = 1;
}

void Interpreter::O_SETSAMPLE() {
	int32 sampleId = readScriptFlagValue();
	int32 sampleNameOffset = readScript32();
	const char *sampleName = _script->getString(_currentInstruction + sampleNameOffset - 4);
	debugInterpreter("O_SETSAMPLE %d %s", sampleId, sampleName);
	_vm->loadSample(sampleId, sampleName);
}

void Interpreter::O_FREESAMPLE() {
	int32 sampleId = readScriptFlagValue();
	debugInterpreter("O_FREESAMPLE sampleId: %d", sampleId);
	_vm->freeSample(sampleId);
}

void Interpreter::O_PLAYSAMPLE() {
	int32 sampleId = readScriptFlagValue();
	uint16 loopType = readScript16();
	debugInterpreter("O_PLAYSAMPLE sampleId %d loopType %d", sampleId, loopType);
	_vm->playSample(sampleId, loopType);
}

void Interpreter::O_PUTOBJECT() {
	int32 roomId = readScriptFlagValue();
	int32 slot = readScriptFlagValue();
	int32 objectId = readScriptFlagValue();
	debugInterpreter("O_PUTOBJECT roomId %d, slot %d, objectId %d", roomId, slot, objectId);
	Room *room = new Room();
	room->loadRoom(_script->getRoomOffset(roomId));
	_vm->_script->setObjId(room->_obj, slot, objectId);
	if (_vm->_locationNr == roomId) {
		_vm->_objSlot[slot] = objectId;
	}
	delete room;
}

void Interpreter::O_REMOBJECT() {
	int32 roomId = readScriptFlagValue();
	int32 slot = readScriptFlagValue();
	debugInterpreter("O_REMOBJECT roomId %d slot %d", roomId, slot);
	Room *room = new Room();
	room->loadRoom(_script->getRoomOffset(roomId));
	_vm->_script->setObjId(room->_obj, slot, 0xFF);
	if (_vm->_locationNr == roomId) {
		_vm->_objSlot[slot] = 0xFF;
	}
	delete room;
}

void Interpreter::O_SHOWANIM() {
	int32 slot = readScriptFlagValue();
	int32 animId = readScriptFlagValue();
	debugInterpreter("O_SHOWANIM slot %d, animId %d", slot, animId);
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

	// WALKAROUND: fix for turning off bard's wife background animation
	// in front of bard's house (location 7) after giving her poem (item 33)
	// in script: GiveLetter (line 11082)
	if (_currentInstruction == kGiveLetterScriptFix) {
		_vm->_backAnimList[1].backAnims[0]._state = 1;
	}
}

void Interpreter::O_CHECKANIMEND() {
	int32 slot = readScriptFlagValue();
	debugInterpreter("O_CHECKANIMEND slot %d", slot);
	if (_vm->_normAnimList[slot]._frame != _vm->_normAnimList[slot]._lastFrame - 1) {
		_currentInstruction -= 4;
		_opcodeNF = 1;
	}
}

void Interpreter::O_FREEANIM() {
	int32 slot = readScriptFlagValue();
	debugInterpreter("O_FREEANIM slot %d", slot);
	_vm->freeNormAnim(slot);
}

void Interpreter::O_CHECKANIMFRAME() {
	int32 slot = readScriptFlagValue();
	int32 frameNumber = readScriptFlagValue();
	debugInterpreter("O_CHECKANIMFRAME slot %d, frameNumber %d", slot, frameNumber);
	if (_vm->_normAnimList[slot]._frame != frameNumber - 1) {
		_currentInstruction -= 6;
		_opcodeNF = 1;
	}
}

void Interpreter::O_PUTBACKANIM() {
	int32 roomId = readScriptFlagValue();
	int32 slot = readScriptFlagValue();
	int32 animId = readScript32();
	debugInterpreter("O_PUTBACKANIM roomId %d, slot %d, animId %d", roomId, slot, animId);
	Room *room = new Room();
	room->loadRoom(_script->getRoomOffset(roomId));
	_vm->_script->setBackAnimId(room->_backAnim, slot, animId);
	if (_vm->_locationNr == roomId) {
		_vm->_script->installSingleBackAnim(_vm->_backAnimList, slot, room->_backAnim);
	}
	delete room;

	// WALKAROUND: fix for turning on 'walking bird' background animation too soon,
	// after completing 'throw a rock' mini-game in Silmaniona location.
	// Second bird shouldn't appear when normal animation is still in use
	// in script lines 13814 and 13848
	if (_currentInstruction == kSecondBirdAnimationScriptFix) {
		if (_vm->_normAnimList[1]._state == 0) {
			_vm->_backAnimList[0].backAnims[0]._state = 1;
		}
	}
}

void Interpreter::O_REMBACKANIM() {
	int32 roomId = readScriptFlagValue();
	int32 slot = readScriptFlagValue();
	debugInterpreter("O_REMBACKANIM roomId %d, slot %d", roomId, slot);
	if (_vm->_locationNr == roomId) {
		_vm->removeSingleBackAnim(slot);
	}
	Room *room = new Room();
	room->loadRoom(_script->getRoomOffset(roomId));
	_vm->_script->setBackAnimId(room->_backAnim, slot, 0);
	delete room;
}

void Interpreter::O_CHECKBACKANIMFRAME() {
	int32 slotId = readScriptFlagValue();
	int32 frameId = readScriptFlagValue();
	debugInterpreter("O_CHECKBACKANIMFRAME slotId %d, frameId %d", slotId, frameId);
	int currAnim = _vm->_backAnimList[slotId]._seq._currRelative;
	if (_vm->_backAnimList[slotId].backAnims[currAnim]._frame != frameId - 1) {
		_currentInstruction -= 6;
		_opcodeNF = 1;
	}
}

// Not used in script
void Interpreter::O_FREEALLSAMPLES() {
	error("O_FREEALLSAMPLES");
}

void Interpreter::O_SETMUSIC() {
	uint16 musicId = readScript16();
	debugInterpreter("O_SETMUSIC musicId %d", musicId);
	_vm->loadMusic(musicId);
}

void Interpreter::O_STOPMUSIC() {
	debugInterpreter("O_STOPMUSIC");
	_vm->stopMusic();
}

void Interpreter::O__WAIT() {
	int32 pause = readScriptFlagValue();
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

// Not used in script
void Interpreter::O_UPDATEOFF() {
	error("O_UPDATEOFF");
}

// Not used in script
void Interpreter::O_UPDATEON() {
	error("O_UPDATEON");
}

// Not used in script
void Interpreter::O_UPDATE () {
	error("O_UPDATE");
}

// Not used in script
void Interpreter::O_CLS() {
	error("O_CLS");
}

void Interpreter::O__CALL() {
	int32 address = readScript32();
	debugInterpreter("O__CALL 0x%04X", _currentInstruction);
	_stack[_stacktop] = _currentInstruction;
	_stacktop++;
	_currentInstruction += address - 4;
}

void Interpreter::O_RETURN() {
	debugInterpreter("O_RETURN 0x%04X", _currentInstruction);
	if (_stacktop > 0) {
		_stacktop--;
		_currentInstruction = _stack[_stacktop];
	} else {
		error("O_RETURN: Stack is empty");
	}
}

void Interpreter::O_GO() {
	int32 opPC = readScript32();
	debugInterpreter("O_GO 0x%04X", opPC);
	_currentInstruction += opPC - 4;
}

void Interpreter::O_BACKANIMUPDATEOFF() {
	int32 slotId = readScriptFlagValue();
	debugInterpreter("O_BACKANIMUPDATEOFF slotId %d", slotId);
	int currAnim = _vm->_backAnimList[slotId]._seq._currRelative;
	if (!_vm->_backAnimList[slotId].backAnims.empty()) {
		_vm->_backAnimList[slotId].backAnims[currAnim]._state = 1;
	}
}

void Interpreter::O_BACKANIMUPDATEON() {
	int32 slotId = readScriptFlagValue();
	debugInterpreter("O_BACKANIMUPDATEON slotId %d", slotId);
	int currAnim = _vm->_backAnimList[slotId]._seq._currRelative;
	if (!_vm->_backAnimList[slotId].backAnims.empty()) {
		_vm->_backAnimList[slotId].backAnims[currAnim]._state = 0;
	}
}

void Interpreter::O_CHANGECURSOR() {
	int32 cursorId = readScriptFlagValue();
	debugInterpreter("O_CHANGECURSOR %x", cursorId);
	_vm->changeCursor(cursorId);
}

// Not used in script
void Interpreter::O_CHANGEANIMTYPE() {
	error("O_CHANGEANIMTYPE");
}

void Interpreter::O__SETFLAG() {
	Flags::Id flagId = readScriptFlagId();
	int32 value = readScriptFlagValue();
	debugInterpreter("O__SETFLAG 0x%04X (%s) = %d", flagId, _flagMap.getFlagName(flagId), value);
	_flags->setFlagValue((Flags::Id)(flagId), value);
}

void Interpreter::O_COMPARE() {
	Flags::Id flagId = readScriptFlagId();
	int32 value = readScriptFlagValue();
	_result = _flags->getFlagValue(flagId) != value;
	debugInterpreter("O_COMPARE flagId 0x%04X (%s), value %d == %d (%d)", flagId, _flagMap.getFlagName(flagId), value, _flags->getFlagValue(flagId), _result);
}

void Interpreter::O_JUMPZ() {
	int32 offset = readScript32();
	if (!_result) {
		_currentInstruction += offset - 4;
	}
	debugInterpreter("O_JUMPZ result = %d, next %08x, offset 0x%08X", _result, _currentInstruction, offset);
}

void Interpreter::O_JUMPNZ() {
	int32 offset = readScript32();
	if (_result) {
		_currentInstruction += offset - 4;
	}
	debugInterpreter("O_JUMPNZ result = %d, next %08x, offset 0x%08X", _result, _currentInstruction, offset);
}

void Interpreter::O_EXIT() {
	int32 exitCode = readScriptFlagValue();
	debugInterpreter("O_EXIT exitCode %d", exitCode);
	_opcodeEnd = true;
	_opcodeNF = 1;
	if (exitCode == 0x2EAD) {
		_vm->scrollCredits();
	}
}

void Interpreter::O_ADDFLAG() {
	Flags::Id flagId = readScriptFlagId();
	int32 value = readScriptFlagValue();
	debugInterpreter("O_ADDFLAG flagId %04x (%s), value %d", flagId, _flagMap.getFlagName(flagId), value);
	_flags->setFlagValue(flagId, _flags->getFlagValue(flagId) + value);
	if (_flags->getFlagValue(flagId)) {
		_result = 1;
	} else {
		_result = 0;
	}
}

void Interpreter::O_TALKANIM() {
	int32 animNumber = readScriptFlagValue();
	int32 slot = readScriptFlagValue();
	debugInterpreter("O_TALKANIM animNumber %d, slot %d", animNumber, slot);
	_vm->doTalkAnim(animNumber, slot, kNormalAnimation);
}

void Interpreter::O_SUBFLAG() {
	Flags::Id flagId = readScriptFlagId();
	int32 value = readScriptFlagValue();
	debugInterpreter("O_SUBFLAG flagId %d, value %d", flagId, value);
	_flags->setFlagValue(flagId, _flags->getFlagValue(flagId) - value);
	if (_flags->getFlagValue(flagId)) {
		_result = 1;
	} else {
		_result = 0;
	}
}

void Interpreter::O_SETSTRING() {
	int32 offset = readScript32();
	debugInterpreter("O_SETSTRING %04d", offset);
	_currentString = offset;
	if (offset >= 80000) {
		_string = _vm->_variaTxt->getString(offset - 80000);
		debugInterpreter("GetVaria %s", _string);
	} else if (offset < 2000) {
		_vm->_dialogData = &_vm->_dialogDat[offset * 4 - 4];
		uint32 of = READ_LE_UINT32(_vm->_talkTxt + offset * 4);
		const char *txt = (const char *)&_vm->_talkTxt[of];
		_string = &_vm->_talkTxt[of];
		debugInterpreter("TalkTxt %d %s", of, txt);
	}
}

void Interpreter::O_ANDFLAG() {
	Flags::Id flagId = readScriptFlagId();
	int32 value = readScriptFlagValue();
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
	int32 mobId = readScriptFlagValue();
	int32 mobOffset = readScriptFlagValue();
	debugInterpreter("O_GETMOBDATA flagId %d, modId %d, mobOffset %d", flagId, mobId, mobOffset);
	int16 value = _vm->_mobList[mobId].getData((Mob::AttrId)mobOffset);
	_flags->setFlagValue(flagId, value);
}

void Interpreter::O_ORFLAG() {
	Flags::Id flagId = readScriptFlagId();
	int32 value = readScriptFlagValue();
	debugInterpreter("O_ORFLAG flagId %d, value %d", flagId, value);
	_flags->setFlagValue(flagId, _flags->getFlagValue(flagId) | value);
	if (_flags->getFlagValue(flagId)) {
		_result = 1;
	} else {
		_result = 0;
	}
}

void Interpreter::O_SETMOBDATA() {
	int32 mobId = readScriptFlagValue();
	int32 mobOffset = readScriptFlagValue();
	int32 value = readScriptFlagValue();
	debugInterpreter("O_SETMOBDATA mobId %d, mobOffset %d, value %d", mobId, mobOffset, value);
	_vm->_mobList[mobId].setData((Mob::AttrId)mobOffset, value);
}

void Interpreter::O_XORFLAG() {
	Flags::Id flagId = readScriptFlagId();
	int32 value = readScriptFlagValue();
	debugInterpreter("O_XORFLAG flagId %d, value %d", flagId, value);
	_flags->setFlagValue(flagId, _flags->getFlagValue(flagId) ^ value);
	if (_flags->getFlagValue(flagId)) {
		_result = 1;
	} else {
		_result = 0;
	}
}

void Interpreter::O_GETMOBTEXT() {
	int32 mob = readScriptFlagValue();
	debugInterpreter("O_GETMOBTEXT mob %d", mob);
	_currentString = _vm->_locationNr * 100 + mob + 60001;
	strncpy((char *)_stringBuf, _vm->_mobList[mob]._examText.c_str(), 1023);
	_string = _stringBuf;
}

void Interpreter::O_MOVEHERO() {
	int32 heroId = readScriptFlagValue();
	int32 x = readScriptFlagValue();
	int32 y = readScriptFlagValue();
	int32 dir = readScriptFlagValue();
	debugInterpreter("O_MOVEHERO heroId %d, x %d, y %d, dir %d", heroId, x, y, dir);
	_vm->moveRunHero(heroId, x, y, dir, false);
}

void Interpreter::O_WALKHERO() {
	int32 heroId = readScriptFlagValue();
	debugInterpreter("O_WALKHERO %d", heroId);
	Hero *hero = nullptr;
	if (!heroId) {
		hero = _vm->_mainHero;
	} else if (heroId == 1) {
		hero = _vm->_secondHero;
	}
	if (hero != nullptr) {
		if (hero->_state != Hero::kHeroStateStay) {
			_currentInstruction -= 4;
			_opcodeNF = 1;
		}
	}
}

void Interpreter::O_SETHERO() {
	int32 heroId = readScriptFlagValue();
	int32 x = readScriptFlagValue();
	int32 y = readScriptFlagValue();
	int32 dir = readScriptFlagValue();
	debugInterpreter("O_SETHERO heroId %d, x %d, y %d, dir %d", heroId, x, y, dir);
	Hero *hero = nullptr;
	if (!heroId) {
		hero = _vm->_mainHero;
	} else if (heroId == 1) {
		hero = _vm->_secondHero;
	}
	if (hero != nullptr) {
		hero->setPos(x, y);
		hero->_lastDirection = dir;
		hero->_visible = 1;
		hero->countDrawPosition();
	}
}

void Interpreter::O_HEROOFF() {
	int32 heroId = readScriptFlagValue();
	debugInterpreter("O_HEROOFF %d", heroId);
	Hero *hero = nullptr;
	if (!heroId) {
		hero = _vm->_mainHero;
	} else if (heroId == 1) {
		hero = _vm->_secondHero;
	}
	if (hero != nullptr) {
		hero->setVisible(false);
	}
}

void Interpreter::O_HEROON() {
	int32 heroId = readScriptFlagValue();
	debugInterpreter("O_HEROON %d", heroId);
	Hero *hero = nullptr;
	if (!heroId) {
		hero = _vm->_mainHero;
	} else if (heroId == 1) {
		hero = _vm->_secondHero;
	}
	if (hero != nullptr) {
		hero->setVisible(true);
	}
}

void Interpreter::O_CLSTEXT() {
	int32 slot = readScriptFlagValue();
	debugInterpreter("O_CLSTEXT slot %d", slot);
	_vm->_textSlots[slot]._str = nullptr;
	_vm->_textSlots[slot]._time = 0;
}

void Interpreter::O_CALLTABLE() {
	Flags::Id flagId = readScriptFlagId();
	int roomNr = _flags->getFlagValue(flagId);
	debugInterpreter("O_CALLTABLE loc %d", roomNr);
	int32 tableOffset = readScript32();
	int initLocationScript = _script->getLocationInitScript(tableOffset, roomNr);
	if (initLocationScript) {
		_stack[_stacktop] = _currentInstruction;
		_stacktop++;
		_currentInstruction = initLocationScript;
	}
}

void Interpreter::O_CHANGEMOB() {
	int32 mob = readScriptFlagValue();
	int32 value = readScriptFlagValue();
	value ^= 1;
	debugInterpreter("O_CHANGEMOB mob %d, value %d", mob, value);
	_vm->_script->setMobVisible(_vm->_room->_mobs, mob, value);
	_vm->_mobList[mob]._visible = value;
}

void Interpreter::O_ADDINV() {
	int32 hero = readScriptFlagValue();
	int32 item = readScriptFlagValue();
	debugInterpreter("O_ADDINV hero %d, item %d", hero, item);
	_vm->addInv(hero, item, false);
}

void Interpreter::O_REMINV() {
	int32 hero = readScriptFlagValue();
	int32 item = readScriptFlagValue();
	debugInterpreter("O_REMINV hero %d, item %d", hero, item);
	_vm->remInv(hero, item);
}

// Not used in script
void Interpreter::O_REPINV() {
	error("O_REPINV");
}

// Not used in script
void Interpreter::O_OBSOLETE_GETACTION() {
	error("O_OBSOLETE_GETACTION");
}

// Not used in script
void Interpreter::O_ADDWALKAREA() {
	error("O_ADDWALKAREA");
}

// Not used in script
void Interpreter::O_REMWALKAREA() {
	error("O_REMWALKAREA");
}

 // Not used in script
void Interpreter::O_RESTOREWALKAREA() {
	error("O_RESTOREWALKAREA");
}

void Interpreter::O_WAITFRAME() {
	debugInterpreter("O_WAITFRAME");
	_opcodeNF = true;
}

void Interpreter::O_SETFRAME() {
	int32 anim = readScriptFlagValue();
	int32 frame = readScriptFlagValue();
	debugInterpreter("O_SETFRAME anim %d, frame %d", anim, frame);
	_vm->_normAnimList[anim]._frame = frame;
}

// Not used in script
void Interpreter::O_RUNACTION() {
	error("O_RUNACTION");
}

void Interpreter::O_COMPAREHI() {
	Flags::Id flag = readScriptFlagId();
	int32 value = readScriptFlagValue();
	int32 flagValue = _flags->getFlagValue(flag);
	if (flagValue > value) {
		_result = 0;
	} else {
		_result = 1;
	}
	debugInterpreter("O_COMPAREHI flag %04x - (%s), value %d, flagValue %d, result %d", flag, _flagMap.getFlagName(flag), value, flagValue, _result);
}

void Interpreter::O_COMPARELO() {
	Flags::Id flag = readScriptFlagId();
	int32 value = readScriptFlagValue();
	int32 flagValue = _flags->getFlagValue(flag);
	if (flagValue < value) {
		_result = 0;
	} else {
		_result = 1;
	}
	debugInterpreter("O_COMPARELO flag %04x - (%s), value %d, flagValue %d, result %d", flag, _flagMap.getFlagName(flag), value, flagValue, _result);
}

// Not used in script
void Interpreter::O_PRELOADSET() {
	error("O_PRELOADSET");
}

// Not used in script
void Interpreter::O_FREEPRELOAD() {
	error("O_FREEPRELOAD");
}

// Not used in script
void Interpreter::O_CHECKINV() {
	error("O_CHECKINV");
}

void Interpreter::O_TALKHERO() {
	int32 hero = readScriptFlagValue();
	debugInterpreter("O_TALKHERO hero %d", hero);
	_vm->talkHero(hero);
}

void Interpreter::O_WAITTEXT() {
	int32 slot = readScriptFlagValue();
	debugInterpreter("O_WAITTEXT slot %d", slot);
	Text &text = _vm->_textSlots[slot];
	if (text._time && text._str) {
		if (_flags->getFlagValue(Flags::ESCAPED)) {
			text._time = 1;
			if (!slot) {
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
	int32 heroId = readScriptFlagValue();
	int32 offset = readScript32();
	debugInterpreter("O_SETHEROANIM hero %d, offset %d", heroId, offset);
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
			hero->_state = Hero::kHeroStateSpec;
		}
	}
}

void Interpreter::O_WAITHEROANIM() {
	int32 heroId = readScriptFlagValue();
	debugInterpreter("O_WAITHEROANIM heroId %d", heroId);
	Hero *hero = nullptr;
	if (!heroId) {
		hero = _vm->_mainHero;
	} else {
		hero = _vm->_secondHero;
	}
	if (hero != nullptr) {
		if (hero->_state == Hero::kHeroStateSpec) {
			_currentInstruction -= 4;
			_opcodeNF = 1;
		}
	}
}

void Interpreter::O_GETHERODATA() {
	Flags::Id flagId = readScriptFlagId();
	int32 heroId = readScriptFlagValue();
	int32 heroOffset = readScriptFlagValue();
	debugInterpreter("O_GETHERODATA flag %04x - (%s), heroId %d, heroOffset %d", flagId, _flagMap.getFlagName(flagId), heroId, heroOffset);
	Hero *hero = nullptr;
	if (!heroId) {
		hero = _vm->_mainHero;
	} else {
		hero = _vm->_secondHero;
	}
	if (hero != nullptr) {
		_flags->setFlagValue(flagId, hero->getData((Hero::AttrId)heroOffset));
	}
}

// No need of implementation here
void Interpreter::O_GETMOUSEBUTTON() {
	debugInterpreter("O_GETMOUSEBUTTON");
}

void Interpreter::O_CHANGEFRAMES() {
	int32 anim = readScriptFlagValue();
	int32 frame = readScriptFlagValue();
	int32 lastFrame = readScriptFlagValue();
	int32 loopFrame = readScriptFlagValue();
	debugInterpreter("O_CHANGFRAMES anim %d, frame %d, lastFrame %d, loopFrame %d", anim, frame, lastFrame, loopFrame);
	_vm->_normAnimList[anim]._frame = frame;
	_vm->_normAnimList[anim]._lastFrame = lastFrame;
	_vm->_normAnimList[anim]._loopFrame = loopFrame;
}

void Interpreter::O_CHANGEBACKFRAMES() {
	int32 anim = readScriptFlagValue();
	int32 frame = readScriptFlagValue();
	int32 lastFrame = readScriptFlagValue();
	int32 loopFrame = readScriptFlagValue();
	debugInterpreter("O_CHANGEBACKFRAMES anim %d, frame %d, lastFrame %d, loopFrame %d", anim, frame, lastFrame, loopFrame);
	int currAnim = _vm->_backAnimList[anim]._seq._currRelative;
	Anim &backAnim = _vm->_backAnimList[anim].backAnims[currAnim];
	backAnim._frame = frame;
	backAnim._lastFrame = lastFrame;
	backAnim._loopFrame = loopFrame;
}

void Interpreter::O_GETBACKANIMDATA() {
	Flags::Id flagId = readScriptFlagId();
	int32 animNumber = readScriptFlagValue();
	int32 animDataOffset = readScriptFlagValue();
	int currAnim = _vm->_backAnimList[animNumber]._seq._currRelative;
	int16 value = _vm->_backAnimList[animNumber].backAnims[currAnim].getAnimData((Anim::AnimOffsets)(animDataOffset));
	debugInterpreter("O_GETBACKANIMDATA flag %04X (%s), animNumber %d, animDataOffset %d, value %d", flagId, _flagMap.getFlagName(flagId), animNumber, animDataOffset, value);
	_flags->setFlagValue((Flags::Id)(flagId), value);
}

void Interpreter::O_GETANIMDATA() {
	Flags::Id flagId = readScriptFlagId();
	int32 anim = readScriptFlagValue();
	int32 animOffset = readScriptFlagValue();
	debugInterpreter("O_GETANIMDATA flag %04X (%s), anim %d, animOffset %d", flagId, _flagMap.getFlagName(flagId), anim, animOffset);
	if (_vm->_normAnimList[anim]._animData != nullptr) {
		_flags->setFlagValue(flagId, _vm->_normAnimList[anim].getAnimData((Anim::AnimOffsets)(animOffset)));
	}
}

void Interpreter::O_SETBGCODE() {
	int32 offset = readScript32();
	_bgOpcodePC = _currentInstruction + offset - 4;
	debugInterpreter("O_SETBGCODE next %08x, offset %08x", _bgOpcodePC, offset);
}

void Interpreter::O_SETBACKFRAME() {
	int32 anim = readScriptFlagValue();
	int32 frame = readScriptFlagValue();
	debugInterpreter("O_SETBACKFRAME anim %d, frame %d", anim, frame);
	int currAnim = _vm->_backAnimList[anim]._seq._currRelative;
	if (_vm->_backAnimList[anim].backAnims[currAnim]._animData != nullptr) {
		_vm->_backAnimList[anim].backAnims[currAnim]._frame = frame;
	}
}

void Interpreter::O_GETRND() {
	Flags::Id flag = readScriptFlagId();
	uint16 rndSeed = readScript16();
	int value = _vm->_randomSource.getRandomNumber(rndSeed - 1);
	debugInterpreter("O_GETRND flag %d, rndSeed %d, value %d", flag, rndSeed, value);
	_flags->setFlagValue(flag, value);
}

void Interpreter::O_TALKBACKANIM() {
	int32 animNumber = readScriptFlagValue();
	int32 slot = readScriptFlagValue();
	debugInterpreter("O_TALKBACKANIM animNumber %d, slot %d", animNumber, slot);
	_vm->doTalkAnim(animNumber, slot, kBackgroundAnimation);
}

// Simplifying, because used only once in Location 20
void Interpreter::O_LOADPATH() {
	readScript32();
	debugInterpreter("O_LOADPATH - path2");
	_vm->loadPath("path2");
}

void Interpreter::O_GETCHAR() {
	Flags::Id flagId = readScriptFlagId();
	debugInterpreter("O_GETCHAR %04X (%s) %02x", flagId, _flagMap.getFlagName(flagId), _flags->getFlagValue(flagId));
	_flags->setFlagValue(flagId, *_string);
	_string++;

	if (_vm->_missingVoice) {	// Sometimes data is missing the END tag, insert it here
		_flags->setFlagValue(flagId, 255);
		_vm->_missingVoice = false;
	}
}

void Interpreter::O_SETDFLAG() {
	Flags::Id flagId = readScriptFlagId();
	int32 address = readScript32();
	debugInterpreter("O_SETDFLAG 0x%04X (%s) = 0x%04X", flagId, _flagMap.getFlagName(flagId), _currentInstruction + address - 4);
	_flags->setFlagValue((Flags::Id)(flagId), _currentInstruction + address - 4);
}

void Interpreter::O_CALLDFLAG() {
	Flags::Id flagId = readScriptFlagId();
	_stack[_stacktop] = _currentInstruction;
	_stacktop++;
	_currentInstruction = _flags->getFlagValue(flagId);
	debugInterpreter("O_CALLDFLAG 0x%04X (%s) = 0x%04X", flagId, _flagMap.getFlagName(flagId), _currentInstruction);
}

void Interpreter::O_PRINTAT() {
	int32 slot = readScriptFlagValue();
	int32 x = readScriptFlagValue();
	int32 y = readScriptFlagValue();
	debugInterpreter("O_PRINTAT slot %d, x %d, y %d", slot, x, y);
	int32 color = _flags->getFlagValue(Flags::KOLOR);
	_vm->printAt(slot, color, (char *)_string, x, y);
	increaseString();
}

void Interpreter::O_ZOOMIN() {
	int32 slot = readScriptFlagValue();
	debugInterpreter("O_ZOOMIN slot %04d", slot);
	_vm->initZoomIn(slot);
}

void Interpreter::O_ZOOMOUT() {
	int32 slot = readScriptFlagValue();
	debugInterpreter("O_ZOOMOUT slot %d", slot);
	_vm->initZoomOut(slot);
}

// Not used in script
void Interpreter::O_SETSTRINGOFFSET() {
	error("O_SETSTRINGOFFSET");
}

void Interpreter::O_GETOBJDATA() {
	Flags::Id flag = readScriptFlagId();
	int32 slot = readScriptFlagValue();
	int32 objOffset = readScriptFlagValue();
	debugInterpreter("O_GETOBJDATA flag %d, objSlot %d, objOffset %d", flag, slot, objOffset);
	int nr = _vm->_objSlot[slot];
	if (nr != 0xFF) {
		int16 value = _vm->_objList[nr]->getData((Object::AttrId)objOffset);
		_flags->setFlagValue(flag, value);
	}
}

void Interpreter::O_SETOBJDATA() {
	int32 slot = readScriptFlagValue();
	int32 objOffset = readScriptFlagValue();
	int32 value = readScriptFlagValue();
	debugInterpreter("O_SETOBJDATA objSlot %d, objOffset %d, value %d", slot, objOffset, value);
	int nr = _vm->_objSlot[slot];
	if (nr != 0xFF) {
		_vm->_objList[nr]->setData((Object::AttrId)objOffset, value);
	}
}

// Not used in script
void Interpreter::O_SWAPOBJECTS() {
	error("O_SWAPOBJECTS");
}

void Interpreter::O_CHANGEHEROSET() {
	int32 heroId = readScriptFlagValue();
	int32 heroSet = readScriptFlagValue();
	debugInterpreter("O_CHANGEHEROSET hero %d, heroSet %d", heroId, heroSet);
	if (!heroId) {
		_vm->_mainHero->loadAnimSet(heroSet);
	} else if (heroId == 1) {
		_vm->_secondHero->loadAnimSet(heroSet);
	}
}

// Not used in script
void Interpreter::O_ADDSTRING() {
	error("O_ADDSTRING");
}

void Interpreter::O_SUBSTRING() {
	int32 value = readScriptFlagValue();
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
		int32 adressOfFirstSequence = (int)READ_LE_UINT16(stringCurrOff);
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
		while ((off = (int)READ_LE_UINT16(stringCurrOff)) != -1) {
			stringCurrOff += 2;
			if (off) {
				line = string + off;
			}
			_vm->_dialogBoxAddr[dialogBox] = line;
			dialogBox++;
		}
		stringCurrOff += 2;

		int dialogOpt = 0;
		while ((off = (int)READ_LE_UINT16(stringCurrOff)) != -1) {
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
	int32 opt = readScriptFlagValue();
	debugInterpreter("O_ENABLEDIALOGOPT opt %d", opt);
	int dialogDataValue = (int)READ_LE_UINT32(_vm->_dialogData);
	dialogDataValue &= ~(1u << opt);
	WRITE_LE_UINT32(_vm->_dialogData, dialogDataValue);
}

void Interpreter::O_DISABLEDIALOGOPT() {
	int32 opt = readScriptFlagValue();
	debugInterpreter("O_DISABLEDIALOGOPT opt %d", opt);
	int dialogDataValue = (int)READ_LE_UINT32(_vm->_dialogData);
	dialogDataValue |= (1u << opt);
	WRITE_LE_UINT32(_vm->_dialogData, dialogDataValue);
}

void Interpreter::O_SHOWDIALOGBOX() {
	int32 box = readScriptFlagValue();
	debugInterpreter("O_SHOWDIALOGBOX box %d", box);
	uint32 currInstr = _currentInstruction;
	_vm->createDialogBox(box);
	_flags->setFlagValue(Flags::DIALINES, _vm->_dialogLines);
	if (_vm->_dialogLines) {
		_vm->changeCursor(1);
		_vm->dialogRun();
		_vm->changeCursor(0);
	}
	_currentInstruction = currInstr;
}

void Interpreter::O_STOPSAMPLE() {
	int32 slot = readScriptFlagValue();
	debugInterpreter("O_STOPSAMPLE slot %d", slot);
	_vm->stopSample(slot);
}

void Interpreter::O_BACKANIMRANGE() {
	int32 slotId = readScriptFlagValue();
	uint16 animId = readScript16();
	int32 low = readScriptFlagValue();
	int32 high = readScriptFlagValue();
	if (animId != 0xFFFF) {
		if (animId & InterpreterFlags::kFlagMask) {
			animId = _flags->getFlagValue((Flags::Id)animId);
		}
	}
	_result = 1;
	if (!_vm->_backAnimList[slotId].backAnims.empty()) {
		int currAnim = _vm->_backAnimList[slotId]._seq._currRelative;
		if (_vm->_backAnimList[slotId].backAnims[currAnim]._animData != nullptr) {
			if (animId == 0xFFFF || _vm->_backAnimList[slotId]._seq._current == animId) {
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
	debugInterpreter("O_CLEARPATH");
	for (uint i = 0; i < _vm->kPathBitmapLen; i++) {
		_vm->_roomPathBitmap[i] = 255;
	}
}

void Interpreter::O_SETPATH() {
	debugInterpreter("O_SETPATH");
	_vm->loadPath("path");
}

void Interpreter::O_GETHEROX() {
	int32 heroId = readScriptFlagValue();
	Flags::Id flagId = readScriptFlagId();
	debugInterpreter("O_GETHEROX heroId %d, flagId %d", heroId, flagId);
	if (!heroId) {
		_flags->setFlagValue(flagId, _vm->_mainHero->_middleX);
	} else if (heroId == 1) {
		_flags->setFlagValue(flagId, _vm->_secondHero->_middleX);
	}
}

void Interpreter::O_GETHEROY() {
	int32 heroId = readScriptFlagValue();
	Flags::Id flagId = readScriptFlagId();
	debugInterpreter("O_GETHEROY heroId %d, flagId %d", heroId, flagId);
	if (!heroId) {
		_flags->setFlagValue(flagId, _vm->_mainHero->_middleY);
	} else if (heroId == 1) {
		_flags->setFlagValue(flagId, _vm->_secondHero->_middleY);
	}
}

void Interpreter::O_GETHEROD() {
	int32 heroId = readScriptFlagValue();
	Flags::Id flagId = readScriptFlagId();
	debugInterpreter("O_GETHEROD heroId %d, flagId %d", heroId, flagId);
	if (!heroId) {
		_flags->setFlagValue(flagId, _vm->_mainHero->_lastDirection);
	} else if (heroId == 1) {
		_flags->setFlagValue(flagId, _vm->_secondHero->_lastDirection);
	}
}

void Interpreter::O_PUSHSTRING() {
	debugInterpreter("O_PUSHSTRING");
	_stringStack.string = _string;
	_stringStack.dialogData = _vm->_dialogData;
	_stringStack.currentString = _currentString;
}

void Interpreter::O_POPSTRING() {
	debugInterpreter("O_POPSTRING");
	_string = _stringStack.string;
	_vm->_dialogData = _stringStack.dialogData;
	_currentString = _stringStack.currentString;
}

void Interpreter::O_SETFGCODE() {
	int32 offset = readScript32();
	_fgOpcodePC = _currentInstruction + offset - 4;
	debugInterpreter("O_SETFGCODE next %08x, offset %08x", _fgOpcodePC, offset);
}

void Interpreter::O_STOPHERO() {
	int32 heroId = readScriptFlagValue();
	debugInterpreter("O_STOPHERO heroId %d", heroId);
	if (!heroId) {
		_vm->_mainHero->freeOldMove();
	} else if (heroId == 1) {
		_vm->_secondHero->freeOldMove();
	}
}

void Interpreter::O_ANIMUPDATEOFF() {
	int32 slotId = readScriptFlagValue();
	debugInterpreter("O_ANIMUPDATEOFF slotId %d", slotId);
	_vm->_normAnimList[slotId]._state = 1;
}

void Interpreter::O_ANIMUPDATEON() {
	int32 slotId = readScriptFlagValue();
	debugInterpreter("O_ANIMUPDATEON slotId %d", slotId);
	_vm->_normAnimList[slotId]._state = 0;
}

void Interpreter::O_FREECURSOR() {
	debugInterpreter("O_FREECURSOR");
	_vm->changeCursor(0);
	_vm->_currentPointerNumber = 1;
	// free memory here?
}

void Interpreter::O_ADDINVQUIET() {
	int32 hero = readScriptFlagValue();
	int32 item = readScriptFlagValue();
	debugInterpreter("O_ADDINVQUIET hero %d, item %d", hero, item);
	_vm->addInv(hero, item, true);
}

void Interpreter::O_RUNHERO() {
	int32 heroId = readScriptFlagValue();
	int32 x = readScriptFlagValue();
	int32 y = readScriptFlagValue();
	int32 dir = readScriptFlagValue();
	debugInterpreter("O_RUNHERO heroId %d, x %d, y %d, dir %d", heroId, x, y, dir);
	_vm->moveRunHero(heroId, x, y, dir, true);
}

void Interpreter::O_SETBACKANIMDATA() {
	uint16 animNumber = readScript16();
	uint16 animDataOffset = readScript16();
	Flags::Id flagId = readScriptFlagId();
	uint16 value = _flags->getFlagValue((Flags::Id)(flagId));
	debugInterpreter("O_SETBACKANIMDATA flag %04X (%s), animNumber %d, animDataOffset %d, value %d", flagId, _flagMap.getFlagName(flagId), animNumber, animDataOffset, value);
	int currAnim = _vm->_backAnimList[animNumber]._seq._currRelative;
	_vm->_backAnimList[animNumber].backAnims[currAnim].setAnimData((Anim::AnimOffsets)(animDataOffset), value);
}

void Interpreter::O_VIEWFLC() {
	int32 animNr = readScriptFlagValue();
	debugInterpreter("O_VIEWFLC animNr %d", animNr);
	_vm->_flcFrameSurface = nullptr;
	_vm->loadAnim(animNr, false);
}

void Interpreter::O_CHECKFLCFRAME() {
	int32 frameNr = readScriptFlagValue();
	debugInterpreter("O_CHECKFLCFRAME frame number %d", frameNr);
	if (_vm->_flicPlayer.getCurFrame() != frameNr) {
		_currentInstruction -= 4;
		_opcodeNF = 1;
	}
}

void Interpreter::O_CHECKFLCEND() {
	const Video::FlicDecoder &flicPlayer = _vm->_flicPlayer;
	debugInterpreter("O_CHECKFLCEND frameCount %d, currentFrame %d", flicPlayer.getFrameCount(), flicPlayer.getCurFrame());
	if (flicPlayer.getFrameCount() - flicPlayer.getCurFrame() > 1) {
		_currentInstruction -= 2;
		_opcodeNF = 1;
	}
}

void Interpreter::O_FREEFLC() {
	debugInterpreter("O_FREEFLC");
	_vm->_flcFrameSurface = nullptr;
}

void Interpreter::O_TALKHEROSTOP() {
	int32 heroId = readScriptFlagValue();
	debugInterpreter("O_TALKHEROSTOP %d", heroId);
	if (!heroId) {
		_vm->_mainHero->_state = Hero::kHeroStateStay;
	} else if (heroId == 1) {
		_vm->_secondHero->_state = Hero::kHeroStateStay;
	}
}

void Interpreter::O_HEROCOLOR() {
	int32 heroId = readScriptFlagValue();
	int32 color = readScriptFlagValue();
	debugInterpreter("O_HEROCOLOR heroId %d, color %d", heroId, color);
	if (!heroId) {
		_vm->_mainHero->_color = color;
	} else if (heroId == 1) {
		_vm->_secondHero->_color = color;
	}
}

void Interpreter::O_GRABMAPA() {
	debugInterpreter("O_GRABMAPA");
	_vm->grabMap();
}

void Interpreter::O_ENABLENAK() {
	int32 nakId = readScriptFlagValue();
	debugInterpreter("O_ENABLENAK nakId %d", nakId);
	_vm->_maskList[nakId]._flags = 0;
}

void Interpreter::O_DISABLENAK() {
	int32 nakId = readScriptFlagValue();
	debugInterpreter("O_DISABLENAK nakId %d", nakId);
	_vm->_maskList[nakId]._flags = 1;
}

void Interpreter::O_GETMOBNAME() {
	int32 modId = readScriptFlagValue();
	debugInterpreter("O_GETMOBNAME modId %d", modId);
	strncpy((char *)_stringBuf, _vm->_mobList[modId]._name.c_str(), 1023);
	_string = _stringBuf;
}

void Interpreter::O_SWAPINVENTORY() {
	int32 hero = readScriptFlagValue();
	debugInterpreter("O_SWAPINVENTORY hero %d", hero);
	_vm->swapInv(hero);
}

void Interpreter::O_CLEARINVENTORY() {
	int32 hero = readScriptFlagValue();
	debugInterpreter("O_CLEARINVENTORY hero %d", hero);
	_vm->clearInv(hero);
}

void Interpreter::O_SKIPTEXT() {
	debugInterpreter("O_SKIPTEXT");
	increaseString();
}

void Interpreter::O_SETVOICEH() {
	int32 slot = readScriptFlagValue();
	debugInterpreter("O_SETVOICEH slot %d", slot);
	static const uint32 VOICE_H_SLOT = 28;
	uint16 voiceLineH = _flags->getFlagValue(Flags::VOICE_H_LINE);
	_vm->setVoice(slot, VOICE_H_SLOT, voiceLineH);
}

void Interpreter::O_SETVOICEA() {
	int32 slot = readScriptFlagValue();
	debugInterpreter("O_SETVOICEA slot %d", slot);
	static const uint32 VOICE_A_SLOT = 29;
	uint16 voiceLineH = _flags->getFlagValue(Flags::VOICE_H_LINE);
	_vm->setVoice(slot, VOICE_A_SLOT, voiceLineH);
}

void Interpreter::O_SETVOICEB() {
	int32 slot = readScriptFlagValue();
	debugInterpreter("O_SETVOICEB slot %d", slot);
	static const uint32 VOICE_B_SLOT = 30;
	uint16 voiceLineH = _flags->getFlagValue(Flags::VOICE_H_LINE);
	_vm->setVoice(slot, VOICE_B_SLOT, voiceLineH);
}

void Interpreter::O_SETVOICEC() {
	int32 slot = readScriptFlagValue();
	debugInterpreter("O_SETVOICEC slot %d", slot);
	static const uint32 VOICE_C_SLOT = 31;
	uint16 voiceLineH = _flags->getFlagValue(Flags::VOICE_H_LINE);
	_vm->setVoice(slot, VOICE_C_SLOT, voiceLineH);
}

void Interpreter::O_SETVOICED() {
	int32 slot = readScriptFlagValue();
	debugInterpreter("O_SETVOICED slot %d", slot);
	static const uint32 VOICE_D_SLOT = 32;
	uint16 voiceLineH = _flags->getFlagValue(Flags::VOICE_H_LINE);
	_vm->setVoice(slot, VOICE_D_SLOT, voiceLineH);
}

void Interpreter::O_VIEWFLCLOOP() {
	int32 animId = readScriptFlagValue();
	debugInterpreter("O_VIEWFLCLOOP animId %d", animId);
	_vm->loadAnim(animId, true);
}

// Not used in script
void Interpreter::O_FLCSPEED() {
	int32 speed = readScriptFlagValue();
	error("O_FLCSPEED speed %d", speed);
}

void Interpreter::O_OPENINVENTORY() {
	debugInterpreter("O_OPENINVENTORY");
	_vm->_showInventoryFlag = true;
	_opcodeNF = 1;
}

void Interpreter::O_KRZYWA() {
	debugInterpreter("O_KRZYWA");
	_vm->makeCurve();
}

void Interpreter::O_GETKRZYWA() {
	debugInterpreter("O_GETKRZYWA");
	_vm->getCurve();
}

void Interpreter::O_GETMOB() {
	Flags::Id flagId = readScriptFlagId();
	int32 posX = readScriptFlagValue();
	int32 posY = readScriptFlagValue();
	debugInterpreter("O_GETMOB flagId %d, posX %d, posY %d", flagId, posX, posY);
	int mobNumber = _vm->getMob(_vm->_mobList, true, posX, posY);
	_flags->setFlagValue(flagId, mobNumber + 1);
}

// Not used in game
void Interpreter::O_INPUTLINE() {
	error("O_INPUTLINE");
}

// Not used in script
void Interpreter::O_BREAK_POINT() {
	error("O_BREAK_POINT");
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

} // End of namespace Prince
