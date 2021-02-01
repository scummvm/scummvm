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

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "trecision/nl/lib/addtype.h"
#include "trecision/nl/sysdef.h"
#include "trecision/nl/struct.h"
#include "trecision/nl/define.h"
#include "trecision/nl/message.h"
#include "trecision/nl/ll/llinc.h"
#include "trecision/nl/extern.h"
#include "trecision/nl/3d/3dinc.h"

namespace Trecision {

#define SendFrame(i) doEvent(ScriptFrame[i].cls, 	\
							ScriptFrame[i].event,   \
							MP_DEFAULT,             \
							ScriptFrame[i].wparam1, \
							ScriptFrame[i].wparam2, \
							ScriptFrame[i].bparam , \
							ScriptFrame[i].lparam)

uint8 CurStack = 0;

#define MAXTEXTSTACK	3

struct StackText {
	uint16 x;
	uint16 y;
	uint16 tcol, scol;
	char  sign[256];
	LLBOOL  Clear;
} TextStack[MAXTEXTSTACK];

int16 TextStackTop = -1;

/*-------------------------------------------------------------------------*/
/*                            ONELEFTINVENTORY          				   */
/*-------------------------------------------------------------------------*/
void OneLeftInventory() {
	extern short RightArrow;
	if (TheIconBase < _inventorySize - ICONSHOWN)
		TheIconBase ++;
	RegenInv(TheIconBase, INVENTORY_SHOW);
	RightArrow = 1;
}

/*-------------------------------------------------------------------------*/
/*                            ONERIGHTINVENTORY          				   */
/*-------------------------------------------------------------------------*/
void OneRightInventory() {
	extern short LeftArrow;
	if (TheIconBase > 0)
		TheIconBase --;
	RegenInv(TheIconBase, INVENTORY_SHOW);
	LeftArrow = 1;
}

/*-------------------------------------------------------------------------*/
/*                                WHATICON           					   */
/*-------------------------------------------------------------------------*/
uint8 WhatIcon(uint16 invmx) {
	if (invmx < ICONMARGSX)
		return 0;
	if (invmx > (SCREENLEN - ICONMARGDX))
		return 0;

	return _inventory[(TheIconBase + ((invmx - ICONMARGSX) / (ICONDX)))];
}

/*-------------------------------------------------------------------------*/
/*                                 ICONPOS           					   */
/*-------------------------------------------------------------------------*/
uint8 IconPos(uint8 icon) {
	uint8 i;

	for (i = 0; i < MAXICON; i++) {
		if (_inventory[i] == icon)
			break;
	}

	return i;
}

/*-------------------------------------------------------------------------*/
/*                                KILLICON           					   */
/*-------------------------------------------------------------------------*/
void KillIcon(uint8 icon) {
	uint8 pos = IconPos(icon);

	if (pos == MAXICON)
		return;
	_inventory[pos] = iNULL;
	for (; pos < _inventorySize; pos++)
		_inventory[pos] = _inventory[pos + 1];
	_inventorySize --;

	if (_inventorySize < ICONSHOWN)
		TheIconBase = 0;

	if ((TheIconBase) && (_inventorySize > ICONSHOWN) && (_inventory[TheIconBase + ICONSHOWN] == iNULL))
//		TheIconBase --;
		TheIconBase = _inventorySize - ICONSHOWN;

	RepaintString();
}

/*-------------------------------------------------------------------------*/
/*                                 ADDICON           					   */
/*-------------------------------------------------------------------------*/
void AddIcon(uint8 icon) {
	if (IconPos(icon) != MAXICON)
		return;
	_inventory[_inventorySize++] = icon;
	if (_inventorySize >= MAXICON)
		warning("AddIcon overflow");

	if (TheIconBase < _inventorySize - ICONSHOWN)
//		TheIconBase ++;
		TheIconBase = _inventorySize - ICONSHOWN;

//	To show the icon that enters the inventory
//	doEvent(MC_INVENTORY,ME_OPEN,MP_DEFAULT,0,0,0,0);
//	SemForceRegenInventory = true;
	RepaintString();
}

/*-------------------------------------------------------------------------*/
/*                               REPLACEICON          					   */
/*-------------------------------------------------------------------------*/
void ReplaceIcon(uint8 oldicon, uint8 newicon) {
	uint8 pos = IconPos(oldicon);

	_inventory[pos] = newicon;
}

