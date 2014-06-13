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
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.
 *
 */

/*
 * This code is based on original Sfinx source code
 * Copyright (c) 1994-1997 Janus B. Wisniewski and L.K. Avalon
 */

#ifndef CGE2_VGA13H_H
#define CGE2_VGA13H_H

#include "common/serializer.h"
#include "common/events.h"
#include "graphics/surface.h"
#include "cge2/general.h"
#include "cge2/bitmap.h"
#include "cge2/snail.h"
#include "cge2/spare.h"
#include "cge2/cge2.h"

namespace CGE2 {

#define kFadeStep        2
#define kVgaColDark      207
#define kVgaColDarkGray  225 /*219*/
#define kVgaColGray      231
#define kVgaColLightGray 237
#define kPixelTransp     0xFE
#define kNoSeq           (-1)
#define kNoPtr           ((uint8)-1)
#define kSprExt          ".SPR"
#define kPalCount        256
#define kPalSize         (kPalCount * 3)

// From FXP.H
class FXP	// fixed point
{
	uint16 f;
	int16 i;
	long& Joined (void) const { return *(long *)&f; }
public:
	FXP (void): f(0), i(0) { }
	FXP (int i0, int f0 = 0) : i(i0), f((int) ((((long) f0) << 16)/100)) { }
	FXP& operator = (const int& x) { i = x; f = 0; return *this; }
	FXP operator + (const FXP& x) const { FXP y; y.Joined() = Joined()+x.Joined(); return y; }
	FXP operator - (const FXP& x) const { FXP y; y.Joined() = Joined()-x.Joined(); return y; }
	FXP operator * (const FXP& x) const {
		FXP y; long t;
		y.i = i * x.i;
		t = ((long) f * x.f) >> 16;
		t += ((long) i * x.f) + ((long) f  * x.i);
		y.f = t & 0xFFFF;
		y.i += t >> 16;
		return y;
	}
	FXP operator / (const FXP& x) const {
		FXP y; bool sign = false;
		if (!x.empty()) {
			long j = Joined(), jx = x.Joined();
			if (j < 0) {
				j = -j;
				sign ^= 1;
			}
			if (jx < 0) {
				jx = -jx;
				sign ^= 1;
			}
			y.i = signed(j / jx);
			long r = j - jx * y.i;
			//-- binary division
			y.f = unsigned((r << 4) / (jx >> 12));
			//------------------
			if (sign)
				y.Joined() = -y.Joined();
		}

		return y;
	}
	//int& operator = (int& a, const FXP& b) { return a = b.i; }
	friend int& operator += (int& a, const FXP& b) { return a += b.i; }
	friend int& operator -= (int& a, const FXP& b) { return a -= b.i; }
	friend FXP& operator += (FXP& a, const int& b) { a.i += b; return a; }
	friend FXP& operator -= (FXP& a, const int& b) { a.i -= b; return a; }
	friend bool operator == (const FXP &a, const FXP &b) { return (a.i == b.i) && (a.f == b.f); }
	friend bool operator != (const FXP &a, const FXP &b) { return (a.i != b.i) || (a.f != b.f); }
	friend bool operator < (const FXP &a, const FXP &b) { return (a.i < b.i) || ((a.i == b.i) && (a.f < b.f)); }
	friend bool operator > (const FXP &a, const FXP &b) { return (a.i > b.i) || ((a.i == b.i) && (a.f > b.f)); }
	int trunc(void) const { return i; }
	int round(void) const { return i + (f > 0x7FFF); }
	bool empty() const { return i == 0 && f == 0; }
};

// From CGETYPE.H:
class V3D {
public:
	FXP _x, _y, _z;
	V3D() { }
	V3D(FXP x, FXP y, FXP z = 0) : _x(x), _y(y), _z(z) { }
	V3D(const V3D &p) : _x(p._x), _y(p._y), _z(p._z) { }
	V3D operator+(const V3D &p) const { return V3D(_x + p._x, _y + p._y, _z + p._z); }
	V3D operator-(const V3D &p) const { return V3D(_x - p._x, _y - p._y, _z - p._z); }
	V3D operator*(long n) const { return V3D(_x * n, _y * n, _z * n); }
	V3D operator/ (long n) const { return V3D(_x / n, _y / n, _z / n); }
	bool operator==(const V3D &p) const { return _x == p._x && _y == p._y && _z == p._z; }
	bool operator!=(const V3D &p) const { return _x != p._x || _y != p._y || _z != p._z; }
	V3D& operator+=(const V3D &x) { return *this = *this + x; }
	V3D& operator-=(const V3D &x) { return *this = *this - x; }
};

class V2D : public Common::Point {
	CGE2Engine *_vm;
public:
	V2D& operator=(const V3D &p3) {
		FXP m = _vm->_eye->_z / (p3._z - _vm->_eye->_z);
		FXP posx = _vm->_eye->_x + (_vm->_eye->_x - p3._x) * m;
		x = posx.round();
		FXP posy = _vm->_eye->_y + (_vm->_eye->_y - p3._y) * m;
		y = posy.round();
		return *this;
	}
	V2D(CGE2Engine *vm) : _vm(vm) { }
	V2D(CGE2Engine *vm, const V3D &p3) : _vm(vm) { *this = p3; }
	V2D(CGE2Engine *vm, int posx, int posy) : _vm(vm), Common::Point(posx, posy) { }
	bool operator<(const V2D &p) const { return (x <  p.x) && (y <  p.y); }
	bool operator<=(const V2D &p) const { return (x <= p.x) && (y <= p.y); }
	bool operator>(const V2D &p) const { return (x >  p.x) && (y >  p.y); }
	bool operator>=(const V2D &p) const { return (x >= p.x) && (y >= p.y); }
	V2D operator+(const V2D &p) const { return V2D(_vm, x + p.x, y + p.y); }
	V2D operator-(const V2D &p) const { return V2D(_vm, x - p.x, y - p.y); }
	bool operator==(const V3D &p) const { V3D tmp(x, y); return tmp._x == p._x && tmp._y == p._y && tmp._z == p._z; }
	bool operator!=(const V3D &p) const { V3D tmp(x, y); return tmp._x != p._x || tmp._y != p._y || tmp._z == p._z; }
	uint16 area() { return x * y; }
	bool limited(const V2D &p) {
		return ((x < p.x) && (y < p.y));
	}
	V2D scale (int z) {
		FXP m = _vm->_eye->_z / (_vm->_eye->_z - z);
		FXP posx = m * x;
		FXP posy = m * y;
		return V2D(_vm, posx.trunc(), posy.trunc());
	}
	static double trunc(double d) { return (d > 0) ? floor(d) : ceil(d); }
	static double round(double number) { return number < 0.0 ? ceil(number) : floor(number); }
};

struct Seq {
	uint8 _now;
	uint8 _next;
	int8 _dx;
	int8 _dy;
	int8 _dz;
	int _dly;
};

class SprExt {
public:
	V2D _p0;
	V2D _p1;
	BitmapPtr _b0;
	BitmapPtr _b1;
	BitmapPtr _shpList;
	int _location;
	Seq *_seq;
	char *_name;
	CommandHandler::Command *_actions[kActions];
	SprExt(CGE2Engine *vm);
};

class Sprite {
protected:
	SprExt *_ext;
	CGE2Engine *_vm;
public:
	int _ref;
	signed char _scene;
	struct Flags {
		uint16 _hide : 1;       // general visibility switch
		uint16 _drag : 1;       // sprite is moveable
		uint16 _hold : 1;       // sprite is held with mouse
		uint16 _trim : 1;       // Trim flag
		uint16 _slav : 1;       // slave object
		uint16 _kill : 1;       // dispose memory after remove
		uint16 _xlat : 1;       // 2nd way display: xlat table
		uint16 _port : 1;       // portable
		uint16 _kept : 1;       // kept in pocket
		uint16 _frnt : 1;       // stay in front of sprite
		uint16 _east : 1;       // talk to east (in opposite to west)
		uint16 _near : 1;       // Near action lock
		uint16 _shad : 1;       // shadow
		uint16 _back : 1;       // 'send to background' request
		uint16 _zmov : 1;       // sprite needs Z-update in queue
		uint16 _tran : 1;       // transparent (untouchable)
	} _flags;
	V2D _pos2D;
	V3D _pos3D;
	V2D _siz;
	uint16 _time;
	struct { byte _ptr, _cnt; } _actionCtrl[kActions];
	int _seqPtr;
	int _seqCnt;
	int _shpCnt;
	char _file[kMaxFile];
	// Following trailer is not saved with the game:
	Sprite *_prev;
	Sprite *_next;
	static byte _constY;
	static byte _follow;
	static Seq _stdSeq8[];

