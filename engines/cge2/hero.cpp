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

#include "cge2/hero.h"
#include "cge2/text.h"
#include "cge2/map.h"

namespace CGE2 {

Hero::Hero(CGE2Engine *vm) : Sprite(vm), _contact(nullptr), _dir(kNoDir),
	_curDim(0), _tracePtr(-1), _ignoreMap(false), _maxDist(0) {

	for (int i = 0; i < kDimMax; i++)
		_dim[i] = nullptr;

	_reachStart = _reachCycle = _sayStart = _funStart = 0;
	_funDel0 = _funDel = 0;
}

Hero::~Hero() {
	contract();
}

Sprite *Hero::expand() {
	if (_ext)
		return this;

	char fname[kMaxPath];
	_vm->mergeExt(fname, _file, kSprExt);

	if (_ext != nullptr)
		delete _ext;

	_ext = new SprExt(_vm);

	if (!*_file)
		return this;

	for (int i = 0; i < kDimMax; i++) {
		if (_dim[i] != nullptr) {
			delete[] _dim[i];
			_dim[i] = nullptr;
		}
	}
	for (int i = 0; i < kDimMax; i++) {
		_dim[i] = new Bitmap[_shpCnt];
		for (int j = 0; j < _shpCnt; j++)
			_dim[i][j].setVM(_vm);
	}

	int cnt[kActions];

	for (int i = 0; i < kActions; i++)
		cnt[i] = 0;

	for (int i = 0; i < kActions; i++) {
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

		ID section = kIdPhase;
		ID id;
		Common::String line;
		char tmpStr[kLineMax + 1];
		int shpcnt = 0;
		int seqcnt = 0;
		int maxnow = 0;
		int maxnxt = 0;

		for (line = sprf.readLine(); !sprf.eos(); line = sprf.readLine()) {
			if (line.empty())
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
					for (int i = 0; i < kDimMax; i++) {
						char *q = p;
						q[1] = '0' + i;
						Bitmap b(_vm, q);
						_dim[i][shpcnt] = b;
						if (!shpcnt)
							_hig[i] = b._h;
					}
					++shpcnt;
					break;
				default:
					break;
				}
			}
		}

		if (curSeq) {
			if (maxnow >= shpcnt)
				error("Bad PHASE in SEQ %s", fname);
			if (maxnxt >= seqcnt)
				error("Bad JUMP in SEQ %s", fname);
			setSeq(curSeq);
		} else
			setSeq(_stdSeq8);

		setShapeList(_dim[0], shpcnt);
	}

	char *tempStr = _vm->_text->getText(_ref + 100);
	char *text = new char[strlen(tempStr) + 1];
	strcpy(text, tempStr);
	_reachStart = atoi(_vm->token(text));
	_reachCycle = atoi(_vm->token(nullptr));
	_sayStart = atoi(_vm->token(nullptr));
	_funStart = atoi(_vm->token(nullptr));
	_funDel = _funDel0 = (72 / _ext->_seq[0]._dly) * atoi(_vm->token(nullptr));
	delete[] text;

	int i = stepSize() / 2;
	_maxDist = (int)sqrt(double(i * i * 2));
	setCurrent();

	return this;
}

Sprite *Hero::contract() {
	for (int i = 0; i < kDimMax; i++) {
		if (_dim[i] != nullptr) {
			delete[] _dim[i];
			if (_ext->_shpList == _dim[i])
				_ext->_shpList = nullptr;
			_dim[i] = nullptr;
		}
	}
	Sprite::contract();
	return this;
}

void Hero::setCurrent() {
	FXP m = _vm->_eye->_z / (_pos3D._z - _vm->_eye->_z);
	FXP tmp = m * _siz.y;
	int h = -(tmp.trunc());

	int i = 0;
	for (; i < kDimMax - 1; i++) {
		if (h >= (_hig[i] + _hig[i + 1]) / 2)
			break;
	}

	_ext->_shpList = _dim[_curDim = i];
}

void Hero::hStep() {
	if (!_ignoreMap && _ext) {
		Seq *seq = _ext->_seq;
		int ptr = seq[_seqPtr]._next;
		seq += ptr;
		if (seq->_dx | seq->_dz) {
			V2D p0(_vm, _pos3D._x.round(), _pos3D._z.round());
			V2D p1(_vm, p0.x + seq->_dx, p0.y + seq->_dz);
			if (mapCross(p0, p1)) {
				park();
				return;
			}
		}
	}
	step();
}

