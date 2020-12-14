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

#include "startrek/resource.h"
#include "startrek/startrek.h"

namespace StarTrek {

void StarTrekEngine::initStarfieldPosition() {
	_starfieldPosition = Point3(0, 0, 0);
	_someMatrix = initMatrix();
}

void StarTrekEngine::initStarfield(int16 x, int16 y, int16 width, int16 height, int16 arg8) {
	// TODO: finish
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
	_flt_50898 = 50.0;
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
	delete r3->bitmap;
	r3->bitmap = nullptr;

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
	int16 var28 = ((_starfieldXVar2 * 3) >> 1);
	int16 xvar = var28 / 2;
	int16 var2a = ((_starfieldYVar2 * 3) >> 1);
	int16 yvar = var2a / 2;
	int16 var8 = _starfieldPointDivisor << 3;

	Common::MemoryReadStreamEndian *file = _resource->loadFile("stars.shp");

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

		if (point2.z > _flt_50898 && point2.z < 0x3fff
		        && abs(point2.x) < point2.z && abs(point2.y) < point2.z) {

			int16 x = _starfieldXVar1 + scaleSpacePosition(point2.x, point2.z);
			int16 y = _starfieldYVar1 - scaleSpacePosition(point2.y, point2.z);

			int fileOffset = file->pos();
			file->readUint32();
			int16 width = file->readUint16();
			int16 height = file->readUint16();

			Common::Rect starRect(x, y, x + width, y + height);
			Common::Rect drawRect = _starfieldRect.findIntersectingRect(starRect);

			file->seek(fileOffset, SEEK_SET);

			Bitmap *bitmap = new Bitmap(file, false);
			if (!drawRect.isEmpty())
				_gfx->drawBitmapToBackground(starRect, drawRect, bitmap);
			delete bitmap;
		} else {
			star->active = false;

			file->readUint32();
			int16 offset2 = file->readUint16() * file->readUint16();
			file->seek(file->pos() + offset2, SEEK_SET);
		}
	}

	delete file;
}

/**
 * Compare 2 r3 objects for the purpose of sorting them by layer before drawing.
 * FIXME: Original returned an int, not a bool. This may affect the stability of the sort...
 */
bool compareR3Objects(R3 *obj1, R3 *obj2) {
	int32 diff = obj1->field54 - obj2->field54;
	if (diff < 0)
		return true;
	else if (diff == 0)
		return false; // original would have a distinct value for this
	else
		return false;
}

void StarTrekEngine::updateStarfieldAndShips(bool arg0) {
	bool enterpriseDestroyed = false;

	_starfieldSprite.bitmapChanged = true;
	_starPositionMatrix = _someMatrix.invert();
	clearStarfieldPixels();
	drawStarfield();

	int numObjects = 0;
	for (int i = 0; i < NUM_SPACE_OBJECTS; i++) {
		R3 *r3 = _r3List[i];
		if (r3 == nullptr)
			continue;
		r3->field34 = 0;

		switch (r3->field1e) { // TODO
		case 1:
			r3 = sub_19f24(r3);
			break;
		case 2: // TODO (specifically for the enterprise?)
			break;
		case 3:
			r3 = sub_19f24(r3);
			break;
		default:
			break;
		}

		if (r3 != nullptr)
			_orderedR3List[numObjects++] = r3;
	}

	if (numObjects != 0) {
		Common::sort(_orderedR3List, _orderedR3List + numObjects, &compareR3Objects);

		for (int i = 0; i < numObjects; i++) {
			R3 *r3 = _orderedR3List[i];
			r3->field34 = 1;

			switch (r3->field1e) { // TODO
			case 1:
				break;
			case 2:
				break;
			case 3:
				if (r3->field20 == 1) {
					// TODO
				} else
					drawR3Shape(r3);
				break;
			default:
				break;
			}
		}
	}

	if (enterpriseDestroyed) {
		showTextbox("", "GENE\\SPACE000#The Enterprise has been destroyed!", 20, 20, TEXTCOLOR_YELLOW, 0);
		showGameOverMenu();
		// FIXME: original game manipulates stack to jump somewhere...
		return;
	}

	// TODO: static on enterprise viewscreen?
}