/*-------------------------------------------------------------------------*/
/*                                 REGENINV            					   */
/*-------------------------------------------------------------------------*/
void RegenInv(uint8 StartIcon, uint8 StartLine) {
	RegenInvStartIcon = StartIcon;
	RegenInvStartLine = StartLine;
}

/*-------------------------------------------------------------------------*/
/*                                ENDSCRIPT           					   */
/*-------------------------------------------------------------------------*/
void EndScript() {
	CurStack--;
	if (CurStack == 0) {
		Semscriptactive = false;
		SemMouseEnabled = true;
		RepaintString();
	}
}

/*-------------------------------------------------------------------------*/
/*                               PLAYSCRIPT           					   */
/*-------------------------------------------------------------------------*/
void PlayScript(uint16 i) {
	CurStack++;
	Semscriptactive = true;
	SemMouseEnabled = false;
	CurScriptFrame[CurStack] = Script[i].firstframe;

	// se evento vuoto termina lo scrpt
	if ((ScriptFrame[CurScriptFrame[CurStack]].cls == 0) && (ScriptFrame[CurScriptFrame[CurStack]].event == 0)) {
		EndScript();
		return;
	}

LOOP:
	SendFrame(CurScriptFrame[CurStack]);
	if ((ScriptFrame[CurScriptFrame[CurStack]].nowait) && !((ScriptFrame[CurScriptFrame[CurStack] + 1].cls == 0) && (ScriptFrame[CurScriptFrame[CurStack] + 1].event == 0))) {
		CurScriptFrame[CurStack]++;
		goto LOOP;
	}
}

/*-------------------------------------------------------------------------*/
/*                               EVALSCRIPT           					   */
/*-------------------------------------------------------------------------*/
void EvalScript() {
	if ((TestEmptyHomoQueue4Script(&Homo) && (TestEmptyQueue(&Game, MC_DIALOG)) && (SemScreenRefreshed))) {
//if(( Homo.len == 0 ) && (TestEmptyQueue(&Game,MC_DIALOG)) && (SemScreenRefreshed)) {
		CurScriptFrame[CurStack]++;
		SemMouseEnabled = false;

		if ((ScriptFrame[CurScriptFrame[CurStack]].cls == 0) && (ScriptFrame[CurScriptFrame[CurStack]].event == 0)) {
			EndScript();
			return;
		}

LOOP:
		SendFrame(CurScriptFrame[CurStack]);
		if ((ScriptFrame[CurScriptFrame[CurStack]].nowait) && !((ScriptFrame[CurScriptFrame[CurStack] + 1].cls == 0) && (ScriptFrame[CurScriptFrame[CurStack] + 1].event == 0))) {
			CurScriptFrame[CurStack]++;
			goto LOOP;
		}
	}
}

/*-------------------------------------------------------------------------*/
/*                                   TEXT              					   */
/*-------------------------------------------------------------------------*/
void Text(uint16 x, uint16 y, const char *sign, uint16 tcol, uint16 scol) {
	TextStackTop++;
	if (TextStackTop >= MAXTEXTSTACK) {
		warning("MaxTextStackReached!");
		return;
	}

	TextStack[TextStackTop].x     = x;
	TextStack[TextStackTop].y     = y;
	TextStack[TextStackTop].tcol  = tcol;
	TextStack[TextStackTop].scol  = scol;
	TextStack[TextStackTop].Clear = false;
	strcpy(TextStack[TextStackTop].sign, sign);
}

/* -----------------08/07/97 22.13-------------------
						ClearText
 --------------------------------------------------*/
void ClearText() {
	if (TextStackTop >= 0) {
//		lo stack non e' vuoto
		if (! TextStack[TextStackTop].Clear)
//			il precedente e' una stringa da scrivere, torno indietro
			TextStackTop--;
	} else {
//		lo stack e' vuoto
		TextStackTop = 0;
		TextStack[TextStackTop].Clear = true;
	}
}

/* -----------------08/07/97 22.14-------------------
					PaintString
 --------------------------------------------------*/
