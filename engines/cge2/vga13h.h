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
 * Copyright (c) 1994-1997 Janusz B. Wisniewski and L.K. Avalon
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

class FXP {
	int32 v;
public:
	FXP(void) : v(0) {}
	FXP (int i0, int f0 = 0) : v((i0 * 256) + ((i0 < 0) ? -f0 : f0)) {}
	FXP &operator=(const int &x) { v = x << 8; return *this; }
	FXP operator+(const FXP &x) const { FXP y; y.v = v + x.v; return y; }
	FXP operator-(const FXP &x) const { FXP y; y.v = v - x.v; return y; }
	FXP operator*(const FXP &x) const;
	FXP operator/(const FXP &x) const;

	friend int &operator+=(int &a, const FXP &b) { return a += b.trunc(); }
	friend int &operator-=(int &a, const FXP &b) { return a -= b.trunc(); }
	friend FXP &operator+=(FXP &a, const int &b) { a.v += b << 8; return a; }
	friend FXP &operator-=(FXP &a, const int &b) { a.v -= b << 8; return a; }
	friend bool operator==(const FXP &a, const FXP &b) { return a.v == b.v; }
	friend bool operator!=(const FXP &a, const FXP &b) { return a.v != b.v; }
	friend bool operator<(const FXP &a, const FXP &b) { return a.v < b.v; }
	friend bool operator>(const FXP &a, const FXP &b) { return a.v > b.v; }
	int trunc(void) const { return v >> 8; }
	int round(void) const { return (v + 0x80) >> 8; }
	bool empty() const { return v == 0; }
	void sync(Common::Serializer &s);
};

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
	void sync(Common::Serializer &s);
};

class V2D : public Common::Point {
	CGE2Engine *_vm;
public:
	V2D &operator=(const V3D &p3) {
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
	bool operator<(const V2D &p) const { return (x < p.x) && (y <  p.y); }
	bool operator<=(const V2D &p) const { return (x <= p.x) && (y <= p.y); }
	bool operator>(const V2D &p) const { return (x > p.x) && (y >  p.y); }
	bool operator>=(const V2D &p) const { return (x >= p.x) && (y >= p.y); }
	V2D operator+(const V2D &p) const { return V2D(_vm, x + p.x, y + p.y); }
	V2D operator-(const V2D &p) const { return V2D(_vm, x - p.x, y - p.y); }
	bool operator==(const V3D &p) const { V3D tmp(x, y); return tmp._x == p._x && tmp._y == p._y && tmp._z == p._z; }
	bool operator!=(const V3D &p) const { V3D tmp(x, y); return tmp._x != p._x || tmp._y != p._y || tmp._z == p._z; }
	bool operator==(const V2D &p) const { return x == p.x && y == p.y; }
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
		bool _hide;       // general visibility switch
		bool _drag;       // sprite is moveable
		bool _hold;       // sprite is held with mouse
		bool _trim;       // Trim flag
		bool _slav;       // slave object
		bool _kill;       // dispose memory after remove
		bool _xlat;       // 2nd way display: xlat table
		bool _port;       // portable
		bool _kept;       // kept in pocket
		bool _frnt;       // stay in front of sprite
		bool _east;       // talk to east (in opposite to west)
		bool _near;       // Near action lock
		bool _shad;       // shadow
		bool _back;       // 'send to background' request
		bool _zmov;       // sprite needs Z-update in queue
		bool _tran;       // transparent (untouchable)
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
		return _ext != nullptr;
	}
	Sprite(CGE2Engine *vm);
	Sprite(CGE2Engine *vm, BitmapPtr shp, int cnt);
	virtual ~Sprite();
	BitmapPtr getShp();
	void setShapeList(BitmapPtr shp, int cnt);
	void moveShapesHi();
	void moveShapesLo();
	int labVal(Action snq, int lab);
	virtual Sprite *expand();
	virtual Sprite *contract();
	void backShow();
	void setName(char *newName);
	inline char *name() {
		return (_ext) ? _ext->_name : nullptr;
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
	virtual void touch(uint16 mask, V2D pos, Common::KeyCode keyCode);
	virtual void tick();
	virtual void setScene(int c);
	void clrHide() { if (_ext) _ext->_b0 = nullptr; }

	void sync(Common::Serializer &s);

	static void (*notify) ();
};

class Queue {
	Sprite *_head;
	Sprite *_tail;
public:
	Queue(bool show);

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
	void clear() { _head = _tail = nullptr; }
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
	bool _mono;
	Graphics::Surface *_page[4];
	Dac *_sysPal;
	struct { uint8 _org, _len, _cnt, _dly; } _rot;

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
	void rotate();
	uint8 closest(Dac *pal, Dac x);

	void palToDac(const byte *palData, Dac *tab);
	void dacToPal(const Dac *tab, byte *palData);
};

class Speaker: public Sprite {
	CGE2Engine *_vm;
public:
	Speaker(CGE2Engine *vm);
};

} // End of namespace CGE2

#endif // CGE2_VGA13H_H
