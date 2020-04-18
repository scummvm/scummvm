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

#include "common/array.h"
#include "common/config-manager.h"
#include "common/rect.h"
#include "graphics/palette.h"
#include "cge2/general.h"
#include "cge2/vga13h.h"
#include "cge2/bitmap.h"
#include "cge2/text.h"
#include "cge2/cge2_main.h"
#include "cge2/cge2.h"
#include "cge2/vga13h.h"

namespace CGE2 {

void V3D::sync(Common::Serializer &s) {
	_x.sync(s);
	_y.sync(s);
	_z.sync(s);
}

FXP FXP::operator*(const FXP& x) const {
	FXP y;
	int32 t1 = (v >> 8) * x.v;
	int32 t2 = ((v & 0xFF) * x.v) >> 8;

	y.v = t1 + t2;
	return y;
}

FXP FXP::operator/(const FXP& x) const {
	FXP y;
	if (x.v != 0) {
		int32 v1 = this->v;
		int32 v2 = x.v;
		bool negFlag = false;

		if (v1 < 0) {
			v1 = -v1;
			negFlag = true;
		}
		if (v2 < 0) {
			v2 = -v2;
			negFlag ^= true;
		}

		int32 v3 = v1 / v2;
		v1 -= v3 * v2;
		v3 <<= 8;

		if (v1 < 0xFFFFFF)
			v1 <<= 8;
		else
			v2 >>= 8;

		v3 += v1 / v2;

		if (negFlag)
			v3 = -v3;

		y.v = v3;
	}

	return y;
}

void FXP::sync(Common::Serializer &s) {
	s.syncAsSint32LE(v);
}

Seq *getConstantSeq(bool seqFlag) {
	const Seq seq1[] = { { 0, 0, 0, 0, 0, 0 } };
	const Seq seq2[] = { { 0, 1, 0, 0, 0, 0 }, { 1, 0, 0, 0, 0, 0 } };

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

byte Sprite::_constY = 0;
byte Sprite::_follow = 0;

Seq Sprite::_stdSeq8[] =
{ { 0, 0, 0, 0, 0, 0 },
  { 1, 1, 0, 0, 0, 0 },
  { 2, 2, 0, 0, 0, 0 },
  { 3, 3, 0, 0, 0, 0 },
  { 4, 4, 0, 0, 0, 0 },
  { 5, 5, 0, 0, 0, 0 },
  { 6, 6, 0, 0, 0, 0 },
  { 7, 7, 0, 0, 0, 0 },
};

SprExt::SprExt(CGE2Engine *vm)
	: _p0(vm, 0, 0), _p1(vm, 0, 0),
     _b0(nullptr), _b1(nullptr), _shpList(nullptr),
     _location(0), _seq(nullptr), _name(nullptr) {
	for (int i = 0; i < kActions; i++)
		_actions[i] = nullptr;
}

Sprite::Sprite(CGE2Engine *vm)
	: _siz(_vm, 0, 0), _seqPtr(kNoSeq), _seqCnt(0), _shpCnt(0),
      _next(nullptr), _prev(nullptr), _time(0),
      _ext(nullptr), _ref(-1), _scene(0), _vm(vm),
      _pos2D(_vm, kScrWidth >> 1, 0), _pos3D(kScrWidth >> 1, 0, 0) {
	memset(_actionCtrl, 0, sizeof(_actionCtrl));
	memset(_file, 0, sizeof(_file));
	memset(&_flags, 0, sizeof(_flags));
	_flags._frnt = true;
}

Sprite::Sprite(CGE2Engine *vm, BitmapPtr shpP, int cnt)
	: _siz(_vm, 0, 0), _seqPtr(kNoSeq), _seqCnt(0), _shpCnt(0),
     _next(nullptr), _prev(nullptr), _time(0),
     _ext(nullptr), _ref(-1), _scene(0), _vm(vm),
     _pos2D(_vm, kScrWidth >> 1, 0), _pos3D(kScrWidth >> 1, 0, 0) {
	memset(_actionCtrl, 0, sizeof(_actionCtrl));
	memset(_file, 0, sizeof(_file));
	memset(&_flags, 0, sizeof(_flags));
	_flags._frnt = true;

	setShapeList(shpP, cnt);
}

Sprite::~Sprite() {
	contract();
}

BitmapPtr Sprite::getShp() {
	SprExt *e = _ext;
	if (!e || !e->_seq)
		return nullptr;

	int i = e->_seq[_seqPtr]._now;
	if (i >= _shpCnt)
		error("Invalid PHASE in SPRITE::Shp() %s - %d", _file, i);
	return e->_shpList + i;
}

void Sprite::setShapeList(BitmapPtr shp, int cnt) {
	_shpCnt = cnt;
	_siz.x = 0;
	_siz.y = 0;

	if (shp) {
		for (int i = 0; i < cnt; i++) {
			BitmapPtr p = shp + i;
			if (p->_w > _siz.x)
				_siz.x = p->_w;
			if (p->_h > _siz.y)
				_siz.y = p->_h;
		}
		expand();
		_ext->_shpList = shp;
		if (!_ext->_seq) {
			setSeq(_stdSeq8);
			_seqCnt = (cnt < ARRAYSIZE(_stdSeq8)) ? cnt : ARRAYSIZE(_stdSeq8);
		}
	}
}

Seq *Sprite::setSeq(Seq *seq) {
	expand();

	Seq *s = _ext->_seq;
	_ext->_seq = seq;
	if (_seqPtr == kNoSeq)
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

void Sprite::setName(char *newName) {
	if (!_ext)
		return;

	if (_ext->_name) {
		delete[] _ext->_name;
		_ext->_name = nullptr;
	}
	if (newName) {
		_ext->_name = new char[strlen(newName) + 1];
		strcpy(_ext->_name, newName);
	}
}

int Sprite::labVal(Action snq, int lab) {
	int lv = -1;
	if (active()) {
		int count = _actionCtrl[snq]._cnt;
		CommandHandler::Command *com = snList(snq);

		int i = 0;
		for (; i < count; i++) {
			if (com[i]._lab == lab)
				break;
		}

		if (i < count)
			return i;
	} else {
		char tmpStr[kLineMax + 1];
		_vm->mergeExt(tmpStr, _file, kSprExt);

		if (_vm->_resman->exist(tmpStr)) { // sprite description file exist
			EncryptedStream sprf(_vm, tmpStr);
			if (sprf.err())
				error("Bad SPR [%s]", tmpStr);

			int cnt = 0;
			ID section = kIdPhase;
			ID id;
			Common::String line;

			while (lv == -1 && !sprf.eos()) {
				line = sprf.readLine();
				if (line.empty())
					continue;

				Common::strlcpy(tmpStr, line.c_str(), sizeof(tmpStr));

				char *p;
				p = _vm->token(tmpStr);

				if (*p == '@') {
					if ((int)section == (int)snq && atoi(p + 1) == lab)
						lv = cnt;
				} else {
					id = _vm->ident(p);
					switch (id) {
					case kIdMTake:
					case kIdFTake:
					case kIdNear:
					case kIdPhase:
					case kIdSeq:
						section = id;
						break;
					default:
						if (id < 0 && (int)section == (int)snq)
							++cnt;
						break;
					}
				}
			}
		}
	}
	return lv;
}

CommandHandler::Command *Sprite::snList(Action type) {
	SprExt *e = _ext;
	return (e) ? e->_actions[type] : nullptr;
}

Sprite *Sprite::expand() {
	if (_ext)
		return this;

	if (_vm->_spriteNotify != nullptr)
		(_vm->*_vm->_spriteNotify)();

	char fname[kPathMax];
	_vm->mergeExt(fname, _file, kSprExt);

	if (_ext != nullptr)
		delete _ext;
	_ext = new SprExt(_vm);

	if (!*_file)
		return this;

	BitmapPtr shplist = new Bitmap[_shpCnt];

	int cnt[kActions],
		shpcnt = 0,
		seqcnt = 0,
		maxnow = 0,
		maxnxt = 0;

	for (int i = 0; i < kActions; i++)
		cnt[i] = 0;

	for (int i = 0; i < kActions; i++){
		byte n = _actionCtrl[i]._cnt;
		if (n)
			_ext->_actions[i] = new CommandHandler::Command[n];
		else
			_ext->_actions[i] = nullptr;
	}

	Seq *curSeq = nullptr;
	if (_seqCnt)
		curSeq = new Seq[_seqCnt];

	if (_vm->_resman->exist(fname)) { // sprite description file exist
		EncryptedStream sprf(_vm, fname);
		if (sprf.err())
			error("Bad SPR [%s]", fname);

		int label = kNoByte;
		ID section = kIdPhase;
		ID id;
		Common::String line;
		char tmpStr[kLineMax + 1];

		for (line = sprf.readLine(); !sprf.eos(); line = sprf.readLine()) {
			if (line.empty())
				continue;
			Common::strlcpy(tmpStr, line.c_str(), sizeof(tmpStr));

			char *p = _vm->token(tmpStr);
			if (*p == '@') {
				label = atoi(p + 1);
				continue;
			}

			id = _vm->ident(p);
			switch (id) {
			case kIdType:
				break;
			case kIdNear:
			case kIdMTake:
			case kIdFTake:
			case kIdPhase:
			case kIdSeq:
				section = id;
				break;
			case kIdName:
				Common::strlcpy(tmpStr, line.c_str(), sizeof(tmpStr));
				for (p = tmpStr; *p != '='; p++) // We search for the =
					;
				setName(_vm->tail(p));
				break;
			default:
				if (id >= kIdNear)
					break;
				Seq *s;
				switch (section) {
				case kIdNear:
				case kIdMTake:
				case kIdFTake:
					id = (ID)_vm->_commandHandler->getComId(p);
					if (_actionCtrl[section]._cnt) {
						CommandHandler::Command *c = &_ext->_actions[section][cnt[section]++];
						c->_commandType = CommandType(id);
						c->_lab = label;
						c->_ref = _vm->number(nullptr);
						c->_val = _vm->number(nullptr);
						c->_spritePtr = nullptr;
					}
					break;
				case kIdSeq:
					s = &curSeq[seqcnt++];
					s->_now = atoi(p);
					if (s->_now > maxnow)
						maxnow = s->_now;
					s->_next = _vm->number(nullptr);
					switch (s->_next) {
					case 0xFF:
						s->_next = seqcnt;
						break;
					case 0xFE:
						s->_next = seqcnt - 1;
						break;
					default:
						break;
					}
					if (s->_next > maxnxt)
						maxnxt = s->_next;
					s->_dx = _vm->number(nullptr);
					s->_dy = _vm->number(nullptr);
					s->_dz = _vm->number(nullptr);
					s->_dly = _vm->number(nullptr);
					break;
				case kIdPhase:
					shplist[shpcnt] = Bitmap(_vm, p);
					shpcnt++;
					break;
				default:
					break;
				}
				break;
			}
			label = kNoByte;
		}

		if (!shpcnt)
			error("No shapes - %s", fname);
	} else // no sprite description: try to read immediately from .BMP
		shplist[shpcnt++] = Bitmap(_vm, _file);

	if (curSeq) {
		if (maxnow >= shpcnt)
			error("Bad PHASE in SEQ %s", fname);
		if (maxnxt && (maxnxt >= seqcnt))
			error("Bad JUMP in SEQ %s", fname);
		setSeq(curSeq);
	} else {
		setSeq(_stdSeq8);
		_seqCnt = (shpcnt < ARRAYSIZE(_stdSeq8)) ? shpcnt : ARRAYSIZE(_stdSeq8);
	}

	setShapeList(shplist, shpcnt);

	if (_file[2] == '~') { // FLY-type sprite
		Seq *nextSeq = _ext->_seq;
		int x = (nextSeq + 1)->_dx, y = (nextSeq + 1)->_dy, z = (nextSeq + 1)->_dz;
		// random position
		nextSeq->_dx = _vm->newRandom(x + x) - x;
		nextSeq->_dy = _vm->newRandom(y + y) - y;
		nextSeq->_dz = _vm->newRandom(z + z) - z;
		gotoxyz(_pos3D + V3D(nextSeq->_dx, nextSeq->_dy, nextSeq->_dz));
	}

	return this;
}

Sprite *Sprite::contract() {
	SprExt *e = _ext;
	if (!e)
		return this;

	if (_file[2] == '~') { // FLY-type sprite
		Seq *curSeq = _ext->_seq;
		// return to middle
		gotoxyz(_pos3D - V3D(curSeq->_dx, curSeq->_dy, curSeq->_dz));
		curSeq->_dx = curSeq->_dy = curSeq->_dz = 0;
	}

	if (_vm->_spriteNotify != nullptr)
		(_vm->*_vm->_spriteNotify)();

	if (e->_name) {
		delete[] e->_name;
		e->_name = nullptr;
	}

	if (e->_shpList) {
		for (int i = 0; i < _shpCnt; i++)
			e->_shpList[i].release();
		delete[] e->_shpList;
		e->_shpList = nullptr;
	}

	if (e->_seq) {
		if (e->_seq == _stdSeq8)
			_seqCnt = 0;
		else {
			delete[] e->_seq;
			e->_seq = nullptr;
		}
	}

	for (int i = 0; i < kActions; i++) {
		if (e->_actions[i]) {
			delete[] e->_actions[i];
			e->_actions[i] = nullptr;
		}
	}

	delete _ext;
	_ext = nullptr;

	return this;
}

void Sprite::backShow() {
	expand();
	show(2);
	show(1);
	_vm->_spare->dispose(this);
}

void Sprite::step(int nr) {
	if (nr >= 0)
		_seqPtr = nr;

	if (_ext && _ext->_seq) {
		V3D p = _pos3D;
		Seq *seq = nullptr;

		if (nr < 0)
			_seqPtr = _ext->_seq[_seqPtr]._next;

		if (_file[2] == '~') { // FLY-type sprite
			seq = _ext->_seq;
			// return to middle
			p._x -= seq->_dx;
			p._y -= seq->_dy;
			p._z -= seq->_dz;
			// generate motion
			if (_vm->newRandom(10) < 5) {
				if ((seq + 1)->_dx)
					seq->_dx += _vm->newRandom(3) - 1;
				if ((seq + 1)->_dy)
					seq->_dy += _vm->newRandom(3) - 1;
				if ((seq + 1)->_dz)
					seq->_dz += _vm->newRandom(3) - 1;
			}
			if (seq->_dx < -(seq + 1)->_dx)
				seq->_dx += 2;
			if (seq->_dx >= (seq + 1)->_dx)
				seq->_dx -= 2;
			if (seq->_dy < -(seq + 1)->_dy)
				seq->_dy += 2;
			if (seq->_dy >= (seq + 1)->_dy)
				seq->_dy -= 2;
			if (seq->_dz < -(seq + 1)->_dz)
				seq->_dz += 2;
			if (seq->_dz >= (seq + 1)->_dz)
				seq->_dz -= 2;
			p._x += seq->_dx;
			p._y += seq->_dy;
			p._z += seq->_dz;
			gotoxyz(p);
		} else {
			seq = _ext->_seq + _seqPtr;
			if (seq) {
				if (seq->_dz == 127 && seq->_dx != 0) {
					_vm->_commandHandlerTurbo->addCommand(kCmdSound, -1, 256 * seq->_dy + seq->_dx, this);
				} else {
					p._x += seq->_dx;
					p._y += seq->_dy;
					p._z += seq->_dz;
					gotoxyz(p);
				}
			}
		}
		if (seq && (seq->_dly >= 0))
			_time = seq->_dly;
	} else if (_vm->_waitRef && _vm->_waitRef == _ref)
		_vm->_waitRef = 0;
}

void Sprite::tick() {
	step();
}

void Sprite::setScene(int c) {
	_scene = c;
}

void Sprite::gotoxyz(int x, int y, int z) {
	gotoxyz(V3D(x, y, z));
}

void Sprite::gotoxyz() {
	gotoxyz(_pos3D);
}

void Sprite::gotoxyz(V2D pos) {
	V2D o = _pos2D;
	int ctr = _siz.x >> 1;
	int rem = _siz.x - ctr;
	byte trim = 0;

	if (_ref / 10 == 14) { // HERO
		int z = _pos3D._z.trunc();
		ctr = (ctr * _vm->_eye->_z.trunc()) / (_vm->_eye->_z.trunc() - z);
		rem = (rem * _vm->_eye->_z.trunc()) / (_vm->_eye->_z.trunc() - z);
		ctr = (ctr * 3) / 4;
		rem = (rem * 3) / 4;
	}

	if (pos.x - ctr < 0) {
		pos.x = ctr;
		++trim;
	}
	if (pos.x + rem > kScrWidth) {
		pos.x = kScrWidth - rem;
		++trim;
	}
	_pos2D.x = pos.x;

	if (pos.y < -kPanHeight) {
		pos.y = -kPanHeight;
		++trim;
	}
	if (pos.y + _siz.y > kWorldHeight) {
		pos.y = kWorldHeight - _siz.y;
		++trim;
	}
	_pos2D.y = pos.y;

	_flags._trim = (trim != 0);

	if (!_follow) {
		FXP m = _vm->_eye->_z / (_pos3D._z - _vm->_eye->_z);
		_pos3D._x = (_vm->_eye->_x + (_vm->_eye->_x - _pos2D.x) / m);
		_pos3D._x = _pos3D._x.round();

		if (!_constY) {
			_pos3D._y = _vm->_eye->_y + (_vm->_eye->_y - _pos2D.y) / m;
			_pos3D._y = _pos3D._y.round();
		}
	}

	if (_next && _next->_flags._slav)
		_next->gotoxyz(_next->_pos2D - o + _pos2D);

	if (_flags._shad)
		_prev->gotoxyz(_prev->_pos2D - o + _pos2D);
}

void Sprite::gotoxyz_(V2D pos) {
	_constY++;
	gotoxyz(pos);
	--_constY;
}

void Sprite::gotoxyz(V3D pos) {
	_follow++;
	if (pos._z != _pos3D._z)
		_flags._zmov = true;
	gotoxyz(V2D(_vm, _pos3D = pos));
	--_follow;
}

void Sprite::center() {
	gotoxyz(kScrWidth >> 1, (kWorldHeight - _siz.y) >> 1, 0);
}

void Sprite::show() {
	SprExt *e = _ext;
	if (e) {
		e->_p0 = e->_p1;
		e->_b0 = e->_b1;
		e->_p1 = _pos2D;
		e->_b1 = getShp();

		if (!_flags._hide)
			e->_b1->show(e->_p1);
	}
}

void Sprite::show(uint16 pg) {
	assert(pg < 4);
	Graphics::Surface *a = _vm->_vga->_page[1];
	_vm->_vga->_page[1] = _vm->_vga->_page[pg];
	getShp()->show(_pos2D);
	_vm->_vga->_page[1] = a;
}

void Sprite::hide() {
	SprExt *e = _ext;
	if (e->_b0)
		e->_b0->hide(e->_p0);
}

BitmapPtr Sprite::ghost() {
	SprExt *e = _ext;
	if (!e->_b1)
		return nullptr;

	BitmapPtr bmp = new Bitmap(_vm, 0, 0, (uint8 *)nullptr);
	bmp->_w = e->_b1->_w;
	bmp->_h = e->_b1->_h;
	bmp->_b = new HideDesc[bmp->_h];
	memcpy(bmp->_b, e->_b1->_b, sizeof(HideDesc)* bmp->_h);
	uint8 *v = new uint8[1];
	*v = (e->_p1.y << 16) + e->_p1.x;
	bmp->_v = v;
	bmp->_map = (e->_p1.y << 16) + e->_p1.x;

	return bmp;
}

void Sprite::sync(Common::Serializer &s) {
	s.syncAsUint16LE(_ref);
	s.syncAsByte(_scene);

	// bitfield in-memory storage is unpredictable, so to avoid
	// any issues, pack/unpack everything manually
	uint16 flags = 0;
	if (s.isLoading()) {
		s.syncAsUint16LE(flags);
		_flags._hide = flags & 0x0001;
		_flags._drag = flags & 0x0002;
		_flags._hold = flags & 0x0004;
		_flags._trim = flags & 0x0008;
		_flags._slav = flags & 0x0010;
		_flags._kill = flags & 0x0020;
		_flags._xlat = flags & 0x0040;
		_flags._port = flags & 0x0080;
		_flags._kept = flags & 0x0100;
		_flags._frnt = flags & 0x0200;
		_flags._east = flags & 0x0400;
		_flags._near = flags & 0x0800;
		_flags._shad = flags & 0x1000;
		_flags._back = flags & 0x2000;
		_flags._zmov = flags & 0x4000;
		_flags._tran = flags & 0x8000;
	} else {
		flags = (flags << 1) | (_flags._tran ? 1 : 0);
		flags = (flags << 1) | (_flags._zmov ? 1 : 0);
		flags = (flags << 1) | (_flags._back ? 1 : 0);
		flags = (flags << 1) | (_flags._shad ? 1 : 0);
		flags = (flags << 1) | (_flags._near ? 1 : 0);
		flags = (flags << 1) | (_flags._east ? 1 : 0);
		flags = (flags << 1) | (_flags._frnt ? 1 : 0);
		flags = (flags << 1) | (_flags._kept ? 1 : 0);
		flags = (flags << 1) | (_flags._port ? 1 : 0);
		flags = (flags << 1) | (_flags._xlat ? 1 : 0);
		flags = (flags << 1) | (_flags._kill ? 1 : 0);
		flags = (flags << 1) | (_flags._slav ? 1 : 0);
		flags = (flags << 1) | (_flags._trim ? 1 : 0);
		flags = (flags << 1) | (_flags._hold ? 1 : 0);
		flags = (flags << 1) | (_flags._drag ? 1 : 0);
		flags = (flags << 1) | (_flags._hide ? 1 : 0);
		s.syncAsUint16LE(flags);
	}

	s.syncAsSint16LE(_pos2D.x);
	s.syncAsSint16LE(_pos2D.y);

	_pos3D.sync(s);

	s.syncAsSint16LE(_siz.x);
	s.syncAsSint16LE(_siz.y);

	s.syncAsUint16LE(_time);
	for (int i = 0; i < kActions; i++){
		s.syncAsByte(_actionCtrl[i]._ptr);
		s.syncAsByte(_actionCtrl[i]._cnt);
	}
	s.syncAsSint16LE(_seqPtr);
	s.syncAsSint16LE(_seqCnt);
	s.syncAsUint16LE(_shpCnt);
	s.syncBytes((byte *)&_file[0], 9);
	_file[8] = '\0';
}

Queue::Queue(bool show) : _head(nullptr), _tail(nullptr) {
}

void Queue::append(Sprite *spr) {
	if (spr->_flags._back)
		spr->backShow();
	else {
		spr->expand();
		if (_tail) {
			spr->_prev = _tail;
			_tail->_next = spr;
		} else
			_head = spr;

		_tail = spr;
	}
}

void Queue::insert(Sprite *spr, Sprite *nxt) {
	if (spr->_flags._back)
		spr->backShow();
	else {
		spr->expand();
		if (nxt == _head) {
			spr->_next = _head;
			_head = spr;
			if (!_tail)
				_tail = spr;
		} else {
			spr->_next = nxt;
			spr->_prev = nxt->_prev;
			if (spr->_prev)
				spr->_prev->_next = spr;
		}
		if (spr->_next)
			spr->_next->_prev = spr;
	}
}

void Queue::insert(Sprite *spr) {
	if (locate(spr))
		return; // We only queue it if it's not already queued.

	Sprite *s;
	for (s = _head; s; s = s->_next) {
		if (s->_pos3D._z < spr->_pos3D._z)
			break;
	}

	if (s)
		insert(spr, s);
	else
		append(spr);
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

	spr->_prev = nullptr;
	spr->_next = nullptr;
	return spr;
}

Sprite *Queue::locate(int ref) {
	for (Sprite *spr = _head; spr; spr = spr->_next) {
		if (spr->_ref == ref)
			return spr;
	}
	return nullptr;
}

bool Queue::locate(Sprite *spr) {
	Sprite *s;
	for (s = _head; s; s = s->_next) {
		if (s == spr)
			return true;
	}

	return false;
}

Vga::Vga(CGE2Engine *vm) : _frmCnt(0), _msg(nullptr), _name(nullptr), _setPal(false), _vm(vm) {
	_rot._org = 1;
	_rot._len = 0;
	_rot._cnt = 0;
	_rot._dly = 1;

	_oldColors = nullptr;
	_newColors = nullptr;
	_showQ = new Queue(true);
	_sysPal = new Dac[kPalCount];

	for (int idx = 0; idx < 4; idx++) {
		_page[idx] = new Graphics::Surface();
		_page[idx]->create(kScrWidth, kScrHeight, Graphics::PixelFormat::createFormatCLUT8());
	}

	_mono = ConfMan.getBool("enable_color_blind");

	_oldColors = (Dac *)malloc(sizeof(Dac) * kPalCount);
	_newColors = (Dac *)malloc(sizeof(Dac) * kPalCount);
	getColors(_oldColors);
	sunset();
	setColors();
	clear(0);
}

Vga::~Vga() {
	Common::String buffer = "";

	free(_oldColors);
	free(_newColors);
	if (_msg)
		buffer = Common::String(_msg);

	if (_name)
		buffer = buffer + " [" + _name + "]";

	debugN("%s", buffer.c_str());

	delete _showQ;
	delete[] _sysPal;

	for (int idx = 0; idx < 4; idx++) {
		_page[idx]->free();
		delete _page[idx];
	}
}

void Vga::waitVR() {
	// Since some of the game parts rely on using vertical sync as a delay mechanism,
	// we're introducing a short delay to simulate it
	g_system->delayMillis(5);
}

void Vga::getColors(Dac *tab) {
	byte palData[kPalSize];
	g_system->getPaletteManager()->grabPalette(palData, 0, kPalCount);
	palToDac(palData, tab);
}

uint8 Vga::closest(Dac *pal, const uint8 colR, const uint8 colG, const uint8 colB) {
#define f(col, lum) ((((uint16)(col)) << 8) / lum)
	uint16 i, dif = 0xFFFF, found = 0;
	uint16 L = colR + colG + colB;
	if (!L)
		L++;
	uint16 R = f(colR, L), G = f(colG, L), B = f(colB, L);
	for (i = 0; i < 256; i++) {
		uint16 l = pal[i]._r + pal[i]._g + pal[i]._b;
		if (!l)
			l++;
		int  r = f(pal[i]._r, l), g = f(pal[i]._g, l), b = f(pal[i]._b, l);
		uint16 D = ((r > R) ? (r - R) : (R - r)) +
		           ((g > G) ? (g - G) : (G - g)) +
		           ((b > B) ? (b - B) : (B - b)) +
		           ((l > L) ? (l - L) : (L - l)) * 10;

		if (D < dif) {
			found = i;
			dif = D;
			if (D == 0)
				break; // exact!
		}
	}
	return found;
#undef f
}

uint8 Vga::closest(Dac *pal, Dac x) {
	long D = 0;
	D = ~D;
	D = (unsigned long)D >> 1; // Maximum value of long.
	long R = x._r;
	long G = x._g;
	long B = x._b;
	int idx = 255;
	for (int n = 0; n < 256; n++) {
		long dR = R - pal[n]._r;
		long dG = G - pal[n]._g;
		long dB = B - pal[n]._b,
			d = dR * dR + dG * dG + dB * dB;
		if (d < D) {
			idx = n;
			D = d;
			if (!d)
				break;
		}
	}
	return idx;
}

uint8 *Vga::glass(Dac *pal, const uint8 colR, const uint8 colG, const uint8 colB) {
	uint8 *x = (uint8 *)malloc(256);
	if (x) {
		for (uint16 i = 0; i < 256; i++) {
			x[i] = closest(pal, ((uint16)(pal[i]._r) * colR) / 255,
			                    ((uint16)(pal[i]._g) * colG) / 255,
			                    ((uint16)(pal[i]._b) * colB) / 255);
		}
	}
	return x;
}

void Vga::palToDac(const byte *palData, Dac *tab) {
	const byte *colP = palData;
	for (int idx = 0; idx < kPalCount; idx++, colP += 3) {
		tab[idx]._r = *colP >> 2;
		tab[idx]._g = *(colP + 1) >> 2;
		tab[idx]._b = *(colP + 2) >> 2;
	}
}

void Vga::dacToPal(const Dac *tab, byte *palData) {
	for (int idx = 0; idx < kPalCount; idx++, palData += 3) {
		*palData = tab[idx]._r << 2;
		*(palData + 1) = tab[idx]._g << 2;
		*(palData + 2) = tab[idx]._b << 2;
	}
}

void Vga::setColors(Dac *tab, int lum) {
	Dac *palP = tab, *destP = _newColors;
	for (int idx = 0; idx < kPalCount; idx++, palP++, destP++) {
		destP->_r = (palP->_r * lum) >> 6;
		destP->_g = (palP->_g * lum) >> 6;
		destP->_b = (palP->_b * lum) >> 6;
	}

	if (_mono) {
		destP = _newColors;
		for (int idx = 0; idx < kPalCount; idx++, destP++) {
			// Form a grayscale color from 30% R, 59% G, 11% B
			uint8 intensity = (((int)destP->_r * 77) + ((int)destP->_g * 151) + ((int)destP->_b * 28)) >> 8;
			destP->_r = intensity;
			destP->_g = intensity;
			destP->_b = intensity;
		}
	}

	_setPal = true;
}

void Vga::setColors() {
	memset(_newColors, 0, kPalSize);
	updateColors();
}

void Vga::sunrise(Dac *tab) {
	for (int i = 0; i <= 64; i += kFadeStep) {
		setColors(tab, i);
		waitVR();
		updateColors();
		g_system->updateScreen();
	}
}

void Vga::sunset() {
	Dac tab[256];
	getColors(tab);
	for (int i = 64; i >= 0; i -= kFadeStep) {
		setColors(tab, i);
		waitVR();
		updateColors();
		g_system->updateScreen();
	}
}

void Vga::show() {
	_vm->_infoLine->update();

	for (Sprite *spr = _showQ->first(); spr; spr = spr->_next) {
		spr->show();
	}

	_vm->_mouse->show();
	update();
	rotate();

	for (Sprite *spr = _showQ->first(); spr; spr = spr->_next) {
		spr->hide();
		if (spr->_flags._zmov) {
			Sprite *s = nullptr;
			Sprite *p = spr->_prev;
			Sprite *n = spr->_next;

			if (spr->_flags._shad) {
				s = p;
				p = s->_prev;
			}

			if ((p && spr->_pos3D._z > p->_pos3D._z) ||
				(n && spr->_pos3D._z < n->_pos3D._z)) {
				_showQ->insert(_showQ->remove(spr));
			}
			spr->_flags._zmov = false;
		}
	}
	_vm->_mouse->hide();
}

void Vga::updateColors() {
	byte palData[kPalSize];
	dacToPal(_newColors, palData);
	g_system->getPaletteManager()->setPalette(palData, 0, 256);
}

void Vga::update() {
	SWAP(Vga::_page[0], Vga::_page[1]);

	if (_setPal) {
		updateColors();
		_setPal = false;
	}

	g_system->copyRectToScreen(Vga::_page[0]->getPixels(), kScrWidth, 0, 0, kScrWidth, kScrHeight);
	g_system->updateScreen();
}

void Vga::rotate() {
	if (_rot._len) {
		Dac c;
		getColors(_newColors);
		c = _newColors[_rot._org];
		memmove(_newColors + _rot._org, _newColors + _rot._org + 1, (_rot._len - 1) * sizeof(Dac));
		_newColors[_rot._org + _rot._len - 1] = c;
		_setPal = true;
	}
}

void Vga::clear(uint8 color) {
	for (int paneNum = 0; paneNum < 4; paneNum++)
		_page[paneNum]->fillRect(Common::Rect(0, 0, kScrWidth, kScrHeight), color);
}

void Vga::copyPage(uint16 d, uint16 s) {
	_page[d]->copyFrom(*_page[s]);
}

void Bitmap::show(V2D pos) {
	xLatPos(pos);

	const byte *srcP = (const byte *)_v;
	byte *screenStartP = (byte *)_vm->_vga->_page[1]->getPixels();
	byte *screenEndP = (byte *)_vm->_vga->_page[1]->getBasePtr(0, kScrHeight);

	// Loop through processing data for each plane. The game originally ran in plane mapped mode, where a
	// given plane holds each fourth pixel sequentially. So to handle an entire picture, each plane's data
	// must be decompressed and inserted into the surface
	for (int planeCtr = 0; planeCtr < 4; planeCtr++) {
		byte *destP = (byte *)_vm->_vga->_page[1]->getBasePtr(pos.x + planeCtr, pos.y);

		for (;;) {
			uint16 v = READ_LE_UINT16(srcP);
			srcP += 2;
			int cmd = v >> 14;
			int count = v & 0x3FFF;

			if (cmd == 0) {
				// End of image
				break;
			}

			// Handle a set of pixels
			while (count-- > 0) {
				// Transfer operation
				switch (cmd) {
				default:
				case 1:
					// SKIP
					break;
				case 2:
					// REPEAT
					if (destP >= screenStartP && destP < screenEndP)
						*destP = *srcP;
					break;
				case 3:
					// COPY
					if (destP >= screenStartP && destP < screenEndP)
						*destP = *srcP;
					srcP++;
					break;
				}

				// Move to next dest position
				destP += 4;
			}

			if (cmd == 2)
				srcP++;
		}
	}
}

void Bitmap::hide(V2D pos) {
	xLatPos(pos);

	// Perform clipping to screen
	int w = MIN<int>(_w, kScrWidth - pos.x);
	int h = MIN<int>(_h, kScrHeight - pos.y);
	if (pos.x < 0) {
		w -= -pos.x;
		pos.x = 0;
		if (w < 0)
			return;
	}
	if (pos.y < 0) {
		h -= -pos.y;
		pos.y = 0;
		if (h < 0)
			return;
	}

	// Perform copying of screen section
	for (int yp = pos.y; yp < pos.y + h; yp++) {
		if (yp >= 0 && yp < kScrHeight) {
			const byte *srcP = (const byte *)_vm->_vga->_page[2]->getBasePtr(pos.x, yp);
			byte *destP = (byte *)_vm->_vga->_page[1]->getBasePtr(pos.x, yp);

			Common::copy(srcP, srcP + w, destP);
		}
	}
}

Speaker::Speaker(CGE2Engine *vm): Sprite(vm), _vm(vm) {
	// Set the sprite list
	BitmapPtr SP = new Bitmap[2];
	uint8 *map = Bitmap::makeSpeechBubbleTail(0, _vm->_font->_colorSet);
	SP[0] = Bitmap(_vm, 15, 16, map);
	delete[] map;
	map = Bitmap::makeSpeechBubbleTail(1, _vm->_font->_colorSet);
	SP[1] = Bitmap(_vm, 15, 16, map);
	delete[] map;
	setShapeList(SP, 2);
}

} // End of namespace CGE2
