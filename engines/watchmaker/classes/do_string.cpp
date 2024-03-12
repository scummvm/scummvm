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

#include "watchmaker/classes/do_string.h"
#include "watchmaker/globvar.h"
#include "watchmaker/message.h"
#include "watchmaker/define.h"
#include "watchmaker/ll/ll_anim.h"
#include "watchmaker/schedule.h"
#include "watchmaker/ll/ll_string.h"
#include "watchmaker/windows_hacks.h"
#include "watchmaker/ll/ll_sound.h"
#include "watchmaker/t2d/expr.h"
#include "watchmaker/classes/do_sound.h"

// Locals
#define DEFAULT_TEXT_X      25
#define DEFAULT_TEXT_Y      537
#define DEFAULT_TEXT_DX     SCREEN_RES_X-DEFAULT_TEXT_X*2

namespace Watchmaker {

char UWStr[100];

/* -----------------18/03/98 11.11-------------------
 *                  PlayerSpeak
 * --------------------------------------------------*/
bool PlayerSpeak(WGame &game, int32 n) {
	int32 EndSpeakTime, SentTime;
	uint8 show_text;
	int32 speechn = 0;
	Init &init = game.init;

	if (!n) return false;

	if (Sentence[n][0] == '*') {
		StartAnim(game, aBOH);
		return false;
	} else if (Sentence[n][0] == '#') {
		StartAnim(game, aPENSA);
		return false;
	}

	SentTime = VisemaRecon(n);
	if ((n <= 15) && (bDialogActive)) {
		switch (n) {
		case 1:
			SentTime = 4560;
			init.Anim[aDUMMY_dR391_A].obj = ocOROLOGIAIO;
			break;
		case 2:
			SentTime = 1200;
			init.Anim[aDUMMY_dR391_A].obj = ocVECCHIO;
			break;
		case 3:
			SentTime = 5960;
			init.Anim[aDUMMY_dR391_A].obj = ocVECCHIO;
			break;
		case 4:
			SentTime = 8200;
			init.Anim[aDUMMY_dR391_A].obj = ocCHIRURGO;
			break;
		case 5:
			SentTime =  800;
			init.Anim[aDUMMY_dR391_A].obj = ocCHIRURGO;
			break;
		case 6:
			SentTime = 4560;
			init.Anim[aDUMMY_dR391_A].obj = ocVECCHIO;
			break;
		case 7:
			SentTime = 7200;
			init.Anim[aDUMMY_dR391_A].obj = ocCHIRURGO;
			break;
		case 8:
			SentTime = 7200;
			init.Anim[aDUMMY_dR391_A].obj = ocVECCHIO;
			break;
		case 9:
			SentTime = 5520;
			init.Anim[aDUMMY_dR391_A].obj = ocCHIRURGO;
			break;
		case 10:
			SentTime = 7720;
			init.Anim[aDUMMY_dR391_A].obj = ocVECCHIO;
			break;
		case 11:
			SentTime = 1600;
			init.Anim[aDUMMY_dR391_A].obj = ocCHIRURGO;
			break;
		case 12:
			SentTime = 1200;
			init.Anim[aDUMMY_dR391_A].obj = ocVECCHIO;
			break;
		case 13:
			SentTime = 2560;
			init.Anim[aDUMMY_dR391_A].obj = ocVECCHIO;
			break;
		case 14:
			SentTime = 6720;
			init.Anim[aDUMMY_dR391_A].obj = ocTRADUTTORE;
			break;
		case 15:
			SentTime = 2200;
			init.Anim[aDUMMY_dR391_A].obj = ocVECCHIO;
			break;
		}
	}

	if (game.gameOptions.subtitles_on)  show_text = 1;
	else                            show_text = 0;

	if (game.gameOptions.speech_on) {
		if (StartSpeech(game, n))   {
			speechn = MAX_SOUNDS + n;
			DebugLogWindow("PP %d, SentTime %d", speechn, SentTime);
		} else                    show_text = 1;
	}

	if (show_text) {
		if (InvStatus & INV_ON)
			Text(DEFAULT_TEXT_X, DEFAULT_TEXT_Y, DEFAULT_TEXT_DX, Sentence[n]);
		else
			Text(20, DEFAULT_TEXT_Y, DEFAULT_TEXT_DX, Sentence[n]);
	}

//	DebugLogFile("PLS (%d) %s",SentTime,Sentence[n]);
	EndSpeakTime = TheTime + SentTime;
	bSomeOneSpeak = TRUE;
	bPlayerSpeak = TRUE;
	bSkipTalk = FALSE;

	if (speechn)
		_vm->_messageSystem.doEvent(EventClass::MC_STRING, ME_PLAYERCONTINUESPEAK_WAITWAVE, MP_WAIT_RETRACE, 0, 0, 0, &speechn, nullptr, nullptr);
	else
		_vm->_messageSystem.doEvent(EventClass::MC_STRING, ME_PLAYERCONTINUESPEAK, MP_WAIT_RETRACE, 0, 0, 0, &EndSpeakTime, nullptr, nullptr);
	return TRUE;
}


/* -----------------18/03/98 11.06-------------------
 *                      doString
 * --------------------------------------------------*/
void doString(WGame &game) {
	switch (TheMessage->event) {
	case ME_PLAYERSPEAK:
		PlayerSpeak(game, TheMessage->wparam1);
		break;

	case ME_PLAYERCONTINUESPEAK:
		if (((int32)TheTime > TheMessage->lparam[0]) || (bSkipTalk)) {
			bSomeOneSpeak = false;
			bPlayerSpeak = false;
			bSkipTalk = false;
			bAnimWaitText = false;
			ClearText();
			if (!bDialogActive) _vm->_messageSystem.addWaitingMsgs(MP_WAIT_LINK);
		} else {
			TheMessage->flags |= MP_WAIT_RETRACE;
			ReEvent();
		}
		break;
	case ME_PLAYERCONTINUESPEAK_WAITWAVE: {
		bool isp;

		isp = sIsPlaying(TheMessage->lparam[0]);

		if ((!isp) || bSkipTalk) {
			if (bSkipTalk && isp) {
				sStopSound(TheMessage->lparam[0]);
			}

			bSomeOneSpeak = false;
			bPlayerSpeak = false;
			bSkipTalk = false;
			bAnimWaitText = false;
			ClearText();
			if (!bDialogActive) _vm->_messageSystem.addWaitingMsgs(MP_WAIT_LINK);
		} else {
			TheMessage->flags |= MP_WAIT_RETRACE;
			ReEvent();
		}
	}
	break;
	}
}

/* -----------------17/03/98 16.19-------------------
 *                  ShowObjName
 * --------------------------------------------------*/
void ShowObjName(Init &init, int32 ob) {
	if (bUseWith & UW_ON) {
		if (bUseWith & UW_USEDI)
			snprintf(UWStr, 100, "Use %s with ", ObjName[init.InvObj[UseWith[USED]].name]);
		else
			snprintf(UWStr, 100, "Use %s with ", ObjName[init.Obj[UseWith[USED]].name]);

		if ((UseWith[USED] != ob) || (bUseWith & UW_USEDI))
			if (ObjName[init.Obj[ob].name] != nullptr)
				Common::strlcat(UWStr, ObjName[init.Obj[ob].name], 100);
		Text(DEFAULT_TEXT_X, DEFAULT_TEXT_Y, DEFAULT_TEXT_DX, UWStr);
	} else if (!ob || !init.Obj[ob].name)
		ClearText();
	else
		Text(DEFAULT_TEXT_X, DEFAULT_TEXT_Y, DEFAULT_TEXT_DX, ObjName[init.Obj[ob].name]);
}

/* -----------------17/03/98 16.19-------------------
 *                  ShowInvObjName
 * --------------------------------------------------*/
void ShowInvObjName(Init &init, int32 obj) {
	if (bUseWith & UW_ON) {
		if (bUseWith & UW_USEDI)
			snprintf(UWStr, 100, "Use %s with ", ObjName[init.InvObj[UseWith[USED]].name]);
		else
			snprintf(UWStr, 100, "Use %s with ", ObjName[init.Obj[UseWith[USED]].name]);

		if ((UseWith[USED] != obj) || !(bUseWith & UW_USEDI))
			if (ObjName[init.InvObj[obj].name] != nullptr)
				Common::strlcat(UWStr, ObjName[init.InvObj[obj].name], 100);
		Text(DEFAULT_TEXT_X, DEFAULT_TEXT_Y, DEFAULT_TEXT_DX, UWStr);
	} else //if ( !obj )
		ClearText();
//	else
//		Text( 100, d3dappi.szClient.cy-30, ObjName[InvObj[obj].name] );
}

} // End of namespace Watchmaker
