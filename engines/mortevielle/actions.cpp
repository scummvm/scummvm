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
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.	 See the
 * GNU General Public License for more details.

 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.
 *
 */

/*
 * This code is based on original Mortville Manor DOS source code
 * Copyright (c) 1987-1989 Lankhor
 */

#include "common/scummsys.h"
#include "mortevielle/dialogs.h"
#include "mortevielle/menu.h"
#include "mortevielle/mortevielle.h"
#include "mortevielle/mouse.h"
#include "mortevielle/outtext.h"
#include "mortevielle/speech.h"

namespace Mortevielle {

/**
 * Engine function - Move
 * @remarks	Originally called 'taller'
 */
void MortevielleEngine::fctMove() {
	if ((_coreVar._currPlace == ROOM26) && (_msg[4] == _menu._moveMenu[6])) {
		_coreVar._currPlace = LANDING;
		_caff = _coreVar._currPlace;
		drawPictureWithText();
		handleDescriptionText(2, _coreVar._currPlace);
	}
	if ((_coreVar._currPlace == LANDING) && (_msg[4] == _menu._moveMenu[6])) {
		if (!_syn)
			ecr3(getEngineString(S_GO_TO));
		tfleche();

		if (_keyPressedEsc)
			_destinationOk = false;

		if ((_anyone) || (_keyPressedEsc))
			return;

		setCoordinates(1);

		if (_num == 0)
			return;

		if (_num == 1) {
			_coreVar._currPlace = OWN_ROOM;
			_menu.setDestinationText(OWN_ROOM);
		} else if (_num == 7) {
			_coreVar._currPlace = ATTIC;
			_menu.setDestinationText(ATTIC);
		} else if (_num != 6)
			_coreVar._currPlace = ROOM26;

		if ((_num > 1) && (_num < 6))
			_roomDoorId = _num - 1;
		else if (_num > 7)
			_roomDoorId = _num - 3;

		if (_num != 6)
			affrep();
		else
			showMoveMenuAlert();
		return;
	}
	exitRoom();
	int menuChoice = 1;

	while (_menu._moveMenu[menuChoice] != _msg[4])
		++menuChoice;

	if (_coreVar._currPlace == MOUNTAIN) {
		if (menuChoice == 1)
			gotoManorFront();
		else if (menuChoice == 2)
			checkManorDistance();
		_menu.setDestinationText(_coreVar._currPlace);
		return;
	} else if (_coreVar._currPlace == INSIDE_WELL) {
		if (menuChoice == 1)
			floodedInWell();
		else if (menuChoice == 2)
			gotoManorBack();
		_menu.setDestinationText(_coreVar._currPlace);
		return;
	} else if ((_coreVar._currPlace == BUREAU) && (menuChoice == 1))
		menuChoice = 6;
	else if (_coreVar._currPlace == KITCHEN) {
		if (menuChoice == 2)
			menuChoice = 6;
		else if (menuChoice == 5)
			menuChoice = 16;
	} else if ((_coreVar._currPlace == CELLAR) && (menuChoice == 3))
		menuChoice = 6;
	else if (((_coreVar._currPlace == LANDING) || (_coreVar._currPlace == ROOM26)) && (menuChoice == 4))
		menuChoice = 6;

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

	if (menuChoice == 1)
		_coreVar._currPlace = BUREAU;
	else if (menuChoice == 2)
		_coreVar._currPlace = KITCHEN;
	else if (menuChoice == 3)
		_coreVar._currPlace = CELLAR;
	else if (menuChoice == 4)
		_coreVar._currPlace = LANDING;
	else if (menuChoice == 5)
		menuChoice = 12;
	else if (menuChoice == 6)
		menuChoice = 11;

	if (menuChoice == 11)
		gotoDiningRoom();
	else if (menuChoice == 12)
		gotoManorFront();
	else if (menuChoice == 13)
		_coreVar._currPlace = CHAPEL;
	else if (menuChoice == 14)
		_coreVar._currPlace = WELL;
	else if (menuChoice == 15)
		checkManorDistance();
	else if (menuChoice == 16)
		gotoManorBack();
	else if (menuChoice == 17) {
		if ((_coreVar._wellObjectId != 120) && (_coreVar._wellObjectId != 140))
			_crep = 997;
		else if (_coreVar._wellObjectId == 120)
			_crep = 181;
		else if (_coreVar._faithScore > 80) {
			_crep = 1505;
			loseGame();
		} else {
			_coreVar._currPlace = INSIDE_WELL;
			affrep();
		}
	}
	if ((menuChoice < 5) || (menuChoice == 13) || (menuChoice == 14))
		affrep();
	resetRoomVariables(_coreVar._currPlace);
	_menu.setDestinationText(_coreVar._currPlace);
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
				if (_coreVar._currPlace == PURPLE_ROOM)
					_coreVar._purpleRoomObjectId = 0;
				if (_coreVar._currPlace == ATTIC) {
					if (_coreVar._atticBallHoleObjectId == _caff)
						_coreVar._atticBallHoleObjectId = 0;
					if (_coreVar._atticRodHoleObjectId == _caff)
						_coreVar._atticRodHoleObjectId = 0;
				}
				if (_coreVar._currPlace == CELLAR)
					_coreVar._cellarObjectId = 0;
				if (_coreVar._currPlace == CRYPT)
					_coreVar._cryptObjectId = 0;
				if (_coreVar._currPlace == SECRET_PASSAGE)
					_coreVar._secretPassageObjectId = 0;
				if (_coreVar._currPlace == WELL)
					_coreVar._wellObjectId = 0;
				_menu.unsetSearchMenu();
				_obpart = false;
				affrep();
			} else {
				_tabdon[kAcha + ((_mchai - 1) * 10) + _searchCount - 1] = 0;
				tsuiv();
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
		ecr3(getEngineString(S_TAKE));
	tfleche();
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
			if ((_coreVar._currPlace == PURPLE_ROOM) && (_coreVar._purpleRoomObjectId != 0))
				putInHand(_coreVar._purpleRoomObjectId);
			if ((_coreVar._currPlace == ATTIC) && (_num == 1) && (_coreVar._atticBallHoleObjectId != 0)) {
				putInHand(_coreVar._atticBallHoleObjectId);
				if ((_crep != 997) && (_crep != 139))
					aniof(2, 7);
			}
			if ((_coreVar._currPlace == ATTIC) && (_num == 2) && (_coreVar._atticRodHoleObjectId != 0)) {
				putInHand(_coreVar._atticRodHoleObjectId);
				if ((_crep != 997) && (_crep != 139))
					aniof(2, 6);
			}
			if ((_coreVar._currPlace == CELLAR) && (_coreVar._cellarObjectId != 0)) {
				putInHand(_coreVar._cellarObjectId);
				if ((_crep != 997) && (_crep != 139))
					aniof(2, 2);
			}
			if ((_coreVar._currPlace == CRYPT) && (_coreVar._cryptObjectId != 0))
				putInHand(_coreVar._cryptObjectId);

			if ((_coreVar._currPlace == SECRET_PASSAGE) && (_coreVar._secretPassageObjectId != 0)) {
				putInHand(_coreVar._secretPassageObjectId);
				if ((_crep != 997) && (_crep != 139)) {
					_crep = 182;
					aniof(2, 1);
				}
			}
			if ((_coreVar._currPlace == WELL) && (_coreVar._wellObjectId != 0)) {
				putInHand(_coreVar._wellObjectId);
				if ((_crep != 997) && (_crep != 139))
					aniof(2, 1);
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
		  || ((_coreVar._currPlace == ROOM9) && (_num == 4))
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
	do {
		++inventIndex;
	} while (_menu._inventoryMenu[inventIndex] != _msg[4]);
	int cz = 0;
	int cy = 0;
	do {
		++cy;
		if (_coreVar._inventory[cy] != 0)
			++cz;
	} while (cz != inventIndex);
	cz = _coreVar._inventory[cy];
	_coreVar._inventory[cy] = 0;
	_menu.setInventoryText();
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
		ecr3(getEngineString(S_LIFT));
	tfleche();
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
				treg(_coreVar._purpleRoomObjectId);
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
			ecr3(getEngineString(S_READ));
		tfleche();
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
	int cx;

	if (_caff > 99) {
		_crep = 103;
		return;
	}
	if (!_syn)
		ecr3(getEngineString(S_LOOK));
	tfleche();
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
						treg(_coreVar._atticRodHoleObjectId);
					else if (_coreVar._atticBallHoleObjectId != 0)
						treg(_coreVar._atticBallHoleObjectId);
				} else {
					_crep = 193;
					if (_coreVar._atticRodHoleObjectId != 0)
						treg(_coreVar._atticRodHoleObjectId);
				}
			}
			if (_coreVar._currPlace == CELLAR) {
				_crep = 164;
				if (_coreVar._cellarObjectId != 0)
					treg(_coreVar._cellarObjectId);
			}
			if (_coreVar._currPlace == SECRET_PASSAGE) {
				_crep = 174;
				if (_coreVar._secretPassageObjectId != 0)
					treg(_coreVar._secretPassageObjectId);
			}
			if (_coreVar._currPlace == WELL) {
				_crep = 131;
				if (_coreVar._wellObjectId != 0)
					treg(_coreVar._wellObjectId);
			}
		}
		return;
	}
	cx = _coreVar._currPlace;
	if (_coreVar._currPlace == CHAPEL)
		cx = 17;
	if ((_coreVar._currPlace > MANOR_FRONT) && (_coreVar._currPlace < DOOR))
		cx -= 4;
	if (_coreVar._currPlace == ROOM26)
		cx = 21;
	_crep = _tabdon[kArega + (cx * 7) + _num - 1];
	if ((_coreVar._currPlace == ATTIC) && (_num == 8))
		_crep = 126;
	if (_coreVar._currPlace == MOUNTAIN)
		_crep = 103;
	if (_crep == 255)
		_crep = 131;
	if ((_coreVar._currPlace == GREEN_ROOM) && (_num == 1))
		treg(144);
	if ((_coreVar._currPlace == BLUE_ROOM) && (_num == 3))
		treg(147);
	if ((_coreVar._currPlace == GREEN_ROOM2) && (_num == 3))
		treg(149);
	if ((_coreVar._currPlace == ROOM9) && (_num == 2))
		treg(30);
	if ((_coreVar._currPlace == DINING_ROOM) && (_num == 3))
		treg(31);
}

