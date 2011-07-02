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

#ifndef __CGE_VGA13H__
#define __CGE_VGA13H__

#include "graphics/surface.h"
#include "cge/general.h"
#include <stddef.h>
#include "cge/bitmap.h"
#include "cge/snail.h"
#include "cge/cge.h"

namespace CGE {

#define TMR_RATE1   16
#define TMR_RATE2   4
#define TMR_RATE   (TMR_RATE1 * TMR_RATE2)

#define MAX_NAME    20
#define VIDEO       0x10

#define NO_CLEAR    0x80
#define TEXT_MODE   0x03
#define M13H        0x13

#ifndef SCR_WID
#define SCR_WID     320
#endif

#ifndef SCR_HIG
#define SCR_HIG     200
#endif


#if 0
#define LIGHT       0xFF
#define DARK        0x00
#define DGRAY       0xF6
#define GRAY        0xFC
#define LGRAY       0xFF
#else
#define LIGHT       0xFF
#define DARK        207
#define DGRAY       225 /*219*/
#define GRAY        231
#define LGRAY       237
#endif

#define NO_SEQ      (-1)
#define NO_PTR      ((uint8)-1)

#define SPR_EXT     ".SPR"

#define IsFile(s)   (access(s, 0) == 0)
#define IsWrit(s)   (access(s, 2) == 0)



struct Rgb {
	uint16 r : 2;
	uint16 R : 6;
	uint16 g : 2;
	uint16 G : 6;
	uint16 b : 2;
	uint16 B : 6;
};

typedef union {
	Dac dac;
	Rgb rgb;
} TRGB;

struct VgaRegBlk {
	uint8 idx;
	uint8 adr;
	uint8 clr;
	uint8 set;
};

struct Seq {
	uint8 Now;
	uint8 Next;
	int8 Dx;
	int8 Dy;
	int Dly;
};

extern Seq _seq1[];
extern Seq _seq2[];
//extern    SEQ *   Compass[];
//extern    SEQ TurnToS[];

#define PAL_CNT  256
#define PAL_SIZ (PAL_CNT * 3)
#define VGAATR_ 0x3C0
#define VGAMIw_ 0x3C0
#define VGASEQ_ 0x3C4
#define VGAMIr_ 0x3CC
#define VGAGRA_ 0x3CE
#define VGACRT_ 0x3D4
#define VGAST1_ 0x3DA
#define VGAATR  (VGAATR_ & 0xFF)
#define VGAMIw  (VGAMIw_ & 0xFF)
#define VGASEQ  (VGASEQ_ & 0xFF)
#define VGAMIr  (VGAMIr_ & 0xFF)
#define VGAGRA  (VGAGRA_ & 0xFF)
#define VGACRT  (VGACRT_ & 0xFF)
#define VGAST1  (VGAST1_ & 0xFF)


class Heart : public Engine_ {
	friend class Engine_;
public:
	Heart();

	bool _enable;
	uint16 *_xTimer;

