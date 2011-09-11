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
#include "cge/walk.h"

namespace CGE {

void CGEEngine::snGame(Sprite *spr, int num) {
	debugC(1, kCGEDebugEngine, "CGEEngine::snGame(spr, %d)", num);

	switch (num) {
	case 1: {
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
			int i = newRandom(3), hand = (dup[0]->_shpCnt == 6);
			Stage++;
			if (hand && Stage > kDressed)
				++hand;
			if (i >= 0 || (dup[i] == spr && newRandom(3) == 0)) {
				_snail->addCom(kSnSeq, -1, 3, dup[0]);               // yes
				_snail->addCom(kSnSeq, -1, 3, dup[1]);               // yes
				_snail->addCom(kSnSeq, -1, 3, dup[2]);               // yes
				_snail->addCom(kSnTNext, -1, 0, dup[0]);             // reset Take
				_snail->addCom(kSnTNext, -1, 0, dup[1]);             // reset Take
				_snail->addCom(kSnTNext, -1, 0, dup[2]);             // reset Take
				_snail->addCom(kSnNNext, -1, 0, dup[0]);             // reset Near
				_snail->addCom(kSnPause, -1, 72, NULL);              // little rest
				_snail->addCom(kSnSay, 1, 16009, NULL);              // hura
				_snail->addCom(kSnSay, buref, 16010, NULL);          // siadaj
				_snail->addCom(kSnSay, 1, 16011, NULL);              // postoj‘

				if (hand) {
					_snail->addCom(kSnSend, 16060 + hand, 16, NULL);   // dawaj r‘k‘
					_snail->addCom(kSnSeq, buref, 4, NULL);            // zdejmowanie
					_snail->addCom(kSnSeq, 16060 + hand, 1, NULL);     // ruch
					_snail->addCom(kSnSound, 16060 + hand, 16002, NULL); // szelest
					_snail->addCom(kSnWait, 16060 + hand, 3, NULL);    // podniesie
					_snail->addCom(kSnSwap, buref, buref + 100, NULL); // rozdziana
					_snail->addCom(kSnSeq, 16016, Stage, NULL);        // rožnie kupa
					_snail->addCom(kSnSend, 16060 + hand, -1, NULL);   // chowaj r‘k‘
					_snail->addCom(kSnWait, 16060 + hand, -1, NULL);   // r‘ka zamar’a
				} else {
					_snail->addCom(kSnSeq, buref, 4, NULL);            // zdejmowanie
					_snail->addCom(kSnSound, 16060 + hand, 16002, NULL); // szelest
					_snail->addCom(kSnWait, buref, -1, NULL);          // zdejmie
					_snail->addCom(kSnSwap, buref, buref + 100, NULL); // rozdziana
					_snail->addCom(kSnSeq, 16016, Stage, NULL);        // rožnie kupa
				}
				//SNPOST(SNSEQ, buref+100, 0, NULL);        // reset
				_snail->addCom(kSnPause, -1, 72, NULL);              // chwilk‘...
				_snail->addCom(kSnSeq, -1, 0, dup[1]);               // odstaw Go
				_snail->addCom(kSnSetXY, -1, 203 + kScrWidth * 49, dup[1]);
				_snail->addCom(kSnSetZ, -1, 7, dup[1]);
				_snail->addCom(kSnSeq, -1, 0, dup[2]);               // odstaw J†
				_snail->addCom(kSnSetXY, -1, 182 + kScrWidth * 62, dup[2]);
				_snail->addCom(kSnSetZ, -1, 9, dup[2]);
				_game = 0;
				return;
			} else {
				_snail->addCom(kSnSeq, -1, 2, dup[0]);               // no
				_snail->addCom(kSnSeq, -1, 2, dup[1]);               // no
				_snail->addCom(kSnSeq, -1, 2, dup[2]);               // no
				_snail->addCom(kSnPause, -1, 72, NULL);              // 1 sec
			}
		}
		_snail->addCom(kSnWalk, 198, 134, NULL);                 // na miejsce
		_snail->addCom(kSnWait, 1, -1, NULL);                    // stoi
		_snail->addCom(kSnCover, 1, 16101, NULL);                // ch’op do bicia
		_snail->addCom(kSnSeq, 16101, 1, NULL);                  // wystaw
		_snail->addCom(kSnWait, 16101, 5, NULL);                 // czekaj
		_snail->addCom(kSnPause, 16101, 24, NULL);               // czekaj chwil‘
		_snail->addCom(kSnSeq, 16040, 1, NULL);                  // plask
		_snail->addCom(kSnSound, 16101, 16001, NULL);            // plask!
		_snail->addCom(kSnPause, 16101, 24, NULL);               // czekaj chwil‘
		_snail->addCom(kSnSeq, 16040, 0, NULL);                  // schowaj plask
		_snail->addCom(kSnWait, 16101, -1, NULL);                // stoi
		_snail->addCom(kSnUncover, 1, 16101, NULL);              // SDS
		if (!_game) {
			_snail->addCom(kSnSay, buref, 16008, NULL);            // zgadnij!
			_game = true;
		}
		}
		break;
	case 2:
		if (_sprTv == NULL) {
			_sprTv = _vga->_showQ->locate(20700);
			_sprK1 = _vga->_showQ->locate(20701);
			_sprK2 = _vga->_showQ->locate(20702);
			_sprK3 = _vga->_showQ->locate(20703);
		}

		if (!_game) { // init
			_snail->addCom(kSnGame, 20002, 2, NULL);
			_game = true;
			break;
		}

		// cont
		_sprK1->step(newRandom(6));
		_sprK2->step(newRandom(6));
		_sprK3->step(newRandom(6));

		if (spr->_ref == 1 && _keyboard->_key[kKeyAlt]) {
			_sprK1->step(5);
			_sprK2->step(5);
			_sprK3->step(5);
		}

		_snail->addCom(kSnSetZ, 20700, 0, NULL);
		bool hit = (_sprK1->_seqPtr + _sprK2->_seqPtr + _sprK3->_seqPtr == 15);
		if (hit) {
			if (spr->_ref == 1) {
				_snail->addCom(kSnSay,       1, 20003, NULL);       // hura!
				_snail->addCom(kSnSeq,   20011,     2, NULL);       // kamera won
				_snail->addCom(kSnSend,  20701,    -1, NULL);       // k1 won
				_snail->addCom(kSnSend,  20702,    -1, NULL);       // k2 won
				_snail->addCom(kSnSend,  20703,    -1, NULL);       // k3 won
				_snail->addCom(kSnSend,  20700,    -1, NULL);       // tv won
				_snail->addCom(kSnKeep,  20007,     0, NULL);       // do kieszeni
				_snail->addCom(kSnSend,  20006,    20, NULL);       // bilon
				_snail->addCom(kSnSound, 20006, 20002, NULL);      // bilon!
				_snail->addCom(kSnSay,   20002, 20004, NULL);
				_snail->addCom(kSnSend,  20010,    20, NULL);       // papier
				_snail->addCom(kSnSound, 20010, 20003, NULL);      // papier!
				_snail->addCom(kSnSay,   20001, 20005, NULL);
				_game = false;
				return;
			} else
				_sprK3->step(newRandom(5));
		}

		if (_gameCase2Cpt < 100) {
			switch (_gameCase2Cpt) {
			case 15:
				_snail->addCom(kSnSay, 20003, 20021, NULL);
				break;
			case 30:
			case 45:
			case 60:
			case 75:
				_snail->addCom(kSnSay, 20003, 20022, NULL);
				break;
			}
			_gameCase2Cpt++;
		}

		switch (spr->_ref) {
		case 1:
			_snail->addCom(kSnSay,   20001, 20011, NULL);         // zapro
			_snail->addCom(kSnSeq,   20001,     1, NULL);             // rzu
			_snail->addCom(kSnWait,  20001,     1, NULL);             // czekaj
			_snail->addCom(kSnSetZ,  20700,     2, NULL);             // skryj k
			_snail->addCom(kSnHide,  20007,     1, NULL);             // skryj k
			_snail->addCom(kSnWait,  20001,    16, NULL);            // czekaj
			_snail->addCom(kSnSeq,   20007,     1, NULL);             // lec†
			_snail->addCom(kSnHide,  20007,     0, NULL);             // poka§
			_snail->addCom(kSnSound, 20007, 20001, NULL);        // grzech
			_snail->addCom(kSnWait,  20007,    -1, NULL);            // koniec
			_snail->addCom(kSnGame,  20001,     2, NULL);             // again!
			break;

		case 20001:
			_snail->addCom(kSnSay,   20002, 20012, NULL);          // zapro
			_snail->addCom(kSnSeq,   20002,     1, NULL);              // rzu
			_snail->addCom(kSnWait,  20002,     3, NULL);             // czekaj
			_snail->addCom(kSnSetZ,  20700,     2, NULL);             // skryj k
			_snail->addCom(kSnHide,  20007,     1, NULL);             // skryj k
			_snail->addCom(kSnWait,  20002,    10, NULL);            // czekaj
			_snail->addCom(kSnSeq,   20007,     2, NULL);              // lec†
			_snail->addCom(kSnHide,  20007,     0, NULL);             // poka§
			_snail->addCom(kSnSound, 20007, 20001, NULL);        // grzech
			_snail->addCom(kSnWait,  20007,    -1, NULL);            // koniec
			_snail->addCom(kSnGame,  20002,     2, NULL);             // again!
			break;

		case 20002:
			_snail->addCom(kSnSay,   20002, 20010, NULL);          // zapro
			_snail->addCom(kSnWalk,  20005,    -1, NULL);            // do stol
			_snail->addCom(kSnWait,      1,    -1, NULL);                // stoi
			_snail->addCom(kSnCover,     1, 20101, NULL);            // grasol
			_snail->addCom(kSnSeq,   20101,     1, NULL);              // rzu
			_snail->addCom(kSnWait,  20101,     5, NULL);             // czekaj
			_snail->addCom(kSnSetZ,  20700,     2, NULL);             // skryj k
			_snail->addCom(kSnHide,  20007,     1, NULL);             // skryj k
			_snail->addCom(kSnWait,  20101,    15, NULL);            // czekaj
			_snail->addCom(kSnSeq,   20007,     1, NULL);              // lec†
			_snail->addCom(kSnHide,  20007,     0, NULL);             // poka§
			_snail->addCom(kSnSound, 20007, 20001, NULL);        // grzech
			_snail->addCom(kSnWait,  20101,    -1, NULL);            // koniec
			_snail->addCom(kSnUncover,   1, 20101, NULL);          // SDS
			_snail->addCom(kSnGame,      1,     2, NULL);                 // again!
			break;
		}
	}
}

