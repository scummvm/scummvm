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

#include "trecision/trecision.h"
#include "trecision/dialog.h"


#include "actor.h"
#include "trecision/nl/message.h"
#include "trecision/nl/proto.h"
#include "trecision/graphics.h"

#include "trecision/defines.h"

namespace Trecision {
DialogManager::DialogManager(TrecisionEngine *vm) : _vm(vm) {
	_curDialog = 0;
	_curChoice = 0;
	_curSubTitle = 0;
	_curDispChoice = 0;
	_curPos = -1;
	_lastPos = -1;
}

DialogManager::~DialogManager() {}

void DialogManager::dialogPrint(int x, int y, int c, const char *txt) {
	SDText curChoice;
	curChoice.set(
		Common::Rect(x, y, _vm->textLength(txt, 0) + x, y),
		Common::Rect(0, 0, MAXX, MAXY),
		c,
		MASKCOL,
		txt
	);
	curChoice.DText();
}

void DialogManager::showChoices(uint16 i) {
	assert(i < MAXDIALOG);

	Dialog *dialog = &_dialog[i];

	int x = 10;
	int y = 5;
	_curPos = -1;
	_lastPos = -1;
	memset(_vm->_screenBuffer, 0, MAXX * TOP * 2);

	for (int c = 0; c < MAXDISPCHOICES; c++)
		_dispChoice[c] = 0;

	_curDispChoice = 0;
	for (int c = dialog->_firstChoice; c < (dialog->_firstChoice + dialog->_choiceNumb); c++) {
		if (!(_choice[c]._flag & DLGCHOICE_HIDE)) {
			_dispChoice[_curDispChoice++] = c;
			dialogPrint(x, y, HWHITE, _vm->_sentence[_choice[c]._sentenceIndex]);
			y += CARHEI;
		}
	}

	_vm->_graphicsMgr->copyToScreen(0, 0, MAXX, TOP);

	_vm->_flagDialogMenuActive = true;
	_vm->showCursor();
}

void DialogManager::updateChoices(int16 dmx, int16 dmy) {
	if ((dmy >= MAXDISPCHOICES) && (dmy < (CARHEI * (_curDispChoice)+5)))
		_curPos = (dmy - 5) / CARHEI;
	else
		_curPos = -1;

	if ((_curPos != _lastPos) && ((_curPos != -1) || (_lastPos != -1))) {
		for (int c = 0; c < MAXDISPCHOICES; c++) {
			if (_dispChoice[c] != 0) {
				if (c == _curPos)
					dialogPrint(10, 5 + c * CARHEI, HGREEN, _vm->_sentence[_choice[_dispChoice[c]]._sentenceIndex]);
				else
					dialogPrint(10, 5 + c * CARHEI, HWHITE, _vm->_sentence[_choice[_dispChoice[c]]._sentenceIndex]);
			}
		}
		_vm->_graphicsMgr->copyToScreen(0, 5, MAXX, (_curDispChoice)*CARHEI + 5);
	}
	_lastPos = _curPos;
}

void DialogManager::selectChoice(int16 dmx, int16 dmy) {
	updateChoices(dmx, dmy);

	if (_curPos != -1) {
		_vm->_flagDialogMenuActive = false;
		playChoice(_dispChoice[_curPos]);
	}
}

void DialogManager::playDialog(uint16 i) {
	_curDialog = i;
	_vm->_flagDialogActive = true;
	_curChoice = 0;
	_curSubTitle = 0;
	_vm->_flagShowCharacter = false;

	_vm->_characterQueue.initQueue();
	_vm->_inventoryStatus = INV_OFF;
	_vm->_inventoryCounter = INVENTORY_HIDE;
	_vm->clearText();
	_vm->drawString();
	PaintScreen(true);

	memset(_vm->_screenBuffer, 0, MAXX * TOP * 2);
	_vm->_graphicsMgr->copyToScreen(0, 0, MAXX, TOP);

	_vm->_animMgr->startFullMotion((const char *)_dialog[i]._startAnim);

	int skip = 0;
	int curChoice = 0;
	for (int c = _dialog[_curDialog]._firstChoice; c < (_dialog[_curDialog]._firstChoice + _dialog[_curDialog]._choiceNumb); ++c) {
		if (!(_choice[c]._flag & DLGCHOICE_HIDE))
			curChoice++;
	}

	if ((_curDialog == dC581) && !(_choice[262]._flag & DLGCHOICE_HIDE))
		skip++;
	if ((_curDialog == dC581) && (curChoice == 1))
		skip++;
	if ((_curDialog == dSHOPKEEPER1A) && (curChoice == 1))
		skip++;
	// if there's a pre-dialog
	if ((_dialog[i]._startLen > 0) && !skip)
		_vm->_animMgr->playMovie(_dialog[i]._startAnim, 0, _dialog[i]._startLen - 1);
	else {
		_vm->_animMgr->smkSoundOnOff(1, false);
		afterChoice();
	}
}

void DialogManager::afterChoice() {
	Dialog *dialog = &_dialog[_curDialog];

	memset(_vm->_screenBuffer, 0, MAXX * TOP * 2);
	_vm->_graphicsMgr->copyToScreen(0, 0, MAXX, TOP);

	switch (_curDialog) {
	case dTRAMP171:
		if (_curChoice == 80) {
			_vm->_obj[ocTRAMP17]._action = 213;
			_vm->_obj[ocTRAMP17]._flag &= ~kObjFlagPerson;
		} else if (_curChoice == 77) {
			_vm->_obj[ocTRAMP17]._action = 211;
			_vm->_obj[ocTRAMP17]._flag &= ~kObjFlagPerson;
		}
		break;

	case dTRAMP1714:
		if (_curChoice == 106) {
			_vm->_obj[ocTRAMP17]._action = 213;
			_vm->_obj[ocTRAMP17]._flag &= ~kObjFlagPerson;
		}
		break;

	case dTRAMP1713:
		if (_curChoice == 91) {
			_vm->_obj[ocTRAMP17]._action = 212;
			_vm->_obj[ocTRAMP17]._flag &= ~kObjFlagPerson;
		}
		break;

	case dTRAMP1716:
		if (_curChoice == 122) {
			_vm->_obj[ocTRAMP17]._action = 212;
			_vm->_obj[ocTRAMP17]._flag &= ~kObjFlagPerson;
		}
		break;

	case dTRAMP1717:
		if (_curChoice == 136) {
			_vm->_obj[ocTRAMP17]._mode &= ~OBJMODE_OBJSTATUS;
			_vm->_obj[oTRAMPD17]._mode |= OBJMODE_OBJSTATUS;
			_vm->_room[kRoom17]._bkgAnim = aBKG17B;
			_vm->addIcon(kItemSkate);
		} else if (_curChoice == 137) {
			_vm->_obj[ocTRAMP17]._flag |= kObjFlagPerson;
		}
		break;

	case dGUARDIAN18:
		if ((_curChoice == 151) || (_curChoice == 152)) {
			_vm->_inventoryObj[kItemRubysPhoto]._action = 1465;
			_vm->_obj[oTESSERA1A]._action = 238;
			if (_vm->_obj[oTESSERA1A]._flag & kObjFlagExtra) {
				_choice[154]._flag &= ~DLGCHOICE_HIDE;
				_choice[153]._flag |= DLGCHOICE_HIDE;
			} else
				_choice[153]._flag &= ~DLGCHOICE_HIDE;
		} else if (_curChoice == 154) {
			if (_vm->_obj[oTESSERA1A]._flag & kObjFlagExtra)
				_choice[183]._flag &= ~DLGCHOICE_HIDE;
		} else if (_curChoice == 155)
			_vm->_obj[ocGUARD18]._action = 228;
		break;
	case dF213B:
	case dF213:
		_vm->setRoom(kRoom21, true);
		break;

	case dF212B:
	case dF212:
		_vm->setRoom(kRoom21, false);
		break;

	case dF321:
		_vm->removeIcon(kItemMakeshiftTorch);
		_vm->_flagShowCharacter = false;
		break;

	case dF4A3:
		_vm->_obj[oCHOCOLATES4A]._examine = 1105;
		_vm->_obj[oCHOCOLATES4A]._action = 1106;
		_vm->_obj[oDOORC4A]._action = 1118;
		_vm->_animMgr->_animTab[aBKG4A]._flag |= SMKANIM_OFF1;
		_vm->_obj[ocHELLEN4A]._mode &= ~OBJMODE_OBJSTATUS;
		_vm->_obj[oHELLENA4A]._mode |= OBJMODE_OBJSTATUS;
		break;

	case dC581:
		_vm->_obj[oWINDOWB58]._mode |= OBJMODE_OBJSTATUS;
		if (_curChoice == 262)
			_vm->_obj[oTASTIERA56]._examine = 1307;
		break;

	case dF542:
		_vm->_obj[oGRATAC54]._mode &= ~OBJMODE_OBJSTATUS;
		_vm->_obj[oDOORC54]._mode &= ~OBJMODE_OBJSTATUS;
		_vm->_obj[oLAVATRICEL54]._mode &= ~OBJMODE_OBJSTATUS;
		_vm->_obj[oLAVATRICEF54]._mode &= ~OBJMODE_OBJSTATUS;
		_vm->_obj[oGRATAA54]._mode |= OBJMODE_OBJSTATUS;
		_vm->_obj[oCHIAVI54]._mode |= OBJMODE_OBJSTATUS;
		_vm->_obj[od54ALLA55]._mode |= OBJMODE_OBJSTATUS;
		break;
	default:
		break;
	}

	// If the player chose to exit the dialog
	if (_choice[_curChoice]._flag & DLGCHOICE_EXITDLG) {
		_vm->_animMgr->stopFullMotion();

		switch (_curDialog) {
		case dPOLIZIOTTO16:
			if ((_choice[61]._flag & kObjFlagDone) && (_choice[62]._flag & kObjFlagDone) && (_vm->_obj[ocPOLIZIOTTO16]._flag & kObjFlagExtra))
				_vm->_obj[ocPOLIZIOTTO16]._mode &= ~OBJMODE_OBJSTATUS;
			break;

		case dTRAMP171:
			if (_curChoice == 77) {
				_vm->_obj[ocTRAMP17]._action = 211;
				_vm->_obj[ocTRAMP17]._flag &= ~kObjFlagPerson;
			} else if (_curChoice == 80)
				_vm->_obj[ocTRAMP17]._action = 213;
			else if (_curChoice == 122)
				_vm->_obj[ocTRAMP17]._action = 211;
			break;

		case dGUARDIAN18:
			if (_curChoice == 152)
				_vm->_obj[ocGUARD18]._mode &= ~OBJMODE_OBJSTATUS;
			else if (_curChoice == 155)
				_vm->StartCharacterAction(a184ENTRACLUB, kRoom19, 2, 0);
			break;

		case dEVA19:
			_vm->_obj[oDOORC18]._flag &= ~kObjFlagRoomOut;
			_vm->_obj[oDOORC18]._action = 221;
			_vm->_obj[ocEVA19]._action = 1999;
			_vm->_obj[ocEVA19]._flag &= ~kObjFlagPerson;
			break;

		case dSHOPKEEPER1A:
			if (_curChoice == 185) {
				doEvent(MC_SYSTEM, ME_CHANGEROOM, MP_SYSTEM, kRoom18, a1810ENTRADALNEGOZIO, 10, _vm->_curObj);
				_vm->_obj[oDOORN18]._flag &= ~kObjFlagRoomOut;
				_vm->_obj[oDOORN18]._action = 218;
				_vm->_obj[oDOORN18]._anim = 0;
			} else if (_curChoice == 183)
				_vm->_obj[oTESSERA1A]._action = 239;
			break;

		case dF181:
			_vm->_obj[oRETE17]._mode |= OBJMODE_OBJSTATUS;
			_vm->_obj[oDOORA17]._examine = 196;
			_vm->_obj[oDOORUA17]._examine = 187;
			_vm->_obj[oDOORUB17]._examine = 192;
			_vm->_obj[oDOORA17]._action = 188;
			_vm->_obj[oDOORUA17]._action = 193;
			_vm->_obj[oDOORUB17]._action = 197;
			_vm->_obj[oFINGERPAD17]._mode &= ~OBJMODE_OBJSTATUS;
			_vm->_room[kRoom17]._flag &= ~kObjFlagDone;
			_vm->_room[kRoom17]._flag |= kObjFlagExtra;
			break;

		case dF1C1:
			CharacterSay(334);
			break;

		case dF1D1:
			CharacterSay(1502);
			break;

		case dF2E1:
			CharacterSay(621);
			_vm->_obj[oCATWALKA2E]._action = 622;
			break;

		case dF2E2:
			CharacterSay(1561);
			_vm->_inventoryObj[kItemMicrowaveGun]._examine = 1562;
			break;

		case dF231:
			_vm->_obj[od21TO23]._goRoom = kRoom23B;
			_vm->_obj[od24TO23]._goRoom = kRoom23B;
			break;

		case dF291:
			_vm->_obj[oSWITCH29]._action = 479;
			_vm->_obj[od22TO29]._goRoom = kRoom29L;
			_vm->_obj[od2ATO29]._goRoom = kRoom29L;
			_vm->_obj[od22TO29]._mode &= ~OBJMODE_OBJSTATUS;
			_vm->_obj[od22TO29I]._mode |= OBJMODE_OBJSTATUS;
			break;

		case dF2G1:
			_vm->_obj[oPANELM2G]._action = 660;
			CharacterSay(659);
			break;

		case dF2G2:
			_vm->_obj[od26TO2G]._goRoom = kRoom2GV;
			_vm->replaceIcon(kItemMinicom, kItemDamagedMinicom);
			break;

		case dF321:
			_vm->StartCharacterAction(a3111TRASCINA, 0, 11, 0);
			break;

		case dF331:
			_vm->_obj[oTUBET33]._lim = Common::Rect(0, 0, 0, 0);
			CharacterSay(1997);
			break;

		case dF362:
			playDialog(dC381);
			break;

		case dC381:
			playDialog(dF381);
			break;

		case dF381:
			doEvent(MC_SYSTEM, ME_CHANGEROOM, MP_SYSTEM, kRoom41, 0, 18, _vm->_curObj);
			_vm->_cyberInventory = _vm->_inventory;
			_vm->_iconBase = 0;
			_vm->_inventory.clear();
			_vm->_inventory.push_back(kItemPositioner);
			break;

		case dF371:
			_vm->_obj[oSCAFFALE36]._anim = a3615AAPRENDESCAFFALE;
			_vm->_animMgr->smkVolumePan(0, 1, 1);
			break;

		case dF431:
			_vm->_flagShowCharacter = true;
			_vm->StartCharacterAction(aWALKIN, 0, 11, 0);
			break;

		case dF451:
			_vm->_obj[od44ALLA45]._goRoom = kRoom45S;
			CharacterSay(1712);
			break;

		case dF491:
			for (int c = oPULSANTE1AD; c <= oPULSANTE33AD; c++) {
				if (!(_vm->_obj[c]._goRoom)) {
					_vm->_obj[c]._goRoom = kRoom4A;
					_vm->_obj[c]._mode |= OBJMODE_OBJSTATUS;
					_vm->_obj[c - 40]._mode &= ~OBJMODE_OBJSTATUS;
					break;
				}
			}
			_vm->removeIcon(kItemShaft);
			playDialog(dC4A1);
			setPosition(12);
			break;

		case dC4A1:
			_vm->_flagCharacterExists = true;
			_vm->_flagShowCharacter = true;
			_vm->_actor->actorStop();
			nextStep();
			break;

		case dF4C1:
			_vm->_inventory = _vm->_cyberInventory;
			_vm->_iconBase = 0;
			_vm->removeIcon(kItemLiftCard);
			_vm->removeIcon(kItemPen);
			_vm->removeIcon(kItemLetterboxKey);
			_vm->removeIcon(kItemLetter);
			_vm->removeIcon(kItemSubwayCard);
			_vm->removeIcon(kItemRubysPhoto);
			_vm->removeIcon(kItemPistol);
			_vm->removeIcon(kItemRubysReport);
			_vm->removeIcon(kItemMembershipCard);
			_vm->removeIcon(kItemMicrowaveGun);
			_vm->removeIcon(kItemFaultyBulb);
			_vm->removeIcon(kItemElevatorRemoteControl);
			_vm->removeIcon(kItemSecurityCard);
			_vm->removeIcon(kItemSecuritySystemSequence);
			_vm->removeIcon(kItemStethoscope);
			_vm->removeIcon(kItemRubysMedicalReport);
			_vm->removeIcon(kItemEgyptologyBook);
			_vm->addIcon(kItemPrisonMap);
			_vm->addIcon(kItemParallelCutter);
			_vm->addIcon(kItemWristComm);
			_vm->StartCharacterAction(a511, 0, 1, 0);
			break;

		case dF4P1:
			CharacterSay(1707);
			break;

		case dF4P2:
			CharacterSay(1708);
			break;

		case dF562:
			_vm->_obj[oDOOR58C55]._action = 1287;
			_vm->_obj[oDOOR58C55]._anim = 0;
			_vm->_obj[oWINDOW58P55]._action = 1292;
			_vm->_obj[oWINDOW58P55]._mode |= OBJMODE_OBJSTATUS;
			_vm->_obj[oWINDOW58P55]._anim = 0;
			break;

		case dF5A1:
			_vm->_obj[oDOOR58C55]._action = 1286;
			_vm->_obj[oWINDOW58P55]._action = 1291;
			_vm->_obj[oWINDOWA5A]._action = 1403;
			_vm->_obj[oGUARDIA58]._mode |= OBJMODE_OBJSTATUS;
			_choice[286]._flag |= kObjFlagDone;
			break;

		case dC581:
			if (!(_choice[886]._flag & kObjFlagDone) && (_choice[258]._flag & kObjFlagDone)) {
				setPosition(1);
				playDialog(dF581);
			}
			break;

		case dC582:
			_vm->_obj[oWINDOWA58]._mode |= OBJMODE_OBJSTATUS;
			_vm->addIcon(kItemGovernorsCode);
			break;

		case dC5A1:
			_vm->_obj[oWINDOWA5A]._action = 1402;
			if (_vm->_room[kRoom5A]._flag & kObjFlagExtra)
				playDialog(dF5A1);
			break;

		case dFLOG:
			playDialog(dINTRO);
			break;

		case dINTRO:
			doEvent(MC_SYSTEM, ME_CHANGEROOM, MP_SYSTEM, kRoom11, 0, 20, _vm->_curObj);
			break;

		case dF582:
			playDialog(dFCRED);
			break;

		case dFCRED:
			doEvent(MC_SYSTEM, ME_QUIT, MP_SYSTEM, 0, 0, 0, 0);
			break;

		default:
			break;
		}
		return;
	}

	// If another dialog starts
	if (_choice[_curChoice]._nextDialog != 0) {
		_curDialog = _choice[_curChoice]._nextDialog;
		_vm->_flagDialogActive = true;
		_curChoice = 0;

		dialog = &_dialog[_curDialog];

		// If there is a pre-dialog
		if (_dialog[_curDialog]._startLen > 0) {
			_vm->_animMgr->playMovie(_dialog[_curDialog]._startAnim, 0, _dialog[_curDialog]._startLen - 1);
			return;
		}
	}

	// Immediately starts the fraud choice
	for (int c = dialog->_firstChoice; c < dialog->_firstChoice + dialog->_choiceNumb; ++c) {
		if ((_choice[c]._flag & DLGCHOICE_FRAUD) && (!(_choice[c]._flag & DLGCHOICE_HIDE))) {
			playChoice(c);
			return;
		}
	}

	// If there's only one option, show it immediately, otherwise show available choices
	int res = 0;
	for (int c = dialog->_firstChoice; c < dialog->_firstChoice + dialog->_choiceNumb; c++) {
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
		playChoice(res);
		return;
	}

	// If no option is visible, close the dialog
	res = 0;
	for (int c = dialog->_firstChoice; c < dialog->_firstChoice + dialog->_choiceNumb; c++) {
		if (!(_choice[c]._flag & DLGCHOICE_HIDE))
			res++;
	}

	if (res == 0) {
		_vm->_animMgr->stopFullMotion();
		if (_curDialog == dC381)
			playDialog(dF381);
		return;
	}

	showChoices(_curDialog);
}

void DialogManager::dialogHandler(int numFrame) {
	if (_vm->_flagDialogActive && !_vm->_flagDialogMenuActive) {
		_vm->hideCursor();
		if (numFrame == _subTitles[_curSubTitle]._startFrame) {
			int i = _curSubTitle++;
			_vm->_sdText._rect.left = _subTitles[i]._x;
			_vm->_sdText._rect.top = _subTitles[i]._y;
			_vm->_sdText.tcol = _subTitles[i]._color;
			_vm->_sdText.text = _vm->_sentence[_subTitles[i]._sentence];
		}
	}
}

void DialogManager::playChoice(uint16 i) {
	assert(i < MAXCHOICE);

	DialogChoice *choice = &_choice[i];
	const int startFrame = choice->_startFrame - 1;
	const int endSubTitle = choice->_firstSubTitle + choice->_subTitleNumb;
	int totalLength = 0;

	memset(_vm->_screenBuffer, 0, MAXX * TOP * 2);
	_vm->_graphicsMgr->copyToScreen(0, 0, MAXX, TOP);

	_curChoice = i;
	_curSubTitle = choice->_firstSubTitle;
	_vm->_flagDialogMenuActive = false;

	choice->_flag |= kObjFlagDone;

	// se era one time la disabilita
	if (choice->_flag & DLGCHOICE_ONETIME)
		choice->_flag |= DLGCHOICE_HIDE;

	// Disable other choices
	for (int c = 0; c < MAXDISPCHOICES; c++) {
		_choice[choice->_off[c]]._flag |= DLGCHOICE_HIDE;
		_choice[choice->_on[c]]._flag &= ~DLGCHOICE_HIDE;
	}

	// WORKAROUND: For some reason, the dialog choices seem to include
	// extra frames for each sentence. We chop them off here
	for (int c = _curSubTitle; c < endSubTitle; c++)
		totalLength += _subTitles[c]._length - 3;

	_vm->hideCursor();
	_vm->_animMgr->playMovie(_dialog[_curDialog]._startAnim, startFrame, startFrame + totalLength - 1);
}

void DialogManager::doDialog() {
	switch (_vm->_curMessage->_event) {
	case ME_ENDCHOICE:
		afterChoice();
		break;

	case ME_STARTDIALOG:
		playDialog(_vm->_curMessage->_u16Param1);
		break;

	default:
		break;
	}
}

bool DialogManager::showCharacterAfterDialog() const {
	switch (_curDialog) {
	case dF321:
	case dF431:
	case dF4C1:
	case dASCENSORE12:
	case dASCENSORE13:
	case dASCENSORE16:
		return false;
	default:
		return true;
	}
}

void DialogManager::syncGameStream(Common::Serializer &ser) {
	for (int a = 0; a < MAXCHOICE; a++) {
		DialogChoice *choice = &_choice[a];
		ser.syncAsUint16LE(choice->_flag);
		ser.syncAsUint16LE(choice->_sentenceIndex);
		ser.syncAsUint16LE(choice->_firstSubTitle);
		ser.syncAsUint16LE(choice->_subTitleNumb);
		for (int i = 0; i < MAXDISPCHOICES; i++)
			ser.syncAsUint16LE(choice->_on[i]);
		for (int i = 0; i < MAXDISPCHOICES; i++)
			ser.syncAsUint16LE(choice->_off[i]);
		ser.syncAsUint16LE(choice->_startFrame);
		ser.syncAsUint16LE(choice->_nextDialog);
	}

	for (int a = 0; a < MAXDIALOG; a++) {
		Dialog *dialog = &_dialog[a];
		ser.syncAsUint16LE(dialog->_flag);
		ser.syncAsUint16LE(dialog->_interlocutor);
		ser.syncBytes((byte *)dialog->_startAnim, 14);
		ser.syncAsUint16LE(dialog->_startLen);
		ser.syncAsUint16LE(dialog->_firstChoice);
		ser.syncAsUint16LE(dialog->_choiceNumb);
		for (int i = 0; i < MAXNEWSMKPAL; i++)
			ser.syncAsUint16LE(dialog->_newPal[i]);
	}
}

void DialogManager::loadData(Common::File *file) {
	for (int i = 0; i < MAXDIALOG; ++i) {
		Dialog *dialog = &_dialog[i];

		dialog->_flag = file->readUint16LE();
		dialog->_interlocutor = file->readUint16LE();

		file->read(&dialog->_startAnim, ARRAYSIZE(dialog->_startAnim));

		dialog->_startLen = file->readUint16LE();
		dialog->_firstChoice = file->readUint16LE();
		dialog->_choiceNumb = file->readUint16LE();

		for (int j = 0; j < MAXNEWSMKPAL; ++j)
			dialog->_newPal[j] = file->readUint16LE();
	}

	for (int i = 0; i < MAXCHOICE; ++i) {
		DialogChoice *choice = &_choice[i];

		choice->_flag = file->readUint16LE();
		choice->_sentenceIndex = file->readUint16LE();
		choice->_firstSubTitle = file->readUint16LE();
		choice->_subTitleNumb = file->readUint16LE();

		for (int j = 0; j < MAXDISPCHOICES; ++j)
			choice->_on[j] = file->readUint16LE();

		for (int j = 0; j < MAXDISPCHOICES; ++j)
			choice->_off[j] = file->readUint16LE();

		choice->_startFrame = file->readUint16LE();
		choice->_nextDialog = file->readUint16LE();
	}

	for (int i = 0; i < MAXSUBTITLES; ++i) {
		DialogSubTitle *subTitle = &_subTitles[i];

		subTitle->_sentence = file->readUint16LE();
		subTitle->_x = file->readUint16LE();
		subTitle->_y = file->readUint16LE();
		subTitle->_color = file->readUint16LE();
		subTitle->_startFrame = file->readUint16LE();
		subTitle->_length = file->readUint16LE();
	}
}

} // End of namespace Trecision
