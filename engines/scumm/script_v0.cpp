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


#include "scumm/actor.h"
#include "scumm/charset.h"
#include "scumm/object.h"
#include "scumm/scumm_v0.h"
#include "scumm/verbs.h"

namespace Scumm {

#define OPCODE(i, x)	_opcodes[i]._OPCODE(ScummEngine_v0, x)

void ScummEngine_v0::setupOpcodes() {
	/* 00 */
	OPCODE(0x00, o5_stopObjectCode);
	OPCODE(0x01, o2_putActor);
	OPCODE(0x02, o5_startMusic);
	OPCODE(0x03, o_doSentence);
	/* 04 */
	OPCODE(0x04, o2_isGreaterEqual);
	OPCODE(0x05, o_stopCurrentScript);
	OPCODE(0x06, o5_getDist);
	OPCODE(0x07, o5_getActorRoom);
	/* 08 */
	OPCODE(0x08, o5_isNotEqual);
	OPCODE(0x09, o_stopCurrentScript);
	OPCODE(0x0a, o_stopCurrentScript);
	OPCODE(0x0b, o_setActorBitVar);
	/* 0C */
	OPCODE(0x0c, o_loadSound);
	OPCODE(0x0d, o_printEgo_c64);
	OPCODE(0x0e, o_putActorAtObject);
	OPCODE(0x0f, o2_clearState02);
	/* 10 */
	OPCODE(0x10, o5_breakHere);
	OPCODE(0x11, o_animateActor);
	OPCODE(0x12, o2_panCameraTo);
	OPCODE(0x13, o_lockCostume);
	/* 14 */
	OPCODE(0x14, o_print_c64);
	OPCODE(0x15, o5_walkActorToActor);
	OPCODE(0x16, o5_getRandomNr);
	OPCODE(0x17, o2_clearState08);
	/* 18 */
	OPCODE(0x18, o5_jumpRelative);
	OPCODE(0x19, o_stopCurrentScript);
	OPCODE(0x1a, o5_move);
	OPCODE(0x1b, o_getActorBitVar);
	/* 1C */
	OPCODE(0x1c, o5_startSound);
	OPCODE(0x1d, o_setBitVar);
	OPCODE(0x1e, o2_walkActorTo);
	OPCODE(0x1f, o2_ifState04);
	/* 20 */
	OPCODE(0x20, o5_stopMusic);
	OPCODE(0x21, o2_putActor);
	OPCODE(0x22, o4_saveLoadGame);
	OPCODE(0x23, o_stopCurrentScript);
	/* 24 */
	OPCODE(0x24, o_unknown2);
	OPCODE(0x25, o5_loadRoom);
	OPCODE(0x26, o_getClosestObjActor);
	OPCODE(0x27, o2_getActorY);
	/* 28 */
	OPCODE(0x28, o5_equalZero);
	OPCODE(0x29, o_setOwnerOf);
	OPCODE(0x2a, o2_delay);
	OPCODE(0x2b, o_setActorBitVar);
	/* 2C */
	OPCODE(0x2c, o_stopCurrentScript);
	OPCODE(0x2d, o2_putActorInRoom);
	OPCODE(0x2e, o_print_c64);
	OPCODE(0x2f, o2_ifState08);
	/* 30 */
	OPCODE(0x30, o_loadCostume);
	OPCODE(0x31, o_getBitVar);
	OPCODE(0x32, o2_setCameraAt);
	OPCODE(0x33, o_lockScript);
	/* 34 */
	OPCODE(0x34, o5_getDist);
	OPCODE(0x35, o_stopCurrentScript);
	OPCODE(0x36, o2_walkActorToObject);
	OPCODE(0x37, o2_clearState04);
	/* 38 */
	OPCODE(0x38, o2_isLessEqual);
	OPCODE(0x39, o_stopCurrentScript);
	OPCODE(0x3a, o2_subtract);
	OPCODE(0x3b, o_stopCurrentScript);
	/* 3C */
	OPCODE(0x3c, o5_stopSound);
	OPCODE(0x3d, o_setBitVar);
	OPCODE(0x3e, o2_walkActorTo);
	OPCODE(0x3f, o2_ifState02);
	/* 40 */
	OPCODE(0x40, o_cutscene);
	OPCODE(0x41, o2_putActor);
	OPCODE(0x42, o2_startScript);
	OPCODE(0x43, o_doSentence);
	/* 44 */
	OPCODE(0x44, o2_isLess);
	OPCODE(0x45, o_stopCurrentScript);
	OPCODE(0x46, o5_increment);
	OPCODE(0x47, o2_getActorX);
	/* 48 */
	OPCODE(0x48, o5_isEqual);
	OPCODE(0x49, o_stopCurrentScript);
	OPCODE(0x4a, o_loadRoom);
	OPCODE(0x4b, o_setActorBitVar);
	/* 4C */
	OPCODE(0x4c, o_loadScript);
	OPCODE(0x4d, o_lockRoom);
	OPCODE(0x4e, o_putActorAtObject);
	OPCODE(0x4f, o2_clearState02);
	/* 50 */
	OPCODE(0x50, o_nop);
	OPCODE(0x51, o_animateActor);
	OPCODE(0x52, o5_actorFollowCamera);
	OPCODE(0x53, o_lockSound);
	/* 54 */
	OPCODE(0x54, o_setObjectName);
	OPCODE(0x55, o5_walkActorToActor);
	OPCODE(0x56, o_getActorMoving);
	OPCODE(0x57, o2_clearState08);
	/* 58 */
	OPCODE(0x58, o_beginOverride);
	OPCODE(0x59, o_stopCurrentScript);
	OPCODE(0x5a, o2_add);
	OPCODE(0x5b, o_getActorBitVar);
	/* 5C */
	OPCODE(0x5c, o5_startSound);
	OPCODE(0x5d, o_setBitVar);
	OPCODE(0x5e, o2_walkActorTo);
	OPCODE(0x5f, o2_ifState04);
	/* 60 */
	OPCODE(0x60, o_cursorCommand);
	OPCODE(0x61, o2_putActor);
	OPCODE(0x62, o2_stopScript);
	OPCODE(0x63, o_stopCurrentScript);
	/* 64 */
	OPCODE(0x64, o_ifActiveObject);
	OPCODE(0x65, o_stopCurrentScript);
	OPCODE(0x66, o_getClosestObjActor);
	OPCODE(0x67, o5_getActorFacing);
	/* 68 */
	OPCODE(0x68, o5_isScriptRunning);
	OPCODE(0x69, o_setOwnerOf);
	OPCODE(0x6a, o_stopCurrentScript);
	OPCODE(0x6b, o_setActorBitVar);
	/* 6C */
	OPCODE(0x6c, o_stopCurrentScript);
	OPCODE(0x6d, o2_putActorInRoom);
	OPCODE(0x6e, o2_dummy);
	OPCODE(0x6f, o2_ifState08);
	/* 70 */
	OPCODE(0x70, o_lights);
	OPCODE(0x71, o_getBitVar);
	OPCODE(0x72, o_nop);
	OPCODE(0x73, o5_getObjectOwner);
	/* 74 */
	OPCODE(0x74, o5_getDist);
	OPCODE(0x75, o_printEgo_c64);
	OPCODE(0x76, o2_walkActorToObject);
	OPCODE(0x77, o2_clearState04);
	/* 78 */
	OPCODE(0x78, o2_isGreater);
	OPCODE(0x79, o_stopCurrentScript);
	OPCODE(0x7a, o_stopCurrentScript);
	OPCODE(0x7b, o_stopCurrentScript);
	/* 7C */
	OPCODE(0x7c, o5_isSoundRunning);
	OPCODE(0x7d, o_setBitVar);
	OPCODE(0x7e, o2_walkActorTo);
	OPCODE(0x7f, o2_ifNotState02);
	/* 80 */
	OPCODE(0x80, o_stopCurrentScript);
	OPCODE(0x81, o2_putActor);
	OPCODE(0x82, o_stopCurrentScript);
	OPCODE(0x83, o_doSentence);
	/* 84 */
	OPCODE(0x84, o2_isGreaterEqual);
	OPCODE(0x85, o_stopCurrentScript);
	OPCODE(0x86, o_nop);
	OPCODE(0x87, o5_getActorRoom);
	/* 88 */
	OPCODE(0x88, o5_isNotEqual);
	OPCODE(0x89, o_stopCurrentScript);
	OPCODE(0x8a, o_stopCurrentScript);
	OPCODE(0x8b, o_setActorBitVar);
	/* 8C */
	OPCODE(0x8c, o_loadSound);
	OPCODE(0x8d, o_stopCurrentScript);
	OPCODE(0x8e, o_putActorAtObject);
	OPCODE(0x8f, o2_setState02);
	/* 90 */
	OPCODE(0x90, o_pickupObject);
	OPCODE(0x91, o_animateActor);
	OPCODE(0x92, o2_panCameraTo);
	OPCODE(0x93, o_unlockCostume);
	/* 94 */
	OPCODE(0x94, o5_print);
	OPCODE(0x95, o2_actorFromPos);
	OPCODE(0x96, o_stopCurrentScript);
	OPCODE(0x97, o2_setState08);
	/* 98 */
	OPCODE(0x98, o2_restart);
	OPCODE(0x99, o_stopCurrentScript);
	OPCODE(0x9a, o5_move);
	OPCODE(0x9b, o_getActorBitVar);
	/* 9C */
	OPCODE(0x9c, o5_startSound);
	OPCODE(0x9d, o_setBitVar);
	OPCODE(0x9e, o2_walkActorTo);
	OPCODE(0x9f, o2_ifNotState04);
	/* A0 */
	OPCODE(0xa0, o5_stopObjectCode);
	OPCODE(0xa1, o2_putActor);
	OPCODE(0xa2, o4_saveLoadGame);
	OPCODE(0xa3, o_stopCurrentScript);
	/* A4 */
	OPCODE(0xa4, o_unknown2);
	OPCODE(0xa5, o5_loadRoom);
	OPCODE(0xa6, o_stopCurrentScript);
	OPCODE(0xa7, o2_getActorY);
	/* A8 */
	OPCODE(0xa8, o5_notEqualZero);
	OPCODE(0xa9, o_setOwnerOf);
	OPCODE(0xaa, o_stopCurrentScript);
	OPCODE(0xab, o_setActorBitVar);
	/* AC */
	OPCODE(0xac, o_stopCurrentScript);
	OPCODE(0xad, o2_putActorInRoom);
	OPCODE(0xae, o_print_c64);
	OPCODE(0xaf, o2_ifNotState08);
	/* B0 */
	OPCODE(0xb0, o_loadCostume);
	OPCODE(0xb1, o_getBitVar);
	OPCODE(0xb2, o2_setCameraAt);
	OPCODE(0xb3, o_unlockScript);
	/* B4 */
	OPCODE(0xb4, o5_getDist);
	OPCODE(0xb5, o_stopCurrentScript);
	OPCODE(0xb6, o2_walkActorToObject);
	OPCODE(0xb7, o2_setState04);
	/* B8 */
	OPCODE(0xb8, o2_isLessEqual);
	OPCODE(0xb9, o_stopCurrentScript);
	OPCODE(0xba, o2_subtract);
	OPCODE(0xbb, o_stopCurrentScript);
	/* BC */
	OPCODE(0xbc, o5_stopSound);
	OPCODE(0xbd, o_setBitVar);
	OPCODE(0xbe, o2_walkActorTo);
	OPCODE(0xbf, o2_ifNotState02);
	/* C0 */
	OPCODE(0xc0, o_endCutscene);
	OPCODE(0xc1, o2_putActor);
	OPCODE(0xc2, o2_startScript);
	OPCODE(0xc3, o_doSentence);
	/* C4 */
	OPCODE(0xc4, o2_isLess);
	OPCODE(0xc5, o_stopCurrentScript);
	OPCODE(0xc6, o5_decrement);
	OPCODE(0xc7, o2_getActorX);
	/* C8 */
	OPCODE(0xc8, o5_isEqual);
	OPCODE(0xc9, o_stopCurrentScript);
	OPCODE(0xca, o_loadRoom);
	OPCODE(0xcb, o_setActorBitVar);
	/* CC */
	OPCODE(0xcc, o_loadScript);
	OPCODE(0xcd, o_unlockRoom);
	OPCODE(0xce, o_putActorAtObject);
	OPCODE(0xcf, o2_setState02);
	/* D0 */
	OPCODE(0xd0, o_nop);
	OPCODE(0xd1, o_animateActor);
	OPCODE(0xd2, o5_actorFollowCamera);
	OPCODE(0xd3, o_unlockSound);
	/* D4 */
	OPCODE(0xd4, o_setObjectName);
	OPCODE(0xd5, o2_actorFromPos);
	OPCODE(0xd6, o_getActorMoving);
	OPCODE(0xd7, o2_setState08);
	/* D8 */
	OPCODE(0xd8, o_stopCurrentScript);
	OPCODE(0xd9, o_stopCurrentScript);
	OPCODE(0xda, o2_add);
	OPCODE(0xdb, o_getActorBitVar);
	/* DC */
	OPCODE(0xdc, o5_startSound);
	OPCODE(0xdd, o_setBitVar);
	OPCODE(0xde, o2_walkActorTo);
	OPCODE(0xdf, o2_ifNotState04);
	/* E0 */
	OPCODE(0xe0, o_cursorCommand);
	OPCODE(0xe1, o2_putActor);
	OPCODE(0xe2, o2_stopScript);
	OPCODE(0xe3, o_stopCurrentScript);
	/* E4 */
	OPCODE(0xe4, o_ifActiveObject);
	OPCODE(0xe5, o_loadRoomWithEgo);
	OPCODE(0xe6, o_stopCurrentScript);
	OPCODE(0xe7, o5_getActorFacing);
	/* E8 */
	OPCODE(0xe8, o5_isScriptRunning);
	OPCODE(0xe9, o_setOwnerOf);
	OPCODE(0xea, o_stopCurrentScript);
	OPCODE(0xeb, o_setActorBitVar);
	/* EC */
	OPCODE(0xec, o_stopCurrentScript);
	OPCODE(0xed, o2_putActorInRoom);
	OPCODE(0xee, o2_dummy);
	OPCODE(0xef, o2_ifNotState08);
	/* F0 */
	OPCODE(0xf0, o_lights);
	OPCODE(0xf1, o_getBitVar);
	OPCODE(0xf2, o_nop);
	OPCODE(0xf3, o5_getObjectOwner);
	/* F4 */
	OPCODE(0xf4, o5_getDist);
	OPCODE(0xf5, o_stopCurrentScript);
	OPCODE(0xf6, o2_walkActorToObject);
	OPCODE(0xf7, o2_setState04);
	/* F8 */
	OPCODE(0xf8, o2_isGreater);
	OPCODE(0xf9, o_stopCurrentScript);
	OPCODE(0xfa, o_stopCurrentScript);
	OPCODE(0xfb, o_stopCurrentScript);
	/* FC */
	OPCODE(0xfc, o5_isSoundRunning);
	OPCODE(0xfd, o_setBitVar);
	OPCODE(0xfe, o2_walkActorTo);
	OPCODE(0xff, o2_ifState02);
}

#define SENTENCE_SCRIPT 2

int ScummEngine_v0::getVarOrDirectWord(byte mask) {
	return getVarOrDirectByte(mask);
}

uint ScummEngine_v0::fetchScriptWord() {
	return fetchScriptByte();
}

int ScummEngine_v0::getActiveObject() {
	if (_opcode & PARAM_2)
		return _activeObject;

	return fetchScriptByte();
}

void ScummEngine_v0::decodeParseString() {
	byte buffer[512];
	byte *ptr = buffer;
	byte c;
	bool insertSpace = false;

	while ((c = fetchScriptByte())) {

		insertSpace = (c & 0x80) != 0;
		c &= 0x7f;

		if (c == '/') {
			*ptr++ = 13;
		} else {
			*ptr++ = c;
		}

		if (insertSpace)
			*ptr++ = ' ';

	}
	*ptr = 0;

	int textSlot = 0;
	_string[textSlot].xpos = 0;
	_string[textSlot].ypos = 0;
	_string[textSlot].right = _screenWidth - 1;
	_string[textSlot].center = false;
	_string[textSlot].overhead = false;

	if (_actorToPrintStrFor == 0xFF)
		_string[textSlot].color = 14;

	actorTalk(buffer);
}

void ScummEngine_v0::drawSentence() {
	Common::Rect sentenceline;
	const byte *temp;
	int sentencePrep = 0;

	if (!(_userState & 32))
		return;

	if (getResourceAddress(rtVerb, _activeVerb)) {
		strcpy(_sentenceBuf, (char*)getResourceAddress(rtVerb, _activeVerb));
	} else {
		return;
	}

	if (_activeObject > 0) {
		temp = getObjOrActorName(_activeObject);
		if (temp) {
			strcat(_sentenceBuf, " ");
			strcat(_sentenceBuf, (const char*)temp);
		}

		if (_verbs[_activeVerb].prep == 0xFF) {
			byte *ptr = getOBCDFromObject(_activeObject);
			assert(ptr);
			sentencePrep = (*(ptr + 11) >> 5);
		} else {
			sentencePrep = _verbs[_activeVerb].prep;
		}
	}

	if (sentencePrep > 0 && sentencePrep <= 4) {
		// The prepositions, like the fonts, were hard code in the engine. Thus
		// we have to do that, too, and provde localized versions for all the
		// languages MM/Zak are available in.
		const char *prepositions[][5] = {
			{ " ", " in", " with", " on", " to" },   // English
			{ " ", " mit", " mit", " mit", " zu" },  // German
			{ " ", " dans", " avec", " sur", " <" }, // French
			{ " ", " in", " con", " su", " a" },     // Italian
			{ " ", " en", " con", " en", " a" },     // Spanish
			};
		int lang;
		switch (_language) {
		case Common::DE_DEU:
			lang = 1;
			break;
		case Common::FR_FRA:
			lang = 2;
			break;
		case Common::IT_ITA:
			lang = 3;
			break;
		case Common::ES_ESP:
			lang = 4;
			break;
		default:
			lang = 0;	// Default to english
		}

		strcat(_sentenceBuf, prepositions[lang][sentencePrep]);
	}

	if (_activeInventory > 0) {
		temp = getObjOrActorName(_activeInventory);
		if (temp) {
			strcat(_sentenceBuf, " ");
			strcat(_sentenceBuf, (const char*)temp);
		}
	}

	_string[2].charset = 1;
	_string[2].ypos = _virtscr[kVerbVirtScreen].topline;
	_string[2].xpos = 0;
	_string[2].right = _virtscr[kVerbVirtScreen].w - 1;
	_string[2].color = 16;

	byte string[80];
	char *ptr = _sentenceBuf;
	int i = 0, len = 0;

	// Maximum length of printable characters
	int maxChars = 40;
	while (*ptr) {
		if (*ptr != '@')
			len++;
		if (len > maxChars) {
			break;
		}

		string[i++] = *ptr++;

	}
	string[i] = 0;

	sentenceline.top = _virtscr[kVerbVirtScreen].topline;
	sentenceline.bottom = _virtscr[kVerbVirtScreen].topline + 8;
	sentenceline.left = 0;
	sentenceline.right = _virtscr[kVerbVirtScreen].w - 1;
	restoreBackground(sentenceline);

	drawString(2, (byte*)string);
}

void ScummEngine_v0::o_stopCurrentScript() {
	int script;

	script = vm.slot[_currentScript].number;

	if (_currentScript != 0 && vm.slot[_currentScript].number == script)
		stopObjectCode();
	else
		stopScript(script);
}

void ScummEngine_v0::o_loadSound() {
	int resid = fetchScriptByte();
	ensureResourceLoaded(rtSound, resid);
}

void ScummEngine_v0::o_lockSound() {
	int resid = fetchScriptByte();
	_res->lock(rtSound, resid);
}

void ScummEngine_v0::o_unlockSound() {
	int resid = fetchScriptByte();
	_res->unlock(rtSound, resid);
}

void ScummEngine_v0::o_loadCostume() {
	int resid = getVarOrDirectByte(PARAM_1);
	ensureResourceLoaded(rtCostume, resid);
}

void ScummEngine_v0::o_lockCostume() {
	int resid = fetchScriptByte();
	_res->lock(rtCostume, resid);
}

void ScummEngine_v0::o_unlockCostume() {
	int resid = fetchScriptByte();
	_res->unlock(rtCostume, resid);
}

void ScummEngine_v0::o_loadScript() {
	int resid = getVarOrDirectByte(PARAM_1);
	ensureResourceLoaded(rtScript, resid);
}

void ScummEngine_v0::o_lockScript() {
	int resid = fetchScriptByte();
	_res->lock(rtScript, resid);
}

void ScummEngine_v0::o_unlockScript() {
	int resid = fetchScriptByte();
	_res->unlock(rtScript, resid);
}

void ScummEngine_v0::o_loadRoom() {
	int resid = getVarOrDirectByte(PARAM_1);
	ensureResourceLoaded(rtRoom, resid);
}

void ScummEngine_v0::o_loadRoomWithEgo() {
	Actor *a;
	int obj, room, x, y, dir;

	obj = fetchScriptByte();
	room = fetchScriptByte();

	a = derefActor(VAR(VAR_EGO), "o_loadRoomWithEgo");

	a->putActor(0, 0, room);
	_egoPositioned = false;

	startScene(a->_room, a, obj);

	getObjectXYPos(obj, x, y, dir);
	AdjustBoxResult r = a->adjustXYToBeInBox(x, y);
	x = r.x;
	y = r.y;
	a->putActor(x, y, _currentRoom);
	a->setDirection(dir + 180);

	camera._dest.x = camera._cur.x = a->getPos().x;
	setCameraAt(a->getPos().x, a->getPos().y);
	setCameraFollows(a);

	_fullRedraw = true;

	resetSentence();

	if (x >= 0 && y >= 0) {
		a->startWalkActor(x, y, -1);
	}
}

void ScummEngine_v0::o_lockRoom() {
	int resid = fetchScriptByte();
	_res->lock(rtRoom, resid);
}

void ScummEngine_v0::o_unlockRoom() {
	int resid = fetchScriptByte();
	_res->unlock(rtRoom, resid);
}

void ScummEngine_v0::o_cursorCommand() {
	// TODO
	int state = 0;

	_currentMode = fetchScriptByte();
	switch (_currentMode) {
	case 0:
		state = 15;
		break;
	case 1:
		state = 31;
		break;
	case 2:
		break;
	case 3:
		state = 247;
		break;
	}

	setUserState(state);
	debug(0, "o_cursorCommand(%d)", _currentMode);
}

void ScummEngine_v0::o_lights() {
	int a;

	a = getVarOrDirectByte(PARAM_1);
	// Convert older light mode values into
	// equivalent values.of later games
	// 0 Darkness
	// 1 Flashlight
	// 2 Lighted area
	if (a == 2)
		_currentLights = 11;
	else if (a == 1)
		_currentLights = 4;
	else
		_currentLights = 0;

	_fullRedraw = true;
}

void ScummEngine_v0::o_animateActor() {
	int act = getVarOrDirectByte(PARAM_1);
	int anim = getVarOrDirectByte(PARAM_2);
	int unk = fetchScriptByte();
	debug(0,"o_animateActor: unk %d", unk);

	ActorC64 *a = (ActorC64*) derefActor(act, "o_animateActor");

	// 0x6993
	if (anim == 0xFE) {
		a->_speaking = 0x80;	// Enabled, but not switching
		return;
	}
	// 0x69A3
	if (anim == 0xFD) {
		a->_speaking = 0x00;
		return;
	}

	a->animateActor(anim);
}

void ScummEngine_v0::o_getActorMoving() {
	getResultPos();
	int act = getVarOrDirectByte(PARAM_1);
	Actor *a = derefActor(act, "o_getActorMoving");
	if (a->_moving)
		setResult(1);
	else
		setResult(2);
}

void ScummEngine_v0::o_putActorAtObject() {
	int obj, x, y;
	Actor *a;

	a = derefActor(getVarOrDirectByte(PARAM_1), "o_putActorAtObject");

	obj = fetchScriptByte();
	if (whereIsObject(obj) != WIO_NOT_FOUND) {
		getObjectXYPos(obj, x, y);
		AdjustBoxResult r = a->adjustXYToBeInBox(x, y);
		x = r.x;
		y = r.y;
	} else {
		x = 30;
		y = 60;
	}

	a->putActor(x, y);
}

void ScummEngine_v0::o_pickupObject() {
	int obj = fetchScriptByte();
	if (obj == 0) {
		obj = _activeObject;
	}

	if (obj < 1) {
		error("pickupObject received invalid index %d (script %d)", obj, vm.slot[_currentScript].number);
	}

	if (getObjectIndex(obj) == -1)
		return;

	if (whereIsObject(obj) == WIO_INVENTORY)	/* Don't take an */
		return;					/* object twice */

	addObjectToInventory(obj, _roomResource);
	markObjectRectAsDirty(obj);
	putOwner(obj, VAR(VAR_EGO));
	putState(obj, getState(obj) | kObjectState_08 | kObjectStateUntouchable);
	clearDrawObjectQueue();

	runInventoryScript(1);
}

void ScummEngine_v0::o_setObjectName() {
	int obj = fetchScriptByte();
	setObjectName(obj);
}

void ScummEngine_v0::o_nop() {
}

// TODO: Maybe translate actor flags in future.
void ScummEngine_v0::o_setActorBitVar() {
	byte act = getVarOrDirectByte(PARAM_1);
	byte mask = getVarOrDirectByte(PARAM_2);
	byte mod = getVarOrDirectByte(PARAM_3);
	
	// 0x63ED
	if (act >= _numActors)
		return;

	ActorC64 *a = (ActorC64 *)derefActor(act, "o_setActorBitVar");

	if (mod)
		a->_miscflags |= mask;
	else
		a->_miscflags &= ~mask;

	debug(0, "o_setActorBitVar(%d, %d, %d)", act, mask, mod);
}

void ScummEngine_v0::o_getActorBitVar() {
	getResultPos();
	byte act = getVarOrDirectByte(PARAM_1);
	byte mask = getVarOrDirectByte(PARAM_2);

	ActorC64 *a = (ActorC64 *)derefActor(act, "o_getActorBitVar");
	setResult((a->_miscflags & mask) ? 1 : 0);

	debug(0, "o_getActorBitVar(%d, %d, %d)", act, mask, (a->_miscflags & mask));
}

void ScummEngine_v0::o_setBitVar() {
	byte flag = getVarOrDirectByte(PARAM_1);
	byte mask = getVarOrDirectByte(PARAM_2);
	byte mod = getVarOrDirectByte(PARAM_3);

	if (mod)
		_bitVars[flag] |= (1 << mask);
	else
		_bitVars[flag] &= ~(1 << mask);

	debug(0, "o_setBitVar (%d, %d %d)", flag, mask, mod);
}

void ScummEngine_v0::o_getBitVar() {
	getResultPos();
	byte flag = getVarOrDirectByte(PARAM_1);
	byte mask = getVarOrDirectByte(PARAM_2);

	setResult((_bitVars[flag] & (1 << mask)) ? 1 : 0);

	debug(0, "o_getBitVar (%d, %d %d)", flag, mask, _bitVars[flag] & (1 << mask));
}

void ScummEngine_v0::o_print_c64() {
	_actorToPrintStrFor = fetchScriptByte();
	decodeParseString();
}

void ScummEngine_v0::o_printEgo_c64() {
	_actorToPrintStrFor = (byte)VAR(VAR_EGO);
	decodeParseString();
}

void ScummEngine_v0::o_doSentence() {
	byte entry = fetchScriptByte();
	byte obj = fetchScriptByte();
	fetchScriptByte();

	runObjectScript(obj, entry, false, false, NULL);
}

void ScummEngine_v0::o_unknown2() {
	byte var1 = fetchScriptByte();
	error("STUB: o_unknown2(%d)", var1);
}

void ScummEngine_v0::o_ifActiveObject() {
	byte obj = fetchScriptByte();

	jumpRelative(obj == _activeInventory);
}

void ScummEngine_v0::o_getClosestObjActor() {
	int obj;
	int act;
	int dist;

	// This code can't detect any actors farther away than 255 units
	// (pixels in newer games, characters in older ones.) But this is
	// perfectly OK, as it is exactly how the original behaved.

	int closest_obj = 0xFF, closest_dist = 0xFF;

	getResultPos();

	act = getVarOrDirectByte(PARAM_1);
	obj = (_opcode & PARAM_2) ? 25 : 7;

	do {
		dist = getObjActToObjActDist(act, obj);
		if (dist < closest_dist) {
			closest_dist = dist;
			closest_obj = obj;
		}
	} while (--obj);

	setResult(closest_obj);
}

void ScummEngine_v0::o_cutscene() {
	vm.cutSceneData[0] = _userState | (_userPut ? 16 : 0);
	vm.cutSceneData[2] = _currentRoom;
	vm.cutSceneData[3] = camera._mode;

	// Hide inventory, freeze scripts, hide cursor
	setUserState(15);

	_sentenceNum = 0;
	stopScript(SENTENCE_SCRIPT);
	resetSentence();

	vm.cutScenePtr[0] = 0;
}

void ScummEngine_v0::o_endCutscene() {
	vm.cutSceneStackPointer = 0;

	VAR(VAR_OVERRIDE) = 0;
	vm.cutSceneScript[0] = 0;
	vm.cutScenePtr[0] = 0;

	// Reset user state to values before cutscene
	setUserState(vm.cutSceneData[0] | 7);

	camera._mode = (byte) vm.cutSceneData[3];
	if (camera._mode == kFollowActorCameraMode) {
		actorFollowCamera(VAR(VAR_EGO));
	} else if (vm.cutSceneData[2] != _currentRoom) {
		startScene(vm.cutSceneData[2], 0, 0);
	}
}

void ScummEngine_v0::o_beginOverride() {
	const int idx = vm.cutSceneStackPointer;
	assert(0 <= idx && idx < 5);

	vm.cutScenePtr[idx] = _scriptPointer - _scriptOrgPointer;
	vm.cutSceneScript[idx] = _currentScript;

	// Skip the jump instruction following the override instruction
	// (the jump is responsible for "skipping" cutscenes, and the reason
	// why we record the current script position in vm.cutScenePtr).
	fetchScriptByte();
	ScummEngine::fetchScriptWord();

	// This is based on disassembly
	VAR(VAR_OVERRIDE) = 0;
}

void ScummEngine_v0::o_setOwnerOf() {
	int obj, owner;

	obj = getVarOrDirectWord(PARAM_1);
	owner = getVarOrDirectByte(PARAM_2);

	if (obj == 0)
		obj = _activeInventory;

	setOwnerOf(obj, owner);
}

void ScummEngine_v0::resetSentence() {
	_activeInventory = 0;
	_activeObject = 0;
	_activeVerb = 13;
}

} // End of namespace Scumm
