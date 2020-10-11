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

#include "common/math.h"

#include "petka/walk.h"
#include "petka/petka.h"
#include "petka/q_manager.h"
#include "petka/q_system.h"
#include "petka/objects/heroes.h"

namespace Petka {

const char *const wayPrefixes[] = {"-w-n.", "-w-ne.", "-w-e.", "-w-se.", "-w-s.", "-w-sw.", "-w-w.", "-w-nw."};

const double kPI = M_PI;
const double k2PI = M_PI * 2;
const double kHalfPI = M_PI_2;

const double kPiArray[] = {M_PI_2, M_PI_4, 0.0, -M_PI_4, -M_PI_2, -3 * M_PI_4, M_PI, 3 * M_PI_4};

Walk::Walk(int id) { // CHECKED
	QManager *mgr = g_vm->resMgr();

	Common::String res = mgr->findResourceName(id);
	res.toLowercase();
	res = res.substr(0, res.find(wayPrefixes[0]));
	for (int i = 0; i < 8; ++i) {
		waysSizes[i] = readWayFile(res + Common::String(wayPrefixes[i]) + "off", &_off1[i], &_off2[i]) - 1;
		readWayFile(res + Common::String(wayPrefixes[i]) + "leg", &_leg1[i], &_leg2[i]);
	}

	for (int i = 0; i < 8; ++i) {
		_offleg1[i] = new int[waysSizes[i] + 1];
		_offleg2[i] = new int[waysSizes[i] + 1];
		for (int j = 1; j <= waysSizes[i]; ++j) {
			_offleg1[i][j] = _leg1[i][j] + _off1[i][j] - _leg1[i][j - 1];
			_offleg2[i][j] = _leg2[i][j] + _off2[i][j] - _leg2[i][j - 1];
		}
		_offleg1[i][0] = _offleg1[i][waysSizes[i]];
		_offleg2[i][0] = _offleg2[i][waysSizes[i]];
	}
	for (int i = 0; i < 8; ++i) {
		int v16 = 0;
		int v18 = 0;
		int idx = 1;
		for (int j = 0; j < 150; ++j) {
			v16 += _offleg1[i][idx];
			v18 += _offleg2[i][idx];
			idx = (idx + 1) % waysSizes[i];
		}
		field_D0[i] = (double)v18 / v16;
	}

	currX = 0.0;
	currY = 0.0;
	_bkg3Count = 0;
	_bkg2Count = 0;
	field_134 = 0;
	_bkg1Count = 0;
	_bkg3_1 = nullptr;
	_bkg3_2 = nullptr;
	_bkg2 = nullptr;
	_bkg3_3 = nullptr;
	_bkg3_4 = nullptr;
	_bkg3_5 = nullptr;
	_bkg1 = nullptr;
	field_190 = 0;
	resId = 0;
}

Walk::~Walk() { // CHECKED
	clearBackground();
	reset();
	for (int i = 0; i < 8; ++i) {
		delete[] _leg1[i];
		delete[] _leg2[i];
		delete[] _off1[i];
		delete[] _off2[i];
		delete[] _offleg1[i];
		delete[] _offleg2[i];

		_leg1[i] = nullptr;
		_leg2[i] = nullptr;
		_off1[i] = nullptr;
		_off2[i] = nullptr;
		_offleg1[i] = nullptr;
		_offleg2[i] = nullptr;

		waysSizes[i] = 0;
	}
}

void Walk::init(Point start, Point end) {
	reset();
	field_134 = 1;

	int v5 = sub_424230(&end) ? moveInside(&end) : sub_423600(end);
	_bkg3_4[0] = sub_424230(&start) ? moveInside(&start) : sub_423600(start);


	destX = end.x;
	destY = end.y;

	currX = end.x;
	currY = end.y;

	if (start == end)
		return;

	if (sub_424160(&start, &end) || v5 != _bkg3_4[0]) {
		for (int i = 0; i < _bkg3Count; ++i)
			_bkg3_5[i] = -1;

		field_134 = 1;
		if (_bkg3_4[0] != v5) {
			do {
				_bkg3_4[field_134] = sub_423970(_bkg3_4[field_134 - 1], _bkg3_5[field_134 - 1]);
				if (_bkg3_4[field_134] >= 0) {
					_bkg3_5[field_134 - 1] = _bkg3_4[field_134];
					if (field_134 <= 1 || _bkg3_4[field_134 - 2] != _bkg3_4[field_134])
						field_134++;
				} else {
					field_134--;
					_bkg3_4[field_134] = -1;
					_bkg3_5[field_134] = -1;
				}

			} while (_bkg3_4[field_134 - 1] != v5);
		}

		int v20 = 1;
		int v21 = 4;
		int v22 = 1;
		int a2 = 4;
		_bkg3_3[0] = start;
		if (field_134 > 1) {
			do {
				int v23 = sub_423A30(_bkg3_4[(v21 - 4) / 4], _bkg3_4[v21 / 4]);
				_bkg3_3[v22].x = (_bkg1[_bkg2[v23].x].x + _bkg1[_bkg2[v23].y].x) / 2;
				_bkg3_3[v22].y = (_bkg1[_bkg2[v23].x].y + _bkg1[_bkg2[v23].y].y) / 2;

				if (v22 > 1 && !sub_424160(&_bkg3_3[v22 - 2], &_bkg3_3[v22])) {
					v20--;

					_bkg3_3[v22 - 1] = _bkg3_3[v22];

					v22--;
					field_134--;
				}

				v21 = a2 + 4;
				v20++;
				v22++;
				a2 += 4;
			} while (v20 < field_134);
		}

		_bkg3_3[v20] = end;
		if (v20 > 1 && !sub_424160(_bkg3_3 + v20 - 2, _bkg3_3 + v20)) {
			_bkg3_3[v20 - 1] = _bkg3_3[v20];
			field_134--;
		}
		field_134++;
	} else {
		_bkg3_3[0] = start;
		_bkg3_3[1] = end;
		field_134 = 2;
	}

	sub_422EA0(_bkg3_3[0], _bkg3_3[1]);
	field_14C = 1;
}

void Walk::clearBackground() { // CHECKED
	delete[] _bkg1;
	_bkg1 = nullptr;
	_bkg1Count = 0;


	if (_bkg3_1) {
		if (_bkg3_2) {
			for (int i = 0; i < _bkg3Count; ++i) {
				delete[] _bkg3_2[i];
				_bkg3_2[i] = nullptr;
			}
			delete[] _bkg3_2;
			_bkg3_2 = nullptr;
		}
		delete[] _bkg3_1;
		_bkg3_1 = nullptr;
		_bkg3Count = 0;
	}

	delete[] _bkg2;
	_bkg2 = nullptr;
	_bkg2Count = 0;

	delete[] _bkg3_3;
	_bkg3_3 = nullptr;

	delete[] _bkg3_4;
	_bkg3_4 = nullptr;

	delete[] _bkg3_5;
	_bkg3_5 = nullptr;
}

void Walk::setBackground(Common::String name) { // CHECKED
	clearBackground();

	name.toLowercase();
	name.replace(name.size() - 3, 3, "cvx");

	Common::SeekableReadStream *stream = g_vm->openFile(name, false);
	if (!stream)
		return;

	_bkg1Count = stream->readUint32LE();
	_bkg1 = new Point[_bkg1Count];

	for (int i = 0; i < _bkg1Count; ++i) {
		_bkg1[i].x = stream->readUint32LE();
		_bkg1[i].y = stream->readUint32LE();
	}

	_bkg2Count = stream->readUint32LE();
	_bkg2 = new Point[_bkg2Count];

	for (int i = 0; i < _bkg2Count; ++i) {
		_bkg2[i].x = stream->readUint32LE();
		_bkg2[i].y = stream->readUint32LE();
	}

	_bkg3Count = stream->readUint32LE();
	_bkg3_1 = new int[_bkg3Count];

	stream->read(_bkg3_1, 4 * _bkg3Count);

	_bkg3_2 = new int*[_bkg3Count];

	for (int i = 0; i < _bkg3Count; ++i) {
		_bkg3_2[i] = new int[_bkg3_1[i]];
		stream->read(_bkg3_2[i], 4 * _bkg3_1[i]);
	}

	delete stream;
	_bkg3_3 = new Point[_bkg3Count + 1];
	_bkg3_4 = new int[_bkg3Count + 1];
	_bkg3_5 = new int[_bkg3Count + 1];

}

void Walk::reset() { // CHECKED
	field_140 = 0.0;
	field_138 = 0.0;
	currX = 0;
	currY = 0;
	resId = 0;
	field_14C = 0;
	field_190 = 0;
	field_194 = 0;
}

Common::Point Walk::currPos() { // CHECKED
	return Common::Point(currX, currY);
}

int Walk::getSpriteId() { // CHECKED
	return resId;
}

int Walk::commonPoint(int idx1, int idx2) { // CHECKED
	if (_bkg2[idx1].x == _bkg2[idx2].x || _bkg2[idx1].x == _bkg2[idx2].y)
		return _bkg2[idx1].x;

	if (_bkg2[idx1].y != _bkg2[idx2].x && _bkg2[idx1].y != _bkg2[idx2].y)
		return 0;

	return _bkg2[idx1].y;
}

int Walk::readWayFile(const Common::String &name, int **p1, int **p2) { // CHECKED
	Common::SeekableReadStream *stream = g_vm->openFile(name, false);
	if (!stream) {
		p1 = nullptr;
		p2 = nullptr;
		return 0;
	}

	const uint items = (uint)stream->size() / 8;

	*p1 = new int[items];
	*p2 = new int[items];

	stream->skip(4);
	for (uint i = 0; i < items; ++i) {
		stream->read(&(*p1)[i], 4);
		stream->read(&(*p2)[i], 4);
	}

	delete stream;
	return items;
}

int Walk::sub_422EA0(Point p1, Point p2) {
	if (p1 == p2)
		return 0;

	Point p = p1;
	p.x += 150;

	double v5 = angle(p1, p, p2);
	double v6;
	if (v5 >= 0.0)
		v6 = k2PI - v5;
	else
		v6 = v5 + k2PI;

	double v30 = 4.0;
	for (uint i = 0; i < ARRAYSIZE(kPiArray); ++i) {
		double v9 = v5 - kPiArray[i];
		if (v9 < 0.0)
			v9 = -v9;
		if (v9 < v30) {
			v30 = v9;
			resId = i;
		}

		double v10 = v6 - kPiArray[i];
		if (v10 < 0.0)
			v10 = -v10;
		if (v10 < v30) {
			v30 = v10;
			resId = i;
		}
	}

	double v28 = p2.x - p1.x;
	double v26 = p2.y - p1.y;
	double v12 = Common::hypotenuse(p2.x - p1.x, p2.y - p1.y);

	double v39 = 1.0 / sqrt(field_D0[resId] * field_D0[resId] - -1.0);
	if (v39 == 0.0)
		field_140 = v28 / v12;
	else
		field_140 = (field_D0[resId] - -1.0 / (v26 / v28)) * (v26 / v12) * v39;

	DBLPoint a1;
	DBLPoint a2;
	DBLPoint a3;

	a1.x = p1.x;
	a1.y = p1.y;

	a2.x = p2.x;
	a2.y = p2.y;

	a3.x = p2.x;
	a3.y = field_D0[resId] * v28 + p1.y;

	double v13 = angle(a1, a2, a3);
	field_140 = cos(v13);
	field_138 = sin(v13);

	double v16 = v13;
	if (v13 < -kHalfPI)
		v16 = v13 + kPI;
	if (v13 > kHalfPI)
		v16 = v13 - kPI;

	field_140 = cos(v16);
	field_138 = sin(v16);

	int v32 = 1;
	double v34 = 0.0;
	double v35 = 0.0;
	double v36 = p1.y;
	v39 = v28 * v28 + v26 * v26 - -1.0;

	int j = 0;
	for (int i = 0; i < 10;) {
		double k = g_vm->getQSystem()->getPetka()->calcPerspective(v36);

		v34 += _offleg1[resId][v32] * k;
		v35 += _offleg2[resId][v32] * k;

		j++;

		v32 = (v32 + 1) % waysSizes[resId];

		v36 = v35 * field_140 + v34 * field_138 + p1.y;

		double v22 = v34 * field_140 - v35 * field_138 + p1.x - p2.x;
		double v38 = v36 - p2.y;

		double v23 = v22 * v22 + v38 * v38;
		if (v23 >= v39) {
			i++;
		} else {
			v39 = v23;
			field_194 = j;
			i = 0;
		}
	}

	field_170 = 0;
	field_178 = 0;

	currX = p1.x;
	currY = p1.y;

	field_150 = p1.x;
	field_158 = p1.y;

	field_190 = 0;
	field_198 = g_vm->getQSystem()->getPetka()->calcPerspective(p1.y);
	return resId;
}

int Walk::sub_423350() { // CHECKED
	field_190 = (field_190 + 1) % waysSizes[resId];

	--field_194;
	if (field_194 < 0) {
		field_14C++;
		if (field_14C < field_134) {
			int t = field_190;
			int id = resId;
			if (id == sub_422EA0(_bkg3_3[field_14C - 1], _bkg3_3[field_14C])) {
				field_190 = t;
				return 1;
			}
			return 2;
		}
		return 0;
	}

	field_198 = g_vm->getQSystem()->getPetka()->calcPerspective(currY);
	field_170 = _offleg1[resId][field_190] * field_198 + field_170;
	field_178 = _offleg2[resId][field_190] * field_198 + field_178;

	currX = field_140 * field_170 - field_178 * field_138 + field_150;
	currY = field_140 * field_178 + field_138 * field_170 + field_158;

	return 1;
}

Common::Point Walk::sub_4234B0() { // CHECKED
	Common::Point p;
	field_198 = g_vm->getQSystem()->getPetka()->calcPerspective(currY);
	p.x = currX - _leg1[resId][field_190] * field_198;
	p.y = currY - _leg2[resId][field_190] * field_198;
	return p;
}

bool Walk::sub_423570(int i1, int i2) { // CHECKED
	if (i1 == i2)
		return false;

	if (_bkg2[i1].x == _bkg2[i2].x || _bkg2[i1].x == _bkg2[i2].y)
		return true;

	return !(_bkg2[i1].y != _bkg2[i2].x && _bkg2[i1].y != _bkg2[i2].y);
}

int Walk::sub_423600(Point p) {
	int j = 0;
	for (int i = 0; i < _bkg3Count; ++i, ++j) {
		int *v4 = new int[_bkg3_1[j]];
		v4[0] = _bkg3_2[j][0];

		for (int k = 0; k < _bkg3_1[j]; ++k) {
			if (sub_423570(v4[0], _bkg3_2[j][k])) {
				v4[1] = _bkg3_2[j][k];
				break;
			}
		}

		for (int k = 2; k < _bkg3_1[j]; ++k) {
			for (int l = 0; l < _bkg3_1[j]; ++l) {
				if (sub_423570(v4[k - 1], _bkg3_2[j][l]) && v4[k - 2] != _bkg3_2[j][l]) {
					v4[k] = _bkg3_2[j][l];
					break;
				}
			}
		}

		int v11 = commonPoint(v4[_bkg3_1[j] - 1], v4[0]);
		int v31 = commonPoint(v4[0], v4[1]);

		double v12 = angle(p, _bkg1[v11], _bkg1[v31]);
		if (p == _bkg1[v11] || p == _bkg1[v31]) {
			delete[] v4;
			return i;
		}


		int k;
		for (k = 1; k < _bkg3_1[j] - 1; ++k) {
			int v16 = commonPoint(v4[k - 1], v4[k]);
			int v32 = commonPoint(v4[k], v4[k + 1]);

			v12 += angle(p, _bkg1[v16], _bkg1[v32]);
			if (p == _bkg1[v16] || p == _bkg1[v32]) {
				delete[] v4;
				return i;
			}
		}

		int v19 = commonPoint(v4[k - 1], v4[k]);
		int v20 = commonPoint(v4[k], v4[0]);

		delete[] v4;

		double v23 = angle(p, _bkg1[v19], _bkg2[v20]);
		v12 += v23;

		if (p == _bkg1[v19] || p == _bkg1[v20])
			return i;

		if (v12 < 0.0)
			v12 = -v12;

		if (v12 > kPI)
			return i;
	}
	debug("Walk bug: Point doesn't belong to any convex");

	return 0;
}

int Walk::sub_423970(int a1, int a2) { // CHECKED
	int index = 0;
	if (a2 >= 0) {
		int v5 = sub_423A30(a1, a2);
		for (int i = 0; i < _bkg3_1[a1]; ++i) {
			if (_bkg3_2[a1][i] == v5) {
				index = i + 1;
				break;
			}
		}
	}

	for (int i = index; i < _bkg3_1[a1]; ++i) {
		for (int j = 0; j < _bkg3Count; ++j) {
			if (j == a1)
				continue;

			for (int k = 0; k < _bkg3_1[j]; ++k) {
				if (_bkg3_2[j][k] == _bkg3_2[a1][i]) {
					return j;
				}
			}
		}
	}
	return -1;
}

int Walk::sub_423A30(int idx1, int idx2) { // CHECKED
	for (int i = 0; i < _bkg3_1[idx1]; ++i) {
		for (int j = 0; j < _bkg3_1[idx2]; ++j) {
			if (_bkg3_2[idx1][i] == _bkg3_2[idx2][j])
				return _bkg3_2[idx1][i];
		}
	}
	return 0;
}

double Walk::angle(Point p1, Point p2, Point p3) { // CHECKED
	return angle(DBLPoint(p1), DBLPoint(p2), DBLPoint(p3));
}

double Walk::angle(DBLPoint p1, DBLPoint p2, DBLPoint p3) { // CHECKED
	if (p1 == p2 || p1 == p3)
		return 0.0;

	double xv1 = p2.x - p1.x;
	double xv2 = p3.x - p1.x;

	double yv1 = p2.y - p1.y;
	double yv2 = p3.y - p1.y;

	double mv1 = Common::hypotenuse(xv1, yv1);
	double mv2 = Common::hypotenuse(xv2, yv2);
	double v13 = (xv1 * xv2 + yv1 * yv2) / (mv1 * mv2);
	if ((xv2 / mv2 * (yv1 / mv1) - yv2 / mv2 * (xv1 / mv1)) < 0.0) // Not sure
		return -acos(v13);
	return acos(v13);
}

int Walk::sub_423E00(Point p1, Point p2, Point p3, Point p4, Point &p5) {
	if (p1.x > p2.x) {
		SWAP(p2, p1);
	}

	if (p3.x > p4.x) {
		SWAP(p3, p4);
	}

	double v11 = (p2.y - p1.y) * (p4.x - p3.x);
	double v12 = (p2.x - p1.x) * (p4.y - p3.y);
	if (v11 == v12)
		return 0;

	double v30;
	if (p2.x - p1.x) {
		if (p4.x - p3.x) {
			v30 = ((double)(p3.y - p1.y) * (p4.x - p3.x) * (p2.x - p1.x) + (v11 * p1.x) - (v12 * p3.x)) / (v11 - v12);
			if (v30 < p1.x || p3.x > v30 || p2.x < v30 || p4.x < v30)
				return 0;
		} else {
			v30 = p3.x;
			if (p3.x < p1.x || p2.x < p3.x)
				return 0;
		}
	} else {
		v30 = p1.x;
		if (p1.x < p3.x)
			return 0;
		if (p4.x < p1.x)
			return 0;
	}

	if (p1.y > p2.y) {
		SWAP(p1, p2);
	}

	if (p3.y > p4.y) {
		SWAP(p3, p4);
	}

	if (p2.y - p1.y) {
		if (p4.y - p3.y) {
			double v21;
			if (p2.x - p1.x) {
				v21 = (v30 - p1.x) * (p2.y - p1.y) / (p2.x - p1.x) + p1.y;
			} else {
				v21 = (v30 - p3.x) * (p4.y - p3.y) / (p4.x - p3.x) + p3.y;
			}

			if (v21 >= p1.y && v21 >= p3.y && v21 <= p2.y && v21 <= p4.y) {
				p5.x = v30;
				p5.y = v21;
				return 1;
			}

		} else {
			if (p3.y >= p1.y) {
				if (p3.y > p2.y)
					return 0;
				p5.x = v30;
				p5.y = p3.y;
				return 1;
			}
		}
	} else {
		if (p1.y >= p3.y) {
			if (p1.y <= p4.y) {
				p5.x = v30;
				p5.y = p1.y;
				return 1;
			}
		}
	}

	return 0;
}

bool Walk::sub_424160(Point *p1, Point *p2) { // CHECKED
	if (*p1 == *p2)
		return false;

	Point p;
	int v = 1;
	if (_bkg1Count <= 1)
		return sub_423E00(_bkg1[v - 1], _bkg1[0], *p1, *p2, p) != 0;

	while (!sub_423E00(_bkg1[v - 1], _bkg1[v], *p1, *p2, p)) {
		if (++v >= _bkg1Count)
			return sub_423E00(_bkg1[v - 1], _bkg1[0], *p1, *p2, p) != 0;
	}
	return true;
}

bool Walk::sub_424230(Point *p1) { // CHECKED
	Point p(0, 0);
	int v = sub_424320(p1, &p);

	p.y = p1->y;
	v = (sub_424320(p1, &p) & 1) + (v & 1);

	p.y = 480;
	v = (sub_424320(p1, &p) & 1) + v;
	v = (sub_424320(p1, &p) & 1) + v;

	p.x = 640;
	v = (sub_424320(p1, &p) & 1) + v;

	p.y = p1->y;
	v = (sub_424320(p1, &p) & 1) + v;

	p.y = 0;
	v = (sub_424320(p1, &p) & 1) + v;

	p.x = p1->x;
	return (sub_424320(p1, &p) & 1) + v < 4;
}

int Walk::sub_424320(Point *p1, Point *p2) { // CHECKED
	if (*p1 == *p2)
		return 0;

	int ret = 0;

	Point p;

	int i;
	for (i = 1; i < _bkg1Count; ++i) {
		if (sub_423E00(_bkg1[i - 1], _bkg1[i], *p1, *p2, p) && *p1 != p && *p2 != p) {
			ret++;
		}
	}

	if (sub_423E00(_bkg1[i - 1], _bkg1[0], *p1, *p2, p) && *p1 != p && *p2 != p) {
		ret++;
	}

	return ret;
}

int Walk::moveInside(Point *p) { // CHECKED
	DBLPoint dp = sub_424610(_bkg1[_bkg2->x], _bkg1[_bkg2->y].x, _bkg1[_bkg2->y].y, *p);

	int index = 0;

	double min = (dp.y - p->y) * (dp.y - p->y) + (dp.x - p->x) * (dp.x - p->x);
	for (int i = 1; i < _bkg1Count; ++i) {
		DBLPoint dp1 = sub_424610(_bkg1[_bkg2[i].x], _bkg1[_bkg2[i].y].x, _bkg1[_bkg2[i].y].y, *p);
		double curr = (dp1.y - p->y) * (dp1.y - p->y) + (dp1.x - p->x) * (dp1.x - p->x);
		if (curr < min) {
			dp = dp1;
			min = curr;
			index = i;
		}

	}

	p->x = dp.x;
	p->y = dp.y;


	for (int i = 0; i < _bkg1Count; ++i) {
		for (int j = 0; j < _bkg3_1[i]; ++j) {
			if (_bkg3_2[i][j] == index)
				return i;
		}
	}

	return 0;
}

DBLPoint Walk::sub_424610(Point p1, int x, int y, Point p2) { // CHECKED
	DBLPoint p;
	double v13;
	double v14;
	if (p1.x == x) {
		v13 = p1.x;
		v14 = p2.y;
	} else {
		double v6 = (double)(y - p1.y) / (x - p1.x);
		double v7 = p1.y - v6 * p1.x;
		v13 = ((p2.y - v7) * v6 + p2.x) / (v6 * v6 - -1.0);
		v14 = v13 * v6 + v7;
	}
	p.x = CLIP<double>(v13, MIN<int>(x, p1.x), MAX<int>(x, p1.x));
	p.y = CLIP<double>(v14, MIN<int>(y, p1.y), MAX<int>(y, p1.y));
	return p;
}

} // End of namespace Petka
