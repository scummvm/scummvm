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

 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.

 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 *
 */

#ifndef ASYLUM_PUZZLES_WHEEL_H
#define ASYLUM_PUZZLES_WHEEL_H

#include "asylum/puzzles/puzzle.h"

namespace Asylum {

class AsylumEngine;

class PuzzleWheel : public Puzzle {
public:
	PuzzleWheel(AsylumEngine *engine);
	~PuzzleWheel();

	// Serializable
	virtual void saveLoadWithSerializer(Common::Serializer &s);

private:
	int32 _currentRect;
	int32 _resourceIndex;
	uint32 _resourceIndexClock;
	uint32 _resourceIndexLever;
	uint32 _frameIndexWheel;
	int32 _frameIndexes[12];
	int32 _frameIndexesSparks[8];

	bool _showTurnedClock;
	bool _turnWheelRight;
	bool _moveLever;
	bool _moveChain;

	//////////////////////////////////////////////////////////////////////////
	// Event Handling
	//////////////////////////////////////////////////////////////////////////
	bool init(const AsylumEvent &evt);
	void updateScreen();
	bool mouseLeftDown(const AsylumEvent &evt);
	bool mouseRightDown(const AsylumEvent &evt);

	//////////////////////////////////////////////////////////////////////////
	// Helpers
	//////////////////////////////////////////////////////////////////////////
	void updateCursor();
	int32 findRect();
	void updateIndex();
	void checkFlags();
	void closeLocks();
	void toggleLocks();
};

} // End of namespace Asylum

#endif // ASYLUM_PUZZLES_WHEEL_H
