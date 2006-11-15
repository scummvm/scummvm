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

	op[65] = &AGOSEngine::oww_addTextBox;
	op[66] = &AGOSEngine::oww_setShortText;
	op[67] = &AGOSEngine::oww_setLongText;
	op[70] = &AGOSEngine::oww_printLongText;
	op[83] = &AGOSEngine::oe1_rescan;
	op[88] = &AGOSEngine::o_haltAnimation;
	op[89] = &AGOSEngine::o_restartAnimation;
	op[98] = &AGOSEngine::os1_animate;
	op[99] = &AGOSEngine::oe1_stopAnimate;
	op[127] = &AGOSEngine::os1_playTune;
	op[135] = &AGOSEngine::os1_pauseGame;
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
	op[177] = &AGOSEngine::os1_screenTextPObj;
	op[178] = &AGOSEngine::os1_getPathPosn;
	op[179] = &AGOSEngine::os1_scnTxtLongText;
	op[180] = &AGOSEngine::os1_mouseOn;
	op[181] = &AGOSEngine::os1_mouseOff;
	op[182] = &AGOSEngine::os1_loadBeard;
	op[183] = &AGOSEngine::os1_unloadBeard;
	op[184] = &AGOSEngine::os1_unloadZone;
	op[185] = &AGOSEngine::os1_loadStrings;
	op[186] = &AGOSEngine::os1_unfreezeZones;
	op[187] = &AGOSEngine::os1_specialFade;
}

// -----------------------------------------------------------------------
// Simon 1 Opcodes
// -----------------------------------------------------------------------

void AGOSEngine::os1_animate() {
	// 98: animate
	uint vgaSpriteId = getVarOrWord();
	uint windowNum = getVarOrByte();
	uint x = getVarOrWord();
	uint y = getVarOrWord();
	uint palette = getVarOrWord();

	if (getFeatures() & GF_TALKIE && vgaSpriteId >= 400) {
		_lastVgaWaitFor = 0;
	}

	_lockWord |= 0x40;
	animate(windowNum, vgaSpriteId / 100, vgaSpriteId, x, y, palette);
	_lockWord &= ~0x40;
}

void AGOSEngine::os1_playTune() {
	// 127:  play tune
	int music = getVarOrWord();
	int track = getVarOrWord();

	if (music != _lastMusicPlayed) {
		_lastMusicPlayed = music;
		loadMusic(music);
		midi.startTrack(track);
	}
}

void AGOSEngine::os1_pauseGame() {
	// 135: pause game
	_system->setFeatureState(OSystem::kFeatureVirtualKeyboard, true);

	// If all else fails, use English as fallback.
	byte keyYes = 'y';
	byte keyNo = 'n';

	switch (_language) {
	case Common::RU_RUS:
		break;
	case Common::PL_POL:
		keyYes = 't';
		break;
	case Common::HB_ISR:
		keyYes = 'f';
		break;
	case Common::ES_ESP:
		keyYes = 's';
		break;
	case Common::IT_ITA:
		keyYes = 's';
		break;
	case Common::FR_FRA:
		keyYes = 'o';
		break;
	case Common::DE_DEU:
		keyYes = 'j';
		break;
	default:
		break;
	}

	for (;;) {
		delay(1);
#ifdef _WIN32_WCE
		if (isSmartphone()) {
			if (_keyPressed) {
				if (_keyPressed == 13)
					shutdown();
				else
					break;
			}
		}
#endif
		if (_keyPressed == keyYes || _keyPressed == (keyYes - 32))
			shutdown();
		else if (_keyPressed == keyNo || _keyPressed == (keyNo - 32))
			break;
	}

	_system->setFeatureState(OSystem::kFeatureVirtualKeyboard, false);
}

void AGOSEngine::os1_screenTextBox() {
	// 161: setup text
	TextLocation *tl = getTextLocation(getVarOrByte());

	tl->x = getVarOrWord();
	tl->y = getVarOrByte();
	tl->width = getVarOrWord();
}
	
void AGOSEngine::os1_screenTextMsg() {
	// 162: print string
	uint vgaSpriteId = getVarOrByte();
	uint color = getVarOrByte();
	uint stringId = getNextStringID();
	const byte *string_ptr = NULL;
	uint speechId = 0;
	TextLocation *tl;

	if (stringId != 0xFFFF)
		string_ptr = getStringPtrByID(stringId);

	if (getFeatures() & GF_TALKIE) {
		if (getGameType() == GType_FF || getGameType() == GType_PP)
			speechId = (uint16)getVarOrWord();
		else
			speechId = (uint16)getNextWord();
	}

	if (getGameType() == GType_FF || getGameType() == GType_PP)
		vgaSpriteId = 1;

	tl = getTextLocation(vgaSpriteId);
	if (_speech && speechId != 0)
		playSpeech(speechId, vgaSpriteId);
	if (((getGameType() == GType_SIMON2 && (getFeatures() & GF_TALKIE)) || getGameType() == GType_FF) &&
		speechId == 0) {
		stopAnimateSimon2(2, vgaSpriteId + 2);
	}

	if (string_ptr != NULL && (speechId == 0 || _subtitles))
		printScreenText(vgaSpriteId, color, (const char *)string_ptr, tl->x, tl->y, tl->width);

}

