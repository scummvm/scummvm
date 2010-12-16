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
 * $URL$
 * $Id$
 *
 */

#ifndef ASYLUM_TIMEMACHINE_H
#define ASYLUM_TIMEMACHINE_H

#include "asylum/puzzles/puzzle.h"

namespace Asylum {

class AsylumEngine;

class PuzzleTimeMachine : public Puzzle {
public:
	PuzzleTimeMachine(AsylumEngine *engine);
	~PuzzleTimeMachine();

	void reset();

private:
	bool _leftButtonClicked;
	uint32 _counter;
	uint32 _counter2;
	uint32 _currentFrameIndex;
	uint32 _frameIndexes[6];
	uint32 _frameCounts[6];
	int32 _frameIncrements[6];
	int8 _state[5];

	bool _data_4572BC;
	bool _data_4572CC;

	uint32 _data_45AAA8;
	uint32 _data_45AAAC;

	//////////////////////////////////////////////////////////////////////////
	// Event Handling
	//////////////////////////////////////////////////////////////////////////
	bool init();
	bool update();
	bool key(const AsylumEvent &evt);
	bool mouse(const AsylumEvent &evt);

	void mouseDown();

	//////////////////////////////////////////////////////////////////////////
	// Helpers
	//////////////////////////////////////////////////////////////////////////
	void updateCursor();

};

} // End of namespace Asylum

#endif // ASYLUM_TIMEMACHINE_H
