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


#include "startrek/startrek.h"

namespace StarTrek {

void StarTrekEngine::initStarfieldPosition() {
	_starfieldPosition = Point3(0, 0, 0);
	_someMatrix = initMatrix();
}

void StarTrekEngine::initStarfield(int16 x, int16 y, int16 width, int16 height, int16 arg8) {
	_starfieldXVar1 = (x + width) / 2;
	_starfieldYVar1 = (y + height) / 2;
	_starfieldXVar2 = (width - x + 1) / 2;
	_starfieldYVar2 = (height - y + 1) / 2;

	_starfieldRect.left = _starfieldXVar1 - _starfieldXVar2;
	_starfieldRect.right = _starfieldXVar1 + _starfieldXVar2;
	_starfieldRect.top = _starfieldYVar1 - _starfieldYVar2;
	_starfieldRect.bottom = _starfieldYVar1 + _starfieldYVar2;

	memset(_starList, 0, sizeof(_starList));
	_starfieldPointDivisor = 150;
}

void StarTrekEngine::addR3(R3 *r3) {
	for (int i = 0; i < NUM_SPACE_OBJECTS; i++) {
		if (_r3List[i] == nullptr) {
			_r3List[i] = r3;
			return;
		}
	}

	error("addR3: out of shapes.");
}

void StarTrekEngine::delR3(R3 *r3) {
	for (int i = 0; i < NUM_SPACE_OBJECTS; i++) {
		if (_r3List[i] == r3) {
			_r3List[i] = nullptr;
			r3->field1e = 0;
			return;
		}
	}

	error("delR3: shape not found.");
}

void StarTrekEngine::clearStarfieldPixels() {
	_gfx->fillBackgroundRect(_starfieldRect, 0);
}

void StarTrekEngine::drawStarfield() {
	// TODO: make these class variables
	float flt_50898 = 50.0; // ?

	int16 var28 = ((_starfieldXVar2 * 3) >> 1);
	int16 xvar = var28 / 2;
	int16 var2a = ((_starfieldYVar2 * 3) >> 1);
	int16 yvar = var2a / 2;
	int16 var8 = _starfieldPointDivisor << 3;

	SharedPtr<FileStream> file = loadFile("stars.shp");

	for (int i = 0; i < NUM_STARS; i++) {
		Star *star = &_starList[i];
		if ((i & 0xf) == 0) {
			file->seek(0, SEEK_SET);
		}

		if (!star->active) {
			int16 var4 = getRandomWord() / var28 - xvar;
			int16 var6 = getRandomWord() / var2a - yvar;
			Point3 point = constructPoint3ForStarfield(var4, var6, var8);
			star->pos = matrixMult(_starPositionMatrix, point) + _starfieldPosition;
			star->active = true;
		}

		Point3 p = star->pos - _starfieldPosition;
		Point3 point2 = matrixMult(p, _starPositionMatrix);

		if (point2.z > flt_50898 && point2.z < 0x3fff
				&& abs(point2.x) < point2.z && abs(point2.y) < point2.z) {

			int16 x = _starfieldXVar1 + (point2.x * _starfieldPointDivisor / point2.z);
			int16 y = _starfieldYVar1 - (point2.y * _starfieldPointDivisor / point2.z);

			int fileOffset = file->pos();
			file->readUint32();
			int16 width = file->readUint16();
			int16 height = file->readUint16();

			Common::Rect starRect(x, y, x + width, y + height);
			Common::Rect drawRect = _starfieldRect.findIntersectingRect(starRect);

			file->seek(fileOffset, SEEK_SET);
			SharedPtr<Bitmap> bitmap = SharedPtr<Bitmap>(new Bitmap(file));

			if (!drawRect.isEmpty())
				_gfx->drawBitmapToBackground(starRect, drawRect, bitmap);
			bitmap.reset();
		} else {
			star->active = false;

			file->readUint32();
			int16 offset2 = file->readUint16() * file->readUint16();
			file->seek(file->pos() + offset2, SEEK_SET);
		}
	}
}

void StarTrekEngine::updateStarfieldAndShips(bool arg0) {
	_starfieldSprite.bitmapChanged = true;
	_starPositionMatrix = _someMatrix.invert();
	clearStarfieldPixels();
	drawStarfield();

	// TODO
}

Point3 StarTrekEngine::constructPoint3ForStarfield(int16 x, int16 y, int16 z) {
	Point3 point;
	point.z = z;
	point.y = y * z / _starfieldPointDivisor;
	point.x = x * z / _starfieldPointDivisor;

	return point;
}

Point3 StarTrekEngine::matrixMult(const Matrix &weight, const Point3 &point) {
	int32 ret[3];
	for (int i = 0; i < 3; i++) {
		ret[i] = weight[i][0].multToInt(point.x & 0xffff) + weight[i][1].multToInt(point.y & 0xffff) + weight[i][2].multToInt(point.z & 0xffff);
	}
	Point3 p;
	p.x = ret[0];
	p.y = ret[1];
	p.z = ret[2];
	return p;
}

Point3 StarTrekEngine::matrixMult(const Point3 &point, const Matrix &weight) {
	Point3 p = Point3();
	p.x = (weight[0][0].multToInt(point.x & 0xffff) + weight[1][0].multToInt(point.y & 0xffff) + weight[2][0].multToInt(point.z & 0xffff));
	p.y = (weight[0][1].multToInt(point.x & 0xffff) + weight[1][1].multToInt(point.y & 0xffff) + weight[2][1].multToInt(point.z & 0xffff));
	p.z = (weight[0][2].multToInt(point.x & 0xffff) + weight[1][2].multToInt(point.y & 0xffff) + weight[2][2].multToInt(point.z & 0xffff));
	return p;
}

Matrix StarTrekEngine::initMatrix() {
	Matrix mat;
	mat[0][0] = 1;
	mat[1][1] = 1;
	mat[2][2] = 1;
	return mat;
}

}
