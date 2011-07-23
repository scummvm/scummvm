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
#include "cge/cge.h"

namespace CGE {

#define FADE_STEP   2
#define TMR_DIV     ((0x8000/TMR_RATE)*2)
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
	{ 0x00,   0x00, 0x00, 0x00 }
};

bool SpeedTest   = false;

Seq *getConstantSeq(bool seqFlag) {
	const Seq seq1[] = { { 0, 0, 0, 0, 0 } };
	const Seq seq2[] = { { 0, 1, 0, 0, 0 }, { 1, 0, 0, 0, 0 } };

	Seq *seq;
	if (seqFlag) {
		seq = (Seq *)malloc(1 * sizeof(Seq));
		*seq = seq1[0];
	} else {
		seq = (Seq *)malloc(2 * sizeof(Seq));
		seq[0] = seq2[0];
		seq[1] = seq2[1];
	}

	return seq;
}


extern "C"  void    SNDMIDIPlay();

/*
static void Video() {
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
}
*/


uint16 *SaveScreen() {
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


Dac mkDac(uint8 r, uint8 g, uint8 b) {
	static Dac x;
	x._r = r;
	x._g = g;
	x._b = b;
	return x;
}


Rgb mkRgb(uint8 r, uint8 g, uint8 b) {
	static Trgb x;
	x._dac._r = r;
	x._dac._g = g;
	x._dac._b = b;
	return x._rgb;
}


Sprite *locate(int ref) {
	Sprite *spr = _vga->_showQ->locate(ref);
	return (spr) ? spr : _vga->_spareQ->locate(ref);
}


Heart::Heart()
	: Engine_(TMR_DIV) {
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
					Sys->tick();
			}
		}
		for (spr = VGA::ShowQ.First(); spr; spr = spr->Next) {
			if (spr->Time) {
				if (!spr->_flags.Hide) {
					if (-- spr->Time == 0)
						spr->tick();
				}
			}
		}
		asm   mov ss,oldSS
		asm   mov sp,oldSP
		run--;
	}
}
*/


void Engine_::newTimer(...) {
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
					Sys->tick();
			}
		}

	    for (spr = VGA::ShowQ.First(); spr; spr = spr->Next) {
			if (spr->Time) { 
				if (!spr->_flags.Hide) {
					if (--spr->Time == 0) 
						spr->tick();
				}
			}
		}
	    asm   mov ss,oldSS
	    asm   mov sp,oldSP
	    run--;
	  }

	*/
	warning("STUB: Engine_::NewTimer");
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


Sprite::Sprite(CGEEngine *vm, BMP_PTR *shpP)
	: _x(0), _y(0), _z(0), _nearPtr(0), _takePtr(0),
	  _next(NULL), _prev(NULL), _seqPtr(NO_SEQ), _time(0), //Delay(0),
	  _ext(NULL), _ref(-1), _cave(0), _vm(vm) {
	memset(_file, 0, sizeof(_file));
	*((uint16 *)&_flags) = 0;
	_ref = 0;
	_x = _y = 0;
	_w = _h = 0;
	_time = 0;
	_seqPtr = 0;
	_shpCnt = 0;
	_prev = _next = NULL;

	setShapeList(shpP);
}


Sprite::~Sprite() {
	contract();
}


BMP_PTR Sprite::shp() {
	register SprExt *e = _ext;
	if (e)
		if (e->_seq) {
			int i = e->_seq[_seqPtr]._now;
			if (i >= _shpCnt) {
				//char s[256];
				//sprintf(s, "Seq=%p ShpCnt=%d SeqPtr=%d Now=%d Next=%d",
				//      Seq, ShpCnt, SeqPtr, Seq[SeqPtr].Now, Seq[SeqPtr].Next);
				//VGA::Exit(s, File);
				error("Invalid PHASE in SPRITE::Shp() %s", _file);
			}
			return e->_shpList[i];
		}
	return NULL;
}


