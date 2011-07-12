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

/*
 * This code is based on original Soltys source code
 * Copyright (c) 1994-1995 Janus B. Wisniewski and L.K. Avalon
 */

#include "cge/general.h"
#include "cge/sound.h"
#include "cge/snail.h"
#include "cge/vga13h.h"
#include "cge/text.h"
#include "cge/cge_main.h"
#include "cge/events.h"

namespace CGE {

extern  Sprite *_pocLight;

//-------------------------------------------------------------------------
//	SPRITE * Pocket[POCKET_NX]={ NULL, NULL, NULL, NULL,
//					    NULL, NULL, NULL, NULL, };
//	int      _pocPtr      =  0;
//-------------------------------------------------------------------------
extern  Sprite *_pocket[];

void CGEEngine::snGame(Sprite *spr, int num) {
	switch (num) {
	case 1 : {
#define STAGES 8
#define DRESSED 3
		static Sprite *dup[3] = { NULL, NULL, NULL };
		int buref = 0;
		int Stage = 0;

		for (dup[0] = _vga->_showQ->first(); dup[0]; dup[0] = dup[0]->_next) {
			buref = dup[0]->_ref;
			if (buref / 1000 == 16 && buref % 100 == 6) {
				Stage = (buref / 100) % 10;
				break;
			}
		}
		if (dup[1] == NULL) {
			dup[1] = _vga->_showQ->locate(16003);    // pan
			dup[2] = _vga->_showQ->locate(16004);    // pani
		}

		if (_game) { // continue game
			int i = new_random(3), hand = (dup[0]->_shpCnt == 6);
			Stage++;
			if (hand && Stage > DRESSED)
				++hand;
			if (i >= 0 || (dup[i] == spr && new_random(3) == 0)) {
				SNPOST(SNSEQ, -1, 3, dup[0]);               // yes
				SNPOST(SNSEQ, -1, 3, dup[1]);               // yes
				SNPOST(SNSEQ, -1, 3, dup[2]);               // yes
				SNPOST(SNTNEXT, -1, 0, dup[0]);             // reset Take
				SNPOST(SNTNEXT, -1, 0, dup[1]);             // reset Take
				SNPOST(SNTNEXT, -1, 0, dup[2]);             // reset Take
				SNPOST(SNNNEXT, -1, 0, dup[0]);             // reset Near
				SNPOST(SNPAUSE, -1, 72, NULL);              // little rest
				SNPOST(SNSAY, 1, 16009, NULL);              // hura
				SNPOST(SNSAY, buref, 16010, NULL);          // siadaj
				SNPOST(SNSAY, 1, 16011, NULL);              // postoj‘

				if (hand) {
					SNPOST(SNSEND, 16060 + hand, 16, NULL);   // dawaj r‘k‘
					SNPOST(SNSEQ, buref, 4, NULL);            // zdejmowanie
					SNPOST(SNSEQ, 16060 + hand, 1, NULL);     // ruch
					SNPOST(SNSOUND, 16060 + hand, 16002, NULL); // szelest
					SNPOST(SNWAIT, 16060 + hand, 3, NULL);    // podniesie
					SNPOST(SNSWAP, buref, buref + 100, NULL); // rozdziana
					SNPOST(SNSEQ, 16016, Stage, NULL);        // rožnie kupa
					SNPOST(SNSEND, 16060 + hand, -1, NULL);   // chowaj r‘k‘
					SNPOST(SNWAIT, 16060 + hand, -1, NULL);   // r‘ka zamar’a
				} else {
					SNPOST(SNSEQ, buref, 4, NULL);            // zdejmowanie
					SNPOST(SNSOUND, 16060 + hand, 16002, NULL); // szelest
					SNPOST(SNWAIT, buref, -1, NULL);          // zdejmie
					SNPOST(SNSWAP, buref, buref + 100, NULL); // rozdziana
					SNPOST(SNSEQ, 16016, Stage, NULL);        // rožnie kupa
				}
				//SNPOST(SNSEQ, buref+100, 0, NULL);        // reset
				SNPOST(SNPAUSE, -1, 72, NULL);              // chwilk‘...

				SNPOST(SNSEQ, -1, 0, dup[1]);               // odstaw Go
				SNPOST(SNSETXY, -1, 203 + SCR_WID * 49, dup[1]);
				SNPOST(SNSETZ, -1, 7, dup[1]);

				SNPOST(SNSEQ, -1, 0, dup[2]);               // odstaw J†
				SNPOST(SNSETXY, -1, 182 + SCR_WID * 62, dup[2]);
				SNPOST(SNSETZ, -1, 9, dup[2]);
				_game = 0;
				return;
			} else {
				SNPOST(SNSEQ, -1, 2, dup[0]);               // no
				SNPOST(SNSEQ, -1, 2, dup[1]);               // no
				SNPOST(SNSEQ, -1, 2, dup[2]);               // no
				SNPOST(SNPAUSE, -1, 72, NULL);              // 1 sec
			}
		}
		SNPOST(SNWALK, 198, 134, NULL);                 // na miejsce
		SNPOST(SNWAIT, 1, -1, NULL);                    // stoi
		SNPOST(SNCOVER, 1, 16101, NULL);                // ch’op do bicia
		SNPOST(SNSEQ, 16101, 1, NULL);                  // wystaw
		SNPOST(SNWAIT, 16101, 5, NULL);                 // czekaj
		SNPOST(SNPAUSE, 16101, 24, NULL);               // czekaj chwil‘
		SNPOST(SNSEQ, 16040, 1, NULL);                  // plask
		SNPOST(SNSOUND, 16101, 16001, NULL);            // plask!
		SNPOST(SNPAUSE, 16101, 24, NULL);               // czekaj chwil‘
		SNPOST(SNSEQ, 16040, 0, NULL);                  // schowaj plask
		SNPOST(SNWAIT, 16101, -1, NULL);                // stoi
		SNPOST(SNUNCOVER, 1, 16101, NULL);              // SDS
		if (!_game) {
			SNPOST(SNSAY, buref, 16008, NULL);            // zgadnij!
			_game = true;
		}
#undef STEPS
#undef DRESSED
	}
	break;
	//--------------------------------------------------------------------
	case 2 : {
		static Sprite *k = NULL, * k1, * k2, * k3;
		static int count = 0;

		if (k == NULL) {
			k  = _vga->_showQ->locate(20700);
			k1 = _vga->_showQ->locate(20701);
			k2 = _vga->_showQ->locate(20702);
			k3 = _vga->_showQ->locate(20703);
		}

		if (!_game) { // init
			SNPOST(SNGAME, 20002, 2, NULL);
			_game = true;
		} else { // cont
			k1->step(new_random(6));
			k2->step(new_random(6));
			k3->step(new_random(6));
			///--------------------
			if (spr->_ref == 1 && _keyboard->_key[ALT]) {
				k1->step(5);
				k2->step(5);
				k3->step(5);
			}
			///--------------------
			SNPOST(SNSETZ, 20700, 0, NULL);
			bool hit = (k1->_seqPtr + k2->_seqPtr + k3->_seqPtr == 15);
			if (hit) {
				if (spr->_ref == 1) {
					SNPOST(SNSAY,      1, 20003, NULL);       // hura!
					SNPOST(SNSEQ,  20011,     2, NULL);       // kamera won
					SNPOST(SNSEND, 20701,    -1, NULL);       // k1 won
					SNPOST(SNSEND, 20702,    -1, NULL);       // k2 won
					SNPOST(SNSEND, 20703,    -1, NULL);       // k3 won
					SNPOST(SNSEND, 20700,    -1, NULL);       // tv won
					SNPOST(SNKEEP, 20007,     0, NULL);       // do kieszeni
					SNPOST(SNSEND, 20006,    20, NULL);       // bilon
					SNPOST(SNSOUND, 20006, 20002, NULL);      // bilon!
					SNPOST(SNSAY,  20002, 20004, NULL);
					SNPOST(SNSEND, 20010,    20, NULL);       // papier
					SNPOST(SNSOUND, 20010, 20003, NULL);      // papier!
					SNPOST(SNSAY,  20001, 20005, NULL);
					_game = false;
					return;
				} else
					k3->step(new_random(5));
			}
			if (count < 100) {
				switch (count) {
				case 15 :
					SNPOST(SNSAY, 20003, 20021, NULL);
					break;
				case 30 :
				case 45 :
				case 60 :
				case 75 :
					SNPOST(SNSAY, 20003, 20022, NULL);
					break;
				}
				count++;
			}
			switch (spr->_ref) {
			case     1 :
				SNPOST(SNSAY,  20001, 20011, NULL);         // zapro
				SNPOST(SNSEQ,  20001, 1, NULL);             // rzu
				SNPOST(SNWAIT, 20001, 1, NULL);             // czekaj
				SNPOST(SNSETZ, 20700, 2, NULL);             // skryj k
				SNPOST(SNHIDE, 20007, 1, NULL);             // skryj k
				SNPOST(SNWAIT, 20001, 16, NULL);            // czekaj
				SNPOST(SNSEQ,  20007, 1, NULL);             // lec†
				SNPOST(SNHIDE, 20007, 0, NULL);             // poka§
				SNPOST(SNSOUND, 20007, 20001, NULL);        // grzech
				SNPOST(SNWAIT, 20007, -1, NULL);            // koniec
				SNPOST(SNGAME, 20001, 2, NULL);             // again!
				break;
			case 20001:
				SNPOST(SNSAY, 20002, 20012, NULL);          // zapro
				SNPOST(SNSEQ, 20002, 1, NULL);              // rzu
				SNPOST(SNWAIT, 20002, 3, NULL);             // czekaj
				SNPOST(SNSETZ, 20700, 2, NULL);             // skryj k
				SNPOST(SNHIDE, 20007, 1, NULL);             // skryj k
				SNPOST(SNWAIT, 20002, 10, NULL);            // czekaj
				SNPOST(SNSEQ, 20007, 2, NULL);              // lec†
				SNPOST(SNHIDE, 20007, 0, NULL);             // poka§
				SNPOST(SNSOUND, 20007, 20001, NULL);        // grzech
				SNPOST(SNWAIT, 20007, -1, NULL);            // koniec
				SNPOST(SNGAME, 20002, 2, NULL);             // again!
				break;
			case 20002:
				SNPOST(SNSAY, 20002, 20010, NULL);          // zapro
				SNPOST(SNWALK, 20005, -1, NULL);            // do stol
				SNPOST(SNWAIT, 1, -1, NULL);                // stoi
				SNPOST(SNCOVER, 1, 20101, NULL);            // grasol
				SNPOST(SNSEQ, 20101, 1, NULL);              // rzu
				SNPOST(SNWAIT, 20101, 5, NULL);             // czekaj
				SNPOST(SNSETZ, 20700, 2, NULL);             // skryj k
				SNPOST(SNHIDE, 20007, 1, NULL);             // skryj k
				SNPOST(SNWAIT, 20101, 15, NULL);            // czekaj
				SNPOST(SNSEQ, 20007, 1, NULL);              // lec†
				SNPOST(SNHIDE, 20007, 0, NULL);             // poka§
				SNPOST(SNSOUND, 20007, 20001, NULL);        // grzech
				SNPOST(SNWAIT, 20101, -1, NULL);            // koniec
				SNPOST(SNUNCOVER, 1, 20101, NULL);          // SDS
				SNPOST(SNGAME, 1, 2, NULL);                 // again!
				break;
			}
		}
	}
	break;
	}
}


void CGEEngine::expandSprite(Sprite *spr) {
	if (spr)
		_vga->_showQ->insert(_vga->_spareQ->remove(spr));
}


void CGEEngine::contractSprite(Sprite *spr) {
	if (spr)
		_vga->_spareQ->append(_vga->_showQ->remove(spr));
}

int CGEEngine::findPocket(Sprite *spr) {
	for (int i = 0; i < POCKET_NX; i++)
	if (_pocket[i] == spr)
		return i;
	return -1;
}


void CGEEngine::selectPocket(int n) {
	if (n < 0 || (_pocLight->_seqPtr && _pocPtr == n)) {
		_pocLight->step(0);
		n = findPocket(NULL);
		if (n >= 0)
			_pocPtr = n;
	} else {
		if (_pocket[n] != NULL) {
			_pocPtr = n;
			_pocLight->step(1);
		}
	}
	_pocLight->gotoxy(POCKET_X + _pocPtr * POCKET_DX + POCKET_SX, POCKET_Y + POCKET_SY);
}

void CGEEngine::pocFul() {
	_hero->park();
	SNPOST(SNWAIT, -1, -1, _hero);
	SNPOST(SNSEQ, -1, POC_FUL, _hero);
	SNPOST(SNSOUND, -1, 2, _hero);
	SNPOST(SNWAIT, -1, -1, _hero);
	SNPOST(SNSAY,  1, POC_FUL_TEXT, _hero);
}

void CGEEngine::hide1(Sprite *spr) {
	SNPOST_(SNGHOST, -1, 0, spr->ghost());
}

void CGEEngine::snGhost(Bitmap *bmp) {
	bmp->hide(bmp->_map & 0xFFFF, bmp->_map >> 16);
	bmp->_m = NULL;
	bmp->_map = 0;
	delete bmp;
}

void CGEEngine::feedSnail(Sprite *spr, SNLIST snq) {
	if (spr)
		if (spr->active()) {
			uint8 ptr = (snq == TAKE) ? spr->_takePtr : spr->_nearPtr;

			if (ptr != NO_PTR) {
				Snail::Com *comtab = spr->snList(snq);
				Snail::Com *c = comtab + ptr;

				if (findPocket(NULL) < 0) {                 // no empty pockets?
					Snail::Com *p;
					for (p = c; p->_com != SNNEXT; p++) {     // find KEEP command
						if (p->_com == SNKEEP) {
							pocFul();
							return;
						}
						if (p->_ptr)
							break;
					}
				}
				while (true) {
					if (c->_com == SNTALK) {
						if ((_snail->_talkEnable = (c->_val != 0)) == false)
							killText();
					}
					if (c->_com == SNNEXT) {
						Sprite *s = (c->_ref < 0) ? spr : locate(c->_ref);
						if (s) {
							uint8 *idx = (snq == TAKE) ? &s->_takePtr : &s->_nearPtr;
							if (*idx != NO_PTR) {
								int v;
								switch (c->_val) {
								case -1 :
									v = c - comtab + 1;
									break;
								case -2 :
									v = c - comtab;
									break;
								case -3 :
									v = -1;
									break;
								default :
									v = c->_val;
									break;
								}
								if (v >= 0)
									*idx = v;
							}
						}
						if (s == spr)
							break;
					}
					if (c->_com == SNIF) {
						Sprite *s = (c->_ref < 0) ? spr : locate(c->_ref);
						if (s) { // sprite extsts
							if (! s->seqTest(-1))
								c = comtab + c->_val;                // not parked
							else
								++c;
						} else
							++c;
					} else {
						SNPOST(c->_com, c->_ref, c->_val, spr);
						if (c->_ptr)
							break;
						else
							c++;
					}
				}
			}
		}
}

const char *Snail::_comTxt[] = {
	"LABEL",  "PAUSE",  "WAIT",    "LEVEL",   "HIDE",
	"SAY",    "INF",    "TIME",    "CAVE",    "KILL",
	"RSEQ",   "SEQ",    "SEND",    "SWAP",    "KEEP",
	"GIVE",   "IF",     "GAME",    "SETX0",   "SETY0",
	"SLAVE",  "SETXY",  "RELX",    "RELY",    "RELZ",
	"SETX",   "SETY",   "SETZ",    "TRANS",   "PORT",
	"NEXT",   "NNEXT",  "TNEXT",   "RNNEXT",  "RTNEXT",
	"RMNEAR", "RMTAKE", "FLAG",    "SETREF",  "BACKPT",
	"FLASH",  "LIGHT",  "SETHB",   "SETVB",   "WALK",
	"REACH",  "COVER",  "UNCOVER", "CLEAR",   "TALK",
	"MOUSE",  "SOUND",  "COUNT",   NULL
};


Snail::Snail(CGEEngine *vm, bool turbo)
	: _turbo(turbo), _busy(false), _textDelay(false),
	  _timerExpiry(0), _talkEnable(true),
	  _head(0), _tail(0), _snList(farnew(Com, 256)), _vm(vm) {
}

Snail::~Snail() {
	free(_snList);
}

void Snail::addCom(SNCOM com, int ref, int val, void *ptr) {
	Com *snc = &_snList[_head++];
	snc->_com = com;
	snc->_ref = ref;
	snc->_val = val;
	snc->_ptr = ptr;
	snc->_cbType = NULLCB;
	if (com == SNCLEAR) {
		_tail = _head;
		killText();
		_timerExpiry = 0;
	}
}

void Snail::addCom2(SNCOM com, int ref, int val, CALLBACK cbType) {
	Com *snc = &_snList[_head++];
	snc->_com = com;
	snc->_ref = ref;
	snc->_val = val;
	snc->_ptr = NULL;
	snc->_cbType = cbType;
	if (com == SNCLEAR) {
		_tail = _head;
		killText();
		_timerExpiry = 0;
	}
}

void Snail::insCom(SNCOM com, int ref, int val, void *ptr) {
	Com *snc;

	if (_busy) {
		_snList[(_tail - 1) & 0xFF] = _snList[_tail];
		snc = &_snList[_tail];
	} else
		snc = &_snList[(_tail - 1) & 0xFF];
	_tail--;
	snc->_com = com;
	snc->_ref = ref;
	snc->_val = val;
	snc->_ptr = ptr;
	if (com == SNCLEAR) {
		_tail = _head;
		killText();
		_timerExpiry = 0;
	}
}

void CGEEngine::snNNext(Sprite *sprel, int p) {
	if (sprel)
		if (sprel->_nearPtr != NO_PTR)
			sprel->_nearPtr = p;
}

void CGEEngine::snTNext(Sprite *sprel, int p) {
	if (sprel)
		if (sprel->_takePtr != NO_PTR)
			sprel->_takePtr = p;
}

void CGEEngine::snRNNext(Sprite *sprel, int p) {
	if (sprel)
		if (sprel->_nearPtr != NO_PTR)
			sprel->_nearPtr += p;
}


void CGEEngine::snRTNext(Sprite *sprel, int p) {
	if (sprel)
		if (sprel->_takePtr != NO_PTR)
			sprel->_takePtr += p;
}

void CGEEngine::snZTrim(Sprite *spr) {
	if (spr)
		if (spr->active()) {
			bool en = _heart->_enable;
			Sprite *s;
			_heart->_enable = false;
			s = (spr->_flags._shad) ? spr->_prev : NULL;
			_vga->_showQ->insert(_vga->_showQ->remove(spr));
			if (s) {
				s->_z = spr->_z;
				_vga->_showQ->insert(_vga->_showQ->remove(s), spr);
			}
			_heart->_enable = en;
		}
}

void CGEEngine::snHide(Sprite *spr, int val) {
	if (spr) {
		spr->_flags._hide = (val >= 0) ? (val != 0) : (!spr->_flags._hide);
		if (spr->_flags._shad)
			spr->_prev->_flags._hide = spr->_flags._hide;
	}
}

void CGEEngine::snRmNear(Sprite *spr) {
	if (spr)
		spr->_nearPtr = NO_PTR;
}

void CGEEngine::snRmTake(Sprite *spr) {
	if (spr)
		spr->_takePtr = NO_PTR;
}

void CGEEngine::snSeq(Sprite *spr, int val) {
	if (spr) {
		if (spr == _hero && val == 0)
			_hero->park();
		else
			spr->step(val);
	}
}

void CGEEngine::snRSeq(Sprite *spr, int val) {
	if (spr)
		snSeq(spr, spr->_seqPtr + val);
}

void CGEEngine::snSend(Sprite *spr, int val) {
	if (spr) {
		int was = spr->_cave;
		bool was1 = (was == 0 || was == _now);
		bool val1 = (val == 0 || val == _now);
		spr->_cave = val;
		if (val1 != was1) {
			if (was1) {
				if (spr->_flags._kept) {
					int n = findPocket(spr);
					if (n >= 0)
						_pocket[n] = NULL;
				}
				hide1(spr);
				contractSprite(spr);
				spr->_flags._slav = false;
			} else {
				if (spr->_ref % 1000 == 0)
					Bitmap::_pal = Vga::_sysPal;
				if (spr->_flags._back)
					spr->backShow(true);
				else
					expandSprite(spr);
				Bitmap::_pal = NULL;
			}
		}
	}
}


void CGEEngine::snSwap(Sprite *spr, int xref) {
	Sprite *xspr = locate(xref);
	if (spr && xspr) {
		int was = spr->_cave;
		int xwas = xspr->_cave;
		bool was1 = (was == 0 || was == _now);
		bool xwas1 = (xwas == 0 || xwas == _now);

		swap(spr->_cave, xspr->_cave);
		swap(spr->_x, xspr->_x);
		swap(spr->_y, xspr->_y);
		swap(spr->_z, xspr->_z);
		if (spr->_flags._kept) {
			int n = findPocket(spr);
			if (n >= 0)
				_pocket[n] = xspr;
			xspr->_flags._kept = true;
			xspr->_flags._port = false;
		}
		if (xwas1 != was1) {
			if (was1) {
				hide1(spr);
				contractSprite(spr);
			} else
				expandSprite(spr);
			if (xwas1) {
				hide1(xspr);
				contractSprite(xspr);
			} else
				expandSprite(xspr);
		}
	}
}


void CGEEngine::snCover(Sprite *spr, int xref) {
	Sprite *xspr = locate(xref);
	if (spr && xspr) {
		spr->_flags._hide = true;
		xspr->_z = spr->_z;
		xspr->_cave = spr->_cave;
		xspr->gotoxy(spr->_x, spr->_y);
		expandSprite(xspr);
		if ((xspr->_flags._shad = spr->_flags._shad) == 1) {
			_vga->_showQ->insert(_vga->_showQ->remove(spr->_prev), xspr);
			spr->_flags._shad = false;
		}
		feedSnail(xspr, NEAR);
	}
}


void CGEEngine::snUncover(Sprite *spr, Sprite *xspr) {
	if (spr && xspr) {
		spr->_flags._hide = false;
		spr->_cave = xspr->_cave;
		spr->gotoxy(xspr->_x, xspr->_y);
		if ((spr->_flags._shad = xspr->_flags._shad) == 1) {
			_vga->_showQ->insert(_vga->_showQ->remove(xspr->_prev), spr);
			xspr->_flags._shad = false;
		}
		spr->_z = xspr->_z;
		snSend(xspr, -1);
		if (spr->_time == 0)
			spr->_time++;
	}
}


void CGEEngine::snSetX0(int cav, int x0) {
	_heroXY[cav - 1]._x = x0;
}

void CGEEngine::snSetY0(int cav, int y0) {
	_heroXY[cav - 1]._y = y0;
}

void CGEEngine::snSetXY(Sprite *spr, uint16 xy) {
	if (spr)
		spr->gotoxy(xy % SCR_WID, xy / SCR_WID);
}

void CGEEngine::snRelX(Sprite *spr, int x) {
	if (spr && _hero)
		spr->gotoxy(_hero->_x + x, spr->_y);
}

void CGEEngine::snRelY(Sprite *spr, int y) {
	if (spr && _hero)
		spr->gotoxy(spr->_x, _hero->_y + y);
}


void CGEEngine::snRelZ(Sprite *spr, int z) {
	if (spr && _hero) {
		spr->_z = _hero->_z + z;
		snZTrim(spr);
	}
}


void CGEEngine::snSetX(Sprite *spr, int x) {
	if (spr)
		spr->gotoxy(x, spr->_y);
}


void CGEEngine::snSetY(Sprite *spr, int y) {
	if (spr)
		spr->gotoxy(spr->_x, y);
}


void CGEEngine::snSetZ(Sprite *spr, int z) {
	if (spr) {
		spr->_z = z;
		//SNPOST_(SNZTRIM, -1, 0, spr);
		snZTrim(spr);
	}
}


void CGEEngine::snSlave(Sprite *spr, int ref) {
	Sprite *slv = locate(ref);
	if (spr && slv) {
		if (spr->active()) {
			snSend(slv, spr->_cave);
			slv->_flags._slav = true;
			slv->_z = spr->_z;
			_vga->_showQ->insert(_vga->_showQ->remove(slv), spr->_next);
		}
	}
}


void CGEEngine::snTrans(Sprite *spr, int trans) {
	if (spr)
		spr->_flags._tran = (trans < 0) ? !spr->_flags._tran : (trans != 0);
}

void CGEEngine::snPort(Sprite *spr, int port) {
	if (spr)
		spr->_flags._port = (port < 0) ? !spr->_flags._port : (port != 0);
}

void CGEEngine::snKill(Sprite *spr) {
	if (spr) {
		if (spr->_flags._kept) {
			int n = findPocket(spr);
			if (n >= 0)
				_pocket[n] = NULL;
		}
		Sprite *nx = spr->_next;
		hide1(spr);
		_vga->_showQ->remove(spr);
		EventManager::clrEvt(spr);
		if (spr->_flags._kill)
			delete spr;
		else {
			spr->_cave = -1;
			_vga->_spareQ->append(spr);
		}
		if (nx) {
			if (nx->_flags._slav)
				snKill(nx);
		}
	}
}


void CGEEngine::snSound(Sprite *spr, int wav, int cnt) {
	if (_sndDrvInfo._dDev) {
		if (wav == -1)
			_sound.stop();
		else
			_sound.play(_fx[wav], (spr) ? ((spr->_x + spr->_w / 2) / (SCR_WID / 16)) : 8, cnt);
	}
}


void CGEEngine::snKeep(Sprite *spr, int stp) {
	selectPocket(-1);
	if (spr && ! spr->_flags._kept && _pocket[_pocPtr] == NULL) {
		snSound(spr, 3, 1);
		_pocket[_pocPtr] = spr;
		spr->_cave = 0;
		spr->_flags._kept = true;
		spr->gotoxy(POCKET_X + POCKET_DX * _pocPtr + POCKET_DX / 2 - spr->_w / 2,
		          POCKET_Y + POCKET_DY / 2 - spr->_h / 2);
		if (stp >= 0)
			spr->step(stp);
	}
	selectPocket(-1);
}


void CGEEngine::snGive(Sprite *spr, int stp) {
	if (spr) {
		int p = findPocket(spr);
		if (p >= 0) {
			_pocket[p] = NULL;
			spr->_cave = _now;
			spr->_flags._kept = false;
			if (stp >= 0)
				spr->step(stp);
		}
	}
	selectPocket(-1);
}


void CGEEngine::snBackPt(Sprite *spr, int stp) {
	if (spr) {
		if (stp >= 0)
			spr->step(stp);
		spr->backShow(true);
	}
}

void CGEEngine::snLevel(Sprite *spr, int lev) {
	while (_lev < lev) {
		_lev++;
		spr = _vga->_spareQ->locate(100 + _lev);
		if (spr) {
			spr->backShow(true);
			spr->_cave = 0;
		}
	}
	_maxCave = _maxCaveArr[_lev];
	if (spr)
		spr->_flags._hide = false;
}


void CGEEngine::snFlag(int fn, bool v) {
	_flag[fn] = v;
}

void CGEEngine::snSetRef(Sprite *spr, int nr) {
	if (spr)
		spr->_ref = nr;
}

void CGEEngine::snFlash(bool on) {
	if (on) {
		Dac *pal = farnew(Dac, PAL_CNT);
		if (pal) {
			memcpy(pal, Vga::_sysPal, PAL_SIZ);
			for (int i = 0; i < PAL_CNT; i++) {
				register int c;
				c = pal[i]._r << 1;
				pal[i]._r = (c < 64) ? c : 63;
				c = pal[i]._g << 1;
				pal[i]._g = (c < 64) ? c : 63;
				c = pal[i]._b << 1;
				pal[i]._b = (c < 64) ? c : 63;
			}
			_vga->setColors(pal, 64);
		}
	} else
		_vga->setColors(Vga::_sysPal, 64);
	_dark = false;
}


void CGEEngine::snLight(bool in) {
	if (in)
		_vga->sunrise(Vga::_sysPal);
	else
		_vga->sunset();
	_dark = ! in;
}

void CGEEngine::snBarrier(int cav, int bar, bool horz) {
	((uint8 *)(_barriers + ((cav > 0) ? cav : _now)))[horz] = bar;
}

void CGEEngine::snWalk(Sprite *spr, int x, int y) {
	if (_hero) {
		if (spr && y < 0)
			_hero->findWay(spr);
		else
			_hero->findWay(XZ(x, y));
	}
}

void CGEEngine::snReach(Sprite *spr, int mode) {
	if (_hero)
		_hero->reach(spr, mode);
}

void CGEEngine::snMouse(bool on) {
	if (on)
		_mouse->on();
	else
		_mouse->off();
}


void Snail::runCom() {
	static int count = 1;
	if (!_busy) {
		_busy = true;
		uint8 tmphea = _head;
		while (_tail != tmphea) {
			Com *snc = &_snList[_tail];

			if (!_turbo) { // only for the slower one
				if (_timerExpiry) {
					// Delay in progress
					if (_timerExpiry > g_system->getMillis())
						// Delay not yet ended
						break;

					// Delay is finished
					_timerExpiry = 0;
				} else {
					if (_textDelay) {
						killText();
						_textDelay = false;
					}
				}
				if (_talk && snc->_com != SNPAUSE)
					break;
			}

			Sprite *sprel = ((snc->_ref >= 0) ? locate(snc->_ref) : ((Sprite *) snc->_ptr));
			switch (snc->_com) {
			case SNLABEL    :
				break;
			case SNPAUSE    :
				_timerExpiry = g_system->getMillis() + snc->_val * SNAIL_FRAME_DELAY;
				if (_talk)
					_textDelay = true;
				break;
			case SNWAIT     :
				if (sprel) {
					if (sprel->seqTest(snc->_val) &&
					        (snc->_val >= 0 || sprel != _hero || _hero->_tracePtr < 0)) {
						_timerExpiry = g_system->getMillis() + sprel->_time * SNAIL_FRAME_DELAY;
					} else
						goto xit;
				}
				break;
			case SNLEVEL    :
				_vm->snLevel(sprel, snc->_val);
				break;
			case SNHIDE     :
				_vm->snHide(sprel, snc->_val);
				break;
			case SNSAY      :
				if (sprel && _talkEnable) {
					if (sprel == _hero && sprel->seqTest(-1))
						sprel->step(HTALK);
					_text->say(_text->getText(snc->_val), sprel);
					_sys->_funDel = HEROFUN0;
				}
				break;
			case SNINF      :
				if (_talkEnable) {
					_vm->inf(_text->getText(snc->_val));
					_sys->_funDel = HEROFUN0;
				}
				break;
			case SNTIME     :
				if (sprel && _talkEnable) {
					if (sprel == _hero && sprel->seqTest(-1))
						sprel->step(HTALK);
					sayTime(sprel);
				}
				break;
			case SNCAVE     :
				_vm->switchCave(snc->_val);
				break;
			case SNKILL     :
				_vm->snKill(sprel);
				break;
			case SNSEQ      :
				_vm->snSeq(sprel, snc->_val);
				break;
			case SNRSEQ     :
				_vm->snRSeq(sprel, snc->_val);
				break;
			case SNSEND     :
				_vm->snSend(sprel, snc->_val);
				break;
			case SNSWAP     :
				_vm->snSwap(sprel, snc->_val);
				break;
			case SNCOVER    :
				_vm->snCover(sprel, snc->_val);
				break;
			case SNUNCOVER  :
				_vm->snUncover(sprel, (snc->_val >= 0) ? locate(snc->_val) : ((Sprite *) snc->_ptr));
				break;
			case SNKEEP     :
				_vm->snKeep(sprel, snc->_val);
				break;
			case SNGIVE     :
				_vm->snGive(sprel, snc->_val);
				break;
			case SNGAME     :
				_vm->snGame(sprel, snc->_val);
				break;
			case SNSETX0    :
				_vm->snSetX0(snc->_ref, snc->_val);
				break;
			case SNSETY0    :
				_vm->snSetY0(snc->_ref, snc->_val);
				break;
			case SNSETXY    :
				_vm->snSetXY(sprel, snc->_val);
				break;
			case SNRELX     :
				_vm->snRelX(sprel, snc->_val);
				break;
			case SNRELY     :
				_vm->snRelY(sprel, snc->_val);
				break;
			case SNRELZ     :
				_vm->snRelZ(sprel, snc->_val);
				break;
			case SNSETX     :
				_vm->snSetX(sprel, snc->_val);
				break;
			case SNSETY     :
				_vm->snSetY(sprel, snc->_val);
				break;
			case SNSETZ     :
				_vm->snSetZ(sprel, snc->_val);
				break;
			case SNSLAVE    :
				_vm->snSlave(sprel, snc->_val);
				break;
			case SNTRANS    :
				_vm->snTrans(sprel, snc->_val);
				break;
			case SNPORT     :
				_vm->snPort(sprel, snc->_val);
				break;
			case SNNEXT     :
			case SNIF       :
			case SNTALK     :
				break;
			case SNMOUSE    :
				_vm->snMouse(snc->_val != 0);
				break;
			case SNNNEXT    :
				_vm->snNNext(sprel, snc->_val);
				break;
			case SNTNEXT    :
				_vm->snTNext(sprel, snc->_val);
				break;
			case SNRNNEXT   :
				_vm->snRNNext(sprel, snc->_val);
				break;
			case SNRTNEXT   :
				_vm->snRTNext(sprel, snc->_val);
				break;
			case SNRMNEAR   :
				_vm->snRmNear(sprel);
				break;
			case SNRMTAKE   :
				_vm->snRmTake(sprel);
				break;
			case SNFLAG     :
				_vm->snFlag(snc->_ref & 3, snc->_val != 0);
				break;
			case SNSETREF   :
				_vm->snSetRef(sprel, snc->_val);
				break;
			case SNBACKPT   :
				_vm->snBackPt(sprel, snc->_val);
				break;
			case SNFLASH    :
				_vm->snFlash(snc->_val != 0);
				break;
			case SNLIGHT    :
				_vm->snLight(snc->_val != 0);
				break;
			case SNSETHB    :
				_vm->snBarrier(snc->_ref, snc->_val, true);
				break;
			case SNSETVB    :
				_vm->snBarrier(snc->_ref, snc->_val, false);
				break;
			case SNWALK     :
				_vm->snWalk(sprel, snc->_ref, snc->_val);
				break;
			case SNREACH    :
				_vm->snReach(sprel, snc->_val);
				break;
			case SNSOUND    :
				_vm->snSound(sprel, snc->_val, count);
				count = 1;
				break;
			case SNCOUNT    :
				count = snc->_val;
				break;
			case SNEXEC     :
				switch (snc->_cbType) {
				case QGAME:
					_vm->qGame();
					break;
				case MINISTEP:
					_vm->miniStep(snc->_val);
					break;
				case XCAVE:
					_vm->xCave();
					break;
				case SELECTSOUND:
					_vm->selectSound();
					break;
				case SNSELECT:
					_vm->snSelect();
					break;
				case SNDSETVOLUME:
					sndSetVolume();
					break;
				}
				break;
			case SNSTEP     :
				sprel->step();
				break;
			case SNZTRIM    :
				_vm->snZTrim(sprel);
				break;
			case SNGHOST    :
				_vm->snGhost((Bitmap *) snc->_ptr);
				break;
			default :
				warning("Unhandled snc->_com in SNMouse(bool)");
				break;
			}
			_tail++;
			if (!_turbo)
				break;
		}
xit:
		_busy = false;
	}
}


bool Snail::idle() {
	return (_head == _tail);
}

} // End of namespace CGE
