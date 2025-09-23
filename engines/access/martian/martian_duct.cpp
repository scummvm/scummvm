/* ScummVM - Graphic Adventure Engine
 *
 * ScummVM is the legal property of its developers, whose names
 * are too numerous to list here. Please refer to the COPYRIGHT
 * file distributed with this source distribution.
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 *
 */

#include "access/martian/martian_duct.h"
#include "access/martian/martian_game.h"
#include "access/martian/martian_resources.h"

namespace Access {

namespace Martian {

MartianDuct::MartianDuct(MartianEngine *vm) : _vm(vm), _stopMoveLoop(false), _playerX(0), _preYOffset(0), _playerY(0), _moveAngle(kMoveAngleNorth), _drawDistX(100), _drawDistY(500), _threshold1(50), _crawlFrame(0), _xOffset(160), _yOffset(160), _xScale(100), _yScale(160), _moveIntent(kMoveIntentNone), _primArrayIdx(0), _mapLoc(0), _nextPlayerX(0), _nextPlayerY(0) {
	ARRAYCLEAR(_matrix[0]);
	ARRAYCLEAR(_matrix[1]);
	ARRAYCLEAR(_matrix[2]);
	ARRAYCLEAR(_tempPoints);
}

MartianDuct::~MartianDuct() {
}

void MartianDuct::duct2() {
	_playerX = 550;
	_preYOffset = 10;
	_playerY = 850;
	doDuct();
}

void MartianDuct::duct4() {
	_playerX = 2750;
	_preYOffset = 10;
	_playerY = 1050;
	doDuct();
}

void MartianDuct::doDuct() {
	_vm->_screen->forceFadeOut();

	_vm->_screen->_windowXAdd = 0;
	_vm->_screen->_windowYAdd = 0;
	_vm->_screen->_screenYOff = 0;

	_vm->_events->hideCursor();
	_vm->_files->loadScreen(20, 0);
	Resource *res = _vm->_files->loadFile(20, 1);
	_vm->_objectsTable[20] = new SpriteResource(_vm, res);
	if (_vm->_inventory->_inv[40]._value == ITEM_IN_INVENTORY) {
		// Show map if we have it.
		_vm->_screen->plotImage(_vm->_objectsTable[20], 8, Common::Point(140, 10));
	}
	_vm->_events->showCursor();
	_vm->_screen->forceFadeIn();
	_crawlFrame = 0;
	_xOffset = 160;
	_yOffset = 100;
	_xScale = 160;
	_yScale = 100;
	// The game initialises some drawing window here but it makes no difference
	_moveAngle = kMoveAngleNorth;
	// game has Y and Z angles X fraction part which is always 0
	_drawDistX = 100;
	_drawDistY = 500;
	_threshold1 = 50;

	drawArrowSprites();
	drawArrowSprites2();
	_vm->_room->_function = FN_NONE;

	// FIXME: Quick HACK: skip this part
	//g_system->displayMessageOnOSD(Common::U32String("Duct section not implemented yet!"));
	//_vm->_flags[0x62] = 1;
	//_vm->_flags[0x55] = 1;
	//_vm->_room->_function = FN_CLEAR1;
	// END HACK

	while (!_vm->shouldQuit()) {
		clearWorkScreenArea();
		updateMatrix();
		applyMatrixToMapData();
		// Draw duct panels
		updatePrimsAndDraw();
		// Draw tex over the top
		_vm->_buffer2.plotImage(_vm->_objectsTable[20], _crawlFrame, Common::Point(140, 94));
		copyBufBlockToScreen();
		if (_vm->_room->_function != FN_NONE)
			break;
		do {
			waitForMoveUpdate();
		} while (!_stopMoveLoop && !_vm->shouldQuitOrRestart());
	}

	delete _vm->_objectsTable[20];
	_vm->_objectsTable[20] = nullptr;
}

void MartianDuct::drawArrowSprites() {
	int x;
	int y;
	int frame;

	_vm->_events->hideCursor();
	_vm->_screen->plotImage(_vm->_objectsTable[20], 7, Common::Point(4, 0x50));
	if (_moveAngle == kMoveAngleNorth) { // (highlight up arrow)
		x = 0x11;
		y = 0x50;
		frame = 9;
	} else if (_moveAngle == kMoveAngleEast) { // highlight right arrow
		x = 0x19;
		y = 0x57;
		frame = 12;
	} else if (_moveAngle == kMoveAngleSouth) { // highlight down arrow
		x = 0xe;
		y = 0x5d;
		frame = 11;
	} else { // West or west2 (left arrow)
		x = 4;
		y = 0x57;
		frame = 10;
	}
	_vm->_screen->plotImage(_vm->_objectsTable[20], frame, Common::Point(x, y));
	_vm->_events->showCursor();
}

void MartianDuct::drawArrowSprites2() {
	_vm->_events->hideCursor();
	_vm->_screen->plotImage(_vm->_objectsTable[20], 14, Common::Point(16, 0x58));
	if (_mapLoc > 3 && _mapLoc < 13) {
		_vm->_screen->plotImage(_vm->_objectsTable[20], 13, Common::Point(17, 0x59));
	}
	_vm->_events->showCursor();
}

void MartianDuct::clearWorkScreenArea() {
	_vm->_buffer2.fillRect(Common::Rect(100, 60, 220, 140), 0);
	// For testing, clear the whole buffer:
	//_vm->_buffer2.fillRect(Common::Rect(320, 200), 0);
}

void MartianDuct::copyBufBlockToScreen() {
	// Start from row 60, 100 across. 100 px by 80 rows.
	_vm->_screen->copyBlock(&_vm->_buffer2, Common::Rect(100, 60, 220, 140));
	// For testing, copy everything:
	//_vm->_screen->copyBuffer(&_vm->_buffer2);
}

void MartianDuct::updateMatrix() {
	// The original does a full fixed-point matrix calculation here, but
	// half the values are always 1 or 0 so it's much simpler than that.
	float moveAngleRad = (float)(_moveAngle / 256.0f) * 2.0f * M_PI;
	float cosVal = cos(moveAngleRad);
	float sinVal = sin(moveAngleRad);

	// 3D rotation through Y axis.  We never rotate through the others.
	_matrix[0][0] = cosVal;
	_matrix[0][1] = 0.0f;
	_matrix[0][2] = sinVal;
	_matrix[1][0] = 0.0f;
	_matrix[1][1] = 1.0f;
	_matrix[1][2] = 0.0f;
	_matrix[2][0] = -sinVal;
	_matrix[2][1] = 0.0f;
	_matrix[2][2] = cosVal;
}

void MartianDuct::applyMatrixToMapData() {
	int16 shapeDataIndex;
	_renderShapes.clear();
	_renderPoints.clear();

	for (const DuctMapPoint *pMapData = DUCT_MAP_DATA; shapeDataIndex = pMapData->shapeType, shapeDataIndex != -1; pMapData++) {
		// The original sets the actual drawing x/y here but then uses them
		// as inputs to the next step so we don't need to set them here.
		const int16 blockX = pMapData->x;
		const int16 blockY = pMapData->y;
		if (abs(blockX + _threshold1 - _playerX) < _drawDistX && abs(blockY + _threshold1 - _playerY) < _drawDistY) {
			const DuctShape *ptr = DUCT_SHAPE_DATA[shapeDataIndex];
			const uint startPointNum = _renderPoints.size();
			const Point3 *pPoint = ptr->points;
			// The data is the point list followed by the other data.
			for (int i = 0; i < ptr->numPts; i++) {
				int16 x = pPoint->x + blockX - _playerX;
				int16 y = pPoint->y - _preYOffset;
				int16 z = pPoint->z + blockY - _playerY;
				pPoint++;
				doMatrixMulAndAddPoint(x, y, z);
			}

			const uint16 *dataPtr = ptr->data;
			for (int j = 0; j < ptr->array2Len; j++) {
				RenderShape shapeData;
				// Input struct is byte,ignored,int16,[array of int16]
				shapeData._col = (byte)dataPtr[0];
				const int dataCount = dataPtr[1];
				dataPtr += 2;
				for (int i = 0; i < dataCount; i++) {
					shapeData._pointIdxs.push_back(*dataPtr + startPointNum);
					dataPtr++;
				}
				_renderShapes.push_back(shapeData);
			}
		}
	}
}

void MartianDuct::updatePrimsAndDraw() {
	int16 shapeZDepth[256];
	int shapeIndexes[256];

	for (uint i = 0; i < _renderShapes.size(); i++)
		shapeZDepth[i] = INT16_MIN;

	for (uint shpNum = 0; shpNum < _renderShapes.size(); shpNum++) {
		const Point3 &pt0 = _renderPoints[_renderShapes[shpNum]._pointIdxs[0]];
		const Point3 &pt2 = _renderPoints[_renderShapes[shpNum]._pointIdxs[2]];
		int16 zDepth = ((int)pt0.z + pt2.z) / 2;
		uint insertPoint = 0;
		uint i = _renderShapes.size();
		while (zDepth <= shapeZDepth[insertPoint] && i > 0) {
			insertPoint++;
			i--;
		}

		if (i == 0)
			continue;

		for (uint j = _renderShapes.size(); j > insertPoint; j--) {
			shapeZDepth[j] = shapeZDepth[j - 1];
			shapeIndexes[j] = shapeIndexes[j - 1];
		}

		shapeZDepth[insertPoint] = zDepth;
		shapeIndexes[insertPoint] = shpNum;
	}

	//debug("**** Begin frame ****");
	for (uint shapeNum = 0; shapeNum < _renderShapes.size(); shapeNum++) {
		int shapeIdx = shapeIndexes[shapeNum];
		_vm->_buffer2._lColor = _renderShapes[shapeIdx]._col;
		int numPoints = _renderShapes[shapeIdx]._pointIdxs.size();
		_primX1Array.clear();
		_primY1Array.clear();
		_primZ1Array.clear();
		_primX2Array.clear();
		_primY2Array.clear();
		_primZ2Array.clear();
		// Link up the points in order to make a polygon.
		int ptIdx;
		for (int pointNum = 0; pointNum < numPoints - 1; pointNum++) {
			ptIdx = _renderShapes[shapeIdx]._pointIdxs[pointNum];
			_primX1Array.push_back(_renderPoints[ptIdx].x);
			_primY1Array.push_back(_renderPoints[ptIdx].y);
			_primZ1Array.push_back(_renderPoints[ptIdx].z);
			ptIdx = _renderShapes[shapeIdx]._pointIdxs[pointNum + 1];
			_primX2Array.push_back(_renderPoints[ptIdx].x);
			_primY2Array.push_back(_renderPoints[ptIdx].y);
			_primZ2Array.push_back(_renderPoints[ptIdx].z);
		}
		ptIdx = _renderShapes[shapeIdx]._pointIdxs[numPoints - 1];
		_primX1Array.push_back(_renderPoints[ptIdx].x);
		_primY1Array.push_back(_renderPoints[ptIdx].y);
		_primZ1Array.push_back(_renderPoints[ptIdx].z);
		ptIdx = _renderShapes[shapeIdx]._pointIdxs[0];
		_primX2Array.push_back(_renderPoints[ptIdx].x);
		_primY2Array.push_back(_renderPoints[ptIdx].y);
		_primZ2Array.push_back(_renderPoints[ptIdx].z);

		assert(_primX1Array.size() == (uint)numPoints);

		bool shouldDraw = !doPrimArrayUpdates(numPoints);

		if (!shouldDraw)
			continue;

		/*
		debug("** shape %d [%d] color %d ** ", shapeNum, shapeIdx, _vm->_buffer2._lColor);
		debugN("XYs: ");
		for (int i = 0; i < numPoints; i++) {
			int16 x = _primX1Array[i];
			int16 y = _primY1Array[i];
			debugN("(%d %d), ", x, y);
		}
		debug(".");

		debugN("RBs: ");
		for (int i = 0; i < numPoints; i++) {
			int16 x = _primX2Array[i];
			int16 y = _primY2Array[i];
			debugN("(%d %d), ", x, y);
		}
		debug(".");
		*/

		doDraw(numPoints);
	}
	//debug("**** End frame ****");
}


#define POLY_DRAW_CODE 1


void MartianDuct::doDraw(int numPoints) {

#if POLY_DRAW_CODE
	//
	// This code is not as efficient as the original game, but it's
	// easier to understand.
	//
	// We have a set of line segments in the _prim**Array arrays and want
	// to draw the polygons that they form, but they are not necessarily
	// in order or handedness.  First remove null segments, then sort the
	// remaining ones by connecting up the start and end points.  Since they
	// are all simple shapes this works fine.
	//

	// First remove null lines (same start and end)
	for (int i = 0; i < numPoints; i++) {
		if (_primX1Array[i] == _primX2Array[i] && _primY1Array[i] == _primY2Array[i]) {
			_primX1Array.remove_at(i);
			_primY1Array.remove_at(i);
			_primX2Array.remove_at(i);
			_primY2Array.remove_at(i);
			numPoints--;
			i--;
			continue;
		}
	}

	// Sort the line segments
	Common::Array<int> x1s;
	Common::Array<int> y1s;
	Common::Array<int> x2s;
	Common::Array<int> y2s;

	x1s.push_back(_primX1Array.remove_at(0));
	y1s.push_back(_primY1Array.remove_at(0));
	x2s.push_back(_primX2Array.remove_at(0));
	y2s.push_back(_primY2Array.remove_at(0));
	int pointsToAdd = numPoints - 1;
	while (pointsToAdd > 0) {
		// Find the next line segment to add, could be in either direction.
		for (int i = 0; i < pointsToAdd; i++) {
			if (_primX1Array[i] == x2s.back() && _primY1Array[i] == y2s.back()) {
				x1s.push_back(_primX1Array.remove_at(i));
				y1s.push_back(_primY1Array.remove_at(i));
				x2s.push_back(_primX2Array.remove_at(i));
				y2s.push_back(_primY2Array.remove_at(i));
				pointsToAdd--;
				break;
			} else if (_primX2Array[i] == x2s.back() && _primY2Array[i] == y2s.back()) {
				x1s.push_back(_primX2Array.remove_at(i));
				y1s.push_back(_primY2Array.remove_at(i));
				x2s.push_back(_primX1Array.remove_at(i));
				y2s.push_back(_primY1Array.remove_at(i));
				pointsToAdd--;
				break;
			}
		}
	}

	// Complete the shape to join up to the start again.
	x1s.push_back(x1s[0]);
	y1s.push_back(y1s[0]);

	_vm->_buffer2.drawPolygonScan(x1s.data(), y1s.data(), x1s.size(), Common::Rect(320, 200), _vm->_buffer2._lColor);

#else

	int16 maxYVal = 0;
	int16 minYVal = 0xff;
	int i = 0;

	// Count of line segments and x1/x2 pairs for each row
	byte segmentCount[202];
	int16 segmentCoords[200][16];

	ARRAYCLEAR(segmentCount);
	for (int y = 0; y < 200; y++)
		ARRAYCLEAR(segmentCoords[y]);

	do {
		int16 primitiveX = _primXArray[i];
		int16 primitiveY = _primYArray[i];
		int16 primitiveB = _primY2Array[i];
		int16 primitiveR = _primX2Array[i];

		maxYVal = MAX(maxYVal, primitiveY);
		maxYVal = MAX(maxYVal, primitiveB);
		minYVal = MIN(minYVal, primitiveY);
		minYVal = MIN(minYVal, primitiveB);

		if (primitiveR < primitiveX) {
			// Flip X and Y
			SWAP(primitiveX, primitiveR);
			SWAP(primitiveY, primitiveB);
		}
		const int16 width = primitiveR - primitiveX;
		int16 x = primitiveX;
		int16 _lastPrimIdx = i;

		if (width == 0) {
			if (primitiveY != primitiveB) {
				if (primitiveB <= primitiveY)
					SWAP(primitiveY, primitiveB);

				int16 y = primitiveY;
				do {
					//byte bVar6 = (byte)array_a344[y * 0x10];
					//*(int16 *)CONCAT11((char)((uint)(array_a344[y * 0x10] >> 8) + CARRY1(bVar6, byteData_bc44[y]),
					//				 bVar6 + byteData_bc44[y]) = primitiveX;
					segmentCoords[y][segmentCount[y]] = primitiveX;
					segmentCoords[y][segmentCount[y] + 1] = primitiveX;
					segmentCount[y]++;
					y++;
				} while (y != primitiveB);
			}
		} else {
			const int height = primitiveB - primitiveY;
			if (height < 0) {
				if (-height < width) {
					i = -width / 2;
					int16 y = primitiveY;
					while (true) {
						do {
							x++;
							i -= height;
						} while (i < 0);
						int16 nextY = y - 1;
						if (nextY == primitiveB)
							break;
						//byte bVar6 = (byte)array_a344[nextY * 0x10];
						//*(int16 *)CONCAT11((char)((uint)(array_a344[nextY * 0x10] >> 8) +
						//			   CARRY1(bVar6,byteData_bc44[y]),
						//			   bVar6 + byteData_bc44[nextY]) = x;
						segmentCoords[y][segmentCount[y]] = primitiveX;
						segmentCoords[y][segmentCount[y] + 1] = x;
						segmentCount[y]++;
						i -= width;
						y = nextY;
					}
				} else {
					i = height / 2;
					int16 y = primitiveY;
					int16 nextY;
					while (nextY = y - 1, nextY != primitiveB) {
						//byte bVar6 = (byte)array_a344[nextY * 0x10];
						//*(int16 *)CONCAT11((char)((uint)(array_a344[nextY * 0x10] >> 8) +
						//			   CARRY1(bVar6,byteData_bc44[primitiveY]),
						//			   bVar6 + byteData_bc44[primitiveY]) = x;
						segmentCount[-y]++;
						i += width;
						y = nextY;
						if (-1 < i) {
							x++;
							i += height;
						}
					}
				}

				SWAP(primitiveX, primitiveR);
				SWAP(primitiveY, primitiveB);
				//byte bVar6 = (byte)array_a344[primitiveY * 0x10];
				//*(int16 *)CONCAT11((char)((uint)(array_a344[primitiveY * 0x10] >> 8) +
				//			 CARRY1(bVar6,byteData_bc44[primitiveY]),
				//			 bVar6 + byteData_bc44[primitiveY]) = primitiveX;
				segmentCount[primitiveY]++;
			} else if (height != 0 && height < width) {
				i = -width / 2;
				int16 y = primitiveY;
				while (true) {
					do {
						x++;
						i += height;
					} while (i < 0);
					//byte bVar6 = (byte)array_a344[y * 0x10];
					//*(int16 *)CONCAT11((char)((uint)(array_a344[y * 0x10] >> 8) +
					//				 CARRY1(bVar6,byteData_bc44[y]),
					//		 bVar6 + byteData_bc44[y]) = x;
					segmentCount[y]++;
					y++;
					if (y == primitiveB)
						break;
					i -= width;
				}
			} else if (height != 0) {
				i = -height / 2;
				int16 y = primitiveY;
				while (true) {
					//byte bVar6 = (byte)array_a344[y * 0x10];
					//*(int16 *)CONCAT11((char)((uint)(array_a344[y * 0x10] >> 8) +
					//				 CARRY1(bVar6,byteData_bc44[y]),
					//				 bVar6 + byteData_bc44[y]) = x;
					segmentCount[y]++;
					y++;
					if (y == primitiveB)
						break;
					i += width;
					if (-1 < i) {
						x++;
						i -= height;
					}
				}
			}
		}
		i = _lastPrimIdx + 1;
	} while (i != numPoints);

	segmentCount[200] = segmentCount[36];
	for (int16 y = minYVal; y <= maxYVal; y++) {
		if (segmentCount[y] == 0)
			continue;

		int16 *lineData = segmentCoords[y];
		/*
		CHECK ME: This probably is supposed to swap incorrect left/right vals, but seems
		to just check values against themselves?
		const int16 numSegs = segmentCount[y];
		int16 right = 0;
		int16 left = 0;
		do {
			const int16 tmpy = lineData[right];
			if (tmpy < lineData[left]) {
				lineData[right] = lineData[left];
				lineData[left] = tmpy;
			}
			right++;
		} while ((right != numSegs) || (right = left + 1, left = right, right != numSegs));
		*/
		for (int16 segNum = 0; segNum < segmentCount[y]; segNum++) {
			const int16 x2 = *(lineData + 1);
			const int16 x1 = *lineData;
			lineData += 2;
			byte *pdest = (byte *)_vm->_buffer2.getBasePtr(x1, y);
			for (i = 0; i < (x2 - x1) + 1; i++) {
				*pdest = _vm->_buffer2._lColor;
				pdest++;
			}
		}
		segmentCount[y] = 0;
	}
#endif
}

void MartianDuct::doMatrixMulAndAddPoint(int16 x, int16 y, int16 z) {
	Point3 pt;
	pt.x = _matrix[0][0] * x + _matrix[1][0] * y + _matrix[2][0] * z;
	pt.y = _matrix[0][1] * x + _matrix[1][1] * y + _matrix[2][1] * z;
	pt.z = _matrix[0][2] * x + _matrix[1][2] * y + _matrix[2][2] * z;
	_renderPoints.push_back(pt);
}

bool MartianDuct::doPrimArrayUpdates(int &numPoints) {
	if (checkAndUpdatePrimArray1(numPoints))
		return true;
	if (checkAndUpdatePrimArray2(numPoints))
		return true;
	if (checkAndUpdatePrimArray3(numPoints))
		return true;
	if (checkAndUpdatePrimArray4(numPoints))
		return true;
	if (checkAndUpdatePrimArray5(numPoints))
		return true;

	assert(numPoints > 0);
	for (int16 idx = 0; idx < numPoints; idx++) {
		Point3 pt1, pt2;
		getPointValuesFromArray(idx, pt1, pt2);
		// The original changes the drawing primitive coordinates here
		// but we only need them to store so we use a temp rect instead.
		const Common::Rect r = calcFinalLineSegment(pt1, pt2);
		_primX1Array[idx] = r.left;
		_primY1Array[idx] = r.top;
		_primX2Array[idx] = r.right;
		_primY2Array[idx] = r.bottom;
	}
	return false;
}

void MartianDuct::getPointValuesFromArray(int idx, Point3 &pt1, Point3 &pt2) const {
	pt1.x = _primX1Array[idx];
	pt1.y = _primY1Array[idx];
	pt1.z = _primZ1Array[idx];
	pt2.x = _primX2Array[idx];
	pt2.y = _primY2Array[idx];
	pt2.z = _primZ2Array[idx];
}

Common::Rect MartianDuct::calcFinalLineSegment(const Point3 &pt1, const Point3 &pt2) const {
	Common::Rect result;
	result.left   = _xOffset + ((int)pt1.x * _xScale) / pt1.z;
	result.top    = _yOffset - ((int)pt1.y * _yScale) / pt1.z;
	result.right  = _xOffset + ((int)pt2.x * _xScale) / pt2.z;
	result.bottom = _yOffset - ((int)pt2.y * _yScale) / pt2.z;
	return result;
}

bool MartianDuct::checkAndUpdatePrimArrayForFlag(int &numPoints, DuctFlags flag, int divmulNum) {
	_primArrayIdx = 0;
	int tempCount = 0;
	for (int idx = 0; idx < numPoints; idx++) {
		Point3 pt1, pt2;
		DuctFlags xyflags;
		DuctFlags rbflags;
		getPointValuesFromArray(idx, pt1, pt2);
		getXYandRBFlags(xyflags, rbflags, pt1, pt2);
		if (xyflags == kDuctFlagNone && rbflags == kDuctFlagNone) {
			storeLastValsToPrimArray(pt1, pt2); // increments _primArrayIdx
		} else if (((xyflags | rbflags) & flag) == kDuctFlagNone) {
			storeLastValsToPrimArray(pt1, pt2);
		} else if ((xyflags & rbflags & flag) == kDuctFlagNone) {
			if ((rbflags & flag) == kDuctFlagNone) {
				// Implicitly xyflags & flag != none
				assert((xyflags & flag) != kDuctFlagNone);
				SWAP(pt1, pt2);
			}
			switch (divmulNum) {
				case 1: pt2 = divmul1(pt1, pt2); break;
				case 2: pt2 = divmul2(pt1, pt2); break;
				case 3: pt2 = divmul3(pt1, pt2); break;
				case 4: pt2 = divmul4(pt1, pt2); break;
				case 5: pt2 = divmul5(pt1, pt2); break;
				default: error("Invalid divmul num");
			}
			_tempPoints[tempCount] = pt2;
			tempCount++;
			storeLastValsToPrimArray(pt1, pt2);
		}
	}
	numPoints = addPointsToMainPrimArray(tempCount);
	return numPoints == 0;
}

bool MartianDuct::checkAndUpdatePrimArray1(int &numPoints) {
	return checkAndUpdatePrimArrayForFlag(numPoints, kDuctFlagZLessThan2, 1);
}

bool MartianDuct::checkAndUpdatePrimArray2(int &numPoints) {
	return checkAndUpdatePrimArrayForFlag(numPoints, kDuctFlagXLessThanNegZ, 2);
}

bool MartianDuct::checkAndUpdatePrimArray3(int &numPoints) {
	return checkAndUpdatePrimArrayForFlag(numPoints, kDuctFlagZLessThanY, 3);
}

bool MartianDuct::checkAndUpdatePrimArray4(int &numPoints) {
	return checkAndUpdatePrimArrayForFlag(numPoints, kDuctFlagZLessThanX, 4);
}

bool MartianDuct::checkAndUpdatePrimArray5(int &numPoints) {
	return checkAndUpdatePrimArrayForFlag(numPoints, kDuctFlagYLessThanNegZ, 5);
}

//
// For these functions the original uses some fixed-point stuff, but replaced
// it with float to be much cleaner.
//
Point3 MartianDuct::divmul1(const Point3 &pt1, const Point3 &pt2) {
	// called for kDuctFlagXLessThanNegZ
	Point3 out;
	out.z = 2;
	float tmp = (2.0f - pt1.z) / (pt2.z - pt1.z);
	out.x = (int)round((pt2.x - pt1.x) * tmp + pt1.x);
	out.y = (int)round((pt2.y - pt1.y) * tmp + pt1.y);
	return out;
}

Point3 MartianDuct::divmul2(const Point3 &pt1, const Point3 &pt2) {
	// called for kDuctFlagXLessThanNegZ
	Point3 out;
	float tmp = (pt1.z + pt1.x * 2.0f) / ((pt1.x - pt2.x) * 2 - pt2.z + pt1.z);
	out.z = (int)round((pt2.z - pt1.z) * tmp + pt1.z);
	out.x = -(out.z / 2);
	out.y = (int)round((pt2.y - pt1.y) * tmp + pt1.y);
	return out;
}

Point3 MartianDuct::divmul3(const Point3 &pt1, const Point3 &pt2) {
	// called for kDuctFlagZLessThanY
	Point3 out;
	float tmp = (pt1.z - pt1.y * 2.0f) / ((pt2.y - pt1.y) * 2 - pt2.z + pt1.z);
	out.y = (int)round((pt2.z - pt1.z) * tmp + pt1.z);
	out.z = out.y;
	out.x = (int)round((pt2.x - pt1.x) * tmp + pt1.x);
	return out;
}

Point3 MartianDuct::divmul4(const Point3 &pt1, const Point3 &pt2) {
	// called for kDuctFlagZLessThanX
	Point3 out;
	float tmp = (pt1.z - pt1.x * 2.0f) / ((pt2.x - pt1.x) * 2 - pt2.z + pt1.z);
	out.z = (int)round((pt2.z - pt1.z) * tmp + pt1.z);
	out.x = out.z / 2;
	out.y = (int)round((pt2.y - pt1.y) * tmp + pt1.y);
	return out;
}

Point3 MartianDuct::divmul5(const Point3 &pt1, const Point3 &pt2) {
	// called for kDuctFlagYLessThanNegZ
	Point3 out;
	float tmp = (pt1.z + pt1.y * 2.0f) / ((pt1.y - pt2.y) * 2 - pt2.z + pt1.z);
	out.z = (int)round((pt2.z - pt1.z) * tmp + pt1.z);
	out.y = -(out.z / 2);
	out.x = (int)round((pt2.x - pt1.x) * tmp + pt1.x);
	return out;
}


void MartianDuct::getXYandRBFlags(DuctFlags &xyflags, DuctFlags &rbflags, const Point3 &pt1, const Point3 &pt2) {
	xyflags = getComparisonFlags(pt1.x * 2, pt1.y * 2, pt1.z);
	rbflags = getComparisonFlags(pt2.x * 2, pt2.y * 2, pt2.z);
}

int MartianDuct::addPointsToMainPrimArray(int tempCount) {
	int dstIdx = _primArrayIdx;
	// Resize arrays if we need to.
	if (dstIdx + tempCount > (int)_primX1Array.size()) {
		_primX1Array.resize(dstIdx + tempCount);
		_primY1Array.resize(dstIdx + tempCount);
		_primZ1Array.resize(dstIdx + tempCount);
		_primX2Array.resize(dstIdx + tempCount);
		_primY2Array.resize(dstIdx + tempCount);
		_primZ2Array.resize(dstIdx + tempCount);
	}

    for (int srcidx = 0; srcidx < tempCount; srcidx += 2) {
		_primX1Array[dstIdx] = _tempPoints[srcidx].x;
		_primY1Array[dstIdx] = _tempPoints[srcidx].y;
		_primZ1Array[dstIdx] = _tempPoints[srcidx].z;
		_primX2Array[dstIdx] = _tempPoints[srcidx + 1].x;
		_primY2Array[dstIdx] = _tempPoints[srcidx + 1].y;
		_primZ2Array[dstIdx] = _tempPoints[srcidx + 1].z;
		dstIdx++;
	}

	return dstIdx;
}

DuctFlags MartianDuct::getComparisonFlags(int16 x, int16 y, int16 z) {
	enum DuctFlags flags = kDuctFlagNone;
	if (z < 2)
		flags = kDuctFlagZLessThan2;

	if (z < y)
		flags = (DuctFlags)(flags | kDuctFlagZLessThanY);

	if (z < x)
		flags = (DuctFlags)(flags | kDuctFlagZLessThanX);

	if (x < -z)
		flags = (DuctFlags)(flags | kDuctFlagXLessThanNegZ);

	if (y < -z)
		flags = (DuctFlags)(flags | kDuctFlagYLessThanNegZ);

	return flags;
}

void MartianDuct::waitForMoveUpdate() {
	// Holding down Insert and O together skips the duct in the original.
	// Simplify to just accept O.
	if (_vm->_events->peekKeyCode() == Common::KEYCODE_o) {
		_vm->_room->_function = FN_CLEAR1;
		_vm->_flags[0x55] = 1;
		_vm->_flags[0x62] = 1;
		_stopMoveLoop = true;
		return;
	}

	_stopMoveLoop = false;
	// Wait for next frame.
	// Frame rate is set as 100fps, but we really want more like 30.
	_vm->_events->delayUntilNextFrame();
	_vm->_events->delayUntilNextFrame();
	_vm->_events->delayUntilNextFrame();
	_vm->_events->pollEvents();
	Common::CustomEventType action = _vm->_events->peekAction();

	Common::Array<Common::Rect> btnCoords;
	for (int i = 0; Martian::DUCT_ARROW_BUTTON_RANGE[i][0] != -1; i += 2) {
		// DUCT_ARROW_BUTTON_RANGE is min/max X, min/max Y
		btnCoords.push_back(Common::Rect(
			Martian::DUCT_ARROW_BUTTON_RANGE[i][0],
			Martian::DUCT_ARROW_BUTTON_RANGE[i + 1][0],
			Martian::DUCT_ARROW_BUTTON_RANGE[i][1],
			Martian::DUCT_ARROW_BUTTON_RANGE[i + 1][1]));
	}

	// Note: buttons are 0 = up, 1 = left, 2 = down, 3 = right
	int hitButton = -1;
	if (_vm->_events->_leftButton)
		hitButton = _vm->_events->checkMouseBox1(btnCoords);

	if (action == kActionMoveUp || hitButton == 0) {
		_moveIntent = kMoveIntentUp;
		drawArrowSprites2();
		if (updateMapLocation()) {
			checkFinished();
			return;
		}
		drawArrowSprites2();
		if (_moveAngle == kMoveAngleNorth) {
			_playerY += 7;
		} else if (_moveAngle == kMoveAngleSouth) {
			_playerY -= 7;
		} else if (_moveAngle == kMoveAngleEast) {
			_playerX += 7;
		} else if (_moveAngle == kMoveAngleWest) {
			_playerX -= 7;
		} else {
			checkFinished();
			return;
		}
		_stopMoveLoop = true;
		_crawlFrame++;
		if (_crawlFrame == 7)
			_crawlFrame = 0;

		checkFinished();
		return;
	} else if (action == kActionMoveDown || hitButton == 2) {
		_moveIntent = kMoveIntentDown;
		drawArrowSprites2();
		if (updateMapLocation()) {
			checkFinished();
			return;
		}

		drawArrowSprites2();
		if (_moveAngle == kMoveAngleNorth) {
			_playerY -= 7;
		} else if (_moveAngle == kMoveAngleSouth) {
			_playerY += 7;
		} else if (_moveAngle == kMoveAngleEast) {
			_playerX -= 7;
		} else if (_moveAngle == kMoveAngleWest) {
			_playerX += 7;
		} else {
			checkFinished();
			return;
		}
		_stopMoveLoop = true;
		_crawlFrame--;
		if (_crawlFrame < 0)
			_crawlFrame = 6;

		checkFinished();
		return;
	} else if (action == kActionMoveLeft || hitButton == 1) {
		_moveIntent = kMoveIntentLeft;
		// Action handled, clear it
		_vm->_events->getAction(action);
		updateMapLocation();
	} else if (action == kActionMoveRight || hitButton == 3) {
		_moveIntent = kMoveIntentRight;
		// Action handled, clear it
		_vm->_events->getAction(action);
		updateMapLocation();
	} else {
		checkFinished();
		return;
	}

	_moveAngle = (MoveAngle)(_moveAngle & 0xff);
	drawArrowSprites();
	drawArrowSprites2();
	_drawDistX = 200;
	_drawDistY = 500;
	if (_moveAngle != kMoveAngleNorth && _moveAngle != kMoveAngleSouth) {
		_drawDistX = 500;
		_drawDistY = 200;
	}
	_stopMoveLoop = true;
}

void MartianDuct::checkFinished() {
	// Check the player position against the exits
	if (abs(_playerX - 2650) < 50 && abs(_playerY - 1050) < 50) {
		_vm->_flags[98] = 0;
		_vm->_room->_function = FN_CLEAR1;
	}
	else if (abs(_playerX - 550) < 50 && abs(_playerY - 750) < 50) {
		// Finished!
		_vm->_flags[98] = 1;
		_vm->_flags[85] = 1;
		_vm->_room->_function = FN_CLEAR1;
	}
}

bool MartianDuct::updateMapLocation() {
	uint16 blk = _threshold1 * 2;
	_nextPlayerX = (_playerX / blk) * blk + _threshold1;
	_nextPlayerY = (_playerY / blk) * blk + _threshold1;
	const DuctMapPoint *mapPt = DUCT_MAP_DATA;
	do {
		const int16 pyType = mapPt->ptType;
		if (pyType == -1)
			return true;

		if (pyType != 0xff) {
			_mapLoc = pyType;

			int16 thresh = _threshold1;
			if (pyType == 6 ||  pyType == 10)
				thresh = -_threshold1;

			if (abs((mapPt->x + thresh) - _nextPlayerX) <= _threshold1) {
				thresh = _threshold1;
				if (pyType == 8)
					thresh = -_threshold1;

				if (abs((mapPt->y + thresh) - _nextPlayerY) <= _threshold1) {
					switch (pyType) {
					case 0: return checkMove0();
					case 1: return checkMove1();
					case 2: return checkMove2();
					case 3: return checkMove3();
					case 4: return checkMove4();
					case 5: return checkMove5();
					case 6: return checkMove6();
					case 7: return checkMove7();
					case 8: return checkMove8();
					case 9: return checkMove9();
					case 10: return checkMove10();
					case 11: return checkMove11();
					case 12: return checkMove12();
					case 13: return checkMove13_14();
					case 14: return checkMove13_14();
					default: error("Unexpected point type in duct map data %d", pyType); return false;
					}
				}
			}
		}
		mapPt++;
	} while( true );

}

void MartianDuct::storeLastValsToPrimArray(const Point3 &pt1, const Point3 &pt2) {
	_primX1Array[_primArrayIdx] = pt1.x;
	_primY1Array[_primArrayIdx] = pt1.y;
	_primZ1Array[_primArrayIdx] = pt1.z;
	_primX2Array[_primArrayIdx] = pt2.x;
	_primY2Array[_primArrayIdx] = pt2.y;
	_primZ2Array[_primArrayIdx] = pt2.z;
	_primArrayIdx++;
}

bool MartianDuct::checkMove0() {
	if (_moveIntent == kMoveIntentRight || _moveIntent == kMoveIntentLeft) {
		_moveAngle = (MoveAngle)(_moveAngle + kMoveAngleSouth);
		return false;
	}

	if (_moveIntent == kMoveIntentUp) {
		if (_moveAngle == kMoveAngleWest)
			return true;

	} else if (_moveIntent == kMoveIntentDown && _moveAngle == kMoveAngleEast) {
		return true;
	}
	return false;
}

bool MartianDuct::checkMove1() {
	if ((_moveIntent == kMoveIntentRight) || (_moveIntent == kMoveIntentLeft)) {
		_moveAngle = (MoveAngle)(_moveAngle + kMoveAngleSouth);
		return false;
	}

	if (_moveIntent == kMoveIntentUp) {
		if (_moveAngle == kMoveAngleEast)
			return true;
	} else if (_moveIntent == kMoveIntentDown && _moveAngle == kMoveAngleWest) {
		return true;
	}
	return false;
}

bool MartianDuct::checkMove2() {
	if (_moveIntent == kMoveIntentRight || _moveIntent == kMoveIntentLeft) {
		_moveAngle = (MoveAngle)(_moveAngle + kMoveAngleSouth);
		return false;
	}
	if (_moveIntent == kMoveIntentUp) {
		if (_moveAngle == kMoveAngleSouth)
			return true;
	} else if (_moveIntent == kMoveIntentDown && _moveAngle == kMoveAngleNorth) {
		return true;
	}
	return false;
}

bool MartianDuct::checkMove3() {
	if (_moveIntent == kMoveIntentRight || _moveIntent == kMoveIntentLeft) {
		_moveAngle = (MoveAngle)(_moveAngle + kMoveAngleSouth);
		return false;
	}
	if (_moveIntent == kMoveIntentUp) {
		if (_moveAngle == kMoveAngleNorth)
			return true;

	} else if (_moveIntent == kMoveIntentDown && _moveAngle == kMoveAngleSouth) {
		return true;
	}
	return false;
}

bool MartianDuct::checkMove4() {
	if (_moveIntent == kMoveIntentRight) {
		_moveAngle = (MoveAngle)(_moveAngle + kMoveAngleEast);
	} else {
		if (_moveIntent != kMoveIntentLeft)
			return false;

		_moveAngle = (MoveAngle)(_moveAngle - kMoveAngleEast);
	}
	_playerY = _nextPlayerY;
	_playerX = _nextPlayerX;
	return false;
}

bool MartianDuct::checkMove5() {
	if (_moveIntent == kMoveIntentRight) {
		if (_moveAngle == kMoveAngleSouth)
			return true;

		_moveAngle = (MoveAngle)(_moveAngle + kMoveAngleEast);
	} else {
		if (_moveIntent != kMoveIntentLeft) {
			if (_moveIntent == kMoveIntentUp)
				return _moveAngle == kMoveAngleWest;

			if (_moveIntent != kMoveIntentDown)
				return true;

			if (_moveAngle != kMoveAngleEast)
				return false;

			return true;
		}
		if (_moveAngle == kMoveAngleNorth)
			return true;

		_moveAngle = (MoveAngle)(_moveAngle - kMoveAngleEast);
	}
	_playerY = _nextPlayerY;
	_playerX = _nextPlayerX;
	return false;
}

bool MartianDuct::checkMove6() {
	if (_moveIntent == kMoveIntentRight) {
		if (_moveAngle == kMoveAngleNorth)
			return true;

		_moveAngle = (MoveAngle)(_moveAngle + kMoveAngleEast);
	} else {
		if (_moveIntent != kMoveIntentLeft) {
			if (_moveIntent == kMoveIntentUp)
				return _moveAngle == kMoveAngleEast;

			if (_moveIntent != kMoveIntentDown)
				return true;

			if (_moveAngle != kMoveAngleWest)
				return false;

			return true;
		}
		if (_moveAngle == kMoveAngleSouth)
			return true;

		_moveAngle = (MoveAngle)(_moveAngle - kMoveAngleEast);
	}
	_playerY = _nextPlayerY;
	_playerX = _nextPlayerX;
	return false;
}

bool MartianDuct::checkMove7() {
	if (_moveIntent == kMoveIntentRight) {
		if (_moveAngle == kMoveAngleEast)
			return true;

		_moveAngle = (MoveAngle)(_moveAngle + kMoveAngleEast);
	}
	else {
		if (_moveIntent != kMoveIntentLeft) {
			if (_moveIntent == kMoveIntentUp)
				return _moveAngle == kMoveAngleSouth;

			if (_moveIntent != kMoveIntentDown)
				return true;

			if (_moveAngle != kMoveAngleNorth)
				return false;

			return true;
		}
		if (_moveAngle == kMoveAngleWest)
			return true;

		_moveAngle = (MoveAngle)(_moveAngle - kMoveAngleEast);
	}
	_playerY = _nextPlayerY;
	_playerX = _nextPlayerX;
	return false;
}

bool MartianDuct::checkMove8() {
	if (_moveIntent == kMoveIntentRight) {
		if (_moveAngle == kMoveAngleWest)
			return true;

		_moveAngle = (MoveAngle)(_moveAngle + kMoveAngleEast);
	} else {
		if (_moveIntent != kMoveIntentLeft) {
			if (_moveIntent == kMoveIntentUp)
				return _moveAngle == kMoveAngleNorth;

			if (_moveIntent != kMoveIntentDown)
				return true;

			if (_moveAngle != kMoveAngleSouth)
				return false;

			return true;
		}

		if (_moveAngle == kMoveAngleEast)
			return true;

		_moveAngle = (MoveAngle)(_moveAngle - kMoveAngleEast);
	}
	_playerY = _nextPlayerY;
	_playerX = _nextPlayerX;
	return false;
}

bool MartianDuct::checkMove9() {
	if (_moveIntent == kMoveIntentRight) {
		if ((_moveAngle != kMoveAngleNorth) && (_moveAngle != kMoveAngleEast)) {
			return true;
		}
		_moveAngle = (MoveAngle)(_moveAngle + kMoveAngleEast);
	} else {
		if (_moveIntent != kMoveIntentLeft) {
			if (_moveIntent == kMoveIntentUp)
				return _moveAngle == kMoveAngleWest || _moveAngle == kMoveAngleNorth;

			if (_moveIntent != kMoveIntentDown)
				return true;

			if (_moveAngle != kMoveAngleEast)
				return _moveAngle == kMoveAngleSouth;

			return true;
		}
		if (_moveAngle != kMoveAngleWest && _moveAngle != kMoveAngleSouth)
			return true;

		_moveAngle = (MoveAngle)(_moveAngle - kMoveAngleEast);
	}
	_playerY = _nextPlayerY;
	_playerX = _nextPlayerX;
	return false;
}

bool MartianDuct::checkMove10() {
	if (_moveIntent == kMoveIntentRight) {
		if (_moveAngle != kMoveAngleEast && _moveAngle != kMoveAngleSouth)
			return true;

		_moveAngle = (MoveAngle)(_moveAngle + kMoveAngleEast);
	} else {
		if (_moveIntent != kMoveIntentLeft) {
			if (_moveIntent == kMoveIntentUp)
				return _moveAngle == kMoveAngleEast || _moveAngle == kMoveAngleNorth;

			if (_moveIntent != kMoveIntentDown)
				return true;

			if (_moveAngle != kMoveAngleWest)
				return _moveAngle == kMoveAngleSouth;

			return true;
		}
		if (_moveAngle != kMoveAngleNorth && _moveAngle != kMoveAngleWest)
			return true;

		_moveAngle = (MoveAngle)(_moveAngle - kMoveAngleEast);
	}
	_playerY = _nextPlayerY;
	_playerX = _nextPlayerX;
	return false;
}

bool MartianDuct::checkMove11() {
	if (_moveIntent == kMoveIntentRight) {
		if (_moveAngle != kMoveAngleWest && _moveAngle != kMoveAngleNorth)
			return true;

		_moveAngle = (MoveAngle)(_moveAngle + kMoveAngleEast);
	} else {
		if (_moveIntent != kMoveIntentLeft) {
			if (_moveIntent == kMoveIntentUp)
				return _moveAngle == kMoveAngleWest || _moveAngle == kMoveAngleSouth;

			if (_moveIntent != kMoveIntentDown) {
				return true;
			}
			if (_moveAngle != kMoveAngleNorth) {
				return _moveAngle == kMoveAngleEast;
			}
			return true;
		}
		if (_moveAngle != kMoveAngleSouth && _moveAngle != kMoveAngleEast)
			return true;

		_moveAngle = (MoveAngle)(_moveAngle - kMoveAngleEast);
	}
	_playerY = _nextPlayerY;
	_playerX = _nextPlayerX;
	return false;
}

bool MartianDuct::checkMove12() {
	if (_moveIntent == kMoveIntentRight) {
		if (_moveAngle != kMoveAngleSouth && _moveAngle != kMoveAngleWest)
			return true;

		_moveAngle = (MoveAngle)(_moveAngle + kMoveAngleEast);
	} else {
		if (_moveIntent != kMoveIntentLeft) {
			if (_moveIntent == kMoveIntentUp)
				return _moveAngle == kMoveAngleEast || _moveAngle == kMoveAngleSouth;

			if (_moveIntent != kMoveIntentDown)
				return true;

			if (_moveAngle != kMoveAngleNorth)
				return _moveAngle == kMoveAngleWest;

			return true;
		}
		if (_moveAngle != kMoveAngleEast && _moveAngle != kMoveAngleNorth)
			return true;

		_moveAngle = (MoveAngle)(_moveAngle - kMoveAngleEast);
	}
	_playerY = _nextPlayerY;
	_playerX = _nextPlayerX;
	return false;
}

bool MartianDuct::checkMove13_14() {
	if (_moveIntent != kMoveIntentRight && _moveIntent != kMoveIntentLeft)
		return false;

	_moveAngle = (MoveAngle)(_moveAngle + kMoveAngleSouth);
	return false;
}



}

} // end namespace Access
