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

/*! \file
 * Operation Stealth script interpreter file
 */

#include "common/endian.h"

#include "cine/cine.h"
#include "cine/bg_list.h"
#include "cine/object.h"
#include "cine/sound.h"
#include "cine/various.h"
#include "cine/script.h"

namespace Cine {

const Opcode OSScript::_opcodeTable[] = {
	/* 00 */
	{ &FWScript::o1_modifyObjectParam, "bbw" },
	{ &FWScript::o1_getObjectParam, "bbb" },
	{ &FWScript::o1_addObjectParam, "bbw" },
	{ &FWScript::o1_subObjectParam, "bbw" },
	/* 04 */
	{ &FWScript::o1_mulObjectParam, "bbw" },
	{ &FWScript::o1_divObjectParam, "bbw" },
	{ &FWScript::o1_compareObjectParam, "bbw" },
	{ &FWScript::o1_setupObject, "bwwww" },
	/* 08 */
	{ &FWScript::o1_checkCollision, "bwwww" },
	{ &FWScript::o1_loadVar, "bc" },
	{ &FWScript::o1_addVar, "bc" },
	{ &FWScript::o1_subVar, "bc" },
	/* 0C */
	{ &FWScript::o1_mulVar, "bc" },
	{ &FWScript::o1_divVar, "bc" },
	{ &FWScript::o1_compareVar, "bc" },
	{ &FWScript::o1_modifyObjectParam2, "bbb" },
	/* 10 */
	{ 0, 0 },
	{ 0, 0 },
	{ 0, 0 },
	{ &FWScript::o1_loadMask0, "b" },
	/* 14 */
	{ &FWScript::o1_unloadMask0, "b" },
	{ &FWScript::o1_addToBgList, "b" },
	{ &FWScript::o1_loadMask1, "b" },
	{ &FWScript::o1_unloadMask1, "b" },
	/* 18 */
	{ &FWScript::o1_loadMask4, "b" },
	{ &FWScript::o1_unloadMask4, "b" },
	{ &FWScript::o1_addSpriteFilledToBgList, "b" },
	{ &FWScript::o1_op1B, "" }, /* TODO: Name this opcode properly. */
	/* 1C */
	{ 0, 0 },
	{ &FWScript::o1_label, "l" },
	{ &FWScript::o1_goto, "b" },
	{ &FWScript::o1_gotoIfSup, "b" },
	/* 20 */
	{ &FWScript::o1_gotoIfSupEqu, "b" },
	{ &FWScript::o1_gotoIfInf, "b" },
	{ &FWScript::o1_gotoIfInfEqu, "b" },
	{ &FWScript::o1_gotoIfEqu, "b" },
	/* 24 */
	{ &FWScript::o1_gotoIfDiff, "b" },
	{ &FWScript::o1_removeLabel, "b" },
	{ &FWScript::o1_loop, "bb" },
	{ 0, 0 },
	/* 28 */
	{ 0, 0 },
	{ 0, 0 },
	{ 0, 0 },
	{ 0, 0 },
	/* 2C */
	{ 0, 0 },
	{ 0, 0 },
	{ 0, 0 },
	{ 0, 0 },
	/* 30 */
	{ 0, 0 },
	{ &FWScript::o1_startGlobalScript, "b" },
	{ &FWScript::o1_endGlobalScript, "b" },
	{ 0, 0 },
	/* 34 */
	{ 0, 0 },
	{ 0, 0 },
	{ 0, 0 },
	{ 0, 0 },
	/* 38 */
	{ 0, 0 },
	{ 0, 0 },
	{ 0, 0 },
	{ &FWScript::o1_loadAnim, "s" },
	/* 3C */
	{ &FWScript::o1_loadBg, "s" },
	{ &FWScript::o2_loadCt, "s" },
	{ 0, 0 },
	{ &FWScript::o2_loadPart, "s" },
	/* 40 */
	{ 0, 0 }, /* o1_closePart, triggered by some scripts (STARTA.PRC 4 for ex.) */
	{ &FWScript::o1_loadNewPrcName, "bs" },
	{ &FWScript::o1_requestCheckPendingDataLoad, "" },
	{ 0, 0 },
	/* 44 */
	{ 0, 0 },
	{ &FWScript::o1_blitAndFade, "" },
	{ &FWScript::o1_fadeToBlack, "" },
	{ &FWScript::o1_transformPaletteRange, "bbwww" },
	/* 48 */
	{ 0, 0 },
	{ &FWScript::o1_setDefaultMenuColor2, "b" },
	{ &FWScript::o1_palRotate, "bbb" },
	{ 0, 0 },
	/* 4C */
	{ 0, 0 },
	{ 0, 0 },
	{ 0, 0 },
	{ &FWScript::o1_break, "" },
	/* 50 */
	{ &FWScript::o1_endScript, "x" },
	{ &FWScript::o1_message, "bwwww" },
	{ &FWScript::o1_loadGlobalVar, "bc" },
	{ &FWScript::o1_compareGlobalVar, "bc" },
	/* 54 */
	{ 0, 0 },
	{ 0, 0 },
	{ 0, 0 },
	{ 0, 0 },
	/* 58 */
	{ 0, 0 },
	{ &FWScript::o1_declareFunctionName, "s" },
	{ &FWScript::o1_freePartRange, "bb" },
	{ &FWScript::o1_unloadAllMasks, "" },
	/* 5C */
	{ 0, 0 },
	{ 0, 0 },
	{ 0, 0 },
	{ 0, 0 },
	/* 60 */
	{ 0, 0 },
	{ 0, 0 },
	{ 0, 0 },
	{ &FWScript::o1_setScreenDimensions, "wwww" },
	/* 64 */
	{ &FWScript::o1_displayBackground, "" },
	{ &FWScript::o1_initializeZoneData, "" },
	{ &FWScript::o1_setZoneDataEntry, "bw" },
	{ &FWScript::o1_getZoneDataEntry, "bb" },
	/* 68 */
	{ &FWScript::o1_setDefaultMenuColor, "b" },
	{ &FWScript::o1_allowPlayerInput, "" },
	{ &FWScript::o1_disallowPlayerInput, "" },
	{ &FWScript::o1_changeDataDisk, "b" }, /* Same as opcodes 0x95 and 0xA9. */
	/* 6C */
	{ 0, 0 },
	{ &FWScript::o1_loadMusic, "s" },
	{ &FWScript::o1_playMusic, "" },
	{ &FWScript::o1_fadeOutMusic, "" },
	/* 70 */
	{ &FWScript::o1_stopSample, "" },
	{ &FWScript::o1_op71, "bw" }, /* TODO: Name this opcode properly. */
	{ &FWScript::o1_op72, "wbw" }, /* Same as opcode 0x73. TODO: Name this opcode properly. */
	{ &FWScript::o1_op72, "wbw" }, /* Same as opcode 0x72. */
	/* 74 */
	{ 0, 0 },
	{ 0, 0 },
	{ 0, 0 },
	{ &FWScript::o2_playSample, "bbwbww" },
	/* 78 */
	{ &FWScript::o2_playSampleAlt, "bbwbww" },
	{ &FWScript::o1_disableSystemMenu, "b" },
	{ &FWScript::o1_loadMask5, "b" },
	{ &FWScript::o1_unloadMask5, "b" }, /* Last opcode used by Future Wars. */
	/* 7C */
	{ 0, 0 },
	{ 0, 0 },
	{ 0, 0 },
	{ &FWScript::o2_addSeqListElement, "bbbbwww" },
	/* 80 */
	{ &FWScript::o2_removeSeq, "bb" },
	{ &FWScript::o2_op81, "" }, /* TODO: Name this opcode properly. */
	{ &FWScript::o2_op82, "bbwwb" }, /* TODO: Name this opcode properly. */
	{ &FWScript::o2_isSeqRunning, "bb" },
	/* 84 */
	{ &FWScript::o2_gotoIfSupNearest, "b" },
	{ &FWScript::o2_gotoIfSupEquNearest, "b" },
	{ &FWScript::o2_gotoIfInfNearest, "b" },
	{ &FWScript::o2_gotoIfInfEquNearest, "b" },
	/* 88 */
	{ &FWScript::o2_gotoIfEquNearest, "b" },
	{ &FWScript::o2_gotoIfDiffNearest, "b" },
	{ 0, 0 },
	{ &FWScript::o2_startObjectScript, "b" },
	/* 8C */
	{ &FWScript::o2_stopObjectScript, "b" },
	{ &FWScript::o2_op8D, "wwwwwwww" }, /* TODO: Name this opcode properly. */
	{ &FWScript::o2_addBackground, "bs" },
	{ &FWScript::o2_removeBackground, "b" },
	/* 90 */
	{ &FWScript::o2_loadAbs, "bs" },
	{ &FWScript::o2_loadBg, "b" },
	{ 0, 0 },
	{ 0, 0 },
	/* 94 */
	{ 0, 0 },
	{ &FWScript::o1_changeDataDisk, "b" }, /* Same as opcodes 0x6B and 0xA9. */
	{ 0, 0 },
	{ 0, 0 },
	/* 98 */
	{ 0, 0 },
	{ 0, 0 },
	{ &FWScript::o2_wasZoneChecked, "b" },
	{ &FWScript::o2_op9B, "wwwwwwww" }, /* TODO: Name this opcode properly. */
	/* 9C */
	{ &FWScript::o2_op9C, "wwww" }, /* TODO: Name this opcode properly. */
	{ &FWScript::o2_useBgScroll, "b" },
	{ &FWScript::o2_setAdditionalBgVScroll, "c" },
	{ &FWScript::o2_op9F, "ww" }, /* TODO: Name this opcode properly. */
	/* A0 */
	{ &FWScript::o2_addGfxElementA0, "ww" }, /* TODO: Name this opcode properly. */
	{ &FWScript::o2_removeGfxElementA0, "ww" }, /* TODO: Name this opcode properly. */
	{ &FWScript::o2_opA2, "ww" }, /* TODO: Name this opcode properly. */
	{ &FWScript::o2_opA3, "ww" }, /* TODO: Name this opcode properly. */
	/* A4 */
	{ &FWScript::o2_loadMask22, "b" }, /* TODO: Name this opcode properly. */
	{ &FWScript::o2_unloadMask22, "b" }, /* TODO: Name this opcode properly. */
	{ 0, 0 },
	{ 0, 0 },
	/* A8 */
	{ 0, 0 },
	{ &FWScript::o1_changeDataDisk, "b" } /* Same as opcodes 0x6B and 0x95. */
};
const unsigned int OSScript::_numOpcodes = ARRAYSIZE(OSScript::_opcodeTable);

/*! \brief Contructor for global scripts
 * \param script Script bytecode reference
 * \param idx Script bytecode index
 */
OSScript::OSScript(const RawScript &script, int16 idx) :
	FWScript(script, idx, new OSScriptInfo) {}

/*! \brief Constructor for object scripts
 * \param script Script bytecode reference
 * \param idx Script bytecode index
 */
OSScript::OSScript(RawObjectScript &script, int16 idx) :
	FWScript(script, idx, new OSScriptInfo) {}

/*! \brief Copy constructor
 */
OSScript::OSScript(const OSScript &src) : FWScript(src, new OSScriptInfo) {}

/*! \brief Restore script state from savefile
 * \param labels Restored script labels
 * \param local Restored local script variables
 * \param compare Restored last comparison result
 * \param pos Restored script position
 */
void OSScript::load(const ScriptVars &labels, const ScriptVars &local, uint16 compare, uint16 pos) {
	FWScript::load(labels, local, compare, pos);
}

/*! \brief Get opcode info string
 * \param opcode Opcode to look for in opcode table
 */
const char *OSScriptInfo::opcodeInfo(byte opcode) const {
	if (opcode == 0 || opcode > OSScript::_numOpcodes) {
		return NULL;
	}

	if (!OSScript::_opcodeTable[opcode - 1].args) {
		warning("Undefined opcode 0x%02X in OSScriptInfo::opcodeInfo", opcode - 1);
		return NULL;
	}

	return OSScript::_opcodeTable[opcode - 1].args;
}

/*! \brief Get opcode handler pointer
 * \param opcode Opcode to look for in opcode table
 */
opFunc OSScriptInfo::opcodeHandler(byte opcode) const {
	if (opcode == 0 || opcode > OSScript::_numOpcodes) {
		return NULL;
	}

	if (!OSScript::_opcodeTable[opcode - 1].proc) {
		warning("Undefined opcode 0x%02X in OSScriptInfo::opcodeHandler", opcode - 1);
		return NULL;
	}

	return OSScript::_opcodeTable[opcode - 1].proc;
}

/*! \brief Create new OSScript instance
 * \param script Script bytecode
 * \param index Bytecode index
 */
FWScript *OSScriptInfo::create(const RawScript &script, int16 index) const {
	return new OSScript(script, index);
}

/*! \brief Create new OSScript instance
 * \param script Object script bytecode
 * \param index Bytecode index
 */
FWScript *OSScriptInfo::create(const RawObjectScript &script, int16 index) const {
	return new OSScript(script, index);
}

/*! \brief Load saved OSScript instance
 * \param script Script bytecode
 * \param index Bytecode index
 * \param local Local variables
 * \param labels Script labels
 * \param compare Last compare result
 * \param pos Position in script
 */
FWScript *OSScriptInfo::create(const RawScript &script, int16 index, const ScriptVars &labels, const ScriptVars &local, uint16 compare, uint16 pos) const {
	OSScript *tmp = new OSScript(script, index);
	assert(tmp);
	tmp->load(labels, local, compare, pos);
	return tmp;
}

/*! \brief Load saved OSScript instance
 * \param script Object script bytecode
 * \param index Bytecode index
 * \param local Local variables
 * \param labels Script labels
 * \param compare Last compare result
 * \param pos Position in script
 */
FWScript *OSScriptInfo::create(const RawObjectScript &script, int16 index, const ScriptVars &labels, const ScriptVars &local, uint16 compare, uint16 pos) const {
	OSScript *tmp = new OSScript(script, index);
	assert(tmp);
	tmp->load(labels, local, compare, pos);
	return tmp;
}

// ------------------------------------------------------------------------
// OPERATION STEALTH opcodes
// ------------------------------------------------------------------------

int FWScript::o2_loadCt() {
	const char *param = getNextString();

	debugC(5, kCineDebugScript, "Line: %d: loadCt(\"%s\")", _line, param);
	loadCtOS(param);
	return 0;
}

int FWScript::o2_loadPart() {
	const char *param = getNextString();

	debugC(5, kCineDebugScript, "Line: %d: loadPart(\"%s\")", _line, param);
	return 0;
}

int FWScript::o2_playSample() {
	if (g_cine->getPlatform() == Common::kPlatformAmiga || g_cine->getPlatform() == Common::kPlatformAtariST) {
		// no-op in these versions
		getNextByte();
		getNextByte();
		getNextWord();
		getNextByte();
		getNextWord();
		getNextWord();
		return 0;
	}
	return o1_playSample();
}

int FWScript::o2_playSampleAlt() {
	byte num = getNextByte();
	byte channel = getNextByte();
	uint16 frequency = getNextWord();
	getNextByte();
	getNextWord();
	uint16 size = getNextWord();

	if (size == 0xFFFF) {
		size = animDataTable[num]._width * animDataTable[num]._height;
	}
	if (animDataTable[num].data()) {
		if (g_cine->getPlatform() == Common::kPlatformPC) {
			// if speaker output is available, play sound on it
			// if it's another device, don't play anything
			// TODO: implement this, it's used in the introduction for example
			// on each letter displayed
		} else {
			g_sound->playSound(channel, frequency, animDataTable[num].data(), size, 0, 0, 63, 0);
		}
	}
	return 0;
}

int FWScript::o2_addSeqListElement() {
	byte param1 = getNextByte();
	byte param2 = getNextByte();
	byte param3 = getNextByte();
	byte param4 = getNextByte();
	uint16 param5 = getNextWord();
	uint16 param6 = getNextWord();
	uint16 param7 = getNextWord();

	debugC(5, kCineDebugScript, "Line: %d: addSeqListElement(%d,%d,%d,%d,%d,%d,%d)", _line, param1, param2, param3, param4, param5, param6, param7);
	addSeqListElement(param1, 0, param2, param3, param4, param5, param6, 0, param7);
	return 0;
}

int FWScript::o2_removeSeq() {
	byte a = getNextByte();
	byte b = getNextByte();

	debugC(5, kCineDebugScript, "Line: %d: removeSeq(%d,%d) -> TODO", _line, a, b);
	removeSeq(a, 0, b);
	return 0;
}

/*! \todo Implement this instruction
 */
int FWScript::o2_op81() {
	warning("STUB: o2_op81()");
	// freeUnkList();
	return 0;
}

/*! \todo Implement this instruction
 */
int FWScript::o2_op82() {
	byte a = getNextByte();
	byte b = getNextByte();
	uint16 c = getNextWord();
	uint16 d = getNextWord();
	byte e = getNextByte();
	warning("STUB: o2_op82(%x, %x, %x, %x, %x)", a, b, c, d, e);
	return 0;
}

int FWScript::o2_isSeqRunning() {
	byte a = getNextByte();
	byte b = getNextByte();

	debugC(5, kCineDebugScript, "Line: %d: OP83(%d,%d) -> TODO", _line, a, b);

	if (isSeqRunning(a, 0, b)) {
		_compare = 1;
	} else {
		_compare = 0;
	}
	return 0;
}

/*! \todo The assert may produce false positives and requires testing
 */
int FWScript::o2_gotoIfSupNearest() {
	byte labelIdx = getNextByte();

	if (_compare == kCmpGT) {
		assert(_labels[labelIdx] != -1);

		debugC(5, kCineDebugScript, "Line: %d: if(>) goto nearest %d (true)", _line, labelIdx);
		_pos = _script.getLabel(*_info, labelIdx, _pos);
	} else {
		debugC(5, kCineDebugScript, "Line: %d: if(>) goto nearest %d (false)", _line, labelIdx);
	}
	return 0;
}

/*! \todo The assert may produce false positives and requires testing
 */
int FWScript::o2_gotoIfSupEquNearest() {
	byte labelIdx = getNextByte();

	if (_compare & (kCmpGT | kCmpEQ)) {
		assert(_labels[labelIdx] != -1);

		debugC(5, kCineDebugScript, "Line: %d: if(>=) goto nearest %d (true)", _line, labelIdx);
		_pos = _script.getLabel(*_info, labelIdx, _pos);
	} else {
		debugC(5, kCineDebugScript, "Line: %d: if(>=) goto nearest %d (false)", _line, labelIdx);
	}
	return 0;
}

/*! \todo The assert may produce false positives and requires testing
 */
int FWScript::o2_gotoIfInfNearest() {
	byte labelIdx = getNextByte();

	if (_compare == kCmpLT) {
		assert(_labels[labelIdx] != -1);

		debugC(5, kCineDebugScript, "Line: %d: if(<) goto nearest %d (true)", _line, labelIdx);
		_pos = _script.getLabel(*_info, labelIdx, _pos);
	} else {
		debugC(5, kCineDebugScript, "Line: %d: if(<) goto nearest %d (false)", _line, labelIdx);
	}
	return 0;
}

/*! \todo The assert may produce false positives and requires testing
 */
int FWScript::o2_gotoIfInfEquNearest() {
	byte labelIdx = getNextByte();

	if (_compare & (kCmpLT | kCmpEQ)) {
		assert(_labels[labelIdx] != -1);

		debugC(5, kCineDebugScript, "Line: %d: if(<=) goto nearest %d (true)", _line, labelIdx);
		_pos = _script.getLabel(*_info, labelIdx, _pos);
	} else {
		debugC(5, kCineDebugScript, "Line: %d: if(<=) goto nearest %d (false)", _line, labelIdx);
	}
	return 0;
}

/*! \todo The assert may produce false positives and requires testing
 */
int FWScript::o2_gotoIfEquNearest() {
	byte labelIdx = getNextByte();

	if (_compare == kCmpEQ) {
		assert(_labels[labelIdx] != -1);

		debugC(5, kCineDebugScript, "Line: %d: if(==) goto nearest %d (true)", _line, labelIdx);
		_pos = _script.getLabel(*_info, labelIdx, _pos);
	} else {
		debugC(5, kCineDebugScript, "Line: %d: if(==) goto nearest %d (false)", _line, labelIdx);
	}
	return 0;
}

/*! \todo The assert may produce false positives and requires testing
 */
int FWScript::o2_gotoIfDiffNearest() {
	byte labelIdx = getNextByte();

	if (_compare != kCmpEQ) {
		assert(_labels[labelIdx] != -1);

		debugC(5, kCineDebugScript, "Line: %d: if(!=) goto nearest %d (true)", _line, labelIdx);
		_pos = _script.getLabel(*_info, labelIdx, _pos);
	} else {
		debugC(5, kCineDebugScript, "Line: %d: if(!=) goto nearest %d (false)", _line, labelIdx);
	}
	return 0;
}

int FWScript::o2_startObjectScript() {
	byte param = getNextByte();

	debugC(5, kCineDebugScript, "Line: %d: startObjectScript(%d)", _line, param);
	runObjectScript(param);
	return 0;
}

int FWScript::o2_stopObjectScript() {
	byte param = getNextByte();

	debugC(5, kCineDebugScript, "Line: %d: stopObjectScript(%d)", _line, param);
	ScriptList::iterator it = objectScripts.begin();

	for (; it != objectScripts.end(); ++it) {
		if ((*it)->_index == param) {
			(*it)->_index = -1;
		}
	}
	return 0;
}

/*! \todo Implement this instruction
 */
int FWScript::o2_op8D() {
	uint16 a = getNextWord();
	uint16 b = getNextWord();
	uint16 c = getNextWord();
	uint16 d = getNextWord();
	uint16 e = getNextWord();
	uint16 f = getNextWord();
	uint16 g = getNextWord();
	uint16 h = getNextWord();
	warning("STUB: o2_op8D(%x, %x, %x, %x, %x, %x, %x, %x)", a, b, c, d, e, f, g, h);
	// _currentScriptElement->compareResult = ...
	return 0;
}

int FWScript::o2_addBackground() {
	byte param1 = getNextByte();
	const char *param2 = getNextString();

	debugC(5, kCineDebugScript, "Line: %d: addBackground(%s,%d)", _line, param2, param1);
	addBackground(param2, param1);
	return 0;
}

int FWScript::o2_removeBackground() {
	byte param = getNextByte();

	assert(param);

	debugC(5, kCineDebugScript, "Line: %d: removeBackground(%d)", _line, param);

	renderer->removeBg(param);
	return 0;
}

int FWScript::o2_loadAbs() {
	byte param1 = getNextByte();
	const char *param2 = getNextString();

	debugC(5, kCineDebugScript, "Line: %d: loadABS(%d,%s)", _line, param1, param2);
	loadAbs(param2, param1);
	return 0;
}

int FWScript::o2_loadBg() {
	byte param = getNextByte();

	assert(param < 9);

	debugC(5, kCineDebugScript, "Line: %d: useBg(%d)", _line, param);

	renderer->selectBg(param);
	return 0;
}

/*! \todo Check the current implementation for correctness
 */
int FWScript::o2_wasZoneChecked() {
	byte param = getNextByte();
	_compare = (param < 16 && zoneData[param]);
	debugC(5, kCineDebugScript, "Line: %d: o2_wasZoneChecked(%d)", _line, param);
	return 0;
}

/*! \todo Implement this instruction
 */
int FWScript::o2_op9B() {
	uint16 a = getNextWord();
	uint16 b = getNextWord();
	uint16 c = getNextWord();
	uint16 d = getNextWord();
	uint16 e = getNextWord();
	uint16 f = getNextWord();
	uint16 g = getNextWord();
	uint16 h = getNextWord();
	warning("STUB: o2_op9B(%x, %x, %x, %x, %x, %x, %x, %x)", a, b, c, d, e, f, g, h);
	return 0;
}

/*! \todo Implement this instruction
 */
int FWScript::o2_op9C() {
	uint16 a = getNextWord();
	uint16 b = getNextWord();
	uint16 c = getNextWord();
	uint16 d = getNextWord();
	warning("STUB: o2_op9C(%x, %x, %x, %x)", a, b, c, d);
	return 0;
}

int FWScript::o2_useBgScroll() {
	byte param = getNextByte();

	assert(param < 9);

	debugC(5, kCineDebugScript, "Line: %d: useBgScroll(%d)", _line, param);

	renderer->selectScrollBg(param);
	return 0;
}

int FWScript::o2_setAdditionalBgVScroll() {
	byte param1 = getNextByte();

	if (param1) {
		byte param2 = getNextByte();

		debugC(5, kCineDebugScript, "Line: %d: additionalBgVScroll = var[%d]", _line, param2);
		renderer->setScroll(_localVars[param2]);
	} else {
		uint16 param2 = getNextWord();

		debugC(5, kCineDebugScript, "Line: %d: additionalBgVScroll = %d", _line, param2);
		renderer->setScroll(param2);
	}
	return 0;
}

/*! \todo Implement this instruction
 */
int FWScript::o2_op9F() {
	warning("o2_op9F()");
	getNextWord();
	getNextWord();
	return 0;
}

int FWScript::o2_addGfxElementA0() {
	uint16 param1 = getNextWord();
	uint16 param2 = getNextWord();

	debugC(5, kCineDebugScript, "Line: %d: addGfxElementA0(%d,%d)", _line, param1, param2);
	addGfxElementA0(param1, param2);
	return 0;
}

/*! \todo Implement this instruction
 */
int FWScript::o2_removeGfxElementA0() {
	uint16 idx = getNextWord();
	uint16 param = getNextWord();
	warning("STUB? o2_removeGfxElementA0(%x, %x)", idx, param);
	removeGfxElementA0(idx, param);
	return 0;
}

/*! \todo Implement this instruction
 */
int FWScript::o2_opA2() {
	uint16 a = getNextWord();
	uint16 b = getNextWord();
	warning("STUB: o2_opA2(%x, %x)", a, b);
	// addGfxElementA2();
	return 0;
}

/*! \todo Implement this instruction
 */
int FWScript::o2_opA3() {
	uint16 a = getNextWord();
	uint16 b = getNextWord();
	warning("STUB: o2_opA3(%x, %x)", a, b);
	// removeGfxElementA2();
	return 0;
}

int FWScript::o2_loadMask22() {
	byte param = getNextByte();

	debugC(5, kCineDebugScript, "Line: %d: addOverlay22(%d)", _line, param);
	addOverlay(param, 22);
	return 0;
}

int FWScript::o2_unloadMask22() {
	byte param = getNextByte();

	debugC(5, kCineDebugScript, "Line: %d: removeOverlay22(%d)", _line, param);
	removeOverlay(param, 22);
	return 0;
}

} // End of namespace Cine
