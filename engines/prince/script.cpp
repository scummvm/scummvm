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

#include "common/debug.h"
#include "common/debug-channels.h"
#include "common/stream.h"
#include "common/archive.h"

#include "audio/decoders/wave.h"
#include "audio/audiostream.h"

namespace Prince {

static const uint16 NUM_OPCODES = 144;

Script::Script(PrinceEngine *vm) : 
	_code(NULL), _stacktop(0), _vm(vm), _opcodeNF(false),
	_waitFlag(0), _result(true) {
}

Script::~Script() {
    delete[] _code;
}

void Script::setFlagValue(Flags::Id flagId, uint16 value) {
	_flags[(uint16)flagId - FLAG_MASK] = value;
}

uint16 Script::getFlagValue(Flags::Id flagId) {
	return _flags[(uint16)flagId - FLAG_MASK];
}

bool Script::loadFromStream(Common::SeekableReadStream &stream) {
    _codeSize = stream.size();
    _code = new byte[_codeSize];

    if (!_code)
        return false;

	stream.read(_code, _codeSize);
	// Initialize the script
	_mode = "fg";
	_fgOpcodePC = READ_LE_UINT32(_code + 4);
	_bgOpcodePC = 0;

    return true;
}

void Script::debugScript(const char *s, ...) {
	char buf[STRINGBUFLEN];
	 va_list va;

    va_start(va, s);
    vsnprintf(buf, STRINGBUFLEN, s, va);
    va_end(va);

	Common::String str = Common::String::format("@0x%08X: ", _lastInstruction);
	str += Common::String::format("op %04d: ", _lastOpcode);
	//debugC(10, DebugChannel::kScript, "PrinceEngine::Script %s %s", str.c_str(), buf);

	debug("Prince::Script frame %08ld mode %s %s %s", _vm->_frameNr, _mode, str.c_str(), buf);
}

void Script::step() {
	if (_bgOpcodePC) {
		_mode = "bg";
		_bgOpcodePC = step(_bgOpcodePC);
	}
	if (_fgOpcodePC) {
		_mode = "fg";
		_fgOpcodePC = step(_fgOpcodePC);
	}
}

uint32 Script::step(uint32 opcodePC) {

	_currentInstruction = opcodePC;
	while (!_opcodeNF)
	{
		_lastInstruction = _currentInstruction;
		// Prepare the base debug string
		Common::String dstr = Common::String::format("@0x%04X: ", _currentInstruction);

		// Get the current opcode
		_lastOpcode = readScript16bits();

		dstr += Common::String::format("op %02d: ", _lastOpcode);

		if (_lastOpcode > NUM_OPCODES)
			error("Trying to execute unknown opcode %s", dstr.c_str());


		//debugScript("");

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

uint8 Script::getCodeByte(uint32 address) {
    if (address >= _codeSize)
        error("Trying to read a script byte at address 0x%04X, while the "
            "script is just 0x%04X bytes long", address, _codeSize);
    return _code[address];
}

uint8 Script::readScript8bits() {
    uint8 data = getCodeByte(_currentInstruction);
    _currentInstruction++;
    return data;
}

uint16 Script::readScript16bits() {
    uint8 lower = readScript8bits();
    uint8 upper = readScript8bits();
    return lower | (upper << 8);
}

uint16 Script::readScriptValue() {
	uint16 value = readScript16bits();
	if (value & FLAG_MASK) {
		value = _flags[value - FLAG_MASK];
	}
	return value;
}

uint32 Script::readScript32bits() {
    uint16 lower = readScript16bits();
    uint16 upper = readScript16bits();
    return lower | (upper << 16);
}

void Script::O_WAITFOREVER() {
	debugScript("O_WAITFOREVER");
	_opcodeNF = 1;
	_currentInstruction -= 2;
}

void Script::O_BLACKPALETTE() {
	debugScript("O_BLACKPALETTE");
}

void Script::O_SETUPPALETTE() {
	debugScript("O_SETUPPALETTE");
}

void Script::O_INITROOM() {
	uint16 roomId = readScript16bits();
	debugScript("O_INITROOM %d", roomId);
	_vm->loadLocation(roomId);
	_opcodeNF = 1;
}

void Script::O_SETSAMPLE() {
	uint16 sampleId = readScript16bits();
	int32 sampleNameOffset = readScript32bits();
	const char * sampleName = (const char *)_code + _currentInstruction + sampleNameOffset - 4;
	debugScript("O_SETSAMPLE %d %s", sampleId, sampleName);
}

void Script::O_FREESAMPLE() {
	uint16 sample = readScript16bits();
	debugScript("O_FREESAMPLE %d", sample);
}

void Script::O_PLAYSAMPLE() {
	uint16 sampleId = readScript16bits();
	uint16 loopType = readScript16bits();
	debugScript("O_PLAYSAMPLE sampleId %d loopType %d", sampleId, loopType);
	_vm->playSample(sampleId, loopType);
}

void Script::O_PUTOBJECT() {
	uint16 roomId = readScript16bits();
	uint16 slot = readScript16bits();
	uint16 objectId = readScript16bits();
	debugScript("O_PUTOBJECT roomId %d, slot %d, objectId %d", roomId, slot, objectId);
}

void Script::O_REMOBJECT() {
	uint16 roomId = readScript16bits();
	uint16 objectId = readScript16bits();

	debugScript("O_REMOBJECT roomId %d objectId %d", roomId, objectId);
}

void Script::O_SHOWANIM() {
	uint16 slot = readScript16bits();
	uint16 animId = readScript16bits();

	debugScript("O_SHOWANIM slot %d, animId %d", slot, animId);
}

void Script::O_CHECKANIMEND() {
	uint16 slot = readScript16bits();
	uint16 frameId = readScript16bits();

	debugScript("O_CHECKANIMEND slot %d, frameId %d", slot, frameId);
	_opcodeNF = 1;
}

void Script::O_FREEANIM() {
	uint16 slot = readScript16bits();
	debugScript("O_FREEANIM slot %d", slot);
}

void Script::O_CHECKANIMFRAME() {
	uint16 slot = readScript16bits();
	uint16 frameId = readScript16bits();

	debugScript("O_CHECKANIMFRAME slot %d, frameId %d", slot, frameId);
	_opcodeNF = 1;
}

void Script::O_PUTBACKANIM() {
	uint16 roomId = readScript16bits();
	uint16 slot = readScript16bits();
	uint32 animId = readScript32bits();
	debugScript("O_PUTBACKANIM roomId %d, slot %d, animId %d", roomId, slot, animId);
}

void Script::O_REMBACKANIM() {
	uint16 roomId = readScript16bits();
	uint16 slot = readScript16bits();

	debugScript("O_REMBACKANIM roomId %d, slot %d", roomId, slot);
}

void Script::O_CHECKBACKANIMFRAME() {
	uint16 slotId = readScript16bits();
	uint16 frameId = readScript16bits();

	debugScript("O_CHECKBACKANIMFRAME slotId %d, frameId %d", slotId, frameId);
	_opcodeNF = 1;
}

void Script::O_FREEALLSAMPLES() {
	debugScript("O_FREEALLSAMPLES");
}

void Script::O_SETMUSIC() {
	uint16 musicId = readScript16bits();

	debugScript("O_SETMUSIC musicId %d", musicId);
}

void Script::O_STOPMUSIC() {
	debugScript("O_STOPMUSIC");
}

void Script::O__WAIT() {
	uint16 pause = readScript16bits();

	debugScript("O__WAIT pause %d", pause);

	if (_waitFlag == 0) {
		// set new wait flag value and continue
		_waitFlag = pause;
		_opcodeNF = 1;
		_currentInstruction -= 4;
		return;
	}

	--_waitFlag;

	if (_waitFlag > 0) {
		_opcodeNF = 1;
		_currentInstruction -= 4;
		return;
	}
}

void Script::O_UPDATEOFF() {
	debugScript("O_UPDATEOFF");
}

void Script::O_UPDATEON() {
	debugScript("O_UPDATEON");
}

void Script::O_UPDATE () {
	debugScript("O_UPDATE");
}

void Script::O_CLS() {
	debugScript("O_CLS");
}

void Script::O__CALL() {
	int32 address = readScript32bits();
	_stack[_stacktop] = _currentInstruction;
	_stacktop++;
	_currentInstruction += address - 4;
	debugScript("O__CALL 0x%04X", _currentInstruction);
}

void Script::O_RETURN() {
	// Get the return address
	if (_stacktop > 0) {
		_stacktop--;
		_currentInstruction = _stack[_stacktop];
		debugScript("O_RETURN 0x%04X", _currentInstruction);
	} else {
		error("Return: Stack is empty");
	}
}

void Script::O_GO() {
	int32 opPC = readScript32bits();
	debugScript("O_GO 0x%04X", opPC);
	_currentInstruction += opPC - 4;
}

void Script::O_BACKANIMUPDATEOFF() {
	uint16 slotId = readScript32bits();
	debugScript("O_BACKANIMUPDATEOFF slotId %d", slotId);
}

void Script::O_BACKANIMUPDATEON() {
	uint16 slot = readScript16bits();
	debugScript("O_BACKANIMUPDATEON %d", slot);
}

void Script::O_CHANGECURSOR() {
	uint16 cursorId = readScript16bits();
	debugScript("O_CHANGECURSOR %x", cursorId);
}

void Script::O_CHANGEANIMTYPE() {
	// NOT IMPLEMENTED
}

void Script::O__SETFLAG() {
	Flags::Id flagId = readScriptFlagId();
	uint16 value = readScriptValue();

	debugScript("O__SETFLAG 0x%04X (%s) = %d", flagId, Flags::getFlagName(flagId), value);

	setFlagValue((Flags::Id)(flagId), value);
}

void Script::O_COMPARE() {
	Flags::Id flagId = readScriptFlagId();
	uint16 value = readScriptValue();

	_result = getFlagValue(flagId) != value;
	debugScript("O_COMPARE flagId 0x%04X (%s), value %d == %d (%d)", flagId, Flags::getFlagName(flagId), value, getFlagValue(flagId), _result);
}

void Script::O_JUMPZ() {
	int32 offset = readScript32bits();
	if (! _result) {
		_currentInstruction += offset - 4;
	}

	debugScript("O_JUMPZ result = %d, next %08x, offset 0x%08X", _result, _currentInstruction, offset);
}

void Script::O_JUMPNZ() {
	int32 offset = readScript32bits();
	if (_result) {
		_currentInstruction += offset - 4;
	}

	debugScript("O_JUMPNZ result = %d, next %08x, offset 0x%08X", _result, _currentInstruction, offset);
}

void Script::O_EXIT() {
	uint16 exitCode = readScript16bits();
	debugScript("O_EXIT exitCode %d", exitCode);
}

void Script::O_ADDFLAG() {
	Flags::Id flagId = readScriptFlagId();
	uint16 value = readScriptValue();

	setFlagValue(flagId, getFlagValue(flagId) + value);
	if (getFlagValue(flagId))
		_result = 1;
	else
		_result = 0;

	debugScript("O_ADDFLAG flagId %04x (%s), value %d", flagId, Flags::getFlagName(flagId), value);
}

void Script::O_TALKANIM() {
	uint16 animSlot = readScript16bits();
	uint16 slot = readScript16bits();

	debugScript("O_TALKANIM animSlot %d, slot %d", animSlot, slot);
}

void Script::O_SUBFLAG() {
	Flags::Id flagId = readScriptFlagId();
	uint16 value = readScriptValue();

	setFlagValue(flagId, getFlagValue(flagId) - value);
	if (getFlagValue(flagId))
		_result = 1;
	else
		_result = 0;

	debugScript("O_SUBFLAG flagId %d, value %d", flagId, value);
}

void Script::O_SETSTRING() {
	int32 offset = readScript32bits();
	_currentString = offset;

	if (offset >= 80000) {
		debugScript("GetVaria %s", _vm->_variaTxt->getString(offset - 80000));
	}
	else if (offset < 2000) {
		uint32 of = READ_LE_UINT32(_vm->_talkTxt+offset*4);
		const char * txt = (const char *)&_vm->_talkTxt[of];
		_string = &_vm->_talkTxt[of];
		debugScript("TalkTxt %d %s", of, txt);
	}

	debugScript("O_SETSTRING %04d", offset);
}

void Script::O_ANDFLAG() {
	Flags::Id flagId = readScriptFlagId();
	uint16 value = readScriptValue();

	debugScript("O_ANDFLAG flagId %d, value %d", flagId, value);

	setFlagValue(flagId, getFlagValue(flagId) & value);

	if (getFlagValue(flagId)) {
		_result = 1;
	} else {
		_result = 0;
	}
}

void Script::O_GETMOBDATA() {
	Flags::Id flagId = readScriptFlagId();
	uint16 mobId = readScript16bits();
	uint16 mobOffset = readScript16bits();

	debugScript("O_GETMOBDATA flagId %d, modId %d, mobOffset %d", flagId, mobId, mobOffset);
}

void Script::O_ORFLAG() {
	Flags::Id flagId = readScriptFlagId();
	uint16 value = readScriptValue();
	
	debugScript("O_ORFLAG flagId %d, value %d", flagId, value);
	
	setFlagValue(flagId, getFlagValue(flagId) | value);

	if (getFlagValue(flagId)) {
		_result = 1;
	} else {
		_result = 0;
	}
}

void Script::O_SETMOBDATA() {
	uint16 mobId = readScript16bits();
	uint16 mobOffset = readScript16bits();
	uint16 value = readScript16bits();

	debugScript("O_SETMOBDATA mobId %d, mobOffset %d, value %d", mobId, mobOffset, value);
}

void Script::O_XORFLAG() {
	Flags::Id flagId = readScriptFlagId();
	uint16 value = readScriptValue();

	debugScript("O_XORFLAG flagId %d, value %d", flagId, value);

	setFlagValue(flagId, getFlagValue(flagId) ^ value);

	if (getFlagValue(flagId)) {
		_result = 1;
	} else {
		_result = 0;
	}
}

void Script::O_GETMOBTEXT() {
	uint16 value = readScript16bits();

	debugScript("O_GETMOBTEXT value %d", value);
}

void Script::O_MOVEHERO() {
	uint16 heroId = readScript16bits();
	uint16 x = readScript16bits();
	uint16 y = readScript16bits();
	uint16 dir = readScript16bits();
	
	debugScript("O_MOVEHERO heroId %d, x %d, y %d, dir %d", heroId, x, y, dir);
}

void Script::O_WALKHERO() {
	uint16 heroId = readScript16bits();

	debugScript("O_WALKHERO %d", heroId);
	_opcodeNF = 1;
}

void Script::O_SETHERO() {
	uint16 hero = readScript16bits();
	uint16 x = readScript16bits();
	uint16 y = readScript16bits();
	uint16 dir = readScript16bits();
	debugScript("O_SETHERO hero %d, x %d, y %d, dir %d", hero, x, y, dir);
}

void Script::O_HEROOFF() {
	uint16 heroId = readScript16bits();
	debugScript("O_HEROOFF %d", heroId);
}

void Script::O_HEROON() {
	uint16 heroId = readScript16bits();
	debugScript("O_HEROON %d", heroId);
}

void Script::O_CLSTEXT() {
	uint16 slot = readScript16bits();
	debugScript("O_CLSTEXT slot %d", slot);
}

void Script::O_CALLTABLE() {
	uint16 flag = readScript16bits();
	int32 table = readScript32bits();

	debugScript("O_CALLTABLE flag %d, table %d", flag, table);
}

void Script::O_CHANGEMOB() {
	uint16 mob = readScript16bits();
	uint16 value = readScript16bits();
	debugScript("O_CHANGEMOB mob %d, value %d", mob, value);
}

void Script::O_ADDINV() {
	uint16 hero = readScript16bits();
	uint16 item = readScript16bits();
	debugScript("O_ADDINV hero %d, item %d", hero, item);
}

void Script::O_REMINV() {
	uint16 hero = readScript16bits();
	uint16 item = readScript16bits();
	debugScript("O_REMINV hero %d, item %d", hero, item);
}

void Script::O_REPINV() {
	uint16 hero = readScript16bits();
	uint16 item1 = readScript16bits();
	uint16 item2 = readScript16bits();
	debugScript("O_REPINV hero %d, item1 %d, item2 %d", hero, item1, item2);
}

void Script::O_OBSOLETE_GETACTION() {
	debugScript("O_OBSOLETE_GETACTION");
}

void Script::O_ADDWALKAREA() {
	uint16 x1 = readScript16bits();
	uint16 y1 = readScript16bits();
	uint16 x2 = readScript16bits();
	uint16 y2 = readScript16bits();
	debugScript("O_ADDWALKAREA x1 %d, y1 %d, x2 %d, y2 %d", x1, y1, x2, y2);
}

void Script::O_REMWALKAREA() {
	uint16 x1 = readScript16bits();
	uint16 y1 = readScript16bits();
	uint16 x2 = readScript16bits();
	uint16 y2 = readScript16bits();
	debugScript("O_REMWALKAREA x1 %d, y1 %d, x2 %d, y2 %d", x1, y1, x2, y2);
}
 
void Script::O_RESTOREWALKAREA() {
	debugScript("O_RESTOREWALKAREA");
}

void Script::O_WAITFRAME() {
	debugScript("O_WAITFRAME");
	_opcodeNF = true;
}

void Script::O_SETFRAME() {
	uint16 anim = readScript16bits();
	uint16 frame = readScript16bits();
	debugScript("O_SETFRAME anim %d, frame %d", anim, frame);
}

void Script::O_RUNACTION() {
	debugScript("O_RUNACTION");
}

void Script::O_COMPAREHI() {
	uint16 flag = readScript16bits();
	uint16 value = readScript16bits();

	debugScript("O_COMPAREHI flag %d, value %d", flag, value);
}

void Script::O_COMPARELO() {
	uint16 flag = readScript16bits();
	uint16 value = readScript16bits();

	debugScript("O_COMPARELO flag %d, value %d", flag, value);
}

void Script::O_PRELOADSET() {
	int32 offset = readScript32bits();
	debugScript("O_PRELOADSET offset %04x", offset);
}

void Script::O_FREEPRELOAD() {
	debugScript("O_FREEPRELOAD");
}

void Script::O_CHECKINV() {
	uint16 hero = readScript16bits();
	uint16 item = readScript16bits();
	debugScript("O_CHECKINV hero %d, item %d", hero, item);
}

void Script::O_TALKHERO() {
	uint16 hero = readScript16bits();
	debugScript("O_TALKHERO hero %d", hero);
}

void Script::O_WAITTEXT() {
	uint16 slot = readScriptValue();
	Text &text = _vm->_textSlots[slot];
	if (text._time) {
		_opcodeNF = 1;
		_currentInstruction -= 4;
	}
}

void Script::O_SETHEROANIM() {
	uint16 hero = readScript16bits();
	int32 offset = readScript32bits();
	debugScript("O_SETHEROANIM hero %d, offset %d", hero, offset);
}

void Script::O_WAITHEROANIM() {
	uint16 hero = readScript16bits();

	debugScript("O_WAITHEROANIM hero %d", hero);
}

void Script::O_GETHERODATA() {
	uint16 flag = readScript16bits();
	uint16 hero = readScript16bits();
	uint16 heroOffset =readScript16bits();
	debugScript("O_GETHERODATA flag %d, hero %d, heroOffset %d", flag, hero, heroOffset);
}

void Script::O_GETMOUSEBUTTON() {
	debugScript("O_GETMOUSEBUTTON");
}

void Script::O_CHANGEFRAMES() {
	uint16 anim = readScript16bits();
	uint16 fr1 = readScript16bits();
	uint16 fr2 = readScript16bits();
	uint16 fr3 = readScript16bits();

	debugScript("O_CHANGFRAMES anim %d, fr1 %d, fr2 %d, fr3 %d", anim, fr1, fr2, fr3);

}

void Script::O_CHANGEBACKFRAMES() {
	uint16 anim = readScript16bits();
	uint16 fr1 = readScript16bits();
	uint16 fr2 = readScript16bits();
	uint16 fr3 = readScript16bits();

	debugScript("O_CHANGEBACKFRAMES anim %d, fr1 %d, fr2 %d, fr3 %d", anim, fr1, fr2, fr3);
}

void Script::O_GETBACKANIMDATA() {
	uint16 flag = readScript16bits();
	uint16 anim = readScript16bits();
	uint16 animOffset = readScript16bits();
	debugScript("O_GETBACKANIMDATA flag %d, anim %d, animOffset %d", flag, anim, animOffset);
}

void Script::O_GETANIMDATA() {
	uint16 flag = readScript16bits();
	uint16 anim = readScript16bits();
	uint16 animOffset = readScript16bits();
	debugScript("O_GETANIMDATA flag %d, anim %d, animOffset %d", flag, anim, animOffset);
}

void Script::O_SETBGCODE() {
	int32 offset = readScript32bits();
	_bgOpcodePC = _currentInstruction + offset;
	debugScript("O_SETBGCODE next %08x, offset %08x", _bgOpcodePC, offset);
}

void Script::O_SETBACKFRAME() {
	uint16 anim = readScript16bits();
	uint16 frame = readScript16bits();

	debugScript("O_SETBACKFRAME anim %d, frame %d", anim, frame);
}

void Script::O_GETRND() {
	uint16 flag = readScript16bits();
	uint16 rndSeed = readScript16bits();
	debugScript("O_GETRND flag %d, rndSeed %d", flag, rndSeed);
}

void Script::O_TALKBACKANIM() {
	uint16 animSlot = readScript16bits();
	uint16 slot = readScript16bits();
	debugScript("O_TALKBACKANIM animSlot %d, slot %d", animSlot, slot);
}

void Script::O_LOADPATH() {
	int32 offset = readScript32bits();
	debugScript("O_LOADPATH offset %d", offset);
}

void Script::O_GETCHAR() {
	Flags::Id flagId = readScriptFlagId();

	setFlagValue(flagId, *_string);

	debugScript("O_GETCHAR %04X (%s) %02x", flagId, Flags::getFlagName(flagId), getFlagValue(flagId));

	++_string;
}

void Script::O_SETDFLAG() {
	uint16 flag = readScript16bits();
	int32 offset = readScript32bits();
	debugScript("O_SETDFLAG flag %d, offset %04x", flag, offset);
}

void Script::O_CALLDFLAG() {
	uint16 flag = readScript16bits();
	debugScript("O_CALLDFLAG flag %d", flag);
}

void Script::O_PRINTAT() {
	uint16 slot = readScript16bits();
	uint16 fr1 = readScript16bits();
	uint16 fr2 = readScript16bits();

	debugScript("O_PRINTAT slot %d, fr1 %d, fr2 %d", slot, fr1, fr2);

	uint8 color = getFlagValue(Flags::KOLOR);

	_vm->printAt(slot, color, (const char *)_string, fr1, fr2);

	while (*_string) {
		++_string;
	}
	++_string;
}

void Script::O_ZOOMIN() {
	uint16 slot = readScript16bits();
	debugScript("O_ZOOMIN slot %04d", slot);
}

void Script::O_ZOOMOUT() {
	uint16 slot = readScript16bits();
	debugScript("O_ZOOMOUT slot %d", slot);
}

void Script::O_SETSTRINGOFFSET() {
	int32 offset = readScript32bits();
	debugScript("O_SETSTRINGOFFSET offset %04x", offset);
}

void Script::O_GETOBJDATA() {
	uint16 flag = readScript16bits();
	uint16 obj = readScript16bits();
	int16 objOffset = readScript16bits();
	debugScript("O_GETOBJDATA flag %d, obj %d, objOffset %d", flag, obj, objOffset);
}

void Script::O_SETOBJDATA() {
	uint16 obj = readScript16bits();
	int16 objOffset = readScript16bits();
	uint16 value = readScript16bits();
	debugScript("O_SETOBJDATA obj %d, objOffset %d, value %d", obj, objOffset, value);
}

void Script::O_SWAPOBJECTS() {
	uint16 obj1 = readScript16bits();
	uint16 obj2 = readScript16bits();
	debugScript("O_SWAPOBJECTS obj1 %d, obj2 %d", obj1, obj2);
}

void Script::O_CHANGEHEROSET() {
	uint16 hero = readScript16bits();
	uint16 heroSet = readScript16bits();
	debugScript("O_CHANGEHEROSET hero %d, heroSet %d", hero, heroSet);
}

void Script::O_ADDSTRING() {
	uint16 value = readScript16bits();
	debugScript("O_ADDSTRING value %d", value);
}

void Script::O_SUBSTRING() {
	uint16 value = readScript16bits();
	debugScript("O_SUBSTRING value %d", value);
}

void Script::O_INITDIALOG() {
	debugScript("O_INITDIALOG");
}

void Script::O_ENABLEDIALOGOPT() {
	uint16 opt = readScript16bits();
	debugScript("O_ENABLEDIALOGOPT opt %d", opt);
}

void Script::O_DISABLEDIALOGOPT() {
	uint16 opt = readScript16bits();
	debugScript("O_DISABLEDIALOGOPT opt %d", opt);
}

void Script::O_SHOWDIALOGBOX() {
	uint16 box = readScript16bits();
	debugScript("O_SHOWDIALOGBOX box %d", box);
}

void Script::O_STOPSAMPLE() {
	uint16 slot = readScript16bits();
	debugScript("O_STOPSAMPLE slot %d", slot);
	_vm->stopSample(slot);
}

void Script::O_BACKANIMRANGE() {
	uint16 slotId = readScript16bits();
	uint16 animId = readScript16bits();
	uint16 low = readScript16bits();
	uint16 high = readScript16bits();

	debugScript("O_BACKANIMRANGE slotId %d, animId %d, low %d, high %d", slotId, animId, low, high);
}

void Script::O_CLEARPATH() {
	debugScript("O_CLEARPATH");
}

void Script::O_SETPATH() {
	debugScript("O_SETPATH");
}

void Script::O_GETHEROX() {
	uint16 heroId = readScript16bits();
	Flags::Id flagId = readScriptFlagId();

	debugScript("O_GETHEROX heroId %d, flagId %d", heroId, flagId);
}

void Script::O_GETHEROY() {
	uint16 heroId = readScript16bits();
	Flags::Id flagId = readScriptFlagId();

	debugScript("O_GETHEROY heroId %d, flagId %d", heroId, flagId);
}

void Script::O_GETHEROD() {
	uint16 heroId = readScript16bits();
	Flags::Id flagId = readScriptFlagId();

	debugScript("O_GETHEROD heroId %d, flagId %d", heroId, flagId);
}

void Script::O_PUSHSTRING() {
	debugScript("O_PUSHSTRING");
}

void Script::O_POPSTRING() {
	debugScript("O_POPSTRING");
}

void Script::O_SETFGCODE() {
	int32 offset = readScript32bits();
	_fgOpcodePC = _currentInstruction + offset;	

	debugScript("O_SETFGCODE next %08x, offset %08x", _fgOpcodePC, offset);
}

void Script::O_STOPHERO() {
	uint16 heroId = readScript16bits();

	debugScript("O_STOPHERO heroId %d", heroId);
}

void Script::O_ANIMUPDATEOFF() {
	uint16 slotId = readScript16bits();
	debugScript("O_ANIMUPDATEOFF slotId %d", slotId);
}

void Script::O_ANIMUPDATEON() {
	uint16 slotId = readScript16bits();
	debugScript("O_ANIMUPDATEON slotId %d", slotId);
}

void Script::O_FREECURSOR() {
	debugScript("O_FREECURSOR");
}

void Script::O_ADDINVQUIET() {
	uint16 heroId = readScript16bits();
	uint16 itemId = readScript16bits();

	debugScript("O_ADDINVQUIET heorId %d, itemId %d", heroId, itemId);
}

void Script::O_RUNHERO() {
	uint16 heroId = readScript16bits();
	uint16 x = readScript16bits();
	uint16 y = readScript16bits();
	uint16 dir = readScript16bits();

	debugScript("O_RUNHERO heroId %d, x %d, y %d, dir %d", heroId, x, y, dir);
}

void Script::O_SETBACKANIMDATA() {
	uint16 animId = readScript16bits();
	uint16 animOffset = readScript16bits();
	uint16 wart = readScript16bits();

	debugScript("O_SETBACKANIMDATA animId %d, animOffset %d, wart %d", animId, animOffset, wart);
}

void Script::O_VIEWFLC() {
	uint16 animNr = readScript16bits();
	debugScript("O_VIEWFLC animNr %d", animNr);
	_vm->loadAnim(animNr, false);
}

void Script::O_CHECKFLCFRAME() {
	uint16 frameNr = readScript16bits();

	debugScript("O_CHECKFLCFRAME frame number %d", frameNr);

	const Video::FlicDecoder &flicPlayer = _vm->_flicPlayer;

	if (flicPlayer.getCurFrame() != frameNr)
	{
		// Move instruction pointer before current instruciton
		// must do this check once again till it's false 
		_currentInstruction -= 2;
		_opcodeNF = 1;
	}
}

void Script::O_CHECKFLCEND() {

	//debugScript("O_CHECKFLCEND");

	const Video::FlicDecoder &flicPlayer = _vm->_flicPlayer;

	//debug("frameCount %d, currentFrame %d", flicPlayer.getFrameCount(), flicPlayer.getCurFrame());

	if (flicPlayer.getFrameCount() - flicPlayer.getCurFrame() > 1)
	{
		// Move instruction pointer before current instruciton
		// must do this check once again till it's false 
		_currentInstruction -= 2;
		_opcodeNF = 1;
	}
}

void Script::O_FREEFLC() {
	debugScript("O_FREEFLC");
}

void Script::O_TALKHEROSTOP() {
	uint16 heroId = readScript16bits();
	debugScript("O_TALKHEROSTOP %d", heroId);
}

void Script::O_HEROCOLOR() {
	uint16 heroId = readScript16bits();
	uint16 kolorr = readScript16bits();
	debugScript("O_HEROCOLOR heroId %d, kolorr %d", heroId, kolorr);
}

void Script::O_GRABMAPA() {
	debugScript("O_GRABMAPA");
}

void Script::O_ENABLENAK() {
	uint16 nakId = readScript16bits();
	debugScript("O_ENABLENAK nakId %d", nakId);
}

void Script::O_DISABLENAK() {
	uint16 nakId = readScript16bits();
	debugScript("O_DISABLENAK nakId %d", nakId);
}

void Script::O_GETMOBNAME() {
	uint16 war = readScript16bits();
	debugScript("O_GETMOBNAME war %d", war);
}

void Script::O_SWAPINVENTORY() {
	uint16 heroId = readScript16bits();
	debugScript("O_SWAPINVENTORY heroId %d", heroId);
}

void Script::O_CLEARINVENTORY() {
	uint16 heroId = readScript16bits();
	debugScript("O_CLEARINVENTORY heroId %d", heroId);
}

void Script::O_SKIPTEXT() {
	debugScript("O_SKIPTEXT");
}

void Script::SetVoice(uint32 slot) {
	_vm->loadVoice(
		slot,
		Common::String::format(
			"%03d-%02d.WAV", 
			_currentString, 
			getFlagValue(Flags::VOICE_H_LINE)
		)
	);
}

void Script::O_SETVOICEH() {
	uint16 txn = readScript16bits();
	debugScript("O_SETVOICEH txn %d", txn);
	SetVoice(txn);
}

void Script::O_SETVOICEA() {
	uint16 txn = readScript16bits();
	debugScript("O_SETVOICEA txn %d", txn);
	SetVoice(txn);
}

void Script::O_SETVOICEB() {
	uint16 txn = readScript16bits();
	debugScript("O_SETVOICEB txn %d", txn);
	SetVoice(txn);
}

void Script::O_SETVOICEC() {
	uint16 txn = readScript16bits();
	debugScript("O_SETVOICEC txn %d", txn);
	SetVoice(txn);
}

void Script::O_VIEWFLCLOOP() {
	uint16 value = readScriptValue();
	debugScript("O_VIEWFLCLOOP animId %d", value);

	_vm->loadAnim(value, true);
}

void Script::O_FLCSPEED() {
	uint16 speed = readScript16bits();
	debugScript("O_FLCSPEED speed %d", speed);
}

void Script::O_OPENINVENTORY() {
	debugScript("O_OPENINVENTORY");
	_opcodeNF = 1;
}

void Script::O_KRZYWA() {
	debugScript("O_KRZYWA");
}

void Script::O_GETKRZYWA() {
	debugScript("O_GETKRZYWA");
}

void Script::O_GETMOB() {
	Flags::Id flagId = readScriptFlagId();
	uint16 mx = readScript16bits();
	uint16 my = readScript16bits();
	debugScript("O_GETMOB flagId %d, mx %d, my %d", flagId, mx, my);
}

void Script::O_INPUTLINE() {
	debugScript("O_INPUTLINE");
}

void Script::O_SETVOICED() {
	uint16 txn = readScript16bits();
	debugScript("O_SETVOICED txn %d", txn);
	SetVoice(txn);
}

void Script::O_BREAK_POINT() {
	debugScript("O_BREAK_POINT");
}

Script::OpcodeFunc Script::_opcodes[NUM_OPCODES] = {
	&Script::O_WAITFOREVER,
	&Script::O_BLACKPALETTE,
	&Script::O_SETUPPALETTE,
	&Script::O_INITROOM,
	&Script::O_SETSAMPLE,
	&Script::O_FREESAMPLE,
	&Script::O_PLAYSAMPLE,
	&Script::O_PUTOBJECT,
	&Script::O_REMOBJECT,
	&Script::O_SHOWANIM,
	&Script::O_CHECKANIMEND,
	&Script::O_FREEANIM,
	&Script::O_CHECKANIMFRAME,
	&Script::O_PUTBACKANIM,
	&Script::O_REMBACKANIM,
	&Script::O_CHECKBACKANIMFRAME,
	&Script::O_FREEALLSAMPLES,
	&Script::O_SETMUSIC,
	&Script::O_STOPMUSIC,
	&Script::O__WAIT,
	&Script::O_UPDATEOFF,
	&Script::O_UPDATEON,
	&Script::O_UPDATE ,
	&Script::O_CLS,
	&Script::O__CALL,
	&Script::O_RETURN,
	&Script::O_GO,
	&Script::O_BACKANIMUPDATEOFF,
	&Script::O_BACKANIMUPDATEON,
	&Script::O_CHANGECURSOR,
	&Script::O_CHANGEANIMTYPE,
	&Script::O__SETFLAG,
	&Script::O_COMPARE,
	&Script::O_JUMPZ,
	&Script::O_JUMPNZ,
	&Script::O_EXIT,
	&Script::O_ADDFLAG,
	&Script::O_TALKANIM,
	&Script::O_SUBFLAG,
	&Script::O_SETSTRING,
	&Script::O_ANDFLAG,
	&Script::O_GETMOBDATA,
	&Script::O_ORFLAG,
	&Script::O_SETMOBDATA,
	&Script::O_XORFLAG,
	&Script::O_GETMOBTEXT,
	&Script::O_MOVEHERO,
	&Script::O_WALKHERO,
	&Script::O_SETHERO,
	&Script::O_HEROOFF,
	&Script::O_HEROON,
	&Script::O_CLSTEXT,
	&Script::O_CALLTABLE,
	&Script::O_CHANGEMOB,
	&Script::O_ADDINV,
	&Script::O_REMINV,
	&Script::O_REPINV,
	&Script::O_OBSOLETE_GETACTION,
	&Script::O_ADDWALKAREA,
	&Script::O_REMWALKAREA,
	&Script::O_RESTOREWALKAREA,
	&Script::O_WAITFRAME,
	&Script::O_SETFRAME,
	&Script::O_RUNACTION,
	&Script::O_COMPAREHI,
	&Script::O_COMPARELO,
	&Script::O_PRELOADSET,
	&Script::O_FREEPRELOAD,
	&Script::O_CHECKINV,
	&Script::O_TALKHERO,
	&Script::O_WAITTEXT,
	&Script::O_SETHEROANIM,
	&Script::O_WAITHEROANIM,
	&Script::O_GETHERODATA,
	&Script::O_GETMOUSEBUTTON,
	&Script::O_CHANGEFRAMES,
	&Script::O_CHANGEBACKFRAMES,
	&Script::O_GETBACKANIMDATA,
	&Script::O_GETANIMDATA,
	&Script::O_SETBGCODE,
	&Script::O_SETBACKFRAME,
	&Script::O_GETRND,
	&Script::O_TALKBACKANIM,
	&Script::O_LOADPATH,
	&Script::O_GETCHAR,
	&Script::O_SETDFLAG,
	&Script::O_CALLDFLAG,
	&Script::O_PRINTAT,
	&Script::O_ZOOMIN,
	&Script::O_ZOOMOUT,
	&Script::O_SETSTRINGOFFSET,
	&Script::O_GETOBJDATA,
	&Script::O_SETOBJDATA,
	&Script::O_SWAPOBJECTS,
	&Script::O_CHANGEHEROSET,
	&Script::O_ADDSTRING,
	&Script::O_SUBSTRING,
	&Script::O_INITDIALOG,
	&Script::O_ENABLEDIALOGOPT,
	&Script::O_DISABLEDIALOGOPT,
	&Script::O_SHOWDIALOGBOX,
	&Script::O_STOPSAMPLE,
	&Script::O_BACKANIMRANGE,
	&Script::O_CLEARPATH,
	&Script::O_SETPATH,
	&Script::O_GETHEROX,
	&Script::O_GETHEROY,
	&Script::O_GETHEROD,
	&Script::O_PUSHSTRING,
	&Script::O_POPSTRING,
	&Script::O_SETFGCODE,
	&Script::O_STOPHERO,
	&Script::O_ANIMUPDATEOFF,
	&Script::O_ANIMUPDATEON,
	&Script::O_FREECURSOR,
	&Script::O_ADDINVQUIET,
	&Script::O_RUNHERO,
	&Script::O_SETBACKANIMDATA,
	&Script::O_VIEWFLC,
	&Script::O_CHECKFLCFRAME,
	&Script::O_CHECKFLCEND,
	&Script::O_FREEFLC,
	&Script::O_TALKHEROSTOP,
	&Script::O_HEROCOLOR,
	&Script::O_GRABMAPA,
	&Script::O_ENABLENAK,
	&Script::O_DISABLENAK,
	&Script::O_GETMOBNAME,
	&Script::O_SWAPINVENTORY,
	&Script::O_CLEARINVENTORY,
	&Script::O_SKIPTEXT,
	&Script::O_SETVOICEH,
	&Script::O_SETVOICEA,
	&Script::O_SETVOICEB,
	&Script::O_SETVOICEC,
	&Script::O_VIEWFLCLOOP,
	&Script::O_FLCSPEED,
	&Script::O_OPENINVENTORY,
	&Script::O_KRZYWA,
	&Script::O_GETKRZYWA,
	&Script::O_GETMOB,
	&Script::O_INPUTLINE,
	&Script::O_SETVOICED,
	&Script::O_BREAK_POINT,
};

}

/* vim: set tabstop=4 noexpandtab: */
