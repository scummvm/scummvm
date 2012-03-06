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
#include "mortevielle/actions.h"
#include "mortevielle/dialogs.h"
#include "mortevielle/level15.h"
#include "mortevielle/menu.h"
#include "mortevielle/mor.h"
#include "mortevielle/mor2.h"
#include "mortevielle/mortevielle.h"
#include "mortevielle/mouse.h"
#include "mortevielle/outtext.h"
#include "mortevielle/parole2.h"
#include "mortevielle/taffich.h"
#include "mortevielle/var_mor.h"

namespace Mortevielle {


/**
 * Engine function - Move
 * @remarks	Originally called 'taller'
 */
void fctMove() {
	int cx;

	if ((g_s._currPlace == 26) && (g_msg[4] == g_vm->_menu._moveMenu[6])) {
		g_s._currPlace = LANDING;
		g_caff = g_s._currPlace;
		afdes(0);
		repon(2, g_s._currPlace);
	}
	if ((g_s._currPlace == LANDING) && (g_msg[4] == g_vm->_menu._moveMenu[6])) {
		if (!g_syn)
			ecr3(g_vm->getEngineString(S_GO_TO));
		tfleche();
		if (g_iesc)
			g_okdes = false;
		if ((g_anyone) || (g_iesc))
			return;
		tcoord(1);
		if (g_num == 0)
			return;
		if (g_num == 1) {
			g_s._currPlace = OWN_ROOM;
			tmlieu(0);
		} else if (g_num == 7) {
			g_s._currPlace = ATTIC;
			tmlieu(13);
		} else if (g_num != 6)
			g_s._currPlace = 26;
		if ((g_num > 1) && (g_num < 6))
			g_ment = g_num - 1;
		if (g_num > 7)
			g_ment = g_num - 3;
		if (g_num != 6)
			affrep();
		else
			aldepl();
		return;
	}
	tsort();
	cx = 0;
	do {
		++cx;
	} while (g_vm->_menu._moveMenu[cx] != g_msg[4]);

	if (g_s._currPlace == MOUNTAIN) {
		if (cx == 1)
			t1deva();
		if (cx == 2)
			t1neig();
		goto L2;
	}

	if (g_s._currPlace == 23) {
		if (cx == 1)
			t1deau();
		if (cx == 2)
			t1derr();
		goto L2;
	}

	if ((g_s._currPlace == BUREAU) && (cx == 1))
		cx = 6;

	if (g_s._currPlace == KITCHEN) {
		if (cx == 2)
			cx = 6;
		if (cx == 5)
			cx = 16;
	}

	if ((g_s._currPlace == CELLAR) && (cx == 3))
		cx = 6;
	if (((g_s._currPlace == LANDING) || (g_s._currPlace == 26)) && (cx == 4))
		cx = 6;
	if ((g_s._currPlace > MOUNTAIN) && (g_s._currPlace != 26))
		cx += 10;
	if ((g_s._currPlace == CHAPEL) && (cx == 13))
		cx = 16;
	if (g_s._currPlace == MANOR_FRONT) {
		if (cx == 12)
			cx = 16;
		else if (cx > 13)
			cx = 15;
	}
	if ((g_s._currPlace == MANOR_BACK) && (cx > 14))
		cx = 15;
	if ((g_s._currPlace == WELL) && (cx != 17))
		if (cx > 13)
			cx = 15;
	if (cx == 1)
		g_s._currPlace = BUREAU;
	else if (cx == 2)
		g_s._currPlace = KITCHEN;
	else if (cx == 3)
		g_s._currPlace = CELLAR;
	else if (cx == 4)
		g_s._currPlace = LANDING;
	else if (cx == 5)
		cx = 12;
	if (cx == 6)
		cx = 11;
	if (cx == 11)
		t1sama();
	else if (cx == 12)
		t1deva();
	else if (cx == 13)
		g_s._currPlace = CHAPEL;
	else if (cx == 14)
		g_s._currPlace = WELL;
	else if (cx == 15)
		t1neig();
	else if (cx == 16)
		t1derr();
	else if (cx == 17) {
		if ((g_s._wellObjectId != 120) && (g_s._wellObjectId != 140))
			g_crep = 997;
		else if (g_s._wellObjectId == 120)
			g_crep = 181;
		else if (g_s._faithScore > 80) {
			g_crep = 1505;
			tperd();
		} else {
			g_s._currPlace = 23;
			affrep();
		}
	}
	if ((cx < 5) || (cx == 13) || (cx == 14))
		affrep();
	debloc(g_s._currPlace);
L2:
	tmlieu(g_s._currPlace);
}

/**
 * Engine function - Take
 * @remarks	Originally called 'tprendre'
 */
void fctTake() {
	if (g_caff > 99) {
		int cx = g_caff;
		avpoing(cx);
		if (g_crep != 139) {
			if (g_ipers > 0)
				g_s._faithScore += 3;
			if (g_obpart) {
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
				unsetSearchMenu();
				g_obpart = false;
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
	if (!g_syn)
		ecr3(g_vm->getEngineString(S_TAKE));
	tfleche();
	if ((g_anyone) || (g_iesc))
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
			if (g_ipers > 0)
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
		if ( ((g_s._currPlace == OWN_ROOM)  && (g_num == 3)) || ((g_s._currPlace == GREEN_ROOM)  && (g_num == 4))
		  || ((g_s._currPlace == PURPLE_ROOM)  && (g_num == 1)) || ((g_s._currPlace == DARKBLUE_ROOM)  && (g_num == 3))
		  || ((g_s._currPlace == BLUE_ROOM)  && (g_num == 6)) || ((g_s._currPlace == RED_ROOM)  && (g_num == 2))
		  || ((g_s._currPlace == BATHROOM)  && (g_num == 6)) || ((g_s._currPlace == GREEN_ROOM2)  && (g_num == 4))
		  || ((g_s._currPlace == 9)  && (g_num == 4)) || ((g_s._currPlace == DINING_ROOM) && (g_num > 2))
		  || ((g_s._currPlace == BUREAU) && (g_num == 7)) || ((g_s._currPlace == KITCHEN) && (g_num == 6))
		  || ((g_s._currPlace == ATTIC) && (g_num > 4))  || ((g_s._currPlace > ATTIC)  && (g_s._currPlace != 23)) )
		  g_crep = 997;
		else if (g_s._currPlace == 23) {
			g_crep = 1504;
			tperd();
		} else
			g_crep = 120;
	}
}
/**
 * Engine function - Inventory / Take
 * @remarks	Originally called 'tsprendre'
 */
void fctInventoryTake() {
	int cx, cy, cz;

	cx = 0;
	do {
		++cx;
	} while (g_vm->_menu._inventoryMenu[cx] != g_msg[4]);
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
	clsf2();
}

/**
 * Engine function - Lift
 * @remarks	Originally called 'tsoulever'
 */
void fctLift() {
//	int cx;

	if (!g_syn)
		ecr3(g_vm->getEngineString(S_LIFT));
	tfleche();
	if ((g_anyone) || (g_iesc))
		return;
	tcoord(3);
	if (g_num == 0) {
		tcoord(8);
		if (g_num != 0) {
			if (g_ipers > 0)
				++g_s._faithScore;
			g_crep = 997;
			if ((g_s._currPlace == PURPLE_ROOM) && (g_s._purpleRoomObjectId != 0))
				treg(g_s._purpleRoomObjectId);
		}
		return;
	}
	if (g_ipers > 0)
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
void fctRead() {
	if (g_caff > 99)
		st4(g_caff);
	else {
		if (!g_syn)
			ecr3(g_vm->getEngineString(S_READ));
		tfleche();
		if (!(g_anyone) && !(g_iesc)) {
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
void fctSelfRead() {
	if (g_s._selectedObjectId == 0)
		g_crep = 186;
	else
		st4(g_s._selectedObjectId);
}

/**
 * Engine function - Look
 * @remarks	Originally called 'tregarder'
 */
void fctLook() {
	int cx;

	if (g_caff > 99) {
		g_crep = 103;
		return;
	}
	if (!g_syn)
		ecr3(g_vm->getEngineString(S_LOOK));
	tfleche();
	if ((g_anyone) || (g_iesc))
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
	if (g_s._currPlace == 26)
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
void fctSelftLook() {
	if (g_s._selectedObjectId != 0)
		treg(g_s._selectedObjectId);
	else
		g_crep = 186;
}

/**
 * Engine function - Search
 * @remarks	Originally called 'tfouiller'
 */
void fctSearch() {
	const byte r[14] = {123, 104, 123, 131, 131, 123, 104, 131, 123, 123, 106, 123, 123, 107};

	if (g_caff > 99) {
		st7(g_caff);
		return;
	}

	if (!g_syn)
		ecr3(g_vm->getEngineString(S_SEARCH));

	tfleche();
	if (g_anyone || g_iesc)
		return;

	if (g_s._currPlace == 23) {
		g_crep = 1504;
		tperd();
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
				if (g_ipers > 0)
					g_s._faithScore += 3;

				rechai(g_mchai);
				if (g_mchai != 0) {
					g_cs = 0;
					g_is = 0;
					g_heroSearching = true;
					setSearchMenu();
					tsuiv();
				} else
					g_crep = 997;
			}
		} else {
			tcoord(8);
			g_crep = 997;
			if (g_num != 0) {
				if (g_ipers > 0)
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
		if (g_ipers > 0)
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
void fctSelfSearch() {
	if (g_s._selectedObjectId != 0)
		st7(g_s._selectedObjectId);
	else
		g_crep = 186;
}

/**
 * Engine function - Open
 * @remarks	Originally called 'touvrir'
 */
void fctOpen() {
	if (!g_syn)
		ecr3(g_vm->getEngineString(S_OPEN));

	if (g_caff == 26) {
		if (g_ment != 0) {
			g_msg[4] = OPCODE_ENTER;
			g_syn = true;
		} else
			g_crep = 997;
		return;
	}

	if (g_caff == 15) {
		aldepl();
		return;
	}

	tfleche();
	if ((g_anyone) || (g_iesc))
		return;

	tcoord(7);
	if (g_num != 0) {
		if (g_ipers > 0)
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
						parole(7, 9, 1);
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
void fctPlace() {
	if (g_s._selectedObjectId == 0) {
		g_crep = 186;
		return;
	}

	if (!g_syn)
		ecr3(g_vm->getEngineString(S_PUT));

	tfleche();
	if (g_iesc)
		g_crep = 998;

	if ((g_anyone) || (g_iesc))
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
					parole(6, -9, 1);

					// Do you want to enter the hidden passage?
					int answer = Alert::show(g_vm->getEngineString(S_YES_NO), 1);
					if (answer == 1) {
						Common::String alertTxt = deline(582);
						Alert::show(alertTxt, 1);

						bool enterPassageFl = KnowledgeCheck::show();
						hideMouse();
						hirs();
						dessine_rouleau();
						clsf2();
						clsf3();
						showMouse();
						tinke();
						drawClock();
						if (g_ipers != 0)
							showPeoplePresent(g_ipers);
						else 
							displayAloneText();

						g_vm->_menu.displayMenu();
						if (enterPassageFl) {
							g_s._currPlace = SECRET_PASSAGE;
							tmlieu(17);
						} else {
							tmlieu(g_s._currPlace);
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
				tperd();
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
void fctTurn() {
	if (g_caff > 99) {
		g_crep = 149;
		return;
	}
	if (!g_syn)
		ecr3(g_vm->getEngineString(S_TURN));
	tfleche();
	if ((g_anyone) || (g_iesc))
		return;
	tcoord(9);
	if (g_num != 0) {
		g_crep = 997;
		if ((g_s._currPlace == ATTIC) && (g_s._atticRodHoleObjectId == 159) && (g_s._atticBallHoleObjectId == 141)) {
			repon(2, 167);
			parole(7, 9, 1);
			int answer = Alert::show(g_vm->getEngineString(S_YES_NO), 1);
			if (answer == 1)
				g_vm->_endGame = true;
			else
				g_crep = 168;
		}
		if ((g_s._currPlace == SECRET_PASSAGE) && (g_s._secretPassageObjectId == 143)) {
			repon(2, 175);
			clsf3();
			parole(6, -9, 1);
			int answer = Alert::show(g_vm->getEngineString(S_YES_NO), 1);
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
void fctSelfHide() {
	if (!g_syn)
		ecr3(g_vm->getEngineString(S_HIDE_SELF));
	tfleche();
	if (!(g_anyone) && !(g_iesc)) {
		tcoord(10);
		if (g_num == 0)
			g_cache = false;
		else {
			g_cache = true;
			g_crep = 999;
		}
	}
}

/**
 * Engine function - Attach
 * @remarks	Originally called 'tattacher'
 */
void fctAttach() {
	if (g_s._selectedObjectId == 0)
		g_crep = 186;
	else {
		if (!g_syn)
			ecr3(g_vm->getEngineString(S_TIE));
		tfleche();
		if (!(g_anyone) && !(g_iesc)) {
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
void fctClose() {
	if (!g_syn)
		ecr3(g_vm->getEngineString(S_CLOSE));

	if (g_caff < 26) {
		tfleche();
		if (g_iesc)
			g_crep = 998;
		if ((g_anyone) || (g_iesc))
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
	if (g_caff == 26)
		g_crep = 999;
}

/**
 * Engine function - Knock
 * @remarks	Originally called 'tfrapper'
 */
void fctKnock() {
	warning("Knock - _currPlace %d", g_s._currPlace);

	if (!g_syn)
		ecr3(g_vm->getEngineString(S_HIT));

	if (g_s._currPlace == LANDING) {
		Alert::show(g_vm->getEngineString(S_BEFORE_USE_DEP_MENU), 1);
		return;
	}

	if (g_s._currPlace < DOOR) {
		tfleche();
		if (!(g_anyone) && !(g_iesc)) {
			if ((g_s._currPlace < MOUNTAIN) && (g_s._currPlace != LANDING))
				g_crep = 133;
			else
				g_crep = 997;
		}

		return;
	}

	if (g_s._currPlace == 26) {
		int rand = (getRandomNumber(0, 8)) - 4;
		parole(11, rand, 1);
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
void fctSelfPut() {
	if (!g_syn)
		ecr3(g_vm->getEngineString(S_POSE));
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
		if ((g_anyone) || (g_iesc))
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
		if (g_caff == 23)
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
void fctListen() {
	if (g_s._currPlace != 26)
		g_crep = 101;
	else {
		if (g_ipers != 0)
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
void fctEat() {
	if ((g_s._currPlace > LANDING) && (g_s._currPlace < 26)) {
		g_crep = 148;
	} else {
		tsort();
		g_s._currPlace = DINING_ROOM;
		g_caff = 10;
		debloc(g_s._currPlace);
		tmlieu(g_s._currPlace);

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
void fctEnter() {
	if ((g_s._currPlace == MANOR_FRONT) || (g_s._currPlace == MANOR_BACK)) {
		t1sama();
		tmlieu(g_s._currPlace);
	} else if (g_s._currPlace == LANDING)
		aldepl();
	else if (g_ment == 0)
		g_crep = 997;
	else if ((g_ment == 9) && (g_s._selectedObjectId != 136)) {
			g_crep = 189;
			g_s._teauto[8] = '*';
	} else {
		int z = 0;
		if (!g_blo)
			t11(g_ment, z);
		if (z != 0) {
			if ((g_ment == 3) || (g_ment == 7))
				g_crep = 179;
			else {
				g_x = (getRandomNumber(0, 10)) - 5;
				parole(7, g_x, 1);
				aniof(1, 1);
				
				g_x = convertBitIndexToCharacterIndex(z);
				++g_s._faithScore;
				g_s._currPlace = LANDING;
				g_msg[3] = MENU_DISCUSS;
				g_msg[4] = g_vm->_menu._discussMenu[g_x];
				g_syn = true;
				if (g_ment == 9) {
					g_col = true;
					g_caff = 70;
					afdes(0);
					repon(2, g_caff);
				} else
					g_col = false;
				debloc(g_ment);
				g_ment = 0;
			}
		} else {
			g_x = (getRandomNumber(0, 10)) - 5;
			parole(7, g_x, 1);
			aniof(1, 1);
			
			g_s._currPlace = g_ment;
			affrep();
			debloc(g_s._currPlace);
			tmlieu(g_s._currPlace);
			g_ment = 0;
			g_mpers = 0;
			g_ipers = 0;
		}
	}
}

/**
 * Engine function - Sleep
 * @remarks	Originally called 'tdormir'
 */
void fctSleep() {
	int z, j, h, m;

	if ((g_s._currPlace > LANDING) && (g_s._currPlace < 26)) {
		g_crep = 148;
		return;
	}
	if (g_s._currPlace != OWN_ROOM) {
		tsort();
		g_s._currPlace = OWN_ROOM;
		affrep();
		afdes(0);
		debloc(g_s._currPlace);
		tmlieu(g_s._currPlace);
	}
	clsf3();
	clsf2();
	ecrf2();
	ecr2(g_vm->getEngineString(S_WANT_TO_WAKE_UP));
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
		answer = Alert::show(g_vm->getEngineString(S_YES_NO), 1);
		g_anyone = false;
	} while (answer != 1);
	g_crep = 998;
	g_num = 0;
}

/**
 * Engine function - Force
 * @remarks	Originally called 'tdefoncer'
 */
void fctForce() {
	if (!g_syn)
		ecr3(g_vm->getEngineString(S_SMASH));
	if (g_caff < 25)
		tfleche();

	if ((!g_anyone) && (!g_iesc)) {
		if (g_s._currPlace != 26)
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
void fctLeave() {
	tsort();
	g_crep = 0;
	if ((g_s._currPlace == MOUNTAIN) || (g_s._currPlace == MANOR_FRONT) || (g_s._currPlace == MANOR_BACK) || (g_s._currPlace == WELL))
		g_crep = 997;
	else {
		int lx = 0;

		if ((g_s._currPlace < CRYPT) || (g_s._currPlace == 26))
			lx = 10;
		if ((g_s._currPlace == DINING_ROOM) || (g_s._currPlace == CHAPEL))
			lx = 21;
		if ((g_s._currPlace < DINING_ROOM) || (g_s._currPlace == ATTIC))
			lx = 15;
		if (g_s._currPlace == CRYPT) {
			lx = 17;
			g_crep = 176;
		}
		if (g_s._currPlace == SECRET_PASSAGE)
			t23coul(lx);
		if (g_s._currPlace == 23)
			lx = 24;
		if (g_crep != 997)
			g_s._currPlace = lx;
		g_caff = lx;
		if (g_crep == 0)
			g_crep = lx;
		debloc(lx);
		tmlieu(lx);
	}
}

/**
 * Engine function - Wait
 * @remarks	Originally called 'tattendre'
 */
void fctWait() {
	int quel;

	g_mpers = 0;
	clsf3();

	int answer;
	do {
		++g_jh;
		tinke();
		if (!g_blo)
			t11(g_s._currPlace, quel);
		if ((g_ipers != 0) && (g_mpers == 0)) {
			g_crep = 998;
			if ((g_s._currPlace == ATTIC) || (g_s._currPlace == CELLAR))
				cavegre();
			if ((g_s._currPlace > OWN_ROOM) && (g_s._currPlace < DINING_ROOM))
				g_anyone = true;
			g_mpers = g_ipers;
			if (!g_anyone)
				tinke();
			return;
		}
		repon(2, 102);
		answer = Alert::show(g_vm->getEngineString(S_YES_NO), 1);
	} while (answer != 2);
	g_crep = 998;
	if (!g_anyone)
		tinke();
}

/**
 * Engine function - Sound
 * @remarks	Originally called 'tsonder'
 */
void fctSound() {
	if (!g_syn)
		ecr3(g_vm->getEngineString(S_PROBE2));
	if (g_caff < 27) {
		tfleche();
		if (!(g_anyone) && (!g_iesc))
			g_crep = 145;
		g_num = 0;
	}
}

/**
 * Engine function - Discuss
 * @remarks	Originally called 'tparler'
 */
void fctDiscuss() {
	bool te[47];
	int cy, cx, max, suj, co, lig, icm, i, choi, x, y, c;
	char tou;
	Common::String lib[47];
	bool f;

	endSearch();
	if (g_col)
		suj = 128;
	else {
		cx = 0;
		do {
			++cx;
		} while (g_vm->_menu._discussMenu[cx] != g_msg[4]);
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
			g_vm->_screenSurface.putxy(co, lig);
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
		g_vm->_screenSurface.putxy(320, 176);
		writetp(lib[46], 0);
		tou = '\0';
		do {
			moveMouse(f, tou);
			CHECK_QUIT;

			getMousePos(x, y, c);
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
					g_vm->_screenSurface.putxy(co, lig);
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
					ix = ix + 23;
				if (ix != choi) {
					if (choi != 0) {
						lig = ((choi - 1) % 23) << 3;
						if (choi > 23)
							co = 320;
						else
							co = 0;
						g_vm->_screenSurface.putxy(co, lig);
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
						g_vm->_screenSurface.putxy(co, lig);
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
		} while (!((tou == '\15') || (((c != 0) || g_vm->getMouseClick()) && (choi != 0))));
		g_vm->setMouseClick(false);
		if (choi != 46) {
			int ix = choi - 1;
			if (g_col) {
				g_col = false;
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
	if (g_col) {
		g_s._faithScore += (3 * (g_s._faithScore / 10));
		hideMouse();
		hirs();
		premtet();
		sparl(0, 138);
		showMouse();
		g_col = false;
		g_s._currPlace = 15;
	}
	g_ctrm = 0;
	hideMouse();
	hirs();
	dessine_rouleau();
	showMouse();
	showPeoplePresent(g_ipers);
	tinke();
	drawClock();
	affrep();
	/* chech;*/
	tmlieu(g_s._currPlace);
	clsf3();
}

/**
 * Engine function - Smell
 * @remarks	Originally called 'tsentir'
 */
void fctSmell() {
	g_crep = 119;
	if (g_caff < 26) {
		if (!g_syn)
			ecr3(g_vm->getEngineString(S_SMELL));
		tfleche();
		if (!(g_anyone) && !(g_iesc))
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
void fctScratch() {
	g_crep = 155;
	if (g_caff < 27) {
		if (!g_syn)
			ecr3(g_vm->getEngineString(S_SCRATCH));
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
	clsf1();
	clsf2();
	clsf3();
	repon(9, 1509);
	tkey1(false);
	hideMouse();
	g_caff = 70;
	taffich();
	hirs();
	premtet();
	sparl(0, 141);
	showMouse();
	clsf1();
	repon(9, 1509);
	repon(2, 142);
	tkey1(false);
	g_caff = 32;
	afdes(0);
	repon(6, 34);
	repon(2, 35);
	musique(0);
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
void MortevielleEngine::loseGame() {
	clsf2();
	musique(0);
	tkey1(false);
	maivid();
	resetVariables();
	dprog();
	g_vh = 10;
	g_vm__ = 0;
	g_vj = 0;
	g_minute = 0;
	g_hour = 10;
	g_day = 0;
	repon(2, 180);

	int answer = Alert::show(g_vm->getEngineString(S_YES_NO), 1);
	_quitGame = (answer != 1);
}

} // End of namespace Mortevielle
