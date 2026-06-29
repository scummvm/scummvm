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
#include "engines/nancy/enginedata.h"

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

	// Temporary values, do not save to file
	int8 _pickedUpPieceID = -1;
	byte _pickedUpPieceRot = 0;
	int _pickedUpPieceLastPos = -1;
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

// Placed bead-type ids on the thread.
struct BeadPuzzleData : public PuzzleData {
	BeadPuzzleData() {}
	virtual ~BeadPuzzleData() {}

	static constexpr uint32 getTag() { return MKTAG('B', 'E', 'A', 'D'); }
	virtual void synchronize(Common::Serializer &ser);

	Common::Array<int16> placedBeads;
};

// Cached current/solved tile layouts for a SortPuzzle. Each cell is encoded as
// 4 consecutive int16s: srcRow, srcCol, value, isEmpty. The first two int16s
// of each array are the grid rows and cols.
struct SortPuzzleData : public PuzzleData {
	SortPuzzleData() {}
	virtual ~SortPuzzleData() {}

	static constexpr uint32 getTag() { return MKTAG('S', 'O', 'R', 'T'); }
	virtual void synchronize(Common::Serializer &ser);

	Common::Array<int16> currentState;
	Common::Array<int16> solvedState;
};

// Per-magnet (left, top, right, bottom, locked) packed as 5 int16s. The
// puzzle's two scenes (3280, 3281) are the same puzzle with the same data,
// so a single flat array suffices.
struct MagnetMazePuzzleData : public PuzzleData {
	MagnetMazePuzzleData() {}
	virtual ~MagnetMazePuzzleData() {}

	static constexpr uint32 getTag() { return MKTAG('M', 'M', 'A', 'Z'); }
	virtual void synchronize(Common::Serializer &ser);

	Common::Array<int16> magnetState;
};

// Per-item (inMap, inItems, mapRow, mapCol, itemsRow, itemsCol) packed as
// 6 int16s.
struct GridMapPuzzleData : public PuzzleData {
	GridMapPuzzleData() {}
	virtual ~GridMapPuzzleData() {}

	static constexpr uint32 getTag() { return MKTAG('G', 'M', 'A', 'P'); }
	virtual void synchronize(Common::Serializer &ser);

	Common::Array<int16> itemState;
};

struct QuizPuzzleData : public PuzzleData {
	QuizPuzzleData() {}
	virtual ~QuizPuzzleData() {}

	static constexpr uint32 getTag() { return MKTAG('Q', 'U', 'I', 'Z'); }
	virtual void synchronize(Common::Serializer &ser);

	// Keyed by solve-scene ID so that multiple QuizPuzzle instances
	// (e.g. a two-page Nancy 9 puzzle) each maintain their own state.
	Common::HashMap<uint16, Common::Array<bool>> boxCorrect;
	Common::HashMap<uint16, Common::Array<Common::String>> typedText;
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

// Nancy 10+ cellphone state mutated by the ChangeCellPhoneInfo,
// SetCellPhoneBatteryAndSignal and AddSearchLink action records,
// persisted between saves.
struct CellPhoneData : public PuzzleData {
	struct LinkEntry {
		Common::String key;       // CVTX key whose looked-up text is shown in the list
		Common::String value;     // CVTX key for the body (email only); unused for search
		int16 extra = 0;          // search mode: page index (mode-1 only); unused for email
		int16 flag = -1;          // stored by the AR but unused by the original; reserved
		int16 eventFlag = -1;     // event-flag index set when the entry is opened
		bool read = false;        // email only: set once the message is opened
	};

	CellPhoneData() {}
	virtual ~CellPhoneData() {}

	static constexpr uint32 getTag() { return MKTAG('C', 'E', 'L', 'L'); }
	virtual void synchronize(Common::Serializer &ser);

	bool noSignal = false;
	bool batteryLow = false;
	// Loaded set to true once the popup has seeded the contact list from
	// the UICL chunk; we then own it as runtime data.
	bool seeded = false;
	Common::Array<UICL::Contact> contacts;

	// Populated by AR 131 (AddSearchLink). Mode 0 → emailMessages (each
	// with a body-text CVTX key + read flag); any non-zero mode →
	// searchLinks (web search topics).
	Common::Array<LinkEntry> emailMessages;
	Common::Array<LinkEntry> searchLinks;

private:
	void syncLinkArray(Common::Serializer &ser, Common::Array<LinkEntry> &arr);
};

// Nancy 11+ AR 69 (TimerControl). 10 software timers, each counting up from
// zero. A "configured" timer (state 5/6) fires a set of event flags, plays an
// optional sound and shows an optional caption once its target duration
// elapses. Started/stopped via ResetAndStartTimer (104) and StopTimer (105),
// which in Nancy 11 carry a timer-slot index.
struct TimerData : public PuzzleData {
	struct Timer {
		enum State { kIdle = 0, kRunning = 1, kPaused = 2, kOneShot = 5, kRepeating = 6 };

		int32 state = kIdle;
		uint32 currentTimeMs = 0;
		uint32 durationMs = 0;
		bool hasFired = false;
		SoundDescription sound;
		Common::String autotextKey;
		Common::String caption;
		FlagDescription flags[10];

		void reset() { *this = Timer(); }
	};

	static const uint kNumTimers = 10;

	TimerData() {}
	virtual ~TimerData() {}

	static constexpr uint32 getTag() { return MKTAG('T', 'M', 'R', 'S'); }
	virtual void synchronize(Common::Serializer &ser);

	Timer timers[kNumTimers];
};

// Nancy 12+ UI resource values (from the UIRC boot chunk), e.g. resource 0 is
// the coin purse amount in cents. Seeded from UIRC on first use, mutated by AR
// 132 (ResourceUse), and persisted between saves.
struct UIResourceData : public PuzzleData {
	UIResourceData() {}
	virtual ~UIResourceData() {}

	static constexpr uint32 getTag() { return MKTAG('U', 'R', 'E', 'S'); }
	virtual void synchronize(Common::Serializer &ser);

	// Set true once seeded from UIRC, so a loaded save isn't re-seeded.
	bool seeded = false;
	Common::Array<int32> values;
};

PuzzleData *makePuzzleData(const uint32 tag);

} // End of namespace Nancy

#endif // NANCY_PUZZLEDATA_H
