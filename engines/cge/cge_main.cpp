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

#include "common/scummsys.h"
#include "cge/general.h"
#include "cge/boot.h"
#include "cge/ident.h"
#include "cge/sound.h"
#include "cge/startup.h"
#include "cge/config.h"
#include "cge/vga13h.h"
#include "cge/snail.h"
#include "cge/text.h"
#include "cge/game.h"
#include "cge/events.h"
#include "cge/cfile.h"
#include "cge/vol.h"
#include "cge/talk.h"
#include "cge/vmenu.h"
#include "cge/gettext.h"
#include "cge/mixer.h"
#include "cge/cge_main.h"
#include "cge/cge.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <fcntl.h>
#include "common/str.h"

namespace CGE {

#define STACK_SIZ   (K(2))
#define SVGCHKSUM   (1956 + Now + OldLev + Game + Music + DemoText)

#define SVG0NAME    ("{{INIT}}" SVG_EXT)
#define SVG0FILE    INI_FILE

extern  uint16  _stklen = (STACK_SIZ * 2);

VGA *Vga;
Heart *_heart;
WALK *Hero;
SYSTEM *Sys;
Sprite *_pocLight;
MOUSE *Mouse;
Sprite *_pocket[POCKET_NX];
Sprite *_sprite;
Sprite *_miniCave;
Sprite *_shadow;
Sprite *_horzLine;
INFO_LINE *InfoLine;
Sprite *_cavLight;
INFO_LINE *DebugLine;

BMP_PTR MB[2];
BMP_PTR HL[2];
BMP_PTR MC[3];
BMP_PTR PR[2];
BMP_PTR SP[3];
BMP_PTR LI[5];

SNAIL *Snail;
SNAIL *Snail_;

// 0.75 - 17II95  - full sound support
// 0.76 - 18II95  - small MiniEMS in DEMO,
//		    unhide CavLight in SNLEVEL
//		    keyclick suppress in startup
//		    keyclick on key service in: SYSTEM, GET_TEXT
// 1.01 - 17VII95 - default savegame with sound ON
//		    coditionals EVA for 2-month evaluation version

/*
    char    Copr[] = "Common Game Engine "
                      #ifdef EVA
                        "ú"
                      #else
                       #ifdef CD
                        "ù"
                       #else
                        " "
                       #endif
                      #endif
                         "  version 1.05 ["
                               #if sizeof(INI_FILE) == sizeof(VFILE)
                            "I"
                               #else
                            "i"
                               #endif
                               #if sizeof(PIC_FILE) == sizeof(VFILE)
                            "B"
                               #else
                            "b"
                               #endif
                                "]\n"
             "Copyright (c) 1994 by Janusz B. Wi$niewski";
*/
char Copr[] = "To be fixed - Copr[]";

static char UsrFnam[15] = "\0É±%^þúÈ¼´ ÇÉ";
static int  OldLev      = 0;
static int  DemoText    = DEMO_TEXT;

//--------------------------------------------------------------------------

bool JBW = false;

//-------------------------------------------------------------------------
int     PocPtr      =  0;

static  EMS      *Mini        = MiniEmm.alloc((uint16)MINI_EMM_SIZE);
static  BMP_PTR  *MiniShpList = NULL;
static  BMP_PTR   MiniShp[]   = { NULL, NULL };
static  bool      Finis       = false;
static  int       Startup     = 1;
int	OffUseCount;
uint16 *intStackPtr = false;

HXY     HeroXY[CAVE_MAX] = {{0, 0}};
BAR     Barriers[1 + CAVE_MAX] = { { 0xFF, 0xFF } };


extern  int FindPocket(Sprite *);

extern  Dac _stdPal[58];

void    FeedSnail(Sprite *spr, SNLIST snq);         // defined in SNAIL
uint8   Cluster::_map[MAP_ZCNT][MAP_XCNT];


uint8  &Cluster::cell(void) {
	return _map[_b][_a];
}


bool Cluster::Protected(void) {
/*
	if (A == Barriers[Now].Vert || B == Barriers[Now].Horz)
		return true;

	_DX = (MAP_ZCNT << 8) + MAP_XCNT;
	_BX = (uint16) this;

	asm   mov ax,1
	asm   mov cl,[bx].(COUPLE)A
	asm   mov ch,[bx].(COUPLE)B
	asm   test    cx,0x8080       // (A < 0) || (B < 0)
	asm   jnz xit

	asm   cmp cl,dl
	asm   jge xit
	asm   cmp ch,dh
	asm   jge xit

	//  if (A < 0 || A >= MAP_XCNT || B < 0 || B >= MAP_ZCNT) return true;

	asm   mov al,dl
	asm   mul ch
	asm   xor ch,ch
	asm   add ax,cx
	asm   mov bx,ax
	_BX += (uint16) Map;
	//asm add bx,offset CLUSTER::Map
	asm   mov al,[bx]
	asm   and ax,0xFF
	asm   jz  xit
	asm   mov ax,1

	//  return Map[B][A] != 0;

	xit: return _AX;
	*/

	warning("STUB: CLUSTER::Protected()");
	return true;
}


Cluster XZ(int x, int y) {
	if (y < MAP_TOP)
		y = MAP_TOP;

	if (y > MAP_TOP + MAP_HIG - MAP_ZGRID)
		y = MAP_TOP + MAP_HIG - MAP_ZGRID;

	return Cluster(x / MAP_XGRID, (y - MAP_TOP) / MAP_ZGRID);
}


Cluster XZ(Couple xy) {
	signed char x, y;
	xy.split(x, y);
	return XZ(x, y);
}


int pocref[POCKET_NX];
uint8   volume[2];

struct SavTab {
	void *Ptr;
	int Len;
	uint8 Flg;
};

SavTab _savTab[] = {
	{ &Now,           sizeof(Now),          1 },
	{ &OldLev,        sizeof(OldLev),       1 },
	{ &DemoText,      sizeof(DemoText),     1 },
	{ &Game,          sizeof(Game),         1 },
	{ &Game,          sizeof(Game),         1 },      // spare 1
	{ &Game,          sizeof(Game),         1 },      // spare 2
	{ &Game,          sizeof(Game),         1 },      // spare 3
	{ &Game,          sizeof(Game),         1 },      // spare 4
//	{ &VGA::Mono,     sizeof(VGA::Mono),    0 },
	{ &Music,         sizeof(Music),        1 },
	{ volume,         sizeof(volume),       1 },
	{ Flag,           sizeof(Flag),         1 },
	{ HeroXY,         sizeof(HeroXY),       1 },
	{ Barriers,       sizeof(Barriers),     1 },
	{ pocref,         sizeof(pocref),       1 },
	{ NULL,           0,                    0 }
};


void CGEEngine::loadGame(XFile &file, bool tiny = false) {
	SavTab *st;
	Sprite *spr;
	int i;

	for (st = _savTab; st->Ptr; st++) {
		if (file._error)
			error("Bad SVG");
		file.read((uint8 *)((tiny || st->Flg) ? st->Ptr : &i), st->Len);
	}

	file.read((uint8 *) &i, sizeof(i));
	if (i != SVGCHKSUM)
		error("%s", Text->getText(BADSVG_TEXT));

	if (STARTUP::Core < CORE_HIG)
		Music = false;

	if (STARTUP::SoundOk == 1 && STARTUP::Mode == 0) {
		SNDDrvInfo.VOL2.D = volume[0];
		SNDDrvInfo.VOL2.M = volume[1];
		SNDSetVolume();
	}

	if (! tiny) { // load sprites & pocket
		while (!file._error) {
			Sprite S(this, NULL);
			uint16 n = file.read((uint8 *) &S, sizeof(S));

			if (n != sizeof(S))
				break;

			S._prev = S._next = NULL;
			spr = (scumm_stricmp(S._file + 2, "MUCHA") == 0) ? new Fly(this, NULL)
			      : new Sprite(this, NULL);
			if (spr == NULL)
				error("No core");
			*spr = S;
			Vga->SpareQ->Append(spr);
		}

		for (i = 0; i < POCKET_NX; i++) {
			register int r = pocref[i];
			_pocket[i] = (r < 0) ? NULL : Vga->SpareQ->Locate(r);
		}
	}
}


static void SaveSound(void) {
	CFile cfg(UsrPath(progName(CFG_EXT)), WRI);
	if (!cfg._error)
		cfg.write(&SNDDrvInfo, sizeof(SNDDrvInfo) - sizeof(SNDDrvInfo.VOL2));
}


static void SaveGame(XFile &file) {
	SavTab *st;
	Sprite *spr;
	int i;

	for (i = 0; i < POCKET_NX; i++) {
		register Sprite *s = _pocket[i];
		pocref[i] = (s) ? s->_ref : -1;
	}

	volume[0] = SNDDrvInfo.VOL2.D;
	volume[1] = SNDDrvInfo.VOL2.M;

	for (st = _savTab; st->Ptr; st++) {
		if (file._error)
			error("Bad SVG");
		file.write((uint8 *) st->Ptr, st->Len);
	}

	file.write((uint8 *) & (i = SVGCHKSUM), sizeof(i));

	for (spr = Vga->SpareQ->First(); spr; spr = spr->_next)
		if (spr->_ref >= 1000)
			if (!file._error)
				file.write((uint8 *)spr, sizeof(*spr));
}


static void HeroCover(int cvr) {
	SNPOST(SNCOVER, 1, cvr, NULL);
}


static void Trouble(int seq, int txt) {
	Hero->park();
	SNPOST(SNWAIT, -1, -1, Hero);
	SNPOST(SNSEQ, -1, seq, Hero);
	SNPOST(SNSOUND, -1, 2, Hero);
	SNPOST(SNWAIT, -1, -1, Hero);
	SNPOST(SNSAY,  1, txt, Hero);
}


static void OffUse(void) {
	Trouble(OFF_USE, OFF_USE_TEXT + new_random(OffUseCount));
}


static void TooFar(void) {
	Trouble(TOO_FAR, TOO_FAR_TEXT);
}


// Used in stubbed function, do not remove!
static void noWay() {
	Trouble(NO_WAY, NO_WAY_TEXT);
}


static void LoadHeroXY(void) {
	INI_FILE cf(progName(".HXY"));
	memset(HeroXY, 0, sizeof(HeroXY));
	if (!cf._error)
		cf.CFREAD(&HeroXY);
}


static void LoadMapping(void) {
	if (Now <= CAVE_MAX) {
		INI_FILE cf(progName(".TAB"));
		if (!cf._error) {
			memset(Cluster::_map, 0, sizeof(Cluster::_map));
			cf.seek((Now - 1) * sizeof(Cluster::_map));
			cf.read((uint8 *) Cluster::_map, sizeof(Cluster::_map));
		}
	}
}


Cluster Trace[MAX_FIND_LEVEL];
int FindLevel;


WALK::WALK(CGEEngine *vm, BMP_PTR *shpl)
	: Sprite(vm, shpl), Dir(NO_DIR), _tracePtr(-1), _vm(vm) {
}


void WALK::tick() {
	if (_flags._hide)
		return;

	_here = XZ(_x + _w / 2, _y + _h);

	if (Dir != NO_DIR) {
		Sprite *spr;
		Sys->FunTouch();
		for (spr = Vga->ShowQ->First(); spr; spr = spr->_next) {
			if (distance(spr) < 2) {
				if (!spr->_flags._near) {
					FeedSnail(spr, NEAR);
					spr->_flags._near = true;
				}
			} else {
				spr->_flags._near = false;
			}
		}
	}

	if (_flags._hold || _tracePtr < 0)
		park();
	else {
		if (_here == Trace[_tracePtr]) {
			if (--_tracePtr < 0)
				park();
		} else {
			signed char dx, dz;
			(Trace[_tracePtr] - _here).split(dx, dz);
			DIR d = (dx) ? ((dx > 0) ? EE : WW) : ((dz > 0) ? SS : NN);
			turn(d);
		}
	}
	step();
	if ((Dir == WW && _x <= 0)           ||
	    (Dir == EE && _x + _w >= SCR_WID) ||
	    (Dir == SS && _y + _w >= WORLD_HIG - 2))
		park();
	else {
		signed char x;            // dummy var
		_here.split(x, _z);         // take current Z position
		SNPOST_(SNZTRIM, -1, 0, this);    // update Hero's pos in show queue
	}
}


int WALK::distance(Sprite *spr) {
	int dx, dz;
	dx = spr->_x - (_x + _w - WALKSIDE);
	if (dx < 0)
		dx = (_x + WALKSIDE) - (spr->_x + spr->_w);

	if (dx < 0)
		dx = 0;

	dx /= MAP_XGRID;
	dz = spr->_z - _z;
	if (dz < 0)
		dz = - dz;

	dx = dx * dx + dz * dz;
	for (dz = 1; dz * dz < dx; dz++)
		;

	return dz - 1;
}


void WALK::turn(DIR d) {
	DIR dir = (Dir == NO_DIR) ? SS : Dir;
	if (d != Dir) {
		step((d == dir) ? (1 + dir + dir) : (9 + 4 * dir + d));
		Dir = d;
	}
}


void WALK::park(void) {
	if (_time == 0)
		++_time;

	if (Dir != NO_DIR) {
		step(9 + 4 * Dir + Dir);
		Dir = NO_DIR;
		_tracePtr = -1;
	}
}


void WALK::findWay(Cluster c) {
	warning("STUB: WALK::findWay");
	/*
	bool Find1Way(void);
	extern uint16 Target;

	if (c != Here) {
		for (FindLevel = 1; FindLevel <= MAX_FIND_LEVEL; FindLevel++) {
			signed char x, z;
			Here.Split(x, z);
			Target = (z << 8) | x;
			c.Split(x, z);
			_CX = (z << 8) | x;
			if (Find1Way())
				break;
		}
		TracePtr = (FindLevel > MAX_FIND_LEVEL) ? -1 : (FindLevel - 1);
		if (TracePtr < 0)
			NoWay();
		Time = 1;
	}
*/
}


void WALK::findWay(Sprite *spr) {
	if (spr && spr != this) {
		int x = spr->_x;
		int z = spr->_z;
		if (spr->_flags._east)
			x += spr->_w + _w / 2 - WALKSIDE;
		else
			x -= _w / 2 - WALKSIDE;
		findWay(Cluster((x / MAP_XGRID),
		                ((z < MAP_ZCNT - MAX_DISTANCE) ? (z + 1)
		                 : (z - 1))));
	}
}


bool WALK::lower(Sprite *spr) {
	return (spr->_y > _y + (_h * 3) / 5);
}


void WALK::reach(Sprite *spr, int mode) {
	if (spr) {
		Hero->findWay(spr);
		if (mode < 0) {
			mode = spr->_flags._east;
			if (lower(spr))
				mode += 2;
		}
	}
	// note: insert SNAIL commands in reverse order
	SNINSERT(SNPAUSE, -1, 64, NULL);
	SNINSERT(SNSEQ, -1, TSEQ + mode, this);
	if (spr) {
		SNINSERT(SNWAIT,  -1, -1, Hero); /////--------$$$$$$$
		//SNINSERT(SNWALK, -1, -1, spr);
	}
	// sequence is not finished,
	// now it is just at sprite appear (disappear) point
}


class SQUARE : public Sprite {
public:
	SQUARE(CGEEngine *vm);
	void Touch(uint16 mask, int x, int y);
private:
	CGEEngine *_vm;
};


SQUARE::SQUARE(CGEEngine *vm)
	: Sprite(vm, MB), _vm(vm) {
	_flags._kill = true;
	_flags._bDel = false;
}


void SQUARE::Touch(uint16 mask, int x, int y) {
	Sprite::touch(mask, x, y);
	if (mask & L_UP) {
		XZ(_x + x, _y + y).cell() = 0;
		SNPOST_(SNKILL, -1, 0, this);
	}
}


void CGEEngine::setMapBrick(int x, int z) {
	SQUARE *s = new SQUARE(this);
	if (s) {
		static char n[] = "00:00";
		s->gotoxy(x * MAP_XGRID, MAP_TOP + z * MAP_ZGRID);
		wtom(x, n + 0, 10, 2);
		wtom(z, n + 3, 10, 2);
		Cluster::_map[z][x] = 1;
		s->setName(n);
		Vga->ShowQ->Insert(s, Vga->ShowQ->First());
	}
}

static void SwitchColorMode(void);
static void SwitchDebug(void);
static void SwitchMusic(void);
static void KillSprite(void);
static void PushSprite(void);
static void PullSprite(void);
static void NextStep(void);
static void SaveMapping(void);

static void KeyClick(void) {
	SNPOST_(SNSOUND, -1, 5, NULL);
}


void CGEEngine::resetQSwitch() {
	SNPOST_(SNSEQ, 123,  0, NULL);
	KeyClick();
}


void CGEEngine::quit() {
	static CHOICE QuitMenu[] = { 
		{ NULL, &CGEEngine::startCountDown },
		{ NULL, &CGEEngine::resetQSwitch   },
		{ NULL, &CGEEngine::dummy          }
	};

	if (Snail->Idle() && ! Hero->_flags._hide) {
		if (VMENU::Addr) {
			SNPOST_(SNKILL, -1, 0, VMENU::Addr);
			resetQSwitch();
		} else {
			QuitMenu[0].Text = Text->getText(QUIT_TEXT);
			QuitMenu[1].Text = Text->getText(NOQUIT_TEXT);
			(new VMENU(this, QuitMenu, -1, -1))->setName(Text->getText(QUIT_TITLE));
			SNPOST_(SNSEQ, 123, 1, NULL);
			KeyClick();
		}
	}
}


static void AltCtrlDel(void) {
		SNPOST_(SNSAY,  -1, A_C_D_TEXT, Hero);
}

// Used in stubbed function, do not remove!
static void miniStep(int stp) {
	if (stp < 0)
		_miniCave->_flags._hide = true;
	else {
		&*Mini;
		*MiniShp[0] = *MiniShpList[stp];
		if (Fx.Current)
			&*(Fx.Current->EAddr());

		_miniCave->_flags._hide = false;
	}
}


static void PostMiniStep(int stp) {
	//static int recent = -2;
	//TODO Change the SNPOST message send to a special way to send function pointer
	//if (MiniCave && stp != recent) SNPOST_(SNEXEC, -1, recent = stp, (void *)&MiniStep);
	warning("STUB: PostMiniStep()");
}

void SYSTEM::SetPal(void) {
	uint i;
	Dac *p = VGA::SysPal + 256 - ArrayCount(_stdPal);
	for (i = 0; i < ArrayCount(_stdPal); i++) {
		p[i]._r = _stdPal[i]._r >> 2;
		p[i]._g = _stdPal[i]._g >> 2;
		p[i]._b = _stdPal[i]._b >> 2;
	}
}


void SYSTEM::FunTouch(void) {
	uint16 n = (PAIN) ? HEROFUN1 : HEROFUN0;
	if (Talk == NULL || n > FunDel)
		FunDel = n;
}


static void ShowBak(int ref) {
	Sprite *spr = Vga->SpareQ->Locate(ref);
	if (spr) {
		Bitmap::_pal = VGA::SysPal;
		spr->expand();
		Bitmap::_pal = NULL;
		spr->show(2);
		Vga->CopyPage(1, 2);
		Sys->SetPal();
		spr->contract();
	}
}


static void caveUp() {
	int BakRef = 1000 * Now;
	if (Music)
		LoadMIDI(Now);

	ShowBak(BakRef);
	LoadMapping();
	Text->Preload(BakRef, BakRef + 1000);
	Sprite *spr = Vga->SpareQ->First();
	while (spr) {
		Sprite *n = spr->_next;
		if (spr->_cave == Now || spr->_cave == 0)
			if (spr->_ref != BakRef) {
				if (spr->_flags._back)
					spr->backShow();
				else
					ExpandSprite(spr);
			}
		spr = n;
	}
	if (SNDDrvInfo.DDEV) {
		Sound.Stop();
		Fx.Clear();
		Fx.Preload(0);
		Fx.Preload(BakRef);
	}

	if (Hero) {
		Hero->gotoxy(HeroXY[Now - 1]._x, HeroXY[Now - 1]._y);
		// following 2 lines trims Hero's Z position!
		Hero->tick();
		Hero->_time = 1;
		Hero->_flags._hide = false;
	}

	if (! Dark)
		Vga->Sunset();

	Vga->CopyPage(0, 1);
	SelectPocket(-1);
	if (Hero)
		Vga->ShowQ->Insert(Vga->ShowQ->Remove(Hero));

	if (_shadow) {
		Vga->ShowQ->Remove(_shadow);
		_shadow->makeXlat(glass(VGA::SysPal, 204, 204, 204));
		Vga->ShowQ->Insert(_shadow, Hero);
		_shadow->_z = Hero->_z;
	}
	FeedSnail(Vga->ShowQ->Locate(BakRef + 999), TAKE);
	Vga->Show();
	Vga->CopyPage(1, 0);
	Vga->Show();
	Vga->Sunrise(VGA::SysPal);
	Dark = false;
	if (! Startup)
		Mouse->On();

	_heart->_enable = true;
}


void CGEEngine::caveDown() {
	Sprite *spr;
	if (!_horzLine->_flags._hide)
		switchMapping();

	for (spr = Vga->ShowQ->First(); spr;) {
		Sprite *n = spr->_next;
		if (spr->_ref >= 1000 /*&& spr->_cave*/) {
			if (spr->_ref % 1000 == 999)
				FeedSnail(spr, TAKE);
			Vga->SpareQ->Append(Vga->ShowQ->Remove(spr));
		}
		spr = n;
	}
	Text->Clear(1000);
}


void CGEEngine::xCave() {
	caveDown();
	caveUp();
}


void CGEEngine::qGame() {
	caveDown();
	OldLev = Lev;
	SaveSound();
	CFile file = CFile(UsrPath(UsrFnam), WRI, RCrypt);
	SaveGame(file);
	Vga->Sunset();
	Finis = true;
}


void CGEEngine::switchCave(int cav) {
	if (cav != Now) {
		_heart->_enable = false;
		if (cav < 0) {
			SNPOST(SNLABEL, -1, 0, NULL);  // wait for repaint
			//TODO Change the SNPOST message send to a special way to send function pointer
			//SNPOST(SNEXEC,  -1, 0, (void *)&QGame); // switch cave
			warning("SwitchCave() - SNPOST");
		} else {
			Now = cav;
			Mouse->Off();
			if (Hero) {
				Hero->park();
				Hero->step(0);
				if (!_isDemo)
				///// protection: auto-destruction on! ----------------------
					Vga->SpareQ->Show = STARTUP::Summa * (cav <= CAVE_MAX);
				/////--------------------------------------------------------
			}
			_cavLight->gotoxy(CAVE_X + ((Now - 1) % CAVE_NX) * CAVE_DX + CAVE_SX,
			              CAVE_Y + ((Now - 1) / CAVE_NX) * CAVE_DY + CAVE_SY);
			KillText();
			if (! Startup)
				KeyClick();
			SNPOST(SNLABEL, -1, 0, NULL);  // wait for repaint
			//TODO Change the SNPOST message send to a special way to send function pointer
			//SNPOST(SNEXEC,   0, 0, (void *)&XCave); // switch cave
			warning("SwitchCave() - SNPOST");
		}
	}
}

SYSTEM::SYSTEM(CGEEngine *vm) : Sprite(vm, NULL), _vm(vm) {
	FunDel = HEROFUN0;
	SetPal();
	Tick();
}

void SYSTEM::Touch(uint16 mask, int x, int y) {
	static int pp = 0;

	FunTouch();

	if (mask & KEYB) {
		int pp0;
		KeyClick();
		KillText();
		if (Startup == 1) {
			SNPOST(SNCLEAR, -1, 0, NULL);
			return;
		}
		pp0 = pp;
		switch (x) {
		case Del:
			if (Keyboard::_key[ALT] && Keyboard::_key[CTRL])
				AltCtrlDel();
			else 
				KillSprite();
			break;
		case 'F':
			if (Keyboard::_key[ALT]) {
				Sprite *m = Vga->ShowQ->Locate(17001);
				if (m) {
					m->step(1);
					m->_time = 216; // 3s
				}
			}
			break;
		case PgUp:
			PushSprite();
			break;
		case PgDn:
			PullSprite();
			break;
		case '+':
			NextStep();
			break;
		case '`':
			if (Keyboard::_key[ALT])
				SaveMapping();
			else
				_vm->switchMapping();
			break;
		case F1:
			SwitchDebug();
			break;
		case F3:
			Hero->step(TSEQ + 4);
			break;
		case F4:
			Hero->step(TSEQ + 5);
			break;
		case F5:
			Hero->step(TSEQ + 0);
			break;
		case F6:
			Hero->step(TSEQ + 1);
			break;
		case F7:
			Hero->step(TSEQ + 2);
			break;
		case F8:
			Hero->step(TSEQ + 3);
			break;
		case F9:
			Sys->FunDel = 1;
			break;
		case 'X':
			if (Keyboard::_key[ALT])
				Finis = true;
			break;
		case '0':
		case '1':
		case '2':
		case '3':
		case '4':
			if (Keyboard::_key[ALT]) {
				SNPOST(SNLEVEL, -1, x - '0', NULL);
				break;
			}
		case '5':
		case '6':
		case '7':
		case '8':
		case '9':
			if (_sprite)
				_sprite->step(x - '0');
			break;
		case F10          :
			if (Snail->Idle() && ! Hero->_flags._hide)
				_vm->startCountDown();
			break;
		case 'J':
			if (pp == 0)
				++pp;
			break;
		case 'B':
			if (pp == 1)
				++pp;
			break;
		case 'W':
			if (pp == 2)
				JBW = !JBW;
			break;
		}
		if (pp == pp0)
			pp = 0;
	} else {
		if (Startup)
			return;
		int cav = 0;
		InfoLine->Update(NULL);
		if (y >= WORLD_HIG) {
			if (x < BUTTON_X) {                           // select cave?
				if (y >= CAVE_Y && y < CAVE_Y + CAVE_NY * CAVE_DY &&
				        x >= CAVE_X && x < CAVE_X + CAVE_NX * CAVE_DX && ! Game) {
					cav = ((y - CAVE_Y) / CAVE_DY) * CAVE_NX + (x - CAVE_X) / CAVE_DX + 1;
					if (cav > MaxCave)
						cav = 0;
				} else {
					cav = 0;
				}
			} else if (mask & L_UP) {
				if (y >= POCKET_Y && y < POCKET_Y + POCKET_NY * POCKET_DY &&
				        x >= POCKET_X && x < POCKET_X + POCKET_NX * POCKET_DX) {
					int n = ((y - POCKET_Y) / POCKET_DY) * POCKET_NX + (x - POCKET_X) / POCKET_DX;
					SelectPocket(n);
				}
			}
		}

		PostMiniStep(cav - 1);

		if (mask & L_UP) {
			if (cav && Snail->Idle() && Hero->_tracePtr < 0)
				_vm->switchCave(cav);

			if (!_horzLine->_flags._hide) {
				if (y >= MAP_TOP && y < MAP_TOP + MAP_HIG) {
					int8 x1, z1;
					XZ(x, y).split(x1, z1);
					Cluster::_map[z1][x1] = 1;
					_vm->setMapBrick(x1, z1);
				}
			} else
			{
				if (! Talk && Snail->Idle() && Hero
				        && y >= MAP_TOP && y < MAP_TOP + MAP_HIG && ! Game) {
					Hero->findWay(XZ(x, y));
				}
			}
		}
	}
}


void SYSTEM::Tick(void) {
	if (! Startup) if (-- FunDel == 0) {
			KillText();
			if (Snail->Idle()) {
				if (PAIN)
					HeroCover(9);
				else if (STARTUP::Core >= CORE_MID) {
					int n = new_random(100);
					if (n > 96)
						HeroCover(6 + (Hero->_x + Hero->_w / 2 < SCR_WID / 2));
					else {
						if (n > 90)
							HeroCover(5);
						else {
							if (n > 60)
								HeroCover(4);
							else
								HeroCover(3);
						}
					}
				}
			}
			FunTouch();
		}
	_time = SYSTIMERATE;
}


/*
static void SpkOpen(void) {
  asm   in  al,0x61
  asm   or  al,0x03
  asm   out 0x61,al
  asm   mov al,0x90
  asm   out 0x43,al
}


static void SpkClose(void) {
  asm   in  al,0x61
  asm   and al,0xFC
  asm   out 0x61,al
}

*/


static void SwitchColorMode(void) {
	SNPOST_(SNSEQ, 121, Vga->Mono = !Vga->Mono, NULL);
	KeyClick();
	Vga->SetColors(VGA::SysPal, 64);
}



static void SwitchMusic(void) {
	if (Keyboard::_key[ALT]) {
		if (VMENU::Addr)
			SNPOST_(SNKILL, -1, 0, VMENU::Addr);
		else {
			SNPOST_(SNSEQ, 122, (Music = false), NULL);
			//TODO Change the SNPOST message send to a special way to send function pointer
			// SNPOST(SNEXEC, -1, 0, (void *)&selectSound);
			warning("SwitchMusic() - SNPOST");
		}
	} else {
		if (STARTUP::Core < CORE_HIG)
			SNPOST(SNINF, -1, NOMUSIC_TEXT, NULL);
		else {
			SNPOST_(SNSEQ, 122, (Music = ! Music), NULL);
			KeyClick();
		}
	}
	if (Music)
		LoadMIDI(Now);
	else
		KillMIDI();
}


void CGEEngine::startCountDown() {
	//SNPOST(SNSEQ, 123, 0, NULL);
	switchCave(-1);
}


void CGEEngine::takeName() {
	if (GetText::_ptr)
		SNPOST_(SNKILL, -1, 0, GetText::_ptr);
	else {
		GetText *tn = new GetText(this, Text->getText(GETNAME_PROMPT), UsrFnam, 8, KeyClick);
		if (tn) {
			tn->setName(Text->getText(GETNAME_TITLE));
			tn->center();
			tn->gotoxy(tn->_x, tn->_y - 10);
			tn->_z = 126;
			Vga->ShowQ->Insert(tn);
		}
	}
}


void CGEEngine::switchMapping() {
	if (_horzLine->_flags._hide) {
		int i;
		for (i = 0; i < MAP_ZCNT; i++) {
			int j;
			for (j = 0; j < MAP_XCNT; j++) {
				if (Cluster::_map[i][j])
					setMapBrick(j, i);
			}
		}
	} else {
		Sprite *s;
		for (s = Vga->ShowQ->First(); s; s = s->_next)
			if (s->_w == MAP_XGRID && s->_h == MAP_ZGRID)
				SNPOST_(SNKILL, -1, 0, s);
	}
	_horzLine->_flags._hide = !_horzLine->_flags._hide;
}


static void KillSprite(void) {
	_sprite->_flags._kill = true;
	_sprite->_flags._bDel = true;
	SNPOST_(SNKILL, -1, 0, _sprite);
	_sprite = NULL;
}


static void PushSprite(void) {
	Sprite *spr = _sprite->_prev;
	if (spr) {
		Vga->ShowQ->Insert(Vga->ShowQ->Remove(_sprite), spr);
		while (_sprite->_z > _sprite->_next->_z)
			_sprite->_z--;
	} else
		SNPOST_(SNSOUND, -1, 2, NULL);
}


static void PullSprite(void) {
	bool ok = false;
	Sprite *spr = _sprite->_next;
	if (spr) {
		spr = spr->_next;
		if (spr)
			ok = (!spr->_flags._slav);
	}
	if (ok) {
		Vga->ShowQ->Insert(Vga->ShowQ->Remove(_sprite), spr);
		if (_sprite->_prev)
			while (_sprite->_z < _sprite->_prev->_z)
				_sprite->_z++;
	} else
		SNPOST_(SNSOUND, -1, 2, NULL);
}


static void NextStep(void) {
	SNPOST_(SNSTEP, 0, 0, _sprite);
}


static void SaveMapping() {
	{
		IoHand cf(progName(".TAB"), UPD);
		if (!cf._error) {
			cf.seek((Now - 1) * sizeof(Cluster::_map));
			cf.write((uint8 *) Cluster::_map, sizeof(Cluster::_map));
		}
	}
	{
		IoHand cf(progName(".HXY"), WRI);
			if (!cf._error) {
				HeroXY[Now - 1]._x = Hero->_x;
				HeroXY[Now - 1]._y = Hero->_y;
				cf.write((uint8 *) HeroXY, sizeof(HeroXY));
		}
	}
}

//              1111111111222222222233333333 334444444444555555555566666666667777777777
//    01234567890123456789012345678901234567 890123456789012345678901234567890123456789
static  char    DebugText[] = " N=00000 F=000000 X=000 Y=000 FPS=0000\0S=00:00 000:000:000 000:000 00  ";

#define NFRE    (DebugText +  3)
#define FFRE    (DebugText + 11)
#define ABSX    (DebugText + 20)
#define ABSY    (DebugText + 26)
#define FRPS    (DebugText + 34)
#define XSPR    (DebugText + 38)
#define SP_N    (DebugText + 41)
#define SP_S    (DebugText + 44)

#define SP_X    (DebugText + 47)
#define SP_Y    (DebugText + 51)
#define SP_Z    (DebugText + 55)
#define SP_W    (DebugText + 59)
#define SP_H    (DebugText + 63)
#define SP_F    (DebugText + 67)
#define SP__    (DebugText + 70)

static void SayDebug(void) {
	if (!DebugLine->_flags._hide) {
		static long t = -1L;
		long t1 = timer();

		if (t1 - t >= 18) {
			static uint32 old = 0L;
			uint32 now = Vga->FrmCnt;
			dwtom(now - old, FRPS, 10, 4);
			old = now;
			t = t1;
		}

		dwtom(Mouse->_x, ABSX, 10, 3);
		dwtom(Mouse->_y, ABSY, 10, 3);
//		dwtom(coreleft(), NFRE, 10, 5);
//		dwtom(farcoreleft(), FFRE, 10, 6);

		// sprite queue size
		uint16 n = 0;
		Sprite *spr;
		for (spr = Vga->ShowQ->First(); spr; spr = spr->_next) {
			++ n;
			if (spr == _sprite) {
				*XSPR = ' ';
				dwtom(n, SP_N, 10, 2);
				dwtom(_sprite->_x, SP_X, 10, 3);
				dwtom(_sprite->_y, SP_Y, 10, 3);
				dwtom(_sprite->_z, SP_Z, 10, 3);
				dwtom(_sprite->_w, SP_W, 10, 3);
				dwtom(_sprite->_h, SP_H, 10, 3);
				dwtom(*(uint16 *)(&_sprite->_flags), SP_F, 16, 2);
			}
		}
		dwtom(n, SP_S, 10, 2);
//		*SP__ = (heapcheck() < 0) ? '!' : ' ';
		DebugLine->Update(DebugText);
	}
}


static void SwitchDebug(void) {
	DebugLine->_flags._hide = ! DebugLine->_flags._hide;
}


void CGEEngine::optionTouch(int opt, uint16 mask) {
	switch (opt) {
	case 1 :
		if (mask & L_UP)
			SwitchColorMode();
		break;
	case 2 :
		if (mask & L_UP)
			SwitchMusic();
		else if (mask & R_UP)
			if (!Mixer::_appear) {
				Mixer::_appear = true;
				new Mixer(this, BUTTON_X, BUTTON_Y);
			}
		break;
	case 3 :
		if (mask & L_UP)
			quit();
		break;
	}
}


#pragma argsused
void Sprite::touch(uint16 mask, int x, int y) {
	Sys->FunTouch();
	if ((mask & ATTN) == 0) {
		InfoLine->Update(name());
		if (mask & (R_DN | L_DN))
			_sprite = this;
		if (_ref / 10 == 12) {
			_vm->optionTouch(_ref % 10, mask);
			return;
		}
		if (_flags._syst)
			return;       // cannot access system sprites
		if (Game) if (mask & L_UP) {
				mask &= ~L_UP;
				mask |= R_UP;
			}
		if ((mask & R_UP) && Snail->Idle()) {
			Sprite *ps = (_pocLight->_seqPtr) ? _pocket[PocPtr] : NULL;
			if (ps) {
				if (_flags._kept || Hero->distance(this) < MAX_DISTANCE) {
					if (works(ps)) {
						FeedSnail(ps, TAKE);
					} else
						OffUse();
					SelectPocket(-1);
				} else
					TooFar();
			} else {
				if (_flags._kept)
					mask |= L_UP;
				else {
					if (Hero->distance(this) < MAX_DISTANCE) {
						///
						if (_flags._port) {
							if (FindPocket(NULL) < 0)
								PocFul();
							else {
								SNPOST(SNREACH, -1, -1, this);
								SNPOST(SNKEEP, -1, -1, this);
								_flags._port = false;
							}
						} else {
							if (_takePtr != NO_PTR) {
								if (snList(TAKE)[_takePtr].Com == SNNEXT)
									OffUse();
								else
									FeedSnail(this, TAKE);
							} else
								OffUse();
						}
					}///
					else
						TooFar();
				}
			}
		}
		if ((mask & L_UP) && Snail->Idle()) {
			if (_flags._kept) {
				int n;
				for (n = 0; n < POCKET_NX; n++) {
					if (_pocket[n] == this) {
						SelectPocket(n);
						break;
					}
				}
			} else
				SNPOST(SNWALK, -1, -1, this); // Hero->FindWay(this);
		}
	}
}


void CGEEngine::loadSprite(const char *fname, int ref, int cav, int col = 0, int row = 0, int pos = 0) {
	static const char *Comd[] = { "Name", "Type", "Phase", "East",
	                              "Left", "Right", "Top", "Bottom",
	                              "Seq", "Near", "Take",
	                              "Portable", "Transparent",
	                              NULL
	                            };
	static const char *Type[] = { "DEAD", "AUTO", "WALK", "NEWTON", "LISSAJOUS",
	                              "FLY", NULL
	                            };
	char line[LINE_MAX];

	int shpcnt = 0;
	int type = 0; // DEAD
	bool east = false;
	bool port = false;
	bool tran = false;
	int i, lcnt = 0;
	uint16 len;

	mergeExt(line, fname, SPR_EXT);
	if (INI_FILE::exist(line)) {      // sprite description file exist
		INI_FILE sprf(line);
		if (sprf._error)
			error("Bad SPR [%s]", line);

		while ((len = sprf.read((uint8 *)line)) != 0) {
			++ lcnt;
			if (len && line[len - 1] == '\n')
				line[-- len] = '\0';
			if (len == 0 || *line == '.')
				continue;

			if ((i = takeEnum(Comd, strtok(line, " =\t"))) < 0)
				error("%s [%s]", NumStr("Bad line ######", lcnt), (const char *)fname);


			switch (i) {
			case  0 : // Name - will be taken in Expand routine
				break;
			case  1 : // Type
				if ((type = takeEnum(Type, strtok(NULL, " \t,;/"))) < 0)
					error("%s [%s]", NumStr("Bad line ######", lcnt), (const char *)fname);
				break;
			case  2 : // Phase
				++ shpcnt;
				break;
			case  3 : // East
				east = (atoi(strtok(NULL, " \t,;/")) != 0);
				break;
			case 11 : // Portable
				port = (atoi(strtok(NULL, " \t,;/")) != 0);
				break;
			case 12 : // Transparent
				tran = (atoi(strtok(NULL, " \t,;/")) != 0);
				break;
			}
		}
		if (! shpcnt)
			error("No shapes [%s]", fname);
	} else { // no sprite description: mono-shaped sprite with only .BMP file
		++shpcnt;
	}

	// make sprite of choosen type
	switch (type) {
	case 1 : { // AUTO
		_sprite = new Sprite(this, NULL);
		if (_sprite) {
			_sprite->gotoxy(col, row);
			//Sprite->Time = 1;//-----------$$$$$$$$$$$$$$$$
		}
		break;
	}
	case 2 : { // WALK
		WALK *w = new WALK(this, NULL);
		if (w && ref == 1) {
			w->gotoxy(col, row);
			if (Hero)
				error("2nd HERO [%s]", fname);
			Hero = w;
		}
		_sprite = w;
		break;
	}
	/*
	case 3 : // NEWTON
	NEWTON * n = new NEWTON(NULL);
	if (n)
	{
	   n->Ay = (bottom-n->H);
	   n->By = 90;
	   n->Cy = 3;
	   n->Bx = 99;
	   n->Cx = 3;
	   n->Goto(col, row);
	 }
	     _sprite = n;
	     break;
	     */
	case 4 : { // LISSAJOUS
		error("Bad type [%s]", fname);
		/*
		LISSAJOUS * l = new LISSAJOUS(NULL);
		if (l)
		{
		   l->Ax = SCR_WID/2;
		   l->Ay = SCR_HIG/2;
		   l->Bx = 7;
		   l->By = 13;
		   l->Cx = 300;
		   l->Cy = 500;
		   *(long *) &l->Dx = 0; // movex * cnt
		   l->Goto(col, row);
		 }
		     _sprite = l;
		     */
		break;
	}
	case 5 : { // FLY
		Fly *f = new Fly(this, NULL);
		_sprite = f;
		//////Sprite->Time = 1;//-----------$$$$$$$$$$$$$$
		break;
	}
	default: { // DEAD
		_sprite = new Sprite(this, NULL);
		if (_sprite)
			_sprite->gotoxy(col, row);
		break;
	}
	}
	if (_sprite) {
		_sprite->_ref = ref;
		_sprite->_cave = cav;
		_sprite->_z = pos;
		_sprite->_flags._east = east;
		_sprite->_flags._port = port;
		_sprite->_flags._tran = tran;
		_sprite->_flags._kill = true;
		_sprite->_flags._bDel = true;

		// Extract the filename, without the extension
		strcpy(_sprite->_file, fname);
		char *p = strchr(_sprite->_file, '.');
		if (p)
			*p = '\0';

		_sprite->_shpCnt = shpcnt;
		Vga->SpareQ->Append(_sprite);
	}
}


void CGEEngine::loadScript(const char *fname) {
	char line[LINE_MAX];
	char *SpN;
	int SpI, SpA, SpX, SpY, SpZ;
	bool BkG = false;
	INI_FILE scrf(fname);
	int lcnt = 0;
	bool ok = true;

	if (scrf._error)
		return;

	while (scrf.read((uint8 *)line) != 0) {
		char *p;

		lcnt++;
		if (*line == 0 || *line == '\n' || *line == '.')
			continue;

		ok = false;   // not OK if break
		// sprite ident number
		if ((p = strtok(line, " \t\n")) == NULL)
			break;
		SpI = atoi(p);
		// sprite file name
		if ((SpN = strtok(NULL, " ,;/\t\n")) == NULL)
			break;
		// sprite cave
		if ((p = strtok(NULL, " ,;/\t\n")) == NULL)
			break;
		SpA = atoi(p);
		// sprite column
		if ((p = strtok(NULL, " ,;/\t\n")) == NULL)
			break;
		SpX = atoi(p);
		// sprite row
		if ((p = strtok(NULL, " ,;/\t\n")) == NULL)
			break;
		SpY = atoi(p);
		// sprite Z pos
		if ((p = strtok(NULL, " ,;/\t\n")) == NULL)
			break;
		SpZ = atoi(p);
		// sprite life
		if ((p = strtok(NULL, " ,;/\t\n")) == NULL)
			break;
		BkG = atoi(p) == 0;

		ok = true;    // no break: OK

		_sprite = NULL;
		loadSprite(SpN, SpI, SpA, SpX, SpY, SpZ);
		if (_sprite && BkG)
			_sprite->_flags._back = true;
	}
	if (! ok)
		error("%s [%s]", NumStr("Bad INI line ######", lcnt), fname);
}


void CGEEngine::mainLoop() {
	SayDebug();

	if (_isDemo) {
//		static uint32 tc = 0;
		if (/* FIXME: TimerCount - tc >= ((182L * 6L) * 5L) && */ Talk == NULL && Snail->Idle()) {
			if (Text->getText(DemoText)) {
				SNPOST(SNSOUND,  -1, 4, NULL); // drumla
				SNPOST(SNINF,  -1, DemoText, NULL);
				SNPOST(SNLABEL, -1, -1, NULL);
				if (Text->getText(++ DemoText) == NULL)
					DemoText = DEMO_TEXT + 1;
			}
			//FIXME: tc = TimerCount;
		}
	}
	Vga->Show();
	Snail_->RunCom();
	Snail->RunCom();

	// Delay to slow things down
	g_system->delayMillis(10);
}


void CGEEngine::loadUser() {
	// set scene
	if (STARTUP::Mode == 0) { // user .SVG file found
		CFile cfile = CFile(UsrPath(UsrFnam), REA, RCrypt);
		loadGame(cfile);
	} else {
		if (STARTUP::Mode == 1) {
			SVG0FILE file = SVG0FILE(SVG0NAME);
			loadGame(file);
		} else {
			loadScript(progName(INI_EXT));
			Music = true;
			CFile file = CFile(SVG0NAME, WRI);
			SaveGame(file);
			error("Ok [%s]", SVG0NAME);
		}
	}
	loadScript(progName(IN0_EXT));
}


void CGEEngine::runGame() {
	Text->Clear();
	Text->Preload(100, 1000);
	LoadHeroXY();

	_cavLight->_flags._tran = true;
	Vga->ShowQ->Append(_cavLight);
	_cavLight->_flags._hide = true;

	static Seq pocSeq[] = { { 0, 0, 0, 0, 20 },
		{ 1, 2, 0, 0,  4 },
		{ 2, 3, 0, 0,  4 },
		{ 3, 4, 0, 0, 16 },
		{ 2, 5, 0, 0,  4 },
		{ 1, 6, 0, 0,  4 },
		{ 0, 1, 0, 0, 16 },
	};
	_pocLight->setSeq(pocSeq);
	_pocLight->_flags._tran = true;
	_pocLight->_time = 1;
	_pocLight->_z = 120;
	Vga->ShowQ->Append(_pocLight);
	SelectPocket(-1);

	// FIXME: Allow ScummVM to handle mouse display
//	Vga->ShowQ->Append(Mouse);

//    ___________
	loadUser();
//    ~~~~~~~~~~~

	if ((_sprite = Vga->SpareQ->Locate(121)) != NULL)
		SNPOST_(SNSEQ, -1, Vga->Mono, _sprite);
	if ((_sprite = Vga->SpareQ->Locate(122)) != NULL)
		_sprite->step(Music);
	SNPOST_(SNSEQ, -1, Music, _sprite);
	if (! Music)
		KillMIDI();

	if (Mini && INI_FILE::exist("MINI.SPR")) {
		uint8 *ptr = (uint8 *) &*Mini;
		if (ptr != NULL) {
			loadSprite("MINI", -1, 0, MINI_X, MINI_Y);
			ExpandSprite(_miniCave = _sprite);  // NULL is ok
			if (_miniCave) {
				_miniCave->_flags._hide = true;
				_miniCave->moveShapes(ptr);
				MiniShp[0] = new Bitmap(*_miniCave->shp());
				MiniShpList = _miniCave->setShapeList(MiniShp);
				PostMiniStep(-1);
			}
		}
	}

	if (Hero) {
		ExpandSprite(Hero);
		Hero->gotoxy(HeroXY[Now - 1]._x, HeroXY[Now - 1]._y);
		if (INI_FILE::exist("00SHADOW.SPR")) {
			loadSprite("00SHADOW", -1, 0, Hero->_x + 14, Hero->_y + 51);
			if ((_shadow = _sprite) != NULL) {
				_shadow->_ref = 2;
				_shadow->_flags._tran = true;
				Hero->_flags._shad = true;
				Vga->ShowQ->Insert(Vga->SpareQ->Remove(_shadow), Hero);
			}
		}
	}

	InfoLine->gotoxy(INFO_X, INFO_Y);
	InfoLine->_flags._tran = true;
	InfoLine->Update(NULL);
	Vga->ShowQ->Insert(InfoLine);

	DebugLine->_z = 126;
	Vga->ShowQ->Insert(DebugLine);

	_horzLine->_y = MAP_TOP - (MAP_TOP > 0);
	_horzLine->_z = 126;
	Vga->ShowQ->Insert(_horzLine);

	Mouse->Busy = Vga->SpareQ->Locate(BUSY_REF);
	if (Mouse->Busy)
		ExpandSprite(Mouse->Busy);

	Startup = 0;

	SNPOST(SNLEVEL, -1, OldLev, &_cavLight);
	_cavLight->gotoxy(CAVE_X + ((Now - 1) % CAVE_NX) * CAVE_DX + CAVE_SX,
	              CAVE_Y + ((Now - 1) / CAVE_NX) * CAVE_DY + CAVE_SY);
	caveUp();

	Keyboard::setClient(Sys);
	// main loop
	while (! Finis) {
		//TODO Change the SNPOST message send to a special way to send function pointer
		// if (FINIS) SNPOST(SNEXEC,  -1, 0, (void *)&QGame);
		warning("RunGame: problematic use of SNPOST");
		mainLoop();
	}

	Keyboard::setClient(NULL);
	_heart->_enable = false;
	SNPOST(SNCLEAR, -1, 0, NULL);
	SNPOST_(SNCLEAR, -1, 0, NULL);
	Mouse->Off();
	Vga->ShowQ->Clear();
	Vga->SpareQ->Clear();
	Hero = NULL;
	_shadow = NULL;
}


void CGEEngine::movie(const char *ext) {
	const char *fn = progName(ext);
	if (INI_FILE::exist(fn)) {
		loadScript(fn);
		ExpandSprite(Vga->SpareQ->Locate(999));
		FeedSnail(Vga->ShowQ->Locate(999), TAKE);

		// FIXME: Allow ScummVM to handle mouse display
		//Vga->ShowQ->Append(Mouse);

		_heart->_enable = true;
		Keyboard::setClient(Sys);
		while (!Snail->Idle())
			mainLoop();

		Keyboard::setClient(NULL);
		_heart->_enable = false;
		SNPOST(SNCLEAR, -1, 0, NULL);
		SNPOST_(SNCLEAR, -1, 0, NULL);
		Vga->ShowQ->Clear();
		Vga->SpareQ->Clear();
	}
}


bool CGEEngine::showTitle(const char *name) {
	Bitmap::_pal = VGA::SysPal;
	BMP_PTR LB[] =  { new Bitmap(name, true), NULL };
	Bitmap::_pal = NULL;
	bool usr_ok = false;

	Sprite D(this, LB);
	D._flags._kill = true;
	D._flags._bDel = true;
	D.center();
	D.show(2);

	if (STARTUP::Mode == 2) {
		inf(SVG0NAME);
		Talk->show(2);
	}

	Vga->Sunset();
	Vga->CopyPage(1, 2);
	Vga->CopyPage(0, 1);
	SelectPocket(-1);
	Vga->Sunrise(VGA::SysPal);

	if (STARTUP::Mode < 2 && !STARTUP::SoundOk) {
		Vga->CopyPage(1, 2);
		Vga->CopyPage(0, 1);
		Vga->ShowQ->Append(Mouse);
		_heart->_enable = true;
		Mouse->On();
		for (selectSound(); !Snail->Idle() || VMENU::Addr;)
			mainLoop();
		Mouse->Off();
		_heart->_enable = false;
		Vga->ShowQ->Clear();
		Vga->CopyPage(0, 2);
		STARTUP::SoundOk = 2;
		if (Music)
			LoadMIDI(0);
	}

	if (STARTUP::Mode < 2) {
		if (_isDemo) {
			strcpy(UsrFnam, progName(SVG_EXT));
			usr_ok = true;
		} else {
			//-----------------------------------------
#ifndef EVA
#ifdef CD
			STARTUP::Summa |= (0xC0 + (DriveCD(0) << 6)) & 0xFF;
#else
//			Boot * b = ReadBoot(getdisk());
			warning("ShowTitle: FIXME ReadBoot");
			Boot *b = readBoot(0);
			uint32 sn = (b->_xSign == 0x29) ? b->_serial : b->_lTotSecs;
			free(b);
			sn -= ((Ident *)Copr)->_disk;
			STARTUP::Summa |= Lo(sn) | Hi(sn);
#endif
			//-----------------------------------------
			movie("X00"); // paylist
			Vga->CopyPage(1, 2);
			Vga->CopyPage(0, 1);
			Vga->ShowQ->Append(Mouse);
			//Mouse.On();
			_heart->_enable = true;
			for (takeName(); GetText::_ptr;)
				mainLoop();
			_heart->_enable = false;
			if (Keyboard::last() == Enter && *UsrFnam)
				usr_ok = true;
			if (usr_ok)
				strcat(UsrFnam, SVG_EXT);
			//Mouse.Off();
			Vga->ShowQ->Clear();
			Vga->CopyPage(0, 2);
#endif
		}

		if (usr_ok && STARTUP::Mode == 0) {
			const char *n = UsrPath(UsrFnam);
			if (CFile::exist(n)) {
				CFile file = CFile(n, REA, RCrypt);
				loadGame(file, true); // only system vars
				Vga->SetColors(VGA::SysPal, 64);
				Vga->Update();
				if (FINIS) {
					++ STARTUP::Mode;
					FINIS = false;
				}
			} else
				++STARTUP::Mode;
		}
	}

	if (STARTUP::Mode < 2)
		movie("X01"); // wink

	Vga->CopyPage(0, 2);

	if (_isDemo)
		return true;
	else
		return (STARTUP::Mode == 2 || usr_ok);
}


/*
void StkDump (void) {
  CFILE f("!STACK.DMP", BFW);
  f.Write((uint8 *) (intStackPtr-STACK_SIZ/2), STACK_SIZ*2);
}
*/


void CGEEngine::cge_main(void) {
	uint16 intStack[STACK_SIZ / 2];
	intStackPtr = intStack;

	//Debug( memset((void *) (-K(2)), 0, K(1)); )
	//Debug( memset((void *) (-K(4)), 0, K(1)); )
	memset(Barriers, 0xFF, sizeof(Barriers));

	if (!Mouse->Exist)
		error("%s", Text->getText(NO_MOUSE_TEXT));

	if (!SVG0FILE::exist(SVG0NAME))
		STARTUP::Mode = 2;

	DebugLine->_flags._hide = true;
	_horzLine->_flags._hide = true;

	//srand((uint16) Timer());
	Sys = new SYSTEM(this);

	if (Music && STARTUP::SoundOk)
		LoadMIDI(0);
/** *****DEBUG*****
	if (STARTUP::Mode < 2)
		movie(LGO_EXT);
*/
	if (showTitle("WELCOME")) {
		if ((!_isDemo) && (STARTUP::Mode == 1))
			movie("X02"); // intro
		runGame();
		Startup = 2;
		if (FINIS)
			movie("X03");
	} else
		Vga->Sunset();
	error("%s", Text->getText(EXIT_OK_TEXT + FINIS));
}

} // End of namespace CGE
