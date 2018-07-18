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
	memset(&_starfieldPosition, 0, sizeof(_starfieldPosition));
	// TODO: matrix initialization
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

void StarTrekEngine::clearStarfieldPixels() {
	_gfx->fillBackgroundRect(_starfieldRect, 0);
}

void StarTrekEngine::drawStarfield() {
	// TODO: make these class variables
	Point3W starPositionWeightings[] = {{0x4000, 0, 0}, {0, 0x4000, 0}, {0, 0, 0x4000}};
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
			star->pos = applyPointWeightings(starPositionWeightings, point) + _starfieldPosition;
			star->active = true;
		}

		Point3 p = star->pos - _starfieldPosition;
		Point3 point2 = applyPointWeightings2(p, starPositionWeightings);

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
	// sub_24b74(...);
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

Point3 StarTrekEngine::applyPointWeightings(Point3W *weight, const Point3 &point) {
	int32 ret[3];
	for (int i = 0; i < 3; i++) {
		ret[i] = weight[i].x * (point.x & 0xffff) + weight[i].y * (point.y & 0xffff) + weight[i].z * (point.z & 0xffff);
		ret[i] <<= 2;
	}
	Point3 p;
	p.x = ret[0] >> 16;
	p.y = ret[1] >> 16;
	p.z = ret[2] >> 16;
	return p;
}

Point3 StarTrekEngine::applyPointWeightings2(const Point3 &point, Point3W *weight) {
	Point3 p = Point3();
	p.x = (weight[0].x * (point.x & 0xffff) + weight[1].x * (point.y & 0xffff) + weight[2].x * (point.z & 0xffff)) << 2;
	p.y = (weight[0].y * (point.x & 0xffff) + weight[1].y * (point.y & 0xffff) + weight[2].y * (point.z & 0xffff)) << 2;
	p.z = (weight[0].z * (point.x & 0xffff) + weight[1].z * (point.y & 0xffff) + weight[2].z * (point.z & 0xffff)) << 2;
	p.x >>= 16;
	p.y >>= 16;
	p.z >>= 16;
	return p;
}

}
