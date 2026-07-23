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

#include "engines/nancy/puzzledata.h"
#include "engines/nancy/enginedata.h"
#include "engines/nancy/nancy.h"

namespace Nancy {

SliderPuzzleData::SliderPuzzleData() : playerHasTriedPuzzle(false) {}

void SliderPuzzleData::synchronize(Common::Serializer &ser) {
	ser.syncAsByte(playerHasTriedPuzzle);

	byte x = 0, y = 0;

	if (ser.isSaving()) {
		y = playerTileOrder.size();
		if (y) {
			x = playerTileOrder.back().size();
		} else {
			x = 0;
		}
	}

	ser.syncAsByte(x);
	ser.syncAsByte(y);

	playerTileOrder.resize(y);

	for (int i = 0; i < y; ++i) {
		playerTileOrder[i].resize(x);
		ser.syncArray(playerTileOrder[i].data(), x, Common::Serializer::Sint16LE);
	}
}

RippedLetterPuzzleData::RippedLetterPuzzleData() :
	order(24, 0),
	rotations(24, 0),
	playerHasTriedPuzzle(false) {}

void RippedLetterPuzzleData::synchronize(Common::Serializer &ser) {
	// Serialize through fixed size buffers so save or load never
	// mutates the live puzzle state
	Common::Array<int8> serializedOrder(24, -1);
	Common::Array<byte> serializedRotations(24, 0);

	for (uint i = 0; i < order.size() && i < serializedOrder.size(); ++i) {
		serializedOrder[i] = order[i];
	}

	for (uint i = 0; i < rotations.size() && i < serializedRotations.size(); ++i) {
		serializedRotations[i] = rotations[i];
	}

	// A piece may still be held while saving; make sure the saved data
	// has it back in the last place it was picked up from
	if (ser.isSaving() && _pickedUpPieceID != -1) {
		serializedOrder[_pickedUpPieceLastPos] = _pickedUpPieceID;
		serializedRotations[_pickedUpPieceLastPos] = _pickedUpPieceRot;
	}

	ser.syncArray(serializedOrder.data(), serializedOrder.size(), Common::Serializer::Byte);
	ser.syncArray(serializedRotations.data(), serializedRotations.size(), Common::Serializer::Byte);

	if (ser.isLoading()) {
		Common::move(serializedOrder.begin(), serializedOrder.end(), order.begin());
		Common::move(serializedRotations.begin(), serializedRotations.end(), rotations.begin());
	}
}

TowerPuzzleData::TowerPuzzleData() {
	order.resize(3, Common::Array<int8>(6, -1));
	playerHasTriedPuzzle = false;
}

void TowerPuzzleData::synchronize(Common::Serializer &ser) {
	ser.syncAsByte(playerHasTriedPuzzle);

	if (ser.isLoading()) {
		order.resize(3, Common::Array<int8>(6, -1));
	}

	for (uint i = 0; i < 3; ++i) {
		ser.syncArray(order[i].data(), 6, Common::Serializer::Byte);
	}
}

RiddlePuzzleData::RiddlePuzzleData() :
	incorrectRiddleID(-1) {}

void RiddlePuzzleData::synchronize(Common::Serializer &ser) {
	byte numRiddles = solvedRiddleIDs.size();
	ser.syncAsByte(numRiddles);

	if (ser.isLoading()) {
		solvedRiddleIDs.resize(numRiddles);
	}

	ser.syncArray(solvedRiddleIDs.data(), numRiddles, Common::Serializer::Byte);
}

SoundEqualizerPuzzleData::SoundEqualizerPuzzleData() {
	sliderValues.resize(6, 255);
}

void SoundEqualizerPuzzleData::synchronize(Common::Serializer &ser) {
	ser.syncArray(sliderValues.data(), 6, Common::Serializer::Byte);
}

SimplePuzzleData::SimplePuzzleData() {
	solvedPuzzle = false;
}

void SimplePuzzleData::synchronize(Common::Serializer &ser) {
	ser.syncAsByte(solvedPuzzle);
}

void JournalData::synchronize(Common::Serializer &ser) {
	uint16 numEntries = journalEntries.size();
	ser.syncAsUint16LE(numEntries);

	if (ser.isLoading()) {
		for (uint i = 0; i < numEntries; ++i) {
			uint16 id = 0;
			ser.syncAsUint16LE(id);
			uint16 numStrings = 0;
			ser.syncAsUint16LE(numStrings);
			auto &entry = journalEntries[id];
			for (uint j = 0; j < numStrings; ++j) {
				entry.push_back(Entry());
				ser.syncString(entry.back().stringID);
				ser.syncAsUint16LE(entry.back().mark);
				if (g_nancy->getGameType() >= kGameTypeNancy9) {
					// NOTE: We did not persist sceneID for journal entries in nancy9
					// in save versions before 4. Fortunately for us, this field is
					// only used in scene 2491, when using the search functionality
					// in the laptop, and it's always initialized by the game scripts,
					// so we can use the script values instead in that scene. Refer to
					// the workaround in ModifyListEntry::execute(), which fixes these
					// values for that scene, in older saved games.
					ser.syncAsUint16LE(entry.back().sceneID, 4);
				}
			}
		}
	} else {
		for (auto &a : journalEntries) {
			uint16 id = a._key;
			ser.syncAsUint16LE(id);
			uint16 numStrings = a._value.size();
			ser.syncAsUint16LE(numStrings);
			for (uint i = 0; i < numStrings; ++i) {
				ser.syncString(a._value[i].stringID);
				ser.syncAsUint16LE(a._value[i].mark);
				if (g_nancy->getGameType() >= kGameTypeNancy9)
					ser.syncAsUint16LE(a._value[i].sceneID);	// added in save version 4
			}
		}
	}
}

TableData::TableData() {
	if (g_nancy->getGameType() == kGameTypeNancy6) {
		auto *tabl = GetEngineData(TABL);
		assert(tabl);

		singleValues.resize(tabl->startIDs.size());
		for (uint i = 0; i < tabl->startIDs.size(); ++i) {
			singleValues[i] = tabl->startIDs[i];
		}
	}
}

void TableData::synchronize(Common::Serializer &ser) {
	byte num = singleValues.size();
	ser.syncAsByte(num);

	if (ser.isLoading()) {
		singleValues.resize(num);
	}

	ser.syncArray(singleValues.data(), num, Common::Serializer::Sint16LE);

	if (g_nancy->getGameType() < kGameTypeNancy8) {
		return;
	}

	num = comboValues.size();
	ser.syncAsByte(num);

	if (ser.isLoading()) {
		comboValues.resize(num);
	}

	ser.syncArray(comboValues.data(), num, Common::Serializer::FloatLE);
}

static void syncInt16Array(Common::Serializer &ser, Common::Array<int16> &arr) {
	uint16 num = (uint16)arr.size();
	ser.syncAsUint16LE(num);
	if (ser.isLoading())
		arr.resize(num);
	ser.syncArray(arr.data(), num, Common::Serializer::Sint16LE);
}

void BeadPuzzleData::synchronize(Common::Serializer &ser) {
	syncInt16Array(ser, placedBeads);
}

void SortPuzzleData::synchronize(Common::Serializer &ser) {
	syncInt16Array(ser, currentState);
	syncInt16Array(ser, solvedState);
}

void MagnetMazePuzzleData::synchronize(Common::Serializer &ser) {
	syncInt16Array(ser, magnetState);
}

void GridMapPuzzleData::synchronize(Common::Serializer &ser) {
	syncInt16Array(ser, itemState);
}

void QuizPuzzleData::synchronize(Common::Serializer &ser) {
	// Serialize as: numScenes, then for each scene: sceneID, numBoxes, box data
	uint16 numScenes = (uint16)boxCorrect.size();
	ser.syncAsUint16LE(numScenes);

	if (ser.isLoading()) {
		boxCorrect.clear();
		typedText.clear();
		for (uint16 s = 0; s < numScenes; ++s) {
			uint16 sceneID = 0;
			ser.syncAsUint16LE(sceneID);
			byte num = 0;
			ser.syncAsByte(num);
			auto &bc = boxCorrect[sceneID];
			auto &tt = typedText[sceneID];
			bc.resize(num, false);
			tt.resize(num);
			for (uint i = 0; i < num; ++i) {
				byte b = 0;
				ser.syncAsByte(b);
				bc[i] = (b != 0);
				ser.syncString(tt[i]);
			}
		}
	} else {
		for (auto &entry : boxCorrect) {
			uint16 sceneID = entry._key;
			ser.syncAsUint16LE(sceneID);
			byte num = (byte)entry._value.size();
			ser.syncAsByte(num);
			auto &tt = typedText[sceneID];
			for (uint i = 0; i < num; ++i) {
				byte b = entry._value[i] ? 1 : 0;
				ser.syncAsByte(b);
				ser.syncString(tt[i]);
			}
		}
	}
}

void TableData::setSingleValue(uint16 index, int16 value) {
	if (singleValues.size() <= index) {
		singleValues.resize(index + 1, kNoTableValue);
	}

	singleValues[index] = value;
}

int16 TableData::getSingleValue(uint16 index) const {
	return index < singleValues.size() ? singleValues[index] : kNoTableValue;
}

void TableData::setComboValue(uint16 index, float value) {
	if (comboValues.size() <= index) {
		comboValues.resize(index + 1, kNoTableValue);
	}

	comboValues[index] = value;
}

float TableData::getComboValue(uint16 index) const {
	return index < comboValues.size() ? comboValues[index] : kNoTableValue;
}

void CellPhoneData::synchronize(Common::Serializer &ser) {
	ser.syncAsByte(noSignal);
	ser.syncAsByte(batteryLow);
	ser.syncAsByte(seeded);

	uint16 numContacts = (uint16)contacts.size();
	ser.syncAsUint16LE(numContacts);

	if (ser.isLoading()) {
		contacts.resize(numContacts);
	}

	char nameBuf[21];
	for (uint16 i = 0; i < numContacts; ++i) {
		UICL::Contact &c = contacts[i];
		ser.syncBytes(c.unknownPrefix, sizeof(c.unknownPrefix));

		if (ser.isSaving()) {
			memset(nameBuf, 0, sizeof(nameBuf));
			Common::strlcpy(nameBuf, c.name.c_str(), sizeof(nameBuf));
		}
		ser.syncBytes((byte *)nameBuf, 20);
		if (ser.isLoading()) {
			nameBuf[20] = '\0';
			c.name = nameBuf;
		}

		ser.syncBytes(c.unknownSuffix, sizeof(c.unknownSuffix));
	}

	syncLinkArray(ser, emailMessages);
	syncLinkArray(ser, searchLinks);
}

void CellPhoneData::syncLinkArray(Common::Serializer &ser, Common::Array<SearchLink> &arr) {
	uint16 n = (uint16)arr.size();
	ser.syncAsUint16LE(n);
	if (ser.isLoading()) {
		arr.resize(n);
	}
	for (uint16 i = 0; i < n; ++i) {
		ser.syncString(arr[i].key);
		ser.syncString(arr[i].value);
		ser.syncAsSint16LE(arr[i].extra);
		ser.syncAsSint16LE(arr[i].flag);
		ser.syncAsSint16LE(arr[i].eventFlag);
		ser.syncAsByte(arr[i].read);
	}
}

void CellPhonePictureData::synchronize(Common::Serializer &ser) {
	uint16 numPictures = (uint16)pictures.size();
	ser.syncAsUint16LE(numPictures);
	if (ser.isLoading()) {
		pictures.resize(numPictures);
	}

	for (uint16 i = 0; i < numPictures; ++i) {
		CapturedPicture &p = pictures[i];
		ser.syncAsUint16LE(p.width);
		ser.syncAsUint16LE(p.height);
		ser.syncAsByte(p.sent);

		uint32 numBytes = (uint32)p.width * p.height * 4;
		if (ser.isLoading()) {
			p.pixels.resize(numBytes);
		}
		if (numBytes) {
			ser.syncBytes(p.pixels.data(), numBytes);
		}
	}
}

void TimerData::synchronize(Common::Serializer &ser) {
	for (uint i = 0; i < kNumTimers; ++i) {
		Timer &t = timers[i];
		ser.syncAsSint32LE(t.state);
		ser.syncAsUint32LE(t.currentTimeMs);
		ser.syncAsUint32LE(t.durationMs);
		ser.syncAsByte(t.hasFired);

		ser.syncString(t.sound.name);
		ser.syncAsUint16LE(t.sound.channelID);
		ser.syncAsUint16LE(t.sound.playCommands);
		ser.syncAsUint16LE(t.sound.numLoops);
		ser.syncAsUint16LE(t.sound.volume);

		ser.syncString(t.autotextKey);
		ser.syncString(t.caption);

		for (uint j = 0; j < ARRAYSIZE(t.flags); ++j) {
			ser.syncAsSint16LE(t.flags[j].label);
			ser.syncAsByte(t.flags[j].flag);
		}
	}
}

void UIResourceData::synchronize(Common::Serializer &ser) {
	ser.syncAsByte(seeded);

	uint16 numValues = (uint16)values.size();
	ser.syncAsUint16LE(numValues);
	if (ser.isLoading()) {
		values.resize(numValues);
	}

	for (uint16 i = 0; i < numValues; ++i) {
		ser.syncAsSint32LE(values[i]);
	}
}

void TaskbarData::synchronize(Common::Serializer &ser) {
	for (uint i = 0; i < kNumButtons; ++i) {
		ser.syncAsByte(overrides[i].active);
		ser.syncAsSint16LE(overrides[i].startScene);
		ser.syncAsSint16LE(overrides[i].endScene);
		ser.syncAsUint16LE(overrides[i].clickSoundMode);
	}

	// Notification badges were added in savegame version 5. Older saves don't
	// have these bytes; the flags stay at their default (cleared) state.
	if (ser.getVersion() >= 5) {
		for (uint i = 0; i < kNumButtons; ++i) {
			for (uint s = 0; s < kNumNotificationSubCategories; ++s) {
				ser.syncAsByte(notifications[i][s]);
			}
		}
	}
}

void WordFindPuzzleData::synchronize(Common::Serializer &ser) {
	ser.syncAsSint16LE(currentWord);
}

PuzzleData *makePuzzleData(const uint32 tag) {
	switch(tag) {
	case WordFindPuzzleData::getTag():
		return new WordFindPuzzleData();
	case SliderPuzzleData::getTag():
		return new SliderPuzzleData();
	case RippedLetterPuzzleData::getTag():
		return new RippedLetterPuzzleData();
	case TowerPuzzleData::getTag():
		return new TowerPuzzleData();
	case RiddlePuzzleData::getTag():
		return new RiddlePuzzleData();
	case SoundEqualizerPuzzleData::getTag():
		return new SoundEqualizerPuzzleData();
	case AssemblyPuzzleData::getTag():
		return new AssemblyPuzzleData();
	case QuizPuzzleData::getTag():
		return new QuizPuzzleData();
	case BeadPuzzleData::getTag():
		return new BeadPuzzleData();
	case SortPuzzleData::getTag():
		return new SortPuzzleData();
	case MagnetMazePuzzleData::getTag():
		return new MagnetMazePuzzleData();
	case GridMapPuzzleData::getTag():
		return new GridMapPuzzleData();
	case JournalData::getTag():
		return new JournalData();
	case TableData::getTag():
		return new TableData();
	case CellPhoneData::getTag():
		return new CellPhoneData();
	case CellPhonePictureData::getTag():
		return new CellPhonePictureData();
	case TimerData::getTag():
		return new TimerData();
	case UIResourceData::getTag():
		return new UIResourceData();
	case TaskbarData::getTag():
		return new TaskbarData();
	default:
		return nullptr;
	}
}

} // End of namespace Nancy