	bool works(Sprite *spr);
	bool seqTest(int n);
	inline bool active() {
		return _ext != NULL;
	}
	Sprite(CGE2Engine *vm);
	Sprite(CGE2Engine *vm, BitmapPtr shp, int cnt);
	virtual ~Sprite();
	BitmapPtr getShp();
	void setShapeList(BitmapPtr shp, int cnt);
	void moveShapesHi();
	void moveShapesLo();
	int labVal(Action snq, int lab);
	Sprite *expand();
	Sprite *contract();
	void backShow();
	void setName(char *newName);
	inline char *name() {
		return (_ext) ? _ext->_name : NULL;
	}
	void gotoxyz(int x, int y, int z = 0);
	void gotoxyz();
	void gotoxyz(V2D pos);
	void gotoxyz_(V2D pos);
	void gotoxyz(V3D pos);
	void center();
	void show(uint16 pg);
	void hide(uint16 pg);
	void show();
	void hide();
	BitmapPtr ghost();
	void step(int nr = -1);
	Seq *setSeq(Seq *seq);
	CommandHandler::Command *snList(Action type);
	virtual void touch(uint16 mask, int x, int y, Common::KeyCode keyCode);
	virtual void tick();
	virtual void setCave(int c);
	void clrHide() { if (_ext) _ext->_b0 = NULL; }
	void sync(Common::Serializer &s);

