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

#include <string.h>
#include <stdio.h>
#include "trecision/nl/lib/addtype.h"
#include "trecision/nl/3d/3dinc.h"
#include "trecision/nl/sysdef.h"
#include "trecision/nl/struct.h"
#include "trecision/nl/define.h"
#include "trecision/nl/message.h"
#include "trecision/nl/extern.h"
#include "trecision/trecision.h"

#include "common/config-manager.h"

namespace Trecision {

// Variabili di servizio comuni a piu' funzioni di string.c
uint16 CurS, lastinv, lastobj;

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
extern uint16 _playingAnims[];
const char *dunno = "?";

/*-------------------------------------------------------------------------*/
/*                                 REPAINTSTRING         				   */
/*-------------------------------------------------------------------------*/
void RepaintString() {
	if (!FlagDialogActive && !FlagDialogMenuActive && !FlagSomeOneSpeak && !Flagscriptactive && FlagMouseEnabled) {
		if (INVAREA(my))
			doEvent(MC_INVENTORY, ME_SHOWICONNAME, MP_DEFAULT, 0, 0, 0, 0);
		else {
			CheckMask(mx, my);
			ShowObjName(g_vm->_curObj, true);
		}
	}
}

/*-------------------------------------------------------------------------*/
/*                                 POSITIONSTRING           			   */
/*-------------------------------------------------------------------------*/
void PositionString(uint16 x, uint16 y, const char *string, uint16 *posx, uint16 *posy, bool characterFl) {
	uint16 TheVideoLen = MAXX;

	uint16 LenText = TextLength(string, 0);
	if (LenText > 960)
		LenText = (LenText * 2 / 5);
	else if (LenText > 320)
		LenText = (LenText * 3 / 5);

	if (x > (LenText >> 1))
		x -= (LenText >> 1);
	else
		x = 0;

	if (x < 5)
		*posx = 5;
	else if ((LenText + x) > (TheVideoLen - 5))
		*posx = TheVideoLen - LenText - 5;
	else
		*posx = x;

	*posy = characterFl ? 0 : VIDEOTOP;
	*posy += y - 1; //15
	if (*posy <= VIDEOTOP)
		*posy = VIDEOTOP + 1;
}

/*-------------------------------------------------------------------------*/
/*                                 SHOWOBJNAME           				   */
/*-------------------------------------------------------------------------*/
void ShowObjName(uint16 obj, bool showhide) {
	uint16 posx;
	uint16 posy;
	char locsent[256];

	if (FlagSomeOneSpeak)
		return;
	if (lastinv) {
		ClearText();
		lastinv = 0;
	}

	if (FlagUseWithStarted && !FlagUseWithLocked) {
		if (!showhide) {
			ClearText();
			lastobj = obj;
			return;
		}

		if ((g_vm->_obj[g_vm->_curObj]._flag & (OBJFLAG_ROOMOUT | OBJFLAG_ROOMIN)) && !(g_vm->_obj[g_vm->_curObj]._flag & OBJFLAG_EXAMINE))
			return;

		strcpy(locsent, g_vm->_sysText[23]);
		if (g_vm->_useWithInv[USED])
			strcat(locsent, g_vm->_objName[g_vm->_inventoryObj[g_vm->_useWith[USED]]._name]);
		else if (g_vm->_obj[g_vm->_useWith[USED]]._mode & OBJMODE_HIDDEN)
			strcat(locsent, dunno);
		else
			strcat(locsent, g_vm->_objName[g_vm->_obj[g_vm->_useWith[USED]]._name]);

		strcat(locsent, g_vm->_sysText[24]);
		if (obj && (g_vm->_useWithInv[USED] || (obj != g_vm->_useWith[USED]))) {
			if (g_vm->_obj[obj]._mode & OBJMODE_HIDDEN)
				strcat(locsent, dunno);
			else
				strcat(locsent, g_vm->_objName[g_vm->_obj[obj]._name]);
		}

		posx = 320;
		posy = MAXY - CARHEI;
		lastobj = (obj | 0x8000);
		uint16 LenText = TextLength(locsent, 0);
		if (posx - (LenText / 2) < 2)
			posx = 2;
		else
			posx = posx - (LenText / 2);

		if ((posx + LenText) > SCREENLEN - 2)
			posx = SCREENLEN - 2 - LenText;

		if (lastobj)
			ClearText();
		Text(posx, posy, locsent, COLOR_INVENTORY, MASKCOL);
	} else {
		if ((!obj) || (!showhide)) {
			ClearText();
			lastobj = obj;
			return;
		}

		if (obj == lastobj)
			return;
		if (!(g_vm->_obj[obj]._flag & OBJFLAG_EXAMINE)) {
			if ((g_vm->_obj[obj]._flag & OBJFLAG_DONE) || (g_vm->_room[g_vm->_obj[obj]._goRoom]._flag & OBJFLAG_DONE)) {
				strcpy(locsent, g_vm->_sysText[25]);
				if (g_vm->_obj[obj]._mode & OBJMODE_HIDDEN)
					strcat(locsent, dunno);
				else
					strcat(locsent, g_vm->_objName[g_vm->_obj[obj]._name]);
			} else
				strcpy(locsent, g_vm->_sysText[26]);
		} else if (g_vm->_obj[obj]._mode & OBJMODE_HIDDEN)
			strcpy(locsent, dunno);
		else
			strcpy(locsent, g_vm->_objName[g_vm->_obj[obj]._name]);

		switch (obj) {
		case oRUOTE2C:
			posx = (g_vm->_obj[obj]._lim[0] + g_vm->_obj[obj]._lim[2]) / 2;
			posy = 187;
			break;

		default:
			posx = (g_vm->_obj[obj]._lim[0] + g_vm->_obj[obj]._lim[2]) / 2;
			posy = g_vm->_obj[obj]._lim[1];
			break;
		}
		PositionString(posx, posy, locsent, &posx, &posy, false);
		if (lastobj)
			ClearText();
		lastobj = obj;
		Text(posx, posy, locsent, COLOR_OBJECT, MASKCOL);
	}
}

/*-------------------------------------------------------------------------*/
/*                                 SHOWINVNAME           				   */
/*-------------------------------------------------------------------------*/
void ShowInvName(uint16 obj, bool showhide) {
	uint16 posx;
	uint16 posy;
	uint16 LenText;

	if ((g_vm->_curRoom == r2BL) || (g_vm->_curRoom == r36F) || (g_vm->_curRoom == r41D) || (g_vm->_curRoom == r49M) || (g_vm->_curRoom == r4CT)
		|| (g_vm->_curRoom == r58T) || (g_vm->_curRoom == r58M) || (g_vm->_curRoom == r59L) || (g_vm->_curRoom == rSYS) || (g_vm->_curRoom == r12CU)
		|| (g_vm->_curRoom == r13CU))
		return;

	if (FlagSomeOneSpeak)
		return;

	if (lastobj) {
		ClearText();
		lastobj = 0;
	}

	if (FlagUseWithStarted && !FlagUseWithLocked) {
		if (/*(!obj) ||*/ (!showhide)) {
			ClearText();
			lastinv = 0;
			return;
		}
		if ((obj | 0x8000) == lastinv)
			return;

		char locsent[256];
		strcpy(locsent, g_vm->_sysText[23]);
		if (g_vm->_useWithInv[USED]) {
			strcat(locsent, g_vm->_objName[g_vm->_inventoryObj[g_vm->_useWith[USED]]._name]);
			strcat(locsent, g_vm->_sysText[24]);
			if (obj && (g_vm->_inventoryObj[g_vm->_useWith[USED]]._name != g_vm->_inventoryObj[obj]._name))
				strcat(locsent, g_vm->_objName[g_vm->_inventoryObj[obj]._name]);
		} else {
			if (g_vm->_obj[g_vm->_useWith[USED]]._mode & OBJMODE_HIDDEN)
				strcat(locsent, dunno);
			else
				strcat(locsent, g_vm->_objName[g_vm->_obj[g_vm->_useWith[USED]]._name]);
			strcat(locsent, g_vm->_sysText[24]);
			if (obj && (g_vm->_obj[g_vm->_useWith[USED]]._name != g_vm->_inventoryObj[obj]._name))
				strcat(locsent, g_vm->_objName[g_vm->_inventoryObj[obj]._name]);
		}
		posx = 320;
		//   	posx = ICONMARGSX+((IconPos(_useWith[USED])-_iconBase)*(ICONDX))+ICONDX/2;
		posy = MAXY - CARHEI;
		lastinv = (obj | 0x8000);
		LenText = TextLength(locsent, 0);
		if (posx - (LenText / 2) < 2)
			posx = 2;
		else
			posx = posx - (LenText / 2);

		if ((posx + LenText) > SCREENLEN - 2)
			posx = SCREENLEN - 2 - LenText;

		if (lastinv)
			ClearText();
		Text(posx, posy, locsent, COLOR_INVENTORY, MASKCOL);
	} else {
		if (obj == lastinv)
			return;

		if (!obj || !showhide) {
			ClearText();
			lastinv = 0;
			return;
		}
		posx = ICONMARGSX + ((IconPos(g_vm->_curInventory) - g_vm->_iconBase) * (ICONDX)) + ICONDX / 2;
		posy = MAXY - CARHEI;
		lastinv = obj;
		LenText = TextLength(g_vm->_objName[g_vm->_inventoryObj[obj]._name], 0);
		if (posx - (LenText / 2) < 2)
			posx = 2;
		else
			posx = posx - (LenText / 2);

		if ((posx + LenText) > SCREENLEN - 2)
			posx = SCREENLEN - 2 - LenText;

		if (lastinv)
			ClearText();

		if (g_vm->_inventoryObj[obj]._name)
			Text(posx, posy, g_vm->_objName[g_vm->_inventoryObj[obj]._name], COLOR_INVENTORY, MASKCOL);
	}
}

/*-------------------------------------------------------------------------*/
/*                             FORMATTINGSUPERSTRING             		   */
/*-------------------------------------------------------------------------*/
void FormattingSuperString() {
	SubStringUsed  = 0;
	substringagain = true;
	while (substringagain) {
		FormattingOneString();
		SubStringUsed++;
	}
}

/* -----------------08/07/97 22.23-------------------
				FormattingOneString
 --------------------------------------------------*/
void FormattingOneString() {
	uint16 i;
	memset(SubString[SubStringUsed], '\0', MAXLENSUBSTRING);

	uint16 SpazioRimasto = (SuperStringLen - SubStringStart);
	for (i = 0; i < SpazioRimasto; i++) {
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

/*-------------------------------------------------------------------------*/
/*                                characterSay             				   */
/*-------------------------------------------------------------------------*/
void CharacterSay(uint16 i) {
	CurS = i;

	//	TODO: Use a define...  ************************
	Flagskipenable = (i != 99999);

	//	if he took some action
	if ((g_vm->_sentence[i][0] == '*') && (_playingAnims[1] == 0))
		StartCharacterAction(hBOH, 0, 0, 0);
	else
		CharacterTalk(g_vm->_sentence[i], true);
}

/*-------------------------------------------------------------------------*/
/*                                CharacterTalk            				   */
/*-------------------------------------------------------------------------*/
void CharacterTalk(const char *s, bool FromCharacterSay) {
	FlagSomeOneSpeak = true;
	FlagCharacterSpeak = true;
	Flagskiptalk = false;

	SuperString = s;
	SuperStringLen = strlen(SuperString);
	SubStringStart = 0;
	CurSubString = 0;
	FormattingSuperString();

	if (!FromCharacterSay)
		Flagskipenable = true;

	CharacterContinueTalk();

	g_vm->_characterQueue.initQueue();
	actorStop();
}

/* -----------------08/07/97 22.26-------------------
				CharacterTalkInAction
 --------------------------------------------------*/
void CharacterTalkInAction(uint16 ss) {
	const char *s = g_vm->_sentence[ss];

	if (g_vm->_sentence[ss][0] == '*')
		return;
	CurS = ss;

	FlagSomeOneSpeak = true;
	FlagCharacterSpeak = true;
	Flagskiptalk = false;
	Flagskipenable = true;

	SuperString = s;
	SuperStringLen = strlen(SuperString);
	SubStringStart = 0;
	CurSubString = 0;
	FormattingSuperString();

	CharacterContinueTalk();
}

/*-------------------------------------------------------------------------*/
/*                            CharacterContinueTalk                		   */
/*-------------------------------------------------------------------------*/
void CharacterContinueTalk() {
	uint16 posx, posy;

	Flagskiptalk = false;
	CharacterSpeakTime = TheTime;

	substringagain = (CurSubString < (SubStringUsed - 1));

	if (FlagCharacterExist)
		PositionString(_actor._lim[0], _actor._lim[2], SubString[CurSubString], &posx, &posy, true);
	else
		PositionString(MAXX / 2, 30, SubString[CurSubString], &posx, &posy, false);

	ClearText();
	if (ConfMan.getBool("subtitles"))
		Text(posx, posy, SubString[CurSubString], COLOR_OBJECT, MASKCOL);

	if (!FlagDialogActive) {
		if (CurSubString)
			sprintf(sn, "s%04d%c.wav", CurS, CurSubString + 'a');
		else
			sprintf(sn, "s%04d.wav", CurS);
	}

	TalkTime = Talk(sn);
	if (!TalkTime)
		TalkTime = (strlen(SubString[CurSubString]) * 5) / 2 + 50;

	CurSubString++;

	doEvent(MC_STRING, ME_CHARACTERSPEAKING, MP_DEFAULT, 0, 0, 0, 0);
}

/*-------------------------------------------------------------------------*/
/*                                CharacterMute            				   */
/*-------------------------------------------------------------------------*/
void CharacterMute() {
	FlagSomeOneSpeak = false;
	FlagCharacterSpeak = false;
	Flagskiptalk = false;
	CharacterSpeakTime = 0L;

	ClearText();
	lastobj = 0;
	lastinv = 0;

	RepaintString();
	StopTalk();

	if ((g_vm->_curRoom == r12CU) || (g_vm->_curRoom == r13CU))
		doEvent(MC_SYSTEM, ME_CHANGEROOM, MP_SYSTEM, g_vm->_oldRoom, 0, 0, g_vm->_curObj);

	//	actorStop();
	//	if(FlagDialogActive) doEvent(MC_DIALOG,ME_FINEBATTUTA,MP_DEFAULT,0,0,0,0);
}

/*-------------------------------------------------------------------------*/
/*                                 SOMEONETALK            				   */
/*-------------------------------------------------------------------------*/
void SomeOneTalk(uint16 s, uint16 Person, uint16 NewAnim, bool FromSomeOneSay) {
	SpeakSomeOneAnimation = NewAnim;
	SpeakSomeOnePerson = Person;
	FlagSomeOneSpeak = true;
	Flagskiptalk = false;

	CurS = s;
	SuperString = g_vm->_sentence[s];
	SuperStringLen = strlen(SuperString);
	SubStringStart = 0;
	CurSubString = 0;

	if (!FromSomeOneSay)
		Flagskipenable = true;

	FormattingSuperString();

	if (SpeakSomeOneAnimation)
		doEvent(MC_ANIMATION, ME_ADDANIM, MP_SYSTEM, SpeakSomeOneAnimation, 0, 0, 0);
	doEvent(MC_STRING, ME_SOMEONEWAIT2SPEAK, MP_DEFAULT, SpeakSomeOneAnimation, 0, 0, 0);
}

/*-------------------------------------------------------------------------*/
/*                             SOMEONECONTINUETALK             			   */
/*-------------------------------------------------------------------------*/
void SomeOneContinueTalk() {
	uint16 posx, posy;

	SomeOneSpeakTime = TheTime;
	Flagskiptalk = false;

	substringagain = (CurSubString < (SubStringUsed - 1));

	if (SpeakSomeOnePerson)
		PositionString(g_vm->_obj[SpeakSomeOnePerson]._lim[0], g_vm->_obj[SpeakSomeOnePerson]._lim[1], SubString[CurSubString], &posx, &posy, false);
	else
		PositionString(_actor._lim[0], _actor._lim[2], SubString[CurSubString], &posx, &posy, true);

	ClearText();
	if (ConfMan.getBool("subtitles"))
		Text(posx, posy, SubString[CurSubString], HYELLOW, MASKCOL);

	if (CurSubString)
		sprintf(sn, "s%04d%c.wav", CurS, CurSubString + 'a');
	else
		sprintf(sn, "s%04d.wav", CurS);
	//	sprintf(sn,"d%02d%03d%02d",_curDialog,_subTitles[_curSubTitle]._sentence,CurSubString);
	TalkTime = Talk(sn);
	if (!TalkTime)
		TalkTime = (strlen(SubString[CurSubString]) * 5) / 2 + 50;

	CurSubString++;
	doEvent(MC_STRING, ME_SOMEONESPEAKING, MP_DEFAULT, 0, 0, 0, 0);
}

/*-------------------------------------------------------------------------*/
/*                                SOMEONEMUTE                 			   */
/*-------------------------------------------------------------------------*/
void SomeOneMute() {
	FlagCharacterSpeak = false;
	Flagskiptalk = false;
	FlagSomeOneSpeak = false;
	SomeOneSpeakTime = 0L;

	ClearText();
	lastobj = 0;
	lastinv = 0;

	RepaintString();
	StopTalk();

	//F	if(FlagDialogActive) doEvent(MC_DIALOG,ME_FINEBATTUTA,MP_DEFAULT,0,0,0,0);
}

/*-------------------------------------------------------------------------*/
/*                                    DOSTRING            				   */
/*-------------------------------------------------------------------------*/
void doString() {
	switch (g_vm->_curMessage->_event) {
	case ME_CHARACTERSPEAK:
		CharacterSay(g_vm->_curMessage->_u16Param1);
		break;

	case ME_CHARACTERSPEAKING:
		if (FlagCharacterSpeak) {
			if (Flagskiptalk || (TheTime > (TalkTime + CharacterSpeakTime))) {
				if (substringagain)
					CharacterContinueTalk();
				else
					CharacterMute();
			} else {
				//	actorDoNextFrame();
				REEVENT;
			}
		}
		break;

	case ME_SOMEONEWAIT2SPEAK:
		if ((!g_vm->_curMessage->_u16Param1) /*FAB: || (AnimObj[_curMessage->_u16Param1]._flag & ONOFF)*/)
			SomeOneContinueTalk();
		else
			REEVENT;
		break;

	case ME_SOMEONEWAIT2MUTE:
		if ((!g_vm->_curMessage->_u16Param1) /*FAB: || (!(AnimObj[_curMessage->_u16Param1]._flag & ONOFF))*/)
			SomeOneMute();
		else
			REEVENT;
		break;

	case ME_SOMEONESPEAKING:
		if (FlagSomeOneSpeak) {
			if (Flagskiptalk || (TheTime >= (TalkTime + SomeOneSpeakTime))) {
				if (substringagain)
					SomeOneContinueTalk();
				else {
					if (SpeakSomeOneAnimation)
						doEvent(MC_ANIMATION, ME_DELANIM, MP_SYSTEM, SpeakSomeOneAnimation, true, 0, 0);
					doEvent(MC_STRING, ME_SOMEONEWAIT2MUTE, MP_DEFAULT, SpeakSomeOneAnimation, 0, 0, 0);
				}
			} else
				REEVENT;
		}
		break;
	}
}

} // End of namespace Trecision
