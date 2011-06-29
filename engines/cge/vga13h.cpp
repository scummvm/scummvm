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

#include "common/rect.h"
#include "graphics/palette.h"
#include "cge/general.h"
#include "cge/vga13h.h"
#include "cge/bitmap.h"
#include "cge/vol.h"
#include "cge/text.h"
#include "cge/cge_main.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <fcntl.h>
#include "cge/cge.h"

namespace CGE {

#define     FADE_STEP   2
#define     TMR_DIV     ((0x8000/TMR_RATE)*2)

//--------------------------------------------------------------------------

static char Report[] = "NearHeap=.....  FarHeap=......\n";
#define NREP         9
#define FREP        24

static  VgaRegBlk VideoMode[] = {
	{ 0x04, VGASEQ, 0x08, 0x04 },   // memory mode
	{ 0x03, VGAGRA, 0xFF, 0x00 },   // data rotate = 0
	{ 0x05, VGAGRA, 0x03, 0x00 },   // R/W mode = 0
	{ 0x06, VGAGRA, 0x02, 0x00 },   // misc
	{ 0x14, VGACRT, 0x40, 0x00 },   // underline
	{ 0x13, VGACRT, 0xFF, 0x28 },   // screen width
	{ 0x17, VGACRT, 0xFF, 0xC3 },   // mode control
	{ 0x11, VGACRT, 0x80, 0x00 },   // vert retrace end
	{ 0x09, VGACRT, 0xEF, 0x01 },   // max scan line
	{ 0x30, VGAATR, 0x00, 0x20 },   // 256 color mode
//		    { 0x12, VGACRT, 0xFF, 0x6E },   // vert display end
//		    { 0x15, VGACRT, 0xFF, 0x7F },   // start vb
//		    { 0x10, VGACRT, 0xFF, 0x94 },   // start vr
	{ 0x00                     }
};


bool SpeedTest   = false;
Seq _seq1[] = { { 0, 0, 0, 0, 0 } };
Seq _seq2[] = { { 0, 1, 0, 0, 0 }, { 1, 0, 0, 0, 0 } };

extern "C"  void    SNDMIDIPlay(void);

char *NumStr(char *str, int num) {
	char *p = strchr(str, '#');
	if (p)
		wtom(num, p, 10, 5);
	return str;
}


static void Video() {
/*
  static uint16 SP_S;

  asm   push    bx
  asm   push    bp
  asm   push    si
  asm   push    di
  asm   push    es
  asm   xor bx,bx       // video page #0
  SP_S = _SP;
  asm   int VIDEO
  _SP = SP_S;
  asm   pop es
  asm   pop di
  asm   pop si
  asm   pop bp
  asm   pop bx
*/
	warning("STUB: Video");
}


uint16 *SaveScreen(void) {
	/*
	  uint16 cxy, cur, siz, * scr = NULL, * sav;

	  // horizontal size of text mode screen
	  asm   mov ah,0x0F     // get current video mode
	  Video();          // BIOS video service
	  asm   xchg    ah,al       // answer in ah
	  asm   push    ax      // preserve width

	  // vertical size of text mode screen
	  asm   mov dl,24       // last row on std screen
	  asm   xor bx,bx       // valid request in BH
	  asm   mov ax,0x1130   // get EGA's last row #
	  Video();          // BIOS video service
	  asm   inc dl      // # of rows = last+1

	  // compute screen size in words
	  asm   pop ax      // restore width
	  asm   mul dl      // width * height

	  siz = _AX;

	  asm   mov ax,0x40     // system data segment
	  asm   mov es,ax
	  asm   mov ax,0B000H   // Mono
	  asm   cmp byte ptr es:[0x49],0x07
	  asm   je  sto
	  asm   mov ax,0B800H   // Color
	  sto:              // store screen address
	  asm   mov word ptr scr+2,ax

	  _AH = 0x0F; Video();      // active page

	  // take cursor shape
	  _AH = 0x03; Video();      // get cursor size
	  cur = _CX;

	  // take cursor position
	  _DH = 0;
	  _AH = 0x03; Video();      // get cursor
	  cxy = _DX;

	  sav = farnew(uint16, siz+3);  // +3 extra uint16s for size and cursor
	  if (sav)
	    {
	      sav[0] = siz;
	      sav[1] = cur;
	      sav[2] = cxy;
	      memcpy(sav+3, scr, siz * 2);
	    }
	  return sav;
	  */
	warning("STUB: SaveScreen");
	return 0;
}


void RestoreScreen(uint16 * &sav) {
	/*
	  uint16 * scr = NULL;

	  asm   mov ax,0x40     // system data segment
	  asm   mov es,ax
	  asm   mov ax,0B000H   // Mono
	  asm   cmp byte ptr es:[0x49],0x07
	  asm   je  sto
	  asm   mov ax,0B800H   // Color
	  sto:              // store screen address
	  asm   mov word ptr scr+2,ax

	  memcpy(scr, sav+3, sav[0] * 2);

	  _AH = 0x0F; Video();      // active page

	  // set cursor shape
	  _CX = sav[1];
	  _AH = 0x01; Video();      // set cursor size

	  // set cursor position
	  _DX = sav[2];
	  _AH = 0x02; Video();      // set cursor

	  free(sav);
	  sav = NULL;
	  */
	warning("STUB: RestoreScreen");
}


DAC MkDAC(uint8 r, uint8 g, uint8 b) {
	static DAC x;
	x.R = r;
	x.G = g;
	x.B = b;
	return x;
}


Rgb MkRGB(uint8 r, uint8 g, uint8 b) {
	static TRGB x;
	x.dac.R = r;
	x.dac.G = g;
	x.dac.B = b;
	return x.rgb;
}


Sprite *Locate(int ref) {
	Sprite *spr = Vga->ShowQ->Locate(ref);
	return (spr) ? spr : Vga->SpareQ->Locate(ref);
}


Heart::Heart(void)
	: ENGINE(TMR_DIV) {
	_enable = false;
	_xTimer = NULL;
}


/*
extern "C" void TimerProc() {
	static SPRITE * spr;
	static uint8 run = 0;

	// decrement external timer uint16
	if (_heart->_xTimer) {
		if (*_heart->_xTimer)
			*_heart->_xTimer--;
		else 
			_heart->_xTimer = NULL;
	}

	if (!run && _heart->_enable) {       // check overrun flag
		static uint16 oldSP, oldSS;
		run++;           // disable 2nd call until current lasts
		asm   mov ax,ds
		asm   mov oldSS,ss
		asm   mov oldSP,sp
		asm   mov ss,ax
		asm   mov sp,0xFF80

		// system pseudo-sprite
		if (Sys) {
			if (Sys->Time) {
				if (--Sys->Time == 0)
					Sys->Tick();
			}
		}
		for (spr = VGA::ShowQ.First(); spr; spr = spr->Next) {
			if (spr->Time) {
				if (!spr->Flags.Hide) {
					if (-- spr->Time == 0)
						spr->Tick();
				}
			}
		}
		asm   mov ss,oldSS
		asm   mov sp,oldSP
		run--;
	}
}
*/


void ENGINE::NewTimer(...) {
	/*
	static SPRITE *spr;
	static uint8 run = 0, cntr1 = TMR_RATE1, cntr2 = TMR_RATE2;
	___1152_Hz___:

	SNDMIDIPlay();
	asm   dec cntr1
	asm   jz  ___72_Hz___
	asm   mov al,0x20     // send...
	asm   out 0x020,al    // ...e-o-i
	return;

	___72_Hz___:

	asm   mov cntr1,TMR_RATE1
	asm   dec cntr2
	asm   jnz my_eoi

	___18_Hz___:

	OldTimer();
	asm   mov cntr2,TMR_RATE2
	asm   jmp short my_int

	// send E-O-I
	my_eoi:
	asm   mov al,0x20
	asm   out 0x020,al
	asm   sti         //  enable interrupts

	my_int: //------72Hz-------//

	// decrement external timer uint16
	if (_heart->XTimer) {
	  if (*_heart->XTimer)
	     *_heart->XTimer--;
	  else
	     _heart->XTimer = NULL;
	}

	if (! run && _heart->Enable) {  // check overrun flag
	    static uint16 oldSP, oldSS;

	    run++;           // disable 2nd call until current lasts
	    asm   mov ax,ds
	    asm   mov oldSS,ss
	    asm   mov oldSP,sp
	    asm   mov ss,ax
	    asm   mov sp,0xFF80

	    // system pseudo-sprite
	    if (Sys) {
			if (Sys->Time) {
				if (--Sys->Time == 0)
					Sys->Tick();
			}
		}

	    for (spr = VGA::ShowQ.First(); spr; spr = spr->Next) {
			if (spr->Time) { 
				if (!spr->Flags.Hide) {
					if (--spr->Time == 0) 
						spr->Tick();
				}
			}
		}
	    asm   mov ss,oldSS
	    asm   mov sp,oldSP
	    run--;
	  }

	*/
	warning("STUB: ENGINE::NewTimer");
}


void Heart::setXTimer(uint16 *ptr) {
	if (_xTimer && ptr != _xTimer)
		*_xTimer = 0;
	_xTimer = ptr;
}


void Heart::setXTimer(uint16 *ptr, uint16 time) {
	setXTimer(ptr);
	*ptr = time;
}


Sprite::Sprite(CGEEngine *vm, BMP_PTR *shp)
	: X(0), Y(0), Z(0), NearPtr(0), TakePtr(0),
	  Next(NULL), Prev(NULL), SeqPtr(NO_SEQ), Time(0), //Delay(0),
	  _ext(NULL), _ref(-1), _cave(0), _vm(vm) {
	memset(File, 0, sizeof(File));
	*((uint16 *)&Flags) = 0;
	SetShapeList(shp);
}


Sprite::~Sprite() {
	Contract();
}


BMP_PTR Sprite::Shp() {
	register SprExt *e = _ext;
	if (e)
		if (e->_seq) {
			int i = e->_seq[SeqPtr].Now;
			if (i >= ShpCnt) {
				//char s[256];
				//sprintf(s, "Seq=%p ShpCnt=%d SeqPtr=%d Now=%d Next=%d",
				//      Seq, ShpCnt, SeqPtr, Seq[SeqPtr].Now, Seq[SeqPtr].Next);
				//VGA::Exit(s, File);
				error("Invalid PHASE in SPRITE::Shp() %s", File);
			}
			return e->_shpList[i];
		}
	return NULL;
}


BMP_PTR *Sprite::SetShapeList(BMP_PTR *shp) {
	BMP_PTR *r = (_ext) ? _ext->_shpList : NULL;

	ShpCnt = 0;
	W = 0;
	H = 0;

	if (shp) {
		BMP_PTR *p;
		for (p = shp; *p; p++) {
			BMP_PTR b = (*p); // ->Code();
			if (b->W > W)
				W = b->W;
			if (b->H > H)
				H = b->H;
			++ShpCnt;
		}
		Expand();
		_ext->_shpList = shp;
		if (!_ext->_seq)
			SetSeq((ShpCnt < 2) ? _seq1 : _seq2);
	}
	return r;
}


void Sprite::MoveShapes(uint8 *buf) {
	BMP_PTR *p;
	for (p = _ext->_shpList; *p; p++) {
		buf += (*p)->MoveVmap(buf);
	}
}


bool Sprite::Works(Sprite *spr) {
	if (spr)
		if (spr->_ext) {
			SNAIL::COM *c = spr->_ext->_take;
			if (c != NULL) {
				c += spr->TakePtr;
				if (c->Ref == _ref)
					if (c->Com != SNLABEL || (c->Val == 0 || c->Val == Now))
						return true;
			}
		}
	return false;
}


Seq *Sprite::SetSeq(Seq *seq) {
	Expand();
	register Seq *s = _ext->_seq;
	_ext->_seq = seq;
	if (SeqPtr == NO_SEQ)
		Step(0);
	else if (Time == 0)
		Step(SeqPtr);
	return s;
}


bool Sprite::SeqTest(int n) {
	if (n >= 0)
		return (SeqPtr == n);
	if (_ext)
		return (_ext->_seq[SeqPtr].Next == SeqPtr);
	return true;
}


SNAIL::COM *Sprite::SnList(SNLIST type) {
	register SprExt *e = _ext;
	if (e)
		return (type == NEAR) ? e->_near : e->_take;
	return NULL;
}


void Sprite::SetName(char *n) {
	if (_ext) {
		if (_ext->_name) {
			delete[] _ext->_name;
			_ext->_name = NULL;
		}
		if (n) {
			if ((_ext->_name = new char[strlen(n) + 1]) != NULL)
				strcpy(_ext->_name, n);
			else
				error("No core [%s]", n);
		}
	}
}


Sprite *Sprite::Expand(void) {
	if (!_ext) {
		bool enbl = _heart->_enable;
		_heart->_enable = false;
		if ((_ext = new SprExt) == NULL)
			error("No core");
		if (*File) {
			static const char *Comd[] = { "Name", "Phase", "Seq", "Near", "Take", NULL };
			char line[LINE_MAX], fname[MAXPATH];
			BMP_PTR *shplist = new BMP_PTR [ShpCnt + 1];
			Seq *seq = NULL;
			int shpcnt = 0,
			    seqcnt = 0,
			    neacnt = 0,
			    takcnt = 0,
			    maxnow = 0,
			    maxnxt = 0;

			SNAIL::COM *nea = NULL;
			SNAIL::COM *tak = NULL;
			MergeExt(fname, File, SPR_EXT);
			if (INI_FILE::Exist(fname)) { // sprite description file exist
				INI_FILE sprf(fname);
				if (! (sprf.Error==0))
					error("Bad SPR [%s]", fname);
				int len = 0, lcnt = 0;
				while ((len = sprf.Read((uint8 *)line)) != 0) {
					++lcnt;
					if (len && line[len - 1] == '\n')
						line[-- len] = '\0';
					if (len == 0 || *line == '.')
						continue;

					switch (TakeEnum(Comd, strtok(line, " =\t"))) {
					case 0 : { // Name
						SetName(strtok(NULL, ""));
						break;
					}
					case 1 : { // Phase
						shplist[shpcnt++] = new BITMAP(strtok(NULL, " \t,;/"));
						break;
					}
					case 2 : { // Seq
						seq = (Seq *) realloc(seq, (seqcnt + 1) * sizeof(*seq));
						if (seq == NULL)
							error("No core [%s]", fname);
						Seq *s = &seq[seqcnt++];
						s->Now  = atoi(strtok(NULL, " \t,;/"));
						if (s->Now > maxnow)
							maxnow = s->Now;
						s->Next = atoi(strtok(NULL, " \t,;/"));
						switch (s->Next) {
						case 0xFF :
							s->Next = seqcnt;
							break;
						case 0xFE :
							s->Next = seqcnt - 1;
							break;
						}
						if (s->Next > maxnxt)
							maxnxt = s->Next;
						s->Dx   = atoi(strtok(NULL, " \t,;/"));
						s->Dy   = atoi(strtok(NULL, " \t,;/"));
						s->Dly  = atoi(strtok(NULL, " \t,;/"));
						break;
					}
					case 3 : { // Near
						if (NearPtr != NO_PTR) {
							nea = (SNAIL::COM *) realloc(nea, (neacnt + 1) * sizeof(*nea));
							if (nea == NULL)
								error("No core [%s]", fname);
							else {
								SNAIL::COM *c = &nea[neacnt++];
								if ((c->Com = (SNCOM) TakeEnum(SNAIL::ComTxt, strtok(NULL, " \t,;/"))) < 0)
									error("%s [%s]", NumStr("Bad NEAR in ######", lcnt), fname);
								c->Ref = atoi(strtok(NULL, " \t,;/"));
								c->Val = atoi(strtok(NULL, " \t,;/"));
								c->Ptr = NULL;
							}
						}
					}
					break;
					case 4 : { // Take
						if (TakePtr != NO_PTR) {
							tak = (SNAIL::COM *) realloc(tak, (takcnt + 1) * sizeof(*tak));
							if (tak == NULL)
								error("No core [%s]", fname);
							else {
								SNAIL::COM *c = &tak[takcnt++];
								if ((c->Com = (SNCOM) TakeEnum(SNAIL::ComTxt, strtok(NULL, " \t,;/"))) < 0)
									error("%s [%s]", NumStr("Bad NEAR in ######", lcnt), fname);
								c->Ref = atoi(strtok(NULL, " \t,;/"));
								c->Val = atoi(strtok(NULL, " \t,;/"));
								c->Ptr = NULL;
							}
						}
						break;
					}
					}
				}
			} else { // no sprite description: try to read immediately from .BMP
				shplist[shpcnt++] = new BITMAP(File);
			}
			shplist[shpcnt] = NULL;
			if (seq) {
				if (maxnow >= shpcnt)
					error("Bad PHASE in SEQ [%s]", fname);
				if (maxnxt >= seqcnt)
					error("Bad JUMP in SEQ [%s]", fname);
				SetSeq(seq);
			} else
				SetSeq((ShpCnt == 1) ? _seq1 : _seq2);
			//disable();  // disable interupt

			SetShapeList(shplist);
			//enable();  // enable interupt
			if (nea)
				nea[neacnt - 1].Ptr = _ext->_near = nea;
			else
				NearPtr = NO_PTR;
			if (tak)
				tak[takcnt - 1].Ptr = _ext->_take = tak;
			else
				TakePtr = NO_PTR;
		}
		_heart->_enable = enbl;
	}
	return this;
}


Sprite *Sprite::Contract(void) {
	register SprExt *e = _ext;
	if (e) {
		if (e->_name)
			delete[] e->_name;
		if (Flags.BDel && e->_shpList) {
			int i;
			for (i = 0; e->_shpList[i]; i++)
			delete e->_shpList[i];
			if (MemType(e->_shpList) == NEAR_MEM)
				delete[] e->_shpList;
		}
		if (MemType(e->_seq) == NEAR_MEM)
			free(e->_seq);
		if (e->_near)
			free(e->_near);
		if (e->_take)
			free(e->_take);
		delete e;
		_ext = NULL;
	}
	return this;
}


Sprite *Sprite::BackShow(bool fast) {
	Expand();
	Show(2);
	Show(1);
	if (fast)
		Show(0);
	Contract();
	return this;
}


void Sprite::Step(int nr) {
	if (nr >= 0)
		SeqPtr = nr;
	if (_ext) {
		Seq *seq;
		if (nr < 0)
			SeqPtr = _ext->_seq[SeqPtr].Next;
		seq = _ext->_seq + SeqPtr;
		if (seq->Dly >= 0) {
			Goto(X + (seq->Dx), Y + (seq->Dy));
			Time = seq->Dly;
		}
	}
}


void Sprite::Tick(void) {
	Step();
}


void Sprite::MakeXlat(uint8 *x) {
	if (_ext) {
		BMP_PTR *b;

		if (Flags.Xlat)
			KillXlat();
		for (b = _ext->_shpList; *b; b++)
			(*b)->M = x;
		Flags.Xlat = true;
	}
}


void Sprite::KillXlat(void) {
	if (Flags.Xlat && _ext) {
		BMP_PTR *b;
		uint8 *m = (*_ext->_shpList)->M;

		switch (MemType(m)) {
		case NEAR_MEM :
			delete[](uint8 *) m;
			break;
		case FAR_MEM  :
			free(m);
			break;
		}
		for (b = _ext->_shpList; *b; b++)
			(*b)->M = NULL;
		Flags.Xlat = false;
	}
}


void Sprite::Goto(int x, int y) {
	int xo = X, yo = Y;
	if (W < SCR_WID) {
		if (x < 0)
			x = 0;
		if (x + W > SCR_WID)
			x = (SCR_WID - W);
		X = x;
	}
	if (H < SCR_HIG) {
		if (y < 0)
			y = 0;
		if (y + H > SCR_HIG)
			y = (SCR_HIG - H);
		Y = y;
	}
	if (Next)
		if (Next->Flags.Slav)
			Next->Goto(Next->X - xo + X, Next->Y - yo + Y);
	if (Flags.Shad)
		Prev->Goto(Prev->X - xo + X, Prev->Y - yo + Y);
}


void Sprite::Center(void) {
	Goto((SCR_WID - W) / 2, (SCR_HIG - H) / 2);
}


void Sprite::Show(void) {
	register SprExt *e;
// asm cli     // critic section...
	e = _ext;
	e->_x0 = e->_x1;
	e->_y0 = e->_y1;
	e->_b0 = e->_b1;
	e->_x1 = X;
	e->_y1 = Y;
	e->_b1 = Shp();
//  asm sti     // ...done!
	if (! Flags.Hide) {
		if (Flags.Xlat)
			e->_b1->XShow(e->_x1, e->_y1);
		else
			e->_b1->Show(e->_x1, e->_y1);
	}
}


void Sprite::Show(uint16 pg) {
	Graphics::Surface *a = VGA::Page[1];
	VGA::Page[1] = VGA::Page[pg & 3];
	Shp()->Show(X, Y);
	VGA::Page[1] = a;
}


void Sprite::Hide(void) {
	register SprExt *e = _ext;
	if (e->_b0)
		e->_b0->Hide(e->_x0, e->_y0);
}


BMP_PTR Sprite::Ghost(void) {
	register SprExt *e = _ext;
	if (e->_b1) {
		BMP_PTR bmp = new BITMAP(0, 0, (uint8 *)NULL);
		if (bmp == NULL)
			error("No core");
		bmp->W = e->_b1->W;
		bmp->H = e->_b1->H;
		if ((bmp->B = farnew(HideDesc, bmp->H)) == NULL)
			error("No Core");
		bmp->V = (uint8 *) memcpy(bmp->B, e->_b1->B, sizeof(HideDesc) * bmp->H);
		// TODO offset correctly in the surface using y1 pitch and x1 and not via offset segment
		//bmp->M = (uint8 *) MK_FP(e->y1, e->x1);
		warning("FIXME: SPRITE::Ghost");
		return bmp;
	}
	return NULL;
}


Sprite *SpriteAt(int x, int y) {
	Sprite *spr = NULL, * tail = Vga->ShowQ->Last();
	if (tail) {
		for (spr = tail->Prev; spr; spr = spr->Prev)
			if (! spr->Flags.Hide && ! spr->Flags.Tran)
				if (spr->Shp()->SolidAt(x - spr->X, y - spr->Y))
					break;
	}
	return spr;
}


QUEUE::QUEUE(bool show) : Head(NULL), Tail(NULL), Show(show) {
}


QUEUE::~QUEUE(void) {
	Clear();
}


void QUEUE::Clear(void) {
	while (Head) {
		Sprite *s = Remove(Head);
		if (s->Flags.Kill)
			delete s;
	}
}


void QUEUE::ForAll(void (*fun)(Sprite *)) {
	Sprite *s = Head;
	while (s) {
		Sprite *n = s->Next;
		fun(s);
		s = n;
	}
}


void QUEUE::Append(Sprite *spr) {
	if (Tail) {
		spr->Prev = Tail;
		Tail->Next = spr;
	} else
		Head = spr;
	Tail = spr;
	if (Show)
		spr->Expand();
	else
		spr->Contract();
}


void QUEUE::Insert(Sprite *spr, Sprite *nxt) {
	if (nxt == Head) {
		spr->Next = Head;
		Head = spr;
		if (! Tail)
			Tail = spr;
	} else {
		spr->Next = nxt;
		spr->Prev = nxt->Prev;
		if (spr->Prev)
			spr->Prev->Next = spr;
	}
	if (spr->Next)
		spr->Next->Prev = spr;
	if (Show)
		spr->Expand();
	else
		spr->Contract();
}


void QUEUE::Insert(Sprite *spr) {
	Sprite *s;
	for (s = Head; s; s = s->Next)
		if (s->Z > spr->Z)
			break;
	if (s)
		Insert(spr, s);
	else
		Append(spr);
	if (Show)
		spr->Expand();
	else
		spr->Contract();
}


Sprite *QUEUE::Remove(Sprite *spr) {
	if (spr == Head)
		Head = spr->Next;
	if (spr == Tail)
		Tail = spr->Prev;
	if (spr->Next)
		spr->Next->Prev = spr->Prev;
	if (spr->Prev)
		spr->Prev->Next = spr->Next;
	spr->Prev = NULL;
	spr->Next = NULL;
	return spr;
}


Sprite *QUEUE::Locate(int ref) {
	Sprite *spr;
	for (spr = Head; spr; spr = spr->Next) {
		if (spr->_ref == ref)
			return spr;
	}
	return NULL;
}


//extern const char Copr[];
Graphics::Surface *VGA::Page[4];
DAC *VGA::SysPal;

void VGA::init() {
	for (int idx = 0; idx < 4; ++idx) {
		Page[idx] = new Graphics::Surface();
		Page[idx]->create(320, 200, Graphics::PixelFormat::createFormatCLUT8());
	}

	SysPal = new DAC[PAL_CNT];
}

void VGA::deinit() {
	for (int idx = 0; idx < 4; ++idx)
		delete Page[idx];

	delete[] SysPal;
}

VGA::VGA(int mode)
	: FrmCnt(0), OldMode(0), OldScreen(NULL), StatAdr(VGAST1_), 
	  Msg(NULL), Nam(NULL), SetPal(false), Mono(0) {
	OldColors = NULL;
	NewColors = NULL;
	ShowQ = new QUEUE(true);
	SpareQ = new QUEUE(false);

	bool std = true;
	int i;
	for (i = 10; i < 20; i++) {
		char *txt = Text->getText(i);
		if (txt) {
//	  puts(txt);
			warning(txt);
			std = false;
		}
	}
	if (std)
//		warning(Copr);
		warning("TODO: Fix Copr");

	SetStatAdr();
	if (StatAdr != VGAST1_)
		++Mono;
	if (IsVga()) {
		OldColors = farnew(DAC, 256);
		NewColors = farnew(DAC, 256);
		OldScreen = SaveScreen();
		GetColors(OldColors);
		Sunset();
		OldMode = SetMode(mode);
		SetColors();
		Setup(VideoMode);
		Clear(0);
	}
}


VGA::~VGA(void) {
	Mono = 0;
	if (IsVga()) {
		Common::String buffer = "";
		Clear(0);
		SetMode(OldMode);
		SetColors();
		RestoreScreen(OldScreen);
		Sunrise(OldColors);
		if (OldColors)
			free(OldColors);
		if (NewColors)
			free(NewColors);
		if (Msg)
			buffer = Common::String(Msg);
		if (Nam)
			buffer = buffer + " [" + Nam + "]";

		warning(buffer.c_str());
	}
}


void VGA::SetStatAdr(void) {
	/*
	asm    mov dx,VGAMIr_
	asm    in  al,dx
	asm    test    al,1        // CGA addressing mode flag
	asm    mov ax,VGAST1_  // CGA addressing
	asm    jnz set_mode_adr
	asm    xor al,0x60     // MDA addressing
	set_mode_adr:
	StatAdr = _AX;
	*/
	warning("STUB: VGA::SetStatADR");
}


#pragma argsused
void VGA::WaitVR(bool on) {
	// Since some of the game parts rely on using vertical sync as a delay mechanism, 
	// we're introducing a short delay to simulate it
	g_system->delayMillis(10);
}


void VGA::Setup(VgaRegBlk *vrb) {
	/*
	  WaitVR();         // *--LOOK!--* resets VGAATR logic
	  asm   cld
	  asm   mov si, vrb     // take address of parameter table
	  asm   mov dh,0x03     // higher byte of I/O address is always 3

	  s:
	  asm   lodsw           // take lower byte of I/O address and index
	  asm   or  ah,ah       // 0 = end of table
	  asm   jz  xit     // no more: exit
	  asm   or  al,al       // indexed register?
	  asm   js  single      // 7th bit set means single register
	  asm   mov dl,ah       // complete I/O address
	  asm   out dx,al       // put index into control register
	  asm   inc dx      // data register is next to control
	  asm   in  al,dx       // take old data

	  write:
	  asm   mov cl,al       // preserve old data
	  asm   lodsw           // take 2 masks from table
	  asm   xor al,0xFF     // invert mask bits
	  asm   and al,cl       // clear bits with "clr" mask
	  asm   or  al,ah       // set bits with "set" mask
	  asm   cmp dl,0xC1     // special case?
	  asm   jne std2        // no: standard job, otherwise...
	  asm   dec dx      // data out reg shares address with index
	  std2:
	  asm   out dx,al       // write new value to register
	  asm   jmp s

	  single:           // read address in al, write address in ah
	  asm   mov dl,al       // complete I/O read address
	  asm   in  al,dx       // take old data
	  asm   mov dl,ah       // complete I/O write address
	  asm   jmp write       // continue standard routine

	  xit:
	  */
	warning("STUB: VGA::Setup");
}


int VGA::SetMode(int mode) {
	// ScummVM provides it's own vieo services
	return 0;
}


void VGA::GetColors(DAC *tab) {
	byte palData[PAL_SIZ];
	g_system->getPaletteManager()->grabPalette(palData, 0, PAL_CNT);
	pal2DAC(palData, tab);
}

void VGA::pal2DAC(const byte *palData, DAC *tab) {
	const byte *colP = palData;
	for (int idx = 0; idx < PAL_CNT; ++idx, colP += 3) {
		tab[idx].R = *colP;
		tab[idx].G = *(colP + 1);
		tab[idx].B = *(colP + 2);
	}
}

void VGA::DAC2pal(const DAC *tab, byte *palData) {
	for (int idx = 0; idx < PAL_CNT; ++idx, palData += 3) {
		*palData = tab[idx].R << 2;
		*(palData + 1) = tab[idx].G << 2;
		*(palData + 2) = tab[idx].B << 2;
	}
}

void VGA::SetColors(DAC *tab, int lum) {
	DAC *palP = tab;
	for (int idx = 0; idx < PAL_CNT; ++idx, ++palP) {
		palP->R = (palP->R * lum) >> 6;
		palP->G = (palP->G * lum) >> 6;
		palP->B = (palP->B * lum) >> 6;
	}

	if (Mono) {
		palP = tab;
		for (int idx = 0; idx < PAL_CNT; ++idx, ++palP) {
			// Form a greyscalce colour from 30% R, 59% G, 11% B
			uint8 intensity = (palP->R * 77) + (palP->G * 151) + (palP->B * 28);
			palP->R = intensity;
			palP->G = intensity;
			palP->B = intensity;
		}
	}

	SetPal = true;
}


void VGA::SetColors(void) {
	memset(NewColors, 0, PAL_SIZ);
	UpdateColors();
}


void VGA::Sunrise(DAC *tab) {
	for (int i = 0; i <= 64; i += FADE_STEP) {
		SetColors(tab, i);
		WaitVR(true);
		UpdateColors();
	}
}


void VGA::Sunset(void) {
	DAC tab[256];
	GetColors(tab);
	for (int i = 64; i >= 0; i -= FADE_STEP) {
		SetColors(tab, i);
		WaitVR(true);
		UpdateColors();
	}
}


void VGA::Show(void) {
	Sprite *spr = ShowQ->First();

	for (spr = ShowQ->First(); spr; spr = spr->Next)
		spr->Show();
	Update();
	for (spr = ShowQ->First(); spr; spr = spr->Next)
		spr->Hide();

	++ FrmCnt;
}


void VGA::UpdateColors(void) {
	byte palData[PAL_SIZ];
	DAC2pal(NewColors, palData);
	g_system->getPaletteManager()->setPalette(palData, 0, 256);
}


void VGA::Update(void) {
	SWAP(VGA::Page[0], VGA::Page[1]);

	if (SetPal) {
		UpdateColors();
		SetPal = false;
	}

	g_system->copyRectToScreen((const byte *)VGA::Page[0]->getBasePtr(0, 0), SCR_WID, 0, 0, SCR_WID, SCR_HIG);
	g_system->updateScreen();
}


void VGA::Clear(uint8 color) {
	for (int paneNum = 0; paneNum < 4; ++paneNum)
		Page[paneNum]->fillRect(Common::Rect(0, 0, SCR_WID, SCR_HIG), color);
}


void VGA::CopyPage(uint16 d, uint16 s) {
	Page[d]->copyFrom(*Page[s]);
}

//--------------------------------------------------------------------------

void BITMAP::XShow(int x, int y) {
	/*
	  uint8 rmsk = x % 4,
	       mask = 1 << rmsk,
	       *scr = VGA::Page[1] + y * (SCR_WID / 4) + x / 4;
	  uint8 *m = (char *) M;
	  uint8  *v = V;

	    asm push    bx
	    asm push    si
	    asm push    ds

	    asm cld
	    asm les di,scr
	    asm lds si,v
	    asm mov bx,m

	    asm mov al,0x02     // map mask register
	    asm mov ah,mask

	  plane:
	    // enable output plane
	    asm mov dx,VGASEQ_
	    asm out dx,ax
	    asm push    ax

	    // select input plane
	    asm mov dx,VGAGRA_
	    asm mov al,0x04     // read map select register
	    asm mov ah,rmsk
	    asm out dx,ax

	    asm push    di

	  block:
	    asm lodsw
	    asm mov cx,ax
	    asm and ch,0x3F
	    asm test    ah,0xC0
	    asm jz  endpl
	    asm jns skip
	    asm jnp incsi       // replicate?
	    asm add si,cx       // skip over data block
	    asm dec si      // fix it before following inc

	  incsi:
	    asm inc si
	  tint:
	    asm mov al,es:[di]
	    //-----------------------------------------------
	    // asm  xlat    ss:0    // unsupported with BASM!
	    __emit__(0x36, 0xD7);   // this stands for above!
	    //-----------------------------------------------
	    asm stosb
	    asm loop    tint
	    asm jmp block

	  skip:
	    asm add di,cx
	    asm jmp block

	  endpl:
	    asm pop di
	    asm pop ax
	    asm inc rmsk
	    asm shl ah,1
	    asm test    ah,0x10
	    asm jz  x_chk
	    asm mov ah,0x01
	    asm mov rmsk,0
	    asm inc di
	  x_chk:
	    asm cmp ah,mask
	    asm jne plane
	    asm pop ds
	    asm pop si
	    asm pop bx
	    */
	warning("STUB: BITMAP::XShow");
}


void BITMAP::Show(int x, int y) {
	const byte *srcP = (const byte *)V;
	byte *destEndP = (byte *)VGA::Page[1]->pixels + (SCR_WID * SCR_HIG);

	// Loop through processing data for each plane. The game originally ran in plane mapped mode, where a
	// given plane holds each fourth pixel sequentially. So to handle an entire picture, each plane's data
	// must be decompressed and inserted into the surface
	for (int planeCtr = 0; planeCtr < 4; ++planeCtr) {
		byte *destP = (byte *)VGA::Page[1]->getBasePtr(x + planeCtr, y);

		for (;;) {
			uint16 v = READ_LE_UINT16(srcP);
			srcP += 2;
			int cmd = v >> 14;
			int count = v & 0x3FFF;

			if (cmd == 0) {
				// End of image
				break;
			}

			assert(destP < destEndP);

			// Handle a set of pixels
			while (count-- > 0) {
				// Transfer operation
				switch (cmd) {
				case 1:
					// SKIP
					break;
				case 2:
					// REPEAT
					*destP = *srcP;
					break;
				case 3:
					// COPY
					*destP = *srcP++;
					break;
				}

				// Move to next dest position
				destP += 4; 
			}

			if (cmd == 2)
				++srcP;
		}
	}
/*
	DEBUG code to display image immediately 
	// Temporary
	g_system->copyRectToScreen((const byte *)VGA::Page[1]->getBasePtr(0, 0), SCR_WID, 0, 0, SCR_WID, SCR_HIG);
	byte palData[PAL_SIZ];
	VGA::DAC2pal(VGA::SysPal, palData);
	g_system->getPaletteManager()->setPalette(palData, 0, PAL_CNT);

	g_system->updateScreen();
	g_system->delayMillis(5000);
*/
}


void BITMAP::Hide(int x, int y) {
	/*
	  uint8 *scr = VGA::Page[1] + y * (SCR_WID / 4) + x / 4;
	  uint16 d = FP_OFF(VGA::Page[2]) - FP_OFF(VGA::Page[1]);
	  HideDesc *b = B;
	  uint16 extra = ((x & 3) != 0);
	  uint16 h = H;

	//  asm push    bx
	    asm push    si
	    asm push    ds

	    asm cld
	    asm les di,scr
	    asm mov si,di
	    asm add si,d        // take bytes from background page
	    asm lds bx,b

	    asm mov dx,VGAGRA_
	    asm mov al,0x05     // R/W mode
	    asm out dx,al
	    asm inc dx
	    asm in  al,dx
	    asm and al,0xF4
	    asm push    ax
	    asm push    dx
	    asm or  al,0x01
	    asm out dx,al

	    asm mov dx,VGASEQ_
	    asm mov ax,0x0F02   // enable all planes
	    asm out dx,ax

	    asm mov dx,ds       // save DS

	  row:
	// skip block
	    asm mov cx,[bx]
	    asm add si,cx
	    asm add di,cx
	    asm mov cx,[bx+2]
	    asm add bx,4
	    asm add cx,extra

	    asm push    es
	    asm pop ds      // set DS to video seg
	    asm rep movsb       // move bytes fast
	    asm sub si,extra
	    asm sub di,extra
	    asm mov ds,dx       // restore DS

	    asm dec h
	    asm jnz row

	    asm pop dx
	    asm pop ax
	    asm out dx,al       // end of copy mode


	    asm pop ds
	    asm pop si
	//  asm pop bx
	*/
	warning("STUB: BITMAP::Hide");
}

} // End of namespace CGE
