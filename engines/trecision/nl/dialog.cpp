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
#include "trecision/nl/lib/addtype.h"
#include "trecision/nl/sysdef.h"
#include "trecision/nl/struct.h"
#include "trecision/nl/ll/llinc.h"
#include "trecision/nl/message.h"
#include "trecision/nl/extern.h"
#include "trecision/nl/define.h"
#include "trecision/trecision.h"

namespace Trecision {

int EndBattuta = 0;
uint16 DispScelte[MAXDISPSCELTE], CurDispScelte;
int16 CurPos, LastPos;

/* -----------------28/07/97 22.11-------------------
					DialogPrint
--------------------------------------------------*/
void DialogPrint(int x, int y, int c, const char *txt) {
	struct SDText theChoice;

	theChoice.x = x;
	theChoice.y = y;
	theChoice.dx = TextLength(txt, 0);
	theChoice.sign = txt;
	theChoice.l[0] = 0;
	theChoice.l[1] = 0;
	theChoice.l[2] = MAXX;
	theChoice.l[3] = MAXY;
	theChoice.tcol = c;
	theChoice.scol = MASKCOL;

	DText(theChoice);
}

/* -----------------28/07/97 22.11-------------------
					MostraScelte
--------------------------------------------------*/
void MostraScelte(uint16 i) {
	Dialog *d = &_dialog[i];

	int y = 5;
	int x = 10;
	CurPos = -1;
	LastPos = -1;
	wordset(Video2, 0, MAXX * TOP);

	for (int c = 0; c < MAXDISPSCELTE; c++)
		DispScelte[c] = 0;

	CurDispScelte = 0;
	for (int c = d->_firstChoice; c < (d->_firstChoice + d->_choiceNumb); c++) {
		if (!(_choice[c]._flag & DLGCHOICE_HIDE)) {
			DispScelte[CurDispScelte++] = c;
			DialogPrint(x, y, HWHITE, Sentence[_choice[c]._sentenceIndex]);
			y += CARHEI;
		}
	}

	ShowScreen(0, 0, MAXX, TOP);

	SemDialogMenuActive = true;
	SemMouseEnabled = true;
}

/* -----------------28/07/97 22.12-------------------
					UpdateScelte
--------------------------------------------------*/
void UpdateScelte(int16 dmx, int16 dmy) {
	if ((dmy >= MAXDISPSCELTE) && (dmy < (CARHEI * (CurDispScelte) + 5)))
		CurPos = (dmy - 5) / CARHEI;
	else
		CurPos = -1;

	if ((CurPos != LastPos) && ((CurPos != -1) || (LastPos != -1))) {
		for (int c = 0; c < MAXDISPSCELTE; c++) {
			if (DispScelte[c] != 0) {
				if (c == CurPos)
					DialogPrint(10, 5 + c * CARHEI, HGREEN, Sentence[_choice[DispScelte[c]]._sentenceIndex]);
				else
					DialogPrint(10, 5 + c * CARHEI, HWHITE, Sentence[_choice[DispScelte[c]]._sentenceIndex]);
			}
		}
		ShowScreen(0, 5, MAXX, (CurDispScelte)*CARHEI + 5);
	}
	LastPos = CurPos;
}

/* -----------------28/07/97 22.12-------------------
					ScegliScelta
--------------------------------------------------*/
void ScegliScelta(int16 dmx, int16 dmy) {
	UpdateScelte(dmx, dmy);

	if (CurPos != -1) {
		SemDialogMenuActive = false;

		PlayScelta(DispScelte[CurPos]);
	}
}

/* -----------------28/07/97 22.08-------------------
					PlayDialog
--------------------------------------------------*/
void PlayDialog(uint16 i) {
	_curDialog = i;
	SemDialogActive = true;
	_curChoice = 0;
	_curSubTitle = 0;
	SemShowHomo = false;		        // prima non c'era

	InitQueue(&Homo);
	_inventoryStatus = INV_OFF;
	_inventoryCounter = INVENTORY_HIDE;
	ClearText();
	PaintString();
	PaintScreen(1);

	wordset(Video2, 0, MAXX * TOP);
	ShowScreen(0, 0, MAXX, TOP);

//	sprintf( UStr, "%sFMV\\%s", GamePath, _dialog[i]._startAnim );
//	sprintf( UStr, "FMV\\%s", _dialog[i]._startAnim );
	StartFullMotion((const char *)_dialog[i]._startAnim);

	int skip = 0;
	int curChoice = 0;
	for (int c = _dialog[_curDialog]._firstChoice; c < (_dialog[_curDialog]._firstChoice + _dialog[_curDialog]._choiceNumb); c++)
		if (!(_choice[c]._flag & DLGCHOICE_HIDE))
			curChoice++;

	if ((_curDialog == dC581) && !(_choice[262]._flag & DLGCHOICE_HIDE))
		skip++;
	if ((_curDialog == dC581) && (curChoice == 1))
		skip++;
	if ((_curDialog == dNEGOZIANTE1A) && (curChoice == 1))
		skip++;
	// se c'e' predialog
	if ((_dialog[i]._startLen > 0) && !(skip))
		PlayFullMotion(1, _dialog[i]._startLen);
	else {
		CallSmackSoundOnOff(1, 0);
		afterChoice(1);
	}
}

/* -----------------28/07/97 22.08-------------------
					afterChoice
--------------------------------------------------*/
void afterChoice(int numframe) {
	Dialog *d = &_dialog[_curDialog];

	wordset(Video2, 0, MAXX * TOP);
	ShowScreen(0, 0, MAXX, TOP);

	switch (_curDialog) {
	case dBARBONE171:
		if (_curChoice == 80) {
			_obj[ocBARBONE17]._action = 213;
			_obj[ocBARBONE17]._flag &= ~OBJFLAG_PERSON;
		} else if (_curChoice == 77) {
			_obj[ocBARBONE17]._action = 211;
			_obj[ocBARBONE17]._flag &= ~OBJFLAG_PERSON;
		}
		break;

	case dBARBONE1714:
		if (_curChoice == 106) {
			_obj[ocBARBONE17]._action = 213;
			_obj[ocBARBONE17]._flag &= ~OBJFLAG_PERSON;
		}
		break;

	case dBARBONE1713:
		if (_curChoice == 91) {
			_obj[ocBARBONE17]._action = 212;
			_obj[ocBARBONE17]._flag &= ~OBJFLAG_PERSON;
		}
		break;

	case dBARBONE1716:
		if (_curChoice == 122) {
			_obj[ocBARBONE17]._action = 212;
			_obj[ocBARBONE17]._flag &= ~OBJFLAG_PERSON;
		}
		break;

	case dBARBONE1717:
		if (_curChoice == 136) {
			_obj[ocBARBONE17]._mode &= ~OBJMODE_OBJSTATUS;
			_obj[oBARBONED17]._mode |= OBJMODE_OBJSTATUS;
			g_vm->_room[r17]._bkgAnim = aBKG17B;
			AddIcon(iPATTINO);
		} else if (_curChoice == 137) {
			_obj[ocBARBONE17]._flag |= OBJFLAG_PERSON;
		}
		break;

	case dGUARDIANO18:
		if ((_curChoice == 151) || (_curChoice == 152)) {
			InvObj[iFOTO]._action = 1465;
			_obj[oTESSERA1A]._action = 238;
			if (_obj[oTESSERA1A]._flag & OBJFLAG_EXTRA) {
				_choice[154]._flag &= ~DLGCHOICE_HIDE;
				_choice[153]._flag |= DLGCHOICE_HIDE;
			} else
				_choice[153]._flag &= ~DLGCHOICE_HIDE;
		} else if (_curChoice == 154) {
			if (_obj[oTESSERA1A]._flag & OBJFLAG_EXTRA)
				_choice[183]._flag &= ~DLGCHOICE_HIDE;
		} else if (_curChoice == 155)
			_obj[ocGUARDIANO18]._action = 228;
		break;

	/*		case dSAM17:
				if( ( _curChoice == 198 ) && ( _curRoom == r17 ) )
					setPosition(4);
			break;
	*/
	case dF213B:
	case dF213:
		SetRoom(r21, true);
		break;

	case dF212B:
	case dF212:
		SetRoom(r21, false);
		break;

	case dF321:
		KillIcon(iTORCIA32);
		SemShowHomo = false;
		break;

	case dF4A3:
		_obj[oCIOCCOLATINI4A]._examine = 1105;
		_obj[oCIOCCOLATINI4A]._action = 1106;
		_obj[oPORTAC4A]._action = 1118;
		AnimTab[aBKG4A].flag |= SMKANIM_OFF1;
		_obj[ocHELLEN4A]._mode &= ~OBJMODE_OBJSTATUS;
		_obj[oHELLENA4A]._mode |= OBJMODE_OBJSTATUS;
		break;

	case dC581:
		_obj[oFINESTRAB58]._mode |= OBJMODE_OBJSTATUS;
		if (_curChoice == 262)
			_obj[oTASTIERA56]._examine = 1307;
		break;

	case dF542:
		_obj[oGRATAC54]._mode &= ~OBJMODE_OBJSTATUS;
		_obj[oPORTAC54]._mode &= ~OBJMODE_OBJSTATUS;
		_obj[oLAVATRICEL54]._mode &= ~OBJMODE_OBJSTATUS;
		_obj[oLAVATRICEF54]._mode &= ~OBJMODE_OBJSTATUS;
		_obj[oGRATAA54]._mode |= OBJMODE_OBJSTATUS;
		_obj[oCHIAVI54]._mode |= OBJMODE_OBJSTATUS;
		_obj[od54ALLA55]._mode |= OBJMODE_OBJSTATUS;
		break;
	}
	// Se ultima scelta era un esci dialogo
	if (_choice[_curChoice]._flag & DLGCHOICE_EXITDLG) {
		StopFullMotion();

		switch (_curDialog) {
		/*			case dASCENSORE12:
			if( _curChoice == 3 )
				StartCharacterAction(a129PARLACOMPUTERESCENDE,r13,20,0);
			else if( _curChoice == 4 )
				StartCharacterAction(a129PARLACOMPUTERESCENDE,r16,20,0);
		break;

		case dASCENSORE13:
			if( _curChoice == 17 )
				StartCharacterAction(a139CHIUDONOPORTESU,r12,20,0);
			else if( _curChoice == 18 )
				StartCharacterAction(a1316CHIUDONOPORTEGIU,r16,20,0);
		break;

		case dASCENSORE16:
			if( _curChoice == 32 )
				StartCharacterAction(a1616SALECONASCENSORE,r12,20,0);
			else if( _curChoice == 33 )
				StartCharacterAction(a1616SALECONASCENSORE,r13,20,0);
		break;
		*/
		case dPOLIZIOTTO16:
			if ((_choice[61]._flag & OBJFLAG_DONE) && (_choice[62]._flag & OBJFLAG_DONE) && (_obj[ocPOLIZIOTTO16]._flag & OBJFLAG_EXTRA))
				_obj[ocPOLIZIOTTO16]._mode &= ~OBJMODE_OBJSTATUS;
			break;

		case dBARBONE171:
			if (_curChoice == 77) {
				_obj[ocBARBONE17]._action = 211;
				_obj[ocBARBONE17]._flag &= ~OBJFLAG_PERSON;
			} else if (_curChoice == 80)
				_obj[ocBARBONE17]._action = 213;
			else if (_curChoice == 122)
				_obj[ocBARBONE17]._action = 211;
			break;

		case dGUARDIANO18:
			if (_curChoice == 152)
				_obj[ocGUARDIANO18]._mode &= ~OBJMODE_OBJSTATUS;
			else if (_curChoice == 155)
				StartCharacterAction(a184ENTRACLUB, r19, 2, 0);
			break;

		case dEVA19:
			_obj[oPORTAC18]._flag &= ~OBJFLAG_ROOMOUT;
			_obj[oPORTAC18]._action = 221;
			_obj[ocEVA19]._action = 1999;
			_obj[ocEVA19]._flag &= ~OBJFLAG_PERSON;
			break;

		case dNEGOZIANTE1A:
			if (_curChoice == 185) {
				doEvent(MC_SYSTEM, ME_CHANGEROOM, MP_SYSTEM, r18, a1810ENTRADALNEGOZIO, 10, g_vm->_curObj);
				_obj[oPORTAN18]._flag &= ~OBJFLAG_ROOMOUT;
				_obj[oPORTAN18]._action = 218;
				_obj[oPORTAN18]._anim = 0;
			} else if (_curChoice == 183)
				_obj[oTESSERA1A]._action = 239;
			break;

		case dF181:
			_obj[oRETE17]._mode |= OBJMODE_OBJSTATUS;
			_obj[oPORTAA17]._examine = 196;
			_obj[oPORTAUA17]._examine = 187;
			_obj[oPORTAUB17]._examine = 192;
			_obj[oPORTAA17]._action = 188;
			_obj[oPORTAUA17]._action = 193;
			_obj[oPORTAUB17]._action = 197;
			_obj[oFINGERPAD17]._mode &= ~OBJMODE_OBJSTATUS;
			g_vm->_room[r17]._flag &= ~OBJFLAG_DONE;
			g_vm->_room[r17]._flag |= OBJFLAG_EXTRA;
			break;

		case dF1C1:
			CharacterSay(334);
			break;

		case dF1D1:
			CharacterSay(1502);
			break;

		case dF2E1:
			CharacterSay(621);
			_obj[oCATWALKA2E]._action = 622;
			break;

		case dF2E2:
			CharacterSay(1561);
			InvObj[iFUCILE]._examine = 1562;
			break;

		case dF231:
			_obj[od21ALLA23]._goRoom = r23B;
			_obj[od24ALLA23]._goRoom = r23B;
			break;

		case dF291:
			_obj[oINTERRUTTORE29]._action = 479;
			_obj[od22ALLA29]._goRoom = r29L;
			_obj[od2AALLA29]._goRoom = r29L;
			_obj[od22ALLA29]._mode &= ~OBJMODE_OBJSTATUS;
			_obj[od22ALLA29I]._mode |= OBJMODE_OBJSTATUS;
			break;

		case dF2G1:
			_obj[oPANNELLOM2G]._action = 660;
			CharacterSay(659);
			break;

		case dF2G2:
			_obj[od26ALLA2G]._goRoom = r2GV;
			ReplaceIcon(iSAM, iSAMROTTO);
			break;

		case dF321:
			StartCharacterAction(a3111TRASCINA, 0, 11, 0);
			break;

		case dF331:
			_obj[oTUBOT33]._lim[0] = 0;
			_obj[oTUBOT33]._lim[1] = 0;
			_obj[oTUBOT33]._lim[2] = 0;
			_obj[oTUBOT33]._lim[3] = 0;
			CharacterSay(1997);
			break;

		case dF362:
			PlayDialog(dC381);
			break;

		case dC381:
			PlayDialog(dF381);
			break;

		case dF381:
			doEvent(MC_SYSTEM, ME_CHANGEROOM, MP_SYSTEM, r41, 0, 18, g_vm->_curObj);
			memcpy(_cyberInventory, _inventory, MAXICON);
			_cyberInventorySize = _inventorySize;
			_inventorySize = 0;
			TheIconBase = 0;
			memset(_inventory, iNULL, MAXICON);
			_inventory[_inventorySize++] = iDISLOCATORE;
			break;

		case dF371:
			_obj[oSCAFFALE36]._anim = a3615AAPRENDESCAFFALE;
			CallSmackVolumePan(0, 1, 1);
			break;

		case dF431:
			SemShowHomo = true;
			StartCharacterAction(aWALKIN, 0, 11, 0);
			break;

		case dF451:
			_obj[od44ALLA45]._goRoom = r45S;
			CharacterSay(1712);
			break;

		case dF491:
			for (int c = oPULSANTE1AD; c <= oPULSANTE33AD; c++) {
				if (!(_obj[c]._goRoom)) {
					_obj[c]._goRoom = r4A;
					_obj[c]._mode |= OBJMODE_OBJSTATUS;
					_obj[c - 40]._mode &= ~OBJMODE_OBJSTATUS;
					break;
				}
			}
			KillIcon(iASTA);
			PlayDialog(dC4A1);
			setPosition(12);
			break;

		case dC4A1:
			SemCharacterExist = true;
			SemShowHomo = true;
			actorStop();
			nextStep();
			break;

		case dF4C1:
			memcpy(_inventory, _cyberInventory, MAXICON);
			_inventorySize = _cyberInventorySize;
			TheIconBase = 0;
			KillIcon(iCARD03);
			KillIcon(iPEN);
			KillIcon(iKEY05);
			KillIcon(iLETTERA12);
			KillIcon(iCARD14);
			KillIcon(iFOTO);
			KillIcon(iPISTOLA15);
			KillIcon(iRAPPORTO);
			KillIcon(iTESSERA);
			KillIcon(iFUCILE);
			KillIcon(iLAMPADINA29);
			KillIcon(iTELECOMANDO2G);
			KillIcon(iCARD36);
			KillIcon(iFOGLIO36);
			KillIcon(iSTETOSCOPIO);
			KillIcon(iREFERTO);
			KillIcon(iLIBRO);
			AddIcon(iMAPPA50);
			AddIcon(iCUTTER);
			AddIcon(iWALKY);
			StartCharacterAction(a511, 0, 1, 0);
			break;

		case dF4P1:
			CharacterSay(1707);
			break;

		case dF4P2:
			CharacterSay(1708);
			break;

		case dF562:
			_obj[oPORTA58C55]._action = 1287;
			_obj[oPORTA58C55]._anim = 0;
			_obj[oFINESTRA58P55]._action = 1292;
			_obj[oFINESTRA58P55]._mode |= OBJMODE_OBJSTATUS;
			_obj[oFINESTRA58P55]._anim = 0;
			break;

		case dF5A1:
			_obj[oPORTA58C55]._action = 1286;
			_obj[oFINESTRA58P55]._action = 1291;
			_obj[oFINESTRAA5A]._action = 1403;
			_obj[oGUARDIA58]._mode |= OBJMODE_OBJSTATUS;
			_choice[286]._flag |= OBJFLAG_DONE;
			break;

		case dC581:
			if (!(_choice[886]._flag & OBJFLAG_DONE) && (_choice[258]._flag & OBJFLAG_DONE)) {
				setPosition(1);
				PlayDialog(dF581);
			}
			break;

		case dC582:
			_obj[oFINESTRAA58]._mode |= OBJMODE_OBJSTATUS;
			AddIcon(iCODICE);
			break;

		case dC5A1:
			_obj[oFINESTRAA5A]._action = 1402;
			if (g_vm->_room[r5A]._flag & OBJFLAG_EXTRA)
				PlayDialog(dF5A1);
			break;

		case dFLOG:
			PlayDialog(dINTRO);
			break;

		case dINTRO:
			doEvent(MC_SYSTEM, ME_CHANGEROOM, MP_SYSTEM, r11, 0, 20, g_vm->_curObj);
			break;

		case dF582:
			PlayDialog(dFCRED);
			break;

		case dFCRED:
			doEvent(MC_SYSTEM, ME_QUIT, MP_SYSTEM, 0, 0, 0, 0);
			break;
		}
		return;
	}

	// se parte altro dialogo
	if (_choice[_curChoice]._nextDialog != 0) {
		extern int FullStart, FullEnd;

		_curDialog = _choice[_curChoice]._nextDialog;
		SemDialogActive = true;
		_curChoice = 0;

		d = &_dialog[_curDialog];

		FullStart = 0;
		FullEnd = 0;

		// se c'e' predialog
		if (_dialog[_curDialog]._startLen > 0) {
			PlayFullMotion(1, _dialog[_curDialog]._startLen);
			return ;
		}
	}

	// fa partire subito tutte le prevarica
	for (int c = d->_firstChoice; c < (d->_firstChoice + d->_choiceNumb); c++) {
		if ((_choice[c]._flag & DLGCHOICE_FRAUD) && (!(_choice[c]._flag & DLGCHOICE_HIDE))) {
			PlayScelta(c);
			return;
		}
	}

	// se c'e' una sola partisubito la fa partire altrimenti mostra le scelte
	int res = 0;
	for (int c = d->_firstChoice; c < (d->_firstChoice + d->_choiceNumb); c++) {
		if (!(_choice[c]._flag & DLGCHOICE_HIDE)) {
			if (_choice[c]._flag & DLGCHOICE_EXITNOW) {
				if (res == 0)
					res = c;
				else {
					res = 0;
					break;
				}
			} else {
				res = 0;
				break;
			}
		}
	}
	if (res != 0) {
		PlayScelta(res);
		return;
	}

	// se sono tutte hidate esce dal dialogo
	res = 0;
	for (int c = d->_firstChoice; c < (d->_firstChoice + d->_choiceNumb); c++) {
		if (!(_choice[c]._flag & DLGCHOICE_HIDE))
			res++;
	}

	if (res == 0) {
		StopFullMotion();
		if (_curDialog == dC381)
			PlayDialog(dF381);
		return;
	}

	MostraScelte(_curDialog);
}

/* -----------------28/07/97 22.08-------------------
					DialogHandler
--------------------------------------------------*/
void DialogHandler(int numframe) {
	if ((SemDialogActive) && (!(SemDialogMenuActive))) {
		SemMouseEnabled = false;
		if (numframe == _subTitles[_curSubTitle]._startFrame) {
			int i = _curSubTitle++;
			BattutaPrint(_subTitles[i]._x, _subTitles[i]._y, _subTitles[i]._color, Sentence[_subTitles[i]._sentence]);
		}
	}
}

/* -----------------28/07/97 22.11-------------------
						PlayScelta
--------------------------------------------------*/
void PlayScelta(uint16 i) {
	DialogChoice *ss = &_choice[i];

	wordset(Video2, 0, MAXX * TOP);
	ShowScreen(0, 0, MAXX, TOP);

	_curChoice = i;
	_curSubTitle = ss->_firstSubTitle;
	EndBattuta = (ss->_firstSubTitle + ss->_subTitleNumb);
	SemDialogMenuActive = false;

	ss->_flag |= OBJFLAG_DONE;

	// se era one time la disabilita
	if (ss->_flag & DLGCHOICE_ONETIME)
		ss->_flag |= DLGCHOICE_HIDE;

	// disattiva le altre scelte
	for (int c = 0; c < MAXDISPSCELTE; c++)
		_choice[ss->_off[c]]._flag |= DLGCHOICE_HIDE;
	// attiva le altre scelte
	for (int c = 0; c < MAXDISPSCELTE; c++)
		_choice[ss->_on[c]]._flag &= ~DLGCHOICE_HIDE;

	int lens = 0;
	for (int c = _curSubTitle; c < EndBattuta; c++)
		lens += _subTitles[c]._length;

	SemMouseEnabled = false;
	PlayFullMotion(ss->_startFrame, ss->_startFrame + lens - 1);
}

/* -----------------28/07/97 22.15-------------------
						DoDialog
 --------------------------------------------------*/
void doDialog() {
	switch (TheMessage->event) {
	case ME_ENDSCELTA:
		afterChoice(TheMessage->wparam1);
		break;

	case ME_STARTDIALOG:
		PlayDialog(TheMessage->wparam1);
		break;
	}
}

} // End of namespace Trecision