Sprite *Hero::setContact() {
	Sprite *spr;
	int md = _maxDist << 1;
	for (spr = _vm->_vga->_showQ->first(); spr; spr = spr->_next) {
		if (spr->_actionCtrl[kNear]._cnt && ((spr->_ref & 255) != 255) && (distance(spr) <= md)) {
			if (spr == _contact)
				return nullptr;
			else
				break;
		}
	}
	return (_contact = spr);
}

void Hero::tick() {
	int z = _pos3D._z.trunc();
	//-- maybe not exactly wid/2, but wid/3 ?
	int d = ((_siz.x / 2) * _vm->_eye->_z.trunc()) / (_vm->_eye->_z.trunc() - z);

	if (_dir != kNoDir) { // just walking...
		if (_flags._hold || _tracePtr < 0)
			park();
		else {
			Sprite *spr = setContact();
			if (spr)
				_vm->feedSnail(spr, kNear, this);
		}
	}
	//---------------------------------------------------------------
	if (_tracePtr >= 0) {
		if (distance(_trace[_tracePtr]) <= _maxDist)
			--_tracePtr;

		if (_tracePtr < 0)
			park();
		else {
			int stp = stepSize() / 2;
			int dx = _trace[_tracePtr]._x.round() - _pos3D._x.round();
			int dz = _trace[_tracePtr]._z.round() - _pos3D._z.round();
			Dir dir = (dx > stp) ? kEE : ((-dx > stp) ? kWW : ((dz > stp) ? kNN : kSS));
			turn(dir);
		}
	}

	//---------------------------------------------------------------
	hStep();
	setCurrent();
	switch (_dir) {
	case kSS:
		if (_pos3D._z < stepSize() / 2)
			park();
		break;
	case kWW:
		if (_pos2D.x <= d)
			park();
		break;
	case kNN:
		if (_pos3D._z > kScrDepth)
			park();
		break;
	case kEE:
		if (_pos2D.x >= kScrWidth - 1 - d)
			park();
		break;
	default:
		break;
	}
	if (_flags._trim)
		gotoxyz_(_pos2D);

	if (_pos3D._z.trunc() != z)
		_flags._zmov = true;

	if (--_funDel == 0)
		fun();
}

int Hero::distance(V3D pos) {
	V3D di = _pos3D - pos;
	int x = di._x.round();
	int z = di._z.round();
	int retval = (int)sqrt((double)x * x + z * z);
	return retval;
}

int Hero::distance(Sprite *spr) {
	V3D pos = spr->_pos3D;
	int mdx = (spr->_siz.x >> 1) + (_siz.x >> 1);
	int dx = (_pos3D._x - spr->_pos3D._x).round();
	if (dx < 0) {
		mdx = -mdx;
		if (dx > mdx)
			pos._x = _pos3D._x;
		else
			pos._x += mdx;
	} else if (dx < mdx)
		pos._x = _pos3D._x;
	else
		pos._x += mdx;

	return distance(pos);
}

void Hero::turn(Dir d) {
	Dir dir = (_dir == kNoDir) ? kSS : _dir;
	if (d != _dir) {
		step((d == dir) ? 57 : (8 + 4 * dir + d));
		_dir = d;
	}
	resetFun();
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
	return (spr->_pos3D._y + (spr->_siz.y >> 2) < 10);
}

void Hero::reach(int mode) {
	Sprite *spr = nullptr;
	if (mode >= 4) {
		spr = _vm->_vga->_showQ->locate(mode);
		if (spr) {
			mode = !spr->_flags._east;      // 0-1
			if (lower(spr))                 // 2-3
				mode += 2;
		}
	}
	// note: insert SNAIL commands in reverse order
	_vm->_commandHandler->insertCommand(kCmdPause, -1, 24, nullptr);
	_vm->_commandHandler->insertCommand(kCmdSeq, -1, _reachStart + _reachCycle * mode, this);
	if (spr) {
		_vm->_commandHandler->insertCommand(kCmdWait, -1, -1, this);
		_vm->_commandHandler->insertCommand(kCmdWalk, -1, spr->_ref, this);
	}
	// sequence is not finished,
	// now it is just at sprite appear (disappear) point
	resetFun();
}

void Hero::fun() {
	if (_vm->_commandHandler->idle()) {
		park();
		_vm->_commandHandler->addCommand(kCmdWait, -1, -1, this);
		_vm->_commandHandler->addCommand(kCmdSeq, -1, _funStart, this);
	}
	_funDel = _funDel0 >> 2;
}

int Hero::len(V2D v) {
	return (int)sqrt(double(v.x * v.x + v.y * v.y));
}

