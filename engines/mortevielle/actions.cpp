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
	if ((g_s._currPlace == ROOM26) && (g_msg[4] == _menu._moveMenu[6])) {
		g_s._currPlace = LANDING;
		g_caff = g_s._currPlace;
		afdes(0);
		repon(2, g_s._currPlace);
	}
	if ((g_s._currPlace == LANDING) && (g_msg[4] == _menu._moveMenu[6])) {
		if (!_syn)
			ecr3(getEngineString(S_GO_TO));
		tfleche();

		if (_keyPressedEsc)
			_okdes = false;

		if ((_anyone) || (_keyPressedEsc))
			return;

		tcoord(1);

		if (g_num == 0)
			return;

		if (g_num == 1) {
			g_s._currPlace = OWN_ROOM;
			_menu.setDestinationMenuText(OWN_ROOM);
		} else if (g_num == 7) {
			g_s._currPlace = ATTIC;
			_menu.setDestinationMenuText(ATTIC);
		} else if (g_num != 6)
			g_s._currPlace = ROOM26;
		else if ((g_num > 1) && (g_num < 6))
			g_ment = g_num - 1;
		else if (g_num > 7)
			g_ment = g_num - 3;

		if (g_num != 6)
			affrep();
		else
			aldepl();
		return;
	}
	tsort();
	int menuChoice = 1;

	while (_menu._moveMenu[menuChoice] != g_msg[4])
		++menuChoice;

	if (g_s._currPlace == MOUNTAIN) {
		if (menuChoice == 1)
			gotoManorFront();
		else if (menuChoice == 2)
			checkManorDistance();
		_menu.setDestinationMenuText(g_s._currPlace);
		return;
	} else if (g_s._currPlace == INSIDE_WELL) {
		if (menuChoice == 1)
			floodedInWell();
		else if (menuChoice == 2)
			gotoManorBack();
		_menu.setDestinationMenuText(g_s._currPlace);
		return;
	} else if ((g_s._currPlace == BUREAU) && (menuChoice == 1))
		menuChoice = 6;
	else if (g_s._currPlace == KITCHEN) {
		if (menuChoice == 2)
			menuChoice = 6;
		else if (menuChoice == 5)
			menuChoice = 16;
	} else if ((g_s._currPlace == CELLAR) && (menuChoice == 3))
		menuChoice = 6;
	else if (((g_s._currPlace == LANDING) || (g_s._currPlace == ROOM26)) && (menuChoice == 4))
		menuChoice = 6;
	
	if ((g_s._currPlace > MOUNTAIN) && (g_s._currPlace != ROOM26))
		menuChoice += 10;
	
	if ((g_s._currPlace == CHAPEL) && (menuChoice == 13))
		menuChoice = 16;
	else if (g_s._currPlace == MANOR_FRONT) {
		if (menuChoice == 12)
			menuChoice = 16;
		else if (menuChoice > 13)
			menuChoice = 15;
	} else if ((g_s._currPlace == MANOR_BACK) && (menuChoice > 14))
		menuChoice = 15;
	else if ((g_s._currPlace == WELL) && (menuChoice > 13) && (menuChoice != 17))
		menuChoice = 15;

	if (menuChoice == 1)
		g_s._currPlace = BUREAU;
	else if (menuChoice == 2)
		g_s._currPlace = KITCHEN;
	else if (menuChoice == 3)
		g_s._currPlace = CELLAR;
	else if (menuChoice == 4)
		g_s._currPlace = LANDING;
	else if (menuChoice == 5)
		menuChoice = 12;
	else if (menuChoice == 6)
		menuChoice = 11;

	if (menuChoice == 11)
		gotoDiningRoom();
	else if (menuChoice == 12)
		gotoManorFront();
	else if (menuChoice == 13)
		g_s._currPlace = CHAPEL;
	else if (menuChoice == 14)
		g_s._currPlace = WELL;
	else if (menuChoice == 15)
		checkManorDistance();
	else if (menuChoice == 16)
		gotoManorBack();
	else if (menuChoice == 17) {
		if ((g_s._wellObjectId != 120) && (g_s._wellObjectId != 140))
			g_crep = 997;
		else if (g_s._wellObjectId == 120)
			g_crep = 181;
		else if (g_s._faithScore > 80) {
			g_crep = 1505;
			loseGame();
		} else {
			g_s._currPlace = INSIDE_WELL;
			affrep();
		}
	}
	if ((menuChoice < 5) || (menuChoice == 13) || (menuChoice == 14))
		affrep();
	debloc(g_s._currPlace);
	_menu.setDestinationMenuText(g_s._currPlace);
}

/**
 * Engine function - Take
 * @remarks	Originally called 'tprendre'
 */