void AGOSEngine::os1_playEffect() {
	// 163: play sound
	uint soundId = getVarOrWord();

	if (getGameId() == GID_SIMON1DOS)
		playSting(soundId);
	else
		_sound->playEffects(soundId);
}

void AGOSEngine::os1_screenTextPObj() {
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

void AGOSEngine::os1_getPathPosn() {
	// 178: path find
	uint x = getVarOrWord();
	uint y = getVarOrWord();
	uint var_1 = getVarOrByte();
	uint var_2 = getVarOrByte();

	const uint16 *p;
	uint i, j;
	uint prev_i;
	uint x_diff, y_diff;
	uint best_i = 0, best_j = 0, best_dist = 0xFFFFFFFF;
	uint maxPath = (getGameType() == GType_FF || getGameType() == GType_PP) ? 100 : 20;

	if (getGameType() == GType_FF || getGameType() == GType_PP) {
		x += _scrollX;
		y += _scrollY;
	} else if (getGameType() == GType_SIMON2) {
		x += _scrollX * 8;
	}

	int end = (getGameType() == GType_FF) ? 9999 : 999;
	prev_i = maxPath + 1 - readVariable(12);
	for (i = maxPath; i != 0; --i) {
		p = (const uint16 *)_pathFindArray[maxPath - i];
		if (!p)
			continue;
		for (j = 0; readUint16Wrapper(&p[0]) != end; j++, p += 2) {
			x_diff = ABS((int16)(readUint16Wrapper(&p[0]) - x));
			y_diff = ABS((int16)(readUint16Wrapper(&p[1]) - 12 - y));

			if (x_diff < y_diff) {
				x_diff /= 4;
				y_diff *= 4;
			}
			x_diff += y_diff /= 4;

			if (x_diff < best_dist || x_diff == best_dist && prev_i == i) {
				best_dist = x_diff;
				best_i = maxPath + 1 - i;
				best_j = j;
			}
		}
	}

	writeVariable(var_1, best_i);
	writeVariable(var_2, best_j);
}

void AGOSEngine::os1_scnTxtLongText() {
	// 179: conversation responses and room descriptions
	uint vgaSpriteId = getVarOrByte();
	uint color = getVarOrByte();
	uint stringId = getVarOrByte();
	uint speechId = 0;
	TextLocation *tl;

	const char *string_ptr = (const char *)getStringPtrByID(_longText[stringId]);
	if (getFeatures() & GF_TALKIE)
		speechId = _longSound[stringId];

	if (getGameType() == GType_FF || getGameType() == GType_PP)
		vgaSpriteId = 1;
	tl = getTextLocation(vgaSpriteId);

	if (_speech && speechId != 0)
		playSpeech(speechId, vgaSpriteId);
	if (string_ptr != NULL && _subtitles)
		printScreenText(vgaSpriteId, color, string_ptr, tl->x, tl->y, tl->width);
}

void AGOSEngine::os1_mouseOn() {
	// 180: force mouseOn
	_mouseHideCount = 0;
}

void AGOSEngine::os1_mouseOff() {
	// 181: force mouseOff
	scriptMouseOff();
}

void AGOSEngine::os1_loadBeard() {
	// 182: load beard
	if (_beardLoaded == false) {
		_beardLoaded = true;
		_lockWord |= 0x8000;
		loadVGABeardFile(328);
		_lockWord &= ~0x8000;
	}
}

void AGOSEngine::os1_unloadBeard() {
	// 183: unload beard
	if (_beardLoaded == true) {
		_beardLoaded = false;
		_lockWord |= 0x8000;
		loadVGABeardFile(23);
		_lockWord &= ~0x8000;
	}
}

void AGOSEngine::os1_unloadZone() {
	// 184: unload zone
	uint a = getVarOrWord();
	VgaPointersEntry *vpe = &_vgaBufferPointers[a];

	vpe->sfxFile = NULL;
	vpe->vgaFile1 = NULL;
	vpe->vgaFile2 = NULL;
}

void AGOSEngine::os1_loadStrings() {
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

void AGOSEngine::os1_unfreezeZones() {
	// 186: freeze zone
	unfreezeBottom();
}

void AGOSEngine::os1_specialFade() {
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

void AGOSEngine::scriptMouseOff() {
	_lockWord |= 0x8000;
	vc34_setMouseOff();
	_lockWord &= ~0x8000;
}

} // End of namespace AGOS