void PaintString() {
	for (int16 i = 0; i <= TextStackTop; i++) {
		if (TextStack[i].Clear) {
			DoClearText();
		} else {
			DoText(TextStack[i].x, TextStack[i].y,
				   TextStack[i].sign,
				   TextStack[i].tcol, TextStack[i].scol);
		}
	}
}

/* -----------------08/07/97 22.15-------------------
						DoText
 --------------------------------------------------*/
void DoText(uint16 x, uint16 y, const char *sign, uint16 tcol, uint16 scol) {
	uint16 hstring;

	TheString.x = x;
	TheString.y = y;
	TheString.dx = TextLength(sign, 0);
	if ((y == (MAXY - CARHEI)) && (TheString.dx > 600)) TheString.dx = TheString.dx * 3 / 5;
	else if ((y != (MAXY - CARHEI)) && (TheString.dx > 960)) TheString.dx = TheString.dx * 2 / 5;
	else if ((y != (MAXY - CARHEI)) && (TheString.dx > 320)) TheString.dx = TheString.dx * 3 / 5;
	TheString.sign = sign;
	TheString.l[0] = 0;
	TheString.l[1] = 0;
	TheString.l[2] = TheString.dx;
	TheString.l[3] = (hstring = CheckDText(TheString));
	TheString.dy   = hstring;
	TheString.tcol = tcol;
	TheString.scol = scol;

	if (TheString.y <= hstring)
		TheString.y += hstring;
	else
		TheString.y -= hstring;

	if (TheString.y <= VIDEOTOP)
		TheString.y  = VIDEOTOP + 1;


	TextStatus |= TEXT_DRAW;
}


/* -----------------08/07/97 22.15-------------------
					DoClearString
 --------------------------------------------------*/
void DoClearText() {
	if ((OldString.sign == NULL) && (TheString.sign)) {
		OldString.x = TheString.x;
		OldString.y = TheString.y;
		OldString.dx = TheString.dx;
		OldString.dy = TheString.dy;
		OldString.sign = TheString.sign;
		OldString.l[0] = TheString.l[0];
		OldString.l[1] = TheString.l[1];
		OldString.l[2] = TheString.l[2];
		OldString.l[3] = TheString.l[3];
		OldString.tcol = TheString.tcol;
		OldString.scol = TheString.scol;
		TheString.sign = NULL;

		TextStatus |= TEXT_DEL;
	}
}

/* -----------------21/01/98 10.22-------------------
 * 				DoSys
 * --------------------------------------------------*/