/**
 * Engine function - Self / Look
 * @remarks	Originally called 'tsregarder'
 */
void MortevielleEngine::fctSelftLook() {
	if (_coreVar._selectedObjectId != 0)
		treg(_coreVar._selectedObjectId);
	else
		_crep = 186;
}

/**
 * Engine function - Search
 * @remarks	Originally called 'tfouiller'
 */
void MortevielleEngine::fctSearch() {
	const byte r[14] = {123, 104, 123, 131, 131, 123, 104, 131, 123, 123, 106, 123, 123, 107};

	if (_caff > 99) {
		getSearchDescription(_caff);
		return;
	}

	if (!_syn)
		ecr3(getEngineString(S_SEARCH));

	tfleche();
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
			int cx = 0;
			do {
				++cx;
			} while ((cx <= 6) && (_num != _openObjects[cx]));
			if (_num != _openObjects[cx])
				_crep = 187;
			else {
				if (_currBitIndex > 0)
					_coreVar._faithScore += 3;

				rechai(_mchai);
				if (_mchai != 0) {
					_searchCount = 0;
					_heroSearching = true;
					_menu.setSearchMenu();
					tsuiv();
				} else
					_crep = 997;
			}
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
							treg(_coreVar._purpleRoomObjectId);
					}
					if (_coreVar._currPlace == CRYPT) {
						_crep = 123;
						if (_coreVar._cryptObjectId != 0)
							treg(_coreVar._cryptObjectId);
					}
				}
			}
		}
	} else {
		if (_currBitIndex > 0)
			_coreVar._faithScore += 3;
		_crep = 997;
		if (_coreVar._currPlace < CELLAR)
			_crep = r[_coreVar._currPlace];

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
		ecr3(getEngineString(S_OPEN));

	if (_caff == ROOM26) {
		if (_roomDoorId != OWN_ROOM) {
			_msg[4] = OPCODE_ENTER;
			_syn = true;
		} else
			_crep = 997;
		return;
	}

	if (_caff == LANDING) {
		showMoveMenuAlert();
		return;
	}

	tfleche();
	if ((_anyone) || (_keyPressedEsc))
		return;

	setCoordinates(7);
	if (_num != 0) {
		if (_currBitIndex > 0)
			_coreVar._faithScore += 2;
		++_openObjCount;
		int tmpPlace = 0;
		do {
			++tmpPlace;
		} while (!((tmpPlace > 6) || (_openObjects[tmpPlace] == 0) || (_openObjects[tmpPlace] == _num)));
		if (_openObjects[tmpPlace] != _num) {
			if (!( ((_num == 3) && ((_coreVar._currPlace == OWN_ROOM)
				                 || (_coreVar._currPlace == ROOM9) 
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
						_speechManager.startSpeech(7, 9, 1);
				}
				_openObjects[tmpPlace] = _num;
				aniof(1, _num);
			}
			tmpPlace = _coreVar._currPlace;
			if (_coreVar._currPlace == CRYPT)
				tmpPlace = CELLAR;
			_crep = _tabdon[kAouvr + (tmpPlace * 7) + _num - 1];
			if (_crep == 254)
				_crep = 999;
		} else
			// display "Already Opened"
			_crep = 18;
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
		ecr3(getEngineString(S_PUT));

	tfleche();
	if (_keyPressedEsc)
		_crep = 998;

	if ((_anyone) || (_keyPressedEsc))
		return;

	setCoordinates(8);
	if (_num != 0) {
		_crep = 999;
		if (_caff == 13) {
			if (_num == 1) {
				if (_coreVar._atticBallHoleObjectId != 0) {
					_crep = 188;
				} else {
					_coreVar._atticBallHoleObjectId = _coreVar._selectedObjectId;
					if (_coreVar._selectedObjectId == 141)
						aniof(1, 7);
				}
			} else if (_coreVar._atticRodHoleObjectId != 0) {
				_crep = 188;
			} else {
				_coreVar._atticRodHoleObjectId = _coreVar._selectedObjectId;
				if (_coreVar._selectedObjectId == 159)
					aniof(1, 6);
			}
		}

		if (_caff == 14) {
			if (_coreVar._cellarObjectId != 0) {
				_crep = 188;
			} else {
				_coreVar._cellarObjectId = _coreVar._selectedObjectId;
				if (_coreVar._selectedObjectId == 151) {
					// Open hidden passage
					aniof(1, 2);
					aniof(1, 1);
					handleDescriptionText(2, 165);
					displayEmptyHand();
					_speechManager.startSpeech(6, -9, 1);

					// Do you want to enter the hidden passage?
					int answer = Alert::show(getEngineString(S_YES_NO), 1);
					if (answer == 1) {
						Common::String alertTxt = getString(582);
						Alert::show(alertTxt, 1);

						bool enterPassageFl = KnowledgeCheck::show();
						_mouse.hideMouse();
						hirs();
						drawRightFrame();
						clearDescriptionBar();
						clearVerbBar();
						_mouse.showMouse();
						prepareRoom();
						drawClock();
						if (_currBitIndex != 0)
							showPeoplePresent(_currBitIndex);
						else
							displayAloneText();

						_menu.displayMenu();
						if (enterPassageFl) {
							_coreVar._currPlace = SECRET_PASSAGE;
							_menu.setDestinationText(SECRET_PASSAGE);
						} else {
							_menu.setDestinationText(_coreVar._currPlace);
							setPal(14);
							drawPicture();
							aniof(1, 2);
							aniof(1, 1);
							alertTxt = getString(577);
							Alert::show(alertTxt, 1);
							aniof(2, 1);
							_crep = 166;
						}
						affrep();
					} else {
						aniof(2, 1);
						_crep = 166;
					}
					return;
				}
			}
		}

		if (_caff == 16) {
			if (_coreVar._cryptObjectId == 0)
				_coreVar._cryptObjectId = _coreVar._selectedObjectId;
			else
				_crep = 188;
		}

		if (_caff == 17) {
			if (_coreVar._secretPassageObjectId != 0) {
				_crep = 188;
			} else if (_coreVar._selectedObjectId == 143) {
				_coreVar._secretPassageObjectId = 143;
				aniof(1, 1);
			} else {
				_crep = 1512;
				loseGame();
			}
		}

		if (_caff == 24) {
			if (_coreVar._wellObjectId != 0) {
				_crep = 188;
			} else if ((_coreVar._selectedObjectId == 140) || (_coreVar._selectedObjectId == 120)) {
				_coreVar._wellObjectId = _coreVar._selectedObjectId;
				aniof(1, 1);
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
		ecr3(getEngineString(S_TURN));
	tfleche();
	if ((_anyone) || (_keyPressedEsc))
		return;
	setCoordinates(9);
	if (_num != 0) {
		_crep = 997;
		if ((_coreVar._currPlace == ATTIC) && (_coreVar._atticRodHoleObjectId == 159) && (_coreVar._atticBallHoleObjectId == 141)) {
			handleDescriptionText(2, 167);
			_speechManager.startSpeech(7, 9, 1);
			int answer = Alert::show(getEngineString(S_YES_NO), 1);
			if (answer == 1)
				_endGame = true;
			else
				_crep = 168;
		}
		if ((_coreVar._currPlace == SECRET_PASSAGE) && (_coreVar._secretPassageObjectId == 143)) {
			handleDescriptionText(2, 175);
			clearVerbBar();
			_speechManager.startSpeech(6, -9, 1);
			int answer = Alert::show(getEngineString(S_YES_NO), 1);
			if (answer == 1) {
				_coreVar._currPlace = CRYPT;
				affrep();
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
		ecr3(getEngineString(S_HIDE_SELF));
	tfleche();
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
			ecr3(getEngineString(S_TIE));
		tfleche();
		if (!(_anyone) && !(_keyPressedEsc)) {
			setCoordinates(8);
			_crep = 997;
			if ((_num != 0) && (_coreVar._currPlace == WELL)) {
				_crep = 999;
				if ((_coreVar._selectedObjectId == 120) || (_coreVar._selectedObjectId == 140)) {
					_coreVar._wellObjectId = _coreVar._selectedObjectId;
					aniof(1, 1);
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
		ecr3(getEngineString(S_CLOSE));

	if (_caff < ROOM26) {
		tfleche();
		if (_keyPressedEsc)
			_crep = 998;
		if ((_anyone) || (_keyPressedEsc))
			return;
		setCoordinates(7);
		if (_num != 0) {
			int cx = 0;
			do {
				++cx;
			} while ((cx <= 6) && (_num != _openObjects[cx]));
			if (_num == _openObjects[cx]) {
				aniof(2, _num);
				_crep = 998;
				_openObjects[cx] = 0;
				--_openObjCount;
				if (_openObjCount < 0)
					_openObjCount = 0;
				int chai = 9999;
				rechai(chai);
				if (_mchai == chai)
					_mchai = 0;
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
		ecr3(getEngineString(S_HIT));

	if (_coreVar._currPlace == LANDING) {
		Alert::show(getEngineString(S_BEFORE_USE_DEP_MENU), 1);
		return;
	}

	if (_coreVar._currPlace < DOOR) {
		tfleche();
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
		_speechManager.startSpeech(11, rand, 1);
		int p = getPresenceStats(rand, _coreVar._faithScore, _roomDoorId);
		int l = _roomDoorId;
		if (l != OWN_ROOM) {
			if (p != -500) {
				if (rand > p)
					_crep = 190;
				else {
					setPresenceFlags(l);
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
		ecr3(getEngineString(S_POSE));
	if (_coreVar._selectedObjectId == 0)
		_crep = 186;
	else {
		if (_caff > 99) {
			_crep = 999;
			ajchai();
			if (_crep != 192)
				displayEmptyHand();
			return;
		}
		tfleche();
		if ((_anyone) || (_keyPressedEsc))
			return;
		setCoordinates(7);
		_crep = 124;
		if (_num != 0) {
			int chai;
			rechai(chai);
			if (chai == 0)
				_crep = 997;
			else {
				int cx = 0;
				do {
					++cx;
				} while ((cx <= 6) && (_num != _openObjects[cx]));
				if (_num != _openObjects[cx])
					_crep = 187;
				else {
					_mchai = chai;
					_crep = 999;
				}
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
						else
							_coreVar._atticBallHoleObjectId = _coreVar._selectedObjectId;
					} else if (_coreVar._atticRodHoleObjectId != 0) {
						_crep = 188;
					} else {
						_coreVar._atticRodHoleObjectId = _coreVar._selectedObjectId;
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
				ajchai();
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
		int p = getPresenceStats(rand, _coreVar._faithScore, _roomDoorId);
		int l = _roomDoorId;
		if (l != OWN_ROOM) {
			if (p != -500) {
				if (rand > p)
					_crep = 101;
				else {
					setPresenceFlags(l);
					int j, h, m;
					updateHour(j, h, m);
					rand = getRandomNumber(1, 100);
					if ((h >= 0) && (h < 8)) {
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
		_caff = 10;
		resetRoomVariables(_coreVar._currPlace);
		_menu.setDestinationText(_coreVar._currPlace);

		int j, h, m;
		updateHour(j, h, m);
		if ((h == 12) || (h == 13) || (h == 19)) {
			_coreVar._faithScore -= (_coreVar._faithScore / 7);
			if (h == 12) {
				if (m == 0)
					h = 4;
				else
					h = 3;
			}

			if ((h == 13) || (h == 19)) {
				if (m == 0)
					h = 2;
				else
					h = 1;
			}

			_currentHourCount += h;
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
		_menu.setDestinationText(_coreVar._currPlace);
	} else if (_coreVar._currPlace == LANDING)
		showMoveMenuAlert();
	else if (_roomDoorId == OWN_ROOM)
		_crep = 997;
	else if ((_roomDoorId == ROOM9) && (_coreVar._selectedObjectId != 136)) {
			_crep = 189;
			_coreVar._teauto[8] = '*';
	} else {
		int z = 0;
		if (!_blo)
			z = getPresence(_roomDoorId);
		if (z != 0) {
			if ((_roomDoorId == TOILETS) || (_roomDoorId == BATHROOM))
				_crep = 179;
			else {
				int randVal = (getRandomNumber(0, 10)) - 5;
				_speechManager.startSpeech(7, randVal, 1);
				aniof(1, 1);

				int charIndex = convertBitIndexToCharacterIndex(z);
				++_coreVar._faithScore;
				_coreVar._currPlace = LANDING;
				_msg[3] = MENU_DISCUSS;
				_msg[4] = _menu._discussMenu[charIndex];
				_syn = true;
				if (_roomDoorId == ROOM9) {
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
			_speechManager.startSpeech(7, randVal, 1);
			aniof(1, 1);

			_coreVar._currPlace = _roomDoorId;
			affrep();
			resetRoomVariables(_coreVar._currPlace);
			_menu.setDestinationText(_coreVar._currPlace);
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
	int z, j, h, m;

	if ((_coreVar._currPlace > LANDING) && (_coreVar._currPlace < ROOM26)) {
		_crep = 148;
		return;
	}
	if (_coreVar._currPlace != OWN_ROOM) {
		exitRoom();
		_coreVar._currPlace = OWN_ROOM;
		affrep();
		drawPictureWithText();
		resetRoomVariables(_coreVar._currPlace);
		_menu.setDestinationText(_coreVar._currPlace);
	}
	clearVerbBar();
	clearDescriptionBar();
	prepareScreenType2();
	ecr2(getEngineString(S_WANT_TO_WAKE_UP));
	updateHour(j, h, m);

	int answer;
	do {
		if (h < 8) {
			_coreVar._faithScore -= (_coreVar._faithScore / 20);
			z = (7 - h) * 2;
			if (m == 30)
				--z;
			_currentHourCount += z;
			h = 7;
		}
		_currentHourCount += 2;
		++h;
		if (h > 23)
			h = 0;
		prepareRoom();
		answer = Alert::show(getEngineString(S_YES_NO), 1);
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
		ecr3(getEngineString(S_SMASH));
	if (_caff < 25)
		tfleche();

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
		_menu.setDestinationText(nextPlace);
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
		if (!_blo)
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
		answer = Alert::show(getEngineString(S_YES_NO), 1);
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
		ecr3(getEngineString(S_PROBE2));
	if (_caff < 27) {
		tfleche();
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
//	int c;
	Common::String lib[47];

	int choice; 
	int displId;

	endSearch();
	if (_col)
		displId = 128;
	else {
		cx = 0;
		do {
			++cx;
		} while (_menu._discussMenu[cx] != _msg[4]);
		_caff = 69 + cx;
		drawPictureWithText();
		handleDescriptionText(2, _caff);
		displId = _caff + 60;
	}
	testKey(false);
	mennor();
	_mouse.hideMouse();
	hirs();
	premtet();
	startDialog(displId);
	hirs();
	for (int ix = 1; ix <= 46; ++ix)
		questionAsked[ix] = false;
	for (int ix = 1; ix <= 45; ++ix) {
		lib[ix] = getString(ix + kQuestionStringIndex);
		for (int i = lib[ix].size(); i <= 40; ++i)
			lib[ix] = lib[ix] + ' ';
	}
	lib[46] = lib[45];
	lib[45] = ' ';
	_mouse.showMouse();
	do {
		choice = 0;
		int posX = 0;
		int posY = 0;
		for (int icm = 1; icm < 43; icm++) {
			_screenSurface.putxy(posX, posY);
			if (_coreVar._teauto[icm] == '*') {
				// If question already asked, write it in reverse video
				if (questionAsked[icm])
					writetp(lib[icm], 1);
				else
					writetp(lib[icm], 0);
			}

			if (icm == 23)  {
				posY = 0;
				posX = 320;
			} else
				posY += 8;
		}
		_screenSurface.putxy(320, 176);
		writetp(lib[46], 0);
		char retKey = '\0';
		bool click;
		do {
			bool dummyFl;
			_mouse.moveMouse(dummyFl, retKey);
			CHECK_QUIT;

			_mouse.getMousePosition(x, y, click);
			x *= (3 - _res);
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
					_screenSurface.putxy(posX, posY);
					if (questionAsked[choice])
						writetp(lib[choice], 0);
					else
						writetp(lib[choice], 1);
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
						_screenSurface.putxy(posX, posY);
						if (questionAsked[choice])
							writetp(lib[choice], 0);
						else
							writetp(lib[choice], 1);
						questionAsked[choice] = ! questionAsked[choice];
					}
					if ((_coreVar._teauto[ix] == '*') || (ix == 46)) {
						posY = ((ix - 1) % 23) << 3;
						if (ix > 23)
							posX = 320;
						else
							posX = 0;
						_screenSurface.putxy(posX, posY);
						if (questionAsked[ix])
							writetp(lib[ix], 0);
						else
							writetp(lib[ix], 1);
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
			} else if (_nbrep[_caff - 69] < _nbrepm[_caff - 69]) {
				displId = _tabdon[kArep + (ix << 3) + (_caff - 70)];
				_coreVar._faithScore += _tabdon[kArcf + ix];
				++_nbrep[_caff - 69];
			} else {
				_coreVar._faithScore += 3;
				displId = 139;
			}
			_mouse.hideMouse();
			hirs();
			premtet();
			startDialog(displId);
			_mouse.showMouse();
			if ((displId == 84) || (displId == 86)) {
				_coreVar._pourc[5] = '*';
				_coreVar._teauto[7] = '*';
			}
			if ((displId == 106) || (displId == 108) || (displId == 94)) {
				for (int indx = 29; indx <= 31; ++indx)
					_coreVar._teauto[indx] = '*';
				_coreVar._pourc[7] = '*';
			}
			if (displId == 70) {
				_coreVar._pourc[8] = '*';
				_coreVar._teauto[32] = '*';
			}
			_mouse.hideMouse();
			hirs();
			_mouse.showMouse();
		}
	} while ((choice != 46) && (displId != 138));
	if (_col) {
		_coreVar._faithScore += (3 * (_coreVar._faithScore / 10));
		_mouse.hideMouse();
		hirs();
		premtet();
		startDialog(138);
		_mouse.showMouse();
		_col = false;
		_coreVar._currPlace = LANDING;
	}
	_controlMenu = 0;
	_mouse.hideMouse();
	hirs();
	drawRightFrame();
	_mouse.showMouse();
	showPeoplePresent(_currBitIndex);
	prepareRoom();
	drawClock();
	affrep();
	/* chech;*/
	_menu.setDestinationText(_coreVar._currPlace);
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
			ecr3(getEngineString(S_SMELL));
		tfleche();
		if (!(_anyone) && !(_keyPressedEsc))
			if (_caff == 16)
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
			ecr3(getEngineString(S_SCRATCH));
		tfleche();
	}
	_num = 0;
}

/**
 * The game is over
 * @remarks	Originally called 'tmaj1'
 */
void MortevielleEngine::endGame() {
	_quitGame = true;
	tlu(13, 152);
	displayEmptyHand();
	clearUpperLeftPart();
	clearDescriptionBar();
	clearVerbBar();
	handleDescriptionText(9, 1509);
	testKey(false);
	_mouse.hideMouse();
	_caff = 70;
	_text.taffich();
	hirs();
	premtet();
	startDialog(141);
	_mouse.showMouse();
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
	// A wait message was displayed.
	// testKey (aka tkey1) was called before and after.
	// Most likely the double call is useless, thus removed
	//
	// testKey(false);
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

	int answer = Alert::show(getEngineString(S_YES_NO), 1);
	_quitGame = (answer != 1);
}

} // End of namespace Mortevielle
