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

void(*Sprite::notify) (void) = nullptr;

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
     _b0(NULL), _b1(NULL), _shpList(NULL),
	 _location(0), _seq(NULL), _name(NULL) {
	for (int i = 0; i < kActions; i++)
		_actions[i] = nullptr;
}

Sprite::Sprite(CGE2Engine *vm)
	: _siz(_vm, 0, 0), _seqPtr(kNoSeq), _seqCnt(0), _shpCnt(0),
      _next(NULL), _prev(NULL), _time(0),
	  _ext(NULL), _ref(-1), _scene(0), _vm(vm),
	  _pos2D(_vm, kScrWidth >> 1, 0), _pos3D(kScrWidth >> 1, 0, 0) {
	memset(_actionCtrl, 0, sizeof(_actionCtrl));
	memset(_file, 0, sizeof(_file));
	memset(&_flags, 0, sizeof(_flags));
	_flags._frnt = 1;
}

Sprite::Sprite(CGE2Engine *vm, BitmapPtr *shpP, int cnt)
	: _siz(_vm, 0, 0), _seqPtr(kNoSeq), _seqCnt(0), _shpCnt(0),
     _next(NULL), _prev(NULL), _time(0),
     _ext(NULL), _ref(-1), _scene(0), _vm(vm),
     _pos2D(_vm, kScrWidth >> 1, 0), _pos3D(kScrWidth >> 1, 0, 0) {
	memset(_actionCtrl, 0, sizeof(_actionCtrl));
	memset(_file, 0, sizeof(_file));
	memset(&_flags, 0, sizeof(_flags));
	_flags._frnt = 1;

	setShapeList(shpP, cnt);
}

Sprite::~Sprite() {
	if (_vm->_sprite == this)
		_vm->_sprite = NULL;

	contract();
}

BitmapPtr Sprite::shp() {
	SprExt *e = _ext;
	if (!e || !e->_seq)
		return NULL;

	int i = e->_seq[_seqPtr]._now;
	if (i >= _shpCnt)
		error("Invalid PHASE in SPRITE::Shp() %s", _file);
	return e->_shpList[i];
}

