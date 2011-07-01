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
#include "cge/bitmaps.h"
#include "cge/text.h"
#include "cge/mouse.h"
#include "cge/cge_main.h"
#include <stdio.h>
#include <stdlib.h>
#include "cge/keybd.h"

namespace CGE {

static void _enable() {
	warning("STUB: _enable");
}

static void _disable() {
	warning("STUB: _disable");
}

int     MaxCave  =  0;

SCB     Scb      = { NULL, 0, NULL };
bool    Flag[4];
bool    Dark     = false;
bool    Game     = false;
int     Now      =  1;
int     Lev      = -1;

extern  Sprite *_pocLight;

//-------------------------------------------------------------------------
//	SPRITE * Pocket[POCKET_NX]={ NULL, NULL, NULL, NULL,
//					    NULL, NULL, NULL, NULL, };
//	int      PocPtr      =  0;
//-------------------------------------------------------------------------
extern  Sprite *_pocket[];
extern  int     PocPtr;

static void SNGame(Sprite *spr, int num) {
	switch (num) {
	case 1 : {
#define STAGES 8
#define DRESSED 3
		static Sprite *dup[3] = { NULL, NULL, NULL };
		int buref = 0;
		int Stage = 0;

		for (dup[0] = Vga->ShowQ->First(); dup[0]; dup[0] = dup[0]->_next) {
			buref = dup[0]->_ref;
			if (buref / 1000 == 16 && buref % 100 == 6) {
				Stage = (buref / 100) % 10;
				break;
			}
		}
		if (dup[1] == NULL) {
			dup[1] = Vga->ShowQ->Locate(16003);    // pan
			dup[2] = Vga->ShowQ->Locate(16004);    // pani
		}

		if (Game) { // continue game
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
				Game = 0;
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
		if (! Game) {
			SNPOST(SNSAY, buref, 16008, NULL);            // zgadnij!
			Game = true;
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
			k  = Vga->ShowQ->Locate(20700);
			k1 = Vga->ShowQ->Locate(20701);
			k2 = Vga->ShowQ->Locate(20702);
			k3 = Vga->ShowQ->Locate(20703);
		}

		if (! Game) { // init
			SNPOST(SNGAME, 20002, 2, NULL);
			Game = true;
		} else { // cont
			k1->Step(new_random(6));
			k2->Step(new_random(6));
			k3->Step(new_random(6));
			///--------------------
			if (spr->_ref == 1 && KEYBOARD::Key[ALT]) {
				k1->Step(5);
				k2->Step(5);
				k3->Step(5);
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
					Game = false;
					return;
				} else
					k3->Step(new_random(5));
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
				++ count;
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


void ExpandSprite(Sprite *spr) {
	if (spr)
		Vga->ShowQ->Insert(Vga->SpareQ->Remove(spr));
}


void ContractSprite(Sprite *spr) {
	if (spr)
		Vga->SpareQ->Append(Vga->ShowQ->Remove(spr));
}

int FindPocket(Sprite *spr) {
	for (int i = 0; i < POCKET_NX; i++)
	if (_pocket[i] == spr)
		return i;
	return -1;
}


void SelectPocket(int n) {
	if (n < 0 || (_pocLight->_seqPtr && PocPtr == n)) {
		_pocLight->Step(0);
		n = FindPocket(NULL);
		if (n >= 0)
			PocPtr = n;
	} else {
		if (_pocket[n] != NULL) {
			PocPtr = n;
			_pocLight->Step(1);
		}
	}
	_pocLight->Goto(POCKET_X + PocPtr * POCKET_DX + POCKET_SX, POCKET_Y + POCKET_SY);
}


void PocFul(void) {
	Hero->park();
	SNPOST(SNWAIT, -1, -1, Hero);
	SNPOST(SNSEQ, -1, POC_FUL, Hero);
	SNPOST(SNSOUND, -1, 2, Hero);
	SNPOST(SNWAIT, -1, -1, Hero);
	SNPOST(SNSAY,  1, POC_FUL_TEXT, Hero);
}


void Hide1(Sprite *spr) {
	SNPOST_(SNGHOST, -1, 0, spr->Ghost());
}


void SNGhost(Bitmap *bmp) {
	// TODO : Get x and y from M but not using segment / offset
	//bmp->Hide(FP_OFF(bmp->_m), FP_SEG(bmp->_m));
	bmp->_m = NULL;
	delete bmp;
	warning("STUB: SNGhost");
}


void FeedSnail(Sprite *spr, SNLIST snq) {
	if (spr)
		if (spr->Active()) {
			uint8 ptr = (snq == TAKE) ? spr->TakePtr : spr->NearPtr;

			if (ptr != NO_PTR) {
				SNAIL::COM *comtab = spr->SnList(snq);
				SNAIL::COM *c = comtab + ptr;

				if (FindPocket(NULL) < 0) {                 // no empty pockets?
					SNAIL::COM *p;
					for (p = c; p->Com != SNNEXT; p++) {     // find KEEP command
						if (p->Com == SNKEEP) {
							PocFul();
							return;
						}
						if (p->Ptr)
							break;
					}
				}
				while (true) {
					if (c->Com == SNTALK) {
						if ((Snail->TalkEnable = (c->Val != 0)) == false)
							KillText();
					}
					if (c->Com == SNNEXT) {
						Sprite *s = (c->Ref < 0) ? spr : Locate(c->Ref);
						if (s) {
							uint8 *idx = (snq == TAKE) ? &s->TakePtr : &s->NearPtr;
							if (*idx != NO_PTR) {
								int v;
								switch (c->Val) {
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
									v = c->Val;
									break;
								}
								if (v >= 0)
									*idx = v;
							}
						}
						if (s == spr)
							break;
					}
					if (c->Com == SNIF) {
						Sprite *s = (c->Ref < 0) ? spr : Locate(c->Ref);
						if (s) { // sprite extsts
							if (! s->SeqTest(-1))
								c = comtab + c->Val;                // not parked
							else
								++c;
						} else
							++c;
					} else {
						SNPOST(c->Com, c->Ref, c->Val, spr);
						if (c->Ptr)
							break;
						else
							++c;
					}
				}
			}
		}
}


const char *SNAIL::ComTxt[] = {
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


SNAIL::SNAIL(CGEEngine *vm, bool turbo)
	: Turbo(turbo), Busy(false), TextDelay(false),
	  _timerExpiry(0), TalkEnable(true),
	  Head(0), Tail(0), SNList(farnew(COM, 256)), _vm(vm) {
}


SNAIL::~SNAIL(void) {
	if (SNList)
		free(SNList);
}


void SNAIL::AddCom(SNCOM com, int ref, int val, void *ptr) {
	_disable();
	COM *snc = &SNList[Head++];
	snc->Com = com;
	snc->Ref = ref;
	snc->Val = val;
	snc->Ptr = ptr;
	if (com == SNCLEAR) {
		Tail = Head;
		KillText();
		_timerExpiry = 0;
	}
	_enable();
}


void SNAIL::InsCom(SNCOM com, int ref, int val, void *ptr) {
	COM *snc;

	_disable();
	if (Busy) {
		SNList[(Tail - 1) & 0xFF] = SNList[Tail];
		snc = &SNList[Tail];
	} else
		snc = &SNList[(Tail - 1) & 0xFF];
	--Tail;
	snc->Com = com;
	snc->Ref = ref;
	snc->Val = val;
	snc->Ptr = ptr;
	if (com == SNCLEAR) {
		Tail = Head;
		KillText();
		_timerExpiry = 0;
	}
	_enable();
}


static void SNNNext(Sprite *sprel, int p) {
	if (sprel)
		if (sprel->NearPtr != NO_PTR)
			sprel->NearPtr = p;
}


static void SNTNext(Sprite *sprel, int p) {
	if (sprel)
		if (sprel->TakePtr != NO_PTR)
			sprel->TakePtr = p;
}


static void SNRNNext(Sprite *sprel, int p) {
	if (sprel)
		if (sprel->NearPtr != NO_PTR)
			sprel->NearPtr += p;
}


static void SNRTNext(Sprite *sprel, int p) {
	if (sprel)
		if (sprel->TakePtr != NO_PTR)
			sprel->TakePtr += p;
}


static void SNZTrim(Sprite *spr) {
	if (spr)
		if (spr->Active()) {
			bool en = _heart->_enable;
			Sprite *s;
			_heart->_enable = false;
			s = (spr->_flags._shad) ? spr->_prev : NULL;
			Vga->ShowQ->Insert(Vga->ShowQ->Remove(spr));
			if (s) {
				s->_z = spr->_z;
				Vga->ShowQ->Insert(Vga->ShowQ->Remove(s), spr);
			}
			_heart->_enable = en;
		}
}


static void SNHide(Sprite *spr, int val) {
	if (spr) {
		spr->_flags._hide = (val >= 0) ? (val != 0) : (!spr->_flags._hide);
		if (spr->_flags._shad)
			spr->_prev->_flags._hide = spr->_flags._hide;
	}
}


static void SNRmNear(Sprite *spr) {
	if (spr)
		spr->NearPtr = NO_PTR;
}


static void SNRmTake(Sprite *spr) {
	if (spr)
		spr->TakePtr = NO_PTR;
}


void SNSeq(Sprite *spr, int val) {
	if (spr) {
		if (spr == Hero && val == 0)
			Hero->park();
		else
			spr->Step(val);
	}
}


void SNRSeq(Sprite *spr, int val) {
	if (spr)
		SNSeq(spr, spr->_seqPtr + val);
}


void SNSend(Sprite *spr, int val) {
	if (spr) {
		int was = spr->_cave;
		bool was1 = (was == 0 || was == Now);
		bool val1 = (val == 0 || val == Now);
		spr->_cave = val;
		if (val1 != was1) {
			if (was1) {
				if (spr->_flags._kept) {
					int n = FindPocket(spr);
					if (n >= 0)
						_pocket[n] = NULL;
				}
				Hide1(spr);
				ContractSprite(spr);
				spr->_flags._slav = false;
			} else {
				if (spr->_ref % 1000 == 0)
					Bitmap::_pal = VGA::SysPal;
				if (spr->_flags._back)
					spr->BackShow(true);
				else
					ExpandSprite(spr);
				Bitmap::_pal = NULL;
			}
		}
	}
}


void SNSwap(Sprite *spr, int xref) {
	Sprite *xspr = Locate(xref);
	if (spr && xspr) {
		int was = spr->_cave;
		int xwas = xspr->_cave;
		bool was1 = (was == 0 || was == Now);
		bool xwas1 = (xwas == 0 || xwas == Now);

		Swap(spr->_cave, xspr->_cave);
		Swap(spr->_x, xspr->_x);
		Swap(spr->_y, xspr->_y);
		Swap(spr->_z, xspr->_z);
		if (spr->_flags._kept) {
			int n = FindPocket(spr);
			if (n >= 0)
				_pocket[n] = xspr;
			xspr->_flags._kept = true;
			xspr->_flags._port = false;
		}
		if (xwas1 != was1) {
			if (was1) {
				Hide1(spr);
				ContractSprite(spr);
			} else
				ExpandSprite(spr);
			if (xwas1) {
				Hide1(xspr);
				ContractSprite(xspr);
			} else
				ExpandSprite(xspr);
		}
	}
}


void SNCover(Sprite *spr, int xref) {
	Sprite *xspr = Locate(xref);
	if (spr && xspr) {
		spr->_flags._hide = true;
		xspr->_z = spr->_z;
		xspr->_cave = spr->_cave;
		xspr->Goto(spr->_x, spr->_y);
		ExpandSprite(xspr);
		if ((xspr->_flags._shad = spr->_flags._shad) == 1) {
			Vga->ShowQ->Insert(Vga->ShowQ->Remove(spr->_prev), xspr);
			spr->_flags._shad = false;
		}
		FeedSnail(xspr, NEAR);
	}
}


void SNUncover(Sprite *spr, Sprite *xspr) {
	if (spr && xspr) {
		spr->_flags._hide = false;
		spr->_cave = xspr->_cave;
		spr->Goto(xspr->_x, xspr->_y);
		if ((spr->_flags._shad = xspr->_flags._shad) == 1) {
			Vga->ShowQ->Insert(Vga->ShowQ->Remove(xspr->_prev), spr);
			xspr->_flags._shad = false;
		}
		spr->_z = xspr->_z;
		SNSend(xspr, -1);
		if (spr->_time == 0)
			++spr->_time;
	}
}


void SNSetX0(int cav, int x0) {
	HeroXY[cav - 1]._x = x0;
}


void SNSetY0(int cav, int y0) {
	HeroXY[cav - 1]._y = y0;
}


void SNSetXY(Sprite *spr, uint16 xy) {
	if (spr)
		spr->Goto(xy % SCR_WID, xy / SCR_WID);
}


void SNRelX(Sprite *spr, int x) {
	if (spr && Hero)
		spr->Goto(Hero->_x + x, spr->_y);
}


void SNRelY(Sprite *spr, int y) {
	if (spr && Hero)
		spr->Goto(spr->_x, Hero->_y + y);
}


void SNRelZ(Sprite *spr, int z) {
	if (spr && Hero) {
		spr->_z = Hero->_z + z;
		SNZTrim(spr);
	}
}


void SNSetX(Sprite *spr, int x) {
	if (spr)
		spr->Goto(x, spr->_y);
}


void SNSetY(Sprite *spr, int y) {
	if (spr)
		spr->Goto(spr->_x, y);
}


void SNSetZ(Sprite *spr, int z) {
	if (spr) {
		spr->_z = z;
		//SNPOST_(SNZTRIM, -1, 0, spr);
		SNZTrim(spr);
	}
}


void SNSlave(Sprite *spr, int ref) {
	Sprite *slv = Locate(ref);
	if (spr && slv) {
		if (spr->Active()) {
			SNSend(slv, spr->_cave);
			slv->_flags._slav = true;
			slv->_z = spr->_z;
			Vga->ShowQ->Insert(Vga->ShowQ->Remove(slv), spr->_next);
		}
	}
}


void SNTrans(Sprite *spr, int trans) {
	if (spr)
		spr->_flags._tran = (trans < 0) ? !spr->_flags._tran : (trans != 0);
}


void SNPort(Sprite *spr, int port) {
	if (spr)
		spr->_flags._port = (port < 0) ? !spr->_flags._port : (port != 0);
}


void SNKill(Sprite *spr) {
	if (spr) {
		if (spr->_flags._kept) {
			int n = FindPocket(spr);
			if (n >= 0)
				_pocket[n] = NULL;
		}
		Sprite *nx = spr->_next;
		Hide1(spr);
		Vga->ShowQ->Remove(spr);
		MOUSE::ClrEvt(spr);
		if (spr->_flags._kill)
			delete spr;
		else {
			spr->_cave = -1;
			Vga->SpareQ->Append(spr);
		}
		if (nx) {
			if (nx->_flags._slav)
				SNKill(nx);
		}
	}
}


static void SNSound(Sprite *spr, int wav, int cnt) {
	if (SNDDrvInfo.DDEV) {
		if (wav == -1)
			Sound.Stop();
		else
			Sound.Play(Fx[wav], (spr) ? ((spr->_x + spr->_w / 2) / (SCR_WID / 16)) : 8, cnt);
	}
}


void SNKeep(Sprite *spr, int stp) {
	SelectPocket(-1);
	if (spr && ! spr->_flags._kept && _pocket[PocPtr] == NULL) {
		SNSound(spr, 3, 1);
		_pocket[PocPtr] = spr;
		spr->_cave = 0;
		spr->_flags._kept = true;
		spr->Goto(POCKET_X + POCKET_DX * PocPtr + POCKET_DX / 2 - spr->_w / 2,
		          POCKET_Y + POCKET_DY / 2 - spr->_h / 2);
		if (stp >= 0)
			spr->Step(stp);
	}
	SelectPocket(-1);
}


void SNGive(Sprite *spr, int stp) {
	if (spr) {
		int p = FindPocket(spr);
		if (p >= 0) {
			_pocket[p] = NULL;
			spr->_cave = Now;
			spr->_flags._kept = false;
			if (stp >= 0)
				spr->Step(stp);
		}
	}
	SelectPocket(-1);
}


static void SNBackPt(Sprite *spr, int stp) {
	if (spr) {
		if (stp >= 0)
			spr->Step(stp);
		spr->BackShow(true);
	}
}


static void SNLevel(Sprite *spr, int lev) {
#ifdef    DEMO
	static int maxcav[] = { CAVE_MAX };
#else
	static int maxcav[] = { 1, 8, 16, 23, 24 };
#endif
	while (Lev < lev) {
		++Lev;
		spr = Vga->SpareQ->Locate(100 + Lev);
		if (spr) {
			spr->BackShow(true);
			spr->_cave = 0;
		}
	}
	MaxCave = maxcav[Lev];
	if (spr)
		spr->_flags._hide = false;
}


static void SNFlag(int fn, bool v) {
	Flag[fn] = v;
}


static void SNSetRef(Sprite *spr, int nr) {
	if (spr)
		spr->_ref = nr;
}


void SNFlash(bool on) {
	if (on) {
		Dac *pal = farnew(Dac, PAL_CNT);
		if (pal) {
			memcpy(pal, VGA::SysPal, PAL_SIZ);
			for (int i = 0; i < PAL_CNT; i++) {
				register int c;
				c = pal[i]._r << 1;
				pal[i]._r = (c < 64) ? c : 63;
				c = pal[i]._g << 1;
				pal[i]._g = (c < 64) ? c : 63;
				c = pal[i]._b << 1;
				pal[i]._b = (c < 64) ? c : 63;
			}
			Vga->SetColors(pal, 64);
		}
	} else
		Vga->SetColors(VGA::SysPal, 64);
	Dark = false;
}


static void SNLight(bool in) {
	if (in)
		Vga->Sunrise(VGA::SysPal);
	else
		Vga->Sunset();
	Dark = ! in;
}


static void SNBarrier(int cav, int bar, bool horz) {
	((uint8 *)(Barriers + ((cav > 0) ? cav : Now)))[horz] = bar;
}


static void SNWalk(Sprite *spr, int x, int y) {
	if (Hero) {
		if (spr && y < 0)
			Hero->findWay(spr);
		else
			Hero->findWay(XZ(x, y));
	}
}


static void SNReach(Sprite *spr, int mode) {
	if (Hero)
		Hero->reach(spr, mode);
}


static void SNMouse(bool on) {
	if (on)
		Mouse->On();
	else
		Mouse->Off();
}


void SNAIL::RunCom(void) {
	static int count = 1;
	if (! Busy) {
		Busy = true;
		uint8 tmphea = Head;
		while (Tail != tmphea) {
			COM *snc = &SNList[Tail];

			if (! Turbo) { // only for the slower one
				if (_timerExpiry && (_timerExpiry > g_system->getMillis()))
					break;
				else {
					if (TextDelay) {
						KillText();
						TextDelay = false;
					}
				}
				if (Talk && snc->Com != SNPAUSE)
					break;
			}

			Sprite *sprel = ((snc->Ref >= 0) ? Locate(snc->Ref) : ((Sprite *) snc->Ptr));
			switch (snc->Com) {
			case SNLABEL    :
				break;
			case SNPAUSE    :
				_timerExpiry = g_system->getMillis() + snc->Val * SNAIL_FRAME_DELAY;
				if (Talk)
					TextDelay = true;
				break;
			case SNWAIT     :
				if (sprel) {
					if (sprel->SeqTest(snc->Val) &&
					        (snc->Val >= 0 || sprel != Hero || Hero->_tracePtr < 0)) {
						_timerExpiry = g_system->getMillis() + sprel->_time * SNAIL_FRAME_DELAY;
					} else
						goto xit;
				}
				break;
			case SNLEVEL    :
				SNLevel(sprel, snc->Val);
				break;
			case SNHIDE     :
				SNHide(sprel, snc->Val);
				break;
			case SNSAY      :
				if (sprel && TalkEnable) {
					if (sprel == Hero && sprel->SeqTest(-1))
						sprel->Step(HTALK);
					Text->Say(Text->getText(snc->Val), sprel);
					Sys->FunDel = HEROFUN0;
				}
				break;
			case SNINF      :
				if (TalkEnable) {
					_vm->inf(Text->getText(snc->Val));
					Sys->FunDel = HEROFUN0;
				}
				break;
			case SNTIME     :
				if (sprel && TalkEnable) {
					if (sprel == Hero && sprel->SeqTest(-1))
						sprel->Step(HTALK);
					SayTime(sprel);
				}
				break;
			case SNCAVE     :
				// SwitchCave(snc->Val);
				warning("Problematic call of SwitchCave in SNAIL::RunCom");
				break;
			case SNKILL     :
				SNKill(sprel);
				break;
			case SNSEQ      :
				SNSeq(sprel, snc->Val);
				break;
			case SNRSEQ     :
				SNRSeq(sprel, snc->Val);
				break;
			case SNSEND     :
				SNSend(sprel, snc->Val);
				break;
			case SNSWAP     :
				SNSwap(sprel, snc->Val);
				break;
			case SNCOVER    :
				SNCover(sprel, snc->Val);
				break;
			case SNUNCOVER  :
				SNUncover(sprel, (snc->Val >= 0) ? Locate(snc->Val) : ((Sprite *) snc->Ptr));
				break;
			case SNKEEP     :
				SNKeep(sprel, snc->Val);
				break;
			case SNGIVE     :
				SNGive(sprel, snc->Val);
				break;
			case SNGAME     :
				SNGame(sprel, snc->Val);
				break;
			case SNSETX0    :
				SNSetX0(snc->Ref, snc->Val);
				break;
			case SNSETY0    :
				SNSetY0(snc->Ref, snc->Val);
				break;
			case SNSETXY    :
				SNSetXY(sprel, snc->Val);
				break;
			case SNRELX     :
				SNRelX(sprel, snc->Val);
				break;
			case SNRELY     :
				SNRelY(sprel, snc->Val);
				break;
			case SNRELZ     :
				SNRelZ(sprel, snc->Val);
				break;
			case SNSETX     :
				SNSetX(sprel, snc->Val);
				break;
			case SNSETY     :
				SNSetY(sprel, snc->Val);
				break;
			case SNSETZ     :
				SNSetZ(sprel, snc->Val);
				break;
			case SNSLAVE    :
				SNSlave(sprel, snc->Val);
				break;
			case SNTRANS    :
				SNTrans(sprel, snc->Val);
				break;
			case SNPORT     :
				SNPort(sprel, snc->Val);
				break;
			case SNNEXT     :
				break;
			case SNIF       :
				break;
			case SNTALK     :
				break;
			case SNMOUSE    :
				SNMouse(snc->Val != 0);
				break;
			case SNNNEXT    :
				SNNNext(sprel, snc->Val);
				break;
			case SNTNEXT    :
				SNTNext(sprel, snc->Val);
				break;
			case SNRNNEXT   :
				SNRNNext(sprel, snc->Val);
				break;
			case SNRTNEXT   :
				SNRTNext(sprel, snc->Val);
				break;
			case SNRMNEAR   :
				SNRmNear(sprel);
				break;
			case SNRMTAKE   :
				SNRmTake(sprel);
				break;
			case SNFLAG     :
				SNFlag(snc->Ref & 3, snc->Val != 0);
				break;
			case SNSETREF   :
				SNSetRef(sprel, snc->Val);
				break;
			case SNBACKPT   :
				SNBackPt(sprel, snc->Val);
				break;
			case SNFLASH    :
				SNFlash(snc->Val != 0);
				break;
			case SNLIGHT    :
				SNLight(snc->Val != 0);
				break;
			case SNSETHB    :
				SNBarrier(snc->Ref, snc->Val, true);
				break;
			case SNSETVB    :
				SNBarrier(snc->Ref, snc->Val, false);
				break;
			case SNWALK     :
				SNWalk(sprel, snc->Ref, snc->Val);
				break;
			case SNREACH    :
				SNReach(sprel, snc->Val);
				break;
			case SNSOUND    :
				SNSound(sprel, snc->Val, count);
				count = 1;
				break;
			case SNCOUNT    :
				count = snc->Val;
				break;
			case SNEXEC     :
			//	TODO: Handle correctly the execution of function pointer coming from Message send SNPOST
			//	((void(*)(int)) (snc->Ptr))(snc->Val); break;
				warning("STUB: SNEXEC code");
			case SNSTEP     :
				sprel->Step();
				break;
			case SNZTRIM    :
				SNZTrim(sprel);
				break;
			case SNGHOST    :
				SNGhost((Bitmap *) snc->Ptr);
				break;
			default :
				warning("Unhandled snc->Com in SNMouse(bool)");
				break;
			}
			Tail++;
			if (!Turbo)
				break;
		}
xit:
		Busy = false;
	}
}


bool SNAIL::Idle(void) {
	return (Head == Tail);
}

} // End of namespace CGE
