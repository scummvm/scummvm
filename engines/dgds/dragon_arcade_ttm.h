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

#ifndef DGDS_DRAGON_ARCADE_TTM_H
#define DGDS_DRAGON_ARCADE_TTM_H

#include "common/types.h"
#include "common/ptr.h"

#include "dgds/image.h"
#include "dgds/ttm.h"

namespace Dgds {

/**
 * A segment of floor in the arcade section with a start x, width, height, and flag.
 */
class ArcadeFloor {
public:
	ArcadeFloor() : x(0), width(0), yval(0), flag(false) {}
	int16 x;
	int16 width;
	byte yval;
	bool flag;

	Common::String dump();
};


/**
The regular TTM interpreter always uses the environment's shape for the brushes,
but the arcade one stores brushes as pointers to the exact frame within the shape.

In practice this may make no difference - maybe we can track use shapes2 and
store the frame number?
*/
class Brush {
public:
	Brush() : _frame(0) {}
	Brush(const Common::SharedPtr<Image> &shape, int16 frame) : _shape(shape), _frame(frame) {}

	void reset() {
		_shape.reset();
		_frame = 0;
	}
	bool isValid() const { return _shape && _shape->loadedFrameCount() > _frame; }
	const Common::SharedPtr<Image> &getShape() const { return _shape; }
	int16 getFrame() const { return _frame; }
private:
	Common::SharedPtr<Image> _shape;
	int16 _frame;
};

struct ArcadeNPCState;

/** A TTM interpreter which is simpler than the main one and
   specialized to the arcade sequences. */
class DragonArcadeTTM {
public:
	DragonArcadeTTM(ArcadeNPCState *npcState);
	void clearDataPtrs();
	int16 load(const char *filename);
	void finishTTMParse(int16 envNum);
	int16 runNextPage(int16 pageNum);
	void freePages(uint16 num);
	void freeShapes();
	void runPagesForEachNPC(int16 xScrollOffset);
	const Common::Array<ArcadeFloor> &getFloorData() { return _floorData; }

	uint16 _currentTTMNum;
	int16 _currentNPCRunningTTM;
	int16 _drawXOffset;
	int16 _drawYOffset;
	int16 _startYOffset;
	bool _doingInit;

private:
	int16 runScriptPage(int16 pageNum);
	int16 handleOperation(TTMEnviro &env, int16 page, uint16 op, byte count, const int16 *ivals, const Common::String &sval);

	int16 _shapes3[6];
	Common::SharedPtr<Image> _shapes[6];
	Common::SharedPtr<Image> _shapes2[6];
	Common::SharedPtr<Image> _allShapes[30];
	Brush _brushes[6];

	byte _drawColFG;
	byte _drawColBG;
	ArcadeNPCState *_npcState;
	// int16 _numA1x4OpsInInit; // implicit by count of items in _floorData
	Common::Array<ArcadeFloor> _floorData;

	// Note: only a subset of the enviro members get used, but
	// use the same structure for simplicity.
	TTMEnviro _ttmEnvs[5];

};

} // end namespace Dgds

#endif // DGDS_DRAGON_ARCADE_TTM_H
