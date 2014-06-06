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
	warning("STUB: Hero::setContact()");
	return this;
}

void Hero::tick() {
	warning("STUB: Hero::tick()");
}

int Hero::distance(V3D pos) {
	warning("STUB: Hero::distance()");
	return 0;
}

int Hero::distance(Sprite *spr) {
	warning("STUB: Hero::distance()");
	return 0;
}

void Hero::turn(Dir d) {
	warning("STUB: Hero::turn()");
}

void Hero::park() {
	warning("STUB: Hero::park()");
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

void Hero::operator ++ () {
	warning("STUB: Hero::operator ++()");
}

void Hero::operator -- () {
	warning("STUB: Hero::operator --()");
}

uint32 Hero::len(V2D v) {
	warning("STUB: Hero::works()");
	return 0;
}

bool Hero::findWay(){
	warning("STUB: Hero::findWay()");
	return false;
}

int Hero::snap(int p, int q, int grid) {
	warning("STUB: Hero::findWay()");
	return 0;
}

void Hero::walkTo(V3D pos) {
	warning("STUB: Hero::walkTo()");
}

void Hero::walkTo(Sprite *spr) {
	warning("STUB: Hero::walkTo()");
}

V3D Hero::screenToGround(V2D pos) {
	double z = _vm->_eye->_z + (_vm->_eye->_y * _vm->_eye->_z) / (double(pos.y) - _vm->_eye->_y);
	double x = _vm->_eye->_x - ((double(pos.x) - _vm->_eye->_x) * (z - _vm->_eye->_z)) / _vm->_eye->_z;
	return V3D(V2D::round(x), 0, V2D::round(z));
}


int Hero::cross(const V2D &a, const V2D &b) {
	warning("STUB: Hero::cross()");
	return 0;
}

int Hero::mapCross(const V2D &a, const V2D &b) {
	warning("STUB: Hero::mapCross()");
	return 0;
}

int Hero::mapCross(const V3D &a, const V3D &b) {
	warning("STUB: Hero::mapCross()");
	return 0;
}

void Hero::setCave(int c) {
	warning("STUB: Hero::mapCross()");
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