void Sprite::setShapeList(BitmapPtr *shp, int cnt) {
	_shpCnt = cnt;
	_siz.x = 0;
	_siz.y = 0;

	if (shp) {
		for (int i = 0; i < cnt; i++) {
			BitmapPtr p = *shp + i;
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

bool Sprite::works(Sprite *spr) {
	//if (!spr || !spr->_ext)
	//	return false;

	//CommandHandler::Command *c = spr->_ext->_take;
	//if (c != NULL) {
	//	c += spr->_takePtr;
	//	if (c->_ref == _ref)
	//		if (c->_commandType != kCmdLabel || (c->_val == 0 || c->_val == _vm->_now))
	//			return true;
	//}

	warning("STUB: Sprite::works()");

	return false;
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
		_ext->_name = NULL;
	}
	if (newName) {
		_ext->_name = new char[strlen(newName) + 1];
		assert(_ext->_name != NULL);
		strcpy(_ext->_name, newName);
	}
}

int Sprite::labVal(Action snq, int lab) {
	warning("STUB: Sprite::labVal()");
	return 0;
}

CommandHandler::Command *Sprite::snList(Action type) {
	SprExt *e = _ext;
	return (e) ? e->_actions[type] : NULL;
}

Sprite *Sprite::expand() {
	if (_ext)
		return this;

	if (notify != nullptr)
		notify();

	_ext = new SprExt(_vm);
	assert(_ext != NULL);

	if (!*_file)
		return this;

	Common::Array<BitmapPtr> shplist;
	for (int i = 0; i < _shpCnt; ++i)
		shplist.push_back(NULL);

	int cnt[kActions],
		shpcnt = 0,
		seqcnt = 0,
		maxnow = 0,
		maxnxt = 0;

	char fname[kPathMax];
	_vm->mergeExt(fname, _file, kSprExt);

	Seq *seq;
	if (_seqCnt) {
		seq = new Seq[_seqCnt];
		if (seq == NULL)
			error("No core %s", fname);
	} else
		seq = nullptr;

	for (int i = 0; i < kActions; i++)
		cnt[i] = 0;

	for (int i = 0; i < kActions; i++){
		byte n = _actionCtrl[i]._cnt;
		if (n) {
			_ext->_actions[i] = new CommandHandler::Command[n];
			if (_ext->_actions[i] == nullptr)
				error("No core %s", fname);
		} else
			_ext->_actions[i] = nullptr;
	}

	if (_vm->_resman->exist(fname)) { // sprite description file exist
		EncryptedStream sprf(_vm, fname);
		if (sprf.err())
			error("Bad SPR [%s]", fname);

		int label = kNoByte;
		ID section = kIdPhase;
		ID id;
		Common::String line;
		char tmpStr[kLineMax + 1];

		for (line = sprf.readLine(); !sprf.eos(); line = sprf.readLine()){
			if (line.size() == 0)
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
				for (p = tmpStr; *p != '='; p++); // We search for the =
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
					id = (ID)_vm->_commandHandler->com(p);
					if (_actionCtrl[section]._cnt) {
						CommandHandler::Command *c = &_ext->_actions[section][cnt[section]++];
						c->_commandType = CommandType(id);
						c->_lab = label;
						if ((p = _vm->token(nullptr)) == NULL)
							error("Unexpected end of file! %s", fname);
						c->_ref = _vm->number(p);
						if ((p = _vm->token(nullptr)) == NULL)
							error("Unexpected end of file! %s", fname);
						c->_val = _vm->number(p);
						c->_spritePtr = nullptr;
					}
					break;
				case kIdSeq:
					s = &seq[seqcnt++];
					s->_now = atoi(p);
					if (s->_now > maxnow)
						maxnow = s->_now;
					if ((p = _vm->token(nullptr)) == NULL)
						break;
					s->_next = _vm->number(p);
					switch (s->_next) {
					case 0xFF:
						s->_next = seqcnt;
						break;
					case 0xFE:
						s->_next = seqcnt - 1;
						break;
					}
					if (s->_next > maxnxt)
						maxnxt = s->_next;
					if ((p = _vm->token(nullptr)) == NULL)
						error("Unexpected end of file! %s", fname);
					s->_dx = _vm->number(p);
					if ((p = _vm->token(nullptr)) == NULL)
						error("Unexpected end of file! %s", fname);
					s->_dy = _vm->number(p);
					if ((p = _vm->token(nullptr)) == NULL)
						error("Unexpected end of file! %s", fname);
					s->_dz = _vm->number(p);
					if ((p = _vm->token(nullptr)) == NULL)
						error("Unexpected end of file! %s", fname);
					s->_dly = _vm->number(p);
					break;
				case kIdPhase:
					BitmapPtr bmp = new Bitmap(_vm, p);
					shplist[shpcnt] = bmp;
					if (!shplist[shpcnt]->moveHi())
						error("No EMS");
					shpcnt++;
					break;
				}
				break;
			}
			label = kNoByte;
		}

		if (!shpcnt)
			error("No shapes - %s", fname);
		} else // no sprite description: try to read immediately from .BMP
		shplist[shpcnt++] = new Bitmap (_vm, _file);

	if (seq) {
		if (maxnow >= shpcnt)
			error("Bad PHASE in SEQ %s", fname);
		if (maxnxt && maxnxt >= seqcnt)
			error("Bad JUMP in SEQ %s", fname);
		setSeq(seq);
	} else {
		setSeq(_stdSeq8);
		_seqCnt = (shpcnt < ARRAYSIZE(_stdSeq8)) ? shpcnt : ARRAYSIZE(_stdSeq8);
	}

	// Set the shape list
	BitmapPtr *shapeList = new BitmapPtr[shplist.size()];
	for (uint i = 0; i < shplist.size(); ++i)
		shapeList[i] = shplist[i];

	setShapeList(shapeList, shpcnt);

	if (_file[2] == '~') { // FLY-type sprite
		Seq *seq = _ext->_seq;
		int x = (seq + 1)->_dx, y = (seq + 1)->_dy, z = (seq + 1)->_dz;
		// random position
		seq->_dx = _vm->newRandom(x + x) - x;
		seq->_dy = _vm->newRandom(y + y) - y;
		seq->_dz = _vm->newRandom(z + z) - z;
		gotoxyz(_pos3D + V3D(seq->_dx, seq->_dy, seq->_dz));
	}

	return this;
}

