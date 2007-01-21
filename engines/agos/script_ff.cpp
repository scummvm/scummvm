/* ScummVM - Scumm Interpreter
 * Copyright (C) 2001  Ludvig Strigeus
 * Copyright (C) 2001-2006 The ScummVM project
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

#include "common/stdafx.h"

#include "common/system.h"

#include "agos/animation.h"
#include "agos/agos.h"

namespace AGOS {

void AGOSEngine::setupFeebleOpcodes(OpcodeProc *op) {
	setupCommonOpcodes(op);

	op[23] = &AGOSEngine::off_chance;
	op[37] = &AGOSEngine::off_jumpOut;
	op[65] = &AGOSEngine::off_addTextBox;
	op[66] = &AGOSEngine::oww_setShortText;
	op[67] = &AGOSEngine::oww_setLongText;
	op[70] = &AGOSEngine::off_printLongText;
	op[83] = &AGOSEngine::os2_rescan;
	op[98] = &AGOSEngine::os2_animate;
	op[99] = &AGOSEngine::os2_stopAnimate;
	op[107] = &AGOSEngine::off_addBox;
	op[122] = &AGOSEngine::off_oracleTextDown;
	op[123] = &AGOSEngine::off_oracleTextUp;
	op[124] = &AGOSEngine::off_ifTime;
	op[131] = &AGOSEngine::off_setTime;
	op[132] = &AGOSEngine::off_saveUserGame;
	op[133] = &AGOSEngine::off_loadUserGame;
	op[134] = &AGOSEngine::off_listSaveGames;
	op[135] = &AGOSEngine::off_checkCD;
	op[161] = &AGOSEngine::off_screenTextBox;
	op[162] = &AGOSEngine::os1_screenTextMsg;
	op[164] = &AGOSEngine::oe2_getDollar2;
	op[165] = &AGOSEngine::off_isAdjNoun;
	op[166] = &AGOSEngine::oe2_b2Set;
	op[167] = &AGOSEngine::oe2_b2Clear;
	op[168] = &AGOSEngine::oe2_b2Zero;
	op[169] = &AGOSEngine::oe2_b2NotZero;
	op[171] = &AGOSEngine::off_hyperLinkOn;
	op[172] = &AGOSEngine::off_hyperLinkOff;
	op[173] = &AGOSEngine::off_checkPaths;
	op[175] = &AGOSEngine::oww_lockZones;
	op[176] = &AGOSEngine::oww_unlockZones;
	op[177] = &AGOSEngine::off_screenTextPObj;
	op[178] = &AGOSEngine::os1_getPathPosn;
	op[179] = &AGOSEngine::os1_scnTxtLongText;
	op[180] = &AGOSEngine::off_mouseOn;
	op[181] = &AGOSEngine::off_mouseOff;
	op[182] = &AGOSEngine::off_loadVideo;
	op[183] = &AGOSEngine::off_playVideo;
	op[184] = &AGOSEngine::os1_unloadZone;
	op[186] = &AGOSEngine::os1_unfreezeZones;
	op[187] = &AGOSEngine::off_centreScroll;
	op[188] = &AGOSEngine::os2_isShortText;
	op[189] = &AGOSEngine::os2_clearMarks;
	op[190] = &AGOSEngine::os2_waitMark;
	op[191] = &AGOSEngine::off_resetPVCount;
	op[192] = &AGOSEngine::off_setPathValues;
	op[193] = &AGOSEngine::off_stopClock;
	op[194] = &AGOSEngine::off_restartClock;
	op[195] = &AGOSEngine::off_setColour;
	op[196] = &AGOSEngine::off_b3Set;
	op[197] = &AGOSEngine::off_b3Clear;
	op[198] = &AGOSEngine::off_b3Zero;
	op[199] = &AGOSEngine::off_b3NotZero;
}

// -----------------------------------------------------------------------
// Feeble Files Opcodes
// -----------------------------------------------------------------------

void AGOSEngine::off_chance() {
	// 23
	uint a = getVarOrWord();

	if (a == 0) {
		setScriptCondition(false);
		return;
	}

	if (a == 100) {
		setScriptCondition(true);
		return;
	}

	if ((uint)_rnd.getRandomNumber(99) < a)
		setScriptCondition(true);
	else
		setScriptCondition(false);
}

void AGOSEngine::off_jumpOut() {
	// 37
	getVarOrByte();
	setScriptReturn(1);
}

void AGOSEngine::off_addTextBox() {
	// 65: add hit area
	uint flags = kBFTextBox | kBFBoxItem;
	uint id = getVarOrWord();
	uint params = id / 1000;
	uint x, y, w, h, num;

	id %= 1000;

	if (params & 1)
		flags |= kBFInvertTouch;

	x = getVarOrWord();
	y = getVarOrWord();
	w = getVarOrWord();
	h = getVarOrWord();
	num = getVarOrByte();
	if (num < _numTextBoxes)
		defineBox(id, x, y, w, h, flags + (num << 8), 208, _dummyItem2);
}

void AGOSEngine::off_printLongText() {
	// 70: show string from array
	int num = getVarOrByte();
	const char *str = (const char *)getStringPtrByID(_longText[num]);
	sendInteractText(num, "%d. %s\n", num, str);
}

void AGOSEngine::off_addBox() {
	// 107: add item hitarea
	uint flags = 0;
	uint id = getVarOrWord();
	uint params = id / 1000;
	uint x, y, w, h, verb;
	Item *item;

	id = id % 1000;

	if (params & 1)
		flags |= kBFInvertTouch;
	if (params & 2)
		flags |= kBFNoTouchName;
	if (params & 4)
		flags |= kBFBoxItem;
	if (params & 8)
		flags |= kBFTextBox;
	if (params & 16)
		flags |= kBFHyperBox;

	x = getVarOrWord();
	y = getVarOrWord();
	w = getVarOrWord();
	h = getVarOrWord();
	item = getNextItemPtrStrange();
	verb = getVarOrWord();
	defineBox(id, x, y, w, h, flags, verb, item);
}

void AGOSEngine::off_oracleTextDown() {
	// 122: oracle text down
	oracleTextDown();
}

void AGOSEngine::off_oracleTextUp() {
	// 123: oracle text up
	oracleTextUp();
}

void AGOSEngine::off_ifTime() {
	// 124: if time
	time_t t;

	uint a = getVarOrWord();
	time(&t);
	t -= _gameStoppedClock;
	t -= a;
	if (t >= _timeStore)
		setScriptCondition(true);
	else
		setScriptCondition(false);
}

void AGOSEngine::off_setTime() {
	// 131
	time(&_timeStore);
	_timeStore -= _gameStoppedClock;
}

void AGOSEngine::off_saveUserGame() {
	// 132: save game
	_noOracleScroll = 0;
	_system->setFeatureState(OSystem::kFeatureVirtualKeyboard, true);
	saveUserGame(countSaveGames() + 1 - readVariable(55));
	_system->setFeatureState(OSystem::kFeatureVirtualKeyboard, false);
}

void AGOSEngine::off_loadUserGame() {
	// 133: load game
	if (readVariable(55) == 999) {
		loadGame(getFileName(GAME_RESTFILE), true);
	} else {
		loadGame(genSaveName(readVariable(55)));
	}
}

void AGOSEngine::off_listSaveGames() {
	// 134: dummy opcode?
	listSaveGames(1);
}

void AGOSEngine::off_checkCD() {
	// 135: switch CD
	uint disc = readVariable(97);

	if (!strcmp(getExtra(), "4CD")) {
		_sound->switchVoiceFile(gss, disc);
	} else if (!strcmp(getExtra(), "2CD")) {
		if (disc == 1 || disc == 2)
			_sound->switchVoiceFile(gss, 1);
		else if (disc == 3 || disc == 4)
			_sound->switchVoiceFile(gss, 2);
	}

	debug(0, "Switch to CD number %d", disc);
}

void AGOSEngine::off_screenTextBox() {
	// 161: setup text
	TextLocation *tl = getTextLocation(getVarOrByte());

	tl->x = getVarOrWord();
	tl->y = getVarOrWord();
	tl->width = getVarOrWord();
}

void AGOSEngine::off_isAdjNoun() {
	// 165: item unk1 unk2 is
	Item *item = getNextItemPtr();
	int16 a = getNextWord(), b = getNextWord();
	if (item->adjective == a && item->noun == b)
		setScriptCondition(true);
	else if (a == -1 && item->noun == b)
		setScriptCondition(true);
	else
		setScriptCondition(false);
}

void AGOSEngine::off_hyperLinkOn() {
	// 171: oracle hyperlink on
	hyperLinkOn(getVarOrWord());
}

void AGOSEngine::off_hyperLinkOff() {
	// 172: oracle hyperlink off
	hyperLinkOff();
}

void AGOSEngine::off_checkPaths() {
	// 173 check paths
	int i, count;
	const uint8 *pathVal1 = _pathValues1;
	bool result = false;

	count = _variableArray2[38];
	for (i = 0; i < count; i++) {
		uint8 val = pathVal1[2];
		if (val == _variableArray2[50] ||
			val == _variableArray2[51] ||
			val == _variableArray2[201] ||
			val == _variableArray2[203] ||
			val == _variableArray2[205] ||
			val == _variableArray2[207] ||
			val == _variableArray2[209] ||
			val == _variableArray2[211] ||
			val == _variableArray2[213] ||
			val == _variableArray2[215] ||
			val == _variableArray2[219] ||
			val == _variableArray2[220] ||
			val == _variableArray2[221] ||
			val == _variableArray2[222] ||
			val == _variableArray2[223] ||
			val == _variableArray2[224] ||
			val == _variableArray2[225] ||
			val == _variableArray2[226]) {
				result = true;
				break;
		}
		pathVal1 += 4;
	}

	_variableArray2[52] = result;
}

void AGOSEngine::off_screenTextPObj() {
	// 177: inventory descriptions
	uint vgaSpriteId = getVarOrByte();
	uint color = getVarOrByte();
	const char *string_ptr = NULL;
	TextLocation *tl = NULL;
	char buf[256];

	SubObject *subObject = (SubObject *)findChildOfType(getNextItemPtr(), 2);
	if (subObject != NULL && subObject->objectFlags & kOFText) {
		string_ptr = (const char *)getStringPtrByID(subObject->objectFlagValue[0]);
		tl = getTextLocation(vgaSpriteId);
	}

	if (subObject != NULL && subObject->objectFlags & kOFVoice) {
		uint offs = getOffsetOfChild2Param(subObject, kOFVoice);
		playSpeech(subObject->objectFlagValue[offs], vgaSpriteId);
	}

	if (subObject != NULL && (subObject->objectFlags & kOFText) && _subtitles) {
		if (subObject->objectFlags & kOFNumber) {
			sprintf(buf, "%d%s", subObject->objectFlagValue[getOffsetOfChild2Param(subObject, kOFNumber)], string_ptr);
			string_ptr = buf;
		}
		if (string_ptr != NULL)
			printScreenText(vgaSpriteId, color, string_ptr, tl->x, tl->y, tl->width);
	}
}

void AGOSEngine::off_mouseOn() {
	// 180: force mouseOn
	if (_mouseCursor != 5) {
		resetVerbs();
		_noRightClick = 0;
	}
	_mouseHideCount = 0;
}

void AGOSEngine::off_mouseOff() {
	// 181: force mouseOff
	scriptMouseOff();
	clearName();
}

void AGOSEngine::off_loadVideo() {
	// 182: load video file
	const byte *filename = getStringPtrByID(getNextStringID());
	_moviePlay->load((const char *)filename);
}

void AGOSEngine::off_playVideo() {
	// 183: play video
	_moviePlay->play();
}

void AGOSEngine::off_centreScroll() {
	// 187
	centreScroll();
}

void AGOSEngine::off_resetPVCount() {
	// 191
	if (getBitFlag(83)) {
		_PVCount1 = 0;
		_GPVCount1 = 0;
	} else {
		_PVCount = 0;
		_GPVCount = 0;
	}
}

void AGOSEngine::off_setPathValues() {
	// 192
	uint8 a = getVarOrByte();
	uint8 b = getVarOrByte();
	uint8 c = getVarOrByte();
	uint8 d = getVarOrByte();
	if (getBitFlag(83)) {
		_pathValues1[_PVCount1++] = a;
		_pathValues1[_PVCount1++] = b;
		_pathValues1[_PVCount1++] = c;
		_pathValues1[_PVCount1++] = d;
	} else {
		_pathValues[_PVCount++] = a;
		_pathValues[_PVCount++] = b;
		_pathValues[_PVCount++] = c;
		_pathValues[_PVCount++] = d;
	}
}

void AGOSEngine::off_stopClock() {
	// 193: pause clock
	_clockStopped = time(NULL);
}

void AGOSEngine::off_restartClock() {
	// 194: resume clock
	if (_clockStopped != 0)
		_gameStoppedClock += time(NULL) - _clockStopped;
	_clockStopped = 0;
}

void AGOSEngine::off_setColour() {
	// 195: set palette colour
	uint c = getVarOrByte() * 4;
	uint r = getVarOrByte();
	uint g = getVarOrByte();
	uint b = getVarOrByte();

	_displayPalette[c + 0] = r;
	_displayPalette[c + 1] = g;
	_displayPalette[c + 2] = b;

	_paletteFlag = 2;
}

void AGOSEngine::off_b3Set() {
	// 196: set bit3
	uint bit = getVarOrByte();
	_bitArrayThree[bit / 16] |= (1 << (bit & 15));
}

void AGOSEngine::off_b3Clear() {
	// 197: clear bit3
	uint bit = getVarOrByte();
	_bitArrayThree[bit / 16] &= ~(1 << (bit & 15));
}

void AGOSEngine::off_b3Zero() {
	// 198: is bit3 clear
	uint bit = getVarOrByte();
	setScriptCondition((_bitArrayThree[bit / 16] & (1 << (bit & 15))) == 0);
}

void AGOSEngine::off_b3NotZero() {
	// 199: is bit3 set
	uint bit = getVarOrByte();
	setScriptCondition((_bitArrayThree[bit / 16] & (1 << (bit & 15))) != 0);
}

} // End of namespace AGOS
