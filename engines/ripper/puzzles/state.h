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
 */

#ifndef RIPPER_PUZZLES_STATE_H
#define RIPPER_PUZZLES_STATE_H

#include "common/rect.h"

namespace Common {
class Serializer;
}

namespace Ripper {

struct ClockPuzzleState {
	ClockPuzzleState();

	int dialIndices[2];
	int firstClockDigits[4];
	int secondClockDigits[4];
	bool halfDay;
};

struct BoardArrangementPuzzleState {
	BoardArrangementPuzzleState();

	Common::Point positions[8];
};

struct KeyGroupPuzzleState {
	KeyGroupPuzzleState();

	int sourceSlots[20];
	int targetSlots[14];
};

struct PuzzleState {
	bool syncGame(Common::Serializer &serializer);

	ClockPuzzleState clock;
	BoardArrangementPuzzleState boardArrangement;
	KeyGroupPuzzleState keyGroup;
};

} // End of namespace Ripper

#endif // RIPPER_PUZZLES_STATE_H
