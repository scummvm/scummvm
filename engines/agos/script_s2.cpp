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

#include "agos/agos.h"

namespace AGOS {

void AGOSEngine::setupSimon2Opcodes(OpcodeProc *op) {
	setupCommonOpcodes(op);

	op[65] = &AGOSEngine::oww_addTextBox;
	op[66] = &AGOSEngine::oww_setShortText;
	op[67] = &AGOSEngine::oww_setLongText;
	op[70] = &AGOSEngine::os2_printLongText;
	op[83] = &AGOSEngine::os2_rescan;
	op[88] = &AGOSEngine::o_haltAnimation;
	op[89] = &AGOSEngine::o_restartAnimation;
	op[98] = &AGOSEngine::os2_animate;
	op[99] = &AGOSEngine::os2_stopAnimate;
	op[127] = &AGOSEngine::os2_playTune;
	op[161] = &AGOSEngine::os1_screenTextBox;
	op[162] = &AGOSEngine::os1_screenTextMsg;
	op[163] = &AGOSEngine::os1_playEffect;
	op[164] = &AGOSEngine::oe2_getDollar2;
	op[165] = &AGOSEngine::oe2_isAdjNoun;
	op[166] = &AGOSEngine::oe2_b2Set;
	op[167] = &AGOSEngine::oe2_b2Clear;
	op[168] = &AGOSEngine::oe2_b2Zero;
	op[169] = &AGOSEngine::oe2_b2NotZero;
	op[175] = &AGOSEngine::oww_lockZones;
	op[176] = &AGOSEngine::oww_unlockZones;
	op[177] = &AGOSEngine::os2_screenTextPObj;
	op[178] = &AGOSEngine::os1_getPathPosn;
	op[179] = &AGOSEngine::os1_scnTxtLongText;
	op[180] = &AGOSEngine::os1_mouseOn;
	op[181] = &AGOSEngine::os2_mouseOff;
	op[184] = &AGOSEngine::os1_unloadZone;
	op[186] = &AGOSEngine::os1_unfreezeZones;
	op[188] = &AGOSEngine::os2_isShortText;
	op[189] = &AGOSEngine::os2_clearMarks;
	op[190] = &AGOSEngine::os2_waitMark;
}

// -----------------------------------------------------------------------
// Simon 2 Opcodes
// -----------------------------------------------------------------------

void AGOSEngine::os2_printLongText() {
	// 70: show string from array
	const char *str = (const char *)getStringPtrByID(_longText[getVarOrByte()]);
	writeVariable(51, strlen(str) / 53 * 8 + 8);
	showMessageFormat("%s\n", str);
}

void AGOSEngine::os2_rescan() {
	// 83: restart subroutine
	if (_exitCutscene) {
		if (getBitFlag(9)) {
			endCutscene();
		}
	} else {
		processSpecialKeys();
	}

	setScriptReturn(-10);
}

void AGOSEngine::os2_animate() {
	// 98: start vga
	uint zoneNum = getVarOrWord();
	uint vgaSpriteId = getVarOrWord();
	uint windowNum = getVarOrByte();
	uint x = getVarOrWord();
	uint y = getVarOrWord();
	uint palette = getVarOrWord();

	_lockWord |= 0x40;
	animate(windowNum, zoneNum, vgaSpriteId, x, y, palette);
	_lockWord &= ~0x40;
}

void AGOSEngine::os2_stopAnimate() {
	// 99: kill sprite
	uint a = getVarOrWord();
	uint b = getVarOrWord();
	stopAnimateSimon2(a, b);
}

void AGOSEngine::os2_playTune() {
	// 127: deals with music
	int music = getVarOrWord();
	int track = getVarOrWord();
	int loop = getVarOrByte();

	// Jamieson630:
	// This appears to be a "load or play music" command.
	// The music resource is specified, and optionally
	// a track as well. Normally we see two calls being
	// made, one to load the resource and another to
	// actually start a track (so the resource is
	// effectively preloaded so there's no latency when
	// starting playback).

	midi.setLoop(loop != 0);
	if (_lastMusicPlayed != music)
		_nextMusicToPlay = music;
	else
		midi.startTrack(track);
}

void AGOSEngine::os2_screenTextPObj() {
	// 177: inventory descriptions
	uint vgaSpriteId = getVarOrByte();
	uint color = getVarOrByte();

	SubObject *subObject = (SubObject *)findChildOfType(getNextItemPtr(), 2);
	if (getFeatures() & GF_TALKIE) {
		if (subObject != NULL && subObject->objectFlags & kOFVoice) {
			uint speechId = subObject->objectFlagValue[getOffsetOfChild2Param(subObject, kOFVoice)];

			if (subObject->objectFlags & kOFNumber) {
				uint speechIdOffs = subObject->objectFlagValue[getOffsetOfChild2Param(subObject, kOFNumber)];

				if (speechId == 116)
					speechId = speechIdOffs + 115;
				if (speechId == 92)
					speechId = speechIdOffs + 98;
				if (speechId == 99)
					speechId = 9;
				if (speechId == 97) {
					switch (speechIdOffs) {
					case 12:
						speechId = 109;
						break;
					case 14:
						speechId = 108;
						break;
					case 18:
						speechId = 107;
						break;
					case 20:
						speechId = 106;
						break;
					case 22:
						speechId = 105;
						break;
					case 28:
						speechId = 104;
						break;
					case 90:
						speechId = 103;
						break;
					case 92:
						speechId = 102;
						break;
					case 100:
						speechId = 51;
						break;
					default:
						error("os2_screenTextPObj: invalid case %d", speechIdOffs);
					}
				}
			}

			if (_speech)
				playSpeech(speechId, vgaSpriteId);
		}

	}

	if (subObject != NULL && subObject->objectFlags & kOFText && _subtitles) {
		const char *stringPtr = (const char *)getStringPtrByID(subObject->objectFlagValue[0]);
		TextLocation *tl = getTextLocation(vgaSpriteId);
		char buf[256];
		int j, k;

		if (subObject->objectFlags & kOFNumber) {
			if (_language == Common::HB_ISR) {
				j = subObject->objectFlagValue[getOffsetOfChild2Param(subObject, kOFNumber)];
				k = (j % 10) * 10;
				k += j / 10;
				if (!(j % 10))
					sprintf(buf,"0%d%s", k, stringPtr);
				else
					sprintf(buf,"%d%s", k, stringPtr);
			} else {
				sprintf(buf,"%d%s", subObject->objectFlagValue[getOffsetOfChild2Param(subObject, kOFNumber)], stringPtr);
			}
			stringPtr = buf;
		}
		if (stringPtr != NULL)
			printScreenText(vgaSpriteId, color, stringPtr, tl->x, tl->y, tl->width);
	}
}

void AGOSEngine::os2_mouseOff() {
	// 181: force mouseOff
	scriptMouseOff();
	changeWindow(1);
	showMessageFormat("\xC");
}

void AGOSEngine::os2_isShortText() {
	// 188: string2 is
	uint i = getVarOrByte();
	uint str = getNextStringID();
	setScriptCondition(str < _numTextBoxes && _shortText[i] == str);
}

void AGOSEngine::os2_clearMarks() {
	// 189: clear_op189_flag
	_marks = 0;
}

void AGOSEngine::os2_waitMark() {
	// 190
	uint i = getVarOrByte();
	if (!(_marks & (1 << i)))
		waitForMark(i);
}

void AGOSEngine::waitForMark(uint i) {
	_exitCutscene = false;
	while (!(_marks & (1 << i))) {
		if (_exitCutscene) {
			if (getGameType() == GType_PP) {
				if (_picture8600)
					break;
			} else {
				if (getBitFlag(9)) {
					endCutscene();
					break;
				}
			}
		} else {
			processSpecialKeys();
		}

		delay(10);
	}
}

} // End of namespace AGOS