void CGEEngine::expandSprite(Sprite *spr) {
	debugC(5, kCGEDebugEngine, "CGEEngine::expandSprite(spr)");

	if (spr)
		_vga->_showQ->insert(_vga->_spareQ->remove(spr));
}

void CGEEngine::contractSprite(Sprite *spr) {
	debugC(1, kCGEDebugEngine, "CGEEngine::contractSprite(spr)");

	if (spr)
		_vga->_spareQ->append(_vga->_showQ->remove(spr));
}

int CGEEngine::findPocket(Sprite *spr) {
	debugC(1, kCGEDebugEngine, "CGEEngine::findPocket(spr)");

	for (int i = 0; i < kPocketNX; i++)
		if (_pocket[i] == spr)
			return i;
	return -1;
}

void CGEEngine::selectPocket(int n) {
	debugC(1, kCGEDebugEngine, "CGEEngine::selectPocket(%d)", n);

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
	_pocLight->gotoxy(kPocketX + _pocPtr * kPocketDX + kPocketSX, kPocketY + kPocketSY);
}

void CGEEngine::pocFul() {
	debugC(1, kCGEDebugEngine, "CGEEngine::pocFul()");

	_hero->park();
	_snail->addCom(kSnWait, -1, -1, _hero);
	_snail->addCom(kSnSeq, -1, kSeqPocketFull, _hero);
	_snail->addCom(kSnSound, -1, 2, _hero);
	_snail->addCom(kSnWait, -1, -1, _hero);
	_snail->addCom(kSnSay,  1, kPocketFull, _hero);
}