void DoSys(uint16 TheObj) {
	extern short SpeechON;
	extern short TextON;
	extern short SFxVol;
	extern short SpeechVol;
	extern short MusicVol;

	switch (TheObj) {
	case o00QUIT:
		if (QuitGame())
			doEvent(MC_SYSTEM, ME_QUIT, MP_SYSTEM, 0, 0, 0, 0);
		break;

	case o00EXIT:
		if (OldRoom == rSYS)
			break;
		doEvent(MC_SYSTEM, ME_CHANGEROOM, MP_SYSTEM, _obj[o00EXIT]._goRoom, 0, 0, 0);
		break;

	case o00SAVE:
		if (OldRoom == rSYS)
			break;
		_curRoom = _obj[o00EXIT]._goRoom;
		SemSaveInventory = true;
		if (!DataSave()) {
			ShowInvName(NO_OBJECTS, false);
			doEvent(MC_INVENTORY, ME_SHOWICONNAME, MP_DEFAULT, mx, my, 0, 0);
//				RegenInventory(RegenInvStartIcon,RegenInvStartLine);
			doEvent(MC_SYSTEM, ME_CHANGEROOM, MP_SYSTEM, _obj[o00EXIT]._goRoom, 0, 0, 0);
		}
		_curRoom = rSYS;
		break;

	case o00LOAD:
		if (!DataLoad()) {
			ShowInvName(NO_OBJECTS, false);
			doEvent(MC_INVENTORY, ME_SHOWICONNAME, MP_DEFAULT, mx, my, 0, 0);
//				RegenInventory(RegenInvStartIcon,RegenInvStartLine);
		}
		break;

	case o00SPEECHON:
		if (TextON) {
			_obj[o00SPEECHON]._mode &= ~OBJMODE_OBJSTATUS;
			_obj[o00SPEECHOFF]._mode |= OBJMODE_OBJSTATUS;
			SpeechON = false;
			_curObj = o00SPEECHOFF;
			RegenRoom();
			ShowObjName(_curObj, true);
		}
		break;

	case o00SPEECHOFF:
		_obj[o00SPEECHOFF]._mode &= ~OBJMODE_OBJSTATUS;
		_obj[o00SPEECHON]._mode |= OBJMODE_OBJSTATUS;
		SpeechON = true;
		_curObj = o00SPEECHON;
		RegenRoom();
		ShowObjName(_curObj, true);
		break;

	case o00TEXTON:
		if (SpeechON) {
			_obj[o00TEXTON]._mode &= ~OBJMODE_OBJSTATUS;
			_obj[o00TEXTOFF]._mode |= OBJMODE_OBJSTATUS;
			TextON = false;
			_curObj = o00TEXTOFF;
			RegenRoom();
			ShowObjName(_curObj, true);
		}
		break;

	case o00TEXTOFF:
		_obj[o00TEXTOFF]._mode &= ~OBJMODE_OBJSTATUS;
		_obj[o00TEXTON]._mode |= OBJMODE_OBJSTATUS;
		TextON = true;
		_curObj = o00TEXTON;
		RegenRoom();
		ShowObjName(_curObj, true);
		break;

	case o00SPEECH1D:
	case o00SPEECH2D:
	case o00SPEECH3D:
	case o00SPEECH4D:
	case o00SPEECH5D:
	case o00SPEECH6D:
	case o00MUSIC1D:
	case o00MUSIC2D:
	case o00MUSIC3D:
	case o00MUSIC4D:
	case o00MUSIC5D:
	case o00MUSIC6D:
	case o00SOUND1D:
	case o00SOUND2D:
	case o00SOUND3D:
	case o00SOUND4D:
	case o00SOUND5D:
	case o00SOUND6D:
		_obj[TheObj]._mode &= ~OBJMODE_OBJSTATUS;
		if ((TheObj != o00SPEECH6D) && (TheObj != o00MUSIC6D) && (TheObj != o00SOUND6D))
			_obj[TheObj + 1]._mode &= ~OBJMODE_OBJSTATUS;
		_obj[TheObj - 1]._mode |= OBJMODE_OBJSTATUS;
		_obj[TheObj - 2]._mode |= OBJMODE_OBJSTATUS;
		RegenRoom();
		if (TheObj < o00MUSIC1D)
			SpeechVol = ((TheObj - 2 - o00SPEECH1D) / 2) * 25;
		else if (TheObj > o00MUSIC6D)
			SFxVol = ((TheObj - 2 - o00SOUND1D) / 2) * 25;
		else
			MusicVol = ((TheObj - 2 - o00MUSIC1D) / 2) * 25;
		break;

	case o00SPEECH1U:
	case o00SPEECH2U:
	case o00SPEECH3U:
	case o00SPEECH4U:
	case o00SPEECH5U:
	case o00MUSIC1U:
	case o00MUSIC2U:
	case o00MUSIC3U:
	case o00MUSIC4U:
	case o00MUSIC5U:
	case o00SOUND1U:
	case o00SOUND2U:
	case o00SOUND3U:
	case o00SOUND4U:
	case o00SOUND5U:
		_obj[TheObj]._mode &= ~OBJMODE_OBJSTATUS;
		_obj[TheObj - 1]._mode &= ~OBJMODE_OBJSTATUS;
		_obj[TheObj + 1]._mode |= OBJMODE_OBJSTATUS;
		if ((TheObj != o00SPEECH5U) && (TheObj != o00MUSIC5U) && (TheObj != o00SOUND5U))
			_obj[TheObj + 2]._mode |= OBJMODE_OBJSTATUS;
		RegenRoom();
		if (TheObj < o00MUSIC1D)
			SpeechVol = ((TheObj + 1 - o00SPEECH1D) / 2) * 25;
		else if (TheObj > o00MUSIC6D)
			SFxVol = ((TheObj + 1 - o00SOUND1D) / 2) * 25;
		else
			MusicVol = ((TheObj + 1 - o00MUSIC1D) / 2) * 25;
		break;
	}
}