void MortevielleEngine::fctTake() {
	if (g_caff > 99) {
		int cx = g_caff;
		avpoing(cx);
		if (g_crep != 139) {
			if (_currBitIndex > 0)
				g_s._faithScore += 3;
			if (_obpart) {
				if (g_s._currPlace == PURPLE_ROOM)
					g_s._purpleRoomObjectId = 0;
				if (g_s._currPlace == ATTIC) {
					if (g_s._atticBallHoleObjectId == g_caff)
						g_s._atticBallHoleObjectId = 0;
					if (g_s._atticRodHoleObjectId == g_caff)
						g_s._atticRodHoleObjectId = 0;
				}
				if (g_s._currPlace == CELLAR)
					g_s._cellarObjectId = 0;
				if (g_s._currPlace == CRYPT)
					g_s._cryptObjectId = 0;
				if (g_s._currPlace == SECRET_PASSAGE)
					g_s._secretPassageObjectId = 0;
				if (g_s._currPlace == WELL)
					g_s._wellObjectId = 0;
				_menu.unsetSearchMenu();
				_obpart = false;
				affrep();
			} else {
				g_tabdon[acha + ((g_mchai - 1) * 10) + g_cs - 1] = 0;
				tsuiv();
				++g_dobj;
				if (g_dobj > 6) {
					g_s._faithScore += 2;
					g_dobj = 0;
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
	if (g_caff == 3) {
		tcoord(2);
		if (g_num == 1) {
			g_crep = 152;
			return;
		}
	}
	tcoord(5);
	if ((g_num == 0) || ((g_num == 1) && (g_s._currPlace == CRYPT))) {
		tcoord(8);
		if (g_num != 0) {
			if (_currBitIndex > 0)
				g_s._faithScore += 3;
			g_crep = 997;
			if ((g_s._currPlace == PURPLE_ROOM) && (g_s._purpleRoomObjectId != 0))
				avpoing(g_s._purpleRoomObjectId);
			if ((g_s._currPlace == ATTIC) && (g_num == 1) && (g_s._atticBallHoleObjectId != 0)) {
				avpoing(g_s._atticBallHoleObjectId);
				if ((g_crep != 997) && (g_crep != 139))
					aniof(2, 7);
			}
			if ((g_s._currPlace == ATTIC) && (g_num == 2) && (g_s._atticRodHoleObjectId != 0)) {
				avpoing(g_s._atticRodHoleObjectId);
				if ((g_crep != 997) && (g_crep != 139))
					aniof(2, 6);
			}
			if ((g_s._currPlace == CELLAR) && (g_s._cellarObjectId != 0)) {
				avpoing(g_s._cellarObjectId);
				if ((g_crep != 997) && (g_crep != 139))
					aniof(2, 2);
			}
			if ((g_s._currPlace == CRYPT) && (g_s._cryptObjectId != 0))
				avpoing(g_s._cryptObjectId);

			if ((g_s._currPlace == SECRET_PASSAGE) && (g_s._secretPassageObjectId != 0)) {
				avpoing(g_s._secretPassageObjectId);
				if ((g_crep != 997) && (g_crep != 139)) {
					g_crep = 182;
					aniof(2, 1);
				}
			}
			if ((g_s._currPlace == WELL) && (g_s._wellObjectId != 0)) {
				avpoing(g_s._wellObjectId);
				if ((g_crep != 997) && (g_crep != 139))
					aniof(2, 1);
			}
			if ((g_crep != 997) && (g_crep != 182) && (g_crep != 139))
				g_crep = 999;
		}
	} else {
		if ( ((g_s._currPlace == OWN_ROOM)  && (g_num == 3))
		  || ((g_s._currPlace == GREEN_ROOM)  && (g_num == 4))
		  || ((g_s._currPlace == PURPLE_ROOM)  && (g_num == 1))
		  || ((g_s._currPlace == DARKBLUE_ROOM)  && (g_num == 3))
		  || ((g_s._currPlace == BLUE_ROOM)  && (g_num == 6))
		  || ((g_s._currPlace == RED_ROOM)  && (g_num == 2))
		  || ((g_s._currPlace == BATHROOM)  && (g_num == 6))
		  || ((g_s._currPlace == GREEN_ROOM2)  && (g_num == 4))
		  || ((g_s._currPlace == ROOM9) && (g_num == 4))
		  || ((g_s._currPlace == DINING_ROOM) && (g_num > 2))
		  || ((g_s._currPlace == BUREAU) && (g_num == 7))
		  || ((g_s._currPlace == KITCHEN) && (g_num == 6))
		  || ((g_s._currPlace == ATTIC) && (g_num > 4))
		  || ((g_s._currPlace > ATTIC) && (g_s._currPlace != INSIDE_WELL)) )
		  g_crep = 997;
		else if (g_s._currPlace == INSIDE_WELL) {
			g_crep = 1504;
			loseGame();
		} else
			g_crep = 120;
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
	} while (_menu._inventoryMenu[cx] != g_msg[4]);
	cz = 0;
	cy = 0;
	do {
		++cy;
		if (ord(g_s._sjer[cy]) != 0)
			++cz;
	} while (cz != cx);
	cz = ord(g_s._sjer[cy]);
	g_s._sjer[cy] = chr(0);
	modinv();
	avpoing(cz);
	g_crep = 998;
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
	if (g_num == 0) {
		tcoord(8);
		if (g_num != 0) {
			if (_currBitIndex > 0)
				++g_s._faithScore;
			g_crep = 997;
			if ((g_s._currPlace == PURPLE_ROOM) && (g_s._purpleRoomObjectId != 0))
				treg(g_s._purpleRoomObjectId);
		}
		return;
	}
	if (_currBitIndex > 0)
		++g_s._faithScore;
	int tmpPlace = g_s._currPlace;
	if (g_s._currPlace == CRYPT)
		tmpPlace = 14;
	else if (g_s._currPlace == MOUNTAIN)
		tmpPlace = 15;
	g_crep = g_tabdon[asoul + (tmpPlace << 3) + (g_num - 1)];
	if (g_crep == 255)
		g_crep = 997;
}

/**
 * Engine function - Read
 * @remarks	Originally called 'tlire'
 */
void MortevielleEngine::fctRead() {
	if (g_caff > 99)
		st4(g_caff);
	else {
		if (!_syn)
			ecr3(getEngineString(S_READ));
		tfleche();
		if (!(_anyone) && !(_keyPressedEsc)) {
			tcoord(4);
			if (g_num != 0)
				g_crep = 107;
		}
	}
}

/**
 * Engine function - Self / Read
 * @remarks	Originally called 'tslire'
 */
void MortevielleEngine::fctSelfRead() {
	if (g_s._selectedObjectId == 0)
		g_crep = 186;
	else
		st4(g_s._selectedObjectId);
}

/**
 * Engine function - Look
 * @remarks	Originally called 'tregarder'
 */
void MortevielleEngine::fctLook() {
	int cx;

	if (g_caff > 99) {
		g_crep = 103;
		return;
	}
	if (!_syn)
		ecr3(getEngineString(S_LOOK));
	tfleche();
	if ((_anyone) || (_keyPressedEsc))
		return;
	tcoord(5);
	if (g_num == 0) {
		tcoord(8);
		g_crep = 131;
		if (g_num != 0) {
			if (g_s._currPlace == ATTIC) {
				if (g_num == 1) {
					g_crep = 164;
					if (g_s._atticRodHoleObjectId != 0)
						treg(g_s._atticRodHoleObjectId);
					else if (g_s._atticBallHoleObjectId != 0)
						treg(g_s._atticBallHoleObjectId);
				} else {
					g_crep = 193;
					if (g_s._atticRodHoleObjectId != 0)
						treg(g_s._atticRodHoleObjectId);
				}
			}
			if (g_s._currPlace == CELLAR) {
				g_crep = 164;
				if (g_s._cellarObjectId != 0)
					treg(g_s._cellarObjectId);
			}
			if (g_s._currPlace == SECRET_PASSAGE) {
				g_crep = 174;
				if (g_s._secretPassageObjectId != 0)
					treg(g_s._secretPassageObjectId);
			}
			if (g_s._currPlace == WELL) {
				g_crep = 131;
				if (g_s._wellObjectId != 0)
					treg(g_s._wellObjectId);
			}
		}
		return;
	}
	cx = g_s._currPlace;
	if (g_s._currPlace == CHAPEL)
		cx = 17;
	if ((g_s._currPlace > MANOR_FRONT) && (g_s._currPlace < DOOR))
		cx -= 4;
	if (g_s._currPlace == ROOM26)
		cx = 21;
	g_crep = g_tabdon[arega + (cx * 7) + g_num - 1];
	if ((g_s._currPlace == ATTIC) && (g_num == 8))
		g_crep = 126;
	if (g_s._currPlace == MOUNTAIN)
		g_crep = 103;
	if (g_crep == 255)
		g_crep = 131;
	if ((g_s._currPlace == GREEN_ROOM) && (g_num == 1))
		treg(144);
	if ((g_s._currPlace == BLUE_ROOM) && (g_num == 3))
		treg(147);
	if ((g_s._currPlace == GREEN_ROOM2) && (g_num == 3))
		treg(149);
	if ((g_s._currPlace == 9) && (g_num == 2))
		treg(30);
	if ((g_s._currPlace == DINING_ROOM) && (g_num == 3))
		treg(31);
}

/**
 * Engine function - Self / Look
 * @remarks	Originally called 'tsregarder'
 */
void MortevielleEngine::fctSelftLook() {
	if (g_s._selectedObjectId != 0)
		treg(g_s._selectedObjectId);
	else
		g_crep = 186;
}

/**
 * Engine function - Search
 * @remarks	Originally called 'tfouiller'
 */
void MortevielleEngine::fctSearch() {
	const byte r[14] = {123, 104, 123, 131, 131, 123, 104, 131, 123, 123, 106, 123, 123, 107};

	if (g_caff > 99) {
		st7(g_caff);
		return;
	}

	if (!_syn)
		ecr3(getEngineString(S_SEARCH));

	tfleche();
	if (_anyone || _keyPressedEsc)
		return;

	if (g_s._currPlace == INSIDE_WELL) {
		g_crep = 1504;
		loseGame();
		return;
	}

	tcoord(6);
	if (g_num == 0) {
		tcoord(7);
		if (g_num != 0) {
			int cx = 0;
			do {
				++cx;
			} while ((cx <= 6) && (g_num != ord(g_touv[cx])));
			if (g_num != ord(g_touv[cx]))
				g_crep = 187;
			else {
				if (_currBitIndex > 0)
					g_s._faithScore += 3;

				rechai(g_mchai);
				if (g_mchai != 0) {
					g_cs = 0;
					g_is = 0;
					_heroSearching = true;
					_menu.setSearchMenu();
					tsuiv();
				} else
					g_crep = 997;
			}
		} else {
			tcoord(8);
			g_crep = 997;
			if (g_num != 0) {
				if (_currBitIndex > 0)
					g_s._faithScore += 3;
				if ((g_s._currPlace != WELL) && (g_s._currPlace != SECRET_PASSAGE) && (g_s._currPlace != ATTIC)) {
					if (g_s._currPlace == PURPLE_ROOM) {
						g_crep = 123;
						if (g_s._purpleRoomObjectId != 0)
							treg(g_s._purpleRoomObjectId);
					}
					if (g_s._currPlace == CRYPT) {
						g_crep = 123;
						if (g_s._cryptObjectId != 0)
							treg(g_s._cryptObjectId);
					}
				}
			}
		}
	} else {
		if (_currBitIndex > 0)
			g_s._faithScore += 3;
		g_crep = 997;
		if (g_s._currPlace < CELLAR)
			g_crep = r[g_s._currPlace];

		if ((g_s._currPlace == TOILETS) && (g_num == 2))
			g_crep = 162;

		if (g_s._currPlace == KITCHEN) {
			if ((g_num == 3) || (g_num == 4))
				g_crep = 162;
			else if (g_num == 5)
				g_crep = 159;
		}

		if (g_s._currPlace == MOUNTAIN)
			g_crep = 104;
		else if (g_s._currPlace == CRYPT)
			g_crep = 155;
	}
}

/**
 * Engine function - Self / Search
 * @remarks	Originally called 'tsfouiller'
 */
void MortevielleEngine::fctSelfSearch() {
	if (g_s._selectedObjectId != 0)
		st7(g_s._selectedObjectId);
	else
		g_crep = 186;
}

/**
 * Engine function - Open
 * @remarks	Originally called 'touvrir'
 */
void MortevielleEngine::fctOpen() {
	if (!_syn)
		ecr3(getEngineString(S_OPEN));

	if (g_caff == ROOM26) {
		if (g_ment != 0) {
			g_msg[4] = OPCODE_ENTER;
			_syn = true;
		} else
			g_crep = 997;
		return;
	}

	if (g_caff == 15) {
		aldepl();
		return;
	}

	tfleche();
	if ((_anyone) || (_keyPressedEsc))
		return;

	tcoord(7);
	if (g_num != 0) {
		if (_currBitIndex > 0)
			g_s._faithScore += 2;
		++g_iouv;
		int tmpPlace = 0;
		do {
			++tmpPlace;
		} while (!((tmpPlace > 6) || (ord(g_touv[tmpPlace]) == 0) || (ord(g_touv[tmpPlace]) == g_num)));
		if (ord(g_touv[tmpPlace]) != g_num) {
			if (!( ((g_num == 3) && ((g_s._currPlace == OWN_ROOM) || (g_s._currPlace == 9) || (g_s._currPlace == BLUE_ROOM) || (g_s._currPlace == BATHROOM)))
			    || ((g_num == 4) && ((g_s._currPlace == GREEN_ROOM) || (g_s._currPlace == PURPLE_ROOM) || (g_s._currPlace == RED_ROOM)))
				|| ((g_s._currPlace == DARKBLUE_ROOM) && (g_num == 5))
				|| ((g_num == 6) && ((g_s._currPlace == BATHROOM) || (g_s._currPlace == DINING_ROOM) || (g_s._currPlace == GREEN_ROOM2) || (g_s._currPlace == ATTIC)))
				|| ((g_s._currPlace == GREEN_ROOM2) && (g_num == 2))
				|| ((g_s._currPlace == KITCHEN) && (g_num == 7))) ) {
				if ( ((g_s._currPlace > DINING_ROOM) && (g_s._currPlace < CELLAR))
				  || ((g_s._currPlace > RED_ROOM) && (g_s._currPlace < DINING_ROOM))
				  || (g_s._currPlace == OWN_ROOM) || (g_s._currPlace == PURPLE_ROOM) || (g_s._currPlace == BLUE_ROOM)) {
					if (getRandomNumber(1, 4) == 3)
						_speechManager.startSpeech(7, 9, 1);
				}
				g_touv[tmpPlace] = chr(g_num);
				aniof(1, g_num);
			}
			tmpPlace = g_s._currPlace;
			if (g_s._currPlace == CRYPT)
				tmpPlace = CELLAR;
			g_crep = g_tabdon[aouvr + (tmpPlace * 7) + g_num - 1];
			if (g_crep == 254)
				g_crep = 999;
		} else
			g_crep = 18;
	}
}

/**
 * Engine function - Place
 * @remarks	Originally called 'tmettre'
 */
void MortevielleEngine::fctPlace() {
	if (g_s._selectedObjectId == 0) {
		g_crep = 186;
		return;
	}

	if (!_syn)
		ecr3(getEngineString(S_PUT));

	tfleche();
	if (_keyPressedEsc)
		g_crep = 998;

	if ((_anyone) || (_keyPressedEsc))
		return;

	tcoord(8);
	if (g_num != 0) {
		g_crep = 999;
		if (g_caff == 13) {
			if (g_num == 1) {
				if (g_s._atticBallHoleObjectId != 0) {
					g_crep = 188;
				} else {
					g_s._atticBallHoleObjectId = g_s._selectedObjectId;
					if (g_s._selectedObjectId == 141)
						aniof(1, 7);
				}
			} else if (g_s._atticRodHoleObjectId != 0) {
				g_crep = 188;
			} else {
				g_s._atticRodHoleObjectId = g_s._selectedObjectId;
				if (g_s._selectedObjectId == 159)
					aniof(1, 6);
			}
		}

		if (g_caff == 14) {
			if (g_s._cellarObjectId != 0) {
				g_crep = 188;
			} else {
				g_s._cellarObjectId = g_s._selectedObjectId;
				if (g_s._selectedObjectId == 151) {
					// Open hidden passage
					aniof(1, 2);
					aniof(1, 1);
					repon(2, 165);
					maivid();
					_speechManager.startSpeech(6, -9, 1);

					// Do you want to enter the hidden passage?
					int answer = Alert::show(getEngineString(S_YES_NO), 1);
					if (answer == 1) {
						Common::String alertTxt = deline(582);
						Alert::show(alertTxt, 1);

						bool enterPassageFl = KnowledgeCheck::show();
						hideMouse();
						hirs();
						drawRightFrame();
						clearScreenType2();
						clearScreenType3();
						showMouse();
						tinke();
						drawClock();
						if (_currBitIndex != 0)
							showPeoplePresent(_currBitIndex);
						else 
							displayAloneText();

						_menu.displayMenu();
						if (enterPassageFl) {
							g_s._currPlace = SECRET_PASSAGE;
							_menu.setDestinationMenuText(SECRET_PASSAGE);
						} else {
							_menu.setDestinationMenuText(g_s._currPlace);
							writepal(14);
							dessin(0);
							aniof(1, 2);
							aniof(1, 1);
							alertTxt = deline(577);
							Alert::show(alertTxt, 1);
							aniof(2, 1);
							g_crep = 166;
						}
						affrep();
					} else {
						aniof(2, 1);
						g_crep = 166;
					}
					return;
				}
			}
		}

		if (g_caff == 16) {
			if (g_s._cryptObjectId == 0)
				g_s._cryptObjectId = g_s._selectedObjectId;
			else
				g_crep = 188;
		}

		if (g_caff == 17) {
			if (g_s._secretPassageObjectId != 0) {
				g_crep = 188;
			} else if (g_s._selectedObjectId == 143) {
				g_s._secretPassageObjectId = 143;
				aniof(1, 1);
			} else {
				g_crep = 1512;
				loseGame();
			}
		}

		if (g_caff == 24) {
			if (g_s._wellObjectId != 0) {
				g_crep = 188;
			} else if ((g_s._selectedObjectId == 140) || (g_s._selectedObjectId == 120)) {
				g_s._wellObjectId = g_s._selectedObjectId;
				aniof(1, 1);
			} else {
				g_crep = 185;
			}
		}

		if (g_crep != 188)
			maivid();
	}
}

/**
 * Engine function - Turn
 * @remarks	Originally called 'ttourner'
 */
void MortevielleEngine::fctTurn() {
	if (g_caff > 99) {
		g_crep = 149;
		return;
	}
	if (!_syn)
		ecr3(getEngineString(S_TURN));
	tfleche();
	if ((_anyone) || (_keyPressedEsc))
		return;
	tcoord(9);
	if (g_num != 0) {
		g_crep = 997;
		if ((g_s._currPlace == ATTIC) && (g_s._atticRodHoleObjectId == 159) && (g_s._atticBallHoleObjectId == 141)) {
			repon(2, 167);
			_speechManager.startSpeech(7, 9, 1);
			int answer = Alert::show(getEngineString(S_YES_NO), 1);
			if (answer == 1)
				_endGame = true;
			else
				g_crep = 168;
		}
		if ((g_s._currPlace == SECRET_PASSAGE) && (g_s._secretPassageObjectId == 143)) {
			repon(2, 175);
			clearScreenType3();
			_speechManager.startSpeech(6, -9, 1);
			int answer = Alert::show(getEngineString(S_YES_NO), 1);
			if (answer == 1) {
				g_s._currPlace = CRYPT;
				affrep();
			} else
				g_crep = 176;
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
		if (g_num == 0)
			_hiddenHero = false;
		else {
			_hiddenHero = true;
			g_crep = 999;
		}
	}
}

/**
 * Engine function - Attach
 * @remarks	Originally called 'tattacher'
 */
void MortevielleEngine::fctAttach() {
	if (g_s._selectedObjectId == 0)
		g_crep = 186;
	else {
		if (!_syn)
			ecr3(getEngineString(S_TIE));
		tfleche();
		if (!(_anyone) && !(_keyPressedEsc)) {
			tcoord(8);
			g_crep = 997;
			if ((g_num != 0) && (g_s._currPlace == WELL)) {
				g_crep = 999;
				if ((g_s._selectedObjectId == 120) || (g_s._selectedObjectId == 140)) {
					g_s._wellObjectId = g_s._selectedObjectId;
					aniof(1, 1);
				} else
					g_crep = 185;
				maivid();
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

	if (g_caff < ROOM26) {
		tfleche();
		if (_keyPressedEsc)
			g_crep = 998;
		if ((_anyone) || (_keyPressedEsc))
			return;
		tcoord(7);
		if (g_num != 0) {
			int cx = 0;
			do {
				++cx;
			} while ((cx <= 6) && (g_num != ord(g_touv[cx])));
			if (g_num == ord(g_touv[cx])) {
				aniof(2, g_num);
				g_crep = 998;
				g_touv[cx] = chr(0);
				--g_iouv;
				if (g_iouv < 0)
					g_iouv = 0;
				int chai = 9999;
				rechai(chai);
				if (g_mchai == chai)
					g_mchai = 0;
			} else {
				g_crep = 187;
			}
		}
	}
	if (g_caff == ROOM26)
		g_crep = 999;
}

/**
 * Engine function - Knock
 * @remarks	Originally called 'tfrapper'
 */
void MortevielleEngine::fctKnock() {
	if (!_syn)
		ecr3(getEngineString(S_HIT));

	if (g_s._currPlace == LANDING) {
		Alert::show(getEngineString(S_BEFORE_USE_DEP_MENU), 1);
		return;
	}

	if (g_s._currPlace < DOOR) {
		tfleche();
		if (!(_anyone) && !(_keyPressedEsc)) {
			if ((g_s._currPlace < MOUNTAIN) && (g_s._currPlace != LANDING))
				g_crep = 133;
			else
				g_crep = 997;
		}

		return;
	}

	if (g_s._currPlace == ROOM26) {
		int rand = (getRandomNumber(0, 8)) - 4;
		_speechManager.startSpeech(11, rand, 1);
		int p;
		ecfren(p, rand, g_s._faithScore, g_ment);
		int l = g_ment;
		if (l != 0) {
			if (p != -500) {
				if (rand > p)
					g_crep = 190;
				else {
					becfren(l);
					getKnockAnswer();
				}
			} else
				getKnockAnswer();
		}

		if (g_ment == 8)
			g_crep = 190;
	}
}

/**
 * Engine function - Self / Put
 * @remarks	Originally called 'tposer'
 */
void MortevielleEngine::fctSelfPut() {
	if (!_syn)
		ecr3(getEngineString(S_POSE));
	if (g_s._selectedObjectId == 0)
		g_crep = 186;
	else {
		if (g_caff > 99) {
			g_crep = 999;
			ajchai();
			if (g_crep != 192)
				maivid();
			return;
		}
		tfleche();
		if ((_anyone) || (_keyPressedEsc))
			return;
		tcoord(7);
		g_crep = 124;
		if (g_num != 0) {
			int chai;
			rechai(chai);
			if (chai == 0)
				g_crep = 997;
			else {
				int cx = 0;
				do {
					++cx;
				} while ((cx <= 6) && (g_num != ord(g_touv[cx])));
				if (g_num != ord(g_touv[cx]))
					g_crep = 187;
				else {
					g_mchai = chai;
					g_crep = 999;
				}
			}
		} else {
			tcoord(8);
			if (g_num != 0) {
				g_crep = 998;
				if (g_caff == PURPLE_ROOM) {
					if (g_s._purpleRoomObjectId != 0)
						g_crep = 188;
					else
						g_s._purpleRoomObjectId = g_s._selectedObjectId;
				}

				if (g_caff == ATTIC) {
					if (g_num == 1) {
						if (g_s._atticBallHoleObjectId != 0)
							g_crep = 188;
						else
							g_s._atticBallHoleObjectId = g_s._selectedObjectId;
					} else if (g_s._atticRodHoleObjectId != 0) {
						g_crep = 188;
					} else {
						g_s._atticRodHoleObjectId = g_s._selectedObjectId;
					}
				}

				if (g_caff == CRYPT) {
					if (g_s._cryptObjectId != 0)
						g_crep = 188;
					else
						g_s._cryptObjectId = g_s._selectedObjectId;
				}

				if (g_caff == WELL)
					g_crep = 185;
				if ((g_caff == CELLAR) || (g_caff == SECRET_PASSAGE))
					g_crep = 124;
			} else {
				g_crep = 124;
				if (g_caff == WELL) {
					tcoord(5);
					if (g_num != 0)
						g_crep = 185;
				}
			}
		}
		if (g_caff == INSIDE_WELL)
			g_crep = 185;
		if ((g_crep == 999) || (g_crep == 185) || (g_crep == 998)) {
			if (g_crep == 999)
				ajchai();
			if (g_crep != 192)
				maivid();
		}
	}
}

/**
 * Engine function - Listen
 * @remarks	Originally called 'tecouter'
 */
void MortevielleEngine::fctListen() {
	if (g_s._currPlace != ROOM26)
		g_crep = 101;
	else {
		if (_currBitIndex != 0)
			++g_s._faithScore;
		int p, rand;
		ecfren(p, rand, g_s._faithScore, g_ment);
		int l = g_ment;
		if (l != 0) {
			if (p != -500) {
				if (rand > p)
					g_crep = 101;
				else {
					becfren(l);
					int j, h, m;
					updateHour(j, h, m);
					rand = getRandomNumber(1, 100);
					if ((h >= 0) && (h < 8)) {
						if (rand > 30)
							g_crep = 101;
						else
							g_crep = 178;
					} else if (rand > 70)
						g_crep = 101;
					else
						g_crep = 178;
				}
			} else
				g_crep = 178;
		}
	}
}

/**
 * Engine function - Eat
 * @remarks	Originally called 'tmanger'
 */
void MortevielleEngine::fctEat() {
	if ((g_s._currPlace > LANDING) && (g_s._currPlace < ROOM26)) {
		g_crep = 148;
	} else {
		tsort();
		g_s._currPlace = DINING_ROOM;
		g_caff = 10;
		debloc(g_s._currPlace);
		_menu.setDestinationMenuText(g_s._currPlace);

		int j, h, m;
		updateHour(j, h, m);
		if ((h == 12) || (h == 13) || (h == 19)) {
			g_s._faithScore -= (g_s._faithScore / 7);
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
			
			g_jh += h;
			g_crep = 135;
			tinke();
		} else {
			g_crep = 134;
		}
	}
}

/**
 * Engine function - Enter
 * @remarks	Originally called 'tentrer'
 */
void MortevielleEngine::fctEnter() {
	if ((g_s._currPlace == MANOR_FRONT) || (g_s._currPlace == MANOR_BACK)) {
		gotoDiningRoom();
		_menu.setDestinationMenuText(g_s._currPlace);
	} else if (g_s._currPlace == LANDING)
		aldepl();
	else if (g_ment == 0)
		g_crep = 997;
	else if ((g_ment == 9) && (g_s._selectedObjectId != 136)) {
			g_crep = 189;
			g_s._teauto[8] = '*';
	} else {
		int z = 0;
		if (!_blo)
			z = t11(g_ment);
		if (z != 0) {
			if ((g_ment == 3) || (g_ment == 7))
				g_crep = 179;
			else {
				g_x = (getRandomNumber(0, 10)) - 5;
				_speechManager.startSpeech(7, g_x, 1);
				aniof(1, 1);
				
				g_x = convertBitIndexToCharacterIndex(z);
				++g_s._faithScore;
				g_s._currPlace = LANDING;
				g_msg[3] = MENU_DISCUSS;
				g_msg[4] = _menu._discussMenu[g_x];
				_syn = true;
				if (g_ment == 9) {
					_col = true;
					g_caff = 70;
					afdes(0);
					repon(2, g_caff);
				} else
					_col = false;
				debloc(g_ment);
				g_ment = 0;
			}
		} else {
			g_x = (getRandomNumber(0, 10)) - 5;
			_speechManager.startSpeech(7, g_x, 1);
			aniof(1, 1);
			
			g_s._currPlace = g_ment;
			affrep();
			debloc(g_s._currPlace);
			_menu.setDestinationMenuText(g_s._currPlace);
			g_ment = 0;
			g_mpers = 0;
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

	if ((g_s._currPlace > LANDING) && (g_s._currPlace < ROOM26)) {
		g_crep = 148;
		return;
	}
	if (g_s._currPlace != OWN_ROOM) {
		tsort();
		g_s._currPlace = OWN_ROOM;
		affrep();
		afdes(0);
		debloc(g_s._currPlace);
		_menu.setDestinationMenuText(g_s._currPlace);
	}
	clearScreenType3();
	clearScreenType2();
	prepareScreenType2();
	ecr2(getEngineString(S_WANT_TO_WAKE_UP));
	updateHour(j, h, m);

	int answer;
	do {
		if (h < 8) {
			g_s._faithScore -= (g_s._faithScore / 20);
			z = (7 - h) * 2;
			if (m == 30)
				--z;
			g_jh += z;
			h = 7;
		}
		g_jh += 2;
		++h;
		if (h > 23)
			h = 0;
		tinke();
		answer = Alert::show(getEngineString(S_YES_NO), 1);
		_anyone = false;
	} while (answer != 1);
	g_crep = 998;
	g_num = 0;
}

/**
 * Engine function - Force
 * @remarks	Originally called 'tdefoncer'
 */
void MortevielleEngine::fctForce() {
	if (!_syn)
		ecr3(getEngineString(S_SMASH));
	if (g_caff < 25)
		tfleche();

	if ((!_anyone) && (!_keyPressedEsc)) {
		if (g_s._currPlace != ROOM26)
			g_crep = 997;
		else {
			g_crep = 143;
			g_s._faithScore += 2;
		}
	}
}

/**
 * Engine function - Leave
 * @remarks	Originally called 'tsortir'
 */
void MortevielleEngine::fctLeave() {
	tsort();
	g_crep = 0;
	if ((g_s._currPlace == MOUNTAIN) || (g_s._currPlace == MANOR_FRONT) || (g_s._currPlace == MANOR_BACK) || (g_s._currPlace == WELL))
		g_crep = 997;
	else {
		int nextPlace = OWN_ROOM;

		if ((g_s._currPlace < CRYPT) || (g_s._currPlace == ROOM26))
			nextPlace = DINING_ROOM;
		else if ((g_s._currPlace == DINING_ROOM) || (g_s._currPlace == CHAPEL))
			nextPlace = MANOR_FRONT;
		else if ((g_s._currPlace < DINING_ROOM) || (g_s._currPlace == ATTIC))
			nextPlace = LANDING;
		else if (g_s._currPlace == CRYPT) {
			nextPlace = SECRET_PASSAGE;
			g_crep = 176;
		} else if (g_s._currPlace == SECRET_PASSAGE)
			nextPlace = t23coul();
		else if (g_s._currPlace == INSIDE_WELL)
			nextPlace = WELL;

		if (g_crep != 997)
			g_s._currPlace = nextPlace;

		g_caff = nextPlace;
		if (g_crep == 0)
			g_crep = nextPlace;
		debloc(nextPlace);
		_menu.setDestinationMenuText(nextPlace);
	}
}

/**
 * Engine function - Wait
 * @remarks	Originally called 'tattendre'
 */
void MortevielleEngine::fctWait() {
	g_mpers = 0;
	clearScreenType3();

	int answer;
	do {
		++g_jh;
		tinke();
		if (!_blo)
			t11(g_s._currPlace);
		if ((_currBitIndex != 0) && (g_mpers == 0)) {
			g_crep = 998;
			if ((g_s._currPlace == ATTIC) || (g_s._currPlace == CELLAR))
				cavegre();
			if ((g_s._currPlace > OWN_ROOM) && (g_s._currPlace < DINING_ROOM))
				_anyone = true;
			g_mpers = _currBitIndex;
			if (!_anyone)
				tinke();
			return;
		}
		repon(2, 102);
		answer = Alert::show(getEngineString(S_YES_NO), 1);
	} while (answer != 2);
	g_crep = 998;
	if (!_anyone)
		tinke();
}

/**
 * Engine function - Sound
 * @remarks	Originally called 'tsonder'
 */
void MortevielleEngine::fctSound() {
	if (!_syn)
		ecr3(getEngineString(S_PROBE2));
	if (g_caff < 27) {
		tfleche();
		if (!(_anyone) && (!_keyPressedEsc))
			g_crep = 145;
		g_num = 0;
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
		} while (_menu._discussMenu[cx] != g_msg[4]);
		g_caff = 69 + cx;
		afdes(0);
		repon(2, g_caff);
		suj = g_caff + 60;
	}
	tkey1(false);
	mennor();
	hideMouse();
	hirs();
	premtet();
	sparl(0, suj);
	hirs();
	for (int ix = 1; ix <= 46; ++ix)
		te[ix] = false;
	for (int ix = 1; ix <= 45; ++ix) {
		lib[ix] = deline(ix + kQuestionStringIndex);
		for (i = lib[ix].size(); i <= 40; ++i)
			lib[ix] = lib[ix] + ' ';
	}
	lib[46] = lib[45];
	lib[45] = ' ';
	showMouse();
	do {
		choi = 0;
		icm = 0;
		co = 0;
		lig = 0;
		do {
			++icm;
			_screenSurface.putxy(co, lig);
			if (g_s._teauto[icm] == '*') {
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
			moveMouse(f, tou);
			CHECK_QUIT;

			getMousePos_(x, y, c);
			x *= (3 - g_res);
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
					if ((g_s._teauto[ix] == '*') || (ix == 46)) {
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
				g_s._currPlace = 15;
				if (g_iouv > 0)
					max = 8;
				else
					max = 4;
				if (getRandomNumber(1, max) == 2)
					suj = 129;
				else {
					suj = 138;
					g_s._faithScore += (3 * (g_s._faithScore / 10));
				}
			} else if (g_nbrep[g_caff - 69] < g_nbrepm[g_caff - 69]) {
				suj = g_tabdon[arep + (ix << 3) + (g_caff - 70)];
				g_s._faithScore += g_tabdon[arcf + ix];
				++g_nbrep[g_caff - 69];
			} else {
				g_s._faithScore += 3;
				suj = 139;
			}
			hideMouse();
			hirs();
			premtet();
			sparl(0, suj);
			showMouse();
			if ((suj == 84) || (suj == 86)) {
				g_s._pourc[5] = '*';
				g_s._teauto[7] = '*';
			}
			if ((suj == 106) || (suj == 108) || (suj == 94)) {
				for (int indx = 29; indx <= 31; ++indx)
					g_s._teauto[indx] = '*';
				g_s._pourc[7] = '*';
			}
			if (suj == 70) {
				g_s._pourc[8] = '*';
				g_s._teauto[32] = '*';
			}
			hideMouse();
			hirs();
			showMouse();
		}
	} while ((choi != 46) && (suj != 138));
	if (_col) {
		g_s._faithScore += (3 * (g_s._faithScore / 10));
		hideMouse();
		hirs();
		premtet();
		sparl(0, 138);
		showMouse();
		_col = false;
		g_s._currPlace = LANDING;
	}
	g_ctrm = 0;
	hideMouse();
	hirs();
	drawRightFrame();
	showMouse();
	showPeoplePresent(_currBitIndex);
	tinke();
	drawClock();
	affrep();
	/* chech;*/
	_menu.setDestinationMenuText(g_s._currPlace);
	clearScreenType3();
}

/**
 * Engine function - Smell
 * @remarks	Originally called 'tsentir'
 */
void MortevielleEngine::fctSmell() {
	g_crep = 119;
	if (g_caff < ROOM26) {
		if (!_syn)
			ecr3(getEngineString(S_SMELL));
		tfleche();
		if (!(_anyone) && !(_keyPressedEsc))
			if (g_caff == 16)
				g_crep = 153;
	} else if (g_caff == 123)
		g_crep = 110;
	g_num = 0;
}

/**
 * Engine function - Scratch
 * @remarks	Originally called 'tgratter'
 */
void MortevielleEngine::fctScratch() {
	g_crep = 155;
	if (g_caff < 27) {
		if (!_syn)
			ecr3(getEngineString(S_SCRATCH));
		tfleche();
	}
	g_num = 0;
}

/**
 * The game is over
 * @remarks	Originally called 'tmaj1'
 */
void MortevielleEngine::endGame() {
	_quitGame = true;
	tlu(13, 152);
	maivid();
	clearScreenType1();
	clearScreenType2();
	clearScreenType3();
	repon(9, 1509);
	tkey1(false);
	hideMouse();
	g_caff = 70;
	taffich();
	hirs();
	premtet();
	sparl(0, 141);
	showMouse();
	clearScreenType1();
	repon(9, 1509);
	repon(2, 142);
	tkey1(false);
	g_caff = 32;
	afdes(0);
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
	maivid();
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