void CGEEngine::hide1(Sprite *spr) {
	debugC(1, kCGEDebugEngine, "CGEEngine::hide1(spr)");

	_snail_->addCom(kSnGhost, -1, 0, spr->ghost());
}

void CGEEngine::snGhost(Bitmap *bmp) {
	debugC(1, kCGEDebugEngine, "CGEEngine::snGhost(bmp)");

	bmp->hide(bmp->_map & 0xFFFF, bmp->_map >> 16);
	bmp->_m = NULL;
	bmp->_map = 0;
	delete bmp;
}

void CGEEngine::feedSnail(Sprite *spr, SnList snq) {
	debugC(1, kCGEDebugEngine, "CGEEngine::feedSnail(spr, snq)");

	if (!spr || !spr->active())
		return;

	uint8 ptr = (snq == kTake) ? spr->_takePtr : spr->_nearPtr;

	if (ptr == kNoPtr)
		return;

	Snail::Com *comtab = spr->snList(snq);
	Snail::Com *c = comtab + ptr;

	if (findPocket(NULL) < 0) {                 // no empty pockets?
		Snail::Com *p;
		for (p = c; p->_com != kSnNext; p++) {     // find KEEP command
			if (p->_com == kSnKeep) {
				pocFul();
				return;
			}
			if (p->_ptr)
				break;
		}
	}
	while (true) {
		if (c->_com == kSnTalk) {
			if ((_snail->_talkEnable = (c->_val != 0)) == false)
				killText();
		}
		if (c->_com == kSnNext) {
			Sprite *s = (c->_ref < 0) ? spr : locate(c->_ref);
			if (s) {
				uint8 *idx = (snq == kTake) ? &s->_takePtr : &s->_nearPtr;
				if (*idx != kNoPtr) {
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
		if (c->_com == kSnIf) {
			Sprite *s = (c->_ref < 0) ? spr : locate(c->_ref);
			if (s) { // sprite extsts
				if (! s->seqTest(-1))
					c = comtab + c->_val;                // not parked
				else
					++c;
			} else
				++c;
		} else {
			_snail->addCom(c->_com, c->_ref, c->_val, spr);
			if (c->_ptr)
				break;
			else
				c++;
		}
	}
}

const char *Snail::_comText[] = {
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
	  _head(0), _tail(0), _snList((Com *)malloc(sizeof(Com) * 256)), _vm(vm) {
}

Snail::~Snail() {
	free(_snList);
}

void Snail::addCom(SnCom com, int ref, int val, void *ptr) {
	Com *snc = &_snList[_head++];
	snc->_com = com;
	snc->_ref = ref;
	snc->_val = val;
	snc->_ptr = ptr;
	snc->_cbType = kNullCB;
	if (com == kSnClear) {
		_tail = _head;
		killText();
		_timerExpiry = 0;
	}
}

void Snail::addCom2(SnCom com, int ref, int val, CallbackType cbType) {
	Com *snc = &_snList[_head++];
	snc->_com = com;
	snc->_ref = ref;
	snc->_val = val;
	snc->_ptr = NULL;
	snc->_cbType = cbType;
	if (com == kSnClear) {
		_tail = _head;
		killText();
		_timerExpiry = 0;
	}
}

void Snail::insCom(SnCom com, int ref, int val, void *ptr) {
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
	if (com == kSnClear) {
		_tail = _head;
		killText();
		_timerExpiry = 0;
	}
}

void CGEEngine::snNNext(Sprite *spr, int p) {
	debugC(1, kCGEDebugEngine, "CGEEngine::snNNext(spr, %d)", p);

	if (spr)
		if (spr->_nearPtr != kNoPtr)
			spr->_nearPtr = p;
}

void CGEEngine::snTNext(Sprite *spr, int p) {
	debugC(1, kCGEDebugEngine, "CGEEngine::snTNext(spr, %d)", p);

	if (spr)
		if (spr->_takePtr != kNoPtr)
			spr->_takePtr = p;
}

void CGEEngine::snRNNext(Sprite *spr, int p) {
	debugC(1, kCGEDebugEngine, "CGEEngine::snRNNext(spr, %d)", p);

	if (spr)
		if (spr->_nearPtr != kNoPtr)
			spr->_nearPtr += p;
}


void CGEEngine::snRTNext(Sprite *spr, int p) {
	debugC(1, kCGEDebugEngine, "CGEEngine::snRTNext(spr, %d)", p);

	if (spr)
		if (spr->_takePtr != kNoPtr)
			spr->_takePtr += p;
}

void CGEEngine::snZTrim(Sprite *spr) {
	debugC(4, kCGEDebugEngine, "CGEEngine::snZTrim(spr)");

	if (!spr || !spr->active())
		return;

	Sprite *s = (spr->_flags._shad) ? spr->_prev : NULL;
	_vga->_showQ->insert(_vga->_showQ->remove(spr));
	if (s) {
		s->_z = spr->_z;
		_vga->_showQ->insert(_vga->_showQ->remove(s), spr);
	}
}

void CGEEngine::snHide(Sprite *spr, int val) {
	debugC(1, kCGEDebugEngine, "CGEEngine::snHide(spr, %d)", val);

	if (spr) {
		spr->_flags._hide = (val >= 0) ? (val != 0) : (!spr->_flags._hide);
		if (spr->_flags._shad)
			spr->_prev->_flags._hide = spr->_flags._hide;
	}
}

void CGEEngine::snRmNear(Sprite *spr) {
	debugC(1, kCGEDebugEngine, "CGEEngine::snRmNear(spr)");

	if (spr)
		spr->_nearPtr = kNoPtr;
}

void CGEEngine::snRmTake(Sprite *spr) {
	debugC(1, kCGEDebugEngine, "CGEEngine::snRmTake(spr)");

	if (spr)
		spr->_takePtr = kNoPtr;
}

void CGEEngine::snSeq(Sprite *spr, int val) {
	debugC(1, kCGEDebugEngine, "CGEEngine::snSeq(spr, %d)", val);

	if (spr) {
		if (spr == _hero && val == 0)
			_hero->park();
		else
			spr->step(val);
	}
}

void CGEEngine::snRSeq(Sprite *spr, int val) {
	debugC(1, kCGEDebugEngine, "CGEEngine::snRSeq(spr, %d)", val);

	if (spr)
		snSeq(spr, spr->_seqPtr + val);
}

void CGEEngine::snSend(Sprite *spr, int val) {
	debugC(1, kCGEDebugEngine, "CGEEngine::snSend(spr, %d)", val);

	if (!spr)
		return;

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
				Bitmap::_pal = _vga->_sysPal;
			if (spr->_flags._back)
				spr->backShow(true);
			else
				expandSprite(spr);
			Bitmap::_pal = NULL;
		}
	}
}

void CGEEngine::snSwap(Sprite *spr, int xref) {
	debugC(1, kCGEDebugEngine, "CGEEngine::snSwap(spr, %d)", xref);

	Sprite *xspr = locate(xref);
	if (!spr || !xspr)
		return;

	int was = spr->_cave;
	int xwas = xspr->_cave;
	bool was1 = (was == 0 || was == _now);
	bool xwas1 = (xwas == 0 || xwas == _now);

	SWAP(spr->_cave, xspr->_cave);
	SWAP(spr->_x, xspr->_x);
	SWAP(spr->_y, xspr->_y);
	SWAP(spr->_z, xspr->_z);
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

void CGEEngine::snCover(Sprite *spr, int xref) {
	debugC(1, kCGEDebugEngine, "CGEEngine::snCover(spr, %d)", xref);

	Sprite *xspr = locate(xref);
	if (!spr || !xspr)
		return;

	spr->_flags._hide = true;
	xspr->_z = spr->_z;
	xspr->_cave = spr->_cave;
	xspr->gotoxy(spr->_x, spr->_y);
	expandSprite(xspr);
	if ((xspr->_flags._shad = spr->_flags._shad) == 1) {
		_vga->_showQ->insert(_vga->_showQ->remove(spr->_prev), xspr);
		spr->_flags._shad = false;
	}
	feedSnail(xspr, kNear);
}

void CGEEngine::snUncover(Sprite *spr, Sprite *xspr) {
	debugC(1, kCGEDebugEngine, "CGEEngine::snUncover(spr, xspr)");

	if (!spr || !xspr)
		return;

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

void CGEEngine::snSetX0(int cav, int x0) {
	debugC(1, kCGEDebugEngine, "CGEEngine::snSetX0(%d, %d)", cav, x0);

	_heroXY[cav - 1].x = x0;
}

void CGEEngine::snSetY0(int cav, int y0) {
	debugC(1, kCGEDebugEngine, "CGEEngine::snSetY0(%d, %d)", cav, y0);

	_heroXY[cav - 1].y = y0;
}

void CGEEngine::snSetXY(Sprite *spr, uint16 xy) {
	debugC(1, kCGEDebugEngine, "CGEEngine::snSetXY(spr, %d)", xy);

	if (spr)
		spr->gotoxy(xy % kScrWidth, xy / kScrWidth);
}

void CGEEngine::snRelX(Sprite *spr, int x) {
	debugC(1, kCGEDebugEngine, "CGEEngine::snRelX(spr, %d)", x);

	if (spr && _hero)
		spr->gotoxy(_hero->_x + x, spr->_y);
}

void CGEEngine::snRelY(Sprite *spr, int y) {
	debugC(1, kCGEDebugEngine, "CGEEngine::snRelY(spr, %d)", y);

	if (spr && _hero)
		spr->gotoxy(spr->_x, _hero->_y + y);
}

void CGEEngine::snRelZ(Sprite *spr, int z) {
	debugC(1, kCGEDebugEngine, "CGEEngine::snRelZ(spr, %d)", z);

	if (spr && _hero) {
		spr->_z = _hero->_z + z;
		snZTrim(spr);
	}
}

void CGEEngine::snSetX(Sprite *spr, int x) {
	debugC(1, kCGEDebugEngine, "CGEEngine::snSetX(spr, %d)", x);

	if (spr)
		spr->gotoxy(x, spr->_y);
}

void CGEEngine::snSetY(Sprite *spr, int y) {
	debugC(1, kCGEDebugEngine, "CGEEngine::snSetY(spr, %d)", y);

	if (spr)
		spr->gotoxy(spr->_x, y);
}

void CGEEngine::snSetZ(Sprite *spr, int z) {
	debugC(1, kCGEDebugEngine, "CGEEngine::snSetZ(spr, %d)", z);

	if (spr) {
		spr->_z = z;
		//SNPOST_(SNZTRIM, -1, 0, spr);
		snZTrim(spr);
	}
}

void CGEEngine::snSlave(Sprite *spr, int ref) {
	debugC(1, kCGEDebugEngine, "CGEEngine::snSlave(spr, %d)", ref);

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
	debugC(1, kCGEDebugEngine, "CGEEngine::snTrans(spr, %d)", trans);

	if (spr)
		spr->_flags._tran = (trans < 0) ? !spr->_flags._tran : (trans != 0);
}

void CGEEngine::snPort(Sprite *spr, int port) {
	debugC(1, kCGEDebugEngine, "CGEEngine::snPort(spr, %d)", port);

	if (spr)
		spr->_flags._port = (port < 0) ? !spr->_flags._port : (port != 0);
}

void CGEEngine::snKill(Sprite *spr) {
	debugC(1, kCGEDebugEngine, "CGEEngine::snKill(spr)");

	if (!spr)
		return;

	if (spr->_flags._kept) {
		int n = findPocket(spr);
		if (n >= 0)
			_pocket[n] = NULL;
	}
	Sprite *nx = spr->_next;
	hide1(spr);
	_vga->_showQ->remove(spr);
	_eventManager->clearEvent(spr);
	if (spr->_flags._kill) {
		delete spr;
	} else {
		spr->_cave = -1;
		_vga->_spareQ->append(spr);
	}
	if (nx) {
		if (nx->_flags._slav)
			snKill(nx);
	}
}

void CGEEngine::snSound(Sprite *spr, int wav) {
	debugC(1, kCGEDebugEngine, "CGEEngine::snSound(spr, %d)", wav);

	if (wav == -1)
		_sound->stop();
	else
		_sound->play((*_fx)[wav], (spr) ? ((spr->_x + spr->_w / 2) / (kScrWidth / 16)) : 8);

	_sound->setRepeat(1);
}

void CGEEngine::snKeep(Sprite *spr, int stp) {
	debugC(1, kCGEDebugEngine, "CGEEngine::snKeep(spr, %d)", stp);

	selectPocket(-1);
	if (spr && ! spr->_flags._kept && _pocket[_pocPtr] == NULL) {
		int16 oldRepeat = _sound->getRepeat();
		_sound->setRepeat(1);
		snSound(spr, 3);
		_sound->setRepeat(oldRepeat);
		_pocket[_pocPtr] = spr;
		spr->_cave = 0;
		spr->_flags._kept = true;
		spr->gotoxy(kPocketX + kPocketDX * _pocPtr + kPocketDX / 2 - spr->_w / 2,
		          kPocketY + kPocketDY / 2 - spr->_h / 2);
		if (stp >= 0)
			spr->step(stp);
	}
	selectPocket(-1);
}

void CGEEngine::snGive(Sprite *spr, int stp) {
	debugC(1, kCGEDebugEngine, "CGEEngine::snGive(spr, %d)", stp);

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
	debugC(1, kCGEDebugEngine, "CGEEngine::snBackPt(spr, %d)", stp);

	if (spr) {
		if (stp >= 0)
			spr->step(stp);
		spr->backShow(true);
	}
}

void CGEEngine::snLevel(Sprite *spr, int lev) {
	debugC(1, kCGEDebugEngine, "CGEEngine::snLevel(spr, %d)", lev);

	assert((lev >= 0) && (lev < 5));

	for (int i = 0; i < 5; i++) {
		spr = _vga->_spareQ->locate(100 + i);
		if (spr) {
			if (i <= lev) {
				spr->backShow(true);
				spr->_cave = 0;
				spr->_flags._hide = false;
			} else {
				spr->_flags._hide = true;
				spr->_cave = -1;
			}
		} else {
			warning("SPR not found! ref: %d", 100 + i);
		}
	}

	_lev = lev;
	_maxCave = _maxCaveArr[_lev];
}

void CGEEngine::snFlag(int indx, bool v) {
	_flag[indx] = v;
}

void CGEEngine::snSetRef(Sprite *spr, int nr) {
	debugC(1, kCGEDebugEngine, "CGEEngine::snSetRef(spr, %d)", nr);

	if (spr)
		spr->_ref = nr;
}

void CGEEngine::snFlash(bool on) {
	debugC(1, kCGEDebugEngine, "CGEEngine::snFlash(%s)", on ? "true" : "false");

	if (on) {
		Dac *pal = (Dac *)malloc(sizeof(Dac) * kPalCount);
		if (pal) {
			memcpy(pal, _vga->_sysPal, kPalSize);
			for (int i = 0; i < kPalCount; i++) {
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
		_vga->setColors(_vga->_sysPal, 64);
	_dark = false;
}

void CGEEngine::snLight(bool in) {
	debugC(1, kCGEDebugEngine, "CGEEngine::snLight(%s)", in ? "true" : "false");

	if (in)
		_vga->sunrise(_vga->_sysPal);
	else
		_vga->sunset();
	_dark = !in;
}

void CGEEngine::snHBarrier(const int cave, const int barX) {
	debugC(1, kCGEDebugEngine, "CGEEngine::snHBarrier(%d, %d)", cave, barX);

	_barriers[(cave > 0) ? cave : _now]._horz = barX;
}

void CGEEngine::snVBarrier(const int cave, const int barY) {
	debugC(1, kCGEDebugEngine, "CGEEngine::snVBarrier(%d, %d)", cave, barY);

	_barriers[(cave > 0) ? cave : _now]._vert = barY;
}

void CGEEngine::snWalk(Sprite *spr, int x, int y) {
	debugC(1, kCGEDebugEngine, "CGEEngine::snWalk(spr, %d, %d)", x, y);

	if (_hero) {
		if (spr && y < 0)
			_hero->findWay(spr);
		else
			_hero->findWay(XZ(x, y));
	}
}

void CGEEngine::snReach(Sprite *spr, int mode) {
	debugC(1, kCGEDebugEngine, "CGEEngine::snReach(spr, %d)", mode);

	if (_hero)
		_hero->reach(spr, mode);
}

void CGEEngine::snMouse(bool on) {
	debugC(1, kCGEDebugEngine, "CGEEngine::snMouse(%s)", on ? "true" : "false");

	if (on)
		_mouse->on();
	else
		_mouse->off();
}

void Snail::runCom() {
	if (_busy)
		return;

	_busy = true;
	uint8 tmpHead = _head;
	while (_tail != tmpHead) {
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
			if (_talk && snc->_com != kSnPause)
				break;
		}

		Sprite *spr = ((snc->_ref >= 0) ? locate(snc->_ref) : ((Sprite *) snc->_ptr));
		switch (snc->_com) {
		case kSnLabel:
			break;
		case kSnPause    :
			_timerExpiry = g_system->getMillis() + snc->_val * kSnailFrameDelay;
			if (_talk)
				_textDelay = true;
			break;
		case kSnWait:
			if (spr) {
				if (spr->seqTest(snc->_val) &&
					(snc->_val >= 0 || spr != _hero || _hero->_tracePtr < 0)) {
					_timerExpiry = g_system->getMillis() + spr->_time * kSnailFrameDelay;
				} else
					goto xit;
			}
			break;
		case kSnLevel:
			_vm->snLevel(spr, snc->_val);
			break;
		case kSnHide:
			_vm->snHide(spr, snc->_val);
			break;
		case kSnSay:
			if (spr && _talkEnable) {
				if (spr == _hero && spr->seqTest(-1))
					spr->step(kSeqHTalk);
				_text->say(_text->getText(snc->_val), spr);
				_sys->_funDel = kHeroFun0;
			}
			break;
		case kSnInf:
			if (_talkEnable) {
				_vm->inf(_text->getText(snc->_val));
				_sys->_funDel = kHeroFun0;
			}
			break;
		case kSnTime:
			if (spr && _talkEnable) {
				if (spr == _hero && spr->seqTest(-1))
					spr->step(kSeqHTalk);
				_text->sayTime(spr);
			}
			break;
		case kSnCave:
			_vm->switchCave(snc->_val);
			break;
		case kSnKill:
			_vm->snKill(spr);
			break;
		case kSnSeq:
			_vm->snSeq(spr, snc->_val);
			break;
		case kSnRSeq:
			_vm->snRSeq(spr, snc->_val);
			break;
		case kSnSend:
			_vm->snSend(spr, snc->_val);
			break;
		case kSnSwap:
			_vm->snSwap(spr, snc->_val);
			break;
		case kSnCover:
			_vm->snCover(spr, snc->_val);
			break;
		case kSnUncover:
			_vm->snUncover(spr, (snc->_val >= 0) ? locate(snc->_val) : ((Sprite *) snc->_ptr));
			break;
		case kSnKeep:
			_vm->snKeep(spr, snc->_val);
			break;
		case kSnGive:
			_vm->snGive(spr, snc->_val);
			break;
		case kSnGame:
			_vm->snGame(spr, snc->_val);
			break;
		case kSnSetX0:
			_vm->snSetX0(snc->_ref, snc->_val);
			break;
		case kSnSetY0:
			_vm->snSetY0(snc->_ref, snc->_val);
			break;
		case kSnSetXY:
			_vm->snSetXY(spr, snc->_val);
			break;
		case kSnRelX:
			_vm->snRelX(spr, snc->_val);
			break;
		case kSnRelY:
			_vm->snRelY(spr, snc->_val);
			break;
		case kSnRelZ:
			_vm->snRelZ(spr, snc->_val);
			break;
		case kSnSetX:
			_vm->snSetX(spr, snc->_val);
			break;
		case kSnSetY:
			_vm->snSetY(spr, snc->_val);
			break;
		case kSnSetZ:
			_vm->snSetZ(spr, snc->_val);
			break;
		case kSnSlave:
			_vm->snSlave(spr, snc->_val);
			break;
		case kSnTrans:
			_vm->snTrans(spr, snc->_val);
			break;
		case kSnPort:
			_vm->snPort(spr, snc->_val);
			break;
		case kSnNext:
		case kSnIf:
		case kSnTalk:
			break;
		case kSnMouse:
			_vm->snMouse(snc->_val != 0);
			break;
		case kSnNNext:
			_vm->snNNext(spr, snc->_val);
			break;
		case kSnTNext:
			_vm->snTNext(spr, snc->_val);
			break;
		case kSnRNNext:
			_vm->snRNNext(spr, snc->_val);
			break;
		case kSnRTNext:
			_vm->snRTNext(spr, snc->_val);
			break;
		case kSnRMNear:
			_vm->snRmNear(spr);
			break;
		case kSnRmTake:
			_vm->snRmTake(spr);
			break;
		case kSnFlag:
			_vm->snFlag(snc->_ref & 3, snc->_val != 0);
			break;
		case kSnSetRef:
			_vm->snSetRef(spr, snc->_val);
			break;
		case kSnBackPt:
			_vm->snBackPt(spr, snc->_val);
			break;
		case kSnFlash:
			_vm->snFlash(snc->_val != 0);
			break;
		case kSnLight:
			_vm->snLight(snc->_val != 0);
			break;
		case kSnSetHBarrier:
			_vm->snHBarrier(snc->_ref, snc->_val);
			break;
		case kSnSetVBarrier:
			_vm->snVBarrier(snc->_ref, snc->_val);
			break;
		case kSnWalk:
			_vm->snWalk(spr, snc->_ref, snc->_val);
			break;
		case kSnReach:
			_vm->snReach(spr, snc->_val);
			break;
		case kSnSound:
			_vm->snSound(spr, snc->_val);
			break;
		case kSnCount:
			_sound->setRepeat(snc->_val);
			break;
		case kSnExec:
			switch (snc->_cbType) {
			case kQGame:
				_vm->qGame();
				break;
			case kMiniStep:
				_vm->miniStep(snc->_val);
				break;
			case kXCave:
				_vm->xCave();
				break;
			case kSndSetVolume:
				sndSetVolume();
				break;
			default:
				error("Unknown Callback Type in SNEXEC");
			}
			break;
		case kSnStep:
			spr->step();
			break;
		case kSnZTrim:
			_vm->snZTrim(spr);
			break;
		case kSnGhost:
			_vm->snGhost((Bitmap *) snc->_ptr);
			break;
		default:
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

bool Snail::idle() {
	return (_head == _tail);
}

} // End of namespace CGE
