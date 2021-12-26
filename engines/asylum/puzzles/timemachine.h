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

#ifndef ASYLUM_PUZZLES_TIMEMACHINE_H
#define ASYLUM_PUZZLES_TIMEMACHINE_H

#include "asylum/puzzles/puzzle.h"

namespace Asylum {

class AsylumEngine;

class PuzzleTimeMachine : public Puzzle {
public:
	PuzzleTimeMachine(AsylumEngine *engine);
	~PuzzleTimeMachine();

	// Serializable
	virtual void saveLoadWithSerializer(Common::Serializer &s);

private:
	bool   _leftButtonClicked;
	uint32 _counter;
	int32  _frameIndexes[6];
	uint32 _frameCounts[6];
	int32  _frameIncrements[5];
	int32  _index;
	uint32 _index2;
	Common::Point _point;
	Common::Point _newPoint;

	// Unused puzzle variables
	int8   _state[5];
	//bool   _data_4572BC;
	//bool   _data_4572CC;
	uint32 _data_45AAA8;
	uint32 _data_45AAAC;
	uint32 _currentFrameIndex;

	//////////////////////////////////////////////////////////////////////////
	// Event Handling
	//////////////////////////////////////////////////////////////////////////
	bool init(const AsylumEvent &evt);
	void updateScreen();
	bool key(const AsylumEvent &evt) { return keyExit(evt); }
	bool mouseLeftDown(const AsylumEvent &evt);
	bool mouseRightDown(const AsylumEvent &evt);

	//////////////////////////////////////////////////////////////////////////
	// Helpers
	//////////////////////////////////////////////////////////////////////////
	void updateCursor();
	void reset();
};

} // End of namespace Asylum

#endif // ASYLUM_PUZZLES_TIMEMACHINE_H
