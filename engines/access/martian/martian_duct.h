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

#ifndef ACCESS_MARTIAN_MARTIAN_DUCT_H
#define ACCESS_MARTIAN_MARTIAN_DUCT_H

#include "common/scummsys.h"
#include "common/array.h"
#include "common/rect.h"
#include "common/events.h"

#include "access/martian/martian_resources.h" // For Point3 .. move it?

namespace Access {

namespace Martian {

enum MoveIntent {
	kMoveIntentNone,
	kMoveIntentUp,
	kMoveIntentLeft,
	kMoveIntentDown,
	kMoveIntentRight,
};

// Move angles - in the original these are indexes into sin/cos lookup tables.
// Add Invalid to ensure at least 2-byte length.
enum MoveAngle {
	kMoveAngleNorth = 0,
	kMoveAngleEast = 0x40,
	kMoveAngleSouth = 0x80,
	kMoveAngleWest = 0xC0,
	kMoveAngleInvalid = 0xffff,
};

enum DuctFlags {
	kDuctFlagNone = 0,
	kDuctFlagZLessThanX = 1,
	kDuctFlagXLessThanNegZ = 2,
	kDuctFlagYLessThanNegZ = 4,
	kDuctFlagZLessThanY = 8,
	kDuctFlagZLessThan2 = 16,
};

struct RenderShape {
	byte _col;
	Common::Array<uint16> _pointIdxs;
};

class MartianEngine;

class MartianDuct {
public:
	MartianDuct(MartianEngine *vm);
	~MartianDuct();

	void duct2();
	void duct4();

private:
	void doDuct();
	void drawArrowSprites();
	void drawArrowSprites2();
	void clearWorkScreenArea();
	void copyBufBlockToScreen();
	void waitForMoveUpdate();
	void storeLastValsToPrimArray(const Point3 &pt1, const Point3 &pt2);
	void updatePlayerPos();
	void updateMatrix();
	void applyMatrixToMapData();
	void updatePrimsAndDraw();
	bool updateMapLocation();
	void checkFinished();
	void doMatrixMulAndAddPoint(int16 x, int16 y, int16 z);
	bool doPrimArrayUpdates(int &tempIdx);
	void doDraw(int counter);

	void getPointValuesFromArray(int offset, Point3 &pt1, Point3 &pt2) const;
	Common::Rect calcFinalLineSegment(const Point3 &pt1, const Point3 &pt2) const;

	bool checkAndUpdatePrimArray1(int &offset);
	bool checkAndUpdatePrimArray2(int &offset);
	bool checkAndUpdatePrimArray3(int &offset);
	bool checkAndUpdatePrimArray4(int &offset);
	bool checkAndUpdatePrimArray5(int &offset);
	bool checkAndUpdatePrimArrayForFlag(int &offset, DuctFlags flag, int divmulNum);

	static Point3 divmul1(const Point3 &pt1, const Point3 &pt2);
	static Point3 divmul2(const Point3 &pt1, const Point3 &pt2);
	static Point3 divmul3(const Point3 &pt1, const Point3 &pt2);
	static Point3 divmul4(const Point3 &pt1, const Point3 &pt2);
	static Point3 divmul5(const Point3 &pt1, const Point3 &pt2);

	bool checkMove0();
	bool checkMove1();
	bool checkMove2();
	bool checkMove3();
	bool checkMove4();
	bool checkMove5();
	bool checkMove6();
	bool checkMove7();
	bool checkMove8();
	bool checkMove9();
	bool checkMove10();
	bool checkMove11();
	bool checkMove12();
	bool checkMove13_14();

	void getXYandRBFlags(DuctFlags &xyflags, DuctFlags &rbflags, const Point3 &pt1, const Point3 &pt2);
	int addPointsToMainPrimArray(int tempCount);
	static DuctFlags getComparisonFlags(int16 x, int16 y, int16 z);

	MartianEngine *_vm;
	int16 _playerX;
	int16 _preYOffset;
	int16 _playerY;

	int16 _nextPlayerX;
	int16 _nextPlayerY;
	int16 _xOffset;
	int16 _yOffset;
	int16 _xScale;
	int16 _yScale;
	uint16 _mapLoc;
	MoveAngle _moveAngle;
	int16 _crawlFrame;
	bool _stopMoveLoop;
	int16 _threshold1;
	int16 _drawDistX;
	int16 _drawDistY;
	MoveIntent _moveIntent;

	// NOTE: Original uses fixed point sin/cos with a lookup table.
	float _matrix[3][3];

	int16 _primArrayIdx;

	Common::Array<Point3> _renderPoints;
	Common::Array<RenderShape> _renderShapes;

	Common::Array<int16> _primX1Array;
	Common::Array<int16> _primY1Array;
	Common::Array<int16> _primZ1Array;
	Common::Array<int16> _primX2Array;
	Common::Array<int16> _primY2Array;
	Common::Array<int16> _primZ2Array;
	Point3 _tempPoints[32];
};

}

} // end namespace Access

#endif // ACCESS_MARTIAN_MARTIAN_DUCT_H
