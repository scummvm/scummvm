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
#include "trecision/nl/struct.h"
#include "trecision/nl/ll/llinc.h"
#include "trecision/nl/message.h"
#include "trecision/nl/extern.h"
#include "trecision/nl/define.h"
#include "trecision/trecision.h"
#include "trecision/graphics.h"
#include "trecision/video.h"

namespace Trecision {

uint16 DispChoice[MAXDISPCHOICES], CurDispChoice;
int16 CurPos, LastPos;

void DialogPrint(int x, int y, int c, const char *txt) {
	SDText curChoice;
	curChoice.set(x, y, TextLength(txt, 0), 0, 0, 0, MAXX, MAXY, c, MASKCOL, txt);
	curChoice.DText();
}

void ShowChoices(uint16 i) {
	Dialog *d = &_dialog[i];

	int y = 5;
	int x = 10;
	CurPos = -1;
	LastPos = -1;
	memset(g_vm->_screenBuffer, 0, MAXX * TOP * 2);

	for (int c = 0; c < MAXDISPCHOICES; c++)
		DispChoice[c] = 0;

	CurDispChoice = 0;
	for (int c = d->_firstChoice; c < (d->_firstChoice + d->_choiceNumb); c++) {
		if (!(g_vm->_choice[c]._flag & DLGCHOICE_HIDE)) {
			DispChoice[CurDispChoice++] = c;
			DialogPrint(x, y, HWHITE, g_vm->_sentence[g_vm->_choice[c]._sentenceIndex]);
			y += CARHEI;
		}
	}

	g_vm->_graphicsMgr->copyToScreen(0, 0, MAXX, TOP);

	g_vm->_flagDialogMenuActive = true;
	g_vm->_flagMouseEnabled = true;
}

void UpdateChoices(int16 dmx, int16 dmy) {
	if ((dmy >= MAXDISPCHOICES) && (dmy < (CARHEI * (CurDispChoice) + 5)))
		CurPos = (dmy - 5) / CARHEI;
	else
		CurPos = -1;

	if ((CurPos != LastPos) && ((CurPos != -1) || (LastPos != -1))) {
		for (int c = 0; c < MAXDISPCHOICES; c++) {
			if (DispChoice[c] != 0) {
				if (c == CurPos)
					DialogPrint(10, 5 + c * CARHEI, HGREEN, g_vm->_sentence[g_vm->_choice[DispChoice[c]]._sentenceIndex]);
				else
					DialogPrint(10, 5 + c * CARHEI, HWHITE, g_vm->_sentence[g_vm->_choice[DispChoice[c]]._sentenceIndex]);
			}
		}
		g_vm->_graphicsMgr->copyToScreen(0, 5, MAXX, (CurDispChoice)*CARHEI + 5);
	}
	LastPos = CurPos;
}

void SelectChoice(int16 dmx, int16 dmy) {
	UpdateChoices(dmx, dmy);

	if (CurPos != -1) {
		g_vm->_flagDialogMenuActive = false;
		PlayChoice(DispChoice[CurPos]);
	}
}

void PlayDialog(uint16 i) {
	_curDialog = i;
	g_vm->_flagDialogActive = true;
	_curChoice = 0;
	_curSubTitle = 0;
	g_vm->_flagShowCharacter = false;

	g_vm->_characterQueue.initQueue();
	g_vm->_inventoryStatus = INV_OFF;
	g_vm->_inventoryCounter = INVENTORY_HIDE;
	g_vm->clearText();
	g_vm->drawString();
	PaintScreen(1);

	memset(g_vm->_screenBuffer, 0, MAXX * TOP * 2);
	g_vm->_graphicsMgr->copyToScreen(0, 0, MAXX, TOP);

	g_vm->_animMgr->startFullMotion((const char *)_dialog[i]._startAnim);

	int skip = 0;
	int curChoice = 0;
	for (int c = _dialog[_curDialog]._firstChoice; c < (_dialog[_curDialog]._firstChoice + _dialog[_curDialog]._choiceNumb); c++)
		if (!(g_vm->_choice[c]._flag & DLGCHOICE_HIDE))
			curChoice++;

	if ((_curDialog == dC581) && !(g_vm->_choice[262]._flag & DLGCHOICE_HIDE))
		skip++;
	if ((_curDialog == dC581) && (curChoice == 1))
		skip++;
	if ((_curDialog == dSHOPKEEPER1A) && (curChoice == 1))
		skip++;
	// if there's a pre-dialog
	if ((_dialog[i]._startLen > 0) && !skip)
#if USE_NEW_VIDEO_CODE
		g_vm->_animMgr->playMovie(_dialog[i]._startAnim, 1, _dialog[i]._startLen);
#else
		g_vm->_animMgr->playFullMotion(1, _dialog[i]._startLen);
#endif	
	else {
		g_vm->_animMgr->smkSoundOnOff(1, false);
		afterChoice(1);
	}
}

void afterChoice(int numframe) {
	Dialog *d = &_dialog[_curDialog];

	memset(g_vm->_screenBuffer, 0, MAXX * TOP * 2);
	g_vm->_graphicsMgr->copyToScreen(0, 0, MAXX, TOP);

	switch (_curDialog) {
	case dTRAMP171:
		if (_curChoice == 80) {
			g_vm->_obj[ocTRAMP17]._action = 213;
			g_vm->_obj[ocTRAMP17]._flag &= ~OBJFLAG_PERSON;
		} else if (_curChoice == 77) {
			g_vm->_obj[ocTRAMP17]._action = 211;
			g_vm->_obj[ocTRAMP17]._flag &= ~OBJFLAG_PERSON;
		}
		break;

	case dTRAMP1714:
		if (_curChoice == 106) {
			g_vm->_obj[ocTRAMP17]._action = 213;
			g_vm->_obj[ocTRAMP17]._flag &= ~OBJFLAG_PERSON;
		}
		break;

	case dTRAMP1713:
		if (_curChoice == 91) {
			g_vm->_obj[ocTRAMP17]._action = 212;
			g_vm->_obj[ocTRAMP17]._flag &= ~OBJFLAG_PERSON;
		}
		break;

	case dTRAMP1716:
		if (_curChoice == 122) {
			g_vm->_obj[ocTRAMP17]._action = 212;
			g_vm->_obj[ocTRAMP17]._flag &= ~OBJFLAG_PERSON;
		}
		break;

	case dTRAMP1717:
		if (_curChoice == 136) {
			g_vm->_obj[ocTRAMP17]._mode &= ~OBJMODE_OBJSTATUS;
			g_vm->_obj[oTRAMPD17]._mode |= OBJMODE_OBJSTATUS;
			g_vm->_room[r17]._bkgAnim = aBKG17B;
			g_vm->addIcon(iSKATE);
		} else if (_curChoice == 137) {
			g_vm->_obj[ocTRAMP17]._flag |= OBJFLAG_PERSON;
		}
		break;

	case dGUARDIAN18:
		if ((_curChoice == 151) || (_curChoice == 152)) {
			g_vm->_inventoryObj[iFOTO]._action = 1465;
			g_vm->_obj[oTESSERA1A]._action = 238;
			if (g_vm->_obj[oTESSERA1A]._flag & OBJFLAG_EXTRA) {
				g_vm->_choice[154]._flag &= ~DLGCHOICE_HIDE;
				g_vm->_choice[153]._flag |= DLGCHOICE_HIDE;
			} else
				g_vm->_choice[153]._flag &= ~DLGCHOICE_HIDE;
		} else if (_curChoice == 154) {
			if (g_vm->_obj[oTESSERA1A]._flag & OBJFLAG_EXTRA)
				g_vm->_choice[183]._flag &= ~DLGCHOICE_HIDE;
		} else if (_curChoice == 155)
			g_vm->_obj[ocGUARD18]._action = 228;
		break;
	case dF213B:
	case dF213:
		g_vm->setRoom(r21, true);
		break;

	case dF212B:
	case dF212:
		g_vm->setRoom(r21, false);
		break;

	case dF321:
		g_vm->removeIcon(iTORCIA32);
		g_vm->_flagShowCharacter = false;
		break;

	case dF4A3:
		g_vm->_obj[oCHOCOLATES4A]._examine = 1105;
		g_vm->_obj[oCHOCOLATES4A]._action = 1106;
		g_vm->_obj[oPORTAC4A]._action = 1118;
		g_vm->_animMgr->_animTab[aBKG4A]._flag |= SMKANIM_OFF1;
		g_vm->_obj[ocHELLEN4A]._mode &= ~OBJMODE_OBJSTATUS;
		g_vm->_obj[oHELLENA4A]._mode |= OBJMODE_OBJSTATUS;
		break;

	case dC581:
		g_vm->_obj[oFINESTRAB58]._mode |= OBJMODE_OBJSTATUS;
		if (_curChoice == 262)
			g_vm->_obj[oTASTIERA56]._examine = 1307;
		break;

	case dF542:
		g_vm->_obj[oGRATAC54]._mode &= ~OBJMODE_OBJSTATUS;
		g_vm->_obj[oPORTAC54]._mode &= ~OBJMODE_OBJSTATUS;
		g_vm->_obj[oLAVATRICEL54]._mode &= ~OBJMODE_OBJSTATUS;
		g_vm->_obj[oLAVATRICEF54]._mode &= ~OBJMODE_OBJSTATUS;
		g_vm->_obj[oGRATAA54]._mode |= OBJMODE_OBJSTATUS;
		g_vm->_obj[oCHIAVI54]._mode |= OBJMODE_OBJSTATUS;
		g_vm->_obj[od54ALLA55]._mode |= OBJMODE_OBJSTATUS;
		break;
	}
	// If the player chose to exit the dialog
	if (g_vm->_choice[_curChoice]._flag & DLGCHOICE_EXITDLG) {
		g_vm->_animMgr->stopFullMotion();

		switch (_curDialog) {
		case dPOLIZIOTTO16:
			if ((g_vm->_choice[61]._flag & OBJFLAG_DONE) && (g_vm->_choice[62]._flag & OBJFLAG_DONE) && (g_vm->_obj[ocPOLIZIOTTO16]._flag & OBJFLAG_EXTRA))
				g_vm->_obj[ocPOLIZIOTTO16]._mode &= ~OBJMODE_OBJSTATUS;
			break;

		case dTRAMP171:
			if (_curChoice == 77) {
				g_vm->_obj[ocTRAMP17]._action = 211;
				g_vm->_obj[ocTRAMP17]._flag &= ~OBJFLAG_PERSON;
			} else if (_curChoice == 80)
				g_vm->_obj[ocTRAMP17]._action = 213;
			else if (_curChoice == 122)
				g_vm->_obj[ocTRAMP17]._action = 211;
			break;

		case dGUARDIAN18:
			if (_curChoice == 152)
				g_vm->_obj[ocGUARD18]._mode &= ~OBJMODE_OBJSTATUS;
			else if (_curChoice == 155)
				StartCharacterAction(a184ENTRACLUB, r19, 2, 0);
			break;

		case dEVA19:
			g_vm->_obj[oPORTAC18]._flag &= ~OBJFLAG_ROOMOUT;
			g_vm->_obj[oPORTAC18]._action = 221;
			g_vm->_obj[ocEVA19]._action = 1999;
			g_vm->_obj[ocEVA19]._flag &= ~OBJFLAG_PERSON;
			break;

		case dSHOPKEEPER1A:
			if (_curChoice == 185) {
				doEvent(MC_SYSTEM, ME_CHANGEROOM, MP_SYSTEM, r18, a1810ENTRADALNEGOZIO, 10, g_vm->_curObj);
				g_vm->_obj[oPORTAN18]._flag &= ~OBJFLAG_ROOMOUT;
				g_vm->_obj[oPORTAN18]._action = 218;
				g_vm->_obj[oPORTAN18]._anim = 0;
			} else if (_curChoice == 183)
				g_vm->_obj[oTESSERA1A]._action = 239;
			break;

		case dF181:
			g_vm->_obj[oRETE17]._mode |= OBJMODE_OBJSTATUS;
			g_vm->_obj[oPORTAA17]._examine = 196;
			g_vm->_obj[oPORTAUA17]._examine = 187;
			g_vm->_obj[oPORTAUB17]._examine = 192;
			g_vm->_obj[oPORTAA17]._action = 188;
			g_vm->_obj[oPORTAUA17]._action = 193;
			g_vm->_obj[oPORTAUB17]._action = 197;
			g_vm->_obj[oFINGERPAD17]._mode &= ~OBJMODE_OBJSTATUS;
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
			g_vm->_obj[oCATWALKA2E]._action = 622;
			break;

		case dF2E2:
			CharacterSay(1561);
			g_vm->_inventoryObj[iRIFLE]._examine = 1562;
			break;

		case dF231:
			g_vm->_obj[od21TO23]._goRoom = r23B;
			g_vm->_obj[od24TO23]._goRoom = r23B;
			break;

		case dF291:
			g_vm->_obj[oSWITCH29]._action = 479;
			g_vm->_obj[od22TO29]._goRoom = r29L;
			g_vm->_obj[od2ATO29]._goRoom = r29L;
			g_vm->_obj[od22TO29]._mode &= ~OBJMODE_OBJSTATUS;
			g_vm->_obj[od22TO29I]._mode |= OBJMODE_OBJSTATUS;
			break;

		case dF2G1:
			g_vm->_obj[oPANELM2G]._action = 660;
			CharacterSay(659);
			break;

		case dF2G2:
			g_vm->_obj[od26TO2G]._goRoom = r2GV;
			g_vm->replaceIcon(iSAM, iSAMROTTO);
			break;

		case dF321:
			StartCharacterAction(a3111TRASCINA, 0, 11, 0);
			break;

		case dF331:
			g_vm->_obj[oTUBET33]._lim = Common::Rect(0, 0, 0, 0);
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
			memcpy(g_vm->_cyberInventory, g_vm->_inventory, MAXICON);
			g_vm->_cyberInventorySize = g_vm->_inventorySize;
			g_vm->_inventorySize = 0;
			g_vm->_iconBase = 0;
			memset(g_vm->_inventory, iNULL, MAXICON);
			g_vm->_inventory[g_vm->_inventorySize++] = iDISLOCATORE;
			break;

		case dF371:
			g_vm->_obj[oSCAFFALE36]._anim = a3615AAPRENDESCAFFALE;
			g_vm->_animMgr->smkVolumePan(0, 1, 1);
			break;

		case dF431:
			g_vm->_flagShowCharacter = true;
			StartCharacterAction(aWALKIN, 0, 11, 0);
			break;

		case dF451:
			g_vm->_obj[od44ALLA45]._goRoom = r45S;
			CharacterSay(1712);
			break;

		case dF491:
			for (int c = oPULSANTE1AD; c <= oPULSANTE33AD; c++) {
				if (!(g_vm->_obj[c]._goRoom)) {
					g_vm->_obj[c]._goRoom = r4A;
					g_vm->_obj[c]._mode |= OBJMODE_OBJSTATUS;
					g_vm->_obj[c - 40]._mode &= ~OBJMODE_OBJSTATUS;
					break;
				}
			}
			g_vm->removeIcon(iROD);
			PlayDialog(dC4A1);
			setPosition(12);
			break;

		case dC4A1:
			g_vm->_flagCharacterExists = true;
			g_vm->_flagShowCharacter = true;
			actorStop();
			nextStep();
			break;

		case dF4C1:
			memcpy(g_vm->_inventory, g_vm->_cyberInventory, MAXICON);
			g_vm->_inventorySize = g_vm->_cyberInventorySize;
			g_vm->_iconBase = 0;
			g_vm->removeIcon(iCARD03);
			g_vm->removeIcon(iPEN);
			g_vm->removeIcon(iKEY05);
			g_vm->removeIcon(iLETTER12);
			g_vm->removeIcon(iCARD14);
			g_vm->removeIcon(iFOTO);
			g_vm->removeIcon(iPISTOLA15);
			g_vm->removeIcon(iRAPPORTO);
			g_vm->removeIcon(iTESSERA);
			g_vm->removeIcon(iRIFLE);
			g_vm->removeIcon(iLAMPADINA29);
			g_vm->removeIcon(iTELECOMANDO2G);
			g_vm->removeIcon(iCARD36);
			g_vm->removeIcon(iFOGLIO36);
			g_vm->removeIcon(iSTETOSCOPIO);
			g_vm->removeIcon(iREFERTO);
			g_vm->removeIcon(iLIBRO);
			g_vm->addIcon(iMAPPA50);
			g_vm->addIcon(iCUTTER);
			g_vm->addIcon(iWALKY);
			StartCharacterAction(a511, 0, 1, 0);
			break;

		case dF4P1:
			CharacterSay(1707);
			break;

		case dF4P2:
			CharacterSay(1708);
			break;

		case dF562:
			g_vm->_obj[oPORTA58C55]._action = 1287;
			g_vm->_obj[oPORTA58C55]._anim = 0;
			g_vm->_obj[oFINESTRA58P55]._action = 1292;
			g_vm->_obj[oFINESTRA58P55]._mode |= OBJMODE_OBJSTATUS;
			g_vm->_obj[oFINESTRA58P55]._anim = 0;
			break;

		case dF5A1:
			g_vm->_obj[oPORTA58C55]._action = 1286;
			g_vm->_obj[oFINESTRA58P55]._action = 1291;
			g_vm->_obj[oFINESTRAA5A]._action = 1403;
			g_vm->_obj[oGUARDIA58]._mode |= OBJMODE_OBJSTATUS;
			g_vm->_choice[286]._flag |= OBJFLAG_DONE;
			break;

		case dC581:
			if (!(g_vm->_choice[886]._flag & OBJFLAG_DONE) && (g_vm->_choice[258]._flag & OBJFLAG_DONE)) {
				setPosition(1);
				PlayDialog(dF581);
			}
			break;

		case dC582:
			g_vm->_obj[oFINESTRAA58]._mode |= OBJMODE_OBJSTATUS;
			g_vm->addIcon(iCODICE);
			break;

		case dC5A1:
			g_vm->_obj[oFINESTRAA5A]._action = 1402;
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
	if (g_vm->_choice[_curChoice]._nextDialog != 0) {
		_curDialog = g_vm->_choice[_curChoice]._nextDialog;
		g_vm->_flagDialogActive = true;
		_curChoice = 0;

		d = &_dialog[_curDialog];

#if (!USE_NEW_VIDEO_CODE)
		g_vm->_animMgr->_fullMotionStart = 0;
		g_vm->_animMgr->_fullMotionEnd = 0;
#endif
		
		// se c'e' predialog
		if (_dialog[_curDialog]._startLen > 0) {
#if USE_NEW_VIDEO_CODE
			g_vm->_animMgr->playMovie(_dialog[_curDialog]._startAnim, 1, _dialog[_curDialog]._startLen);
#else
			g_vm->_animMgr->playFullMotion(1, _dialog[_curDialog]._startLen);
#endif
			return;
		}
	}

	// fa partire subito tutte le prevarica
	for (int c = d->_firstChoice; c < (d->_firstChoice + d->_choiceNumb); c++) {
		if ((g_vm->_choice[c]._flag & DLGCHOICE_FRAUD) && (!(g_vm->_choice[c]._flag & DLGCHOICE_HIDE))) {
			PlayChoice(c);
			return;
		}
	}

	// If there's only one option, show it immediately, otherwise show available choices
	int res = 0;
	for (int c = d->_firstChoice; c < (d->_firstChoice + d->_choiceNumb); c++) {
		if (!(g_vm->_choice[c]._flag & DLGCHOICE_HIDE)) {
			if (g_vm->_choice[c]._flag & DLGCHOICE_EXITNOW) {
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
		PlayChoice(res);
		return;
	}

	// If no option is visible, close the dialog
	res = 0;
	for (int c = d->_firstChoice; c < (d->_firstChoice + d->_choiceNumb); c++) {
		if (!(g_vm->_choice[c]._flag & DLGCHOICE_HIDE))
			res++;
	}

	if (res == 0) {
		g_vm->_animMgr->stopFullMotion();
		if (_curDialog == dC381)
			PlayDialog(dF381);
		return;
	}

	ShowChoices(_curDialog);
}

void DialogHandler(int numframe) {
	if (g_vm->_flagDialogActive && !g_vm->_flagDialogMenuActive) {
		g_vm->_flagMouseEnabled = false;
		if (numframe == _subTitles[_curSubTitle]._startFrame) {
			int i = _curSubTitle++;
#if (!USE_NEW_VIDEO_CODE)
			g_vm->_oldSdText.set(g_vm->_sdText);
#endif
			g_vm->_sdText.x = _subTitles[i]._x;
			g_vm->_sdText.y = _subTitles[i]._y;
			g_vm->_sdText.tcol = _subTitles[i]._color;
			g_vm->_sdText.text = g_vm->_sentence[_subTitles[i]._sentence];
		}
	}
}

void PlayChoice(uint16 i) {
	DialogChoice *ss = &g_vm->_choice[i];

	memset(g_vm->_screenBuffer, 0, MAXX * TOP * 2);
	g_vm->_graphicsMgr->copyToScreen(0, 0, MAXX, TOP);

	_curChoice = i;
	_curSubTitle = ss->_firstSubTitle;
	g_vm->_flagDialogMenuActive = false;

	ss->_flag |= OBJFLAG_DONE;

	// se era one time la disabilita
	if (ss->_flag & DLGCHOICE_ONETIME)
		ss->_flag |= DLGCHOICE_HIDE;

	// Disable other choices
	for (int c = 0; c < MAXDISPCHOICES; c++) {
		g_vm->_choice[ss->_off[c]]._flag |= DLGCHOICE_HIDE;
		g_vm->_choice[ss->_on[c]]._flag &= ~DLGCHOICE_HIDE;
	}

	int totalLength = 0;
	int subtitleCount = ss->_firstSubTitle + ss->_subTitleNumb;
	for (int c = _curSubTitle; c < subtitleCount; c++)
		totalLength += _subTitles[c]._length;

	g_vm->_flagMouseEnabled = false;
#if USE_NEW_VIDEO_CODE
	g_vm->_animMgr->playMovie(_dialog[_curDialog]._startAnim, ss->_startFrame, ss->_startFrame + totalLength - 1);
#else
	g_vm->_animMgr->playFullMotion(ss->_startFrame, ss->_startFrame + totalLength - 1);
#endif
	
}

void doDialog() {
	switch (g_vm->_curMessage->_event) {
	case ME_ENDCHOICE:
		afterChoice(g_vm->_curMessage->_u16Param1);
		break;

	case ME_STARTDIALOG:
		PlayDialog(g_vm->_curMessage->_u16Param1);
		break;
	}
}

} // End of namespace Trecision