bool Hero::findWay(){
	V2D p0(_vm, _pos3D._x.round(), _pos3D._z.round());
	V2D p1(_vm, _trace[_tracePtr]._x.round(), _trace[_tracePtr]._z.round());
	V2D ph(_vm, p1.x, p0.y);
	V2D pv(_vm, p0.x, p1.y);
	bool pvOk = (!mapCross(p0, pv) && !mapCross(pv, p1));
	bool phOk = (!mapCross(p0, ph) && !mapCross(ph, p1));
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
	int d = abs(q - p) % grid;
	if (d > (grid >> 1))
		d -= grid;
	return (q >= p) ? (q - d) : (q + d);
}

void Hero::walkTo(V3D pos) {
	if (distance(pos) <= _maxDist)
		return;

	int stp = stepSize();
	pos._x = snap(_pos3D._x.round(), pos._x.round(), stp);
	pos._y = 0;
	pos._z = snap(_pos3D._z.round(), pos._z.round(), stp);

	V2D p0(_vm, _pos3D._x.round(), _pos3D._z.round());
	V2D p1(_vm, pos._x.round(), pos._z.round());
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
	FXP z = _vm->_eye->_z + (_vm->_eye->_y * _vm->_eye->_z) / (FXP(pos.y) - _vm->_eye->_y);
	FXP x = _vm->_eye->_x - ((FXP(pos.x) - _vm->_eye->_x) * (z - _vm->_eye->_z)) / _vm->_eye->_z;
	return V3D(x.round(), 0, z.round());
}

int Hero::cross(const V2D &a, const V2D &b) {
	int x = _pos3D._x.trunc();
	int z = _pos3D._z.trunc();
	int r = ((_siz.x / 3) * _vm->_eye->_z.trunc()) / (_vm->_eye->_z.trunc() - z);
	return _vm->cross(a, b, V2D(_vm, x - r, z), V2D(_vm, x + r, z)) << 1;
}

bool CGE2Engine::cross(const V2D &a, const V2D &b, const V2D &c, const V2D &d) {
	if (contain(a, b, c) || contain(a, b, d) || contain(c, d, a) || contain(c, d, b))
		return true;

	return sgn(det(a, b, c)) != sgn(det(a, b, d)) && sgn(det(c, d, a)) != sgn(det(c, d, b));
}

bool CGE2Engine::contain(const V2D &a, const V2D &b, const V2D &p) {
	if (det(a, b, p))
		return false;

	return ((long)(a.x - p.x) * (p.x - b.x) >= 0 && (long)(a.y - p.y) * (p.y - b.y) >= 0);
}

long CGE2Engine::det(const V2D &a, const V2D &b, const V2D &c) {
	return ((long)a.x * b.y + (long)b.x * c.y + (long)c.x * a.y) - ((long)c.x * b.y + (long)b.x * a.y + (long)a.x * c.y);
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
	return mapCross(V2D(_vm, a._x.round(), a._z.round()), V2D(_vm, b._x.round(), b._z.round()));
}

int CGE2Engine::mapCross(const V2D &a, const V2D &b) {
	int cnt = 0;
	V2D *n0 = nullptr;
	V2D *p = nullptr;
	for (int i = 0; i < _map->size(); i++) {
		V2D *n = _map->getCoord(i);
		if (p) {
			if (cross(a, b, *n0, *n))
				++cnt;

			if (*n == *p)
				p = nullptr;
		} else {
			p = n;
		}
		n0 = n;
	}
	return cnt;
}

void Hero::setScene(int c) {
	Sprite::setScene(c);
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
	if (!spr || !spr->_ext)
		return false;

	bool ok = false;

	Action a = _vm->_heroTab[_vm->_sex]->_ptr->action();
	CommandHandler::Command *ct = spr->_ext->_actions[a];
	if (ct) {
		int i = spr->_actionCtrl[a]._ptr;
		int n = spr->_actionCtrl[a]._cnt;
		while (i < n && !ok) {
			CommandHandler::Command *c = &ct[i++];
			if (c->_commandType != kCmdUse)
				break;
			ok = (c->_ref == _ref);
			if (c->_val > 255) {
				if (ok) {
					int p = spr->labVal(a, c->_val >> 8);
					if (p >= 0)
						spr->_actionCtrl[a]._ptr = p;
					else
						ok = false;
				}
			} else {
				if (c->_val && c->_val != _vm->_now)
					ok = false;
				break;
			}
		}
	}

	return ok;
}

} // End of namespace CGE2
