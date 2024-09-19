/* ScummVM - Graphic Adventure Engine
 *
 * ScummVM is the legal property of its developers, whose names
 * are too numerous to list here. Please refer to the COPYRIGHT
 * file distributed with this source distribution.
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 *
 */

#ifdef ENABLE_HE

#include "common/config-manager.h"
#include "common/savefile.h"
#include "common/system.h"

#include "scumm/actor.h"
#include "scumm/charset.h"
#include "scumm/dialogs.h"
#include "scumm/file.h"
#include "scumm/he/intern_he.h"
#include "scumm/he/localizer.h"
#include "scumm/object.h"
#include "scumm/resource.h"
#include "scumm/scumm.h"
#include "scumm/he/sound_he.h"
#include "scumm/util.h"
#include "scumm/verbs.h"

namespace Scumm {

#define OPCODE(i, x)	_opcodes[i]._OPCODE(ScummEngine_v72he, x)

void ScummEngine_v72he::setupOpcodes() {
	ScummEngine_v71he::setupOpcodes();

	OPCODE(0x02, o72_pushDWord);
	OPCODE(0x04, o72_getScriptString);
	_opcodes[0x0a].setProc(0, 0);
	OPCODE(0x1b, o72_isAnyOf);
	_opcodes[0x42].setProc(0, 0);
	_opcodes[0x46].setProc(0, 0);
	_opcodes[0x4a].setProc(0, 0);
	_opcodes[0x4e].setProc(0, 0);
	OPCODE(0x50, o72_resetCutscene);
	OPCODE(0x52, o72_findObjectWithClassOf);
	OPCODE(0x54, o72_getObjectImageX);
	OPCODE(0x55, o72_getObjectImageY);
	OPCODE(0x56, o72_captureWizImage);
	OPCODE(0x58, o72_getTimer);
	OPCODE(0x59, o72_setTimer);
	OPCODE(0x5a, o72_getSoundPosition);
	OPCODE(0x5e, o72_startScript);
	OPCODE(0x60, o72_startObject);
	OPCODE(0x61, o72_drawObject);
	OPCODE(0x62, o72_printWizImage);
	OPCODE(0x63, o72_getArrayDimSize);
	OPCODE(0x64, o72_getNumFreeArrays);
	_opcodes[0x97].setProc(0, 0);	// was: o6_setObjectName
	OPCODE(0x9c, o72_roomOps);
	OPCODE(0x9d, o72_actorOps);
	OPCODE(0x9e, o72_verbOps);
	OPCODE(0xa0, o72_findObject);
	OPCODE(0xa4, o72_arrayOps);
	OPCODE(0xae, o72_systemOps);
	OPCODE(0xba, o72_talkActor);
	OPCODE(0xbb, o72_talkEgo);
	OPCODE(0xbc, o72_dimArray);
	OPCODE(0xc0, o72_dim2dimArray);
	OPCODE(0xc1, o72_traceStatus);
	OPCODE(0xc8, o72_kernelGetFunctions);
	OPCODE(0xce, o72_drawWizImage);
	OPCODE(0xcf, o72_debugInput);
	OPCODE(0xd5, o72_jumpToScript);
	OPCODE(0xda, o72_openFile);
	OPCODE(0xdb, o72_readFile);
	OPCODE(0xdc, o72_writeFile);
	OPCODE(0xdd, o72_findAllObjects);
	OPCODE(0xde, o72_deleteFile);
	OPCODE(0xdf, o72_rename);
	OPCODE(0xe1, o72_getPixel);
	OPCODE(0xe3, o72_pickVarRandom);
	OPCODE(0xea, o72_redimArray);
	OPCODE(0xf3, o72_readINI);
	OPCODE(0xf4, o72_writeINI);
	OPCODE(0xf8, o72_getResourceSize);
	OPCODE(0xf9, o72_createDirectory);
	OPCODE(0xfa, o72_setSystemMessage);
}

static const int arrayDataSizes[] = { 0, 1, 4, 8, 8, 16, 32 };

byte *ScummEngine_v72he::defineArray(int array, int type, int downMin, int downMax,
											int acrossMin, int acrossMax, bool newArray, int *newid) {
	int arrayPtr;
	int size;
	ArrayHeader *ah;

	assert(downMin >= 0 && downMin <= downMax);
	assert(acrossMin >= 0 && acrossMin <= acrossMax);
	assert(0 <= type && type <= 6);


	if (type == kBitArray || type == kNibbleArray)
		type = kByteArray;

	if (!newArray)
		nukeArray(array);

	arrayPtr = findFreeArrayId();

	if (newid)
		*newid = arrayPtr;

	debug(9, "defineArray (array %d, downMin %d, downMax %d acrossMin %d acrossMax %d", arrayPtr, downMin, downMax, acrossMin, acrossMax);

	if (!newArray && (array & 0x80000000)) {
		error("Can't define bit variable as array pointer");
	}

	if (!newArray)
		writeVar(array, (_game.heversion >= 80 ? (arrayPtr | MAGIC_ARRAY_NUMBER) : arrayPtr));

	size = ((downMax - downMin + 1) * (acrossMax - acrossMin + 1) * arrayDataSizes[type]) / 8;

	ah = (ArrayHeader *)_res->createResource(rtString,
		(_game.heversion >= 80 ? (arrayPtr & ~MAGIC_ARRAY_NUMBER) : arrayPtr),
		size + sizeof(ArrayHeader));

	ah->type = TO_LE_32(type);
	ah->acrossMin = TO_LE_32(acrossMin);
	ah->acrossMax = TO_LE_32(acrossMax);
	ah->downMin = TO_LE_32(downMin);
	ah->downMax = TO_LE_32(downMax);

	return ah->data;
}

int ScummEngine_v72he::readArray(int array, int idx2, int idx1) {
	debug(9, "readArray (array %d, down %d, aMin %d)", readVar(array), idx2, idx1);

	if (readVar(array) == 0)
		error("readArray: Reference to zeroed array pointer");

	ArrayHeader *ah = (ArrayHeader *)getResourceAddress(rtString, readVar(array));

	if (!ah)
		error("readArray: invalid array %d (%d)", array, readVar(array));

	if (idx2 < (int)FROM_LE_32(ah->downMin) || idx2 > (int)FROM_LE_32(ah->downMax) ||
		idx1 < (int)FROM_LE_32(ah->acrossMin) || idx1 > (int)FROM_LE_32(ah->acrossMax)) {
		error("readArray: array %d out of bounds: [%d, %d] exceeds [%d..%d, %d..%d]",
			  array, idx1, idx2, FROM_LE_32(ah->acrossMin), FROM_LE_32(ah->acrossMax),
			  FROM_LE_32(ah->downMin), FROM_LE_32(ah->downMax));
	}

#if defined(USE_ENET) && defined(USE_LIBCURL)
	if (_enableHECompetitiveOnlineMods) {
		// Mod for Backyard Baseball 2001 online competitive play: allow baserunners to be
		// turned around while they're jogging to the next base on a pop-up.
		// The game checks if the runner is forced to the next base (i.e. there's a runner on the base behind them),
		// and if they are then basepath clicks to turn them around have no effect.
		// Here we return 0 (false) under certain conditions, so these clicks now have the desired effect.
		if (_game.id == GID_BASEBALL2001 &&
			_currentRoom == 3 && vm.slot[_currentScript].number == 2076 &&  // This is the script that handles basepath clicks
			readVar(399) == 1 &&  // This checks that we're playing online
			readVar(0x8000 + 11) == 1 &&  // The ball is a pop-up
			readVar(291) < 2 &&  // Less than two outs
			// This is the array of baserunner status info, and the value in position 8 specifies whether the runner is forced
			array == 295 && idx1 == 8) {
				int runnerIdx = readVar(342);
				if (readArray(array, runnerIdx, 6) == 1 && readArray(array, runnerIdx, 7) == 1) {
					// Bugfix: if runner is going forward to 1st base, return 1 so they can't turn around
					return 1;
				} else {
					return 0;
				}
		}
	}
#endif

	const int offset = (FROM_LE_32(ah->acrossMax) - FROM_LE_32(ah->acrossMin) + 1) *
		(idx2 - FROM_LE_32(ah->downMin)) + (idx1 - FROM_LE_32(ah->acrossMin));

	switch (FROM_LE_32(ah->type)) {
	case kByteArray:
	case kStringArray:
		return ah->data[offset];

	case kIntArray:
		return (int16)READ_LE_UINT16(ah->data + offset * 2);

	case kDwordArray:
		return (int32)READ_LE_UINT32(ah->data + offset * 4);

	default:
		break;
	}

	return 0;
}

void ScummEngine_v72he::writeArray(int array, int idx2, int idx1, int value) {
	debug(9, "writeArray (array %d, down %d, aMin %d, value %d)", readVar(array), idx2, idx1, value);

	if (readVar(array) == 0)
		error("writeArray: Reference to zeroed array pointer");

	ArrayHeader *ah = (ArrayHeader *)getResourceAddress(rtString, readVar(array));

	if (!ah)
		error("writeArray: Invalid array (%d) reference", readVar(array));

	if (idx2 < (int)FROM_LE_32(ah->downMin) || idx2 > (int)FROM_LE_32(ah->downMax) ||
		idx1 < (int)FROM_LE_32(ah->acrossMin) || idx1 > (int)FROM_LE_32(ah->acrossMax)) {
		error("writeArray: array %d out of bounds: [%d, %d] exceeds [%d..%d, %d..%d]",
			  array, idx1, idx2, FROM_LE_32(ah->acrossMin), FROM_LE_32(ah->acrossMax),
			  FROM_LE_32(ah->downMin), FROM_LE_32(ah->downMax));
	}

	const int offset = (FROM_LE_32(ah->acrossMax) - FROM_LE_32(ah->acrossMin) + 1) *
		(idx2 - FROM_LE_32(ah->downMin)) - FROM_LE_32(ah->acrossMin) + idx1;

	switch (FROM_LE_32(ah->type)) {
	case kByteArray:
	case kStringArray:
		ah->data[offset] = value;
		break;

	case kIntArray:
		WRITE_LE_UINT16(ah->data + offset * 2, value);
		break;

	case kDwordArray:
		WRITE_LE_UINT32(ah->data + offset * 4, value);
		break;

	default:
		break;
	}
}

int ScummEngine_v72he::setupStringArray(int size) {
	writeVar(0, 0);
	defineArray(0, kStringArray, 0, 0, 0, size + 1);
	writeArray(0, 0, 0, 0);
	return readVar(0);
}

int ScummEngine_v72he::setupStringArrayFromString(const char *cStr) {
	// this is PUI_ScummStringArrayFromCString() found in PUSERMAC.cpp
	// I can see how its done up there in setupStringArray()
	// yet I'd note that 'SCUMMVAR_user_reserved' var was used instead of 0
	// and strlen(), not strlen() + 1 was used
	// plus, this function actually copies the string, not just 'sets up' the array

	writeVar(0, 0);

	int len = strlen(cStr) + 1;
	byte *ptr = defineArray(0, kStringArray, 0, 0, 0, len);
	if (ptr != nullptr)
		Common::strlcpy((char*)ptr, cStr, len);

	return readVar(0);
}

void ScummEngine_v72he::readArrayFromIndexFile() {
	int num;
	int a, b, c;

	while ((num = _fileHandle->readUint16LE()) != 0) {
		a = _fileHandle->readUint16LE();
		b = _fileHandle->readUint16LE();
		c = _fileHandle->readUint16LE();

		if (c == 1)
			defineArray(num, kBitArray, 0, a, 0, b);
		else
			defineArray(num, kDwordArray, 0, a, 0, b);
	}
}

void ScummEngine_v72he::arrayBlockOperation(
	int dstVariable, int dstDownMin, int dstDownMax, int dstAcrossMin, int dstAcrossMax,
	int a2Variable, int a2DownMin, int a2DownMax, int a2AcrossMin, int a2AcrossMax,
	int a1Variable, int a1DownMin, int a1DownMax, int a1AcrossMin, int a1AcrossMax,
	int (*op)(int a2, int a1)) {
	int downCount, acrossCount;
	int dstD, dstA, a1D, a1A, a2D, a2A;
	int dstDIndex, dstAIndex;
	int a1DIndex, a1AIndex;
	int a2DIndex, a2AIndex;

	checkArrayLimits(dstVariable, dstDownMin, dstDownMax, dstAcrossMin, dstAcrossMax);
	checkArrayLimits(a2Variable, a2DownMin, a2DownMax, a2AcrossMin, a2AcrossMax);
	checkArrayLimits(a1Variable, a1DownMin, a1DownMax, a1AcrossMin, a1AcrossMax);

	dstD = ((dstDownMax - dstDownMin) + 1);
	dstA = ((dstAcrossMax - dstAcrossMin) + 1);
	a1D = ((a1DownMax - a1DownMin) + 1);
	a1A = ((a1AcrossMax - a1AcrossMin) + 1);
	a2D = ((a2DownMax - a2DownMin) + 1);
	a2A = ((a2AcrossMax - a2AcrossMin) + 1);

	if (((dstD != a1D) || (a1D != a2D)) || ((dstA != a1A) || (a1A != a2A))) {

		debug("ScummEngine_v72he::arrayBlockOperation(): "
			"{%8d}[ %4d to %4d ][ %4d to %4d ] = "
			"({%8d}[ %4d to %4d ][ %4d to %4d ] <?> "
			"{%8d}[ %4d to %4d ][ %4d to %4d ] <?>)\n",
			dstVariable, dstDownMin, dstDownMax, dstAcrossMin, dstAcrossMax,
			a2Variable, a2DownMin, a2DownMax, a2AcrossMin, a2AcrossMax,
			a1Variable, a1DownMin, a1DownMax, a1AcrossMin, a1AcrossMax);

		error("Invalid ranges for array block math operation");
	}

	acrossCount = dstA;
	downCount = dstD;

	dstDIndex = dstDownMin;
	a1DIndex = a1DownMin;
	a2DIndex = a2DownMin;

	for (int downCounter = 0; downCounter < downCount; downCounter++) {
		dstAIndex = dstAcrossMin;
		a1AIndex = a1AcrossMin;
		a2AIndex = a2AcrossMin;

		for (int acrossCounter = 0; acrossCounter < acrossCount; acrossCounter++) {
			writeArray(dstVariable, dstDIndex, dstAIndex,
				(*op)(readArray(a2Variable, a2DIndex, a2AIndex), readArray(a1Variable, a1DIndex, a1AIndex)));

			dstAIndex++;
			a1AIndex++;
			a2AIndex++;
		}

		dstDIndex++;
		a1DIndex++;
		a2DIndex++;
	}
}

void ScummEngine_v72he::copyScriptString(byte *dst, int dstSize) {
	byte string[1024];
	byte chr;
	int pos = 0;

	int array = pop();
	if (array == -1) {
		if (_stringLength == 1)
			error("String stack underflow");

		_stringLength -= 2;
		 while ((chr = _stringBuffer[_stringLength]) != 0) {
			string[pos] = chr;
			pos++;

			if (pos > dstSize)
				error("String too long to pop");

			_stringLength--;
		}

		string[pos] = 0;
		_stringLength++;

		// Reverse string
		int len = resStrLen(string);
		while (len--)
			*dst++ = string[len];
	} else {
		writeVar(0, array);
		while ((chr = readArray(0, 0, pos)) != 0) {
			*dst++ = chr;
			pos++;
		}
	}
	*dst = 0;
}

void ScummEngine_v72he::decodeScriptString(byte *dst, bool scriptString) {
	const byte *src;
	int args[31];
	int num, len, val;
	byte chr, string[1024];
	byte *dst0 = dst;
	memset(args, 0, sizeof(args));
	memset(string, 0, sizeof(string));

	// Get stack list, plus one
	num = pop();
	for (int i = num; i >= 0; i--)
		args[i] = pop();

	// Get string
	if (scriptString) {
		len = resStrLen(_scriptPointer) + 1;
		memcpy(string, _scriptPointer, len);
		_scriptPointer += len;
	} else {
		copyScriptString(string, sizeof(string));
		len = resStrLen(string) + 1;
	}

	if (_localizer) {
		strncpy((char *) string, _localizer->translate((char *) string).c_str(), sizeof(string) - 1);
	}

	// Decode string
	num = 0;
	val = 0;
	while (len--) {
		chr = string[num++];
		if (chr == '%') {
			chr = string[num++];
			switch (chr) {
			case 'b':
				//dst += Common::sprintf_s((char *)dst, "%b", args[val++]);
				break;
			case 'c':
				*dst++ = args[val++];
				break;
			case 'd':
				dst += Common::sprintf_s((char *)dst, sizeof(string) - (dst - dst0), "%d", args[val++]);
				break;
			case 's':
				src = getStringAddress(args[val++]);
				if (src) {
					while (*src != 0)
						*dst++ = *src++;
				}
				break;
			case 'x':
				dst += Common::sprintf_s((char *)dst, sizeof(string) - (dst - dst0), "%x", args[val++]);
				break;
			default:
				*dst++ = '%';
				num--;
				break;
			}
		} else {
			*dst++ = chr;
		}
	}
	*dst = 0;

	if (_localizer) {
		strncpy((char *) dst0, _localizer->translate((char *) dst0).c_str(), sizeof(string) - 1);
	}
}

int ScummEngine_v72he::findObject(int x, int y, int num, int *args) {
	int b, cls, i, result;

	for (i = 1; i < _numLocalObjects; i++) {
		result = 0;
		if ((_objs[i].obj_nr < 1) || getClass(_objs[i].obj_nr, kObjectClassUntouchable))
			continue;

		// Check polygon bounds
		if (_wiz->doesObjectHavePolygon(_objs[i].obj_nr)) {
			if (_wiz->testForObjectPolygon(_objs[i].obj_nr, x, y))
				result = _objs[i].obj_nr;
			else if (VAR_POLYGONS_ONLY != 0xFF && VAR(VAR_POLYGONS_ONLY))
				continue;
		}

		if (!result) {
			// Check object bounds
			if (_objs[i].x_pos <= x && _objs[i].width + _objs[i].x_pos > x &&
			    _objs[i].y_pos <= y && _objs[i].height + _objs[i].y_pos > y)
					result = _objs[i].obj_nr;
		}

		if (result) {
			if (!num)
				return result;

			// Check object class
			cls = args[0];
			b = getClass(_objs[i].obj_nr, cls);
			if ((cls & 0x80 && b) || (!(cls & 0x80) && !b))
				return result;
		}
	}

	return 0;
}

void ScummEngine_v72he::o72_pushDWord() {
	push(fetchScriptDWordSigned());
}

void ScummEngine_v72he::o72_getScriptString() {
	byte chr;

	while ((chr = fetchScriptByte()) != 0) {
		_stringBuffer[_stringLength] = chr;
		_stringLength++;

		if (_stringLength >= 4096)
			error("String stack overflow");
	}

	_stringBuffer[_stringLength] = 0;
	_stringLength++;
}

void ScummEngine_v72he::o72_isAnyOf() {
	int args[128];
	int num, value;

	num = getStackList(args, ARRAYSIZE(args));
	value = pop();

	for (int i = 0; i < num; i++) {
		if (args[i] == value) {
			push(1);
			return;
		}
	}

	push(0);
}

void ScummEngine_v72he::o72_resetCutscene() {
	int idx;

	idx = vm.cutSceneStackPointer;
	vm.cutSceneStackPointer = 0;
	vm.cutScenePtr[idx] = 0;
	vm.cutSceneScript[idx] = 0;

	VAR(VAR_OVERRIDE) = 0;
}

void ScummEngine_v72he::o72_findObjectWithClassOf() {
	int args[16], num;

	num = getStackList(args, ARRAYSIZE(args));
	int y = pop();
	int x = pop();
	int r = findObject(x, y, num, args);
	push(r);
}

void ScummEngine_v72he::o72_getObjectImageX() {
	int object = pop();
	int objnum = getObjectIndex(object);

	if (objnum == -1) {
		push(0);
		return;
	}

	push(_objs[objnum].x_pos / 8);
}

void ScummEngine_v72he::o72_getObjectImageY() {
	int object = pop();
	int objnum = getObjectIndex(object);

	if (objnum == -1) {
		push(0);
		return;
	}

	push(_objs[objnum].y_pos / 8);
}

void ScummEngine_v72he::o72_captureWizImage() {
	int y2 = pop();
	int x2 = pop();
	int y1 = pop();
	int x1 = pop();
	int image = pop();

	_wiz->takeAWiz(image, x1, y1, x2, y2, false, true);

	if (_game.heversion >= 99) {
		_res->setModified(rtImage, image);
	}
}

void ScummEngine_v72he::o72_getTimer() {
	int timer = pop();
	byte cmd = fetchScriptByte();

	if (cmd == 10 || cmd == 50) {
		push(getHETimer(timer));
	} else {
		push(0);
	}
}

void ScummEngine_v72he::o72_setTimer() {
	int timer = pop();
	byte cmd = fetchScriptByte();

	if (cmd == 158 || cmd == 61) {
		setHETimer(timer);
	} else {
		error("TIMER command %d?", cmd);
	}
}

void ScummEngine_v72he::o72_getSoundPosition() {
	int snd = pop();
	push(((SoundHE *)_sound)->getSoundPosition(snd));
}

void ScummEngine_v72he::o72_startScript() {
	int args[25];
	int script;
	byte flags;

	getStackList(args, ARRAYSIZE(args));
	script = pop();
	flags = fetchScriptByte();

	// HACK: The credits script in Russian HE99 version of Freddi Fish 3
	// uses null strings, causing various errors, so skip it.
	if (_game.id == GID_FREDDI3 && _game.heversion == 99 && _language == Common::RU_RUS &&
		_currentRoom == 40 && script == 2057) {
			return;
	}

	runScript(script, (flags == SO_BAK || flags == SO_BAKREC), (flags == SO_REC || flags == SO_BAKREC), args);
}

void ScummEngine_v72he::o72_startObject() {
	int args[25];
	int script, entryp;
	byte flags;

	getStackList(args, ARRAYSIZE(args));
	entryp = pop();
	script = pop();
	flags = fetchScriptByte();
	runObjectScript(script, entryp, (flags == SO_BAK || flags == SO_BAKREC), (flags == SO_REC || flags == SO_BAKREC), args);
}

void ScummEngine_v72he::o72_drawObject() {
	byte subOp = fetchScriptByte();
	int state, y, x;

	switch (subOp) {
	case SO_AT_IMAGE:
		state = pop();
		y = pop();
		x = pop();
		break;
	case SO_IMAGE:
		state = pop();
		if (state == 0)
			state = 1;
		y = x = -100;
		break;
	case ScummEngine_v6::SubOpType::SO_AT:
		state = 1;
		y = pop();
		x = pop();
		break;
	default:
		error("o72_drawObject: default case %d", subOp);
	}

	int object = pop();
	int objnum = getObjectIndex(object);
	if (objnum == -1)
		return;

	if (y != -100 && x != -100) {
		_objs[objnum].x_pos = x * 8;
		_objs[objnum].y_pos = y * 8;
	}

	if (state != -1) {
		addObjectToDrawQue(objnum);
		putState(object, state);
	}
}

void ScummEngine_v72he::o72_printWizImage() {
	int image = pop();

	_wiz->simpleDrawAWiz(image, 0, 0, 0, kWRFPrint);
}

void ScummEngine_v72he::o72_getArrayDimSize() {
	byte subOp = fetchScriptByte();
	int32 maxValue, minValue;
	ArrayHeader *ah = (ArrayHeader *)getResourceAddress(rtString, readVar(fetchScriptWord()));
	if (!ah) {
		push(0);
		return;
	}

	switch (subOp) {
	case SO_NONE:
	case SO_NIBBLE:
		maxValue = FROM_LE_32(ah->acrossMax);
		minValue = FROM_LE_32(ah->acrossMin);
		push(maxValue - minValue + 1);
		break;
	case SO_BIT:
		maxValue = FROM_LE_32(ah->downMax);
		minValue = FROM_LE_32(ah->downMin);
		push(maxValue - minValue + 1);
		break;
	case SO_BYTE:
		push(FROM_LE_32(ah->acrossMin));
		break;
	case SO_INT:
		push(FROM_LE_32(ah->acrossMax));
		break;
	case SO_DWORD:
		push(FROM_LE_32(ah->downMin));
		break;
	case SO_STRING:
		push(FROM_LE_32(ah->downMax));
		break;
	default:
		error("o72_getArrayDimSize: default case %d", subOp);
	}
}

void ScummEngine_v72he::o72_getNumFreeArrays() {
	const ResourceManager::ResTypeData &rtd = _res->_types[rtString];
	int i, num = 0;

	for (i = 1; i < _numArray; i++) {
		if (!rtd[i]._address)
			num++;
	}

	push(num);
}

void ScummEngine_v72he::o72_roomOps() {
	int a, b, c, d, e;

	byte subOp = fetchScriptByte();

	switch (subOp) {
	case SO_ROOM_SCROLL:
		b = pop();
		a = pop();
		if (a < (_screenWidth / 2))
			a = (_screenWidth / 2);
		if (b < (_screenWidth / 2))
			b = (_screenWidth / 2);
		if (a > _roomWidth - (_screenWidth / 2))
			a = _roomWidth - (_screenWidth / 2);
		if (b > _roomWidth - (_screenWidth / 2))
			b = _roomWidth - (_screenWidth / 2);
		VAR(VAR_CAMERA_MIN_X) = a;
		VAR(VAR_CAMERA_MAX_X) = b;
		break;

	case SO_ROOM_SCREEN:
		b = pop();
		a = pop();
		initScreens(a, _screenHeight);
		break;

	case SO_ROOM_PALETTE:
		d = pop();
		c = pop();
		b = pop();
		a = pop();
		setPalColor(d, a, b, c);
		break;

	case SO_ROOM_INTENSITY:
		c = pop();
		b = pop();
		a = pop();
		darkenPalette(a, a, a, b, c);
		break;

	case SO_ROOM_SAVEGAME:
		_saveTemporaryState = true;
		_saveLoadSlot = pop();
		_saveLoadFlag = pop();
		break;

	case SO_ROOM_FADE:
		// Defaults to 1 but doesn't use fade effects
		a = pop();
		break;

	case SO_RGB_ROOM_INTENSITY:
		e = pop();
		d = pop();
		c = pop();
		b = pop();
		a = pop();
		darkenPalette(a, b, c, d, e);
		break;

	case SO_ROOM_NEW_PALETTE:
		a = pop();
		setCurrentPalette(a);
		break;

	case SO_ROOM_COPY_PALETTE:
		a = pop();
		b = pop();
		copyPalColor(a, b);
		break;

	case SO_ROOM_SAVEGAME_BY_NAME:
		byte buffer[256];

		copyScriptString((byte *)buffer, sizeof(buffer));

		_saveLoadFileName = (char *)buffer;
		debug(1, "o72_roomOps: case 221: filename %s", _saveLoadFileName.c_str());

		_saveLoadFlag = pop();
		_saveLoadSlot = 255;
		_saveTemporaryState = true;
		break;

	case SO_OBJECT_ORDER:
		b = pop();
		a = pop();
		swapObjects(a, b);
		break;

	case SO_ROOM_PALETTE_IN_ROOM:
		b = pop();
		a = pop();
		setRoomPalette(a, b);
		break;

	default:
		error("o72_roomOps: default case %d", subOp);
	}
}

void ScummEngine_v72he::o72_actorOps() {
	ActorHE *a;
	int i, j, k;
	int args[32];
	byte string[256];

	byte subOp = fetchScriptByte();
	if (subOp == SO_ACTOR_INIT) {
		_curActor = pop();
		return;
	}

	a = (ActorHE *)derefActorSafe(_curActor, "o72_actorOps");
	if (!a)
		return;

	switch (subOp) {
	case SO_CONDITION: 		// (HE 80+)
		k = getStackList(args, ARRAYSIZE(args));
		for (i = 0; i < k; ++i) {
			a->setUserCondition(args[i] & 0x7F, args[i] & 0x80);
		}
		break;
	case SO_TALK_CONDITION: 		// (HE 80+)
		k = pop();
		if (k == 0)
			k = _rnd.getRandomNumberRng(1, 10);
		a->_heNoTalkAnimation = 1;
		a->setTalkCondition(k);
		break;
	case SO_PRIORITY: 		// (HE 90+)
		a->_layer = pop();
		a->_needRedraw = true;
		break;
	case SO_ACTOR_DEFAULT_CLIPPED:
	{
		int x1, y1, x2, y2;
		y2 = pop();
		x2 = pop();
		y1 = pop();
		x1 = pop();
		setActorClippingRect(-1, x1, y1, x2, y2);
		break;
	}
	case ScummEngine_v6::SubOpType::SO_AT:		// (HE 98+)
		j = pop();
		i = pop();
		a->putActor(i, j);
		break;
	case SO_CLIPPED:		// (HE 99+)
	{
		int x1, y1, x2, y2;
		y2 = pop();
		x2 = pop();
		y1 = pop();
		x1 = pop();
		if (_curActor) {
			setActorClippingRect(_curActor, x1, y1, x2, y2);
		}
		break;
	}
	case SO_ERASE: // 	// (HE 90+)
		a->setActorEraseType(pop());
		break;
	case SO_COSTUME:
		a->setActorCostume(pop());
		break;
	case SO_STEP_DIST:
		j = pop();
		i = pop();
		a->setActorWalkSpeed(i, j);
		break;
	case SO_SOUND:
		k = getStackList(args, ARRAYSIZE(args));
		for (i = 0; i < k; i++)
			a->_sound[i] = args[i];
		break;
	case SO_WALK_ANIMATION:
		a->_walkFrame = pop();
		break;
	case SO_TALK_ANIMATION:
		a->_talkStopFrame = pop();
		a->_talkStartFrame = pop();
		break;
	case SO_STAND_ANIMATION:
		a->_standFrame = pop();
		break;
	case SO_ANIMATION:
		// dummy case in scumm6
		pop();
		pop();
		pop();
		break;
	case SO_DEFAULT:
		a->initActor(0);
		break;
	case SO_ELEVATION:
		a->setElevation(pop());
		break;
	case SO_ANIMATION_DEFAULT:
		a->_initFrame = 1;
		a->_walkFrame = 2;
		a->_standFrame = 3;
		a->_talkStartFrame = 4;
		a->_talkStopFrame = 5;
		break;
	case SO_PALETTE:
		j = pop();
		i = pop();
		assertRange(0, i, 255, "palette slot");
		a->remapActorPaletteColor(i, j);
		a->_needRedraw = true;
		break;
	case SO_TALK_COLOR:
		a->_talkColor = pop();
		// WORKAROUND bug #13730: defined subtitles color 16 is very dark and hard to read on the dark background.
		// we change it to brighter color to ease reading.
		if (_game.id == GID_FREDDI4 && _game.heversion == 98 && _currentRoom == 43 && a->_talkColor == 16 && enhancementEnabled(kEnhSubFmtCntChanges))
			a->_talkColor = 200;
		break;
	case SO_ACTOR_NAME:
		copyScriptString(string, sizeof(string));
		loadPtrToResource(rtActorName, a->_number, string);
		break;
	case SO_INIT_ANIMATION:
		a->_initFrame = pop();
		break;
	case SO_ACTOR_WIDTH:
		a->_width = pop();
		break;
	case SO_SCALE:
		i = pop();
		a->setScale(i, i);
		break;
	case SO_NEVER_ZCLIP:
		a->_forceClip = 0;
		break;
	case SO_ALWAYS_ZCLIP:
		a->_forceClip = pop();
		break;
	case SO_IGNORE_BOXES:
		a->_ignoreBoxes = 1;
		a->_forceClip = 0;
		if (a->isInCurrentRoom())
			a->putActor();
		break;
	case SO_FOLLOW_BOXES:
		a->_ignoreBoxes = 0;
		a->_forceClip = 0;
		if (a->isInCurrentRoom())
			a->putActor();
		break;
	case SO_ANIMATION_SPEED:
		a->setAnimSpeed(pop());
		break;
	case SO_SHADOW:
		a->_heShadow = pop();

		if (_game.heversion >= 80 && a->_heShadow) {
			if (!_wiz->getColorMixBlockPtrForWiz(a->_heShadow)) {
				debug(4, "o72_actorOps(): SO_SHADOW: Image %d missing shadow table", a->_heShadow);
				a->_heShadow = 0;
			}
		}

		a->_needRedraw = true;
		a->_needBgReset = true;

		break;
	case SO_TEXT_OFFSET:
		a->_talkPosY = pop();
		a->_talkPosX = pop();
		break;
	case SO_CHARSET_SET:		// (HE 72+)
		a->_charset = pop();
		break;
	case SO_ROOM_PALETTE:		// (HE 99+)
		a->_hePaletteNum = pop();
		a->_needRedraw = true;
		break;
	case SO_ACTOR_VARIABLE:
		i = pop();
		a->setAnimVar(pop(), i);
		break;
	case SO_ACTOR_IGNORE_TURNS_ON:
		a->_ignoreTurns = true;
		break;
	case SO_ACTOR_IGNORE_TURNS_OFF:
		a->_ignoreTurns = false;
		break;
	case SO_NEW:
		a->initActor(2);
		break;
	case SO_BACKGROUND_ON:
		a->drawActorToBackBuf(a->getPos().x, a->getPos().y);
		break;
	case SO_BACKGROUND_OFF:
		a->_drawToBackBuf = false;
		a->_needRedraw = true;
		a->_needBgReset = true;
		break;
	case SO_TALKIE:
		{
		copyScriptString(string, sizeof(string));
		int slot = pop();

		int len = resStrLen(string) + 1;
		memcpy(a->_heTalkQueue[slot].sentence, string, len);

		a->_heTalkQueue[slot].posX = a->_talkPosX;
		a->_heTalkQueue[slot].posY = a->_talkPosY;
		a->_heTalkQueue[slot].color = a->_talkColor;
		break;
		}
	default:
		error("o72_actorOps: default case %d", subOp);
	}
}

void ScummEngine_v72he::o72_verbOps() {
	int slot, a, b;
	VerbSlot *vs;
	byte name[200];

	byte subOp = fetchScriptByte();
	if (subOp == SO_VERB_INIT) {
		_curVerb = pop();
		_curVerbSlot = getVerbSlot(_curVerb, 0);
		assertRange(0, _curVerbSlot, _numVerbs - 1, "new verb slot");
		return;
	}
	vs = &_verbs[_curVerbSlot];
	slot = _curVerbSlot;
	switch (subOp) {
	case SO_VERB_IMAGE:
		a = pop();
		if (_curVerbSlot) {
			setVerbObject(_roomResource, a, slot);
			vs->type = kImageVerbType;
			vs->imgindex = a;
		}
		break;
	case SO_VERB_NAME:
		copyScriptString(name, sizeof(name));
		loadPtrToResource(rtVerb, slot, name);
		vs->type = kTextVerbType;
		vs->imgindex = 0;
		break;
	case SO_VERB_COLOR:
		vs->color = pop();
		break;
	case SO_VERB_HICOLOR:
		vs->hicolor = pop();
		break;
	case SO_VERB_AT:
		vs->curRect.top = pop();
		vs->curRect.left = pop();
		break;
	case SO_VERB_ON:
		vs->curmode = 1;
		break;
	case SO_VERB_OFF:
		vs->curmode = 0;
		break;
	case SO_VERB_DELETE:
		slot = getVerbSlot(pop(), 0);
		killVerb(slot);
		break;
	case SO_VERB_NEW:
		slot = getVerbSlot(_curVerb, 0);
		if (slot == 0) {
			for (slot = 1; slot < _numVerbs; slot++) {
				if (_verbs[slot].verbid == 0)
					break;
			}
			if (slot == _numVerbs)
				error("Too many verbs");
			_curVerbSlot = slot;
		}
		vs = &_verbs[slot];
		vs->verbid = _curVerb;
		vs->color = 2;
		vs->hicolor = 0;
		vs->dimcolor = 8;
		vs->type = kTextVerbType;
		vs->charset_nr = _string[0]._default.charset;
		vs->curmode = 0;
		vs->saveid = 0;
		vs->key = 0;
		vs->center = 0;
		vs->imgindex = 0;
		break;
	case SO_VERB_DIMCOLOR:
		vs->dimcolor = pop();
		break;
	case SO_VERB_DIM:
		vs->curmode = 2;
		break;
	case SO_VERB_KEY:
		vs->key = pop();
		break;
	case SO_VERB_CENTER:
		vs->center = 1;
		break;
	case SO_VERB_NAME_STR:
		a = pop();
		if (a == 0) {
			loadPtrToResource(rtVerb, slot, (const byte *)"");
		} else {
			loadPtrToResource(rtVerb, slot, getStringAddress(a));
		}
		vs->type = kTextVerbType;
		vs->imgindex = 0;
		break;
	case SO_VERB_IMAGE_IN_ROOM:
		b = pop();
		a = pop();

		if (slot && a != vs->imgindex) {
			setVerbObject(b, a, slot);
			vs->type = kImageVerbType;
			vs->imgindex = a;
		}
		break;
	case SO_VERB_BAKCOLOR:
		vs->bkcolor = pop();
		break;
	case 255:
		drawVerb(slot, 0);
		verbMouseOver(0);
		break;
	default:
		error("o72_verbops: default case %d", subOp);
	}
}

void ScummEngine_v72he::o72_findObject() {
	int y = pop();
	int x = pop();
	int r = findObject(x, y, 0, 0);
	push(r);
}

static int arrayBlockSubOp(int a2, int a1) {
	return (a2 - a1);
}

static int arrayBlockAddOp(int a2, int a1) {
	return (a2 + a1);
}

static int arrayBlockBANDOp(int a2, int a1) {
	return (a2 & a1);
}

static int arrayBlockBOROp(int a2, int a1) {
	return (a2 | a1);
}

static int arrayBlockBXOROp(int a2, int a1) {
	return (a2 ^ a1);
}

void ScummEngine_v72he::o72_arrayOps() {
	byte *data;
	byte string[1024];
	int acrossMax, acrossMin, downMax, downMin;
	int id, len, b, c, list[128];

	byte subOp = fetchScriptByte();
	int array = fetchScriptWord();
	debug(9,"o72_arrayOps: array %d case %d", array, subOp);

	switch (subOp) {
	case SO_STRING:
		copyScriptString(string, sizeof(string));
		len = resStrLen(string);
		data = defineArray(array, kStringArray, 0, 0, 0, len);
		memcpy(data, string, len);
		break;

	case SO_COMPLEX_ARRAY_ASSIGNMENT:
		{
			len = getStackList(list, ARRAYSIZE(list));
			acrossMax = pop();
			acrossMin = pop();
			downMax = pop();
			downMin = pop();
			id = readVar(array);

			if (id == 0) {
				defineArray(array, kDwordArray, downMin, downMax, acrossMin, acrossMax);
			}

			checkArrayLimits(array, downMin, downMax, acrossMin, acrossMax);

			int *currPtr = list;
			int countDown = len;

			for (int downCounter = downMin; downCounter <= downMax; downCounter++) {
				for (int acrossCounter = acrossMin; acrossCounter <= acrossMax; acrossCounter++) {
					writeArray(array, downCounter, acrossCounter, *currPtr++);

					if (!--countDown) {
						countDown = len;
						currPtr = list;
					}
				}
			}
		}

		break;
	case SO_COMPLEX_ARRAY_COPY_OPERATION:
		{
			int srcAcrossMax = pop();
			int srcAcrossMin = pop();
			int srcDownMax = pop();
			int srcDownMin = pop();
			int srcVariable = fetchScriptWord();
			int dstAcrossMax = pop();
			int dstAcrossMin = pop();
			int dstDownMax = pop();
			int dstDownMin = pop();

			if (dstAcrossMax - dstAcrossMin != srcAcrossMax - srcAcrossMin || srcDownMax - srcDownMin != dstDownMax - dstDownMin) {
				error("Source and dest ranges dataOffsetPtr are mismatched");
			}

			copyArray(array, dstDownMin, dstDownMax, dstAcrossMin, dstAcrossMax, srcVariable, srcDownMin, srcDownMax, srcAcrossMin, srcAcrossMax);
		}

		break;
	case SO_RANGE_ARRAY_ASSIGNMENT:
		{
			int rangeEnd = pop();
			int rangeStart = pop();
			acrossMax = pop();
			acrossMin = pop();
			downMax = pop();
			downMin = pop();
			id = readVar(array);

			if (id == 0) {
				defineArray(array, kDwordArray, downMin, downMax, acrossMin, acrossMax);
			}

			checkArrayLimits(array, downMin, downMax, acrossMin, acrossMax);

			int value = rangeStart;
			int count = ABS(rangeStart - rangeEnd) + 1;
			int stepValue = (rangeStart <= rangeEnd) ? 1 : -1;
			int countDown = count;

			for (int downCounter = downMin; downCounter <= downMax; downCounter++) {
				for (int acrossCounter = acrossMin; acrossCounter <= acrossMax; acrossCounter++) {
					writeArray(array, downCounter, acrossCounter, value);

					if (!--countDown) {
						value = rangeStart;
						countDown = count;
					} else {
						value += stepValue;
					}
				}
			}
		}

		break;
	case SO_COMPLEX_ARRAY_MATH_OPERATION:
		{
			// Used by script 84 (Send end of play info) in Backyard Football during online play.
			int a2Variable = fetchScriptWord();
			int a1Variable = fetchScriptWord();
			int mathOperationType = pop();

			int a1AcrossMax = pop();
			int a1AcrossMin = pop();
			int a1DownMax = pop();
			int a1DownMin = pop();

			int a2AcrossMax = pop();
			int a2AcrossMin = pop();
			int a2DownMax = pop();
			int a2DownMin = pop();

			acrossMax = pop();
			acrossMin = pop();
			downMax = pop();
			downMin = pop();

			debug(0, "Complex: %d = %d[%d to %d][%d to %d] %c %d[%d to %d][%d to %d]", array,
				a1Variable, a1AcrossMin, a1DownMax, a1AcrossMin, a1DownMax,
				" +-&|^"[mathOperationType],
				a2Variable, a2AcrossMin, a2DownMax, a2AcrossMin, a2DownMax);

			int dstD = downMax - downMin + 1;
			int dstA = acrossMax - acrossMin + 1;

			int a1D = a1DownMax - a1DownMin + 1;
			int a1A = a1AcrossMax - a1AcrossMin + 1;
			int a2D = a2DownMax - a2DownMin + 1;
			int a2A = a2AcrossMax - a2AcrossMin + 1;

			id = readVar(array);
			if (id == 0) {
				defineArray(array, kDwordArray, downMin, downMax, acrossMin, acrossMax);
			}

			if (a1D != a2D || a1D != dstD || a1A != a2A || a1A != dstA) {
				error("Operation dataOffsetPtr mismatch (%d vs %d)(%d vs %d)", a1D, a2D, a1A, a2A);
			}

			switch (mathOperationType) {
			case 1: // Addition
				arrayBlockOperation(
					array, downMin, downMax, acrossMin, acrossMax,
					a2Variable, a2DownMin, a2DownMax, a2AcrossMin, a2AcrossMax,
					a1Variable, a1DownMin, a1DownMax, a1AcrossMin, a1AcrossMax,
					arrayBlockAddOp);
				break;

			case 2: // Subtraction
				arrayBlockOperation(
					array, downMin, downMax, acrossMin, acrossMax,
					a2Variable, a2DownMin, a2DownMax, a2AcrossMin, a2AcrossMax,
					a1Variable, a1DownMin, a1DownMax, a1AcrossMin, a1AcrossMax,
					arrayBlockSubOp);
				break;

			case 3: // Binary AND
				arrayBlockOperation(
					array, downMin, downMax, acrossMin, acrossMax,
					a2Variable, a2DownMin, a2DownMax, a2AcrossMin, a2AcrossMax,
					a1Variable, a1DownMin, a1DownMax, a1AcrossMin, a1AcrossMax,
					arrayBlockBANDOp);
				break;

			case 4: // Binary OR
				arrayBlockOperation(
					array, downMin, downMax, acrossMin, acrossMax,
					a2Variable, a2DownMin, a2DownMax, a2AcrossMin, a2AcrossMax,
					a1Variable, a1DownMin, a1DownMax, a1AcrossMin, a1AcrossMax,
					arrayBlockBOROp);
				break;

			case 5: // Binary BXOR
				arrayBlockOperation(
					array, downMin, downMax, acrossMin, acrossMax,
					a2Variable, a2DownMin, a2DownMax, a2AcrossMin, a2AcrossMax,
					a1Variable, a1DownMin, a1DownMax, a1AcrossMin, a1AcrossMax,
					arrayBlockBXOROp);
				break;

			default:
				error("Invalid array math operation (%d)", mathOperationType);
				break;
			}

			break;
		}
	case SO_FORMATTED_STRING:
		decodeScriptString(string);
		len = resStrLen(string);
		data = defineArray(array, kStringArray, 0, 0, 0, len);
		memcpy(data, string, len);
		break;
	case SO_ASSIGN_INT_LIST:
		b = pop();
		c = pop();
		id = readVar(array);
		if (id == 0) {
			defineArray(array, kDwordArray, 0, 0, 0, b + c - 1);
		}
		while (c--) {
			writeArray(array, 0, b + c, pop());
		}
		break;
	case SO_ASSIGN_2DIM_LIST:
		len = getStackList(list, ARRAYSIZE(list));
		id = readVar(array);
		if (id == 0)
			error("Must DIM a two dimensional array before assigning");
		c = pop();
		while (--len >= 0) {
			writeArray(array, c, len, list[len]);
		}
		break;
	default:
		error("o72_arrayOps: default case %d (array %d)", subOp, array);
	}
}

void ScummEngine_v72he::o72_systemOps() {
	byte string[1024];

	byte subOp = fetchScriptByte();

	switch (subOp) {
	case SO_FLUSH_OBJECT_DRAW_QUE: // HE80+
		clearDrawObjectQueue();
		break;
	case SO_UPDATE_SCREEN: // HE80+
		backgroundToForegroundBlit(Common::Rect(_screenWidth, _screenHeight));
		updatePalette();
		break;
	case SO_RESTART:
		restart();
		break;
	case SO_QUIT:
		// Confirm shutdown
		confirmExitDialog();
		break;
	case SO_QUIT_QUIT:
		quitGame();
		break;
	case SO_START_SYSTEM_STRING:
		copyScriptString(string, sizeof(string));
		debug(0, "Start executable (%s)", string);
		break;
	case SO_RESTART_STRING:
		copyScriptString(string, sizeof(string));
		debug(0, "Start game (%s)", string);
		break;
	default:
		error("o72_systemOps invalid case %d", subOp);
	}
}

void ScummEngine_v72he::o72_talkActor() {
	Actor *a;

	int act = pop();

	_string[0].loadDefault();

	// A value of 225 can occur when examining the gold in the mine of pajama, after mining the gold.
	// This is a script bug, the script should set the subtitle color, not actor number.
	// This script bug was fixed in the updated version of pajama.
	if (act == 225) {
		_string[0].color = act;
	} else {
		_actorToPrintStrFor = act;
		if (_actorToPrintStrFor != 0xFF) {
			a = derefActor(_actorToPrintStrFor, "o72_talkActor");
			_string[0].color = a->_talkColor;
		}
	}

	actorTalk(_scriptPointer);

	_scriptPointer += resStrLen(_scriptPointer) + 1;
}

void ScummEngine_v72he::o72_talkEgo() {
	push(VAR(VAR_EGO));
	o72_talkActor();
}

void ScummEngine_v72he::o72_dimArray() {
	int data;

	byte subOp = fetchScriptByte();

	switch (subOp) {
	case SO_BIT:
		data = kBitArray;
		break;
	case SO_NIBBLE:
		data = kNibbleArray;
		break;
	case SO_BYTE:
		data = kByteArray;
		break;
	case SO_INT:
		data = kIntArray;
		break;
	case SO_DWORD:
		data = kDwordArray;
		break;
	case SO_STRING:
		data = kStringArray;
		break;
	case SO_UNDIM_ARRAY:
		nukeArray(fetchScriptWord());
		return;
	default:
		error("o72_dimArray: default case %d", subOp);
	}

	defineArray(fetchScriptWord(), data, 0, 0, 0, pop());
}


void ScummEngine_v72he::o72_dim2dimArray() {
	int data, acrossMax, downMax;

	byte subOp = fetchScriptByte();

	switch (subOp) {
	case SO_BIT:
		data = kBitArray;
		break;
	case SO_NIBBLE:
		data = kNibbleArray;
		break;
	case SO_BYTE:
		data = kByteArray;
		break;
	case SO_INT:
		data = kIntArray;
		break;
	case SO_DWORD:
		data = kDwordArray;
		break;
	case SO_STRING:
		data = kStringArray;
		break;
	default:
		error("o72_dim2dimArray: default case %d", subOp);
	}

	acrossMax = pop();
	downMax = pop();
	defineArray(fetchScriptWord(), data, 0, downMax, 0, acrossMax);
}

void ScummEngine_v72he::o72_traceStatus() {
	byte string[80];

	copyScriptString(string, sizeof(string));
	pop();
}

void ScummEngine_v72he::o72_kernelGetFunctions() {
	int args[29];
	byte *data;
	getStackList(args, ARRAYSIZE(args));

	switch (args[0]) {
	case 1:
		writeVar(0, 0);
		data = defineArray(0, kByteArray, 0, 0, 0, virtScreenSave(0, args[1], args[2], args[3], args[4]));
		virtScreenSave(data, args[1], args[2], args[3], args[4]);
		push(readVar(0));
		break;
	default:
		error("o72_kernelGetFunctions: default case %d", args[0]);
	}
}

void ScummEngine_v72he::o72_drawWizImage() {
	WizBufferElement wi;
	wi.flags = pop();
	wi.y = pop();
	wi.x = pop();
	wi.image = pop();
	wi.state = 0;
	_wiz->simpleDrawAWiz(wi.image, wi.state, wi.x, wi.y, wi.flags);
}

void ScummEngine_v72he::debugInput(byte* string) {
	byte *debugInputString;

	DebugInputDialog dialog(this, (char *)string);
	runDialog(dialog);
	while (!dialog.done) {
		parseEvents();
		dialog.handleKeyDown(_keyPressed);
	}

	writeVar(0, 0);
	debugInputString = defineArray(0, kStringArray, 0, 0, 0, dialog.buffer.size());
	memcpy(debugInputString, dialog.buffer.c_str(), dialog.buffer.size());
	push(readVar(0));
}

void ScummEngine_v72he::o72_debugInput() {
	byte string[255];

	copyScriptString(string, sizeof(string));
	debugInput(string);
}

void ScummEngine_v72he::o72_jumpToScript() {
	int args[25];
	int script;
	byte flags;

	getStackList(args, ARRAYSIZE(args));
	script = pop();
	flags = fetchScriptByte();
	stopObjectCode();
	runScript(script, (flags == SO_BAK || flags == SO_BAKREC), (flags == SO_REC || flags == SO_BAKREC), args);
}

void ScummEngine_v72he::o72_openFile() {
	int mode, slot, i;
	byte buffer[256];

	mode = pop();
	copyScriptString(buffer, sizeof(buffer));
	debug(1, "Trying to open file '%s'", (char *)buffer);

	slot = -1;
	for (i = 1; i < 17; i++) {
		if (_hInFileTable[i] == 0 && _hOutFileTable[i] == 0) {
			slot = i;
			break;
		}
	}

	if (slot != -1) {
		switch (mode) {
		case 1:   // Read mode
			_hInFileTable[slot] = openFileForReading(buffer);
			break;
		case 2:   // Write mode
			if (!strchr((char *)buffer, '/')) {
				_hOutFileTable[slot] = openSaveFileForWriting(buffer);
			}
			break;
		case 6: // Append mode
			if (!strchr((char *)buffer, '/'))
				_hOutFileTable[slot] = openSaveFileForAppending(buffer);
			break;
		default:
			error("o72_openFile(): wrong open file mode %d", mode);
		}

		if (_hInFileTable[slot] == 0 && _hOutFileTable[slot] == 0)
			slot = -1;

	}
	debug(1, "o72_openFile: slot %d, mode %d", slot, mode);
	push(slot);
}

int ScummEngine_v72he::readFileToArray(int slot, int32 size) {
	writeVar(0, 0);
	byte *data = defineArray(0, kByteArray, 0, 0, 0, size);

	if (slot != -1) {
		assert(_hInFileTable[slot]);
		_hInFileTable[slot]->read(data, size + 1);
	}

	int returnValue = readVar(0);

	if (_game.heversion >= 80)
		returnValue |= MAGIC_ARRAY_NUMBER;

	return returnValue;
}

void ScummEngine_v72he::o72_readFile() {
	int slot, val;
	int32 size;
	byte subOp = fetchScriptByte();

	switch (subOp) {
	case SO_BYTE:
		slot = pop();
		if (slot == -1) {
			val = 0;
		} else {
			assert(_hInFileTable[slot]);
			val = _hInFileTable[slot]->readByte();
		}

		push(val);
		break;
	case SO_INT:
		slot = pop();
		if (slot == -1) {
			val = 0;
		} else {
			assert(_hInFileTable[slot]);
			val = _hInFileTable[slot]->readUint16LE();
		}

		push(val);
		break;
	case SO_DWORD:
		slot = pop();
		if (slot == -1) {
			val = 0;
		} else {
			assert(_hInFileTable[slot]);
			val = _hInFileTable[slot]->readUint32LE();
		}

		push(val);
		break;
	case SO_ARRAY:
		fetchScriptByte();
		size = pop();
		slot = pop();
		val = readFileToArray(slot, size);

		push(val);
		break;
	default:
		error("o72_readFile: default case %d", subOp);
	}
}

void ScummEngine_v72he::writeFileFromArray(int slot, int32 resID) {
	ArrayHeader *ah = (ArrayHeader *)getResourceAddress(rtString, resID);
	int32 size = (FROM_LE_32(ah->acrossMax) - FROM_LE_32(ah->acrossMin) + 1) *
		(FROM_LE_32(ah->downMax) - FROM_LE_32(ah->downMin) + 1);

	if (slot != -1) {
		assert(_hOutFileTable[slot]);
		_hOutFileTable[slot]->write(ah->data, size);
	}
}

void ScummEngine_v72he::getStringFromArray(int arrayNumber, char *buffer, int maxLength) {
	// I'm not really sure it belongs here and not some other version
	// this is ARRAY_GetStringFromArray() from ARRAYS.cpp of SPUTM

	// this function makes a C-string out of <arrayNumber> contents

	VAR(0) = arrayNumber; // it was 0 in original code, but I've seen ScummVM Moonbase code which uses VAR_U32_ARRAY_UNK

	int i, ch;
	for (i = 0; i < maxLength; ++i) {
		if (!(ch = readArray(0, 0, i))) {
			break;
		}

		buffer[i] = ch;
	}

	buffer[i] = 0;
}

void ScummEngine_v72he::o72_writeFile() {
	int32 resID = pop();
	int slot = pop();
	byte subOp = fetchScriptByte();

	// The original doesn't make assumptions of any
	// kind when the slot is -1 (which is a possible
	// value from the scripts) and does NOPs instead...
	if (slot != -1)
		assert(_hOutFileTable[slot]);

	// Arrays will handle the -1 value by themselves...
	if (slot == -1 && subOp != SO_ARRAY)
		return;

	switch (subOp) {
	case SO_BYTE:
		_hOutFileTable[slot]->writeByte(resID);
		break;
	case SO_INT:
		_hOutFileTable[slot]->writeUint16LE(resID);
		break;
	case SO_DWORD:
		_hOutFileTable[slot]->writeUint32LE(resID);
		break;
	case SO_ARRAY:
		fetchScriptByte();
		writeFileFromArray(slot, resID);
		break;
	default:
		error("o72_writeFile: default case %d", subOp);
	}
}

void ScummEngine_v72he::o72_findAllObjects() {
	int room = pop();
	int i;

	if (room != _currentRoom)
		error("o72_findAllObjects: current room is not %d", room);

	writeVar(0, 0);
	defineArray(0, kDwordArray, 0, 0, 0, _numLocalObjects);
	writeArray(0, 0, 0, _numLocalObjects);

	for (i = 1; i < _numLocalObjects; i++) {
		writeArray(0, 0, i, _objs[i].obj_nr);
	}

	push(readVar(0));
}

void ScummEngine_v72he::o72_deleteFile() {
	byte buffer[256];

	copyScriptString(buffer, sizeof(buffer));

	debug(1, "o72_deleteFile(%s)", (char *)buffer);

	deleteSaveFile(buffer);
}

void ScummEngine_v72he::o72_rename() {
	byte buffer1[100],buffer2[100];

	copyScriptString(buffer1, sizeof(buffer1));
	copyScriptString(buffer2, sizeof(buffer2));

	debug(1, "o72_rename(%s to %s)", (char *)buffer2, (char *)buffer1);

	renameSaveFile(buffer2, buffer1);
}

void ScummEngine_v72he::o72_getPixel() {
	uint16 area;

	int y = pop();
	int x = pop();
	byte subOp = fetchScriptByte();

	VirtScreen *vs = findVirtScreen(y);
	if (vs == NULL || x > _screenWidth - 1 || x < 0) {
		push(-1);
		return;
	}

	switch (subOp) {
	case ScummEngine_v100he::SO_BACKGROUND_ON: // HE 100
	case SO_BACKGROUND_ON:
		if (_game.features & GF_16BIT_COLOR)
			area = READ_UINT16(vs->getBackPixels(x, y - vs->topline));
		else
			area = *vs->getBackPixels(x, y - vs->topline);
		break;
	case ScummEngine_v100he::SO_BACKGROUND_OFF: // HE 100
	case SO_BACKGROUND_OFF:
		if (_game.features & GF_16BIT_COLOR)
			area = READ_UINT16(vs->getPixels(x, y - vs->topline));
		else
			area = *vs->getPixels(x, y - vs->topline);
		break;
	default:
		error("o72_getPixel: default case %d", subOp);
	}
	push(area);
}

void ScummEngine_v72he::o72_pickVarRandom() {
	int num;
	int args[100];
	int32 acrossMax;

	num = getStackList(args, ARRAYSIZE(args));
	int value = fetchScriptWord();

	if (readVar(value) == 0) {
		defineArray(value, kDwordArray, 0, 0, 0, num);
		if (num > 0) {
			int16 counter = 0;
			do {
				writeArray(value, 0, counter + 1, args[counter]);
			} while (++counter < num);
		}

		shuffleArray(value, 1, num);
		writeArray(value, 0, 0, 2);
		push(readArray(value, 0, 1));
		return;
	}

	num = readArray(value, 0, 0);

	ArrayHeader *ah = (ArrayHeader *)getResourceAddress(rtString, readVar(value));
	acrossMax = FROM_LE_32(ah->acrossMax);

	if (acrossMax < num) {
		int32 var_2 = readArray(value, 0, num - 1);
		shuffleArray(value, 1, acrossMax);
		if (readArray(value, 0, 1) == var_2) {
			num = 2;
		} else {
			num = 1;
		}
	}

	writeArray(value, 0, 0, num + 1);
	push(readArray(value, 0, num));
}

void ScummEngine_v72he::o72_redimArray() {
	int newX, newY;
	newY = pop();
	newX = pop();

	byte subOp = fetchScriptByte();

	switch (subOp) {
	case SO_BYTE:
		redimArray(fetchScriptWord(), 0, newX, 0, newY, kByteArray);
		break;
	case SO_INT:
		redimArray(fetchScriptWord(), 0, newX, 0, newY, kIntArray);
		break;
	case SO_DWORD:
		redimArray(fetchScriptWord(), 0, newX, 0, newY, kDwordArray);
		break;
	default:
		error("o72_redimArray: default type %d", subOp);
	}
}

void ScummEngine_v72he::redimArray(int arrayId, int newDim2start, int newDim2end,
								   int newDim1start, int newDim1end, int type) {
	int newSize, oldSize;
	int rawArray = readVar(arrayId);
	if (rawArray == 0)
		error("redimArray: Reference to zeroed array pointer");

	if (_game.heversion >= 80) {
		if ((rawArray & MAGIC_ARRAY_NUMBER) != MAGIC_ARRAY_NUMBER) {
			error("redimArray: Illegal array pointer not having magic number (%d,%d)", arrayId, rawArray);
		}

		rawArray &= ~MAGIC_ARRAY_NUMBER;
	}

	ArrayHeader *ah = (ArrayHeader *)getResourceAddress(rtString, rawArray);

	if (!ah)
		error("redimArray: Invalid array (%d) reference", rawArray);

	newSize = arrayDataSizes[type] / 8;
	oldSize = arrayDataSizes[FROM_LE_32(ah->type)] / 8;

	newSize *= (newDim1end - newDim1start + 1) * (newDim2end - newDim2start + 1);
	oldSize *= (FROM_LE_32(ah->acrossMax) - FROM_LE_32(ah->acrossMin) + 1) *
		(FROM_LE_32(ah->downMax) - FROM_LE_32(ah->downMin) + 1);

	if (newSize != oldSize)
		error("redimArray: array %d redim mismatch", rawArray);

	ah->type = TO_LE_32(type);
	ah->acrossMin = TO_LE_32(newDim1start);
	ah->acrossMax = TO_LE_32(newDim1end);
	ah->downMin = TO_LE_32(newDim2start);
	ah->downMax = TO_LE_32(newDim2end);
}

void ScummEngine_v72he::checkArrayLimits(int array, int downMin, int downMax, int acrossMin, int acrossMax) {
	if (acrossMax < acrossMin) {
		error("Across max %d smaller than min %d", acrossMax, acrossMin);
	}

	if (downMax < downMin) {
		error("Down max %d smaller than min %d", downMax, downMin);
	}

	ArrayHeader *ah = (ArrayHeader *)getResourceAddress(rtString, readVar(array));
	assert(ah);
	if ((int)FROM_LE_32(ah->downMin) > downMin || (int)FROM_LE_32(ah->downMax) < downMax || (int)FROM_LE_32(ah->acrossMin) > acrossMin || (int)FROM_LE_32(ah->acrossMax) < acrossMax) {
		error("Invalid array access (%d,%d,%d,%d) limit (%d,%d,%d,%d)", downMin, downMax, acrossMin, acrossMax, FROM_LE_32(ah->downMin), FROM_LE_32(ah->downMax), FROM_LE_32(ah->acrossMin), FROM_LE_32(ah->acrossMax));
	}
}

void ScummEngine_v72he::copyArray(int dstVariable, int dstDownMin, int dstDownMax, int dstAcrossMin, int dstAcrossMax,
								  int srcVariable, int srcDownMin, int srcDownMax, int srcAcrossMin, int srcAcrossMax) {
	byte *dstPtr, *srcPtr;
	int dstOffset, srcOffset;
	int dataSize;

	checkArrayLimits(dstVariable, dstDownMin, dstDownMax, dstAcrossMin, dstAcrossMax);
	checkArrayLimits(srcVariable, srcDownMin, srcDownMax, srcAcrossMin, srcAcrossMax);

	int dstDownCount = dstDownMax - dstDownMin + 1;
	int dstAcrossCount = dstAcrossMax - dstAcrossMin + 1;
	int srcDownCount = srcDownMax - srcDownMin + 1;
	int srcAcrossCount = srcAcrossMax - srcAcrossMin + 1;

	if (srcDownCount != dstDownCount || srcAcrossCount != dstAcrossCount) {
		error("Operation size mismatch (%d vs %d)(%d vs %d)", dstDownCount, srcDownCount, dstAcrossCount, srcAcrossCount);
	}

	if (dstVariable != srcVariable) {
		ArrayHeader *dstArray = (ArrayHeader *)getResourceAddress(rtString, readVar(dstVariable));
		assert(dstArray);
		ArrayHeader *srcArray = (ArrayHeader *)getResourceAddress(rtString, readVar(srcVariable));
		assert(srcArray);

		if (FROM_LE_32(dstArray->type) == FROM_LE_32(srcArray->type)) {
			getArrayDataPtrAndDataSize(dstArray, dstDownMin, dstAcrossMin, dstAcrossMax, &dstPtr, &dstOffset, &dataSize);
			getArrayDataPtrAndDataSize(srcArray, srcDownMin, srcAcrossMin, srcAcrossMax, &srcPtr, &srcOffset, &dataSize);

			for (int dstDownCounter = dstDownMin; dstDownCounter <= dstDownMax; dstDownCounter++) {
				memcpy(dstPtr, srcPtr, dataSize);
				dstPtr += dstOffset;
				srcPtr += srcOffset;
			}
		} else {
			int srcDownIndex = srcDownMin;

			for (int dstDownCounter = dstDownMin; dstDownCounter <= dstDownMax; dstDownCounter++) {
				int srcAcrossIndex = srcAcrossMin;

				for (int dstAcrossCounter = dstAcrossMin; dstAcrossCounter <= dstAcrossMax; dstAcrossCounter++) {
					writeArray(dstVariable, dstDownCounter, dstAcrossCounter,
						readArray(srcVariable, srcDownIndex, srcAcrossIndex++));
				}

				srcDownIndex++;
			}
		}
	} else {
		if (srcDownMin != dstDownMin || srcAcrossMin != dstAcrossMin) {
			int dstArray = readVar(dstVariable);

			ArrayHeader *dstArrayPtr = (ArrayHeader *)getResourceAddress(rtString, dstArray & ~MAGIC_ARRAY_NUMBER);
			ArrayHeader *srcArrayPtr = dstArrayPtr;

			if (!dstArrayPtr) {
				error("Missing array (%d,%d,4) reference", (dstArray & ~MAGIC_ARRAY_NUMBER), dstArray);
			}

			bool useMemcpy = false;
			if ((dstDownMin < srcDownMin) || _game.heversion > 99) {
				useMemcpy = true;

				getArrayDataPtrAndDataSize(dstArrayPtr, dstDownMin, dstAcrossMin, dstAcrossMax, &dstPtr, &dstOffset, &dataSize);
				getArrayDataPtrAndDataSize(srcArrayPtr, srcDownMin, srcAcrossMin, srcAcrossMax, &srcPtr, &srcOffset, &dataSize);
			} else {
				getArrayDataPtrAndDataSize(dstArrayPtr, dstDownMax, dstAcrossMin, dstAcrossMax, &dstPtr, &dstOffset, &dataSize);
				getArrayDataPtrAndDataSize(srcArrayPtr, srcDownMax, srcAcrossMin, srcAcrossMax, &srcPtr, &srcOffset, &dataSize);

				useMemcpy = (dstAcrossMin <= srcAcrossMin);
			}
			
			if (useMemcpy) {
				for (int dstDownCounter = dstDownMin; dstDownCounter <= dstDownMax; dstDownCounter++) {
					memcpy(dstPtr, srcPtr, dataSize);
					dstPtr += dstOffset;
					srcPtr += srcOffset;
				}
			} else {
				for (int dstDownCounter = dstDownMin; dstDownCounter <= dstDownMax; dstDownCounter++) {
					memmove(dstPtr, srcPtr, dataSize);
					dstPtr += dstOffset;
					srcPtr += srcOffset;
				}
			}
		}
	}
}

void ScummEngine_v72he::getArrayDataPtrAndDataSize(ArrayHeader *headerPtr, int down, int aMin, int aMax, byte **ptrPtr, int *dataOffsetPtr, int *dataSizePtr) {
	const int acrossCount = FROM_LE_32(headerPtr->acrossMax) - FROM_LE_32(headerPtr->acrossMin) + 1;
	const int index = acrossCount * (down - FROM_LE_32(headerPtr->downMin)) + aMin - FROM_LE_32(headerPtr->acrossMin);

	switch (FROM_LE_32(headerPtr->type)) {
	case kByteArray:
	case kStringArray:
		*dataSizePtr = aMax - aMin + 1;
		*dataOffsetPtr = acrossCount;
		*ptrPtr = headerPtr->data + index;
		break;
	case kIntArray:
		*dataSizePtr = (aMax - aMin) * 2 + 2;
		*dataOffsetPtr = acrossCount * 2;
		*ptrPtr = headerPtr->data + index * 2;
		break;
	case kDwordArray:
		*dataSizePtr = (aMax - aMin) * 4 + 4;
		*dataOffsetPtr = acrossCount * 4;
		*ptrPtr = headerPtr->data + index * 4;
		break;
	default:
		error("Invalid array type %d", FROM_LE_32(headerPtr->type));
	}
}

void ScummEngine_v72he::o72_readINI() {
	byte option[128];
	byte *data;

	copyScriptString(option, sizeof(option));
	byte subOp = fetchScriptByte();

	switch (subOp) {
	case ScummEngine_v100he::SO_DWORD: // HE 100
	case SO_DWORD: // number
		if (!strcmp((char *)option, "DisablePrinting") || !strcmp((char *)option, "NoPrinting")) {
			push(1);
		} else if (!strcmp((char *)option, "DisableMaiaUpdates")) {
			// WORKAROUND: Override the update checks.
			// This gets checked in Baseball 2001 and will check for
			// updates before connecting to the servers, since we
			// don't support game updates and such updates appears to
			// be lost, skip through the whole thing.  This disables
			// the update button on the login screen as well.
			push(1);
		} else if (!strcmp((char *)option, "InternetConnect")) {
			// WORKAROUND: In the patched version of Backyard
			// Football 2002, they added an option to join a game
			// over the internet by manually inputing an IP address.
			// Our network implementation does not support this currently,
			// and we have our own way of connecting to joins over the
			// Internet anyways, so force disable it.
			push(0);
		} else if (!strcmp((char *)option, "TextOn")) {
			push(ConfMan.getBool("subtitles"));
		} else if (!strcmp((char *)option, "Disk") && (_game.id == GID_BIRTHDAYRED || _game.id == GID_BIRTHDAYYELLOW)) {
			// WORKAROUND: Override the disk detection
			// This removes the reliance on having the binary file around (which is
			// very bad for the Mac version) just for the scripts to figure out if
			// we're running Yellow or Red
			if (_game.id == GID_BIRTHDAYRED)
				push(4);
			else
				push(2);
		} else {
			push(ConfMan.getInt((char *)option));
		}
		break;
	case ScummEngine_v100he::SO_STRING: // HE 100
	case SO_STRING: // string
		writeVar(0, 0);
		if (!strcmp((char *)option, "HE3File")) {
			Common::String fileName = generateFilename(-3).toString('/');
			int len = resStrLen((const byte *)fileName.c_str());
			data = defineArray(0, kStringArray, 0, 0, 0, len);
			memcpy(data, fileName.c_str(), len);
		} else if (!strcmp((char *)option, "GameResourcePath") || !strcmp((char *)option, "SaveGamePath")) {
			// We set SaveGamePath in order to detect where it used
			// in convertFilePath and to avoid warning about invalid
			// path in Macintosh versions.
			data = defineArray(0, kStringArray, 0, 0, 0, 2);
			if (_game.platform == Common::kPlatformMacintosh)
				memcpy(data, (const char *)"*:", 2);
			else
				memcpy(data, (const char *)"*\\", 2);
		} else {
			const char *entry = (ConfMan.get((char *)option).c_str());
			int len = resStrLen((const byte *)entry);
			data = defineArray(0, kStringArray, 0, 0, 0, len);
			memcpy(data, entry, len);
		}
		push(readVar(0));
		break;
	default:
		error("o72_readINI: default type %d", subOp);
	}

	debug(1, "o72_readINI: Option %s", option);
}

void ScummEngine_v72he::o72_writeINI() {
	int value;
	byte option[256], string[1024];

	byte subOp = fetchScriptByte();

	switch (subOp) {
	case ScummEngine_v100he::SO_DWORD: // HE 100
	case SO_DWORD: // number
		value = pop();
		copyScriptString(option, sizeof(option));
		debug(1, "o72_writeINI: Option %s Value %d", option, value);

		ConfMan.setInt((char *)option, value);
		break;
	case ScummEngine_v100he::SO_STRING: // HE 100
	case SO_STRING: // string
		copyScriptString(string, sizeof(string));
		copyScriptString(option, sizeof(option));
		debug(1, "o72_writeINI: Option %s String %s", option, string);

		// Filter out useless setting
		if (!strcmp((char *)option, "HETest"))
			return;

		// Filter out confusing subtitle setting
		if (!strcmp((char *)option, "TextOn"))
			return;

		// Filter out confusing path settings
		if (!strcmp((char *)option, "DownLoadPath") || !strcmp((char *)option, "GameResourcePath") || !strcmp((char *)option, "SaveGamePath"))
			return;

		ConfMan.set((char *)option, (char *)string);
		break;
	default:
		error("o72_writeINI: default type %d", subOp);
	}

	ConfMan.flushToDisk();
}

void ScummEngine_v72he::o72_getResourceSize() {
	const byte *ptr;
	int size;
	ResType type;

	int resid = pop();
	if (_game.heversion == 72) {
		push(getSoundResourceSize(resid));
		return;
	}

	byte subOp = fetchScriptByte();

	switch (subOp) {
	case SO_SOUND_SIZE:
		push(getSoundResourceSize(resid));
		return;
	case SO_ROOM_SIZE:
		type = rtRoomImage;
		break;
	case SO_IMAGE_SIZE:
		type = rtImage;
		break;
	case SO_COSTUME_SIZE:
		type = rtCostume;
		break;
	case SO_SCRIPT_SIZE:
		type = rtScript;
		break;
	default:
		error("o72_getResourceSize: default type %d", subOp);
	}

	ptr = getResourceAddress(type, resid);
	assert(ptr);
	size = READ_BE_UINT32(ptr + 4) - 8;
	push(size);
}

void ScummEngine_v72he::o72_createDirectory() {
	byte directoryName[255];

	copyScriptString(directoryName, sizeof(directoryName));
	debug(1,"o72_createDirectory: %s", directoryName);
}

void ScummEngine_v72he::o72_setSystemMessage() {
	byte name[1024];

	copyScriptString(name, sizeof(name));
	byte subOp = fetchScriptByte();

	switch (subOp) {
	case SO_PAUSE_MESSAGE:
		debug(1,"o72_setSystemMessage: (%d) %s", subOp, name);
		break;
	case SO_PAUSE_TITLE: // Set Version
		debug(1,"o72_setSystemMessage: (%d) %s", subOp, name);
		break;
	case SO_PAUSE_OPTION:
		debug(1,"o72_setSystemMessage: (%d) %s", subOp, name);
		break;
	case SO_TITLE_BAR: // Set Window Caption
		// TODO: The 'name' string can contain non-ASCII data. This can lead to
		// problems, because the encoding used for "name" is not clear.
		//
		// Possible fixes/workarounds:
		// - Simply stop using this. It's a rather unimportant "feature" anyway.
		// - Try to translate the text to UTF-32.
		//_system->setWindowCaption(Common::U32String((const char *)name));
		break;
	default:
		error("o72_setSystemMessage: default case %d", subOp);
	}
}

void ScummEngine_v72he::decodeParseString(int m, int n) {
	Actor *a;
	int i, colors, size;
	int args[31];
	byte name[1024];

	byte b = fetchScriptByte();

	switch (b) {
	case ScummEngine_v6::SubOpType::SO_AT:
		_string[m].ypos = pop();
		_string[m].xpos = pop();
		_string[m].overhead = false;
		break;
	case SO_COLOR:
		_string[m].color = pop();
		break;
	case SO_CLIPPED:
		_string[m].right = pop();
		break;
	case SO_CENTER:
		_string[m].center = true;
		_string[m].overhead = false;
		break;
	case SO_LEFT:
		_string[m].center = false;
		_string[m].overhead = false;
		break;
	case SO_OVERHEAD:
		_string[m].overhead = true;
		_string[m].no_talk_anim = false;
		break;
	case SO_MUMBLE:
		_string[m].no_talk_anim = true;
		break;
	case SO_TEXTSTRING:
		printString(m, _scriptPointer);
		_scriptPointer += resStrLen(_scriptPointer) + 1;
		break;
	case SO_FORMATTED_STRING:
		decodeScriptString(name, true);
		printString(m, name);
		break;
	case SO_TALKIE:
		{
		byte *dataPtr = getResourceAddress(rtTalkie, pop());
		byte *text = findWrappedBlock(MKTAG('T','E','X','T'), dataPtr, 0, 0);
		size = getResourceDataSize(text);
		memcpy(name, text, size);
		printString(m, name);
		}
		break;
	case SO_COLOR_LIST:
		colors = pop();
		if (colors == 1) {
			_string[m].color = pop();
			// WORKAROUND bug #13730: defined subtitles color 16 is very dark and hard to read on the dark background.
			// we change it to brighter color to ease reading.
			if (_game.id == GID_FREDDI4 && _game.heversion == 98 && _currentRoom == 43 && _string[m].color == 16 && enhancementEnabled(kEnhSubFmtCntChanges))
				_string[m].color = 200;
		} else {
			push(colors);
			getStackList(args, ARRAYSIZE(args));
			for (i = 0; i < 16; i++)
				_charsetColorMap[i] = _charsetData[_string[m]._default.charset][i] = (unsigned char)args[i];
			_string[m].color = _charsetColorMap[0];
		}
		break;
	case SO_BASEOP:
		_string[m].loadDefault();
		if (n) {
			_actorToPrintStrFor = pop();
			if (_actorToPrintStrFor != 0xFF) {
				a = derefActor(_actorToPrintStrFor, "decodeParseString");
				_string[m].color = a->_talkColor;
			}
		}
		break;
	case SO_END:
		_string[m].saveDefault();
		break;
	default:
		error("decodeParseString: default case 0x%x", b);
	}
}

} // End of namespace Scumm

#endif // ENABLE_HE
