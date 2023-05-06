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

#include "common/serializer.h"
#include "common/array.h"

#ifndef NANCY_PUZZLEDATA_H
#define NANCY_PUZZLEDATA_H

namespace Nancy {

// The following structs contain persistent data for specific
// puzzle types, which is to be stored in savefiles

struct PuzzleData {
	PuzzleData() {}
	virtual ~PuzzleData() {}

	virtual void synchronize(Common::Serializer &ser) = 0;
};

struct SliderPuzzleData : public PuzzleData {
	SliderPuzzleData();

	static constexpr uint32 getTag() { return MKTAG('S', 'L', 'I', 'D'); }
	virtual void synchronize(Common::Serializer &ser);

	Common::Array<Common::Array<int16>> playerTileOrder;
	bool playerHasTriedPuzzle;
};

struct RippedLetterPuzzleData : public PuzzleData {
	RippedLetterPuzzleData();

	static constexpr uint32 getTag() { return MKTAG('R', 'I', 'P', 'L'); }
	virtual void synchronize(Common::Serializer &ser);

	Common::Array<int8> order;
	Common::Array<byte> rotations;
	bool playerHasTriedPuzzle;
};

struct TowerPuzzleData : public PuzzleData {
	TowerPuzzleData();

	static constexpr uint32 getTag() { return MKTAG('T', 'O', 'W', 'R'); }
	virtual void synchronize(Common::Serializer &ser);

	Common::Array<Common::Array<int8>> order;
	bool playerHasTriedPuzzle;
};

struct RiddlePuzzleData : public PuzzleData {
	RiddlePuzzleData();

	static constexpr uint32 getTag() { return MKTAG('R', 'I', 'D', 'L'); }
	virtual void synchronize(Common::Serializer &ser);

	Common::Array<byte> solvedRiddleIDs;
	int8 incorrectRiddleID;
};

PuzzleData *makePuzzleData(const uint32 tag);

} // End of namespace Nancy

#endif // NANCY_PUZZLEDATA_H