R3 *StarTrekEngine::sub_19f24(R3 *r3) {
	r3->matrix2 = r3->matrix;
	r3->field36 = r3->pos;
	r3->field36 -= _starfieldPosition;
	r3->field54 = r3->field36.getDiagonal();
	Point3 point = matrixMult(r3->field36, _starPositionMatrix);
	r3->field36 = point;

	if (sub_1c022(r3)) {
		if (r3->field1e >= 1 && r3->field1e <= 3)
			r3->matrix2 *= _starPositionMatrix;
		r3->field58 = _starfieldXVar1 + scaleSpacePosition(r3->field36.x, r3->field36.z);
		r3->field5a = _starfieldYVar1 - scaleSpacePosition(r3->field36.y, r3->field36.z);
		return r3;
	} else
		return nullptr;
}

void StarTrekEngine::drawR3Shape(R3 *r3) {
	if (r3->field1e != 3) // TODO: remove this
		return;

	if (r3->funcPtr1 != 0) {
		// TODO: call it
	}

	if (r3->bitmap != nullptr) {
		double dbl68 = ((double)r3->field24 * _starfieldPointDivisor) / r3->field36.z;
		double dbl70 = 1.0 / dbl68;

		double dbl30 = 1.0;
		double dbl28 = 0.0;
		double dbl20 = 0.0;
		double dbl18 = 1.0;
		double dbl60 = 1.0;
		double dbl58 = 0.0;
		double dbl50 = 0.0;
		double dbl48 = 0.0;

		switch (r3->field1e) {
		case 1: // TODO
			break;
		case 3:
			dbl30 = dbl68;
			dbl28 = 0.0;
			dbl20 = 0.0;
			dbl18 = dbl68;
			dbl60 = dbl70;
			dbl58 = 0.0;
			dbl50 = 0.0;
			dbl48 = dbl70;
			break;
		case 4: // TODO
			break;
		default: // TODO
			break;
		}

		r3->field80 = dbl60;
		r3->field88 = dbl58;
		r3->field90 = dbl50;
		r3->field98 = dbl48;

		// dbl30, (bitmap->xoffset + 2), r3->field58,
		double tmp = r3->field58 - ((double)r3->bitmap->xoffset + 2) * dbl30;
		// dbl20, (bitmap->yoffset + 2), tmp
		double dbl10 = tmp - ((double)r3->bitmap->yoffset + 2) * dbl20;

		// dbl28, (bitmap->xoffset + 2), r3->field5a
		tmp = r3->field5a - ((double)r3->bitmap->xoffset + 2) * dbl28;
		// dbl18, (bitmap->yoffset + 2), tmp
		double dbl8 = tmp - ((double)r3->bitmap->yoffset + 2) * dbl18;

		// dbl60, r3->field58, bitmap->xoffset + 2
		tmp = ((double)r3->bitmap->xoffset + 2) - dbl60 * r3->field58;
		double dbl40 = tmp - dbl50 * r3->field5a;

		tmp = ((double)r3->bitmap->yoffset + 2) - dbl58 * r3->field58;
		double dbl38 = tmp - dbl48 * r3->field5a;

		double dbl3e4 = (double)r3->bitmap->width + 2;
		double dbl3ec = (double)r3->bitmap->height + 2;

		double thing[8];
		tmp = 1.0 * dbl30;
		thing[0] = 1.0 * dbl20 + tmp + dbl10;

		tmp = 1.0 * dbl28;
		thing[1] = 1.0 * dbl18 + tmp + dbl8;

		tmp = 1.0 * dbl30;
		thing[2] = dbl3ec * dbl20 + tmp + dbl10;

		tmp = 1.0 * dbl28;
		thing[3] = dbl3ec * dbl18 + tmp + dbl8;

		thing[4] = dbl3ec * dbl20 + dbl3e4 * dbl30 + dbl10;
		thing[5] = dbl3ec * dbl18 + dbl3e4 * dbl28 + dbl8;

		thing[6] = 1.0 * dbl20 + dbl3e4 * dbl30 + dbl10;
		thing[7] = 1.0 * dbl18 + dbl3e4 * dbl28 + dbl8;

		int16 *rightBounds = new int16[SCREEN_HEIGHT];
		int16 *leftBounds = new int16[SCREEN_HEIGHT];

		for (int y = _starfieldRect.top; y < _starfieldRect.bottom; y++) {
			leftBounds[y] = _starfieldRect.right;
			rightBounds[y] = _starfieldRect.left - 1;
		}

		int16 shpImageTop = _starfieldRect.bottom - 1;
		int16 shpImageBottom = _starfieldRect.top;

		for (int i = 0; i < 4; i++) {
			int16 index1 = i;
			int16 index2 = (i + 1) & 3;

			if (thing[index1 * 2 + 1] > thing[index2 * 2 + 1]) {
				index1 = index2;
				index2 = i;
			}

			int16 top = ceil(thing[index1 * 2 + 1]);
			int16 bottom = floor(thing[index2 * 2 + 1]);

			if (top > bottom)
				continue;
			if (top > _starfieldRect.bottom - 1)
				continue;
			if (bottom < _starfieldRect.top)
				continue;
			if (top < _starfieldRect.top)
				top = _starfieldRect.top;
			if (bottom > _starfieldRect.bottom - 1)
				bottom = _starfieldRect.bottom - 1;

			if (top < shpImageTop)
				shpImageTop = top;
			if (bottom > shpImageBottom)
				shpImageBottom = bottom;

			double dbl3f4;
			if (thing[index2 * 2 + 1] == thing[index1 * 2 + 1])
				dbl3f4 = 0.0;
			else
				dbl3f4 = (thing[index2 * 2] - thing[index1 * 2]) / (thing[index2 * 2 + 1] - thing[index1 * 2 + 1]);

			int32 boundDiff = (int32)(0x10000 * dbl3f4); // var3ec
			int32 boundBase = (int32)(((top - thing[index1 * 2 + 1]) * dbl3f4 + thing[index1 * 2]) * 0x10000); // var3e8

			for (int y = top; y <= bottom; y++) {
				int16 rightBound = boundBase >> 16; // var3f6
				int16 leftBound = (boundBase + 0xffff) >> 16; // var3f8

				if (leftBound < _starfieldRect.left)
					leftBound = _starfieldRect.left;
				if (leftBound < leftBounds[y])
					leftBounds[y] = leftBound;

				if (rightBound > _starfieldRect.right - 1)
					rightBound = _starfieldRect.right - 1;
				if (rightBound > rightBounds[y])
					rightBounds[y] = rightBound;

				boundBase += boundDiff;
			}
		}

		while (shpImageTop <= shpImageBottom) {
			if (leftBounds[shpImageTop] > rightBounds[shpImageTop])
				shpImageTop++;
			else
				break;
		}
		while (shpImageTop <= shpImageBottom) {
			if (leftBounds[shpImageBottom] > rightBounds[shpImageBottom])
				shpImageBottom--;
			else
				break;
		}

		if (shpImageTop <= shpImageBottom) {
			bool var3fa = false;
			if (r3->field1e == 2) {
				// TODO
			} else
				var3fa = (((r3->field24 * _starfieldPointDivisor) << 1) / 3 <= r3->field36.z);

			if (!var3fa) {
				if (r3->field1e == 3) {
					// Is this used anywhere?
					//uint16 var3fc = (_frameIndex << 3) ^ _frameIndex;
				} else {
					// TODO
				}
			}

			// Amount added to X/Y positions after each pixel is drawn
			int16 xDiff = (int16)(dbl60 * 256);
			int16 yDiff = (int16)(dbl58 * 256);

			int16 var3f2 = (int16)(dbl50 * 256);
			int16 var3f4 = (int16)(dbl48 * 256);

			int16 var3f6 = var3f2 * shpImageTop + (int16)(dbl40 * 256);
			int16 var3f8 = var3f4 * shpImageTop + (int16)(dbl38 * 256);

			Bitmap tmpBitmap(256, 249);
			byte *otherBuffer = new byte[256 * 256];

			int16 bitmapWidth = r3->bitmap->width;
			int16 bitmapHeight = r3->bitmap->height;

			if (bitmapHeight > 245)
				error("Shape height too big in drawR3Shape!");
			if (bitmapWidth > 252)
				error("Shape width too big in drawR3Shape!");

			for (int i = 0; i < 2; i++)
				memset(otherBuffer + i * 256, 0, bitmapWidth + 4);
			for (int i = 0; i < 2; i++)
				memset(otherBuffer + (bitmapHeight + 2 + i) * 256, 0, bitmapWidth + 4);

			for (int i = 0; i < bitmapHeight; i++) {
				otherBuffer[512 + i * 256 + 0] = 0;
				otherBuffer[512 + i * 256 + 1] = 0;
				otherBuffer[512 + i * 256 + bitmapWidth + 2] = 0;
				otherBuffer[512 + i * 256 + bitmapWidth + 3] = 0;
			}

			if (r3->field1e == 2) {
				// TODO
			} else
				_gfx->copyRectBetweenBitmaps(&tmpBitmap, 2, 2, r3->bitmap, 0, 0, bitmapWidth, bitmapHeight);

			byte *bgPixels = _gfx->getBackgroundPixels() + shpImageTop * SCREEN_WIDTH;

			for (int y = shpImageTop; y <= shpImageBottom; y++) {
				int16 leftBound = leftBounds[y];
				int16 rowWidth = rightBounds[y] - leftBound;

				int16 srcX = leftBound * xDiff + var3f6;
				int16 srcY = leftBound * yDiff + var3f8;
				var3f6 += var3f2;
				var3f8 += var3f4;

				byte *bgPixels2 = bgPixels + leftBound;
				bgPixels += SCREEN_WIDTH;

				if (rowWidth == 0)
					continue;

				if (var3fa) {
					srcX += 0x80;
					srcY += 0x80;

					int16 cx = srcX;
					int16 bx = srcY;
					byte *di = bgPixels2;

					for (int a = 0; a < rowWidth; a++) {
						byte b = tmpBitmap.pixels[(bx & 0xff00) + (cx >> 8)];
						cx += xDiff;
						bx += yDiff;
						if (b == 0)
							di++;
						else
							*(di++) = b;
					}
				} else {
					warning("Unimplemented branch in \"drawR3Shape\"");
				}
			}

			delete[] otherBuffer;
		}

		delete[] rightBounds;
		delete[] leftBounds;
	}

	if (r3->funcPtr2 != 0) {
		// TODO: call it
	}
}