Sprite *Sprite::contract() {
	SprExt *e = _ext;
	if (!e)
		return this;

	if (_file[2] == '~') { // FLY-type sprite
		Seq *seq = _ext->_seq;
		// return to middle
		gotoxyz(_pos3D - V3D(seq->_dx, seq->_dy, seq->_dz));
		seq->_dx = seq->_dy = seq->_dz = 0;
	}

	if (notify)
		notify();

	if (e->_name)
		delete[] e->_name;

	if (e->_shpList) {
		for (int i = 0; i < _shpCnt; i++)
			e->_shpList[i]->release();
		delete[] e->_shpList;
	}

	if (e->_seq) {
		if (e->_seq == _stdSeq8)
			_seqCnt = 0;
		else
			delete[] e->_seq;
	}

	for (int i = 0; i < kActions; i++)
		if (e->_actions[i])
			delete[] e->_actions[i];

	_ext = nullptr;

	return this;
}

void Sprite::backShow(void) {
	expand();
	show(2);
	show(1);
	_vm->_spare->dispose(this);
}

void Sprite::step(int nr) {
	if (nr >= 0)
		_seqPtr = nr;

	if (_ext) {
		V3D p = _pos3D;
		Seq *seq = nullptr;

		if (nr < 0)
			_seqPtr = _ext->_seq[_seqPtr]._next;

		if (_file[2] == '~') { // FLY-type sprite
			warning("STUB: Sprite::step() - FLY-type sprite");
		} else {
			seq = _ext->_seq + _seqPtr;
			if (seq->_dz == 127 && seq->_dx != 0) {
				_vm->_commandHandlerTurbo->addCommand(kCmdSound, -1, 256 * seq->_dy + seq->_dx, this);
			} else {
				p._x += seq->_dx;
				p._y += seq->_dy;
				p._z += seq->_dz;
				//if (!_flags._kept)
				//	gotoxyz(p);
				warning("STUB: Sprite::step()");
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

void Sprite::makeXlat(uint8 *x) {
	if (!_ext)
		return;

	if (_flags._xlat)
		killXlat();
	for (BitmapPtr *b = _ext->_shpList; *b; b++)
		(*b)->_m = x;
	_flags._xlat = true;
}

void Sprite::killXlat() {
	if (!_flags._xlat || !_ext)
		return;

	uint8 *m = (*_ext->_shpList)->_m;
	free(m);

	for (BitmapPtr *b = _ext->_shpList; *b; b++)
		(*b)->_m = NULL;
	_flags._xlat = false;
}

void Sprite::gotoxyz(int x, int y, int z) {
	gotoxyz(V3D(x, y, z));
}

void Sprite::gotoxyz(void) {
	gotoxyz(_pos3D);
}

void Sprite::gotoxyz(V2D pos) {
	V2D o = _pos2D;
	int ctr = _siz.x >> 1;
	int rem = _siz.x - ctr;
	byte trim = 0;

	if (_ref / 10 == 14) { // HERO
		int z = V2D::trunc(_pos3D._z);
		ctr = (ctr * V2D::trunc(_vm->_eye->_z) / (V2D::trunc(_vm->_eye->_z) - z));
		rem = (rem * V2D::trunc(_vm->_eye->_z) / (V2D::trunc(_vm->_eye->_z) - z));
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
		double m = _vm->_eye->_z / (_pos3D._z - _vm->_eye->_z);
		_pos3D._x = (_vm->_eye->_x + V2D::round(_vm->_eye->_x - _pos2D.x) / m);
		if (!_constY)
			_pos3D._y = (_vm->_eye->_y + V2D::round(_vm->_eye->_y - _pos2D.y) / m);
	}

	if (_next && _next->_flags._slav)
		_next->gotoxyz(_next->_pos2D - o + _pos2D);

	if (_flags._shad)
		_prev->gotoxyz(_prev->_pos2D - o + _pos2D);

	if (_ref == 141 && _pos3D._y >= 5)
		warning("Sprite::gotoxyz - asm nop");
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
		e->_b1 = shp();
	}
	if (!_flags._hide)
		e->_b1->show(e->_p1.x, e->_p1.y);
}

void Sprite::show(uint16 pg) {
	Graphics::Surface *a = _vm->_vga->_page[1];
	_vm->_vga->_page[1] = _vm->_vga->_page[pg];
	shp()->show(_pos2D.x, _pos2D.y);
	_vm->_vga->_page[1] = a;
}

void Sprite::hide() {
	SprExt *e = _ext;
	if (e->_b0)
		e->_b0->hide(e->_p0.x, e->_p0.y);
}

BitmapPtr Sprite::ghost() {
	SprExt *e = _ext;
	if (!e->_b1)
		return nullptr;

	BitmapPtr bmp = new Bitmap(_vm, 0, 0, (uint8 *)nullptr);
	assert(bmp != nullptr);
	bmp->_w = e->_b1->_w;
	bmp->_h = e->_b1->_h;
	bmp->_b = new HideDesc[bmp->_h];
	assert(bmp->_b != nullptr);
	memcpy(bmp->_b, e->_b1->_b, sizeof(HideDesc)* bmp->_h);
	uint8 *v = new uint8;
	*v = (e->_p1.y << 16) + e->_p1.x;
	bmp->_v = v;

	return bmp;
}

void Sprite::sync(Common::Serializer &s) {
	uint16 unused = 0;

	s.syncAsUint16LE(unused);
	s.syncAsUint16LE(unused);	// _ext
	s.syncAsUint16LE(_ref);
	s.syncAsByte(_scene);

	// bitfield in-memory storage is unpredictable, so to avoid
	// any issues, pack/unpack everything manually
	uint16 flags = 0;
	if (s.isLoading()) {
		s.syncAsUint16LE(flags);
		_flags._hide = flags & 0x0001 ? true : false;
		_flags._near = flags & 0x0002 ? true : false;
		_flags._drag = flags & 0x0004 ? true : false;
		_flags._hold = flags & 0x0008 ? true : false;
		//_flags._dummy = flags & 0x0010 ? true : false;
		_flags._slav = flags & 0x0020 ? true : false;
		//_flags._syst = flags & 0x0040 ? true : false;
		_flags._kill = flags & 0x0080 ? true : false;
		_flags._xlat = flags & 0x0100 ? true : false;
		_flags._port = flags & 0x0200 ? true : false;
		_flags._kept = flags & 0x0400 ? true : false;
		_flags._east = flags & 0x0800 ? true : false;
		_flags._shad = flags & 0x1000 ? true : false;
		_flags._back = flags & 0x2000 ? true : false;
		//_flags._bDel = flags & 0x4000 ? true : false;
		_flags._tran = flags & 0x8000 ? true : false;
	} else {
		flags = (flags << 1) | _flags._tran;
		//flags = (flags << 1) | _flags._bDel;
		flags = (flags << 1) | _flags._back;
		flags = (flags << 1) | _flags._shad;
		flags = (flags << 1) | _flags._east;
		flags = (flags << 1) | _flags._kept;
		flags = (flags << 1) | _flags._port;
		flags = (flags << 1) | _flags._xlat;
		flags = (flags << 1) | _flags._kill;
		//flags = (flags << 1) | _flags._syst;
		flags = (flags << 1) | _flags._slav;
		//flags = (flags << 1) | _flags._dummy;
		flags = (flags << 1) | _flags._hold;
		flags = (flags << 1) | _flags._drag;
		flags = (flags << 1) | _flags._near;
		flags = (flags << 1) | _flags._hide;
		s.syncAsUint16LE(flags);
	}

	warning("STUB: Sprite::sync() - Flags changed compared to CGE1's Sprite type.");

	s.syncAsUint16LE(_pos3D._x);
	s.syncAsUint16LE(_pos3D._y);
	s.syncAsByte(_pos3D._z);
	s.syncAsUint16LE(_time);
	s.syncAsSint16LE(_seqPtr);
	s.syncAsUint16LE(_shpCnt);
	s.syncBytes((byte *)&_file[0], 9);
	_file[8] = '\0';

	s.syncAsUint16LE(unused);	// _prev
	s.syncAsUint16LE(unused);	// _next
}

Queue::Queue(bool show) : _head(NULL), _tail(NULL) {
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
	Sprite *s;
	for (s = _head; s; s = s->_next)
		if (s->_pos3D._z < spr->_pos3D._z)
			break;

	if (s)
		insert(spr, s);
	else
		append(spr);
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
	for (Sprite *spr = _head; spr; spr = spr->_next) {
		if (spr->_ref == ref)
			return spr;
	}
	return NULL;
}

bool Queue::locate(Sprite *spr) {
	Sprite *s;
	for (s = _head; s; s = s->_next)
		if (s == spr)
			return true;
	return false;
}

Vga::Vga(CGE2Engine *vm) : _frmCnt(0), _msg(NULL), _name(NULL), _setPal(false), _mono(0), _vm(vm) {
	_oldColors = NULL;
	_newColors = NULL;
	_showQ = new Queue(true);
	_spareQ = new Queue(false);
	_sysPal = new Dac[kPalCount];

	for (int idx = 0; idx < 4; idx++) {
		_page[idx] = new Graphics::Surface();
		_page[idx]->create(kScrWidth, kScrHeight, Graphics::PixelFormat::createFormatCLUT8());
	}

	/*if (ConfMan.getBool("enable_color_blind"))
		_mono = 1;*/
	
	warning("STUB: Vga::Vga()");

	_oldColors = (Dac *)malloc(sizeof(Dac) * kPalCount);
	_newColors = (Dac *)malloc(sizeof(Dac) * kPalCount);
	getColors(_oldColors);
	sunset();
	setColors();
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
	warning("STUB: Vga::~Vga()");

	free(_oldColors);
	free(_newColors);
	if (_msg)
		buffer = Common::String(_msg);
	if (_name)
		buffer = buffer + " [" + _name + "]";

	debugN("%s", buffer.c_str());

	delete _showQ;
	delete _spareQ;
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

uint8 *Vga::glass(Dac *pal, const uint8 colR, const uint8 colG, const uint8 colB) {
	uint8 *x = (uint8 *)malloc(256);
	if (x) {
		uint16 i;
		for (i = 0; i < 256; i++) {
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
			// Form a greyscalce color from 30% R, 59% G, 11% B
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
	}
}

void Vga::sunset() {
	Dac tab[256];
	getColors(tab);
	for (int i = 64; i >= 0; i -= kFadeStep) {
		setColors(tab, i);
		waitVR();
		updateColors();
	}
}

void Vga::show() {
	for (Sprite *spr = _showQ->first(); spr; spr = spr->_next)
		spr->show();
	update();
	for (Sprite *spr = _showQ->first(); spr; spr = spr->_next)
		spr->hide();

	_frmCnt++;
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
	/*if (_vm->_showBoundariesFl) {
		Vga::_page[0]->hLine(0, 200 - kPanHeight, 320, 0xee);
		if (_vm->_barriers[_vm->_now]._horz != 255) {
		for (int i = 0; i < 8; i++)
		Vga::_page[0]->vLine((_vm->_barriers[_vm->_now]._horz * 8) + i, 0, 200, 0xff);
		}
		if (_vm->_barriers[_vm->_now]._vert != 255) {
		for (int i = 0; i < 4; i++)
		Vga::_page[0]->hLine(0, 80 + (_vm->_barriers[_vm->_now]._vert * 4) + i, 320, 0xff);
		}
		}*/

	warning("STUB: Vga::update()");

	g_system->copyRectToScreen(Vga::_page[0]->getPixels(), kScrWidth, 0, 0, kScrWidth, kScrHeight);
	g_system->updateScreen();
}

void Vga::clear(uint8 color) {
	for (int paneNum = 0; paneNum < 4; paneNum++)
		_page[paneNum]->fillRect(Common::Rect(0, 0, kScrWidth, kScrHeight), color);
}

void Vga::copyPage(uint16 d, uint16 s) {
	_page[d]->copyFrom(*_page[s]);
}

//--------------------------------------------------------------------------

void Bitmap::xShow(int16 x, int16 y) {
	const byte *srcP = (const byte *)_v;
	byte *destEndP = (byte *)_vm->_vga->_page[1]->getBasePtr(0, kScrHeight);
	byte *lookupTable = _m;

	// Loop through processing data for each plane. The game originally ran in plane mapped mode, where a
	// given plane holds each fourth pixel sequentially. So to handle an entire picture, each plane's data
	// must be decompressed and inserted into the surface
	for (int planeCtr = 0; planeCtr < 4; planeCtr++) {
		byte *destP = (byte *)_vm->_vga->_page[1]->getBasePtr(x + planeCtr, y);

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
				srcP++;
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
	V2D pos(_vm, x, y);
	xLatPos(pos);
	x = pos.x;
	y = pos.y;
	const byte *srcP = (const byte *)_v;
	byte *destEndP = (byte *)_vm->_vga->_page[1]->getBasePtr(0, kScrHeight);

	// Loop through processing data for each plane. The game originally ran in plane mapped mode, where a
	// given plane holds each fourth pixel sequentially. So to handle an entire picture, each plane's data
	// must be decompressed and inserted into the surface
	for (int planeCtr = 0; planeCtr < 4; planeCtr++) {
		byte *destP = (byte *)_vm->_vga->_page[1]->getBasePtr(x + planeCtr, y);

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
				srcP++;
		}
	}
}


void Bitmap::hide(int16 x, int16 y) {
	V2D pos(_vm, x, y);
	xLatPos(pos);
	x = pos.x;
	y = pos.y;
	for (int yp = y; yp < y + _h; yp++) {
		const byte *srcP = (const byte *)_vm->_vga->_page[2]->getBasePtr(x, yp);
		byte *destP = (byte *)_vm->_vga->_page[1]->getBasePtr(x, yp);

		Common::copy(srcP, srcP + _w, destP);
	}
}

/*--------------------------------------------------------------------------*/

HorizLine::HorizLine(CGE2Engine *vm) : Sprite(vm), _vm(vm) {
	// Set the sprite list
	BitmapPtr *HL = new BitmapPtr[2];
	HL[0] = new Bitmap(_vm, "HLINE");
	HL[1] = NULL;

	setShapeList(HL, 1);

	warning("HorizLine::HorizLine() - Recheck this!");
}

SceneLight::SceneLight(CGE2Engine *vm) : Sprite(vm), _vm(vm) {
	// Set the sprite list
	BitmapPtr *PR = new BitmapPtr[2];
	PR[0] = new Bitmap(_vm, "PRESS");
	PR[1] = NULL;

	setShapeList(PR, 1);

	warning("SceneLight::SceneLight() - Recheck this!");
}

Speaker::Speaker(CGE2Engine *vm): Sprite(vm), _vm(vm) {
	// Set the sprite list
	BitmapPtr *SP = new BitmapPtr[3];
	SP[0] = new Bitmap(_vm, "SPK_L");
	SP[1] = new Bitmap(_vm, "SPK_R");
	SP[2] = NULL;

	setShapeList(SP, 2);

	warning("Speaker::Speaker() - Recheck this!");
}

PocLight::PocLight(CGE2Engine *vm): Sprite(vm), _vm(vm) {
	// Set the sprite list
	BitmapPtr *LI = new BitmapPtr[5];
	LI[0] = new Bitmap(_vm, "LITE0");
	LI[1] = new Bitmap(_vm, "LITE1");
	LI[2] = new Bitmap(_vm, "LITE2");
	LI[3] = new Bitmap(_vm, "LITE3");
	LI[4] = NULL;

	setShapeList(LI, 4);

	_flags._kill = false;

	warning("PocLight::PocLight() - Recheck this!");
}

} // End of namespace CGE2
