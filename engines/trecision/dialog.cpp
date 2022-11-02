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

 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.

 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 *
 */

#include "trecision/actor.h"
#include "trecision/animmanager.h"
#include "trecision/defines.h"
#include "trecision/dialog.h"
#include "trecision/graphics.h"
#include "trecision/logic.h"
#include "trecision/pathfinding3d.h"
#include "trecision/text.h"
#include "trecision/trecision.h"
#include "trecision/video.h"

namespace Trecision {
void Dialog::clear() {
	_flag = 0;
	_interlocutor = 0;
	memset(_startAnim, 0, 14);
	_startLen = 0;
	_firstChoice = 0;
	_choiceNumb = 0;
	for (uint16 i = 0; i < MAXNEWSMKPAL; ++i)
		_newPal[i] = 0;
}

void DialogSubTitle::clear() {
	_sentence = 0;
	_x = _y = 0;
	_color = 0;
	_startFrame = 0;
	_length = 0;
}

void DialogChoice::clear() {
	_flag = 0;
	_sentenceIndex = 0;
	_firstSubTitle = _subTitleNumb = 0;
	for (int i = 0; i < MAXDISPCHOICES; ++i) {
		_on[i] = _off[i] = 0;
	}
	
	_startFrame = 0;
	_nextDialog = 0;
}

/**************************************************************/

DialogManager::DialogManager(TrecisionEngine *vm) : _vm(vm) {
	_curDialog = 0;
	_curChoice = 0;
	_curSubTitle = 0;
	_curDispChoice = 0;
	_curPos = -1;
	_lastPos = -1;

	for (int i = 0; i < MAXDIALOG; ++i)
		_dialog[i].clear();

	for (int i = 0; i < MAXCHOICE; ++i)
		_choice[i].clear();

	for (int i = 0; i < MAXSUBTITLES; ++i)
		_subTitles[i].clear();

	for (int i = 0; i < MAXDISPCHOICES; ++i)
		_dispChoice[i] = 0;
}

DialogManager::~DialogManager() {}

void DialogManager::dialogPrint(int x, int y, int c, const Common::String &txt) {
	SDText curChoice;
	curChoice.set(
		Common::Rect(x, y, _vm->textLength(txt) + x, y),
		Common::Rect(0, 0, MAXX, MAXY),
		c,
		txt
	);
	curChoice.draw(_vm);
}

void DialogManager::showChoices(uint16 i) {
	assert(i < MAXDIALOG);

	Dialog *dialog = &_dialog[i];

	int x = 10;
	int y = 5;
	_curPos = -1;
	_lastPos = -1;
	_vm->_graphicsMgr->clearScreenBufferTop();

	for (int c = 0; c < MAXDISPCHOICES; ++c)
		_dispChoice[c] = 0;

	_curDispChoice = 0;
	for (int c = dialog->_firstChoice; c < dialog->_firstChoice + dialog->_choiceNumb; ++c) {
		if (isChoiceVisible(c)) {
			_dispChoice[_curDispChoice] = c;
			++_curDispChoice;
			dialogPrint(x, y, HWHITE, _vm->_sentence[_choice[c]._sentenceIndex]);
			y += CARHEI;
		}
	}

	_vm->_graphicsMgr->copyToScreen(0, 0, MAXX, TOP);

	_vm->_flagDialogMenuActive = true;
	_vm->_graphicsMgr->showCursor();
}

void DialogManager::updateChoices(int16 dmx, int16 dmy) {
	if ((dmy >= MAXDISPCHOICES) && (dmy < CARHEI * _curDispChoice + 5))
		_curPos = (dmy - 5) / CARHEI;
	else
		_curPos = -1;

	if ((_curPos != _lastPos) && ((_curPos != -1) || (_lastPos != -1))) {
		for (int c = 0; c < MAXDISPCHOICES; ++c) {
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
		playChoice(_dispChoice[_curPos], false);
	}
}

void DialogManager::playDialog(uint16 i) {
	_vm->closeInventoryImmediately();

	_curDialog = i;
	_curChoice = 0;
	_curSubTitle = 0;

	if (_curDialog == dSHOPKEEPER1A)
		_dialog[_curDialog]._startLen = 0;
	
	_vm->_animMgr->startFullMotion();

	bool skip = false;
	int curChoice = 0;
	for (int c = _dialog[_curDialog]._firstChoice; c < _dialog[_curDialog]._firstChoice + _dialog[_curDialog]._choiceNumb; ++c) {
		if (isChoiceVisible(c))
			++curChoice;
	}

	if ((_curDialog == dC581 && isChoiceVisible(262)) ||
		(_curDialog == dC581 && curChoice == 1) ||
		(_curDialog == dSHOPKEEPER1A && curChoice == 1))
		skip = true;
	// if there's a pre-dialog
	if (_dialog[i]._startLen > 0 && !skip) {
		_vm->_animMgr->playMovie(_dialog[i]._startAnim, 1, _dialog[i]._startLen);
	} else {
		_vm->_animMgr->smkToggleAudio(1, false);
		afterChoice();
	}

	if (_curDialog == dSHOPKEEPER1A)
		_dialog[_curDialog]._startLen = 1;
}

void DialogManager::toggleChoice(uint16 choice, bool enable) {
	if (enable)
		_choice[choice]._flag &= ~DLGCHOICE_HIDE;
	else
		_choice[choice]._flag |= DLGCHOICE_HIDE;
}

void DialogManager::clearExitFlag(uint16 choice) {
	_choice[choice]._flag &= ~DLGCHOICE_EXITDLG;
}

bool DialogManager::isChoiceVisible(uint16 choice) const {
	return !(_choice[choice]._flag & DLGCHOICE_HIDE);
}

bool DialogManager::isDialogFinished(uint16 choice) const {
	return _choice[choice]._flag & kObjFlagDone;
}

void DialogManager::afterChoice() {
	Dialog *dialog = &_dialog[_curDialog];

	_vm->_graphicsMgr->clearScreenBufferTop();
	_vm->_graphicsMgr->copyToScreen(0, 0, MAXX, TOP);

	switch (_curDialog) {
	case dTRAMP171:
		if (_curChoice == 80) {
			_vm->_obj[ocTRAMP17]._action = 213;
			_vm->_obj[ocTRAMP17].setFlagPerson(false);
		} else if (_curChoice == 77) {
			_vm->_obj[ocTRAMP17]._action = 211;
			_vm->_obj[ocTRAMP17].setFlagPerson(false);
		}
		break;

	case dTRAMP1714:
		if (_curChoice == 106) {
			_vm->_obj[ocTRAMP17]._action = 213;
			_vm->_obj[ocTRAMP17].setFlagPerson(false);
		}
		break;

	case dTRAMP1713:
		if (_curChoice == 91) {
			_vm->_obj[ocTRAMP17]._action = 212;
			_vm->_obj[ocTRAMP17].setFlagPerson(false);
		}
		break;

	case dTRAMP1716:
		if (_curChoice == 122) {
			_vm->_obj[ocTRAMP17]._action = 212;
			_vm->_obj[ocTRAMP17].setFlagPerson(false);
		}
		break;

	case dTRAMP1717:
		if (_curChoice == 136) {
			_vm->setObjectVisible(ocTRAMP17, false);
			_vm->setObjectVisible(oTRAMPD17, true);
			_vm->_room[kRoom17]._bkgAnim = aBKG17B;
			_vm->addIcon(kItemSkate);
		} else if (_curChoice == 137) {
			_vm->_obj[ocTRAMP17].setFlagPerson(true);
		}
		break;

	case dGUARDIAN18:
		if ((_curChoice == 151) || (_curChoice == 152)) {
			_vm->_inventoryObj[kItemRubysPhoto]._action = 1465;
			_vm->_obj[oTESSERA1A]._action = 238;
			if (_vm->_obj[oTESSERA1A].isFlagExtra()) {
				toggleChoice(154, true);
				toggleChoice(153, false);
			} else
				toggleChoice(153, true);
		} else if (_curChoice == 154) {
			if (_vm->_obj[oTESSERA1A].isFlagExtra())
				toggleChoice(183, true);
		} else if (_curChoice == 155)
			_vm->_obj[ocGUARD18]._action = 228;
		break;
	case dF213B:
	case dF213:
		_vm->_logicMgr->setupAltRoom(kRoom21, true);
		break;

	case dF212B:
	case dF212:
		_vm->_logicMgr->setupAltRoom(kRoom21, false);
		break;

	case dF321:
		_vm->removeIcon(kItemMakeshiftTorch);
		_vm->_flagShowCharacter = false;
		break;

	case dF4A3:
		_vm->_obj[oCHOCOLATES4A]._examine = 1105;
		_vm->_obj[oCHOCOLATES4A]._action = 1106;
		_vm->_obj[oDOORC4A]._action = 1118;
		_vm->_animMgr->_animTab[aBKG4A].toggleAnimArea(1, false);
		_vm->setObjectVisible(ocHELLEN4A, false);
		_vm->setObjectVisible(oHELLENA4A, true);
		break;

	case dC581:
		_vm->setObjectVisible(oWINDOWB58, true);
		if (_curChoice == 262)
			_vm->_obj[oKEYBOARD56]._examine = 1307;
		break;

	case dF542:
		_vm->setObjectVisible(oGRATAC54, false);
		_vm->setObjectVisible(oDOORC54, false);
		_vm->setObjectVisible(oLAVATRICEL54, false);
		_vm->setObjectVisible(oLAVATRICEF54, false);
		_vm->setObjectVisible(oGRATAA54, true);
		_vm->setObjectVisible(oCHIAVI54, true);
		_vm->setObjectVisible(od54TO55, true);
		break;
	default:
		break;
	}

	// If the player chose to exit the dialog
	if (_choice[_curChoice]._flag & DLGCHOICE_EXITDLG) {
		_vm->_animMgr->stopFullMotion();

		switch (_curDialog) {
		case dPOLIZIOTTO16:
			if ((isDialogFinished(61)) && (isDialogFinished(62)) && _vm->_obj[ocPOLIZIOTTO16].isFlagExtra())
				_vm->setObjectVisible(ocPOLIZIOTTO16, false);
			break;

		case dTRAMP171:
			if (_curChoice == 77) {
				_vm->_obj[ocTRAMP17]._action = 211;
				_vm->_obj[ocTRAMP17].setFlagPerson(false);
			} else if (_curChoice == 80)
				_vm->_obj[ocTRAMP17]._action = 213;
			else if (_curChoice == 122)
				_vm->_obj[ocTRAMP17]._action = 211;
			break;

		case dGUARDIAN18:
			if (_curChoice == 152)
				_vm->setObjectVisible(ocGUARD18, false);
			else if (_curChoice == 155)
				_vm->startCharacterAction(a184ENTRACLUB, kRoom19, 2, 0);
			break;

		case dEVA19:
			_vm->_obj[oDOORC18].setFlagRoomOut(false);
			_vm->_obj[oDOORC18]._action = 221;
			_vm->_obj[ocEVA19]._action = 1999;
			_vm->_obj[ocEVA19].setFlagPerson(false);
			break;

		case dSHOPKEEPER1A:
			if (_curChoice == 185) {
				_vm->changeRoom(kRoom18, a1810ENTRADALNEGOZIO, 10);
				_vm->_obj[oDOORN18].setFlagRoomOut(false);
				_vm->_obj[oDOORN18]._action = 218;
				_vm->setObjectAnim(oDOORN18, 0);
			} else if (_curChoice == 183)
				_vm->_obj[oTESSERA1A]._action = 239;
			break;

		case dF181:
			_vm->setObjectVisible(oRETE17, true);
			_vm->_obj[oDOORA17]._examine = 196;
			_vm->_obj[oDOORUA17]._examine = 187;
			_vm->_obj[oDOORUB17]._examine = 192;
			_vm->_obj[oDOORA17]._action = 188;
			_vm->_obj[oDOORUA17]._action = 193;
			_vm->_obj[oDOORUB17]._action = 197;
			_vm->setObjectVisible(oFINGERPAD17, false);
			_vm->_room[kRoom17].setDone(false);
			_vm->_room[kRoom17].setExtra(true);
			break;

		case dF1C1:
			_vm->_textMgr->characterSay(kSentenceMapZoo);
			break;

		case dF1D1:
			_vm->_textMgr->characterSay(kSentenceItWorked);
			break;

		case dF2E1:
			_vm->_textMgr->characterSay(kSentenceGoodDeterrent);
			_vm->_obj[oCATWALKA2E]._action = 622;
			break;

		case dF2E2:
			_vm->_textMgr->characterSay(kSentenceWastedCritter);
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
			_vm->setObjectVisible(od22TO29, false);
			_vm->setObjectVisible(od22TO29I, true);
			break;

		case dF2G1:
			_vm->_obj[oPANELM2G]._action = 660;
			_vm->_textMgr->characterSay(kSentenceHopeDidntWasteTheKid);
			break;

		case dF2G2:
			_vm->_obj[od26TO2G]._goRoom = kRoom2GV;
			_vm->replaceIcon(kItemMinicom, kItemDamagedMinicom);
			break;

		case dF321:
			_vm->startCharacterAction(a3111TRASCINA, 0, 11, 0);
			break;

		case dF331:
			_vm->_obj[oTUBET33]._area = Common::Rect(0, 0, 0, 0);
			_vm->_textMgr->characterSay(kSentenceSecretPassage);
			break;

		case dF362:
			playDialog(dC381);
			break;

		case dC381:
			playDialog(dF381);
			break;

		case dF381:
			_vm->changeRoom(kRoom41, 0, 18);
			_vm->_cyberInventory = _vm->_inventory;
			_vm->_iconBase = 0;
			_vm->_inventory.clear();
			_vm->_inventory.push_back(kItemPositioner);
			break;

		case dF371:
			_vm->setObjectAnim(oSCAFFALE36, a3615AAPRENDESCAFFALE);
			_vm->_animMgr->smkToggleTrackAudio(0, 1, true);
			break;

		case dF431:
			_vm->_flagShowCharacter = true;
			_vm->startCharacterAction(aWALKIN, 0, 11, 0);
			break;

		case dF451:
			_vm->_obj[od44TO45]._goRoom = kRoom45S;
			_vm->_textMgr->characterSay(kSentenceTheSpiderHasEscaped);
			break;

		case dF491:
			for (int c = oPULSANTE1AD; c <= oPULSANTE33AD; ++c) {
				if (!_vm->_obj[c]._goRoom) {
					_vm->_obj[c]._goRoom = kRoom4A;
					_vm->setObjectVisible(c, true);
					_vm->setObjectVisible(c - 40, false);
					break;
				}
			}
			_vm->removeIcon(kItemShaft);
			playDialog(dC4A1);
			_vm->_pathFind->setPosition(12);
			break;

		case dC4A1:
			_vm->_flagShowCharacter = true;
			_vm->_actor->actorStop();
			_vm->_pathFind->nextStep();
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
			_vm->startCharacterAction(a511, 0, 1, 0);
			break;

		case dF4P1:
			_vm->_textMgr->characterSay(kSentenceItDidntWork);
			break;

		case dF4P2:
			_vm->_textMgr->characterSay(kSentenceTakeThatWolfman);
			break;

		case dF562:
			_vm->_obj[oDOOR58C55]._action = 1287;
			_vm->setObjectAnim(oDOOR58C55, 0);
			_vm->_obj[oWINDOW58P55]._action = 1292;
			_vm->setObjectVisible(oWINDOW58P55, true);
			_vm->setObjectAnim(oWINDOW58P55, 0);
			break;

		case dF5A1:
			_vm->_obj[oDOOR58C55]._action = 1286;
			_vm->_obj[oWINDOW58P55]._action = 1291;
			_vm->_obj[oWINDOWA5A]._action = 1403;
			_vm->setObjectVisible(oGUARDIA58, true);
			_choice[286]._flag |= kObjFlagDone;
			break;

		case dC581:
			if (!isDialogFinished(886) && isDialogFinished(258)) {
				_vm->_pathFind->setPosition(1);
				playDialog(dF581);
			}
			break;

		case dC582:
			_vm->setObjectVisible(oWINDOWA58, true);
			_vm->addIcon(kItemGovernorsCode);
			break;

		case dC5A1:
			_vm->_obj[oWINDOWA5A]._action = 1402;
			if (_vm->_room[kRoom5A].hasExtra())
				playDialog(dF5A1);
			break;

		case dFLOG:
			playDialog(dINTRO);
			break;

		case dINTRO:
			_vm->changeRoom(kRoom11);
			break;

		case dF582:
			playDialog(dFCRED);
			break;

		case dFCRED:
			_vm->quitGame();
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
			_vm->_animMgr->playMovie(_dialog[_curDialog]._startAnim, 1, _dialog[_curDialog]._startLen);
			return;
		}
	}

	// Immediately starts the fraud choice
	for (int c = dialog->_firstChoice; c < dialog->_firstChoice + dialog->_choiceNumb; ++c) {
		if ((_choice[c]._flag & DLGCHOICE_FRAUD) && isChoiceVisible(c)) {
			const bool singleChoice = dialog->_choiceNumb == 1;
			playChoice(c, singleChoice);
			return;
		}
	}

	// If there's only one option, show it immediately, otherwise show available choices
	int res = 0;
	for (int c = dialog->_firstChoice; c < dialog->_firstChoice + dialog->_choiceNumb; ++c) {
		if (isChoiceVisible(c)) {
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
		const bool singleChoice = dialog->_choiceNumb == 1;
		playChoice(res, singleChoice);
		return;
	}

	// If no option is visible, close the dialog
	res = 0;
	for (int c = dialog->_firstChoice; c < dialog->_firstChoice + dialog->_choiceNumb; ++c) {
		if (isChoiceVisible(c))
			++res;
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
		_vm->_graphicsMgr->hideCursor();
		if (numFrame == _subTitles[_curSubTitle]._startFrame) {
			int i = _curSubTitle;
			++_curSubTitle;
			_vm->_drawText._rect.left = _subTitles[i]._x;
			_vm->_drawText._rect.top = _subTitles[i]._y;
			_vm->_drawText._textColor = _subTitles[i]._color;
			_vm->_drawText._text = _vm->_sentence[_subTitles[i]._sentence];
		}
	}
}

void DialogManager::playChoice(uint16 i, bool singleChoice) {
	assert(i < MAXCHOICE);

	DialogChoice *choice = &_choice[i];
	const int startFrame = choice->_startFrame;
	const int endSubTitle = choice->_firstSubTitle + choice->_subTitleNumb;
	int totalLength = 0;

	_vm->_graphicsMgr->clearScreenBufferTop();
	_vm->_graphicsMgr->copyToScreen(0, 0, MAXX, TOP);

	_curChoice = i;
	_curSubTitle = choice->_firstSubTitle;
	_vm->_flagDialogMenuActive = false;

	choice->_flag |= kObjFlagDone;

	// if it was 'one time', disable it
	if (choice->_flag & DLGCHOICE_ONETIME)
		toggleChoice(i, false);

	// Disable other choices
	for (int c = 0; c < MAXDISPCHOICES; ++c) {
		toggleChoice(choice->_off[c], false);
		toggleChoice(choice->_on[c], true);
	}

	for (int c = _curSubTitle; c < endSubTitle; ++c)
		totalLength += _subTitles[c]._length - 1;

	_vm->_graphicsMgr->hideCursor();
	_vm->_animMgr->playMovie(_dialog[_curDialog]._startAnim, startFrame, startFrame + totalLength - 1, singleChoice);
}

void DialogManager::doDialog() {
	switch (_vm->_curMessage->_event) {
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

bool DialogManager::handleShopKeeperDialog(uint16 curObj) {
	for (int c = _dialog[dSHOPKEEPER1A]._firstChoice; c < (_dialog[dSHOPKEEPER1A]._firstChoice + _dialog[dSHOPKEEPER1A]._choiceNumb); ++c) {
		if (isChoiceVisible(c)) {
			playDialog(_vm->_obj[curObj]._goRoom);
			return false;
		}
	}

	return true;
}

void DialogManager::syncGameStream(Common::Serializer &ser) {
	for (int i = 0; i < MAXCHOICE; ++i) {
		DialogChoice *choice = &_choice[i];
		ser.syncAsUint16LE(choice->_flag);
		ser.syncAsUint16LE(choice->_sentenceIndex);
		ser.syncAsUint16LE(choice->_firstSubTitle);
		ser.syncAsUint16LE(choice->_subTitleNumb);
		for (int j = 0; j < MAXDISPCHOICES; ++j)
			ser.syncAsUint16LE(choice->_on[j]);
		for (int j = 0; j < MAXDISPCHOICES; ++j)
			ser.syncAsUint16LE(choice->_off[j]);
		ser.syncAsUint16LE(choice->_startFrame);
		ser.syncAsUint16LE(choice->_nextDialog);
	}

	for (int i = 0; i < MAXDIALOG; ++i) {
		Dialog *dialog = &_dialog[i];
		ser.syncAsUint16LE(dialog->_flag);
		ser.syncAsUint16LE(dialog->_interlocutor);
		ser.syncBytes((byte *)dialog->_startAnim, 14);
		ser.syncAsUint16LE(dialog->_startLen);
		ser.syncAsUint16LE(dialog->_firstChoice);
		ser.syncAsUint16LE(dialog->_choiceNumb);
		for (int j = 0; j < MAXNEWSMKPAL; ++j)
			ser.syncAsUint16LE(dialog->_newPal[j]);
	}
}

void DialogManager::loadData(Common::SeekableReadStreamEndian *stream) {
	for (int i = 0; i < MAXDIALOG; ++i) {
		Dialog *dialog = &_dialog[i];

		dialog->_flag = stream->readUint16();
		dialog->_interlocutor = stream->readUint16();

		stream->read(&dialog->_startAnim, 14);

		dialog->_startLen = stream->readUint16();
		dialog->_firstChoice = stream->readUint16();
		dialog->_choiceNumb = stream->readUint16();

		for (int j = 0; j < MAXNEWSMKPAL; ++j)
			dialog->_newPal[j] = stream->readUint16();
	}

	for (int i = 0; i < MAXCHOICE; ++i) {
		DialogChoice *choice = &_choice[i];

		choice->_flag = stream->readUint16();
		choice->_sentenceIndex = stream->readUint16();
		choice->_firstSubTitle = stream->readUint16();
		choice->_subTitleNumb = stream->readUint16();

		for (int j = 0; j < MAXDISPCHOICES; ++j)
			choice->_on[j] = stream->readUint16();

		for (int j = 0; j < MAXDISPCHOICES; ++j)
			choice->_off[j] = stream->readUint16();

		choice->_startFrame = stream->readUint16();
		choice->_nextDialog = stream->readUint16();
	}

	for (int i = 0; i < MAXSUBTITLES; ++i) {
		DialogSubTitle *subTitle = &_subTitles[i];

		subTitle->_sentence = stream->readUint16();
		subTitle->_x = stream->readUint16();
		subTitle->_y = stream->readUint16();
		subTitle->_color = stream->readUint16();
		subTitle->_startFrame = stream->readUint16();
		subTitle->_length = stream->readUint16();
	}

	if (_vm->isDemo()) {
		_subTitles[975]._length = 113;
	}
}

} // End of namespace Trecision