BMP_PTR *Sprite::setShapeList(BMP_PTR *shpP) {
	BMP_PTR *r = (_ext) ? _ext->_shpList : NULL;

	_shpCnt = 0;
	_w = 0;
	_h = 0;

	if (shpP) {
		BMP_PTR *p;
		for (p = shpP; *p; p++) {
			BMP_PTR b = (*p); // ->Code();
			if (b->_w > _w)
				_w = b->_w;
			if (b->_h > _h)
				_h = b->_h;
			_shpCnt++;
		}
		expand();
		_ext->_shpList = shpP;
		_flags._bDel = true;
		if (!_ext->_seq)
			setSeq(getConstantSeq(_shpCnt < 2));
	}
	return r;
}


void Sprite::moveShapes(uint8 *buf) {
	BMP_PTR *p;
	for (p = _ext->_shpList; *p; p++) {
		buf += (*p)->moveVmap(buf);
	}
}


bool Sprite::works(Sprite *spr) {
	if (spr)
		if (spr->_ext) {
			Snail::Com *c = spr->_ext->_take;
			if (c != NULL) {
				c += spr->_takePtr;
				if (c->_ref == _ref)
					if (c->_com != kSnLabel || (c->_val == 0 || c->_val == _vm->_now))
						return true;
			}
		}
	return false;
}


Seq *Sprite::setSeq(Seq *seq) {
	if (_ext) {
		free(_ext->_seq);
		_ext->_seq = NULL;
	}

	expand();
	register Seq *s = _ext->_seq;
	_ext->_seq = seq;
	if (_seqPtr == NO_SEQ)
		step(0);
	else if (_time == 0)
		step(_seqPtr);
	return s;
}


bool Sprite::seqTest(int n) {
	if (n >= 0)
		return (_seqPtr == n);
	if (_ext)
		return (_ext->_seq[_seqPtr]._next == _seqPtr);
	return true;
}


Snail::Com *Sprite::snList(SnList type) {
	register SprExt *e = _ext;
	if (e)
		return (type == kNear) ? e->_near : e->_take;
	return NULL;
}