bool StarTrekEngine::sub_1c022(R3 *r3) {
	//Point3 point = r3->field36;
	//if (point.z < _flt_50898)
	//	return false;
	return true; // TODO: finish this properly
}

Point3 StarTrekEngine::constructPoint3ForStarfield(int16 x, int16 y, int16 z) {
	Point3 point;
	point.z = z;
	point.y = y * z / _starfieldPointDivisor;
	point.x = x * z / _starfieldPointDivisor;

	return point;
}

Point3 StarTrekEngine::matrixMult(const Matrix &weight, const Point3 &point) {
	Point3 p;
	for (int i = 0; i < 3; i++) {
		p[i] = 0;
		for (int j = 0; j < 3; j++)
			p[i] += (int16)weight[i][j].multToInt(point[j]);
	}
	return p;
}

Point3 StarTrekEngine::matrixMult(const Point3 &point, const Matrix &weight) {
	Point3 p = Point3();
	for (int i = 0; i < 3; i++) {
		p[i] = 0;
		for (int j = 0; j < 3; j++)
			p[i] += (int16)weight[j][i].multToInt(point[j]);
	}
	return p;
}

int32 StarTrekEngine::scaleSpacePosition(int32 x, int32 z) {
	if (x == 0 || z == 0)
		return 0;
	return (x * _starfieldPointDivisor) / z;
}

Matrix StarTrekEngine::initMatrix() {
	Matrix mat;
	mat[0][0] = 1.0;
	mat[1][1] = 1.0;
	mat[2][2] = 1.0;
	return mat;
}

Matrix StarTrekEngine::initSpeedMatrixForXZMovement(Angle angle, const Matrix &matrix) {
	Fixed14 sinVal = sin(angle.toDouble());
	Fixed14 cosVal = cos(angle.toDouble());

	Matrix matrix1 = initMatrix();
	matrix1[0].x = cosVal;
	matrix1[0].z = -sinVal;
	matrix1[2].x = sinVal;
	matrix1[2].z = cosVal;

	return matrix * matrix1;
}

} // End of namespace StarTrek
