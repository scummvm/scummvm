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

#include "mortevielle/actions.h"
#include "mortevielle/mor.h"
#include "mortevielle/mor2.h"

namespace Mortevielle {

/* NIVEAU 4 */
const char stouinon[] = "[2][ ][OUI][NON]";

/* overlay */
void taller() {
	int mx, cx, cy;

	if ((s.mlieu == 26) && (msg[4] == depl[6])) {
		s.mlieu = 15;
		caff = s.mlieu;
		afdes(0);
		repon(2, s.mlieu);
	}
	if ((s.mlieu == 15) && (msg[4] == depl[6])) {
		if (! syn)  ecr3("aller");
		tfleche();
		if (iesc)  okdes = false;
		if ((anyone) || (iesc))  return;
		tcoord(1);
		if (num == 0)  return;
		if (num == 1) {
			s.mlieu = 0;
			tmlieu(0);
		} else if (num == 7) {
			s.mlieu = 13;
			tmlieu(13);
		} else if (num != 6)  s.mlieu = 26;
		if ((num > 1) && (num < 6))  ment = num - 1;
		if (num > 7)  ment = num - 3;
		if (num != 6)  affrep();
		else aldepl();
		return;
	}
	tsort();
	cx = 0;
	do {
		cx = cx + 1;
	} while (!(depl[cx] == msg[4]));
	if (s.mlieu == 19) {
		if (cx == 1)  t1deva();
		if (cx == 2)  t1neig();
		goto L2;
	}
	if (s.mlieu == 23) {
		if (cx == 1)  t1deau();
		if (cx == 2)  t1derr();
		goto L2;
	}
	if ((s.mlieu == 11) && (cx == 1))  cx = 6;
	if (s.mlieu == 12) {
		if (cx == 2)  cx = 6;
		if (cx == 5)  cx = 16;
	}
	if ((s.mlieu == 14) && (cx == 3))  cx = 6;
	if (((s.mlieu == 15) || (s.mlieu == 26)) && (cx == 4))  cx = 6;
	if ((s.mlieu > 19) && (s.mlieu != 26))  cx = cx + 10;
	if ((s.mlieu == 20) && (cx == 13))  cx = 16;
	if (s.mlieu == 21) {
		if (cx == 12)  cx = 16;
		else if (cx > 13)  cx = 15;
	}
	if ((s.mlieu == 22) && (cx > 14))  cx = 15;
	if ((s.mlieu == 24) && (cx != 17))
		if (cx > 13)  cx = 15;
	if (cx == 1)  s.mlieu = 11;
	else if (cx == 2)  s.mlieu = 12;
	else if (cx == 3)  s.mlieu = 14;
	else if (cx == 4)  s.mlieu = 15;
	else if (cx == 5)  cx = 12;
	if (cx == 6)  cx = 11;
	if (cx == 11)  t1sama();
	else if (cx == 12)  t1deva();
	else if (cx == 13)  s.mlieu = 20;
	else if (cx == 14)  s.mlieu = 24;
	else if (cx == 15)  t1neig();
	else if (cx == 16)  t1derr();
	else if (cx == 17) {
		if ((s.ipuit != 120) && (s.ipuit != 140))  crep = 997;
		else if (s.ipuit == 120)  crep = 181;
		else if (s.conf > 80) {
			crep = 1505;
			tperd();
		} else {
			s.mlieu = 23;
			affrep();
		}
	}
	if ((cx < 5) || (cx == 13) || (cx == 14))  affrep();
	debloc(s.mlieu);
L2:
	tmlieu(s.mlieu);
}

/* overlay */ void tprendre() {
	int cx, cy, cz;

	if (caff > 99) {
		cx = caff;
		avpoing(cx);
		if (crep != 139) {
			if (ipers > 0)  s.conf = s.conf + 3;
			if (obpart) {
				if (s.mlieu == 2)  s.iloic = 0;
				if (s.mlieu == 13) {
					if (s.iboul == caff)  s.iboul = 0;
					if (s.ibag == caff)  s.ibag = 0;
				}
				if (s.mlieu == 14)  s.icave = 0;
				if (s.mlieu == 16)  s.icryp = 0;
				if (s.mlieu == 17)  s.ivier = 0;
				if (s.mlieu == 24)  s.ipuit = 0;
				mfouen();
				obpart = false;
				affrep();
			} else {
				tabdon[acha + (pred(int, mchai) * 10) + pred(int, cs)] = 0;
				tsuiv();
				dobj = dobj + 1;
				if (dobj > 6) {
					s.conf = s.conf + 2;
					dobj = 0;
				}
			}
		}
		return;
	}
	if (! syn)  ecr3("prendre");
	tfleche();
	if ((anyone) || (iesc))  return;
	if (caff == 3) {
		tcoord(2);
		if (num == 1) {
			crep = 152;
			return;
		}
	}
	tcoord(5);
	if ((num == 0) || ((num == 1) && (s.mlieu == 16))) {
		tcoord(8);
		if (num != 0) {
			if (ipers > 0)  s.conf = s.conf + 3;
			crep = 997;
			if ((s.mlieu == 2) && (s.iloic != 0))  avpoing(s.iloic);
			if ((s.mlieu == 13) && (num == 1) && (s.iboul != 0)) {
				avpoing(s.iboul);
				if ((crep != 997) && (crep != 139))  aniof(2, 7);
			}
			if ((s.mlieu == 13) && (num == 2) && (s.ibag != 0)) {
				avpoing(s.ibag);
				if ((crep != 997) && (crep != 139))  aniof(2, 6);
			}
			if ((s.mlieu == 14) && (s.icave != 0)) {
				avpoing(s.icave);
				if ((crep != 997) && (crep != 139))  aniof(2, 2);
			}
			if ((s.mlieu == 16) && (s.icryp != 0))  avpoing(s.icryp);
			if ((s.mlieu == 17) && (s.ivier != 0)) {
				avpoing(s.ivier);
				if ((crep != 997) && (crep != 139)) {
					crep = 182;
					aniof(2, 1);
				}
			}
			if ((s.mlieu == 24) && (s.ipuit != 0)) {
				avpoing(s.ipuit);
				if ((crep != 997) && (crep != 139))  aniof(2, 1);
			}
			if ((crep != 997) && (crep != 182) && (crep != 139))  crep = 999;
		}
	} else {
		if (((s.mlieu == 0) && (num == 3)) || ((s.mlieu == 1) && (num == 4))
		        || ((s.mlieu == 2) && (num == 1)) || ((s.mlieu == 4) && (num == 3))
		        || ((s.mlieu == 5) && (num == 6)) || ((s.mlieu == 6) && (num == 2))
		        || ((s.mlieu == 7) && (num == 6)) || ((s.mlieu == 8) && (num == 4))
		        || ((s.mlieu == 9) && (num == 4)) || ((s.mlieu == 10) && (num > 2))
		        || ((s.mlieu == 11) && (num == 7)) || ((s.mlieu == 12) && (num == 6))
		        || ((s.mlieu == 13) && (num > 4)) || ((s.mlieu > 13)
		                && (s.mlieu != 23)))  crep = 997;
		else {
			if (s.mlieu == 23) {
				crep = 1504;
				tperd();
			} else crep = 120;
		}
	}
}

/* overlay */ void tsprendre() {
	int cx, cy, cz;

	cx = 0;
	do {
		cx = cx + 1;
	} while (!(invt[cx] == msg[4]));
	cz = 0;
	cy = 0;
	do {
		cy = cy + 1;
		if (ord(s.sjer[cy]) != 0)  cz = cz + 1;
	} while (!(cz == cx));
	cz = ord(s.sjer[cy]);
	s.sjer[cy] = chr(0);
	modinv();
	avpoing(cz);
	crep = 998;
	clsf2();
}

/* overlay */ void tsoulever()

{
	int cx;

	if (! syn)  ecr3("soulever");
	tfleche();
	if ((anyone) || (iesc))  return;
	tcoord(3);
	if (num == 0) {
		tcoord(8);
		if (num != 0) {
			if (ipers > 0)  s.conf = s.conf + 1;
			crep = 997;
			if ((s.mlieu == 2) && (s.iloic != 0))  treg(s.iloic);
		}
		return;
	}
	if (ipers > 0)  s.conf = s.conf + 1;
	cx = s.mlieu;
	if (s.mlieu == 16)  cx = 14;
	if (s.mlieu == 19)  cx = 15;
	crep = tabdon[asoul + (cx << 3) + pred(int, num)];
	if (crep == 255)  crep = 997;
}

/* overlay */ void tlire() {
	int iaff;

	if (caff > 99)  st4(caff);
	else {
		if (! syn)  ecr3("lire");
		tfleche();
		if (!(anyone) && !(iesc)) {
			tcoord(4);
			if (num != 0)  crep = 107;
		}
	}
}

/* overlay */ void tslire() {
	if (s.derobj == 0)  crep = 186;
	else st4(s.derobj);
}

/* overlay */ void tregarder() {
	int cx;

	if (caff > 99) {
		crep = 103;
		return;
	}
	if (! syn)  ecr3("regarder");
	tfleche();
	if ((anyone) || (iesc))  return;
	tcoord(5);
	if (num == 0) {
		tcoord(8);
		crep = 131;
		if (num != 0) {
			if (s.mlieu == 13) {
				if (num == 1) {
					crep = 164;
					if (s.ibag != 0)  treg(s.ibag);
					else if (s.iboul != 0)  treg(s.iboul);
				} else {
					crep = 193;
					if (s.ibag != 0)  treg(s.ibag);
				}
			}
			if (s.mlieu == 14) {
				crep = 164;
				if (s.icave != 0)  treg(s.icave);
			}
			if (s.mlieu == 17) {
				crep = 174;
				if (s.ivier != 0)  treg(s.ivier);
			}
			if (s.mlieu == 24) {
				crep = 131;
				if (s.ipuit != 0)  treg(s.ipuit);
			}
		}
		return;
	}
	cx = s.mlieu;
	if (s.mlieu == 20)  cx = 17;
	if ((s.mlieu > 21) && (s.mlieu < 25))  cx = cx - 4;
	if (s.mlieu == 26)  cx = 21;
	crep = tabdon[arega + (cx * 7) + pred(int, num)];
	if ((s.mlieu == 13) && (num == 8))  crep = 126;
	if (s.mlieu == 19)  crep = 103;
	if (crep == 255)  crep = 131;
	if ((s.mlieu == 1) && (num == 1))  treg(144);
	if ((s.mlieu == 5) && (num == 3))  treg(147);
	if ((s.mlieu == 8) && (num == 3))  treg(149);
	if ((s.mlieu == 9) && (num == 2))  treg(30);
	if ((s.mlieu == 10) && (num == 3))  treg(31);
}

/* overlay */ void tsregarder() {
	if (s.derobj != 0)  treg(s.derobj);
	else crep = 186;
}

/* overlay */ void tfouiller() {
	const byte[14] r = {123, 104, 123, 131, 131, 123, 104, 131, 123, 123, 106, 123, 123, 107};
	int cx;

	if (caff > 99) {
		st7(caff);
		return;
	}
	if (! syn)  ecr3("fouiller");
	tfleche();
	if (anyone || iesc)  return;
	if (s.mlieu == 23) {
		crep = 1504;
		tperd();
		return;
	}
	tcoord(6);
	if (num == 0) {
		tcoord(7);
		if (num != 0) {
			cx = 0;
			do {
				cx = cx + 1;
			} while (!((cx > 6) || (num == ord(touv[cx]))));
			if (num != ord(touv[cx]))  crep = 187;
			else {
				if (ipers > 0)  s.conf = s.conf + 3;
				rechai(mchai);
				if (mchai != 0) {
					cs = 0;
					is = 0;
					fouil = true;
					mfoudi();
					tsuiv();
				} else crep = 997;
			}
		} else {
			tcoord(8);
			crep = 997;
			if (num != 0) {
				if (ipers > 0)  s.conf = s.conf + 3;
				if ((s.mlieu != 24) && (s.mlieu != 17) && (s.mlieu != 13)) {
					if (s.mlieu == 2) {
						crep = 123;
						if (s.iloic != 0)  treg(s.iloic);
					}
					if (s.mlieu == 16) {
						crep = 123;
						if (s.icryp != 0)  treg(s.icryp);
					}
				}
			}
		}
	} else {
		if (ipers > 0)  s.conf = s.conf + 3;
		crep = 997;
		if (s.mlieu < 14)  crep = r[s.mlieu];
		if ((s.mlieu == 3) && (num == 2))  crep = 162;
		if (s.mlieu == 12) {
			if ((num == 3) || (num == 4))  crep = 162;
			if (num == 5)  crep = 159;
		}
		if (s.mlieu == 19)  crep = 104;
		if (s.mlieu == 16)  crep = 155;
	}
}

/* overlay */ void tsfouiller() {
	if (s.derobj != 0)  st7(s.derobj);
	else crep = 186;
}

/* overlay */ void touvrir() {
	int cx, haz;

	if (! syn)  ecr3("ouvrir");
	if (caff == 26) {
		if (ment != 0) {
			msg[4] = entrer;
			syn = true;
		} else crep = 997;
		return;
	}
	if (caff == 15) {
		aldepl();
		return;
	}
	tfleche();
	if ((anyone) || (iesc))  return;
	tcoord(7);
	if (num != 0) {
		if (ipers > 0)  s.conf = s.conf + 2;
		iouv = iouv + 1;
		cx = 0;
		do {
			cx = cx + 1;
		} while (!((cx > 6) || (ord(touv[cx]) == 0) || (ord(touv[cx]) == num)));
		if (ord(touv[cx]) != num) {
			if (!
			        (
			            ((num == 3) && ((s.mlieu == 0) || (s.mlieu == 9) || (s.mlieu == 5) || (s.mlieu == 7)))
			            ||
			            ((num == 4) && ((s.mlieu == 1) || (s.mlieu == 2) || (s.mlieu == 6))) ||
			            ((s.mlieu == 4) && (num == 5)) ||
			            ((num == 6) && ((s.mlieu == 7) || (s.mlieu == 10) ||
			                            (s.mlieu == 8) || (s.mlieu == 13))) ||
			            ((s.mlieu == 8) && (num == 2)) ||
			            ((s.mlieu == 12) && (num == 7)))) {
				if (((s.mlieu > 10) && (s.mlieu < 14)) ||
				        ((s.mlieu > 6) && (s.mlieu < 10)) ||
				        (s.mlieu == 0) || (s.mlieu == 2) || (s.mlieu == 5)) {
					haz = hazard(1, 4);
					if (haz == 3)  parole(7, 9, 1);
				}
				touv[cx] = chr(num);
				aniof(1, num);
			}
			cx = s.mlieu;
			if (s.mlieu == 16)  cx = 14;
			crep = tabdon[aouvr + (cx * 7) + pred(int, num)];
			if (crep == 254)  crep = 999;
		} else crep = 18;
	}
}

/* overlay */ void tmettre() {
	int quel;
	bool entre;
	phrase st;
	Common::String str_;
	int i, tay;

	if (s.derobj == 0) {
		crep = 186;
		return;
	}
	if (! syn)  ecr3("mettre");
	tfleche();
	if (iesc)  crep = 998;
	if ((anyone) || (iesc))  return;
	tcoord(8);
	if (num != 0) {
		crep = 999;
		if (caff == 13) {
			if (num == 1) {
				if (s.iboul != 0)  crep = 188;
				else {
					s.iboul = s.derobj;
					if (s.derobj == 141)  aniof(1, 7);
				}
			} else if (s.ibag != 0)  crep = 188;
			else {
				s.ibag = s.derobj;
				if (s.derobj == 159)  aniof(1, 6);
			}
		}
		if (caff == 14)
			if (s.icave != 0)  crep = 188;
			else {
				s.icave = s.derobj;
				if (s.derobj == 151) {
					aniof(1, 2);
					aniof(1, 1);
					repon(2, 165);
					maivid();
					parole(6, -9, 1);
					quel = do_alert(stouinon, 1);
					if (quel == 1)  {
						deline(582, st, tay);
						i = do_alert(delig, 1);
						tesok = false;
						entre = ques();
						hide_mouse();
						hirs();
						dessine_rouleau();
						clsf2();
						clsf3();
						show_mouse();
						tinke();
						pendule();
						if (ipers != 0)  affper(ipers);
						else person();
						menu_aff();
						if (entre) {
							s.mlieu = 17;
							tmlieu(17);
						} else {
							tmlieu(s.mlieu);
							writepal(14);
							dessin(0);
							aniof(1, 2);
							aniof(1, 1);
							deline(577, st, tay);
							i = do_alert(delig, 1);
							aniof(2, 1);
							crep = 166;
						}
						affrep();
					} else {
						aniof(2, 1);
						crep = 166;
						tesok = true;
					}
					return;
				}
			}
		if (caff == 16)
			if (s.icryp == 0)  s.icryp = s.derobj;
			else crep = 188;
		if (caff == 17)
			if (s.ivier != 0)  crep = 188;
			else if (s.derobj == 143) {
				s.ivier = 143;
				aniof(1, 1);
			} else {
				crep = 1512;
				tperd();
			}
		if (caff == 24)
			if (s.ipuit != 0)  crep = 188;
			else if ((s.derobj == 140) || (s.derobj == 120)) {
				s.ipuit = s.derobj;
				aniof(1, 1);
			} else crep = 185;
		if (crep != 188)  maivid();
	}
}

/* overlay */ void ttourner() {
	int quel;

	if (caff > 99) {
		crep = 149;
		return;
	}
	if (! syn)  ecr3("tourner");
	tfleche();
	if ((anyone) || (iesc))  return;
	tcoord(9);
	if (num != 0) {
		crep = 997;
		if ((s.mlieu == 13) && (s.ibag == 159) && (s.iboul == 141)) {
			repon(2, 167);
			parole(7, 9, 1);
			quel = do_alert(stouinon, 1);
			if (quel == 1)  solu = true;
			else crep = 168;
		}
		if ((s.mlieu == 17) && (s.ivier == 143)) {
			repon(2, 175);
			clsf3();
			parole(6, -9, 1);
			quel = do_alert(stouinon, 1);
			if (quel == 1) {
				s.mlieu = 16;
				affrep();
			} else crep = 176;
		}
	}
}

/* overlay */ void tcacher() {
	if (! syn)  ecr3("se cacher");
	tfleche();
	if (!(anyone) && !(iesc)) {
		tcoord(10);
		if (num == 0)  cache = false;
		else {
			cache = true;
			crep = 999;
		}
	}
}

/* overlay */ void tattacher() {
	if (s.derobj == 0)  crep = 186;
	else {
		if (! syn)  ecr3("attacher");
		tfleche();
		if (!(anyone) && !(iesc)) {
			tcoord(8);
			crep = 997;
			if ((num != 0) && (s.mlieu == 24)) {
				crep = 999;
				if ((s.derobj == 120) || (s.derobj == 140)) {
					s.ipuit = s.derobj;
					aniof(1, 1);
				} else crep = 185;
				maivid();
			}
		}
	}
}

/* overlay */ void tfermer() {
	int cx, chai;

	if (! syn)  ecr3("fermer");
	if (caff < 26) {
		tfleche();
		if (iesc)  crep = 998;
		if ((anyone) || (iesc))  return;
		tcoord(7);
		if (num != 0) {
			cx = 0;
			do {
				cx = cx + 1;
			} while (!((cx > 6) || (num == ord(touv[cx]))));
			if (num == ord(touv[cx])) {
				aniof(2, num);
				crep = 998;
				touv[cx] = chr(0);
				iouv = iouv - 1;
				if (iouv < 0)  iouv = 0;
				chai = 9999;
				rechai(chai);
				if (mchai == chai)  mchai = 0;
			} else crep = 187;
		}
	}
	if (caff == 26)  crep = 999;
}

/* overlay */ void tfrapper() {
	int l, p, haz;

	if (! syn)  ecr3("frapper");
	if (s.mlieu == 15) {
		l = do_alert("[1][ | Avant, utilisez le menu DEP...][ok]", 1);
		return;
	}
	if (s.mlieu < 25) {
		tfleche();
		if (!(anyone) && !(iesc))
			if ((s.mlieu < 19) && (s.mlieu != 15))  crep = 133;
			else crep = 997;
		return;
	}
	if (s.mlieu == 26) {
		haz = (hazard(0, 8)) - 4;
		parole(11, haz, 1);
		ecfren(p, haz, s.conf, ment);
		l = ment;
		if (l != 0)
			if (p != -500) {
				if (haz > p)  crep = 190;
				else {
					becfren(l);
					frap();
				}
			} else frap();
		if (ment == 8)  crep = 190;
	}
}

/* overlay */ void tposer() {
	int cx, chai;

	if (! syn)  ecr3("poser");
	if (s.derobj == 0)  crep = 186;
	else {
		if (caff > 99) {
			crep = 999;
			ajchai();
			if (crep != 192)  maivid();
			return;
		}
		tfleche();
		if ((anyone) || (iesc))  return;
		tcoord(7);
		crep = 124;
		if (num != 0) {
			rechai(chai);
			if (chai == 0)  crep = 997;
			else {
				cx = 0;
				do {
					cx = cx + 1;
				} while (!((cx > 6) || (num == ord(touv[cx]))));
				if (num != ord(touv[cx]))  crep = 187;
				else {
					mchai = chai;
					crep = 999;
				}
			}
		} else {
			tcoord(8);
			if (num != 0) {
				crep = 998;
				if (caff == 2)
					if (s.iloic != 0)  crep = 188;
					else s.iloic = s.derobj;
				if (caff == 13) {
					if (num == 1) {
						if (s.iboul != 0)  crep = 188;
						else s.iboul = s.derobj;
					} else if (s.ibag != 0)  crep = 188;
					else s.ibag = s.derobj;
				}
				if (caff == 16)
					if (s.icryp != 0)  crep = 188;
					else s.icryp = s.derobj;
				if (caff == 24)  crep = 185;
				if ((caff == 14) || (caff == 17))  crep = 124;
			} else {
				crep = 124;
				if (caff == 24) {
					tcoord(5);
					if (num != 0)  crep = 185;
				}
			}
		}
		if (caff == 23)  crep = 185;
		if ((crep == 999) || (crep == 185) || (crep == 998)) {
			if (crep == 999)  ajchai();
			if (crep != 192)  maivid();
		}
	}
}

/* overlay */ void tecouter() {
	int l, p, haz, j, h, m;

	if (s.mlieu != 26)  crep = 101;
	else {
		if (ipers != 0)  s.conf = s.conf + 1;
		ecfren(p, haz, s.conf, ment);
		l = ment;
		if (l != 0)
			if (p != -500) {
				if (haz > p)  crep = 101;
				else {
					becfren(l);
					calch(j, h, m);
					haz = hazard(1, 100);
					if ((h >= 0) && (h < 8)) {
						if (haz > 30)  crep = 101;
						else crep = 178;
					} else if (haz > 70)  crep = 101;
					else crep = 178;
				}
			} else crep = 178;
	}
}

/* overlay */ void tmanger() {
	int j, h, m;

	if ((s.mlieu > 15) && (s.mlieu < 26))
		crep = 148;
	else {
		tsort();
		s.mlieu = 10;
		caff = 10;
		debloc(s.mlieu);
		tmlieu(s.mlieu);
		calch(j, h, m);
		if ((h == 12) || (h == 13) || (h == 19)) {
			s.conf = s.conf - (s.conf / 7);
			if (h == 12)
				if (m == 0)  h = 4;
				else h = 3;
			if ((h == 13) || (h == 19))
				if (m == 0)  h = 2;
				else h = 1;
			jh = jh + h;
			crep = 135;
			tinke();
		} else crep = 134;
	}
}

/* overlay */ void tentrer() {
	int x, z;

	if ((s.mlieu == 21) || (s.mlieu == 22)) {
		t1sama();
		tmlieu(s.mlieu);
	} else if (s.mlieu == 15)  aldepl();
	else if (ment == 0)  crep = 997;
	else {
		if ((ment == 9) && (s.derobj != 136)) {
			crep = 189;
			s.teauto[8] = '*';
		} else {
			if (! blo)  t11(ment, z);
			if (z != 0) {
				if ((ment == 3) || (ment == 7))  crep = 179;
				else {
					x = (hazard(0, 10)) - 5;
					parole(7, x, 1);
					aniof(1, 1);
					/*       tkey(5,32000);*/
					tip(z, x);
					s.conf = s.conf + 1;
					s.mlieu = 15;
					msg[3] = discut;
					msg[4] = disc[x];
					syn = true;
					if (ment == 9) {
						col = true;
						caff = 70;
						afdes(0);
						repon(2, caff);
					} else col = false;
					debloc(ment);
					ment = 0;
				}
			} else {
				x = (hazard(0, 10)) - 5;
				parole(7, x, 1);
				aniof(1, 1);
				/*      tkey(1,32000);*/
				s.mlieu = ment;
				affrep();
				debloc(s.mlieu);
				tmlieu(s.mlieu);
				ment = 0;
				mpers = 0;
				ipers = 0;
			}
		}
	}
}

/* overlay */ void tdormir() {
	const char m1[] = "D‚sirez-vous vous r‚veiller?";
	int z, j, h, m, quel;

	if ((s.mlieu > 15) && (s.mlieu < 26)) {
		crep = 148;
		return;
	}
	if (s.mlieu != 0) {
		tsort();
		s.mlieu = 0;
		affrep();
		afdes(0);
		debloc(s.mlieu);
		tmlieu(s.mlieu);
	}
	clsf3();
	clsf2();
	ecrf2();
	ecr2(m1);
	calch(j, h, m);
	do {
		if (h < 8) {
			s.conf = s.conf - (s.conf / 20);
			z = (7 - h) * 2;
			if (m == 30)  z = z - 1;
			jh = jh + z;
			h = 7;
		}
		jh = jh + 2;
		h = h + 1;
		if (h > 23)  h = 0;
		tinke();
		quel = do_alert(stouinon, 1);
		anyone = false;
	} while (!(quel == 1));
	crep = 998;
	num = 0;
}

/* overlay */ void tdefoncer() {
	if (! syn)  ecr3("d‚foncer");
	if (caff < 25)  tfleche();
	if ((! anyone) && (! iesc))
		if (s.mlieu != 26)  crep = 997;
		else {
			crep = 143;
			s.conf = s.conf + 2;
		}
}

/* overlay */ void tsortir() {
	int lx;

	tsort();
	crep = 0;
	if ((s.mlieu == 19) || (s.mlieu == 21) || (s.mlieu == 22)
	        || (s.mlieu == 24))  crep = 997;
	else {
		if ((s.mlieu < 16) || (s.mlieu == 26))  lx = 10;
		if ((s.mlieu == 10) || (s.mlieu == 20))  lx = 21;
		if ((s.mlieu < 10) || (s.mlieu == 13))  lx = 15;
		if (s.mlieu == 16) {
			lx = 17;
			crep = 176;
		}
		if (s.mlieu == 17)  t23coul(lx);
		if (s.mlieu == 23)  lx = 24;
		if (crep != 997)  s.mlieu = lx;
		caff = lx;
		if (crep == 0)  crep = lx;
		debloc(lx);
		tmlieu(lx);
	}
}

/* overlay */ void tattendre() {
	int quel;

	mpers = 0;
	clsf3();
	do {
		jh = jh + 1;
		tinke();
		if (! blo)  t11(s.mlieu, quel);
		if ((ipers != 0) && (mpers == 0)) {
			crep = 998;
			if ((s.mlieu == 13) || (s.mlieu == 14))  cavegre();
			if ((s.mlieu > 0) && (s.mlieu < 10))  anyone = true;
			mpers = ipers;
			if (! anyone)  tinke();
			return;
		}
		repon(2, 102);
		quel = do_alert(stouinon, 1);
	} while (!(quel == 2));
	crep = 998;
	if (! anyone)  tinke();
}

/* overlay */ void tsonder() {
	if (! syn)  ecr3("sonder");
	if (caff < 27) {
		tfleche();
		if (!(anyone) && (! iesc))  crep = 145;
		num = 0;
	}
}

/* overlay */ void tparler()

{
	array<1, 46, bool> te;
	int ix, cy, cx, max, haz, suj, co, lig, icm,
	        i, tay, choi, x, y, c;
	char tou;
	array<1, 46, varying_string<40> > lib;
	phrase st;
	bool f;



	finfouil();
	if (col)  suj = 128;
	else {
		cx = 0;
		do {
			cx = cx + 1;
		} while (!(disc[cx] == msg[4]));
		caff = 69 + cx;
		afdes(0);
		repon(2, caff);
		suj = caff + 60;
	}
	tkey1(false);
	mennor();
	hide_mouse();
	hirs();
	premtet();
	sparl(0, suj);
	hirs();
	for (ix = 1; ix <= 46; ix ++) te[ix] = false;
	for (ix = 1; ix <= 45; ix ++) {
		deline(ix + c_tparler, st, tay);
		lib[ix] = delig;
		for (i = tay; i <= 40; i ++) lib[ix] = lib[ix] + ' ';
	}
	lib[46] = lib[45];
	lib[45] = ' ';
	show_mouse();
	do {
		choi = 0;
		icm = 0;
		co = 0;
		lig = 0;
		do {
			icm = succ(int, icm);
			putxy(co, lig);
			if (s.teauto[icm] == '*')
				if (te[icm])  writetp(lib[icm], 1);
				else writetp(lib[icm], 0);
			if (icm == 23)  {
				lig = 0;
				co = 320;
			} else lig = lig + 8;
		} while (!(icm == 42));
		putxy(320, 176);
		writetp(lib[46], 0);
		tou = '\0';
		do {
			mov_mouse(f, tou);
			/*     if keypressed then read(kbd,tou);*/
			read_pos_mouse(x, y, c);
			x = x * (3 - res);
			if (x > 319)  cx = 41;
			else cx = 1;
			cy = succ(int, (cardinal)y >> 3);                  /* 0-199 => 1-25 */
			if ((cy > 23) || ((cx == 41) && (set::of(range(20, 22), eos).has(cy)))) {
				if (choi != 0) {
					lig = ((choi - 1) % 23) << 3;
					if (choi > 23)  co = 320;
					else co = 0;
					putxy(co, lig);
					if (te[choi])  writetp(lib[choi], 0);
					else writetp(lib[choi], 1);
					te[choi] = ! te[choi];
					choi = 0;
				}
			} else {
				ix = cy;
				if (cx == 41)  ix = ix + 23;
				if (ix != choi) {
					if (choi != 0) {
						lig = ((choi - 1) % 23) << 3;
						if (choi > 23)  co = 320;
						else co = 0;
						putxy(co, lig);
						if (te[choi])  writetp(lib[choi], 0);
						else writetp(lib[choi], 1);
						te[choi] = ! te[choi];
					}
					if ((s.teauto[ix] == '*') || (ix == 46)) {
						lig = ((ix - 1) % 23) << 3;
						if (ix > 23)  co = 320;
						else co = 0;
						putxy(co, lig);
						if (te[ix])  writetp(lib[ix], 0);
						else writetp(lib[ix], 1);
						te[ix] = ! te[ix];
						choi = ix;
					} else choi = 0;
				}
			}
		} while (!((tou == '\15') || (((c != 0) || clic) && (choi != 0))));
		clic = false;
		if (choi != 46) {
			ix = choi - 1;
			if (col) {
				col = false;
				s.mlieu = 15;
				if (iouv > 0)  max = 8;
				else max = 4;
				haz = hazard(1, max);
				if (haz == 2)  suj = 129;
				else {
					suj = 138;
					s.conf = s.conf + (3 * (s.conf / 10));
				}
			} else if (nbrep[caff - 69] < nbrepm[caff - 69]) {
				suj = tabdon[arep + (ix << 3) + (caff - 70)];
				s.conf = s.conf + tabdon[arcf + ix];
				nbrep[caff - 69] = nbrep[caff - 69] + 1;
			} else {
				s.conf = s.conf + 3;
				suj = 139;
			}
			hide_mouse();
			hirs();
			premtet();
			sparl(0, suj);
			show_mouse();
			if ((suj == 84) || (suj == 86)) {
				s.pourc[5] = '*';
				s.teauto[7] = '*';
			}
			if ((suj == 106) || (suj == 108) || (suj == 94)) {
				for (ix = 29; ix <= 31; ix ++) s.teauto[ix] = '*';
				s.pourc[7] = '*';
			}
			if (suj == 70) {
				s.pourc[8] = '*';
				s.teauto[32] = '*';
			}
			hide_mouse();
			hirs();
			show_mouse();
		}
	} while (!((choi == 46) || (suj == 138)));
	if (col) {
		s.conf = s.conf + (3 * (s.conf / 10));
		hide_mouse();
		hirs();
		premtet();
		sparl(0, 138);
		show_mouse();
		col = false;
		s.mlieu = 15;
	}
	ctrm = 0;
	hide_mouse();
	hirs();
	dessine_rouleau();
	show_mouse();
	affper(ipers);
	tinke();
	pendule();
	affrep();
	/* chech;*/
	tmlieu(s.mlieu);
	clsf3();
}

/* overlay */ void tsentir() {
	crep = 119;
	if (caff < 26) {
		if (! syn)  ecr3("sentir");
		tfleche();
		if (!(anyone) && !(iesc))
			if (caff == 16)  crep = 153;
	} else if (caff == 123)  crep = 110;
	num = 0;
}

/* overlay */ void tgratter() {
	crep = 155;
	if (caff < 27) {
		if (! syn)  ecr3("gratter");
		tfleche();
	}
	num = 0;
}

/* NIVEAU 2 */
/* overlay */ void tmaj1() {           /* Le jeu est termin‚ !!! */
	arret = true;
	tlu(13, 152);
	maivid();
	clsf1();
	clsf2();
	clsf3();
	repon(9, 1509);
	tkey1(false);
	hide_mouse();
	caff = 70;
	taffich();
	hirs();
	premtet();
	sparl(0, 141);
	show_mouse();
	clsf1();
	repon(9, 1509);
	repon(2, 142);
	tkey1(false);
	caff = 32;
	afdes(0);
	repon(6, 34);
	repon(2, 35);
	musique(0);
	tkey1(false);
	messint(2036);
	tkey1(false);
	inzon();
}

/* overlay */ void tencore() {         /* Perdu !!! */
	int quel;

	clsf2();
	musique(0);
	tkey1(false);
	maivid();
	inzon();
	dprog();
	vh = 10;
	vm = 0;
	vj = 0;
	min = 0;
	heu = 10;
	jou = 0;
	repon(2, 180);
	quel = do_alert(stouinon, 1);
	arret = (quel != 1);
}

} // End of namespace Mortevielle
