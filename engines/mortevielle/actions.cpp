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
 * Copyright (c) 1988-1989 Lankhor
 */

#include "common/scummsys.h"
#include "mortevielle/dialogs.h"
#include "mortevielle/menu.h"
#include "mortevielle/mor.h"
#include "mortevielle/mortevielle.h"
#include "mortevielle/mouse.h"
#include "mortevielle/outtext.h"
#include "mortevielle/speech.h"
#include "mortevielle/var_mor.h"

namespace Mortevielle {

/**
 * Engine function - Move
 * @remarks	Originally called 'taller'
 */
void MortevielleEngine::fctMove() {
	if ((_coreVar._currPlace == ROOM26) && (_msg[4] == _menu._moveMenu[6])) {
		_coreVar._currPlace = LANDING;
		_caff = _coreVar._currPlace;
		afdes();
		repon(2, _coreVar._currPlace);
	}
	if ((_coreVar._currPlace == LANDING) && (_msg[4] == _menu._moveMenu[6])) {
		if (!_syn)
			ecr3(getEngineString(S_GO_TO));
		tfleche();

		if (_keyPressedEsc)
			_okdes = false;

		if ((_anyone) || (_keyPressedEsc))
			return;

		tcoord(1);

		if (_num == 0)
			return;

		if (_num == 1) {
			_coreVar._currPlace = OWN_ROOM;
			_menu.setDestinationMenuText(OWN_ROOM);
		} else if (_num == 7) {
			_coreVar._currPlace = ATTIC;
			_menu.setDestinationMenuText(ATTIC);
		}
		
		if (_num != 6)
			_coreVar._currPlace = ROOM26;
		
		if ((_num > 1) && (_num < 6))
			_ment = _num - 1;
		else if (_num > 7)
			_ment = _num - 3;

		if (_num != 6)
			affrep();
		else
			showMoveMenuAlert();
		return;
	}
	tsort();
	int menuChoice = 1;

	while (_menu._moveMenu[menuChoice] != _msg[4])
		++menuChoice;

	if (_coreVar._currPlace == MOUNTAIN) {
		if (menuChoice == 1)
			gotoManorFront();
		else if (menuChoice == 2)
			checkManorDistance();
		_menu.setDestinationMenuText(_coreVar._currPlace);
		return;
	} else if (_coreVar._currPlace == INSIDE_WELL) {
		if (menuChoice == 1)
			floodedInWell();
		else if (menuChoice == 2)
			gotoManorBack();
		_menu.setDestinationMenuText(_coreVar._currPlace);
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
	_menu.setDestinationMenuText(_coreVar._currPlace);
}

/**
 * Engine function - Take
 * @remarks	Originally called 'tprendre'
 */
void MortevielleEngine::fctTake() {
	if (_caff > 99) {
		int cx = _caff;
		avpoing(cx);
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
				_tabdon[kAcha + ((_mchai - 1) * 10) + _cs - 1] = 0;
				tsuiv();
				++_dobj;
				if (_dobj > 6) {
					_coreVar._faithScore += 2;
					_dobj = 0;
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
		tcoord(2);
		if (_num == 1) {
			_crep = 152;
			return;
		}
	}
	tcoord(5);
	if ((_num == 0) || ((_num == 1) && (_coreVar._currPlace == CRYPT))) {
		tcoord(8);
		if (_num != 0) {
			if (_currBitIndex > 0)
				_coreVar._faithScore += 3;
			_crep = 997;
			if ((_coreVar._currPlace == PURPLE_ROOM) && (_coreVar._purpleRoomObjectId != 0))
				avpoing(_coreVar._purpleRoomObjectId);
			if ((_coreVar._currPlace == ATTIC) && (_num == 1) && (_coreVar._atticBallHoleObjectId != 0)) {
				avpoing(_coreVar._atticBallHoleObjectId);
				if ((_crep != 997) && (_crep != 139))
					aniof(2, 7);
			}
			if ((_coreVar._currPlace == ATTIC) && (_num == 2) && (_coreVar._atticRodHoleObjectId != 0)) {
				avpoing(_coreVar._atticRodHoleObjectId);
				if ((_crep != 997) && (_crep != 139))
					aniof(2, 6);
			}
			if ((_coreVar._currPlace == CELLAR) && (_coreVar._cellarObjectId != 0)) {
				avpoing(_coreVar._cellarObjectId);
				if ((_crep != 997) && (_crep != 139))
					aniof(2, 2);
			}
			if ((_coreVar._currPlace == CRYPT) && (_coreVar._cryptObjectId != 0))
				avpoing(_coreVar._cryptObjectId);

			if ((_coreVar._currPlace == SECRET_PASSAGE) && (_coreVar._secretPassageObjectId != 0)) {
				avpoing(_coreVar._secretPassageObjectId);
				if ((_crep != 997) && (_crep != 139)) {
					_crep = 182;
					aniof(2, 1);
				}
			}
			if ((_coreVar._currPlace == WELL) && (_coreVar._wellObjectId != 0)) {
				avpoing(_coreVar._wellObjectId);
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
	int cx, cy, cz;

	cx = 0;
	do {
		++cx;
	} while (_menu._inventoryMenu[cx] != _msg[4]);
	cz = 0;
	cy = 0;
	do {
		++cy;
		if (ord(_coreVar._sjer[cy]) != 0)
			++cz;
	} while (cz != cx);
	cz = ord(_coreVar._sjer[cy]);
	_coreVar._sjer[cy] = chr(0);
	modinv();
	avpoing(cz);
	_crep = 998;
	clearScreenType2();
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
	tcoord(3);
	if (_num == 0) {
		tcoord(8);
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
	_crep = _tabdon[asoul + (tmpPlace << 3) + (_num - 1)];
	if (_crep == 255)
		_crep = 997;
}

/**
 * Engine function - Read
 * @remarks	Originally called 'tlire'
 */
void MortevielleEngine::fctRead() {
	if (_caff > 99)
		st4(_caff);
	else {
		if (!_syn)
			ecr3(getEngineString(S_READ));
		tfleche();
		if (!(_anyone) && !(_keyPressedEsc)) {
			tcoord(4);
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
		st4(_coreVar._selectedObjectId);
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
	tcoord(5);
	if (_num == 0) {
		tcoord(8);
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
	_crep = _tabdon[arega + (cx * 7) + _num - 1];
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
	if ((_coreVar._currPlace == 9) && (_num == 2))
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
		st7(_caff);
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

	tcoord(6);
	if (_num == 0) {
		tcoord(7);
		if (_num != 0) {
			int cx = 0;
			do {
				++cx;
			} while ((cx <= 6) && (_num != ord(_touv[cx])));
			if (_num != ord(_touv[cx]))
				_crep = 187;
			else {
				if (_currBitIndex > 0)
					_coreVar._faithScore += 3;

				rechai(_mchai);
				if (_mchai != 0) {
					_cs = 0;
					_is = 0;
					_heroSearching = true;
					_menu.setSearchMenu();
					tsuiv();
				} else
					_crep = 997;
			}
		} else {
			tcoord(8);
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
		st7(_coreVar._selectedObjectId);
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
		if (_ment != 0) {
			_msg[4] = OPCODE_ENTER;
			_syn = true;
		} else
			_crep = 997;
		return;
	}

	if (_caff == 15) {
		showMoveMenuAlert();
		return;
	}

	tfleche();
	if ((_anyone) || (_keyPressedEsc))
		return;

	tcoord(7);
	if (_num != 0) {
		if (_currBitIndex > 0)
			_coreVar._faithScore += 2;
		++_iouv;
		int tmpPlace = 0;
		do {
			++tmpPlace;
		} while (!((tmpPlace > 6) || (ord(_touv[tmpPlace]) == 0) || (ord(_touv[tmpPlace]) == _num)));
		if (ord(_touv[tmpPlace]) != _num) {
			if (!( ((_num == 3) && ((_coreVar._currPlace == OWN_ROOM) || (_coreVar._currPlace == 9) || (_coreVar._currPlace == BLUE_ROOM) || (_coreVar._currPlace == BATHROOM)))
			    || ((_num == 4) && ((_coreVar._currPlace == GREEN_ROOM) || (_coreVar._currPlace == PURPLE_ROOM) || (_coreVar._currPlace == RED_ROOM)))
				|| ((_coreVar._currPlace == DARKBLUE_ROOM) && (_num == 5))
				|| ((_num == 6) && ((_coreVar._currPlace == BATHROOM) || (_coreVar._currPlace == DINING_ROOM) || (_coreVar._currPlace == GREEN_ROOM2) || (_coreVar._currPlace == ATTIC)))
				|| ((_coreVar._currPlace == GREEN_ROOM2) && (_num == 2))
				|| ((_coreVar._currPlace == KITCHEN) && (_num == 7))) ) {
				if ( ((_coreVar._currPlace > DINING_ROOM) && (_coreVar._currPlace < CELLAR))
				  || ((_coreVar._currPlace > RED_ROOM) && (_coreVar._currPlace < DINING_ROOM))
				  || (_coreVar._currPlace == OWN_ROOM) || (_coreVar._currPlace == PURPLE_ROOM) || (_coreVar._currPlace == BLUE_ROOM)) {
					if (getRandomNumber(1, 4) == 3)
						_speechManager.startSpeech(7, 9, 1);
				}
				_touv[tmpPlace] = chr(_num);
				aniof(1, _num);
			}
			tmpPlace = _coreVar._currPlace;
			if (_coreVar._currPlace == CRYPT)
				tmpPlace = CELLAR;
			_crep = _tabdon[aouvr + (tmpPlace * 7) + _num - 1];
			if (_crep == 254)
				_crep = 999;
		} else
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

	tcoord(8);
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
					repon(2, 165);
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
						clearScreenType2();
						clearScreenType3();
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
							_menu.setDestinationMenuText(SECRET_PASSAGE);
						} else {
							_menu.setDestinationMenuText(_coreVar._currPlace);
							setPal(14);
							dessin(0);
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
	tcoord(9);
	if (_num != 0) {
		_crep = 997;
		if ((_coreVar._currPlace == ATTIC) && (_coreVar._atticRodHoleObjectId == 159) && (_coreVar._atticBallHoleObjectId == 141)) {
			repon(2, 167);
			_speechManager.startSpeech(7, 9, 1);
			int answer = Alert::show(getEngineString(S_YES_NO), 1);
			if (answer == 1)
				_endGame = true;
			else
				_crep = 168;
		}
		if ((_coreVar._currPlace == SECRET_PASSAGE) && (_coreVar._secretPassageObjectId == 143)) {
			repon(2, 175);
			clearScreenType3();
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
		tcoord(10);
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
			tcoord(8);
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
		tcoord(7);
		if (_num != 0) {
			int cx = 0;
			do {
				++cx;
			} while ((cx <= 6) && (_num != ord(_touv[cx])));
			if (_num == ord(_touv[cx])) {
				aniof(2, _num);
				_crep = 998;
				_touv[cx] = chr(0);
				--_iouv;
				if (_iouv < 0)
					_iouv = 0;
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
		int p = getPresenceStats(rand, _coreVar._faithScore, _ment);
		int l = _ment;
		if (l != 0) {
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

		if (_ment == 8)
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
		tcoord(7);
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
				} while ((cx <= 6) && (_num != ord(_touv[cx])));
				if (_num != ord(_touv[cx]))
					_crep = 187;
				else {
					_mchai = chai;
					_crep = 999;
				}
			}
		} else {
			tcoord(8);
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
					tcoord(5);
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
		int p = getPresenceStats(rand, _coreVar._faithScore, _ment);
		int l = _ment;
		if (l != 0) {
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
		tsort();
		_coreVar._currPlace = DINING_ROOM;
		_caff = 10;
		resetRoomVariables(_coreVar._currPlace);
		_menu.setDestinationMenuText(_coreVar._currPlace);

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
			
			_jh += h;
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
		_menu.setDestinationMenuText(_coreVar._currPlace);
	} else if (_coreVar._currPlace == LANDING)
		showMoveMenuAlert();
	else if (_ment == 0)
		_crep = 997;
	else if ((_ment == 9) && (_coreVar._selectedObjectId != 136)) {
			_crep = 189;
			_coreVar._teauto[8] = '*';
	} else {
		int z = 0;
		if (!_blo)
			z = t11(_ment);
		if (z != 0) {
			if ((_ment == 3) || (_ment == 7))
				_crep = 179;
			else {
				_x = (getRandomNumber(0, 10)) - 5;
				_speechManager.startSpeech(7, _x, 1);
				aniof(1, 1);
				
				_x = convertBitIndexToCharacterIndex(z);
				++_coreVar._faithScore;
				_coreVar._currPlace = LANDING;
				_msg[3] = MENU_DISCUSS;
				_msg[4] = _menu._discussMenu[_x];
				_syn = true;
				if (_ment == 9) {
					_col = true;
					_caff = 70;
					afdes();
					repon(2, _caff);
				} else
					_col = false;
				resetRoomVariables(_ment);
				_ment = 0;
			}
		} else {
			_x = (getRandomNumber(0, 10)) - 5;
			_speechManager.startSpeech(7, _x, 1);
			aniof(1, 1);
			
			_coreVar._currPlace = _ment;
			affrep();
			resetRoomVariables(_coreVar._currPlace);
			_menu.setDestinationMenuText(_coreVar._currPlace);
			_ment = 0;
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
		tsort();
		_coreVar._currPlace = OWN_ROOM;
		affrep();
		afdes();
		resetRoomVariables(_coreVar._currPlace);
		_menu.setDestinationMenuText(_coreVar._currPlace);
	}
	clearScreenType3();
	clearScreenType2();
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
			_jh += z;
			h = 7;
		}
		_jh += 2;
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
	tsort();
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
			nextPlace = t23coul();
		else if (_coreVar._currPlace == INSIDE_WELL)
			nextPlace = WELL;

		if (_crep != 997)
			_coreVar._currPlace = nextPlace;

		_caff = nextPlace;
		if (_crep == 0)
			_crep = nextPlace;
		resetRoomVariables(nextPlace);
		_menu.setDestinationMenuText(nextPlace);
	}
}

/**
 * Engine function - Wait
 * @remarks	Originally called 'tattendre'
 */
void MortevielleEngine::fctWait() {
	_savedBitIndex = 0;
	clearScreenType3();

	int answer;
	do {
		++_jh;
		prepareRoom();
		if (!_blo)
			t11(_coreVar._currPlace);
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
		repon(2, 102);
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
	bool te[47];
	int cy, cx, max, suj, co, lig, icm, i, choi, x, y, c;
	char tou;
	Common::String lib[47];
	bool f;

	endSearch();
	if (_col)
		suj = 128;
	else {
		cx = 0;
		do {
			++cx;
		} while (_menu._discussMenu[cx] != _msg[4]);
		_caff = 69 + cx;
		afdes();
		repon(2, _caff);
		suj = _caff + 60;
	}
	tkey1(false);
	mennor();
	_mouse.hideMouse();
	hirs();
	premtet();
	startDialog(suj);
	hirs();
	for (int ix = 1; ix <= 46; ++ix)
		te[ix] = false;
	for (int ix = 1; ix <= 45; ++ix) {
		lib[ix] = getString(ix + kQuestionStringIndex);
		for (i = lib[ix].size(); i <= 40; ++i)
			lib[ix] = lib[ix] + ' ';
	}
	lib[46] = lib[45];
	lib[45] = ' ';
	_mouse.showMouse();
	do {
		choi = 0;
		icm = 0;
		co = 0;
		lig = 0;
		do {
			++icm;
			_screenSurface.putxy(co, lig);
			if (_coreVar._teauto[icm] == '*') {
				if (te[icm])
					writetp(lib[icm], 1);
				else
					writetp(lib[icm], 0);
			}

			if (icm == 23)  {
				lig = 0;
				co = 320;
			} else
				lig = lig + 8;
		} while (icm != 42);
		_screenSurface.putxy(320, 176);
		writetp(lib[46], 0);
		tou = '\0';
		do {
			_mouse.moveMouse(f, tou);
			CHECK_QUIT;

			_mouse.getMousePosition(x, y, c);
			x *= (3 - _res);
			if (x > 319)
				cx = 41;
			else
				cx = 1;
			cy = ((uint)y >> 3) + 1;      // 0-199 => 1-25
			if ((cy > 23) || ((cx == 41) && ((cy >= 20) && (cy <= 22)))) {
				if (choi != 0) {
					lig = ((choi - 1) % 23) << 3;
					if (choi > 23)
						co = 320;
					else
						co = 0;
					_screenSurface.putxy(co, lig);
					if (te[choi])
						writetp(lib[choi], 0);
					else
						writetp(lib[choi], 1);
					te[choi] = !te[choi];
					choi = 0;
				}
			} else {
				int ix = cy;
				if (cx == 41)
					ix += 23;
				if (ix != choi) {
					if (choi != 0) {
						lig = ((choi - 1) % 23) << 3;
						if (choi > 23)
							co = 320;
						else
							co = 0;
						_screenSurface.putxy(co, lig);
						if (te[choi])
							writetp(lib[choi], 0);
						else
							writetp(lib[choi], 1);
						te[choi] = ! te[choi];
					}
					if ((_coreVar._teauto[ix] == '*') || (ix == 46)) {
						lig = ((ix - 1) % 23) << 3;
						if (ix > 23)
							co = 320;
						else
							co = 0;
						_screenSurface.putxy(co, lig);
						if (te[ix])
							writetp(lib[ix], 0);
						else
							writetp(lib[ix], 1);
						te[ix] = ! te[ix];
						choi = ix;
					} else
						choi = 0;
				}
			}
		} while (!((tou == '\15') || (((c != 0) || getMouseClick()) && (choi != 0))));
		setMouseClick(false);
		if (choi != 46) {
			int ix = choi - 1;
			if (_col) {
				_col = false;
				_coreVar._currPlace = 15;
				if (_iouv > 0)
					max = 8;
				else
					max = 4;
				if (getRandomNumber(1, max) == 2)
					suj = 129;
				else {
					suj = 138;
					_coreVar._faithScore += (3 * (_coreVar._faithScore / 10));
				}
			} else if (_nbrep[_caff - 69] < _nbrepm[_caff - 69]) {
				suj = _tabdon[arep + (ix << 3) + (_caff - 70)];
				_coreVar._faithScore += _tabdon[arcf + ix];
				++_nbrep[_caff - 69];
			} else {
				_coreVar._faithScore += 3;
				suj = 139;
			}
			_mouse.hideMouse();
			hirs();
			premtet();
			startDialog(suj);
			_mouse.showMouse();
			if ((suj == 84) || (suj == 86)) {
				_coreVar._pourc[5] = '*';
				_coreVar._teauto[7] = '*';
			}
			if ((suj == 106) || (suj == 108) || (suj == 94)) {
				for (int indx = 29; indx <= 31; ++indx)
					_coreVar._teauto[indx] = '*';
				_coreVar._pourc[7] = '*';
			}
			if (suj == 70) {
				_coreVar._pourc[8] = '*';
				_coreVar._teauto[32] = '*';
			}
			_mouse.hideMouse();
			hirs();
			_mouse.showMouse();
		}
	} while ((choi != 46) && (suj != 138));
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
	_menu.setDestinationMenuText(_coreVar._currPlace);
	clearScreenType3();
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
	clearScreenType1();
	clearScreenType2();
	clearScreenType3();
	repon(9, 1509);
	tkey1(false);
	_mouse.hideMouse();
	_caff = 70;
	taffich();
	hirs();
	premtet();
	startDialog(141);
	_mouse.showMouse();
	clearScreenType1();
	repon(9, 1509);
	repon(2, 142);
	tkey1(false);
	_caff = 32;
	afdes();
	repon(6, 34);
	repon(2, 35);
	startMusicOrSpeech(0);
	tkey1(false);
	// A wait message was displayed. 
	// tkey1 was called before and after.
	// Most likely the double call is useless, thus removed
	//
	// tkey1(false);
	resetVariables();
}

/**
 * You lost!
 * @remarks	Originally called 'tencore'
 */
void MortevielleEngine::askRestart() {
	clearScreenType2();
	startMusicOrSpeech(0);
	tkey1(false);
	displayEmptyHand();
	resetVariables();
	initGame();
	_currHour = 10;
	_currHalfHour = 0;
	_currDay = 0;
	_minute = 0;
	_hour = 10;
	_day = 0;
	repon(2, 180);

	int answer = Alert::show(getEngineString(S_YES_NO), 1);
	_quitGame = (answer != 1);
}

} // End of namespace Mortevielle
