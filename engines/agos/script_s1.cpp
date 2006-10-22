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

#include "agos/agos.h"

namespace AGOS {

void AGOSEngine::setupSimon1Opcodes(OpcodeProc *op) {
	setupCommonOpcodes(op);

	op[65] = &AGOSEngine::o_addTextBox;
	op[66] = &AGOSEngine::o_setShortText;
	op[67] = &AGOSEngine::o_setLongText;
	op[70] = &AGOSEngine::o1_printLongText;
	op[83] = &AGOSEngine::o1_rescan;
	op[98] = &AGOSEngine::o1_animate;
	op[99] = &AGOSEngine::o1_stopAnimate;
	op[127] = &AGOSEngine::o1_playTune;
	op[161] = &AGOSEngine::o_screenTextBox;
	op[162] = &AGOSEngine::o_screenTextMsg;
	op[163] = &AGOSEngine::o_playEffect;
	op[164] = &AGOSEngine::o_getDollar2;
	op[165] = &AGOSEngine::o_isAdjNoun;
	op[166] = &AGOSEngine::o_b2Set;
	op[167] = &AGOSEngine::o_b2Clear;
	op[168] = &AGOSEngine::o_b2Zero;
	op[169] = &AGOSEngine::o_b2NotZero;
	op[175] = &AGOSEngine::o_lockZones;
	op[176] = &AGOSEngine::o_unlockZones;
	op[177] = &AGOSEngine::o1_screenTextPObj;
	op[178] = &AGOSEngine::o_getPathPosn;
	op[179] = &AGOSEngine::o_scnTxtLongText;
	op[180] = &AGOSEngine::o_mouseOn;
	op[181] = &AGOSEngine::o1_mouseOff;
	op[182] = &AGOSEngine::o1_loadBeard;
	op[183] = &AGOSEngine::o1_unloadBeard;
	op[184] = &AGOSEngine::o_unloadZone;
	op[185] = &AGOSEngine::o1_loadStrings;
	op[186] = &AGOSEngine::o_unfreezeZones;
	op[187] = &AGOSEngine::o1_specialFade;
}

// -----------------------------------------------------------------------
// Simon 1 Opcodes
// -----------------------------------------------------------------------

void AGOSEngine::o1_printLongText() {
	// 70: show string from array
	const char *str = (const char *)getStringPtrByID(_longText[getVarOrByte()]);
	showMessageFormat("%s\n", str);
}

void AGOSEngine::o1_rescan() {
	// 83: restart subroutine
	setScriptReturn(-10);
}

void AGOSEngine::o1_animate() {
	// 98: start vga
	uint vgaSpriteId = getVarOrWord();
	uint windowNum = getVarOrByte();
	uint x = getVarOrWord();
	uint y = getVarOrWord();
	uint palette = getVarOrWord();

	if (getGameType() == GType_SIMON1 && (getFeatures() & GF_TALKIE) && vgaSpriteId >= 400) {
		_lastVgaWaitFor = 0;
	}

	_lockWord |= 0x40;
	animate(windowNum, vgaSpriteId / 100, vgaSpriteId, x, y, palette);
	_lockWord &= ~0x40;
}

void AGOSEngine::o1_stopAnimate() {
	// 99: kill sprite
	stopAnimateSimon1(getVarOrWord());
}

void AGOSEngine::o1_playTune() {
	// 127: deals with music
	int music = getVarOrWord();
	int track = getVarOrWord();

	// Jamieson630:
	// This appears to be a "load or play music" command.
	// The music resource is specified, and optionally
	// a track as well. Normally we see two calls being
	// made, one to load the resource and another to
	// actually start a track (so the resource is
	// effectively preloaded so there's no latency when
	// starting playback).

	if (music != _lastMusicPlayed) {
		_lastMusicPlayed = music;
		loadMusic(music);
		midi.startTrack(track);
	}
}

void AGOSEngine::o1_screenTextPObj() {
	// 177: inventory descriptions
	uint vgaSpriteId = getVarOrByte();
	uint color = getVarOrByte();

	SubObject *subObject = (SubObject *)findChildOfType(getNextItemPtr(), 2);
	if (getFeatures() & GF_TALKIE) {
		if (subObject != NULL && subObject->objectFlags & kOFVoice) {
			uint offs = getOffsetOfChild2Param(subObject, kOFVoice);
			playSpeech(subObject->objectFlagValue[offs], vgaSpriteId);
		} else if (subObject != NULL && subObject->objectFlags & kOFNumber) {
			uint offs = getOffsetOfChild2Param(subObject, kOFNumber);
			playSpeech(subObject->objectFlagValue[offs] + 3550, vgaSpriteId);
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

void AGOSEngine::o1_mouseOff() {
	// 181: force mouseOff
	scriptMouseOff();
}

void AGOSEngine::o1_loadBeard() {
	// 182: load beard
	if (_beardLoaded == false) {
		_beardLoaded = true;
		_lockWord |= 0x8000;
		loadVGABeardFile(328);
		_lockWord &= ~0x8000;
	}
}

void AGOSEngine::o1_unloadBeard() {
	// 183: unload beard
	if (_beardLoaded == true) {
		_beardLoaded = false;
		_lockWord |= 0x8000;
		loadVGABeardFile(23);
		_lockWord &= ~0x8000;
	}
}

void AGOSEngine::o1_loadStrings() {
	// 185: load sound files
	_soundFileId = getVarOrWord();
	if (getPlatform() == Common::kPlatformAmiga && getFeatures() & GF_TALKIE) {
		char buf[10];
		sprintf(buf, "%d%s", _soundFileId, "Effects");
		_sound->readSfxFile(buf);
		sprintf(buf, "%d%s", _soundFileId, "simon");
		_sound->readVoiceFile(buf);
	}
}

void AGOSEngine::o1_specialFade() {
	// 187: fade to black
	uint i;

	memcpy(_videoBuf1, _currentPalette, 4 * 256);

	for (i = 32; i != 0; --i) {
		paletteFadeOut(_videoBuf1, 32, 8);
		paletteFadeOut(_videoBuf1 + 4 * 48, 144, 8);
		paletteFadeOut(_videoBuf1 + 4 * 208, 48, 8);
		_system->setPalette(_videoBuf1, 0, 256);
		delay(5);
	}

	memcpy(_currentPalette, _videoBuf1, 1024);
	memcpy(_displayPalette, _videoBuf1, 1024);
}

} // End of namespace AGOS
