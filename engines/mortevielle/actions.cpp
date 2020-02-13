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
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.
 *
 */

/*
 * This code is based on original Mortville Manor DOS source code
 * Copyright (c) 1987-1989 Lankhor
 */

#include "mortevielle/mortevielle.h"
#include "mortevielle/dialogs.h"
#include "mortevielle/menu.h"
#include "mortevielle/mouse.h"
#include "mortevielle/outtext.h"

#include "common/scummsys.h"

namespace Mortevielle {

/**
 * Engine function - Move
 * @remarks	Originally called 'taller'
 */
void MortevielleEngine::fctMove() {
	int oldMenu = (_menu->_moveMenu[6]._menuId << 8) | _menu->_moveMenu[6]._actionId;
	if ((_coreVar._currPlace == ROOM26) && (_currAction == oldMenu)) {
		_coreVar._currPlace = LANDING;
		_caff = _coreVar._currPlace;
		drawPictureWithText();
		handleDescriptionText(2, _coreVar._currPlace);
	}
	if ((_coreVar._currPlace == LANDING) && (_currAction == oldMenu)) {
		if (!_syn)
			displayTextInVerbBar(getEngineString(S_GO_TO));
		displayStatusArrow();

		if (_keyPressedEsc)
			_destinationOk = false;

		if ((_anyone) || (_keyPressedEsc))
			return;

		setCoordinates(1);

		if (_num == 0)
			return;

		if (_num == 1) {
			_coreVar._currPlace = OWN_ROOM;
			_menu->setDestinationText(OWN_ROOM);
		} else if (_num == 7) {
			_coreVar._currPlace = ATTIC;
			_menu->setDestinationText(ATTIC);
		} else if (_num != 6)
			_coreVar._currPlace = ROOM26;

		if ((_num > 1) && (_num < 6))
			_roomDoorId = _num - 1;
		else if (_num > 7)
			_roomDoorId = _num - 3;

		if (_num != 6)
			prepareDisplayText();
		else
			showMoveMenuAlert();
		return;
	}
	exitRoom();
	int menuChoice = 1;
	oldMenu = (_menu->_moveMenu[menuChoice]._menuId << 8) | _menu->_moveMenu[menuChoice]._actionId;
	while (oldMenu != _currAction) {
		++menuChoice;
		oldMenu = (_menu->_moveMenu[menuChoice]._menuId << 8) | _menu->_moveMenu[menuChoice]._actionId;
	}

	switch (_coreVar._currPlace) {
	case MOUNTAIN:
		if (menuChoice == 1)
			gotoManorFront();
		else if (menuChoice == 2)
			checkManorDistance();
		_menu->setDestinationText(_coreVar._currPlace);
		return;
	case INSIDE_WELL:
		if (menuChoice == 1)
			floodedInWell();
		else if (menuChoice == 2)
			gotoManorBack();
		_menu->setDestinationText(_coreVar._currPlace);
		return;
	case BUREAU:
		if (menuChoice == 1)
			menuChoice = 6;
		break;
	case KITCHEN:
		if (menuChoice == 2)
			menuChoice = 6;
		else if (menuChoice == 5)
			menuChoice = 16;
		break;
	case CELLAR:
		if (menuChoice == 3)
			menuChoice = 6;
		break;
	case LANDING:
	case ROOM26:
		if (menuChoice == 4)
			menuChoice = 6;
		break;
	default:
		break;
	}

	if ((_coreVar._currPlace > MOUNTAIN) && (_coreVar._currPlace != ROOM26))
		menuChoice += 10;

	if ((_coreVar._currPlace == CHAPEL) && (menuChoice == 13))
		menuChoice = 16;
	else if (_coreVar._currPlace == MANOR_FRONT) {
		if (menuChoice == 12)
			menuChoice = 16;
		else if (menuChoice > 13)
			menuChoice = 15;
	} else if ((_coreVar._currPlace == MANOR_BACK) && (menuChoice > 14))
		menuChoice = 15;
	else if ((_coreVar._currPlace == WELL) && (menuChoice > 13) && (menuChoice != 17))
		menuChoice = 15;

	switch (menuChoice) {
	case 1:
		_coreVar._currPlace = BUREAU;
		break;
	case 2:
		_coreVar._currPlace = KITCHEN;
		break;
	case 3:
		_coreVar._currPlace = CELLAR;
		break;
	case 4:
		_coreVar._currPlace = LANDING;
		break;
	case 5:
	case 12:
		gotoManorFront();
		break;
	case 6:
	case 11:
		gotoDiningRoom();
		break;
	case 13:
		_coreVar._currPlace = CHAPEL;
		break;
	case 14:
		_coreVar._currPlace = WELL;
		break;
	case 15:
		checkManorDistance();
		break;
	case 16:
		gotoManorBack();
		break;
	case 17:
		if ((_coreVar._wellObjectId != 120) && (_coreVar._wellObjectId != 140))
			_crep = 997;
		else if (_coreVar._wellObjectId == 120)
			_crep = 181;
		else if (_coreVar._faithScore > 80) {
			_crep = 1505;
			loseGame();
		} else {
			_coreVar._currPlace = INSIDE_WELL;
			prepareDisplayText();
		}
		break;
	default:
		break;
	}

	if ((menuChoice < 5) || (menuChoice == 13) || (menuChoice == 14))
		prepareDisplayText();
	resetRoomVariables(_coreVar._currPlace);
	_menu->setDestinationText(_coreVar._currPlace);
}

/**
 * Engine function - Take
 * @remarks	Originally called 'tprendre'
 */
void MortevielleEngine::fctTake() {
	if (_caff > 99) {
		int cx = _caff;
		putInHand(cx);
		if (_crep != 139) {
			if (_currBitIndex > 0)
				_coreVar._faithScore += 3;
			if (_obpart) {
				switch (_coreVar._currPlace) {
				case PURPLE_ROOM:
					_coreVar._purpleRoomObjectId = 0;
					break;
				case ATTIC:
					if (_coreVar._atticBallHoleObjectId == _caff)
						_coreVar._atticBallHoleObjectId = 0;
					if (_coreVar._atticRodHoleObjectId == _caff)
						_coreVar._atticRodHoleObjectId = 0;
					break;
				case CELLAR:
					_coreVar._cellarObjectId = 0;
					break;
				case CRYPT:
					_coreVar._cryptObjectId = 0;
					break;
				case SECRET_PASSAGE:
					_coreVar._secretPassageObjectId = 0;
					break;
				case WELL:
					_coreVar._wellObjectId = 0;
					break;
				default:
					break;
				}
				_menu->unsetSearchMenu();
				_obpart = false;
				prepareDisplayText();
			} else {
				_tabdon[kAcha + ((_curSearchObjId - 1) * 10) + _searchCount - 1] = 0;
				prepareNextObject();
				++_takeObjCount;
				if (_takeObjCount > 6) {
					_coreVar._faithScore += 2;
					_takeObjCount = 0;
				}
			}
		}
		return;
	}
	if (!_syn)
		displayTextInVerbBar(getEngineString(S_TAKE));
	displayStatusArrow();
	if ((_anyone) || (_keyPressedEsc))
		return;
	if (_caff == 3) {
		setCoordinates(2);
		if (_num == 1) {
			_crep = 152;
			return;
		}
	}
	setCoordinates(5);
	if ((_num == 0) || ((_num == 1) && (_coreVar._currPlace == CRYPT))) {
		setCoordinates(8);
		if (_num != 0) {
			if (_currBitIndex > 0)
				_coreVar._faithScore += 3;
			_crep = 997;

			switch (_coreVar._currPlace) {
			case PURPLE_ROOM:
				if (_coreVar._purpleRoomObjectId != 0)
					putInHand(_coreVar._purpleRoomObjectId);
				break;
			case ATTIC:
				if ((_num == 1) && (_coreVar._atticBallHoleObjectId != 0)) {
					putInHand(_coreVar._atticBallHoleObjectId);
					if ((_crep != 997) && (_crep != 139))
						displayAnimFrame(2, 7);
				} else if ((_num == 2) && (_coreVar._atticRodHoleObjectId != 0)) {
					putInHand(_coreVar._atticRodHoleObjectId);
					if ((_crep != 997) && (_crep != 139))
						displayAnimFrame(2, 6);
				}
				break;
			case CELLAR:
				if (_coreVar._cellarObjectId != 0) {
					putInHand(_coreVar._cellarObjectId);
					if ((_crep != 997) && (_crep != 139))
						displayAnimFrame(2, 2);
				}
				break;
			case CRYPT:
				if (_coreVar._cryptObjectId != 0)
					putInHand(_coreVar._cryptObjectId);
				break;
			case SECRET_PASSAGE:
				if (_coreVar._secretPassageObjectId != 0) {
					putInHand(_coreVar._secretPassageObjectId);
					if ((_crep != 997) && (_crep != 139)) {
						_crep = 182;
						displayAnimFrame(2, 1);
					}
				}
				break;
			case WELL:
				if (_coreVar._wellObjectId != 0) {
					putInHand(_coreVar._wellObjectId);
					if ((_crep != 997) && (_crep != 139))
						displayAnimFrame(2, 1);
				}
				break;
			default:
				break;
			}

			if ((_crep != 997) && (_crep != 182) && (_crep != 139))
				_crep = 999;
		}
	} else {
		if ( ((_coreVar._currPlace == OWN_ROOM)  && (_num == 3))
		  || ((_coreVar._currPlace == GREEN_ROOM)  && (_num == 4))
		  || ((_coreVar._currPlace == PURPLE_ROOM)  && (_num == 1))
		  || ((_coreVar._currPlace == DARKBLUE_ROOM)  && (_num == 3))
		  || ((_coreVar._currPlace == BLUE_ROOM)  && (_num == 6))
		  || ((_coreVar._currPlace == RED_ROOM)  && (_num == 2))
		  || ((_coreVar._currPlace == BATHROOM)  && (_num == 6))
		  || ((_coreVar._currPlace == GREEN_ROOM2)  && (_num == 4))
		  || ((_coreVar._currPlace == JULIA_ROOM) && (_num == 4))
		  || ((_coreVar._currPlace == DINING_ROOM) && (_num > 2))
		  || ((_coreVar._currPlace == BUREAU) && (_num == 7))
		  || ((_coreVar._currPlace == KITCHEN) && (_num == 6))
		  || ((_coreVar._currPlace == ATTIC) && (_num > 4))
		  || ((_coreVar._currPlace > ATTIC) && (_coreVar._currPlace != INSIDE_WELL)) )
		  _crep = 997;
		else if (_coreVar._currPlace == INSIDE_WELL) {
			_crep = 1504;
			loseGame();
		} else
			_crep = 120;
	}
}
/**
 * Engine function - Inventory / Take
 * @remarks	Originally called 'tsprendre'
 */
void MortevielleEngine::fctInventoryTake() {
	int inventIndex = 0;
	int oldMenu = 0;
	do {
		++inventIndex;
		oldMenu = (_menu->_inventoryMenu[inventIndex]._menuId << 8) | _menu->_inventoryMenu[inventIndex]._actionId;
	} while (oldMenu != _currAction);
	int cz = 0;
	int cy = 0;
	do {
		++cy;
		if (_coreVar._inventory[cy] != 0)
			++cz;
	} while (cz != inventIndex);
	cz = _coreVar._inventory[cy];
	_coreVar._inventory[cy] = 0;
	_menu->setInventoryText();
	putInHand(cz);
	_crep = 998;
	clearDescriptionBar();
}

/**
 * Engine function - Lift
 * @remarks	Originally called 'tsoulever'
 */
void MortevielleEngine::fctLift() {
	if (!_syn)
		displayTextInVerbBar(getEngineString(S_LIFT));
	displayStatusArrow();
	if ((_anyone) || (_keyPressedEsc))
		return;
	setCoordinates(3);
	if (_num == 0) {
		setCoordinates(8);
		if (_num != 0) {
			if (_currBitIndex > 0)
				++_coreVar._faithScore;
			_crep = 997;
			if ((_coreVar._currPlace == PURPLE_ROOM) && (_coreVar._purpleRoomObjectId != 0))
				displayLookScreen(_coreVar._purpleRoomObjectId);
		}
		return;
	}
	if (_currBitIndex > 0)
		++_coreVar._faithScore;
	int tmpPlace = _coreVar._currPlace;
	if (_coreVar._currPlace == CRYPT)
		tmpPlace = 14;
	else if (_coreVar._currPlace == MOUNTAIN)
		tmpPlace = 15;
	_crep = _tabdon[kAsoul + (tmpPlace << 3) + (_num - 1)];
	if (_crep == 255)
		_crep = 997;
}

/**
 * Engine function - Read
 * @remarks	Originally called 'tlire'
 */
void MortevielleEngine::fctRead() {
	if (_caff > 99)
		getReadDescription(_caff);
	else {
		if (!_syn)
			displayTextInVerbBar(getEngineString(S_READ));
		displayStatusArrow();
		if (!(_anyone) && !(_keyPressedEsc)) {
			setCoordinates(4);
			if (_num != 0)
				_crep = 107;
		}
	}
}

/**
 * Engine function - Self / Read
 * @remarks	Originally called 'tslire'
 */
void MortevielleEngine::fctSelfRead() {
	if (_coreVar._selectedObjectId == 0)
		_crep = 186;
	else
		getReadDescription(_coreVar._selectedObjectId);
}

/**
 * Engine function - Look
 * @remarks	Originally called 'tregarder'
 */
void MortevielleEngine::fctLook() {
	if (_caff > 99) {
		_crep = 103;
		return;
	}
	if (!_syn)
		displayTextInVerbBar(getEngineString(S_LOOK));
	displayStatusArrow();
	if ((_anyone) || (_keyPressedEsc))
		return;
	setCoordinates(5);
	if (_num == 0) {
		setCoordinates(8);
		_crep = 131;
		if (_num != 0) {
			if (_coreVar._currPlace == ATTIC) {
				if (_num == 1) {
					_crep = 164;
					if (_coreVar._atticRodHoleObjectId != 0)
						displayLookScreen(_coreVar._atticRodHoleObjectId);
					else if (_coreVar._atticBallHoleObjectId != 0)
						displayLookScreen(_coreVar._atticBallHoleObjectId);
				} else {
					_crep = 193;
					if (_coreVar._atticRodHoleObjectId != 0)
						displayLookScreen(_coreVar._atticRodHoleObjectId);
				}
			}
			if (_coreVar._currPlace == CELLAR) {
				_crep = 164;
				if (_coreVar._cellarObjectId != 0)
					displayLookScreen(_coreVar._cellarObjectId);
			}
			if (_coreVar._currPlace == SECRET_PASSAGE) {
				_crep = 174;
				if (_coreVar._secretPassageObjectId != 0)
					displayLookScreen(_coreVar._secretPassageObjectId);
			}
			if (_coreVar._currPlace == WELL) {
				_crep = 131;
				if (_coreVar._wellObjectId != 0)
					displayLookScreen(_coreVar._wellObjectId);
			}
		}
		return;
	}

	int cx = _coreVar._currPlace;
	switch (_coreVar._currPlace) {
	case CHAPEL:
		cx = 17;
		break;
	case MANOR_BACK:
	case INSIDE_WELL:
	case WELL:
		cx -= 4;
		break;
	case ROOM26:
		cx = 21;
		break;
	default:
		break;
	}

	_crep = _tabdon[kArega + (cx * 7) + _num - 1];
	if ((_coreVar._currPlace == ATTIC) && (_num == 8))
		_crep = 126;
	if (_coreVar._currPlace == MOUNTAIN)
		_crep = 103;
	if (_crep == 255)
		_crep = 131;
	if ((_coreVar._currPlace == GREEN_ROOM) && (_num == 1))
		displayLookScreen(144);
	if ((_coreVar._currPlace == BLUE_ROOM) && (_num == 3))
		displayLookScreen(147);
	if ((_coreVar._currPlace == GREEN_ROOM2) && (_num == 3))
		displayLookScreen(149);
	if ((_coreVar._currPlace == JULIA_ROOM) && (_num == 2))
		displayLookScreen(30);
	if ((_coreVar._currPlace == DINING_ROOM) && (_num == 3))
		displayLookScreen(31);
}

/**
 * Engine function - Self / Look
 * @remarks	Originally called 'tsregarder'
 */
void MortevielleEngine::fctSelftLook() {
	if (_coreVar._selectedObjectId != 0)
		displayLookScreen(_coreVar._selectedObjectId);
	else
		_crep = 186;
}

/**
 * Engine function - Search
 * @remarks	Originally called 'tfouiller'
 */
void MortevielleEngine::fctSearch() {
	static const byte answerArr[14] = {123, 104, 123, 131, 131, 123, 104, 131, 123, 123, 106, 123, 123, 107};

	if (_caff > 99) {
		getSearchDescription(_caff);
		return;
	}

	if (!_syn)
		displayTextInVerbBar(getEngineString(S_SEARCH));

	displayStatusArrow();
	if (_anyone || _keyPressedEsc)
		return;

	if (_coreVar._currPlace == INSIDE_WELL) {
		_crep = 1504;
		loseGame();
		return;
	}

	setCoordinates(6);
	if (_num == 0) {
		setCoordinates(7);
		if (_num != 0) {
			int i;
			for (i = 1; i <= 6; i++) {
				if (_num == _openObjects[i])
					break;
			}

			if (i <= 6) {
				if (_currBitIndex > 0)
					_coreVar._faithScore += 3;

				_curSearchObjId = getFirstObject();
				if (_curSearchObjId != 0) {
					_searchCount = 0;
					_is = 0;
					_heroSearching = true;
					_menu->setSearchMenu();
					prepareNextObject();
				} else
					_crep = 997;
			} else
				_crep = 187;
		} else {
			setCoordinates(8);
			_crep = 997;
			if (_num != 0) {
				if (_currBitIndex > 0)
					_coreVar._faithScore += 3;
				if ((_coreVar._currPlace != WELL) && (_coreVar._currPlace != SECRET_PASSAGE) && (_coreVar._currPlace != ATTIC)) {
					if (_coreVar._currPlace == PURPLE_ROOM) {
						_crep = 123;
						if (_coreVar._purpleRoomObjectId != 0)
							displayLookScreen(_coreVar._purpleRoomObjectId);
					}
					if (_coreVar._currPlace == CRYPT) {
						_crep = 123;
						if (_coreVar._cryptObjectId != 0)
							displayLookScreen(_coreVar._cryptObjectId);
					}
				}
			}
		}
	} else {
		if (_currBitIndex > 0)
			_coreVar._faithScore += 3;
		_crep = 997;
		if (_coreVar._currPlace < CELLAR)
			_crep = answerArr[_coreVar._currPlace];

		if ((_coreVar._currPlace == TOILETS) && (_num == 2))
			_crep = 162;

		if (_coreVar._currPlace == KITCHEN) {
			if ((_num == 3) || (_num == 4))
				_crep = 162;
			else if (_num == 5)
				_crep = 159;
		}

		if (_coreVar._currPlace == MOUNTAIN)
			_crep = 104;
		else if (_coreVar._currPlace == CRYPT)
			_crep = 155;
	}
}

/**
 * Engine function - Self / Search
 * @remarks	Originally called 'tsfouiller'
 */
void MortevielleEngine::fctSelfSearch() {
	if (_coreVar._selectedObjectId != 0)
		getSearchDescription(_coreVar._selectedObjectId);
	else
		_crep = 186;
}

/**
 * Engine function - Open
 * @remarks	Originally called 'touvrir'
 */
void MortevielleEngine::fctOpen() {
	if (!_syn)
		displayTextInVerbBar(getEngineString(S_OPEN));

	if (_caff == ROOM26) {
		if (_roomDoorId != OWN_ROOM) {
			_currAction = _menu->_opcodeEnter;
			_syn = true;
		} else
			_crep = 997;
		return;
	}

	if (_caff == LANDING) {
		showMoveMenuAlert();
		return;
	}

	displayStatusArrow();
	if ((_anyone) || (_keyPressedEsc))
		return;

	setCoordinates(7);
	if (_num != 0) {
		if (_currBitIndex > 0)
			_coreVar._faithScore += 2;
		++_openObjCount;
		int i;
		for (i = 1; (i <= 6); i++) {
			if ((_openObjects[i] == 0) || (_openObjects[i] == _num))
				break;
		}

		if (i > 6) {
			warning("Unexpected action: Too many open objects");
			return;
		}

		if (_openObjects[i] == _num)
			// display "Already Opened"
			_crep = 18;
		else {
			if (!( ((_num == 3) && ((_coreVar._currPlace == OWN_ROOM)
				                 || (_coreVar._currPlace == JULIA_ROOM)
								 || (_coreVar._currPlace == BLUE_ROOM)
								 || (_coreVar._currPlace == BATHROOM)))
			    || ((_num == 4) && ((_coreVar._currPlace == GREEN_ROOM)
				                 || (_coreVar._currPlace == PURPLE_ROOM)
								 || (_coreVar._currPlace == RED_ROOM)))
				|| ((_coreVar._currPlace == DARKBLUE_ROOM) && (_num == 5))
				|| ((_num == 6) && ((_coreVar._currPlace == BATHROOM)
				                 || (_coreVar._currPlace == DINING_ROOM)
								 || (_coreVar._currPlace == GREEN_ROOM2)
								 || (_coreVar._currPlace == ATTIC)))
				|| ((_coreVar._currPlace == GREEN_ROOM2) && (_num == 2))
				|| ((_coreVar._currPlace == KITCHEN) && (_num == 7))) ) {
				if ( ((_coreVar._currPlace > DINING_ROOM) && (_coreVar._currPlace < CELLAR))
				  || ((_coreVar._currPlace > RED_ROOM) && (_coreVar._currPlace < DINING_ROOM))
				  || (_coreVar._currPlace == OWN_ROOM)
				  || (_coreVar._currPlace == PURPLE_ROOM)
				  || (_coreVar._currPlace == BLUE_ROOM)) {
					if (getRandomNumber(1, 4) == 3)
						_soundManager->startSpeech(7, 9, 1);
				}
				_openObjects[i] = _num;
				displayAnimFrame(1, _num);
				_soundManager->waitSpeech();
			}
			int tmpPlace = _coreVar._currPlace;
			if (_coreVar._currPlace == CRYPT)
				tmpPlace = CELLAR;
			_crep = _tabdon[kAouvr + (tmpPlace * 7) + _num - 1];
			if (_crep == 254)
				_crep = 999;
		}
	}
}

/**
 * Engine function - Place
 * @remarks	Originally called 'tmettre'
 */
void MortevielleEngine::fctPlace() {
	if (_coreVar._selectedObjectId == 0) {
		_crep = 186;
		return;
	}

	if (!_syn)
		displayTextInVerbBar(getEngineString(S_PUT));

	displayStatusArrow();
	if (_keyPressedEsc)
		_crep = 998;

	if ((_anyone) || (_keyPressedEsc))
		return;

	setCoordinates(8);
	if (_num != 0) {
		_crep = 999;
		if (_caff == ATTIC) {
			if (_num == 1) {
				if (_coreVar._atticBallHoleObjectId != 0) {
					_crep = 188;
				} else {
					_coreVar._atticBallHoleObjectId = _coreVar._selectedObjectId;
					if (_coreVar._selectedObjectId == 141)
						displayAnimFrame(1, 7);
				}
			} else if (_coreVar._atticRodHoleObjectId != 0) {
				_crep = 188;
			} else {
				_coreVar._atticRodHoleObjectId = _coreVar._selectedObjectId;
				if (_coreVar._selectedObjectId == 159)
					displayAnimFrame(1, 6);
			}
		}

		if (_caff == CELLAR) {
			if (_coreVar._cellarObjectId != 0) {
				_crep = 188;
			} else {
				_coreVar._cellarObjectId = _coreVar._selectedObjectId;
				if (_coreVar._selectedObjectId == 151) {
					// Open hidden passage
					displayAnimFrame(1, 2);
					displayAnimFrame(1, 1);
					handleDescriptionText(2, 165);
					displayEmptyHand();
					_soundManager->startSpeech(6, -9, 1);

					// Do you want to enter the hidden passage?
					int answer = _dialogManager->show(getEngineString(S_YES_NO));
					if (answer == 1) {
						Common::String alertTxt = getString(582);
						_dialogManager->show(alertTxt);

						bool enterPassageFl = _dialogManager->showKnowledgeCheck();
						_mouse->hideMouse();
						clearScreen();
						drawRightFrame();
						clearDescriptionBar();
						clearVerbBar();
						_mouse->showMouse();
						prepareRoom();
						drawClock();
						if (_currBitIndex != 0)
							showPeoplePresent(_currBitIndex);
						else
							displayAloneText();

						_menu->displayMenu();
						if (enterPassageFl) {
							_coreVar._currPlace = SECRET_PASSAGE;
							_menu->setDestinationText(SECRET_PASSAGE);
						} else {
							_menu->setDestinationText(_coreVar._currPlace);
							setPal(14);
							drawPicture();
							displayAnimFrame(1, 2);
							displayAnimFrame(1, 1);
							alertTxt = getString(577);
							_dialogManager->show(alertTxt);
							displayAnimFrame(2, 1);
							_crep = 166;
						}
						prepareDisplayText();
					} else {
						displayAnimFrame(2, 1);
						_crep = 166;
					}
					return;
				}
			}
		}

		if (_caff == CRYPT) {
			if (_coreVar._cryptObjectId == 0)
				_coreVar._cryptObjectId = _coreVar._selectedObjectId;
			else
				_crep = 188;
		}

		if (_caff == SECRET_PASSAGE) {
			if (_coreVar._secretPassageObjectId != 0) {
				_crep = 188;
			} else if (_coreVar._selectedObjectId == 143) {
				_coreVar._secretPassageObjectId = 143;
				displayAnimFrame(1, 1);
			} else {
				_crep = 1512;
				loseGame();
			}
		}

		if (_caff == WELL) {
			if (_coreVar._wellObjectId != 0) {
				_crep = 188;
			} else if ((_coreVar._selectedObjectId == 140) || (_coreVar._selectedObjectId == 120)) {
				_coreVar._wellObjectId = _coreVar._selectedObjectId;
				displayAnimFrame(1, 1);
			} else {
				_crep = 185;
			}
		}

		if (_crep != 188)
			displayEmptyHand();
	}
}

/**
 * Engine function - Turn
 * @remarks	Originally called 'ttourner'
 */
void MortevielleEngine::fctTurn() {
	if (_caff > 99) {
		_crep = 149;
		return;
	}
	if (!_syn)
		displayTextInVerbBar(getEngineString(S_TURN));
	displayStatusArrow();
	if ((_anyone) || (_keyPressedEsc))
		return;
	setCoordinates(9);
	if (_num != 0) {
		_crep = 997;
		if ((_coreVar._currPlace == ATTIC) && (_coreVar._atticRodHoleObjectId == 159) && (_coreVar._atticBallHoleObjectId == 141)) {
			handleDescriptionText(2, 167);
			_soundManager->startSpeech(7, 9, 1);
			int answer = _dialogManager->show(getEngineString(S_YES_NO));
			if (answer == 1)
				_endGame = true;
			else
				_crep = 168;
		}
		if ((_coreVar._currPlace == SECRET_PASSAGE) && (_coreVar._secretPassageObjectId == 143)) {
			handleDescriptionText(2, 175);
			clearVerbBar();
			_soundManager->startSpeech(6, -9, 1);
			int answer = _dialogManager->show(getEngineString(S_YES_NO));
			if (answer == 1) {
				_coreVar._currPlace = CRYPT;
				prepareDisplayText();
			} else
				_crep = 176;
		}
	}
}

/**
 * Engine function - Hide Self
 * @remarks	Originally called 'tcacher'
 */
void MortevielleEngine::fctSelfHide() {
	if (!_syn)
		displayTextInVerbBar(getEngineString(S_HIDE_SELF));
	displayStatusArrow();
	if (!(_anyone) && !(_keyPressedEsc)) {
		setCoordinates(10);
		if (_num == 0)
			_hiddenHero = false;
		else {
			_hiddenHero = true;
			_crep = 999;
		}
	}
}

/**
 * Engine function - Attach
 * @remarks	Originally called 'tattacher'
 */
void MortevielleEngine::fctAttach() {
	if (_coreVar._selectedObjectId == 0)
		_crep = 186;
	else {
		if (!_syn)
			displayTextInVerbBar(getEngineString(S_TIE));
		displayStatusArrow();
		if (!(_anyone) && !(_keyPressedEsc)) {
			setCoordinates(8);
			_crep = 997;
			if ((_num != 0) && (_coreVar._currPlace == WELL)) {
				_crep = 999;
				if ((_coreVar._selectedObjectId == 120) || (_coreVar._selectedObjectId == 140)) {
					_coreVar._wellObjectId = _coreVar._selectedObjectId;
					displayAnimFrame(1, 1);
				} else
					_crep = 185;
				displayEmptyHand();
			}
		}
	}
}

/**
 * Engine function - Close
 * @remarks	Originally called 'tfermer'
 */
void MortevielleEngine::fctClose() {
	if (!_syn)
		displayTextInVerbBar(getEngineString(S_CLOSE));

	if (_caff < ROOM26) {
		displayStatusArrow();
		if (_keyPressedEsc)
			_crep = 998;
		if ((_anyone) || (_keyPressedEsc))
			return;
		setCoordinates(7);
		if (_num != 0) {
			int i;
			for (i = 1; i <= 6; ++i) {
				if (_num == _openObjects[i])
					break;
			}

			if (i <= 6) {
				displayAnimFrame(2, _num);
				_crep = 998;
				_openObjects[i] = 0;
				--_openObjCount;
				if (_openObjCount < 0)
					_openObjCount = 0;
				int objId = getFirstObject();
				if (_curSearchObjId == objId)
					_curSearchObjId = 0;
			} else {
				_crep = 187;
			}
		}
	}
	if (_caff == ROOM26)
		_crep = 999;
}

/**
 * Engine function - Knock
 * @remarks	Originally called 'tfrapper'
 */
void MortevielleEngine::fctKnock() {
	if (!_syn)
		displayTextInVerbBar(getEngineString(S_HIT));

	if (_coreVar._currPlace == LANDING) {
		_dialogManager->show(getEngineString(S_BEFORE_USE_DEP_MENU));
		return;
	}

	if (_coreVar._currPlace < DOOR) {
		displayStatusArrow();
		if (!(_anyone) && !(_keyPressedEsc)) {
			if ((_coreVar._currPlace < MOUNTAIN) && (_coreVar._currPlace != LANDING))
				_crep = 133;
			else
				_crep = 997;
		}

		return;
	}

	if (_coreVar._currPlace == ROOM26) {
		int rand = (getRandomNumber(0, 8)) - 4;
		_soundManager->startSpeech(11, rand, 1);
		int pres = getPresenceStats(rand, _coreVar._faithScore, _roomDoorId);
		if (_roomDoorId != OWN_ROOM) {
			if (pres != -500) {
				if (rand > pres)
					_crep = 190;
				else {
					setPresenceFlags(_roomDoorId);
					getKnockAnswer();
				}
			} else
				getKnockAnswer();
		}

		if (_roomDoorId == GREEN_ROOM2)
			_crep = 190;
	}
}

/**
 * Engine function - Self / Put
 * @remarks	Originally called 'tposer'
 */
void MortevielleEngine::fctSelfPut() {
	if (!_syn)
		displayTextInVerbBar(getEngineString(S_POSE));
	if (_coreVar._selectedObjectId == 0)
		_crep = 186;
	else {
		if (_caff > 99) {
			_crep = 999;
			putObject();
			if (_crep != 192)
				displayEmptyHand();
			return;
		}
		displayStatusArrow();
		if ((_anyone) || (_keyPressedEsc))
			return;
		setCoordinates(7);
		_crep = 124;
		if (_num != 0) {
			int objId = getFirstObject();
			if (objId == 0)
				_crep = 997;
			else {
				int i;
				for (i = 1; i <= 6; i++) {
					if (_num == _openObjects[i])
						break;
				}

				if (i <= 6) {
					_curSearchObjId = objId;
					_crep = 999;
				} else
					_crep = 187;
			}
		} else {
			setCoordinates(8);
			if (_num != 0) {
				_crep = 998;
				if (_caff == PURPLE_ROOM) {
					if (_coreVar._purpleRoomObjectId != 0)
						_crep = 188;
					else
						_coreVar._purpleRoomObjectId = _coreVar._selectedObjectId;
				}

				if (_caff == ATTIC) {
					if (_num == 1) {
						if (_coreVar._atticBallHoleObjectId != 0)
							_crep = 188;
						else {
							_coreVar._atticBallHoleObjectId = _coreVar._selectedObjectId;
							displayAnimFrame(1, 7);
						}
					} else if (_coreVar._atticRodHoleObjectId != 0) {
						_crep = 188;
					} else {
						_coreVar._atticRodHoleObjectId = _coreVar._selectedObjectId;
						displayAnimFrame(1, 6);
					}
				}

				if (_caff == CRYPT) {
					if (_coreVar._cryptObjectId != 0)
						_crep = 188;
					else
						_coreVar._cryptObjectId = _coreVar._selectedObjectId;
				}

				if (_caff == WELL)
					_crep = 185;
				if ((_caff == CELLAR) || (_caff == SECRET_PASSAGE))
					_crep = 124;
			} else {
				_crep = 124;
				if (_caff == WELL) {
					setCoordinates(5);
					if (_num != 0)
						_crep = 185;
				}
			}
		}
		if (_caff == INSIDE_WELL)
			_crep = 185;
		if ((_crep == 999) || (_crep == 185) || (_crep == 998)) {
			if (_crep == 999)
				putObject();
			if (_crep != 192)
				displayEmptyHand();
		}
	}
}

/**
 * Engine function - Listen
 * @remarks	Originally called 'tecouter'
 */
void MortevielleEngine::fctListen() {
	if (_coreVar._currPlace != ROOM26)
		_crep = 101;
	else {
		if (_currBitIndex != 0)
			++_coreVar._faithScore;
		int rand;
		int pres = getPresenceStats(rand, _coreVar._faithScore, _roomDoorId);
		if (_roomDoorId != OWN_ROOM) {
			if (pres != -500) {
				if (rand > pres)
					_crep = 101;
				else {
					setPresenceFlags(_roomDoorId);
					int day, hour, minute;
					updateHour(day, hour, minute);
					rand = getRandomNumber(1, 100);
					if ((hour >= 0) && (hour < 8)) {
						if (rand > 30)
							_crep = 101;
						else
							_crep = 178;
					} else if (rand > 70)
						_crep = 101;
					else
						_crep = 178;
				}
			} else
				_crep = 178;
		}
	}
}

/**
 * Engine function - Eat
 * @remarks	Originally called 'tmanger'
 */
void MortevielleEngine::fctEat() {
	if ((_coreVar._currPlace > LANDING) && (_coreVar._currPlace < ROOM26)) {
		_crep = 148;
	} else {
		exitRoom();
		_coreVar._currPlace = DINING_ROOM;
		_caff = DINING_ROOM;
		resetRoomVariables(_coreVar._currPlace);
		_menu->setDestinationText(_coreVar._currPlace);

		int day, hour, minute;
		updateHour(day, hour, minute);
		if ((hour == 12) || (hour == 13) || (hour == 19)) {
			_coreVar._faithScore -= (_coreVar._faithScore / 7);
			if (hour == 12) {
				if (minute == 0)
					hour = 4;
				else
					hour = 3;
			}

			if ((hour == 13) || (hour == 19)) {
				if (minute == 0)
					hour = 2;
				else
					hour = 1;
			}

			_currentHourCount += hour;
			_crep = 135;
			prepareRoom();
		} else {
			_crep = 134;
		}
	}
}

/**
 * Engine function - Enter
 * @remarks	Originally called 'tentrer'
 */
void MortevielleEngine::fctEnter() {
	if ((_coreVar._currPlace == MANOR_FRONT) || (_coreVar._currPlace == MANOR_BACK)) {
		gotoDiningRoom();
		_menu->setDestinationText(_coreVar._currPlace);
	} else if (_coreVar._currPlace == LANDING)
		showMoveMenuAlert();
	else if (_roomDoorId == OWN_ROOM)
		_crep = 997;
	else if ((_roomDoorId == JULIA_ROOM) && (_coreVar._selectedObjectId != 136)) {
		_crep = 189;
		_coreVar._availableQuestion[8] = '*';
	} else {
		int pres = 0;
		if (!_outsideOnlyFl)
			pres = getPresence(_roomDoorId);
		if (pres != 0) {
			if ((_roomDoorId == TOILETS) || (_roomDoorId == BATHROOM))
				_crep = 179;
			else {
				int randVal = (getRandomNumber(0, 10)) - 5;
				_soundManager->startSpeech(7, randVal, 1);
				displayAnimFrame(1, 1);
				_soundManager->waitSpeech();

				int charIndex = convertBitIndexToCharacterIndex(pres);
				++_coreVar._faithScore;
				_coreVar._currPlace = LANDING;
				_currMenu = MENU_DISCUSS;
				_currAction = (_menu->_discussMenu[charIndex]._menuId << 8) | _menu->_discussMenu[charIndex]._actionId;
				_syn = true;
				if (_roomDoorId == JULIA_ROOM) {
					_col = true;
					_caff = 70;
					drawPictureWithText();
					handleDescriptionText(2, _caff);
				} else
					_col = false;
				resetRoomVariables(_roomDoorId);
				_roomDoorId = OWN_ROOM;
			}
		} else {
			int randVal = (getRandomNumber(0, 10)) - 5;
			_soundManager->startSpeech(7, randVal, 1);
			displayAnimFrame(1, 1);
			_soundManager->waitSpeech();

			_coreVar._currPlace = _roomDoorId;
			prepareDisplayText();
			resetRoomVariables(_coreVar._currPlace);
			_menu->setDestinationText(_coreVar._currPlace);
			_roomDoorId = OWN_ROOM;
			_savedBitIndex = 0;
			_currBitIndex = 0;
		}
	}
}

/**
 * Engine function - Sleep
 * @remarks	Originally called 'tdormir'
 */
void MortevielleEngine::fctSleep() {
	if ((_coreVar._currPlace > LANDING) && (_coreVar._currPlace < ROOM26)) {
		_crep = 148;
		return;
	}
	if (_coreVar._currPlace != OWN_ROOM) {
		exitRoom();
		_coreVar._currPlace = OWN_ROOM;
		prepareDisplayText();
		drawPictureWithText();
		resetRoomVariables(_coreVar._currPlace);
		_menu->setDestinationText(_coreVar._currPlace);
	}
	clearVerbBar();
	clearDescriptionBar();
	prepareScreenType2();
	displayTextBlock(getEngineString(S_WANT_TO_WAKE_UP));
	int day, hour, minute;
	updateHour(day, hour, minute);

	int answer;
	do {
		if (hour < 8) {
			_coreVar._faithScore -= (_coreVar._faithScore / 20);
			int z = (7 - hour) * 2;
			if (minute == 30)
				--z;
			_currentHourCount += z;
			hour = 7;
		}
		_currentHourCount += 2;
		++hour;
		if (hour > 23)
			hour = 0;
		prepareRoom();
		answer = _dialogManager->show(getEngineString(S_YES_NO));
		_anyone = false;
	} while (answer != 1);
	_crep = 998;
	_num = 0;
}

/**
 * Engine function - Force
 * @remarks	Originally called 'tdefoncer'
 */
void MortevielleEngine::fctForce() {
	if (!_syn)
		displayTextInVerbBar(getEngineString(S_SMASH));
	if (_caff < DOOR)
		displayStatusArrow();

	if ((!_anyone) && (!_keyPressedEsc)) {
		if (_coreVar._currPlace != ROOM26)
			_crep = 997;
		else {
			_crep = 143;
			_coreVar._faithScore += 2;
		}
	}
}

/**
 * Engine function - Leave
 * @remarks	Originally called 'tsortir'
 */
void MortevielleEngine::fctLeave() {
	exitRoom();
	_crep = 0;
	if ((_coreVar._currPlace == MOUNTAIN) || (_coreVar._currPlace == MANOR_FRONT) || (_coreVar._currPlace == MANOR_BACK) || (_coreVar._currPlace == WELL))
		_crep = 997;
	else {
		int nextPlace = OWN_ROOM;

		if ((_coreVar._currPlace < CRYPT) || (_coreVar._currPlace == ROOM26))
			nextPlace = DINING_ROOM;
		else if ((_coreVar._currPlace == DINING_ROOM) || (_coreVar._currPlace == CHAPEL))
			nextPlace = MANOR_FRONT;
		else if ((_coreVar._currPlace < DINING_ROOM) || (_coreVar._currPlace == ATTIC))
			nextPlace = LANDING;
		else if (_coreVar._currPlace == CRYPT) {
			nextPlace = SECRET_PASSAGE;
			_crep = 176;
		} else if (_coreVar._currPlace == SECRET_PASSAGE)
			nextPlace = checkLeaveSecretPassage();
		else if (_coreVar._currPlace == INSIDE_WELL)
			nextPlace = WELL;

		if (_crep != 997)
			_coreVar._currPlace = nextPlace;

		_caff = nextPlace;
		if (_crep == 0)
			_crep = nextPlace;
		resetRoomVariables(nextPlace);
		_menu->setDestinationText(nextPlace);
	}
}

/**
 * Engine function - Wait
 * @remarks	Originally called 'tattendre'
 */
void MortevielleEngine::fctWait() {
	_savedBitIndex = 0;
	clearVerbBar();

	int answer;
	do {
		++_currentHourCount;
		prepareRoom();
		if (!_outsideOnlyFl)
			getPresence(_coreVar._currPlace);
		if ((_currBitIndex != 0) && (_savedBitIndex == 0)) {
			_crep = 998;
			if ((_coreVar._currPlace == ATTIC) || (_coreVar._currPlace == CELLAR))
				initCaveOrCellar();
			if ((_coreVar._currPlace > OWN_ROOM) && (_coreVar._currPlace < DINING_ROOM))
				_anyone = true;
			_savedBitIndex = _currBitIndex;
			if (!_anyone)
				prepareRoom();
			return;
		}
		handleDescriptionText(2, 102);
		answer = _dialogManager->show(getEngineString(S_YES_NO));
	} while (answer != 2);
	_crep = 998;
	if (!_anyone)
		prepareRoom();
}

/**
 * Engine function - Sound
 * @remarks	Originally called 'tsonder'
 */
void MortevielleEngine::fctSound() {
	if (!_syn)
		displayTextInVerbBar(getEngineString(S_PROBE2));
	if (_caff < 27) {
		displayStatusArrow();
		if (!(_anyone) && (!_keyPressedEsc))
			_crep = 145;
		_num = 0;
	}
}

/**
 * Engine function - Discuss
 * @remarks	Originally called 'tparler'
 */
void MortevielleEngine::fctDiscuss() {
	bool questionAsked[47];
	int cy, cx;
	int x, y;
	Common::String lib[47];

	int choice;
	int displId;

	endSearch();
	if (_col)
		displId = 128;
	else {
		cx = 0;
		int oldMenu;
		do {
			++cx;
			oldMenu = (_menu->_discussMenu[cx]._menuId << 8) | _menu->_discussMenu[cx]._actionId;
		} while (oldMenu != _currAction);
		_caff = 69 + cx;
		drawPictureWithText();
		handleDescriptionText(2, _caff);
		displId = _caff + 60;
	}
	testKey(false);
	menuUp();
	_mouse->hideMouse();
	clearScreen();
	drawDiscussionBox();
	startDialog(displId);
	clearScreen();
	for (int ix = 1; ix <= 46; ++ix)
		questionAsked[ix] = false;
	for (int ix = 1; ix <= 45; ++ix) {
		lib[ix] = getString(ix + kQuestionStringIndex);
		for (int i = lib[ix].size(); i <= 40; ++i)
			lib[ix] = lib[ix] + ' ';
	}
	lib[46] = lib[45];
	lib[45] = ' ';
	_mouse->showMouse();
	do {
		choice = 0;
		int posX = 0;
		int posY = 0;
		for (int icm = 1; icm < 43; icm++) {
			_screenSurface->putxy(posX, posY);
			if (_coreVar._availableQuestion[icm] == '*') {
				// If question already asked, write it in reverse video
				if (questionAsked[icm])
					displayQuestionText(lib[icm], 1);
				else
					displayQuestionText(lib[icm], 0);
			}

			if (icm == 23)  {
				posY = 0;
				posX = 320;
			} else
				posY += 8;
		}
		_screenSurface->putxy(320, 176);
		displayQuestionText(lib[46], 0);
		char retKey = '\0';
		bool click;
		do {
			bool dummyFl;
			_mouse->moveMouse(dummyFl, retKey);
			if (shouldQuit())
				return;

			_mouse->getMousePosition(x, y, click);
			x *= (3 - kResolutionScaler);
			if (x > 319)
				cx = 41;
			else
				cx = 1;
			cy = ((uint)y >> 3) + 1;      // 0-199 => 1-25
			if ((cy > 23) || ((cx == 41) && ((cy >= 20) && (cy <= 22)))) {
				if (choice != 0) {
					posY = ((choice - 1) % 23) << 3;
					if (choice > 23)
						posX = 320;
					else
						posX = 0;
					_screenSurface->putxy(posX, posY);
					if (questionAsked[choice])
						displayQuestionText(lib[choice], 0);
					else
						displayQuestionText(lib[choice], 1);
					questionAsked[choice] = !questionAsked[choice];
					choice = 0;
				}
			} else {
				int ix = cy;
				if (cx == 41)
					ix += 23;
				if (ix != choice) {
					if (choice != 0) {
						posY = ((choice - 1) % 23) << 3;
						if (choice > 23)
							posX = 320;
						else
							posX = 0;
						_screenSurface->putxy(posX, posY);
						if (questionAsked[choice])
							displayQuestionText(lib[choice], 0);
						else
							displayQuestionText(lib[choice], 1);
						questionAsked[choice] = ! questionAsked[choice];
					}
					if ((ix == 46) || (_coreVar._availableQuestion[ix] == '*')) {
						posY = ((ix - 1) % 23) << 3;
						if (ix > 23)
							posX = 320;
						else
							posX = 0;
						_screenSurface->putxy(posX, posY);
						if (questionAsked[ix])
							displayQuestionText(lib[ix], 0);
						else
							displayQuestionText(lib[ix], 1);
						questionAsked[ix] = ! questionAsked[ix];
						choice = ix;
					} else
						choice = 0;
				}
			}
		} while (!((retKey == '\15') || (((click != 0) || getMouseClick()) && (choice != 0))));
		setMouseClick(false);

		// If choice is not "End of Conversation"
		if (choice != 46) {
			int ix = choice - 1;
			if (_col) {
				_col = false;
				_coreVar._currPlace = 15;
				int maxRandVal;
				if (_openObjCount > 0)
					maxRandVal = 8;
				else
					maxRandVal = 4;
				if (getRandomNumber(1, maxRandVal) == 2)
					displId = 129;
				else {
					displId = 138;
					_coreVar._faithScore += (3 * (_coreVar._faithScore / 10));
				}
			} else if (_charAnswerCount[_caff - 69] < _charAnswerMax[_caff - 69]) {
				displId = _tabdon[kArep + (ix << 3) + (_caff - 70)];
				_coreVar._faithScore += _tabdon[kArcf + ix];
				++_charAnswerCount[_caff - 69];
			} else {
				_coreVar._faithScore += 3;
				displId = 139;
			}
			_mouse->hideMouse();
			clearScreen();
			drawDiscussionBox();
			startDialog(displId);
			_mouse->showMouse();
			if ((displId == 84) || (displId == 86)) {
				_coreVar._pctHintFound[5] = '*';
				_coreVar._availableQuestion[7] = '*';
			}
			if ((displId == 106) || (displId == 108) || (displId == 94)) {
				for (int indx = 29; indx <= 31; ++indx)
					_coreVar._availableQuestion[indx] = '*';
				_coreVar._pctHintFound[7] = '*';
			}
			if (displId == 70) {
				_coreVar._pctHintFound[8] = '*';
				_coreVar._availableQuestion[32] = '*';
			}
			_mouse->hideMouse();
			clearScreen();
			_mouse->showMouse();
		}
	} while ((choice != 46) && (displId != 138));
	if (_col) {
		_coreVar._faithScore += (3 * (_coreVar._faithScore / 10));
		_mouse->hideMouse();
		clearScreen();
		drawDiscussionBox();
		startDialog(138);
		_mouse->showMouse();
		_col = false;
		_coreVar._currPlace = LANDING;
	}
	_controlMenu = 0;
	_mouse->hideMouse();
	clearScreen();
	drawRightFrame();
	_mouse->showMouse();
	showPeoplePresent(_currBitIndex);
	prepareRoom();
	drawClock();
	prepareDisplayText();
	/* chech;*/
	_menu->setDestinationText(_coreVar._currPlace);
	clearVerbBar();
}

/**
 * Engine function - Smell
 * @remarks	Originally called 'tsentir'
 */
void MortevielleEngine::fctSmell() {
	_crep = 119;
	if (_caff < ROOM26) {
		if (!_syn)
			displayTextInVerbBar(getEngineString(S_SMELL));
		displayStatusArrow();
		if (!(_anyone) && !(_keyPressedEsc))
			if (_caff == CRYPT)
				_crep = 153;
	} else if (_caff == 123)
		_crep = 110;
	_num = 0;
}

/**
 * Engine function - Scratch
 * @remarks	Originally called 'tgratter'
 */
void MortevielleEngine::fctScratch() {
	_crep = 155;
	if (_caff < 27) {
		if (!_syn)
			displayTextInVerbBar(getEngineString(S_SCRATCH));
		displayStatusArrow();
	}
	_num = 0;
}

/**
 * The game is over
 * @remarks	Originally called 'tmaj1'
 */
void MortevielleEngine::endGame() {
	_quitGame = true;
	displayNarrativePicture(13, 152);
	displayEmptyHand();
	clearUpperLeftPart();
	clearDescriptionBar();
	clearVerbBar();
	handleDescriptionText(9, 1509);
	testKey(false);
	_mouse->hideMouse();
	_caff = 70;
	_text->taffich();
	clearScreen();
	drawDiscussionBox();
	startDialog(141);
	_mouse->showMouse();
	clearUpperLeftPart();
	handleDescriptionText(9, 1509);
	handleDescriptionText(2, 142);
	testKey(false);
	_caff = 32;
	drawPictureWithText();
	handleDescriptionText(6, 34);
	handleDescriptionText(2, 35);
	startMusicOrSpeech(0);
	testKey(false);
	displayInterScreenMessage(2036);
	testKey(false);
	resetVariables();
}

/**
 * You lost!
 * @remarks	Originally called 'tencore'
 */
void MortevielleEngine::askRestart() {
	clearDescriptionBar();
	startMusicOrSpeech(0);
	testKey(false);
	displayEmptyHand();
	resetVariables();
	initGame();
	_currHour = 10;
	_currHalfHour = 0;
	_currDay = 0;
	_minute = 0;
	_hour = 10;
	_day = 0;
	handleDescriptionText(2, 180);

	int answer = _dialogManager->show(getEngineString(S_YES_NO));
	_quitGame = (answer != 1);
}

} // End of namespace Mortevielle