	static void (*notify) ();
};

class Queue {
	Sprite *_head;
	Sprite *_tail;
public:
	Queue(bool show);
	~Queue();

	void append(Sprite *spr);
	void insert(Sprite *spr, Sprite *nxt);
	void insert(Sprite *spr);
	Sprite *remove(Sprite *spr);
	Sprite *first() {
		return _head;
	}
	Sprite *last() {
		return _tail;
	}
	Sprite *locate(int ref);
	bool locate(Sprite *spr);
	void clear();
};

class Vga {
	CGE2Engine *_vm;
	bool _setPal;
	Dac *_oldColors;
	Dac *_newColors;
	const char *_msg;
	const char *_name;

	void updateColors();
	void setColors();
	void waitVR();
	uint8 closest(Dac *pal, const uint8 colR, const uint8 colG, const uint8 colB);

public:
	uint32 _frmCnt;
	Queue *_showQ;
	int _mono;
	Graphics::Surface *_page[4];
	Dac *_sysPal;

	Vga(CGE2Engine *vm);
	~Vga();

	uint8 *glass(Dac *pal, const uint8 colR, const uint8 colG, const uint8 colB);
	void getColors(Dac *tab);
	void setColors(Dac *tab, int lum);
	void clear(uint8 color);
	void copyPage(uint16 d, uint16 s);
	void sunrise(Dac *tab);
	void sunset();
	void show();
	void update();

	void palToDac(const byte *palData, Dac *tab);
	void dacToPal(const Dac *tab, byte *palData);
};

class HorizLine: public Sprite {
	CGE2Engine *_vm;
public:
	HorizLine(CGE2Engine *vm);
};

class SceneLight: public Sprite {
	CGE2Engine *_vm;
public:
	SceneLight(CGE2Engine *vm);
};

class Speaker: public Sprite {
	CGE2Engine *_vm;
public:
	Speaker(CGE2Engine *vm);
};

class PocLight: public Sprite {
	CGE2Engine *_vm;
public:
	PocLight(CGE2Engine *vm);
};

} // End of namespace CGE2

#endif // CGE2_VGA13H_H
