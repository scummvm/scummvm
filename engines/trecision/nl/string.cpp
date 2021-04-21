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

#include "common/scummsys.h"
#include "trecision/nl/3d/3dinc.h"
#include "trecision/nl/struct.h"
#include "trecision/nl/define.h"
#include "trecision/nl/message.h"
#include "trecision/nl/proto.h"
#include "trecision/nl/extern.h"
#include "trecision/trecision.h"

#include "common/config-manager.h"
#include "trecision/sound.h"
#include "trecision/video.h"

namespace Trecision {

// Variabili di servizio comuni a piu' funzioni di string.c
uint16 CurS;

uint16 SpeakSomeOneAnimation;
uint16 SpeakSomeOnePerson;

const char  *SuperString;
uint16 SuperStringLen;
char  SubString[MAXSUBSTRING][MAXLENSUBSTRING];
uint16 SubStringStart;
bool  substringagain;
uint16 CurSubString;
uint16 SubStringUsed;

char sn[13];
uint32 TalkTime;
const char *dunno = "?";

void PositionString(uint16 x, uint16 y, const char *string, uint16 *posx, uint16 *posy, bool characterFl) {
	uint16 lenText = g_vm->TextLength(string, 0);
	if (lenText > 960)
		lenText = (lenText * 2 / 5);
	else if (lenText > 320)
		lenText = (lenText * 3 / 5);

	if (x > (lenText >> 1))
		x -= (lenText >> 1);
	else
		x = 0;

	*posx = CLIP<uint16>(x, 5, MAXX - lenText - 5);

	*posy = characterFl ? 0 : VIDEOTOP;
	*posy += y - 1; //15
	if (*posy <= VIDEOTOP)
		*posy = VIDEOTOP + 1;
}

void ShowObjName(uint16 obj, bool showhide) {
	uint16 posx;
	uint16 posy;
	Common::String locsent;

	if (g_vm->_flagSomeoneSpeaks)
		return;

	if (g_vm->_lastInv) {
		g_vm->clearText();
		g_vm->_lastInv = 0;
	}

	if (g_vm->_flagUseWithStarted) {
		if (!showhide) {
			g_vm->clearText();
			g_vm->_lastObj = obj;
			return;
		}

		if ((g_vm->_obj[g_vm->_curObj]._flag & (kObjFlagRoomOut | kObjFlagRoomIn)) && !(g_vm->_obj[g_vm->_curObj]._flag & kObjFlagExamine))
			return;

		locsent += g_vm->_sysText[kMessageUse];
		if (g_vm->_useWithInv[USED])
			locsent += g_vm->_objName[g_vm->_inventoryObj[g_vm->_useWith[USED]]._name];
		else if (g_vm->_obj[g_vm->_useWith[USED]]._mode & OBJMODE_HIDDEN)
			locsent += dunno;
		else
			locsent += g_vm->_objName[g_vm->_obj[g_vm->_useWith[USED]]._name];

		locsent += g_vm->_sysText[kMessageWith];
		if (obj && (g_vm->_useWithInv[USED] || (obj != g_vm->_useWith[USED]))) {
			if (g_vm->_obj[obj]._mode & OBJMODE_HIDDEN)
				locsent += dunno;
			else
				locsent += g_vm->_objName[g_vm->_obj[obj]._name];
		}

		g_vm->_lastObj = (obj | 0x8000);
		uint16 lenText = g_vm->TextLength(locsent.c_str(), 0);

		posx = CLIP(320 - (lenText / 2), 2, MAXX - 2 - lenText);
		posy = MAXY - CARHEI;

		if (g_vm->_lastObj)
			g_vm->clearText();
		g_vm->addText(posx, posy, locsent.c_str(), COLOR_INVENTORY, MASKCOL);
	} else {
		if (!obj || !showhide) {
			g_vm->clearText();
			g_vm->_lastObj = obj;
			return;
		}

		if (obj == g_vm->_lastObj)
			return;
		if (!(g_vm->_obj[obj]._flag & kObjFlagExamine)) {
			if ((g_vm->_obj[obj]._flag & kObjFlagDone) || (g_vm->_room[g_vm->_obj[obj]._goRoom]._flag & kObjFlagDone)) {
				locsent = g_vm->_sysText[kMessageGoto];
				if (g_vm->_obj[obj]._mode & OBJMODE_HIDDEN)
					locsent += dunno;
				else
					locsent += g_vm->_objName[g_vm->_obj[obj]._name];
			} else
				locsent = g_vm->_sysText[kMessageGoto2];
		} else if (g_vm->_obj[obj]._mode & OBJMODE_HIDDEN)
			locsent = dunno;
		else
			locsent = g_vm->_objName[g_vm->_obj[obj]._name];

		posx = (g_vm->_obj[obj]._lim.left + g_vm->_obj[obj]._lim.right) / 2;
		posy = (obj == oWHEELS2C) ? 187 : g_vm->_obj[obj]._lim.top;

		PositionString(posx, posy, locsent.c_str(), &posx, &posy, false);
		if (g_vm->_lastObj)
			g_vm->clearText();
		g_vm->_lastObj = obj;
		g_vm->addText(posx, posy, locsent.c_str(), COLOR_OBJECT, MASKCOL);
	}
}

void FormattingSuperString() {
	SubStringUsed  = 0;
	substringagain = true;
	while (substringagain) {
		FormattingOneString();
		SubStringUsed++;
	}
}

void FormattingOneString() {
	uint16 i;
	memset(SubString[SubStringUsed], '\0', MAXLENSUBSTRING);

	const uint16 available = (SuperStringLen - SubStringStart);
	for (i = 0; i < available; i++) {
		switch (SuperString[i + SubStringStart]) {
		case '\0':
			substringagain = false;
			return;

		case '@':
			substringagain = true;
			SubStringStart += (i + 1);
			return;

		default:
			SubString[SubStringUsed][i] = SuperString[i + SubStringStart];
			break;
		}
	}

	SubString[SubStringUsed][i] = '\0';
	substringagain = false;
}

void CharacterSay(uint16 i) {
	CurS = i;

	//	TODO: Use a define...  ************************
	// CHECKME: 'i' can't be equal to 99999 because it's a uint16... 
	g_vm->_flagSkipEnable = (i != 99999);

	//	if he took some action
	if (g_vm->_sentence[i][0] == '*' && !g_vm->_animMgr->_playingAnims[kSmackerAction])
		StartCharacterAction(hBOH, 0, 0, 0);
	else
		CharacterTalk(g_vm->_sentence[i]);
}

void CharacterTalk(const char *s) {
	g_vm->_flagSomeoneSpeaks = true;
	g_vm->_flagCharacterSpeak = true;
	g_vm->_flagSkipTalk = false;

	SuperString = s;
	SuperStringLen = strlen(SuperString);
	SubStringStart = 0;
	CurSubString = 0;
	FormattingSuperString();

	CharacterContinueTalk();

	g_vm->_characterQueue.initQueue();
	actorStop();
}

void CharacterTalkInAction(uint16 ss) {
	const char *s = g_vm->_sentence[ss];

	if (s[0] == '*')
		return;
	CurS = ss;

	g_vm->_flagSomeoneSpeaks = true;
	g_vm->_flagCharacterSpeak = true;
	g_vm->_flagSkipTalk = false;
	g_vm->_flagSkipEnable = true;

	SuperString = s;
	SuperStringLen = strlen(SuperString);
	SubStringStart = 0;
	CurSubString = 0;
	FormattingSuperString();

	CharacterContinueTalk();
}

void CharacterContinueTalk() {
	g_vm->_flagSkipTalk = false;
	_characterSpeakTime = TheTime;

	substringagain = (CurSubString < (SubStringUsed - 1));

	uint16 posx, posy;
	if (g_vm->_flagCharacterExists)
		PositionString(g_vm->_actor->_lim[0], g_vm->_actor->_lim[2], SubString[CurSubString], &posx, &posy, true);
	else
		PositionString(MAXX / 2, 30, SubString[CurSubString], &posx, &posy, false);

	g_vm->clearText();
	if (ConfMan.getBool("subtitles"))
		g_vm->addText(posx, posy, SubString[CurSubString], COLOR_OBJECT, MASKCOL);

	if (!g_vm->_flagDialogActive) {
		if (CurSubString)
			sprintf(sn, "s%04d%c.wav", CurS, CurSubString + 'a');
		else
			sprintf(sn, "s%04d.wav", CurS);
	}

	TalkTime = g_vm->_soundMgr->talkStart(sn);
	if (!TalkTime)
		TalkTime = (strlen(SubString[CurSubString]) * 5) / 2 + 50;

	CurSubString++;

	doEvent(MC_STRING, ME_CHARACTERSPEAKING, MP_DEFAULT, 0, 0, 0, 0);
}

void CharacterMute() {
	g_vm->_flagSomeoneSpeaks = false;
	g_vm->_flagCharacterSpeak = false;
	g_vm->_flagSkipTalk = false;
	_characterSpeakTime = 0L;

	g_vm->clearText();
	g_vm->_lastObj = 0;
	g_vm->_lastInv = 0;

	g_vm->redrawString();
	g_vm->_soundMgr->talkStop();

	if ((g_vm->_curRoom == kRoom12CU) || (g_vm->_curRoom == kRoom13CU))
		doEvent(MC_SYSTEM, ME_CHANGEROOM, MP_SYSTEM, g_vm->_oldRoom, 0, 0, g_vm->_curObj);
}

void SomeoneTalk(uint16 s, uint16 Person, uint16 NewAnim, bool FromSomeOneSay) {
	SpeakSomeOneAnimation = NewAnim;
	SpeakSomeOnePerson = Person;
	g_vm->_flagSomeoneSpeaks = true;
	g_vm->_flagSkipTalk = false;

	CurS = s;
	SuperString = g_vm->_sentence[s];
	SuperStringLen = strlen(SuperString);
	SubStringStart = 0;
	CurSubString = 0;

	if (!FromSomeOneSay)
		g_vm->_flagSkipEnable = true;

	FormattingSuperString();

	if (SpeakSomeOneAnimation)
		doEvent(MC_ANIMATION, ME_ADDANIM, MP_SYSTEM, SpeakSomeOneAnimation, 0, 0, 0);
	doEvent(MC_STRING, ME_SOMEONEWAIT2SPEAK, MP_DEFAULT, SpeakSomeOneAnimation, 0, 0, 0);
}

void SomeoneContinueTalk() {
	uint16 posx, posy;

	_someoneSpeakTime = TheTime;
	g_vm->_flagSkipTalk = false;

	substringagain = (CurSubString < (SubStringUsed - 1));

	if (SpeakSomeOnePerson)
		PositionString(g_vm->_obj[SpeakSomeOnePerson]._lim.left, g_vm->_obj[SpeakSomeOnePerson]._lim.top, SubString[CurSubString], &posx, &posy, false);
	else
		PositionString(g_vm->_actor->_lim[0], g_vm->_actor->_lim[2], SubString[CurSubString], &posx, &posy, true);

	g_vm->clearText();
	if (ConfMan.getBool("subtitles"))
		g_vm->addText(posx, posy, SubString[CurSubString], HYELLOW, MASKCOL);

	if (CurSubString)
		sprintf(sn, "s%04d%c.wav", CurS, CurSubString + 'a');
	else
		sprintf(sn, "s%04d.wav", CurS);

	TalkTime = g_vm->_soundMgr->talkStart(sn);
	if (!TalkTime)
		TalkTime = (strlen(SubString[CurSubString]) * 5) / 2 + 50;

	CurSubString++;
	doEvent(MC_STRING, ME_SOMEONESPEAKING, MP_DEFAULT, 0, 0, 0, 0);
}

void someoneMute() {
	g_vm->_flagCharacterSpeak = false;
	g_vm->_flagSkipTalk = false;
	g_vm->_flagSomeoneSpeaks = false;
	_someoneSpeakTime = 0;

	g_vm->clearText();
	g_vm->_lastObj = 0;
	g_vm->_lastInv = 0;

	g_vm->redrawString();
	g_vm->_soundMgr->talkStop();
}

void doString() {
	switch (g_vm->_curMessage->_event) {
	case ME_CHARACTERSPEAK:
		CharacterSay(g_vm->_curMessage->_u16Param1);
		break;

	case ME_CHARACTERSPEAKING:
		if (g_vm->_flagCharacterSpeak) {
			if (g_vm->_flagSkipTalk || (TheTime > TalkTime + _characterSpeakTime)) {
				if (substringagain)
					CharacterContinueTalk();
				else
					CharacterMute();
			} else
				REEVENT;
		}
		break;

	case ME_SOMEONEWAIT2SPEAK:
		if (!g_vm->_curMessage->_u16Param1)
			SomeoneContinueTalk();
		else
			REEVENT;
		break;

	case ME_SOMEONEWAIT2MUTE:
		if (!g_vm->_curMessage->_u16Param1)
			someoneMute();
		else
			REEVENT;
		break;

	case ME_SOMEONESPEAKING:
		if (g_vm->_flagSomeoneSpeaks) {
			if (g_vm->_flagSkipTalk || (TheTime >= (TalkTime + _someoneSpeakTime))) {
				if (substringagain)
					SomeoneContinueTalk();
				else {
					if (SpeakSomeOneAnimation)
						doEvent(MC_ANIMATION, ME_DELANIM, MP_SYSTEM, SpeakSomeOneAnimation, true, 0, 0);
					doEvent(MC_STRING, ME_SOMEONEWAIT2MUTE, MP_DEFAULT, SpeakSomeOneAnimation, 0, 0, 0);
				}
			} else
				REEVENT;
		}
		break;
	default:
		break;
	}
}

} // End of namespace Trecision