void Sprite::setName(char *n) {
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


Sprite *Sprite::expand() {
	if (!_ext) {
		bool enbl = _heart->_enable;
		_heart->_enable = false;
		if ((_ext = new SprExt) == NULL)
			error("No core");
		if (*_file) {
			static const char *Comd[] = { "Name", "Phase", "Seq", "Near", "Take", NULL };
			char line[kLineMax], fname[kPathMax];
			BMP_PTR *shplist = new BMP_PTR[_shpCnt + 1];
			Seq *seq = NULL;
			int shpcnt = 0,
			    seqcnt = 0,
			    neacnt = 0,
			    takcnt = 0,
			    maxnow = 0,
			    maxnxt = 0;

			Snail::Com *nea = NULL;
			Snail::Com *tak = NULL;
			mergeExt(fname, _file, SPR_EXT);
			if (INI_FILE::exist(fname)) { // sprite description file exist
				INI_FILE sprf(fname);
				if (!(sprf._error==0))
					error("Bad SPR [%s]", fname);
				int len = 0, lcnt = 0;
				while ((len = sprf.read((uint8 *)line)) != 0) {
					++lcnt;
					if (len && line[len - 1] == '\n')
						line[-- len] = '\0';
					if (len == 0 || *line == '.')
						continue;

					switch (takeEnum(Comd, strtok(line, " =\t"))) {
					case 0 : { // Name
						setName(strtok(NULL, ""));
						break;
					}
					case 1 : { // Phase
						shplist[shpcnt++] = new Bitmap(strtok(NULL, " \t,;/"), true);
						break;
					}
					case 2 : { // Seq
						seq = (Seq *) realloc(seq, (seqcnt + 1) * sizeof(*seq));
						if (seq == NULL)
							error("No core [%s]", fname);
						Seq *s = &seq[seqcnt++];
						s->_now  = atoi(strtok(NULL, " \t,;/"));
						if (s->_now > maxnow)
							maxnow = s->_now;
						s->_next = atoi(strtok(NULL, " \t,;/"));
						switch (s->_next) {
						case 0xFF :
							s->_next = seqcnt;
							break;
						case 0xFE :
							s->_next = seqcnt - 1;
							break;
						}
						if (s->_next > maxnxt)
							maxnxt = s->_next;
						s->_dx   = atoi(strtok(NULL, " \t,;/"));
						s->_dy   = atoi(strtok(NULL, " \t,;/"));
						s->_dly  = atoi(strtok(NULL, " \t,;/"));
						break;
					}
					case 3 : { // Near
						if (_nearPtr != NO_PTR) {
							nea = (Snail::Com *) realloc(nea, (neacnt + 1) * sizeof(*nea));
							if (nea == NULL)
								error("No core [%s]", fname);
							else {
								Snail::Com *c = &nea[neacnt++];
								if ((c->_com = (SnCom)takeEnum(Snail::_comTxt, strtok(NULL, " \t,;/"))) < 0)
									error("Bad NEAR in %d [%s]", lcnt, fname);
								c->_ref = atoi(strtok(NULL, " \t,;/"));
								c->_val = atoi(strtok(NULL, " \t,;/"));
								c->_ptr = NULL;
							}
						}
					}
					break;
					case 4 : { // Take
						if (_takePtr != NO_PTR) {
							tak = (Snail::Com *) realloc(tak, (takcnt + 1) * sizeof(*tak));
							if (tak == NULL)
								error("No core [%s]", fname);
							else {
								Snail::Com *c = &tak[takcnt++];
								if ((c->_com = (SnCom)takeEnum(Snail::_comTxt, strtok(NULL, " \t,;/"))) < 0)
									error("Bad NEAR in %d [%s]", lcnt, fname);
								c->_ref = atoi(strtok(NULL, " \t,;/"));
								c->_val = atoi(strtok(NULL, " \t,;/"));
								c->_ptr = NULL;
							}
						}
						break;
					}
					}
				}
			} else { // no sprite description: try to read immediately from .BMP
				shplist[shpcnt++] = new Bitmap(_file, true);
			}
			shplist[shpcnt] = NULL;
			if (seq) {
				if (maxnow >= shpcnt)
					error("Bad PHASE in SEQ [%s]", fname);
				if (maxnxt >= seqcnt)
					error("Bad JUMP in SEQ [%s]", fname);
				setSeq(seq);
			} else
				setSeq(getConstantSeq(_shpCnt == 1));

			setShapeList(shplist);

			if (nea)
				nea[neacnt - 1]._ptr = _ext->_near = nea;
			else
				_nearPtr = NO_PTR;
			if (tak)
				tak[takcnt - 1]._ptr = _ext->_take = tak;
			else
				_takePtr = NO_PTR;
		}
		_heart->_enable = enbl;
	}
	return this;
}


Sprite *Sprite::contract() {
	register SprExt *e = _ext;
	if (e) {
		if (e->_name)
			delete[] e->_name;
		if (_flags._bDel && e->_shpList) {
			int i;
			for (i = 0; e->_shpList[i]; i++)
				delete e->_shpList[i];
			delete[] e->_shpList;
		}

		free(e->_seq);
		free(e->_near);
		free(e->_take);

		delete e;
		_ext = NULL;
	}
	return this;
}


Sprite *Sprite::backShow(bool fast) {
	expand();
	show(2);
	show(1);
	if (fast)
		show(0);
	contract();
	return this;
}


void Sprite::step(int nr) {
	if (nr >= 0)
		_seqPtr = nr;
	if (_ext) {
		Seq *seq;
		if (nr < 0)
			_seqPtr = _ext->_seq[_seqPtr]._next;
		seq = _ext->_seq + _seqPtr;
		if (seq->_dly >= 0) {
			gotoxy(_x + (seq->_dx), _y + (seq->_dy));
			_time = seq->_dly;
		}
	}
}


void Sprite::tick() {
	step();
}


void Sprite::makeXlat(uint8 *x) {
	if (_ext) {
		BMP_PTR *b;

		if (_flags._xlat)
			killXlat();
		for (b = _ext->_shpList; *b; b++)
			(*b)->_m = x;
		_flags._xlat = true;
	}
}


void Sprite::killXlat() {
	if (_flags._xlat && _ext) {
		BMP_PTR *b;
		uint8 *m = (*_ext->_shpList)->_m;
		free(m);

		for (b = _ext->_shpList; *b; b++)
			(*b)->_m = NULL;
		_flags._xlat = false;
	}
}


void Sprite::gotoxy(int x, int y) {
	int xo = _x, yo = _y;
	if (_x < kScrWidth) {
		if (x < 0)
			x = 0;
		if (x + _w > kScrWidth)
			x = (kScrWidth - _w);
		_x = x;
	}
	if (_h < kScrHeight) {
		if (y < 0)
			y = 0;
		if (y + _h > kScrHeight)
			y = (kScrHeight - _h);
		_y = y;
	}
	if (_next)
		if (_next->_flags._slav)
			_next->gotoxy(_next->_x - xo + _x, _next->_y - yo + _y);
	if (_flags._shad)
		_prev->gotoxy(_prev->_x - xo + _x, _prev->_y - yo + _y);
}


void Sprite::center() {
	gotoxy((kScrWidth - _w) / 2, (kScrHeight - _h) / 2);
}


void Sprite::show() {
	register SprExt *e;
// asm cli     // critic section...
	e = _ext;
	e->_x0 = e->_x1;
	e->_y0 = e->_y1;
	e->_b0 = e->_b1;
	e->_x1 = _x;
	e->_y1 = _y;
	e->_b1 = shp();
//  asm sti     // ...done!
	if (!_flags._hide) {
		if (_flags._xlat)
			e->_b1->xShow(e->_x1, e->_y1);
		else
			e->_b1->show(e->_x1, e->_y1);
	}
}


void Sprite::show(uint16 pg) {
	Graphics::Surface *a = Vga::_page[1];
	Vga::_page[1] = Vga::_page[pg & 3];
	shp()->show(_x, _y);
	Vga::_page[1] = a;
}


void Sprite::hide() {
	register SprExt *e = _ext;
	if (e->_b0)
		e->_b0->hide(e->_x0, e->_y0);
}


BMP_PTR Sprite::ghost() {
	register SprExt *e = _ext;
	if (e->_b1) {
		BMP_PTR bmp = new Bitmap(0, 0, (uint8 *)NULL);
		if (bmp == NULL)
			error("No core");
		bmp->_w = e->_b1->_w;
		bmp->_h = e->_b1->_h;
		if ((bmp->_b = new HideDesc[bmp->_h]) == NULL)
			error("No Core");
		bmp->_v = (uint8 *) memcpy(bmp->_b, e->_b1->_b, sizeof(HideDesc) * bmp->_h);
		bmp->_map = (e->_y1 << 16) + e->_x1;
		return bmp;
	}
	return NULL;
}

void Sprite::sync(Common::Serializer &s) {
	uint16 unused;

	s.syncAsUint16LE(unused);
	s.syncAsUint16LE(unused);	// _ext
	s.syncAsUint16LE(_ref);
	s.syncAsByte(_cave);
	s.syncBytes((byte *)&_flags, 2);
	s.syncAsUint16LE(_x);
	s.syncAsUint16LE(_y);
	s.syncAsByte(_z);
	s.syncAsUint16LE(_w);
	s.syncAsUint16LE(_h);
	s.syncAsUint16LE(_time);
	s.syncAsByte(_nearPtr);
	s.syncAsByte(_takePtr);
	s.syncAsSint16LE(_seqPtr);
	s.syncAsUint16LE(_shpCnt);
	s.syncBytes((byte *)&_file[0], 9);
	_file[8] = '\0';

	s.syncAsUint16LE(unused);	// _prev
	s.syncAsUint16LE(unused);	// _next
}

Sprite *spriteAt(int x, int y) {
	Sprite *spr = NULL, * tail = _vga->_showQ->last();
	if (tail) {
		for (spr = tail->_prev; spr; spr = spr->_prev) {
			if (! spr->_flags._hide && ! spr->_flags._tran) {
				if (spr->shp()->solidAt(x - spr->_x, y - spr->_y))
					break;
			}
		}
	}
	return spr;
}


Queue::Queue(bool show) : _head(NULL), _tail(NULL), _show(show) {
}


Queue::~Queue() {
	clear();
}


void Queue::clear() {
	while (_head) {
		Sprite *s = remove(_head);
		if (s->_flags._kill)
			delete s;
	}
}


void Queue::forAll(void (*fun)(Sprite *)) {
	Sprite *s = _head;
	while (s) {
		Sprite *n = s->_next;
		fun(s);
		s = n;
	}
}


void Queue::append(Sprite *spr) {
	if (_tail) {
		spr->_prev = _tail;
		_tail->_next = spr;
	} else
		_head = spr;
	_tail = spr;
	if (_show)
		spr->expand();
	else
		spr->contract();
}


void Queue::insert(Sprite *spr, Sprite *nxt) {
	if (nxt == _head) {
		spr->_next = _head;
		_head = spr;
		if (!_tail)
			_tail = spr;
	} else {
		assert(nxt);
		spr->_next = nxt;
		spr->_prev = nxt->_prev;
		if (spr->_prev)
			spr->_prev->_next = spr;
	}
	if (spr->_next)
		spr->_next->_prev = spr;
	if (_show)
		spr->expand();
	else
		spr->contract();
}


void Queue::insert(Sprite *spr) {
	Sprite *s;
	for (s = _head; s; s = s->_next)
		if (s->_z > spr->_z)
			break;
	if (s)
		insert(spr, s);
	else
		append(spr);
	if (_show)
		spr->expand();
	else
		spr->contract();
}

template<typename T>
inline bool contains(const Common::List<T> &l, const T &v) {
	return (Common::find(l.begin(), l.end(), v) != l.end());
}

Sprite *Queue::remove(Sprite *spr) {
	if (spr == _head)
		_head = spr->_next;
	if (spr == _tail)
		_tail = spr->_prev;
	if (spr->_next)
		spr->_next->_prev = spr->_prev;
	if (spr->_prev)
		spr->_prev->_next = spr->_next;
	spr->_prev = NULL;
	spr->_next = NULL;
	return spr;
}


Sprite *Queue::locate(int ref) {
	Sprite *spr;
	for (spr = _head; spr; spr = spr->_next) {
		if (spr->_ref == ref)
			return spr;
	}
	return NULL;
}


//extern const char Copr[];
Graphics::Surface *Vga::_page[4];
Dac *Vga::_sysPal;

void Vga::init() {
	for (int idx = 0; idx < 4; ++idx) {
		_page[idx] = new Graphics::Surface();
		_page[idx]->create(320, 200, Graphics::PixelFormat::createFormatCLUT8());
	}

	_sysPal = new Dac[PAL_CNT];
}

void Vga::deinit() {
	for (int idx = 0; idx < 4; ++idx) {
		_page[idx]->free();
		delete _page[idx];
	}

	delete[] _sysPal;
}

Vga::Vga(int mode)
	: _frmCnt(0), _oldMode(0), _oldScreen(NULL), _statAdr(VGAST1_), 
	  _msg(NULL), _nam(NULL), _setPal(false), _mono(0) {
	_oldColors = NULL;
	_newColors = NULL;
	_showQ = new Queue(true);
	_spareQ = new Queue(false);

	bool std = true;
	int i;
	for (i = 10; i < 20; i++) {
		char *txt = _text->getText(i);
		if (txt) {
			debugN("%s", txt);
			std = false;
		}
	}
	if (std)
//		warning(Copr);
		warning("TODO: Fix Copr");

	setStatAdr();
	if (_statAdr != VGAST1_)
		_mono++;
	_oldColors = (Dac *) malloc(sizeof(Dac) * PAL_CNT);
	_newColors = (Dac *) malloc(sizeof(Dac) * PAL_CNT);
	_oldScreen = SaveScreen();
	getColors(_oldColors);
	sunset();
	_oldMode = setMode(mode);
	setColors();
	setup(VideoMode);
	clear(0);
}


Vga::~Vga() {
	_mono = 0;

	Common::String buffer = "";
/*
	clear(0);
	setMode(_oldMode);
	setColors();
	restoreScreen(_oldScreen);
	sunrise(_oldColors);
*/
	free(_oldColors);
	free(_newColors);
	if (_msg)
		buffer = Common::String(_msg);
	if (_nam)
		buffer = buffer + " [" + _nam + "]";

	debugN("%s", buffer.c_str());

	delete _showQ;
	delete _spareQ;
}


void Vga::setStatAdr() {
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
	warning("STUB: VGA::setStatADR");
}


#pragma argsused
void Vga::waitVR(bool on) {
	// Since some of the game parts rely on using vertical sync as a delay mechanism, 
	// we're introducing a short delay to simulate it
	g_system->delayMillis(10);
}


void Vga::setup(VgaRegBlk *vrb) {
	/*
	  waitVR();         // *--LOOK!--* resets VGAATR logic
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
	warning("STUB: VGA::setup");
}


int Vga::setMode(int mode) {
	// ScummVM provides it's own vieo services
	return 0;
}


void Vga::getColors(Dac *tab) {
	byte palData[PAL_SIZ];
	g_system->getPaletteManager()->grabPalette(palData, 0, PAL_CNT);
	palToDac(palData, tab);
}

void Vga::palToDac(const byte *palData, Dac *tab) {
	const byte *colP = palData;
	for (int idx = 0; idx < PAL_CNT; ++idx, colP += 3) {
		tab[idx]._r = *colP >> 2;
		tab[idx]._g = *(colP + 1) >> 2;
		tab[idx]._b = *(colP + 2) >> 2;
	}
}

void Vga::dacToPal(const Dac *tab, byte *palData) {
	for (int idx = 0; idx < PAL_CNT; ++idx, palData += 3) {
		*palData = tab[idx]._r << 2;
		*(palData + 1) = tab[idx]._g << 2;
		*(palData + 2) = tab[idx]._b << 2;
	}
}

void Vga::setColors(Dac *tab, int lum) {
	Dac *palP = tab, *destP = _newColors;
	for (int idx = 0; idx < PAL_CNT; ++idx, ++palP, ++destP) {
		destP->_r = (palP->_r * lum) >> 6;
		destP->_g = (palP->_g * lum) >> 6;
		destP->_b = (palP->_b * lum) >> 6;
	}

	if (_mono) {
		destP = _newColors;
		for (int idx = 0; idx < PAL_CNT; ++idx, ++destP) {
			// Form a greyscalce colour from 30% R, 59% G, 11% B
			uint8 intensity = (((int)destP->_r * 77) + ((int)destP->_g * 151) + ((int)destP->_b * 28)) >> 8;
			destP->_r = intensity;
			destP->_g = intensity;
			destP->_b = intensity;
		}
	}

	_setPal = true;
}


void Vga::setColors() {
	memset(_newColors, 0, PAL_SIZ);
	updateColors();
}


void Vga::sunrise(Dac *tab) {
	for (int i = 0; i <= 64; i += FADE_STEP) {
		setColors(tab, i);
		waitVR(true);
		updateColors();
	}
}


void Vga::sunset() {
	Dac tab[256];
	getColors(tab);
	for (int i = 64; i >= 0; i -= FADE_STEP) {
		setColors(tab, i);
		waitVR(true);
		updateColors();
	}
}


void Vga::show() {
	Sprite *spr = _showQ->first();

	for (spr = _showQ->first(); spr; spr = spr->_next)
		spr->show();
	update();
	for (spr = _showQ->first(); spr; spr = spr->_next)
		spr->hide();

	_frmCnt++;
}


void Vga::updateColors() {
	byte palData[PAL_SIZ];
	dacToPal(_newColors, palData);
	g_system->getPaletteManager()->setPalette(palData, 0, 256);
}


void Vga::update() {
	SWAP(Vga::_page[0], Vga::_page[1]);

	if (_setPal) {
		updateColors();
		_setPal = false;
	}

	g_system->copyRectToScreen((const byte *)Vga::_page[0]->getBasePtr(0, 0), kScrWidth, 0, 0, kScrWidth, kScrHeight);
	g_system->updateScreen();
}


void Vga::clear(uint8 color) {
	for (int paneNum = 0; paneNum < 4; ++paneNum)
		_page[paneNum]->fillRect(Common::Rect(0, 0, kScrWidth, kScrHeight), color);
}


void Vga::copyPage(uint16 d, uint16 s) {
	_page[d]->copyFrom(*_page[s]);
}

//--------------------------------------------------------------------------

void Bitmap::xShow(int16 x, int16 y) {
	debugC(4, kCGEDebugBitmap, "Bitmap::xShow(%d, %d)", x, y);

	const byte *srcP = (const byte *)_v;
	byte *destEndP = (byte *)Vga::_page[1]->pixels + (kScrWidth * kScrHeight);
	byte *lookupTable = _m;

	// Loop through processing data for each plane. The game originally ran in plane mapped mode, where a
	// given plane holds each fourth pixel sequentially. So to handle an entire picture, each plane's data
	// must be decompressed and inserted into the surface
	for (int planeCtr = 0; planeCtr < 4; ++planeCtr) {
		byte *destP = (byte *)Vga::_page[1]->getBasePtr(x + planeCtr, y);

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

			if (cmd == 2)
				++srcP;
			else if (cmd == 3)
				srcP += count;

			// Handle a set of pixels
			while (count-- > 0) {
				// Transfer operation
				switch (cmd) {
				case 1:
					// SKIP
					break;
				case 2:
				case 3:
					// TINT
					*destP = lookupTable[*destP];
					break;
				}

				// Move to next dest position
				destP += 4; 
			}
		}
	}
}


void Bitmap::show(int16 x, int16 y) {
	debugC(5, kCGEDebugBitmap, "Bitmap::show(%d, %d)", x, y);

	const byte *srcP = (const byte *)_v;
	byte *destEndP = (byte *)Vga::_page[1]->pixels + (kScrWidth * kScrHeight);

	// Loop through processing data for each plane. The game originally ran in plane mapped mode, where a
	// given plane holds each fourth pixel sequentially. So to handle an entire picture, each plane's data
	// must be decompressed and inserted into the surface
	for (int planeCtr = 0; planeCtr < 4; ++planeCtr) {
		byte *destP = (byte *)Vga::_page[1]->getBasePtr(x + planeCtr, y);

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


void Bitmap::hide(int16 x, int16 y) {
	debugC(5, kCGEDebugBitmap, "Bitmap::hide(%d, %d)", x, y);

	for (int yp = y; yp < y + _h; yp++) {
		const byte *srcP = (const byte *)Vga::_page[2]->getBasePtr(x, yp);
		byte *destP = (byte *)Vga::_page[1]->getBasePtr(x, yp);

		Common::copy(srcP, srcP + _w, destP);
	}
}

/*--------------------------------------------------------------------------*/

HorizLine::HorizLine(CGEEngine *vm): Sprite(vm, NULL) {
	// Set the sprite list
	BMP_PTR *HL = new BMP_PTR[2];
	HL[0] = new Bitmap("HLINE", true);
	HL[1] = NULL;

	setShapeList(HL);
}

CavLight::CavLight(CGEEngine *vm): Sprite(vm, NULL) {
	// Set the sprite list
	BMP_PTR *PR = new BMP_PTR[2];
	PR[0] = new Bitmap("PRESS", true);
	PR[1] = NULL;

	setShapeList(PR);
}

Spike::Spike(CGEEngine *vm): Sprite(vm, NULL) {
	// Set the sprite list
	BMP_PTR *SP = new BMP_PTR[3];
	SP[0] = new Bitmap("SPK_L", true);
	SP[1] = new Bitmap("SPK_R", true);
	SP[2] = NULL;

	setShapeList(SP);
}

PocLight::PocLight(CGEEngine *vm): Sprite(vm, NULL) {
	// Set the sprite list
	BMP_PTR *LI = new BMP_PTR[5];
	LI[0] = new Bitmap("LITE0", true);
	LI[1] = new Bitmap("LITE1", true);
	LI[2] = new Bitmap("LITE2", true);
	LI[3] = new Bitmap("LITE3", true);
	LI[4] = NULL;

	setShapeList(LI);

	_flags._kill = false;
}

} // End of namespace CGE
