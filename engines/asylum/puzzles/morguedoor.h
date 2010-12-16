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

#ifndef ASYLUM_MORGUEDOOR_H
#define ASYLUM_MORGUEDOOR_H

#include "asylum/puzzles/puzzle.h"

namespace Asylum {

class AsylumEngine;

class PuzzleMorgueDoor : public Puzzle {
public:
	PuzzleMorgueDoor(AsylumEngine *engine);
	~PuzzleMorgueDoor();

private:
	enum PuzzleObject {
		kTopLever = 0,
		kBottomLever = 1,
		kTopLeverOpened = 2,
		kBottomLeverOpened = 3,
		kTopRightValve = 4,
		kCenterValve = 5,
		kRightGear = 6,
		kTopRightLever = 7,
		kTopSmallLever = 8,
		kBottomSmallLever = 9,
		kTopGear = 10,
		kBottomGear = 11
	};

	int32 _frameCounts[12]; // indexes 8 & 9 aren't used
	int32 _frameIndexes[12];
	bool _data_4572A4;
	bool _data_4572A8;
	bool _data_4572AC;
	bool _data_4572B0;

	uint32 _data_45A9D8;
	uint32 _data_45A9DC;

	bool _topLeverOpen;
	bool _bottomLeverOpen;
	bool _moveTopGear;
	bool _moveBottomGear;
	bool _flag5;
	bool _flag6;
	bool _flag7;

	//////////////////////////////////////////////////////////////////////////
	// Event Handling
	//////////////////////////////////////////////////////////////////////////
	bool init();
	bool update();
	bool key(const AsylumEvent &evt);
	bool mouse(const AsylumEvent &evt);

	bool mouseDown();

	//////////////////////////////////////////////////////////////////////////
	// Helpers
	//////////////////////////////////////////////////////////////////////////
	void updateCursor();
	void updateState();
};

} // End of namespace Asylum

#endif // ASYLUM_MORGUEDOOR_H