	void setXTimer(uint16 *ptr);
	void setXTimer(uint16 *ptr, uint16 time);
};


class SprExt {
public:
	int _x0, _y0;
	int _x1, _y1;
	BMP_PTR _b0, _b1;
	BMP_PTR *_shpList;
	Seq *_seq;
	char *_name;
	Snail::Com *_near, *_take;
	SprExt() :
		_x0(0), _y0(0),
		_x1(0), _y1(0),
		_b0(NULL), _b1(NULL),
		_shpList(NULL), _seq(NULL),
		_name(NULL), _near(NULL), _take(NULL)
	{}
};


class Sprite {
protected:
	SprExt *_ext;
public:
	int _ref;
	signed char _cave;
	struct FLAGS {
		uint16 _hide : 1;       // general visibility switch
		uint16 _near : 1;       // Near action lock
		uint16 _drag : 1;       // sprite is moveable
		uint16 _hold : 1;       // sprite is held with mouse
		uint16 _____ : 1;       // intrrupt driven animation
		uint16 _slav : 1;       // slave object
		uint16 _syst : 1;       // system object
		uint16 _kill : 1;       // dispose memory after remove
		uint16 _xlat : 1;       // 2nd way display: xlat table
		uint16 _port : 1;       // portable
		uint16 _kept : 1;       // kept in pocket
		uint16 _east : 1;       // talk to east (in opposite to west)
		uint16 _shad : 1;       // shadow
		uint16 _back : 1;       // 'send to background' request
		uint16 _bDel : 1;       // delete bitmaps in ~SPRITE
		uint16 _tran : 1;       // transparent (untouchable)
	} _flags;
	int _x;
	int _y;
	signed char _z;
	uint16 _w;
	uint16 _h;
	uint16 _time;
	uint8 _nearPtr;
	uint8 _takePtr;
	int _seqPtr;
	int _shpCnt;
	char _file[MAXFILE];
	Sprite *_prev;
	Sprite *_next;
	bool works(Sprite *spr);
	bool seqTest(int n);
	inline bool active() {
		return _ext != NULL;
	}
	Sprite(CGEEngine *vm, BMP_PTR *shp);
	virtual ~Sprite(void);
	BMP_PTR shp();
	BMP_PTR *setShapeList(BMP_PTR *shp);
	void moveShapes(uint8 *buf);
	Sprite *expand();
	Sprite *contract();
	Sprite *backShow(bool fast = false);
	void setName(char *n);
	inline char *name() {
		return (_ext) ? _ext->_name : NULL;
	}
	void gotoxy(int x, int y);
	void center();
	void show();
	void hide();
	BMP_PTR ghost();
	void show(uint16 pg);
	void makeXlat(uint8 *x);
	void killXlat();
	void step(int nr = -1);
	Seq *setSeq(Seq *seq);
	Snail::Com *snList(SNLIST type);
	virtual void touch(uint16 mask, int x, int y);
	virtual void tick();
private:
	CGEEngine *_vm;
};


class QUEUE {
	Sprite *Head, * Tail;
public:
	bool Show;
	QUEUE(bool show);
	~QUEUE(void);
	void Append(Sprite *spr);
	void Insert(Sprite *spr, Sprite *nxt);
	void Insert(Sprite *spr);
	Sprite *Remove(Sprite *spr);
	void ForAll(void (*fun)(Sprite *));
	Sprite *First(void) {
		return Head;
	}
	Sprite *Last(void) {
		return Tail;
	}
	Sprite *Locate(int ref);
	void Clear(void);
};


class VGA {
	uint16 OldMode;
	uint16 *OldScreen;
	uint16 StatAdr;
	bool SetPal;
	Dac *OldColors;
	Dac *NewColors;
	const char *Msg;
	const char *Nam;

	int SetMode(int mode);
	void UpdateColors(void);
	void SetColors(void);
	void SetStatAdr(void);
	void WaitVR(bool on);
public:
	uint32 FrmCnt;
	QUEUE *ShowQ, *SpareQ;
	int Mono;
	static Graphics::Surface *Page[4];
	static Dac *SysPal;

	VGA(int mode);
	~VGA(void);
	static void init();
	static void deinit();

	void Setup(VgaRegBlk *vrb);
	void GetColors(Dac *tab);
	void SetColors(Dac *tab, int lum);
	void Clear(uint8 color);
	void CopyPage(uint16 d, uint16 s);
	void Sunrise(Dac *tab);
	void Sunset();
	void Show();
	void Update();

	static void pal2DAC(const byte *palData, Dac *tab);
	static void DAC2pal(const Dac *tab, byte *palData);
};


Dac MkDAC(uint8 r, uint8 g, uint8 b);
Rgb MkRGB(uint8 r, uint8 g, uint8 b);


template <class CBLK>
uint8 Closest(CBLK *pal, CBLK x) {
#define f(col, lum) ((((uint16)(col)) << 8) / lum)
	uint16 i, dif = 0xFFFF, found = 0;
	uint16 L = x._r + x._g + x._b;
	if (!L)
		++L;
	uint16 R = f(x._r, L), G = f(x._g, L), B = f(x._b, L);
	for (i = 0; i < 256; i++) {
		uint16 l = pal[i]._r + pal[i]._g + pal[i]._b;
		if (! l)
			++l;
		int  r = f(pal[i]._r, l), g = f(pal[i]._g, l), b = f(pal[i]._b, l);
		uint16 D = ((r > R) ? (r - R) : (R - r)) +
		           ((g > G) ? (g - G) : (G - g)) +
		           ((b > B) ? (b - B) : (B - b)) +
		           ((l > L) ? (l - L) : (L - l)) * 10 ;

		if (D < dif) {
			found = i;
			dif = D;
			if (D == 0)
				break;    // exact!
		}
	}
	return found;
#undef f
}

//static void       Video       (void);
uint16 *SaveScreen(void);
void    RestoreScreen(uint16 * &sav);
Sprite *SpriteAt(int x, int y);
Sprite *Locate(int ref);

extern      bool        SpeedTest;

} // End of namespace CGE

#endif
