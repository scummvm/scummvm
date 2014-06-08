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

#include "cge2/hero.h"
#include "cge2/text.h"
#include "cge2/cge2_main.h"

namespace CGE2 {

Hero::Hero(CGE2Engine *vm)
	: Sprite(vm), _contact(nullptr), _dir(kNoDir),
      _curDim(0), _tracePtr(-1), _ignoreMap(false) {
}

Sprite *Hero::expand() { // It's very similar to Sprite's expand, but doesn't bother with "labels" for example. TODO: Try to unify the two later!
	if (_ext)
		return this;

	char *text = _vm->_text->getText(_ref + 100);
	char fname[kMaxPath];
	_vm->mergeExt(fname, _file, kSprExt);
	_ext = new SprExt(_vm);
	if (_ext == nullptr)
		error("No core %s", fname);

	if (*_file) {
		int cnt[kActions];
		Seq *seq;
		int section = kIdPhase;

		for (int i = 0; i < kDimMax; i++) {
			_dim[i] = new Bitmap[_shpCnt];
			for (int j = 0; j < _shpCnt; j++)
				_dim[i][j].setVM(_vm);
		}

		if (_seqCnt) {
			seq = new Seq[_seqCnt];
			if (seq == nullptr)
				error("No core %s", fname);
		} else
			seq = nullptr;

		for (int i = 0; i < kActions; i++)
			cnt[i] = 0;

		for (int i = 0; i < kActions; i++) {
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

			ID id;
			Common::String line;
			char tmpStr[kLineMax + 1];
			int shpcnt = 0;
			int seqcnt = 0;
			int maxnow = 0;
			int maxnxt = 0;

			for (line = sprf.readLine(); !sprf.eos(); line = sprf.readLine()) {
				if (line.size() == 0)
					continue;
				Common::strlcpy(tmpStr, line.c_str(), sizeof(tmpStr));

				char *p = _vm->token(tmpStr);

				id = _vm->ident(p);
				switch (id) {
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
						id = (ID)_vm->_commandHandler->getComId(p);
						if (_actionCtrl[section]._cnt) {
							CommandHandler::Command *c = &_ext->_actions[section][cnt[section]++];
							c->_commandType = CommandType(id);
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
						for (int i = 0; i < kDimMax; i++) {
							char *q = p;
							q[1] = '0' + i;
							Bitmap b(_vm, q);
							if (!b.moveHi())
								error("No EMS %s", q);
							_dim[i][shpcnt] = b;
							if (!shpcnt)
								_hig[i] = b._h;
						}
						++shpcnt;
						break;
					}
				}
			}
			if (seq) {
				if (maxnow >= shpcnt)
					error("Bad PHASE in SEQ %s", fname);
				if (maxnxt >= seqcnt)
					error("Bad JUMP in SEQ %s", fname);
				setSeq(seq);
			} else
				setSeq(_stdSeq8);

			setShapeList(_dim[0], shpcnt);
		}
	}
	_reachStart = atoi(_vm->token(text));
	_reachCycle = atoi(_vm->token(nullptr));
	_sayStart = atoi(_vm->token(nullptr));
	_funStart = atoi(_vm->token(nullptr));
	_funDel = _funDel0 = (72 / _ext->_seq[0]._dly) * atoi(_vm->token(nullptr));
	int i = stepSize() / 2;
	_maxDist = sqrt(double(i * i * 2));
	setCurrent();
	
	return this;
}

void Hero::setCurrent() {
	double m = _vm->_eye->_z / (_pos3D._z - _vm->_eye->_z);
	int h = -(V2D::trunc(m * _siz.y));

	int i = 0;
	for (; i < kDimMax; i++) {
		if (h >= (_hig[i] + _hig[i + 1]) / 2)
			break;
	}

	_ext->_shpList = _dim[_curDim = i];
}

void Hero::hStep() {
	warning("STUB: Hero::hStep()");
}

Sprite *Hero::setContact() {
	Sprite *spr;
	int md = _maxDist << 1;
	for (spr = _vm->_vga->_showQ->first(); spr; spr = spr->_next) {
		if (spr->_actionCtrl[kNear]._cnt && (spr->_ref & 255) != 255) {
			if (distance(spr) <= md) {
				if (spr == _contact)
					return nullptr;
				else
					break;
			}
		}
	}
	return (_contact = spr);
}

void Hero::tick() {
	warning("STUB: Hero::tick()");
}

int Hero::distance(V3D pos) {
	V3D di = _pos3D - pos;
	long x = V2D::round(di._x);
	long z = V2D::round(di._z);
	return ((x * x + z * z) * (x * x + z * z));
}

int Hero::distance(Sprite *spr) {
	V3D pos = spr->_pos3D;
	int mdx = (spr->_siz.x >> 1) + (_siz.x >> 1);
	int dx = V2D::round(_pos3D._x - spr->_pos3D._x);
	if (dx < 0) {
		mdx = -mdx;
		if (dx > mdx)
			pos._x = _pos3D._x;
		else
			pos._x += mdx;
	} else {
		if (dx < mdx)
			pos._x = _pos3D._x;
		else
			pos._x += mdx;
	}
	return distance(pos);
}

void Hero::turn(Dir d) {
	warning("STUB: Hero::turn()");
}

void Hero::park() {
	if (_dir != kNoDir) {
		step(8 + 5 * _dir);
		_dir = kNoDir;
		_trace[0] = _pos3D;
		_tracePtr = -1;
		setCurrent();
		_flags._zmov = true;
	}
	_ignoreMap = false;
	if (_time == 0)
		++_time;
}

bool Hero::lower(Sprite * spr) {
		warning("STUB: Hero::lower()");
		return false;
}

void Hero::reach(int mode) {
	warning("STUB: Hero::reach()");
}

void Hero::fun() {
	warning("STUB: Hero::fun()");
}

int Hero::len(V2D v) {
	return ((v.x * v.x + v.y * v.y) * (v.x * v.x + v.y * v.y));
}

bool Hero::findWay(){
	V2D p0(_vm, V2D::round(_pos3D._x), V2D::round(_pos3D._z));
	V2D p1(_vm, V2D::round(_trace[_tracePtr]._x), V2D::round(_trace[_tracePtr]._z));
	bool pvOk;
	bool phOk;
	V2D ph(_vm, p1.x, p0.y);
	V2D pv(_vm, p0.x, p1.y);
	pvOk = (!mapCross(p0, pv) && !mapCross(pv, p1));
	phOk = (!mapCross(p0, ph) && !mapCross(ph, p1));
	int md = (_maxDist >> 1);
	if (pvOk && (len(ph - p0) <= md || len(p1 - ph) <= md))
		return true;
	if (phOk && (len(pv - p0) <= md || len(p1 - pv) <= md))
		return true;

	if (pvOk) {
		_trace[++_tracePtr] = V3D(pv.x, 0, pv.y);
		return true;
	}
	if (phOk) {
		_trace[++_tracePtr] = V3D(ph.x, 0, ph.y);
		return true;
	}
	return false;
}

int Hero::snap(int p, int q, int grid) {
	int d = q - p;
	d = ((d >= 0) ? d : -d) % grid;
	if (d > (grid >> 1))
		d -= grid;
	return (q >= p) ? (q - d) : (q + d);
}

void Hero::walkTo(V3D pos) {
	if (distance(pos) <= _maxDist)
		return;
	int stp = stepSize();
	pos._x = snap(V2D::round(_pos3D._x), V2D::round(pos._x), stp);
	pos._y = 0;
	pos._z = snap(V2D::round(_pos3D._z), V2D::round(pos._z), stp);

	V2D p0(_vm, V2D::round(_pos3D._x), V2D::round(_pos3D._z));
	V2D p1(_vm, V2D::round(pos._x), V2D::round(pos._z));
	resetFun();
	int cnt = mapCross(p0, p1);
	if ((cnt & 1) == 0) { // even == way exists
		_trace[_tracePtr = 0] = pos;
		if (!findWay()) {
			int i;
			++_tracePtr;
			for (i = stp; i < kMaxTry; i += stp) {
				_trace[_tracePtr] = pos + V3D(i, 0, 0);
				if (!mapCross(_trace[_tracePtr - 1], _trace[_tracePtr]) && findWay())
					break;

				_trace[_tracePtr] = pos + V3D(-i, 0, 0);
				if (!mapCross(_trace[_tracePtr - 1], _trace[_tracePtr]) && findWay())
					break;

				_trace[_tracePtr] = pos + V3D(0, 0, i);
				if (!mapCross(_trace[_tracePtr - 1], _trace[_tracePtr]) && findWay())
					break;

				_trace[_tracePtr] = pos + V3D(0, 0, -i);
				if (!mapCross(_trace[_tracePtr - 1], _trace[_tracePtr]) && findWay())
					break;
			}
			if (i >= kMaxTry)
				_trace[_tracePtr] = V3D(_pos3D._x, 0, pos._z); // not found
		}
	}
}

void Hero::walkTo(Sprite *spr) {
	int mdx = _siz.x >> 1;
	int stp = (stepSize() + 1) / 2;
	if (!spr->_flags._east)
		mdx = -mdx;
	walkTo(spr->_pos3D + V3D(mdx, 0, (!spr->_flags._frnt || spr->_pos3D._z < 8) ? stp : -stp));
}

V3D Hero::screenToGround(V2D pos) {
	double z = _vm->_eye->_z + (_vm->_eye->_y * _vm->_eye->_z) / (double(pos.y) - _vm->_eye->_y);
	double x = _vm->_eye->_x - ((double(pos.x) - _vm->_eye->_x) * (z - _vm->_eye->_z)) / _vm->_eye->_z;
	return V3D(V2D::round(x), 0, V2D::round(z));
}

int Hero::cross(const V2D &a, const V2D &b) {
	int x = V2D::trunc(_pos3D._x);
	int z = V2D::trunc(_pos3D._z);
	int r = ((_siz.x / 3) * V2D::trunc(_vm->_eye->_z)) / (V2D::trunc(_vm->_eye->_z) - z);
	return _vm->cross(a, b, V2D(_vm, x - r, z), V2D(_vm, x + r, z)) << 1;
}

bool CGE2Engine::cross(const V2D &a, const V2D &b, const V2D &c, const V2D &d) {
	if (contain(a, b, c))
		return true;
	if (contain(a, b, d))
		return true;
	if (contain(c, d, a))
		return true;
	if (contain(c, d, b))
		return true;
	return sgn(det(a, b, c)) != sgn(det(a, b, d)) && sgn(det(c, d, a)) != sgn(det(c, d, b));
}

bool CGE2Engine::contain(const V2D &a, const V2D &b, const V2D &p) {
	if (det(a, b, p))
		return false;
	return ((long)(a.x - p.x) * (p.x - b.x) >= 0 && (long)(a.y - p.y) * (p.y - b.y) >= 0);
}

long CGE2Engine::det(const V2D &a, const V2D &b, const V2D &c) {
	long n = ((long)a.x * b.y + (long)b.x * c.y + (long)c.x*a.y) - ((long)c.x*b.y + (long)b.x*a.y + (long)a.x*c.y);
	return n;
}

int CGE2Engine::sgn(long n) {
	return (n == 0) ? 0 : ((n > 0) ? 1 : -1);
}

int Hero::mapCross(const V2D &a, const V2D &b) {
	Hero *o = other();
	int n = (o->_scene == _scene) ? o->cross(a, b) : 0;
	if (!_ignoreMap)
		n += _vm->mapCross(a, b);
	return n;
}

int Hero::mapCross(const V3D &a, const V3D &b) {
	return mapCross(V2D(_vm, V2D::round(a._x), V2D::round(a._z)), V2D(_vm, V2D::round(b._x), V2D::round(b._z)));
}

int CGE2Engine::mapCross(const V2D &a, const V2D &b) {
	warning("STUB: CGE2Engine::mapCross()");
	return 0;
}

void Hero::setCave(int c) {
	_scene = c;
	resetFun();
}

void Hero::operator++() {
	if (_curDim > 0)
		_ext->_shpList = _dim[--_curDim];
}

void Hero::operator--() {
	if (_curDim < kDimMax - 1)
		_ext->_shpList = _dim[++_curDim];
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

} // End of namespace CGE2