/* -----------------09/02/98 15.44-------------------
 * 					SetRoom
 * --------------------------------------------------*/
void SetRoom(unsigned short r, bool b) {
	switch (r) {
	case r21:
		if (!b) {
			read3D("21.3d");
			Room[r21]._flag &= ~OBJFLAG_EXTRA;
			setPosition(14);
			_obj[oCATENAT21]._position = 5;
			_obj[oUSCITA21]._position = 11;

			// se so come andare di la'
			if (((IconPos(iSBARRA21) != MAXICON) && ((_choice[436]._flag & OBJFLAG_DONE) || (_choice[466]._flag & OBJFLAG_DONE)))
					|| ((_choice[451]._flag & OBJFLAG_DONE) || (_choice[481]._flag & OBJFLAG_DONE))) {
				_obj[od21ALLA23]._flag |= OBJFLAG_ROOMOUT;
				_obj[od21ALLA23]._flag &= ~OBJFLAG_EXAMINE;
			} else {
				_obj[od21ALLA23]._flag &= ~OBJFLAG_ROOMOUT;
				_obj[od21ALLA23]._flag |= OBJFLAG_EXAMINE;
			}
			_obj[od21ALLA23]._anim = 0;
			_obj[oUSCITA21]._mode |= OBJMODE_OBJSTATUS;

			_obj[od21ALLA22]._flag |= OBJFLAG_ROOMOUT;
			_obj[od21ALLA22]._flag &= ~OBJFLAG_EXAMINE;
			_obj[od21ALLA22]._anim = aWALKOUT;
			_obj[oPORTAA21]._anim = a212;
			_obj[oPORTAC21]._anim = a219;

			_obj[oCUNICOLO21]._mode |= OBJMODE_OBJSTATUS;
			_obj[oCARTELLONE21]._mode |= OBJMODE_OBJSTATUS;
		} else {
			read3D("212.3d");
			Room[r21]._flag |= OBJFLAG_EXTRA;
			setPosition(15);
			_obj[oCATENAT21]._position = 6;
			_obj[oUSCITA21]._position = 21;

			_obj[od21ALLA23]._flag |= OBJFLAG_ROOMOUT;
			_obj[od21ALLA23]._flag &= ~OBJFLAG_EXAMINE;
			_obj[od21ALLA23]._anim = aWALKOUT;
			_obj[oUSCITA21]._mode |= OBJMODE_OBJSTATUS;

			// se so come andare di la'
			if (((IconPos(iSBARRA21) != MAXICON) && ((_choice[436]._flag & OBJFLAG_DONE) || (_choice[466]._flag & OBJFLAG_DONE)))
					|| ((_choice[451]._flag & OBJFLAG_DONE) || (_choice[481]._flag & OBJFLAG_DONE))) {
				_obj[od21ALLA22]._flag |= OBJFLAG_ROOMOUT;
				_obj[od21ALLA22]._flag &= ~OBJFLAG_EXAMINE;
			} else {
				_obj[od21ALLA22]._flag &= ~OBJFLAG_ROOMOUT;
				_obj[od21ALLA22]._flag |= OBJFLAG_EXAMINE;
			}
			_obj[od21ALLA22]._anim = 0;
			_obj[od21ALLA22]._examine = 335;
			_obj[od21ALLA22]._action = 335;
			_obj[oPORTAA21]._anim = 0;
			_obj[oPORTAC21]._anim = 0;

			_obj[oCUNICOLO21]._mode &= ~OBJMODE_OBJSTATUS;
			_obj[oCARTELLONE21]._mode &= ~OBJMODE_OBJSTATUS;
		}
		break;
	case r24:
		if (!b) {
			read3D("24.3d");
			Room[r24]._flag &= ~OBJFLAG_EXTRA;
			_obj[oPASSAGGIO24]._position = 3;
			_obj[oMACERIE24]._position = 3;
			_obj[oDUMMY24]._mode &= ~OBJMODE_OBJSTATUS;
			_obj[oDUMMY24A]._mode |= OBJMODE_OBJSTATUS;
		} else {
			read3D("242.3d");
			Room[r24]._flag |= OBJFLAG_EXTRA;
			_obj[od24ALLA26]._mode |= OBJMODE_OBJSTATUS;
			_obj[oPASSAGGIO24]._position = 4;
			_obj[oMACERIE24]._position = 4;
			_obj[oDUMMY24A]._mode &= ~OBJMODE_OBJSTATUS;
			_obj[oDUMMY24]._mode |= OBJMODE_OBJSTATUS;
		}
		break;

	case r2A:
		if (!b) {
			read3D("2A.3d");
			Room[r2A]._flag &= ~OBJFLAG_EXTRA;
			_obj[oDUMMY2A2]._mode |= OBJMODE_OBJSTATUS;
			_obj[oDUMMY2A]._mode &= ~OBJMODE_OBJSTATUS;
		} else {
			read3D("2A2.3d");
			Room[r2A]._flag |= OBJFLAG_EXTRA;
			_obj[oDUMMY2A]._mode |= OBJMODE_OBJSTATUS;
			_obj[oDUMMY2A2]._mode &= ~OBJMODE_OBJSTATUS;
		}
		break;
	case r2B:
		if (!b) {
			read3D("2B.3d");
			Room[r2B]._flag &= ~OBJFLAG_EXTRA;
			_obj[oPORTA2B]._mode |= OBJMODE_OBJSTATUS;
			_obj[od2BALLA28]._mode &= ~OBJMODE_OBJSTATUS;
		} else {
			read3D("2B2.3d");
			Room[r2B]._flag |= OBJFLAG_EXTRA;
			_obj[oPORTA2B]._mode &= ~OBJMODE_OBJSTATUS;
			_obj[od2BALLA28]._mode |= OBJMODE_OBJSTATUS;
		}
		break;
	case r2E:
		if (!b) {
			_obj[oCATWALKA2E]._nbox = BACKGROUND;
			_obj[oCATWALKA2E]._position = 2;
			_obj[oCATWALKA2E]._anim = a2E2PRIMAPALLONTANANDO;
			read3D("2E.3d");
			Room[r2E]._flag &= ~OBJFLAG_EXTRA;
			_obj[oDUMMY2E]._mode &= ~OBJMODE_OBJSTATUS;
			_obj[oENTRANCE2E]._flag &= ~OBJFLAG_EXAMINE;
			_obj[oPASSERELLAB2E]._mode &= ~OBJMODE_OBJSTATUS;
			_obj[oCRATERE2E]._mode &= ~OBJMODE_OBJSTATUS;
			_obj[oARBUSTI2E]._mode &= ~OBJMODE_OBJSTATUS;
			_obj[oCREPACCIO2E]._position = 6;
		} else {
			_obj[oCATWALKA2E]._position = 3;
			_obj[oCATWALKA2E]._anim = a2E3PRIMAPAVVICINANDO;
			read3D("2E2.3d");
			Room[r2E]._flag |= OBJFLAG_EXTRA;
			_obj[oDUMMY2E]._mode |= OBJMODE_OBJSTATUS;
			_obj[oENTRANCE2E]._mode |= OBJMODE_OBJSTATUS;
			_obj[oENTRANCE2E]._flag |= OBJFLAG_EXAMINE;
			_obj[oPASSERELLAB2E]._mode |= OBJMODE_OBJSTATUS;
			_obj[oCRATERE2E]._mode |= OBJMODE_OBJSTATUS;
			_obj[oARBUSTI2E]._mode |= OBJMODE_OBJSTATUS;
			_obj[oCREPACCIO2E]._position = 7;
			_obj[oCATWALKA2E]._nbox = FOREGROUND;
		}
		break;
	case r2GV:
		if (!b) {
			_obj[oVIADOTTO2GV]._position = 7;
			_obj[oVIADOTTO2GV]._anim = a2G7ATTRAVERSAPONTICELLO;
			read3D("2GV.3d");
			Room[r2GV]._flag &= ~OBJFLAG_EXTRA;
			_obj[oDUMMY2GV]._mode &= ~OBJMODE_OBJSTATUS;
			_obj[oRAGAZZOS2GV]._mode &= ~OBJMODE_OBJSTATUS;
			_obj[oCOCCODRILLO2GV]._mode &= ~OBJMODE_OBJSTATUS;
		}
		break;
	default:
		break;
	}

	RegenRoom();
}

} // End of namespace Trecision
