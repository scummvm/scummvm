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

#include    "cge/general.h"
#include    "cge/boot.h"
#include    "cge/ident.h"
#include    "cge/sound.h"
#include    "cge/startup.h"
#include    "cge/config.h"
#include    "cge/vga13h.h"
#include    "cge/snail.h"
#include    "cge/text.h"
#include    "cge/game.h"
#include    "cge/mouse.h"
#include    "cge/keybd.h"
#include    "cge/cfile.h"
#include    "cge/vol.h"
#include    "cge/talk.h"
#include    "cge/vmenu.h"
#include    "cge/gettext.h"
#include    "cge/mixer.h"
#include    "cge/cge_main.h"
#include    <stdio.h>
#include    <stdlib.h>
#include    <string.h>
#include    <fcntl.h>

#include "common/str.h"

namespace CGE {

#define     STACK_SIZ   (K(2))
#define     SVGCHKSUM   (1956+Now+OldLev+Game+Music+DemoText)

#define   SVG0NAME    ("{{INIT}}" SVG_EXT)
#define   SVG0FILE    CFILE

extern  uint16  _stklen = (STACK_SIZ * 2);

VGA *Vga;
HEART *Heart;
WALK *Hero;
SYSTEM *Sys;
SPRITE *PocLight;
MOUSE *Mouse;
SPRITE *Pocket[POCKET_NX];
SPRITE *Sprite;
SPRITE *MiniCave;
SPRITE *Shadow;
SPRITE *HorzLine;
INFO_LINE *InfoLine;
SPRITE *CavLight;
INFO_LINE *DebugLine;

BMP_PTR MB[2];
BMP_PTR HL[2];
BMP_PTR MC[3];
BMP_PTR PR[2];
BMP_PTR SP[3];
BMP_PTR LI[5];

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
DAC *SysPal = farnew(DAC, PAL_CNT);

//-------------------------------------------------------------------------
int     PocPtr      =  0;

static  EMS      *Mini        = MiniEmm.Alloc((uint16)MINI_EMM_SIZE);
static  BMP_PTR  *MiniShpList = NULL;
static  BMP_PTR   MiniShp[]   = { NULL, NULL };
static  KEYBOARD  Keyboard;
static  bool      Finis       = false;
static  int       Startup     = 1;
int	OffUseCount;
uint16 *intStackPtr = false;


HXY     HeroXY[CAVE_MAX] = {{0, 0}};
BAR     Barriers[1 + CAVE_MAX] = { { 0xFF, 0xFF } };


extern  int FindPocket(SPRITE *);

extern  DAC StdPal[58];

void    FeedSnail(SPRITE *spr, SNLIST snq);         // defined in SNAIL
uint8   CLUSTER::Map[MAP_ZCNT][MAP_XCNT];


uint8  &CLUSTER::Cell(void) {
	return Map[B][A];
}


bool CLUSTER::Protected(void) {
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


CLUSTER XZ(int x, int y) {
	if (y < MAP_TOP)
		y = MAP_TOP;

	if (y > MAP_TOP + MAP_HIG - MAP_ZGRID)
		y = MAP_TOP + MAP_HIG - MAP_ZGRID;

	return CLUSTER(x / MAP_XGRID, (y - MAP_TOP) / MAP_ZGRID);
}


CLUSTER XZ(COUPLE xy) {
	signed char x, y;
	xy.Split(x, y);
	return XZ(x, y);
}


int pocref[POCKET_NX];
uint8   volume[2];

struct  SAVTAB {
	void *Ptr;
	int Len;
	uint8 Flg;
} SavTab[] = {
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


static void LoadGame(XFILE &file, bool tiny = false) {
	SAVTAB *st;
	SPRITE *spr;
	int i;

	for (st = SavTab; st->Ptr; st ++) {
		if (file.Error)
			error("Bad SVG");
		file.Read((uint8 *)((tiny || st->Flg) ? st->Ptr : &i), st->Len);
	}

	file.Read((uint8 *) &i, sizeof(i));
	if (i != SVGCHKSUM)
		error(Text->getText(BADSVG_TEXT));

	if (STARTUP::Core < CORE_HIG)
		Music = false;

	if (STARTUP::SoundOk == 1 && STARTUP::Mode == 0) {
		SNDDrvInfo.VOL2.D = volume[0];
		SNDDrvInfo.VOL2.M = volume[1];
		SNDSetVolume();
	}

	if (! tiny) { // load sprites & pocket
		while (! file.Error) {
			SPRITE S(NULL);
			uint16 n = file.Read((uint8 *) &S, sizeof(S));

			if (n != sizeof(S))
				break;

			S.Prev = S.Next = NULL;
			spr = (scumm_stricmp(S.File + 2, "MUCHA") == 0) ? new FLY(NULL)
			      : new SPRITE(NULL);
			if (spr == NULL)
				error("No core");
			*spr = S;
			Vga->SpareQ->Append(spr);
		}

		for (i = 0; i < POCKET_NX; i ++) {
			register int r = pocref[i];
			Pocket[i] = (r < 0) ? NULL : Vga->SpareQ->Locate(r);
		}
	}
}


static void SaveSound(void) {
	CFILE cfg(UsrPath(ProgName(CFG_EXT)), WRI);
	if (! cfg.Error) cfg.Write(&SNDDrvInfo, sizeof(SNDDrvInfo) - sizeof(SNDDrvInfo.VOL2));
}


static void SaveGame(XFILE &file) {
	SAVTAB *st;
	SPRITE *spr;
	int i;

	for (i = 0; i < POCKET_NX; i ++) {
		register SPRITE *s = Pocket[i];
		pocref[i] = (s) ? s->Ref : -1;
	}

	volume[0] = SNDDrvInfo.VOL2.D;
	volume[1] = SNDDrvInfo.VOL2.M;

	for (st = SavTab; st->Ptr; st ++) {
		if (file.Error)
			error("Bad SVG");
		file.Write((uint8 *) st->Ptr, st->Len);
	}

	file.Write((uint8 *) & (i = SVGCHKSUM), sizeof(i));

	for (spr = Vga->SpareQ->First(); spr; spr = spr->Next)
		if (spr->Ref >= 1000)
			if (!file.Error)
				file.Write((uint8 *)spr, sizeof(*spr));
}


static void HeroCover(int cvr) {
	SNPOST(SNCOVER, 1, cvr, NULL);
}


static void Trouble(int seq, int txt) {
	Hero->Park();
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


static void NoWay(void) {
	Trouble(NO_WAY, NO_WAY_TEXT);
}


static void LoadHeroXY(void) {
	INI_FILE cf(ProgName(".HXY"));
	memset(HeroXY, 0, sizeof(HeroXY));
	if (! cf.Error)
		cf.CFREAD(&HeroXY);
}


static void LoadMapping(void) {
	if (Now <= CAVE_MAX) {
		INI_FILE cf(ProgName(".TAB"));
		if (! cf.Error) {
			memset(CLUSTER::Map, 0, sizeof(CLUSTER::Map));
			cf.Seek((Now - 1) * sizeof(CLUSTER::Map));
			cf.Read((uint8 *) CLUSTER::Map, sizeof(CLUSTER::Map));
		}
	}
}


CLUSTER Trace[MAX_FIND_LEVEL];
int FindLevel;


WALK::WALK(BMP_PTR *shpl)
	: SPRITE(shpl), Dir(NO_DIR), TracePtr(-1) {
}


void WALK::Tick(void) {
	if (Flags.Hide)
		return;

	Here = XZ(X + W / 2, Y + H);

	if (Dir != NO_DIR) {
		SPRITE *spr;
		Sys->FunTouch();
		for (spr = Vga->ShowQ->First(); spr; spr = spr->Next) {
				if (Distance(spr) < 2) {
				if (! spr->Flags.Near) {
					FeedSnail(spr, NEAR);
					spr->Flags.Near = true;
				}
			} else spr->Flags.Near = false;
		}
	}

	if (Flags.Hold || TracePtr < 0)
		Park();
	else {
		if (Here == Trace[TracePtr]) {
			if (-- TracePtr < 0)
				Park();
		} else {
			signed char dx, dz;
			(Trace[TracePtr] - Here).Split(dx, dz);
			DIR d = (dx) ? ((dx > 0) ? EE : WW) : ((dz > 0) ? SS : NN);
			Turn(d);
		}
	}
	Step();
	if ((Dir == WW && X <= 0)           ||
	    (Dir == EE && X + W >= SCR_WID) ||
	    (Dir == SS && Y + W >= WORLD_HIG - 2))
		Park();
	else {
		signed char x;            // dummy var
		Here.Split(x, Z);         // take current Z position
		SNPOST_(SNZTRIM, -1, 0, this);    // update Hero's pos in show queue
	}
}


int WALK::Distance(SPRITE *spr) {
	int dx, dz;
	dx = spr->X - (X + W - WALKSIDE);
	if (dx < 0)
		dx = (X + WALKSIDE) - (spr->X + spr->W);

	if (dx < 0)
		dx = 0;

	dx /= MAP_XGRID;
	dz = spr->Z - Z;
	if (dz < 0)
		dz = - dz;

	dx = dx * dx + dz * dz;
	for (dz = 1; dz * dz < dx; dz ++)
		;

	return dz - 1;
}


void WALK::Turn(DIR d) {
	DIR dir = (Dir == NO_DIR) ? SS : Dir;
	if (d != Dir) {
		Step((d == dir) ? (1 + dir + dir) : (9 + 4 * dir + d));
		Dir = d;
	}
}


void WALK::Park(void) {
	if (Time == 0)
		++Time;

	if (Dir != NO_DIR) {
		Step(9 + 4 * Dir + Dir);
		Dir = NO_DIR;
		TracePtr = -1;
	}
}


void WALK::FindWay(CLUSTER c) {
	warning("STUB: Find1Way");
	/*
	bool Find1Way(void);
	extern uint16 Target;

	if (c != Here) {
		for (FindLevel = 1; FindLevel <= MAX_FIND_LEVEL; FindLevel ++) {
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


void WALK::FindWay(SPRITE *spr) {
	if (spr && spr != this) {
		int x = spr->X, z = spr->Z;
		if (spr->Flags.East)
			x += spr->W + W / 2 - WALKSIDE;
		else
			x -= W / 2 - WALKSIDE;
		FindWay(CLUSTER((x / MAP_XGRID),
		                ((z < MAP_ZCNT - MAX_DISTANCE) ? (z + 1)
		                 : (z - 1))));
	}
}


bool WALK::Lower(SPRITE *spr) {
	return (spr->Y > Y + (H * 3) / 5);
}


void WALK::Reach(SPRITE *spr, int mode) {
	if (spr) {
		Hero->FindWay(spr);
		if (mode < 0) {
			mode = spr->Flags.East;
			if (Lower(spr))
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


class SQUARE : public SPRITE {
public:
	SQUARE(void);
	void Touch(uint16 mask, int x, int y);
};


SQUARE::SQUARE(void)
	: SPRITE(MB) {
	Flags.Kill = true;
	Flags.BDel = false;
}


void SQUARE::Touch(uint16 mask, int x, int y) {
	SPRITE::Touch(mask, x, y);
	if (mask & L_UP) {
		XZ(X + x, Y + y).Cell() = 0;
		SNPOST_(SNKILL, -1, 0, this);
	}
}


static void SetMapBrick(int x, int z) {
	SQUARE *s = new SQUARE;
	if (s) {
		static char n[] = "00:00";
		s->Goto(x * MAP_XGRID, MAP_TOP + z * MAP_ZGRID);
		wtom(x, n + 0, 10, 2);
		wtom(z, n + 3, 10, 2);
		CLUSTER::Map[z][x] = 1;
		s->SetName(n);
		Vga->ShowQ->Insert(s, Vga->ShowQ->First());
	}
}

void   dummy(void) {}
static void SwitchMapping(void);
static void SwitchColorMode(void);
static void StartCountDown(void);
static void SwitchDebug(void);
static void SwitchMusic(void);
static void KillSprite(void);
static void PushSprite(void);
static void PullSprite(void);
static void BackPaint(void);
static void NextStep(void);
static void SaveMapping(void);

static void KeyClick(void) {
	SNPOST_(SNSOUND, -1, 5, NULL);
}


static void ResetQSwitch(void) {
	SNPOST_(SNSEQ, 123,  0, NULL);
	KeyClick();
}


static void Quit(void) {
	static CHOICE QuitMenu[] = { { NULL, StartCountDown },
		{ NULL, ResetQSwitch   },
		{ NULL, dummy          }
	};

	if (Snail.Idle() && ! Hero->Flags.Hide) {
		if (VMENU::Addr) {
			SNPOST_(SNKILL, -1, 0, VMENU::Addr);
			ResetQSwitch();
		} else {
			QuitMenu[0].Text = Text->getText(QUIT_TEXT);
			QuitMenu[1].Text = Text->getText(NOQUIT_TEXT);
			(new VMENU(QuitMenu, -1, -1))->SetName(Text->getText(QUIT_TITLE));
			SNPOST_(SNSEQ, 123, 1, NULL);
			KeyClick();
		}
	}
}


static void AltCtrlDel(void) {
		SNPOST_(SNSAY,  -1, A_C_D_TEXT, Hero);
}


static void MiniStep(int stp) {
	if (stp < 0)
		MiniCave->Flags.Hide = true;
	else {
		&*Mini;
		*MiniShp[0] = *MiniShpList[stp];
		if (Fx.Current)
			&*(Fx.Current->EAddr());

		MiniCave->Flags.Hide = false;
	}
}


static void PostMiniStep(int stp) {
	//static int recent = -2;
	//TODO Change the SNPOST message send to a special way to send function pointer
	//if (MiniCave && stp != recent) SNPOST_(SNEXEC, -1, recent = stp, (void *)&MiniStep);
	warning("STUB: PostMiniStep()");
}

void SYSTEM::SetPal(void) {
	int i;
	DAC *p = SysPal + 256 - ArrayCount(StdPal);
	for (i = 0; i < ArrayCount(StdPal); i ++) {
		p[i].R = StdPal[i].R >> 2;
		p[i].G = StdPal[i].G >> 2;
		p[i].B = StdPal[i].B >> 2;
	}
}


void SYSTEM::FunTouch(void) {
	uint16 n = (PAIN) ? HEROFUN1 : HEROFUN0;
	if (Talk == NULL || n > FunDel)
		FunDel = n;
}


static void ShowBak(int ref) {
	SPRITE *spr = Vga->SpareQ->Locate(ref);
	if (spr) {
		BITMAP::Pal = SysPal;
		spr->Expand();
		BITMAP::Pal = NULL;
		spr->Show(2);
		Vga->CopyPage(1, 2);
		Sys->SetPal();
		spr->Contract();
	}
}


static void CaveUp(void) {
	int BakRef = 1000 * Now;
	if (Music)
		LoadMIDI(Now);

	ShowBak(BakRef);
	LoadMapping();
	Text->Preload(BakRef, BakRef + 1000);
	SPRITE *spr = Vga->SpareQ->First();
	while (spr) {
		SPRITE *n = spr->Next;
		if (spr->Cave == Now || spr->Cave == 0)
			if (spr->Ref != BakRef) {
				if (spr->Flags.Back)
					spr->BackShow();
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
		Hero->Goto(HeroXY[Now - 1].X, HeroXY[Now - 1].Y);
		// following 2 lines trims Hero's Z position!
		Hero->Tick();
		Hero->Time = 1;
		Hero->Flags.Hide = false;
	}

	if (! Dark)
		Vga->Sunset();

	Vga->CopyPage(0, 1);
	SelectPocket(-1);
	if (Hero)
		Vga->ShowQ->Insert(Vga->ShowQ->Remove(Hero));

	if (Shadow) {
		Vga->ShowQ->Remove(Shadow);
		Shadow->MakeXlat(Glass(SysPal, 204, 204, 204));
		Vga->ShowQ->Insert(Shadow, Hero);
		Shadow->Z = Hero->Z;
	}
	FeedSnail(Vga->ShowQ->Locate(BakRef + 999), TAKE);
	Vga->Show();
	Vga->CopyPage(1, 0);
	Vga->Show();
	Vga->Sunrise(SysPal);
	Dark = false;
	if (! Startup)
		Mouse->On();

	Heart->Enable = true;
}


static void CaveDown(void) {
	SPRITE *spr;
	if (! HorzLine->Flags.Hide)
		SwitchMapping();

	for (spr = Vga->ShowQ->First(); spr;) {
		SPRITE *n = spr->Next;
		if (spr->Ref >= 1000 /*&& spr->Cave*/) {
			if (spr->Ref % 1000 == 999)
				FeedSnail(spr, TAKE);
			Vga->SpareQ->Append(Vga->ShowQ->Remove(spr));
		}
		spr = n;
	}
	Text->Clear(1000);
}


static void XCave(void) {
	CaveDown();
	CaveUp();
}


static void QGame(void) {
	CaveDown();
	OldLev = Lev;
	SaveSound();
	CFILE file = CFILE(UsrPath(UsrFnam), WRI, RCrypt);
	SaveGame(file);
	Vga->Sunset();
	Finis = true;
}


void SwitchCave(int cav) {
	if (cav != Now) {
		Heart->Enable = false;
		if (cav < 0) {
			SNPOST(SNLABEL, -1, 0, NULL);  // wait for repaint
			//TODO Change the SNPOST message send to a special way to send function pointer
			//SNPOST(SNEXEC,  -1, 0, (void *)&QGame); // switch cave
			warning("SwitchCave() - SNPOST");
		} else {
			Now = cav;
			Mouse->Off();
			if (Hero) {
				Hero->Park();
				Hero->Step(0);
#ifndef DEMO
				///// protection: auto-destruction on! ----------------------
				Vga->SpareQ->Show = STARTUP::Summa * (cav <= CAVE_MAX);
				/////--------------------------------------------------------
#endif
			}
			CavLight->Goto(CAVE_X + ((Now - 1) % CAVE_NX) * CAVE_DX + CAVE_SX,
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

SYSTEM::SYSTEM() : SPRITE(NULL) {
	FunDel = HEROFUN0;
	SetPal();
	Tick();
}

void SYSTEM::Touch(uint16 mask, int x, int y) {
	static int pp = 0;
	void SwitchCave(int cav);

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
			if (KEYBOARD::Key[ALT] && KEYBOARD::Key[CTRL])
				AltCtrlDel();
			else 
				KillSprite();
			break;
		case 'F':
			if (KEYBOARD::Key[ALT]) {
				SPRITE *m = Vga->ShowQ->Locate(17001);
				if (m) {
					m->Step(1);
					m->Time = 216; // 3s
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
			if (KEYBOARD::Key[ALT])
				SaveMapping();
			else
				SwitchMapping();
			break;
		case F1:
			SwitchDebug();
			break;
		case F3:
			Hero->Step(TSEQ + 4);
			break;
		case F4:
			Hero->Step(TSEQ + 5);
			break;
		case F5:
			Hero->Step(TSEQ + 0);
			break;
		case F6:
			Hero->Step(TSEQ + 1);
			break;
		case F7:
			Hero->Step(TSEQ + 2);
			break;
		case F8:
			Hero->Step(TSEQ + 3);
			break;
		case F9:
			Sys->FunDel = 1;
			break;
		case 'X':
			if (KEYBOARD::Key[ALT])
				Finis = true;
			break;
		case '0':
		case '1':
		case '2':
		case '3':
		case '4':
			if (KEYBOARD::Key[ALT]) {
				SNPOST(SNLEVEL, -1, x - '0', NULL);
				break;
			}
		case '5':
		case '6':
		case '7':
		case '8':
		case '9':
			if (Sprite)
				Sprite->Step(x - '0');
			break;
		case F10          :
			if (Snail.Idle() && ! Hero->Flags.Hide)
				StartCountDown();
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
			if (cav && Snail.Idle() && Hero->TracePtr < 0)
				SwitchCave(cav);

			if (!HorzLine->Flags.Hide) {
				if (y >= MAP_TOP && y < MAP_TOP + MAP_HIG) {
					int8 x1, z1;
					XZ(x, y).Split(x1, z1);
					CLUSTER::Map[z1][x1] = 1;
					SetMapBrick(x1, z1);
				}
			} else
			{
				if (! Talk && Snail.Idle() && Hero
				        && y >= MAP_TOP && y < MAP_TOP + MAP_HIG && ! Game) {
					Hero->FindWay(XZ(x, y));
				}
			}
		}
	}
}


void SYSTEM::Tick(void) {
	if (! Startup) if (-- FunDel == 0) {
			KillText();
			if (Snail.Idle()) {
				if (PAIN)
					HeroCover(9);
				else if (STARTUP::Core >= CORE_MID) {
					int n = new_random(100);
					if (n > 96)
						HeroCover(6 + (Hero->X + Hero->W / 2 < SCR_WID / 2));
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
	Time = SYSTIMERATE;
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
	Vga->SetColors(SysPal, 64);
}



static void SwitchMusic(void) {
	if (KEYBOARD::Key[ALT]) {
		if (VMENU::Addr)
			SNPOST_(SNKILL, -1, 0, VMENU::Addr);
		else {
			SNPOST_(SNSEQ, 122, (Music = false), NULL);
			//TODO Change the SNPOST message send to a special way to send function pointer
			// SNPOST(SNEXEC, -1, 0, (void *)&SelectSound);
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


static void StartCountDown(void) {
	//SNPOST(SNSEQ, 123, 0, NULL);
	SwitchCave(-1);
}


#ifndef DEMO
static void TakeName(void) {
	if (GET_TEXT::Ptr)
		SNPOST_(SNKILL, -1, 0, GET_TEXT::Ptr);
	else {
		GET_TEXT *tn = new GET_TEXT(Text->getText(GETNAME_PROMPT), UsrFnam, 8, KeyClick);
		if (tn) {
			tn->SetName(Text->getText(GETNAME_TITLE));
			tn->Center();
			tn->Goto(tn->X, tn->Y - 10);
			tn->Z = 126;
			Vga->ShowQ->Insert(tn);
		}
	}
}
#endif


static void SwitchMapping(void) {
	if (HorzLine->Flags.Hide) {
		int i;
		for (i = 0; i < MAP_ZCNT; i ++) {
			int j;
			for (j = 0; j < MAP_XCNT; j ++) {
				if (CLUSTER::Map[i][j])
					SetMapBrick(j, i);
			}
		}
	} else {
		SPRITE *s;
		for (s = Vga->ShowQ->First(); s; s = s->Next)
			if (s->W == MAP_XGRID && s->H == MAP_ZGRID)
				SNPOST_(SNKILL, -1, 0, s);
	}
	HorzLine->Flags.Hide = ! HorzLine->Flags.Hide;
}


static void KillSprite(void) {
	Sprite->Flags.Kill = true;
	Sprite->Flags.BDel = true;
	SNPOST_(SNKILL, -1, 0, Sprite);
	Sprite = NULL;
}


static void PushSprite(void) {
	SPRITE *spr = Sprite->Prev;
	if (spr) {
		Vga->ShowQ->Insert(Vga->ShowQ->Remove(Sprite), spr);
		while (Sprite->Z > Sprite->Next->Z)
			--Sprite->Z;
	} else
		SNPOST_(SNSOUND, -1, 2, NULL);
}


static void PullSprite(void) {
	bool ok = false;
	SPRITE *spr = Sprite->Next;
	if (spr) {
		spr = spr->Next;
		if (spr)
			ok = (!spr->Flags.Slav);
	}
	if (ok) {
		Vga->ShowQ->Insert(Vga->ShowQ->Remove(Sprite), spr);
		if (Sprite->Prev)
			while (Sprite->Z < Sprite->Prev->Z)
				++Sprite->Z;
	} else
		SNPOST_(SNSOUND, -1, 2, NULL);
}


static void NextStep(void) {
	SNPOST_(SNSTEP, 0, 0, Sprite);
}


static void SaveMapping(void) {
	{
		IOHAND cf(ProgName(".TAB"), UPD);
		if (!cf.Error) {
			cf.Seek((Now - 1) * sizeof(CLUSTER::Map));
			cf.Write((uint8 *) CLUSTER::Map, sizeof(CLUSTER::Map));
		}
	}
	{
		IOHAND cf(ProgName(".HXY"), WRI);
			if (!cf.Error) {
				HeroXY[Now - 1].X = Hero->X;
				HeroXY[Now - 1].Y = Hero->Y;
				cf.Write((uint8 *) HeroXY, sizeof(HeroXY));
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
	if (!DebugLine->Flags.Hide) {
		static long t = -1L;
		long t1 = Timer();

		if (t1 - t >= 18) {
			static uint32 old = 0L;
			uint32 now = Vga->FrmCnt;
			dwtom(now - old, FRPS, 10, 4);
			old = now;
			t = t1;
		}

		dwtom(Mouse->X, ABSX, 10, 3);
		dwtom(Mouse->Y, ABSY, 10, 3);
//		dwtom(coreleft(), NFRE, 10, 5);
//		dwtom(farcoreleft(), FFRE, 10, 6);

		// sprite queue size
		uint16 n = 0;
		SPRITE *spr;
		for (spr = Vga->ShowQ->First(); spr; spr = spr->Next) {
			++ n;
			if (spr == Sprite) {
				*XSPR = ' ';
				dwtom(n, SP_N, 10, 2);
				dwtom(Sprite->X, SP_X, 10, 3);
				dwtom(Sprite->Y, SP_Y, 10, 3);
				dwtom(Sprite->Z, SP_Z, 10, 3);
				dwtom(Sprite->W, SP_W, 10, 3);
				dwtom(Sprite->H, SP_H, 10, 3);
				dwtom(*(uint16 *)(&Sprite->Flags), SP_F, 16, 2);
			}
		}
		dwtom(n, SP_S, 10, 2);
//		*SP__ = (heapcheck() < 0) ? '!' : ' ';
		DebugLine->Update(DebugText);
	}
}


static void SwitchDebug(void) {
	DebugLine->Flags.Hide = ! DebugLine->Flags.Hide;
}


static void OptionTouch(int opt, uint16 mask) {
	switch (opt) {
	case 1 :
		if (mask & L_UP)
			SwitchColorMode();
		break;
	case 2 :
		if (mask & L_UP)
			SwitchMusic();
		else if (mask & R_UP)
			if (! MIXER::Appear) {
				MIXER::Appear = true;
				new MIXER(BUTTON_X, BUTTON_Y);
			}
		break;
	case 3 :
		if (mask & L_UP)
			Quit();
		break;
	}
}


#pragma argsused
void SPRITE::Touch(uint16 mask, int x, int y) {
	Sys->FunTouch();
	if ((mask & ATTN) == 0) {
		InfoLine->Update(Name());
		if (mask & (R_DN | L_DN))
			Sprite = this;
		if (Ref / 10 == 12) {
			OptionTouch(Ref % 10, mask);
			return;
		}
		if (Flags.Syst)
			return;       // cannot access system sprites
		if (Game) if (mask & L_UP) {
				mask &= ~L_UP;
				mask |= R_UP;
			}
		if ((mask & R_UP) && Snail.Idle()) {
			SPRITE *ps = (PocLight->SeqPtr) ? Pocket[PocPtr] : NULL;
			if (ps) {
				if (Flags.Kept || Hero->Distance(this) < MAX_DISTANCE) {
					if (Works(ps)) {
						FeedSnail(ps, TAKE);
					} else
						OffUse();
					SelectPocket(-1);
				} else
					TooFar();
			} else {
				if (Flags.Kept)
					mask |= L_UP;
				else {
					if (Hero->Distance(this) < MAX_DISTANCE) {
						///
						if (Flags.Port) {
							if (FindPocket(NULL) < 0)
								PocFul();
							else {
								SNPOST(SNREACH, -1, -1, this);
								SNPOST(SNKEEP, -1, -1, this);
								Flags.Port = false;
							}
						} else {
							if (TakePtr != NO_PTR) {
								if (SnList(TAKE)[TakePtr].Com == SNNEXT)
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
		if ((mask & L_UP) && Snail.Idle()) {
			if (Flags.Kept) {
				int n;
				for (n = 0; n < POCKET_NX; n ++) {
					if (Pocket[n] == this) {
						SelectPocket(n);
						break;
					}
				}
			} else
				SNPOST(SNWALK, -1, -1, this); // Hero->FindWay(this);
		}
	}
}


static void LoadSprite(const char *fname, int ref, int cav, int col = 0, int row = 0, int pos = 0) {
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

	MergeExt(line, fname, SPR_EXT);
	if (INI_FILE::Exist(line)) {      // sprite description file exist
		INI_FILE sprf(line);
		if (sprf.Error)
			error("Bad SPR [%s]", line);

		while ((len = sprf.Read((uint8 *)line)) != 0) {
			++ lcnt;
			if (len && line[len - 1] == '\n')
				line[-- len] = '\0';
			if (len == 0 || *line == '.')
				continue;

			if ((i = TakeEnum(Comd, strtok(line, " =\t"))) < 0)
				error("%s [%s]", NumStr("Bad line ######", lcnt), fname);


			switch (i) {
			case  0 : // Name - will be taken in Expand routine
				break;
			case  1 : // Type
				if ((type = TakeEnum(Type, strtok(NULL, " \t,;/"))) < 0)
					error("%s [%s]", NumStr("Bad line ######", lcnt), fname);
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
		Sprite = new SPRITE(NULL);
		if (Sprite) {
			Sprite->Goto(col, row);
			//Sprite->Time = 1;//-----------$$$$$$$$$$$$$$$$
		}
		break;
	}
	case 2 : { // WALK
		WALK *w = new WALK(NULL);
		if (w && ref == 1) {
			w->Goto(col, row);
			if (Hero)
				error("2nd HERO [%s]", fname);
			Hero = w;
		}
		Sprite = w;
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
	     Sprite = n;
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
		   * (long *) &l->Dx = 0; // movex * cnt
		   l->Goto(col, row);
		 }
		     Sprite = l;
		     */
		break;
	}
	case 5 : { // FLY
		FLY *f = new FLY(NULL);
		Sprite = f;
		//////Sprite->Time = 1;//-----------$$$$$$$$$$$$$$
		break;
	}
	default: { // DEAD
		Sprite = new SPRITE(NULL);
		if (Sprite)
			Sprite->Goto(col, row);
		break;
	}
	}
	if (Sprite) {
		Sprite->Ref = ref;
		Sprite->Cave = cav;
		Sprite->Z = pos;
		Sprite->Flags.East = east;
		Sprite->Flags.Port = port;
		Sprite->Flags.Tran = tran;
		Sprite->Flags.Kill = true;
		Sprite->Flags.BDel = true;
		//fnsplit(fname, NULL, NULL, Sprite->File, NULL);
		warning("LoadSprite: use of fnsplit");

		Sprite->ShpCnt = shpcnt;
		Vga->SpareQ->Append(Sprite);
	}
}


static void LoadScript(const char *fname) {
	char line[LINE_MAX];
	char *SpN;
	int SpI, SpA, SpX, SpY, SpZ;
	bool BkG = false;
	INI_FILE scrf(fname);
	int lcnt = 0;
	bool ok = true;

	if (scrf.Error)
		return;

	while (scrf.Read((uint8 *)line) != 0) {
		char *p;

		++lcnt;
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

		Sprite = NULL;
		LoadSprite(SpN, SpI, SpA, SpX, SpY, SpZ);
		if (Sprite && BkG)
			Sprite->Flags.Back = true;
	}
	if (! ok)
		error("%s [%s]", NumStr("Bad INI line ######", lcnt), fname);
}


static void MainLoop(void) {
	SayDebug();

#ifdef DEMO
	static uint32 tc = 0;
	if (/* FIXME: TimerCount - tc >= ((182L*6L) * 5L) && */ Talk == NULL && Snail.Idle()) {
		if (Text->getText(DemoText)) {
			SNPOST(SNSOUND,  -1, 4, NULL); // drumla
			SNPOST(SNINF,  -1, DemoText, NULL);
			SNPOST(SNLABEL, -1, -1, NULL);
			if (Text->getText(++ DemoText) == NULL)
				DemoText = DEMO_TEXT + 1;
		}
		//FIXME: tc = TimerCount;
	}
#endif

	Vga->Show();
	Snail_.RunCom();
	Snail.RunCom();
}


void LoadUser(void) {
	// set scene
	if (STARTUP::Mode == 0) { // user .SVG file found
		CFILE cfile = CFILE(UsrPath(UsrFnam), REA, RCrypt);
		LoadGame(cfile);
	} else {
		if (STARTUP::Mode == 1) {
			SVG0FILE file = SVG0FILE(SVG0NAME);
			LoadGame(file);
		} else {
			LoadScript(ProgName(INI_EXT));
			Music = true;
			CFILE file = CFILE(SVG0NAME, WRI);
			SaveGame(file);
			error("Ok [%s]", SVG0NAME);
		}
	}
	LoadScript(ProgName(IN0_EXT));
}


static void RunGame(void) {
	Text->Clear();
	Text->Preload(100, 1000);
	LoadHeroXY();

	CavLight->Flags.Tran = true;
	Vga->ShowQ->Append(CavLight);
	CavLight->Flags.Hide = true;

	static SEQ PocSeq[] = { { 0, 0, 0, 0, 20 },
		{ 1, 2, 0, 0,  4 },
		{ 2, 3, 0, 0,  4 },
		{ 3, 4, 0, 0, 16 },
		{ 2, 5, 0, 0,  4 },
		{ 1, 6, 0, 0,  4 },
		{ 0, 1, 0, 0, 16 },
	};
	PocLight->SetSeq(PocSeq);
	PocLight->Flags.Tran = true;
	PocLight->Time = 1;
	PocLight->Z = 120;
	Vga->ShowQ->Append(PocLight);
	SelectPocket(-1);

	Vga->ShowQ->Append(Mouse);

//    ___________
	LoadUser();
//    ~~~~~~~~~~~

	if ((Sprite = Vga->SpareQ->Locate(121)) != NULL)
		SNPOST_(SNSEQ, -1, Vga->Mono, Sprite);
	if ((Sprite = Vga->SpareQ->Locate(122)) != NULL)
		Sprite->Step(Music);
	SNPOST_(SNSEQ, -1, Music, Sprite);
	if (! Music)
		KillMIDI();

	if (Mini && INI_FILE::Exist("MINI.SPR")) {
		uint8 *ptr = (uint8 *) &*Mini;
		if (ptr != NULL) {
			LoadSprite("MINI", -1, 0, MINI_X, MINI_Y);
			ExpandSprite(MiniCave = Sprite);  // NULL is ok
			if (MiniCave) {
				MiniCave->Flags.Hide = true;
				MiniCave->MoveShapes(ptr);
				MiniShp[0] = new BITMAP(*MiniCave->Shp());
				MiniShpList = MiniCave->SetShapeList(MiniShp);
				PostMiniStep(-1);
			}
		}
	}

	if (Hero) {
		ExpandSprite(Hero);
		Hero->Goto(HeroXY[Now - 1].X, HeroXY[Now - 1].Y);
		if (INI_FILE::Exist("00SHADOW.SPR")) {
			LoadSprite("00SHADOW", -1, 0, Hero->X + 14, Hero->Y + 51);
			if ((Shadow = Sprite) != NULL) {
				Shadow->Ref = 2;
				Shadow->Flags.Tran = true;
				Hero->Flags.Shad = true;
				Vga->ShowQ->Insert(Vga->SpareQ->Remove(Shadow), Hero);
			}
		}
	}

	InfoLine->Goto(INFO_X, INFO_Y);
	InfoLine->Flags.Tran = true;
	InfoLine->Update(NULL);
	Vga->ShowQ->Insert(InfoLine);

	DebugLine->Z = 126;
	Vga->ShowQ->Insert(DebugLine);

	HorzLine->Y = MAP_TOP - (MAP_TOP > 0);
	HorzLine->Z = 126;
	Vga->ShowQ->Insert(HorzLine);

	Mouse->Busy = Vga->SpareQ->Locate(BUSY_REF);
	if (Mouse->Busy)
		ExpandSprite(Mouse->Busy);

	Startup = 0;

	SNPOST(SNLEVEL, -1, OldLev, &CavLight);
	CavLight->Goto(CAVE_X + ((Now - 1) % CAVE_NX) * CAVE_DX + CAVE_SX,
	              CAVE_Y + ((Now - 1) / CAVE_NX) * CAVE_DY + CAVE_SY);
	CaveUp();

	KEYBOARD::SetClient(Sys);
	// main loop
	while (! Finis) {
		//TODO Change the SNPOST message send to a special way to send function pointer
		// if (FINIS) SNPOST(SNEXEC,  -1, 0, (void *)&QGame);
		warning("RunGame: problematic use of SNPOST");
		MainLoop();
	}

	KEYBOARD::SetClient(NULL);
	Heart->Enable = false;
	SNPOST(SNCLEAR, -1, 0, NULL);
	SNPOST_(SNCLEAR, -1, 0, NULL);
	Mouse->Off();
	Vga->ShowQ->Clear();
	Vga->SpareQ->Clear();
	Hero = NULL;
	Shadow = NULL;
}


void Movie(const char *ext) {
	const char *fn = ProgName(ext);
	if (INI_FILE::Exist(fn)) {
		LoadScript(fn);
		ExpandSprite(Vga->SpareQ->Locate(999));
		FeedSnail(Vga->ShowQ->Locate(999), TAKE);
		Vga->ShowQ->Append(Mouse);
		Heart->Enable = true;
		KEYBOARD::SetClient(Sys);
		while (! Snail.Idle()) {
			MainLoop();
		}
		KEYBOARD::SetClient(NULL);
		Heart->Enable = false;
		SNPOST(SNCLEAR, -1, 0, NULL);
		SNPOST_(SNCLEAR, -1, 0, NULL);
		Vga->ShowQ->Clear();
		Vga->SpareQ->Clear();
	}
}


bool ShowTitle(const char *name) {
	BITMAP::Pal = SysPal;
	BMP_PTR LB[] =  { new BITMAP(name), NULL };
	BITMAP::Pal = NULL;
	bool usr_ok = false;

	SPRITE D(LB);
	D.Flags.Kill = true;
	D.Flags.BDel = true;
	D.Center();
	D.Show(2);

	if (STARTUP::Mode == 2) {
		Inf(SVG0NAME);
		Talk->Show(2);
	}

	Vga->Sunset();
	Vga->CopyPage(1, 2);
	Vga->CopyPage(0, 1);
	SelectPocket(-1);
	Vga->Sunrise(SysPal);

	if (STARTUP::Mode < 2 && ! STARTUP::SoundOk) {
		Vga->CopyPage(1, 2);
		Vga->CopyPage(0, 1);
		Vga->ShowQ->Append(Mouse);
		Heart->Enable = true;
		Mouse->On();
		for (SelectSound(); ! Snail.Idle() || VMENU::Addr;)
			MainLoop();
		Mouse->Off();
		Heart->Enable = false;
		Vga->ShowQ->Clear();
		Vga->CopyPage(0, 2);
		STARTUP::SoundOk = 2;
		if (Music)
			LoadMIDI(0);
	}

	if (STARTUP::Mode < 2) {
#ifdef    DEMO
		strcpy(UsrFnam, ProgName(SVG_EXT));
		usr_ok = true;
#else
		//-----------------------------------------
#ifndef EVA
#ifdef CD
		STARTUP::Summa |= (0xC0 + (DriveCD(0) << 6)) & 0xFF;
#else
//	  Boot * b = ReadBoot(getdisk());
		warning("ShowTitle: FIXME ReadBoot");
		Boot *b = ReadBoot(0);
		uint32 sn = (b->XSign == 0x29) ? b->Serial : b->lTotSecs;
		free(b);
		sn -= ((IDENT *)Copr)->disk;
		STARTUP::Summa |= Lo(sn) | Hi(sn);
#endif
#endif
		//-----------------------------------------
		Movie("X00"); // paylist
		Vga->CopyPage(1, 2);
		Vga->CopyPage(0, 1);
		Vga->ShowQ->Append(Mouse);
		//Mouse.On();
		Heart->Enable = true;
		for (TakeName(); GET_TEXT::Ptr;)
			MainLoop();
		Heart->Enable = false;
		if (KEYBOARD::Last() == Enter && *UsrFnam)
			usr_ok = true;
		if (usr_ok)
			strcat(UsrFnam, SVG_EXT);
		//Mouse.Off();
		Vga->ShowQ->Clear();
		Vga->CopyPage(0, 2);
#endif
		if (usr_ok && STARTUP::Mode == 0) {
			const char *n = UsrPath(UsrFnam);
			if (CFILE::Exist(n)) {
				CFILE file = CFILE(n, REA, RCrypt);
				LoadGame(file, true); // only system vars
				Vga->SetColors(SysPal, 64);
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
		Movie("X01"); // wink

	Vga->CopyPage(0, 2);

#ifdef DEMO
	return true;
#else
	return (STARTUP::Mode == 2 || usr_ok);
#endif
}


/*
void StkDump (void) {
  CFILE f("!STACK.DMP", BFW);
  f.Write((uint8 *) (intStackPtr-STACK_SIZ/2), STACK_SIZ*2);
}
*/


void cge_main(void) {
	uint16 intStack[STACK_SIZ / 2];
	intStackPtr = intStack;

	//Debug( memset((void *) (-K(2)), 0, K(1)); )
	//Debug( memset((void *) (-K(4)), 0, K(1)); )
	memset(Barriers, 0xFF, sizeof(Barriers));

	if (!Mouse->Exist)
		error("%s", Text->getText(NO_MOUSE_TEXT));
	if (!SVG0FILE::Exist(SVG0NAME))
		STARTUP::Mode = 2;

	DebugLine->Flags.Hide = true;
	HorzLine->Flags.Hide = true;

	//srand((uint16) Timer());
	Sys = new SYSTEM;

	if (Music && STARTUP::SoundOk)
		LoadMIDI(0);
	if (STARTUP::Mode < 2)
		Movie(LGO_EXT);
	if (ShowTitle("WELCOME")) {
#ifndef   DEMO
		if (STARTUP::Mode == 1)
			Movie("X02"); // intro
#endif
		RunGame();
		Startup = 2;
		if (FINIS)
			Movie("X03");
	} else
		Vga->Sunset();
	error("%s", Text->getText(EXIT_OK_TEXT + FINIS));
}

} // End of namespace CGE
