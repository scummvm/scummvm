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

#ifndef ASYLUM_PUZZLES_CLOCK_H
#define ASYLUM_PUZZLES_CLOCK_H

#include "asylum/puzzles/puzzle.h"

namespace Asylum {

class AsylumEngine;

class PuzzleClock : public Puzzle {
public:
	PuzzleClock(AsylumEngine *engine);
	~PuzzleClock();

	// Serializable
	virtual void saveLoadWithSerializer(Common::Serializer &s);

private:
	uint32 _frameIndexes[3];
	bool _showCursor;
	bool _rightButtonClicked;
	int32 _currentRect;
	uint32 _currentFrameIndex;

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
	void setFlag();
};

} // End of namespace Asylum

#endif // ASYLUM_PUZZLES_CLOCK_H
