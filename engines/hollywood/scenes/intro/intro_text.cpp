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
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 *
 */

#include "hollywood/scenes/intro/intro_text.h"

#include "common/debug.h"
#include "common/file.h"
#include "common/path.h"

#include "hollywood/graphics.h"
#include "hollywood/hollywood.h"

namespace Hollywood {

const uint kIntroTextDecodeKeySize = 0x141;
const uint kIntroTextStageOffsetTableSize = 0xff4;
const uint kIntroTextDescriptorTableSize = 0x186a0;
const uint kIntroTextSpeechCueDescriptorTableSize = 20000;
const uint kIntroTextSmallRowSize = 0x29;
const uint kIntroTextLargeRowSize = 0x141;
const uint kIntroTextLargeRowBaseIndex = 500;

IntroTextStore::IntroTextStore() {
	_decodeKey.resize(kIntroTextDecodeKeySize);
	_stageBlock.resize(kIntroTextDescriptorTableSize);
	_staticSpeechCueDescriptors.resize(kIntroTextSpeechCueDescriptorTableSize);
}

bool IntroTextStore::loadStage(const char *archiveName, const char *sceneDebugName, uint stageIndex) {
	Common::File file;
	if (!file.open(Common::Path(archiveName))) {
		warning("Failed to open %s for %s text", archiveName, sceneDebugName);
		return false;
	}

	if (file.read(_decodeKey.data(), _decodeKey.size()) != _decodeKey.size()) {
		warning("Failed to read %s row decode key", archiveName);
		return false;
	}

	const uint32 stageOffsetEntry = kIntroTextDecodeKeySize + (stageIndex * 4);
	if (stageOffsetEntry + 4 > kIntroTextDecodeKeySize + kIntroTextStageOffsetTableSize ||
			stageOffsetEntry + 4 > (uint32)file.size()) {
		warning("%s has no stage %u offset entry", archiveName, stageIndex);
		return false;
	}

	file.seek(stageOffsetEntry);
	const uint32 stageOffset = file.readUint32LE();
	if (stageOffset == 0 || stageOffset + kIntroTextDescriptorTableSize + 3 > (uint32)file.size()) {
		warning("%s stage %u descriptor table is out of range", archiveName, stageIndex);
		return false;
	}

	file.seek(stageOffset);
	if (file.read(_stageBlock.data(), _stageBlock.size()) != _stageBlock.size()) {
		warning("Failed to read %s stage %u descriptor table", archiveName, stageIndex);
		return false;
	}

	const byte smallRowCount = file.readByte();
	const uint16 largeRowCount = file.readUint16LE();
	const uint32 smallRowBytes = (uint32)smallRowCount * kIntroTextSmallRowSize;
	const uint32 largeRowBytes = (uint32)largeRowCount * kIntroTextLargeRowSize;
	if (file.pos() + smallRowBytes + largeRowBytes > file.size()) {
		warning("%s stage %u text rows are out of range", archiveName, stageIndex);
		return false;
	}

	file.seek(file.pos() + smallRowBytes);
	_stageLargeRows.resize(largeRowBytes);
	if (file.read(_stageLargeRows.data(), _stageLargeRows.size()) != _stageLargeRows.size()) {
		warning("Failed to read %s stage %u large text rows", archiveName, stageIndex);
		return false;
	}

	for (uint row = 0; row < largeRowCount; ++row) {
		for (uint column = 0; column < kIntroTextLargeRowSize; ++column)
			_stageLargeRows[row * kIntroTextLargeRowSize + column] -= _decodeKey[column];
	}

	debugC(1, kDebugResources, "Loaded %s stage %u intro text rows: largeRows=%u",
		archiveName, stageIndex, largeRowCount);
	return true;
}

bool IntroTextStore::loadStaticSpeechCues(const char *archiveName, const char *sceneDebugName,
		uint32 speechCueDescriptorOffset) {
	Common::File file;
	if (!file.open(Common::Path(archiveName))) {
		warning("Failed to open %s for %s static speech cues", archiveName, sceneDebugName);
		return false;
	}

	if (speechCueDescriptorOffset + kIntroTextSpeechCueDescriptorTableSize > (uint32)file.size()) {
		warning("%s static speech cue table is out of range", archiveName);
		return false;
	}

	file.seek(speechCueDescriptorOffset);
	if (file.read(_staticSpeechCueDescriptors.data(), _staticSpeechCueDescriptors.size()) !=
			_staticSpeechCueDescriptors.size()) {
		warning("Failed to read %s static speech cue table", archiveName);
		return false;
	}

	return true;
}

bool IntroTextStore::getStageCue(uint16 rowIndex, byte frameIndex, uint16 &textRecordId,
		byte &continuationCount, uint16 &voiceSampleId) const {
	const uint offset = ((uint)frameIndex + (uint)rowIndex * 100) * 5;
	if (offset + 5 > _stageBlock.size())
		return false;

	textRecordId = readUint16LE(_stageBlock, offset);
	continuationCount = _stageBlock[offset + 2];
	voiceSampleId = readUint16LE(_stageBlock, offset + 3);
	return textRecordId != 0;
}

bool IntroTextStore::getStaticSpeechCue(uint16 rowIndex, byte frameIndex, uint16 &textRecordId,
		byte &continuationCount, uint16 &voiceSampleId) const {
	const uint offset = ((uint)frameIndex + (uint)rowIndex * 10) * 5;
	if (offset + 5 > _staticSpeechCueDescriptors.size())
		return false;

	textRecordId = readUint16LE(_staticSpeechCueDescriptors, offset);
	continuationCount = _staticSpeechCueDescriptors[offset + 2];
	voiceSampleId = readUint16LE(_staticSpeechCueDescriptors, offset + 3);
	return textRecordId != 0;
}

Common::String IntroTextStore::largeTextRecord(uint16 recordId) const {
	if (recordId < kIntroTextLargeRowBaseIndex)
		return Common::String();

	const uint localRecordId = recordId - kIntroTextLargeRowBaseIndex;
	const uint offset = localRecordId * kIntroTextLargeRowSize;
	if (offset >= _stageLargeRows.size())
		return Common::String();

	const byte *row = _stageLargeRows.data() + offset;
	uint length = 0;
	while (length < kIntroTextLargeRowSize && row[length] != 0)
		++length;

	return Common::String((const char *)row, length);
}

} // End of namespace Hollywood
