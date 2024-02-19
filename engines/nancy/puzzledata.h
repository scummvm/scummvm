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
#include "common/hashmap.h"

#include "engines/nancy/commontypes.h"

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
	virtual ~SliderPuzzleData() {}

	static constexpr uint32 getTag() { return MKTAG('S', 'L', 'I', 'D'); }
	virtual void synchronize(Common::Serializer &ser);

	Common::Array<Common::Array<int16>> playerTileOrder;
	bool playerHasTriedPuzzle;
};

struct RippedLetterPuzzleData : public PuzzleData {
	RippedLetterPuzzleData();
	virtual ~RippedLetterPuzzleData() {}

	static constexpr uint32 getTag() { return MKTAG('R', 'I', 'P', 'L'); }
	virtual void synchronize(Common::Serializer &ser);

	Common::Array<int8> order;
	Common::Array<byte> rotations;
	bool playerHasTriedPuzzle;
};

struct TowerPuzzleData : public PuzzleData {
	TowerPuzzleData();
	virtual ~TowerPuzzleData() {}

	static constexpr uint32 getTag() { return MKTAG('T', 'O', 'W', 'R'); }
	virtual void synchronize(Common::Serializer &ser);

	Common::Array<Common::Array<int8>> order;
	bool playerHasTriedPuzzle;
};

struct RiddlePuzzleData : public PuzzleData {
	RiddlePuzzleData();
	virtual ~RiddlePuzzleData() {}

	static constexpr uint32 getTag() { return MKTAG('R', 'I', 'D', 'L'); }
	virtual void synchronize(Common::Serializer &ser);

	Common::Array<byte> solvedRiddleIDs;
	int8 incorrectRiddleID;
};

struct SoundEqualizerPuzzleData : public PuzzleData {
	SoundEqualizerPuzzleData();
	virtual ~SoundEqualizerPuzzleData() {}

	static constexpr uint32 getTag() { return MKTAG('S', 'E', 'Q', 'L'); }
	virtual void synchronize(Common::Serializer &ser);

	Common::Array<byte> sliderValues;
};

// Contains a single bool indicating whether the puzzle was solved
struct SimplePuzzleData : public PuzzleData {
	SimplePuzzleData();
	virtual ~SimplePuzzleData() {}

	virtual void synchronize(Common::Serializer &ser);

	bool solvedPuzzle;
};

struct AssemblyPuzzleData : public SimplePuzzleData {
	static constexpr uint32 getTag() { return MKTAG('A', 'S', 'M', 'B'); }
};

struct JournalData : public PuzzleData {
	JournalData() {}
	virtual ~JournalData() {}

	struct Entry {
		Entry(const Common::String &s = Common::String(), uint16 m = 0, uint16 sc = kNoScene) : stringID(s), mark(m), sceneID(sc) {}

		Common::String stringID;
		uint16 mark = 0;
		uint16 sceneID = kNoScene;
	};

	static constexpr uint32 getTag() { return MKTAG('J', 'O', 'U', 'R'); }
	virtual void synchronize(Common::Serializer &ser);

	Common::HashMap<uint16, Common::Array<Entry>> journalEntries;
};

// Contains variables that can be read and modified through action records.
// Mixes two separate things:
// - the exhibit data table in nancy6
// - the general variable storage in nancy8 and up
// The exhibit data was only ever used in nancy6, so mixing these should be ok.
struct TableData : public PuzzleData {
	TableData();
	virtual ~TableData() {}

	static constexpr uint32 getTag() { return MKTAG('T', 'A', 'B', 'L'); }
	virtual void synchronize(Common::Serializer &ser);

	void setSingleValue(uint16 index, int16 value);
	int16 getSingleValue(uint16 index) const;

	void setComboValue(uint16 index, float value);
	float getComboValue(uint16 index) const;

	Common::Array<int16> singleValues;
	Common::Array<float> comboValues;
};

PuzzleData *makePuzzleData(const uint32 tag);

} // End of namespace Nancy

#endif // NANCY_PUZZLEDATA_H
