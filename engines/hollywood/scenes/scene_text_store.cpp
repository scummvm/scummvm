/* ScummVM - Graphic Adventure Engine
 *
 * ScummVM is the legal property of its developers, whose names
 * are too numerous to list here. Please refer to the COPYRIGHT
 * file distributed with this program.
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

#include "hollywood/scenes/scene_text_store.h"

#include "common/debug.h"
#include "common/file.h"
#include "common/path.h"

#include "hollywood/graphics.h"
#include "hollywood/hollywood.h"

namespace Hollywood {

const uint kSceneTextDecodeKeySize = 0x141;
const uint kSceneTextStageOffsetTableSize = 0xff4;
const uint kSceneTextDescriptorTableSize = 0x186a0;
const uint kSceneTextSpeechCueDescriptorTableSize = 20000;
const uint kSceneTextSmallRowSize = 0x29;
const uint kSceneTextLargeRowSize = 0x141;
const uint kSceneTextLargeRowBaseIndex = 500;

SceneTextStore::SceneTextStore() :
		_stageVoiceSampleBase(0),
		_validateSequentialVoiceMap(false) {
	decodeKey.resize(kSceneTextDecodeKeySize);
	stageBlock.resize(kSceneTextDescriptorTableSize);
	staticSpeechCueDescriptors.resize(kSceneTextSpeechCueDescriptorTableSize);
}

bool SceneTextStore::load(const char *archiveName, const char *sceneDebugName,
		uint stageIndex, uint inventoryRowsOffsetIndex,
		uint32 speechCueDescriptorOffset, bool validateSequentialVoiceMap) {
	_validateSequentialVoiceMap = validateSequentialVoiceMap;
	_stageVoiceSampleBase = 0;

	Common::File file;
	if (!file.open(Common::Path(archiveName))) {
		warning("Failed to open %s for %s text", archiveName, sceneDebugName);
		return false;
	}

	if (!readDecodeKey(file, archiveName) ||
			!readStaticSpeechCues(file, archiveName, speechCueDescriptorOffset, 3))
		return false;

	const byte ownerSmallRowCount = file.readByte();
	const uint16 ownerLargeRowCount = file.readUint16LE();
	if (file.err()) {
		warning("Failed to read %s static text row counts", archiveName);
		return false;
	}

	if (!readInventoryRows(file, archiveName, inventoryRowsOffsetIndex,
			ownerSmallRowCount, ownerLargeRowCount) ||
			!readStage(file, archiveName, stageIndex))
		return false;

	const uint largeRowCount = stageLargeRows.size() / kSceneTextLargeRowSize;
	_stageVoiceSampleBase = _validateSequentialVoiceMap ?
		findStageVoiceSampleBase(largeRowCount) : 0;
	if (_validateSequentialVoiceMap && _stageVoiceSampleBase == 0) {
		warning("%s stage %u has no valid speech cue mapping", archiveName, stageIndex);
		return false;
	}

	return true;
}

bool SceneTextStore::loadStage(const char *archiveName, const char *sceneDebugName,
		uint stageIndex) {
	_validateSequentialVoiceMap = false;
	_stageVoiceSampleBase = 0;
	inventoryOwnerSmallRows.clear();
	inventoryOwnerLargeRows.clear();

	Common::File file;
	if (!file.open(Common::Path(archiveName))) {
		warning("Failed to open %s for %s text", archiveName, sceneDebugName);
		return false;
	}

	return readDecodeKey(file, archiveName) && readStage(file, archiveName, stageIndex);
}

bool SceneTextStore::loadStaticSpeechCues(const char *archiveName,
		const char *sceneDebugName, uint32 speechCueDescriptorOffset) {
	Common::File file;
	if (!file.open(Common::Path(archiveName))) {
		warning("Failed to open %s for %s static speech cues", archiveName, sceneDebugName);
		return false;
	}

	return readStaticSpeechCues(file, archiveName, speechCueDescriptorOffset);
}

bool SceneTextStore::readDecodeKey(Common::SeekableReadStream &stream,
		const char *archiveName) {
	decodeKey.resize(kSceneTextDecodeKeySize);
	stream.seek(0);
	if (stream.read(decodeKey.data(), decodeKey.size()) != decodeKey.size()) {
		warning("Failed to read %s row decode key", archiveName);
		return false;
	}

	return true;
}

bool SceneTextStore::readStage(Common::SeekableReadStream &stream,
		const char *archiveName, uint stageIndex) {
	const uint32 stageOffsetEntry = kSceneTextDecodeKeySize + stageIndex * 4;
	if (stageOffsetEntry + 4 > kSceneTextDecodeKeySize + kSceneTextStageOffsetTableSize ||
			stageOffsetEntry + 4 > (uint32)stream.size()) {
		warning("%s has no stage %u offset entry", archiveName, stageIndex);
		return false;
	}

	stream.seek(stageOffsetEntry);
	const uint32 stageOffset = stream.readUint32LE();
	if (stageOffset == 0 ||
			stageOffset + kSceneTextDescriptorTableSize + 3 > (uint32)stream.size()) {
		warning("%s stage %u descriptor table is out of range", archiveName, stageIndex);
		return false;
	}

	stageBlock.resize(kSceneTextDescriptorTableSize);
	stream.seek(stageOffset);
	if (stream.read(stageBlock.data(), stageBlock.size()) != stageBlock.size()) {
		warning("Failed to read %s stage %u descriptor table", archiveName, stageIndex);
		return false;
	}

	const byte smallRowCount = stream.readByte();
	const uint16 largeRowCount = stream.readUint16LE();
	const uint32 smallRowBytes = (uint32)smallRowCount * kSceneTextSmallRowSize;
	const uint32 largeRowBytes = (uint32)largeRowCount * kSceneTextLargeRowSize;
	if (stream.pos() + smallRowBytes + largeRowBytes > stream.size()) {
		warning("%s stage %u text rows are out of range", archiveName, stageIndex);
		return false;
	}

	stageSmallRows.resize((uint32)(smallRowCount + 1) * kSceneTextSmallRowSize);
	memset(stageSmallRows.data(), 0, stageSmallRows.size());
	if (stream.read(stageSmallRows.data() + kSceneTextSmallRowSize, smallRowBytes) !=
			smallRowBytes) {
		warning("Failed to read %s stage %u small text rows", archiveName, stageIndex);
		return false;
	}

	stageLargeRows.resize(largeRowBytes);
	if (stream.read(stageLargeRows.data(), stageLargeRows.size()) !=
			stageLargeRows.size()) {
		warning("Failed to read %s stage %u large text rows", archiveName, stageIndex);
		return false;
	}

	for (uint row = 1; row <= smallRowCount; ++row) {
		for (uint column = 0; column < kSceneTextSmallRowSize; ++column)
			stageSmallRows[row * kSceneTextSmallRowSize + column] -= decodeKey[column];
	}
	for (uint row = 0; row < largeRowCount; ++row) {
		for (uint column = 0; column < kSceneTextLargeRowSize; ++column)
			stageLargeRows[row * kSceneTextLargeRowSize + column] -= decodeKey[column];
	}

	debugC(1, kDebugResources, "Loaded %s stage %u text rows: smallRows=%u largeRows=%u",
		archiveName, stageIndex, smallRowCount, largeRowCount);
	return true;
}

bool SceneTextStore::readStaticSpeechCues(Common::SeekableReadStream &stream,
		const char *archiveName, uint32 speechCueDescriptorOffset,
		uint trailingByteCount) {
	if (speechCueDescriptorOffset + kSceneTextSpeechCueDescriptorTableSize +
			trailingByteCount > (uint32)stream.size()) {
		warning("%s static speech cue table is out of range", archiveName);
		return false;
	}

	staticSpeechCueDescriptors.resize(kSceneTextSpeechCueDescriptorTableSize);
	stream.seek(speechCueDescriptorOffset);
	if (stream.read(staticSpeechCueDescriptors.data(),
			staticSpeechCueDescriptors.size()) != staticSpeechCueDescriptors.size()) {
		warning("Failed to read %s static speech cue table", archiveName);
		return false;
	}

	return true;
}

bool SceneTextStore::readInventoryRows(Common::SeekableReadStream &stream,
		const char *archiveName, uint inventoryRowsOffsetIndex,
		byte smallRowCount, uint16 largeRowCount) {
	const uint32 rowsOffsetEntry = kSceneTextDecodeKeySize + inventoryRowsOffsetIndex * 4;
	if (rowsOffsetEntry + 4 > kSceneTextDecodeKeySize + kSceneTextStageOffsetTableSize ||
			rowsOffsetEntry + 4 > (uint32)stream.size()) {
		warning("%s static text row offset entry is out of range", archiveName);
		return false;
	}

	stream.seek(rowsOffsetEntry);
	const uint32 rowsOffset = stream.readUint32LE();
	const uint32 smallRowBytes = (uint32)smallRowCount * kSceneTextSmallRowSize;
	const uint32 largeRowBytes = (uint32)largeRowCount * kSceneTextLargeRowSize;
	if (rowsOffset == 0 ||
			rowsOffset + smallRowBytes + largeRowBytes > (uint32)stream.size()) {
		warning("%s static text rows are out of range", archiveName);
		return false;
	}

	inventoryOwnerSmallRows.resize((uint32)(smallRowCount + 1) * kSceneTextSmallRowSize);
	memset(inventoryOwnerSmallRows.data(), 0, inventoryOwnerSmallRows.size());
	inventoryOwnerLargeRows.resize((uint32)(largeRowCount + 1) * kSceneTextLargeRowSize);
	memset(inventoryOwnerLargeRows.data(), 0, inventoryOwnerLargeRows.size());
	stream.seek(rowsOffset);
	if (stream.read(inventoryOwnerSmallRows.data() + kSceneTextSmallRowSize,
			smallRowBytes) != smallRowBytes) {
		warning("Failed to read %s static small text rows", archiveName);
		return false;
	}
	if (stream.read(inventoryOwnerLargeRows.data() + kSceneTextLargeRowSize,
			largeRowBytes) != largeRowBytes) {
		warning("Failed to read %s static large text rows", archiveName);
		return false;
	}

	for (uint row = 1; row <= smallRowCount; ++row) {
		for (uint column = 0; column < kSceneTextSmallRowSize; ++column)
			inventoryOwnerSmallRows[row * kSceneTextSmallRowSize + column] -= decodeKey[column];
	}
	for (uint row = 1; row <= largeRowCount; ++row) {
		for (uint column = 0; column < kSceneTextLargeRowSize; ++column)
			inventoryOwnerLargeRows[row * kSceneTextLargeRowSize + column] -= decodeKey[column];
	}

	return true;
}

Common::String SceneTextStore::inventoryItemName(byte itemId) const {
	const uint offset = (uint)itemId * kSceneTextSmallRowSize;
	if (offset >= inventoryOwnerSmallRows.size())
		return Common::String();

	const byte *row = inventoryOwnerSmallRows.data() + offset;
	uint length = 0;
	while (offset + length < inventoryOwnerSmallRows.size() &&
			length < kSceneTextSmallRowSize && row[length] != 0)
		++length;

	return Common::String((const char *)row, length);
}

Common::String SceneTextStore::dialogueMenuText(byte stageId, byte textRowId) const {
	const uint offset = ((uint)stageId * 100 + textRowId) * 5;
	if (offset + 5 > stageBlock.size())
		return Common::String();

	return largeTextRecord(readUint16LE(stageBlock, offset));
}

SceneSpeechCue SceneTextStore::stageCue(uint16 rowIndex, byte frameIndex) const {
	const uint offset = ((uint)frameIndex + (uint)rowIndex * 100) * 5;
	if (offset + 5 > stageBlock.size())
		return SceneSpeechCue();

	return SceneSpeechCue(readUint16LE(stageBlock, offset), stageBlock[offset + 2],
		readUint16LE(stageBlock, offset + 3));
}

SceneSpeechCue SceneTextStore::staticSpeechCue(uint16 rowIndex, byte frameIndex) const {
	const uint offset = ((uint)frameIndex + (uint)rowIndex * 10) * 5;
	if (offset + 5 > staticSpeechCueDescriptors.size())
		return SceneSpeechCue();

	return SceneSpeechCue(readUint16LE(staticSpeechCueDescriptors, offset),
		staticSpeechCueDescriptors[offset + 2],
		readUint16LE(staticSpeechCueDescriptors, offset + 3));
}

bool SceneTextStore::getStageCue(uint16 rowIndex, byte frameIndex,
		uint16 &textRecordId, byte &continuationCount, uint16 &voiceSampleId) const {
	const SceneSpeechCue cue = stageCue(rowIndex, frameIndex);
	textRecordId = cue.textRecordId;
	continuationCount = cue.continuationCount;
	voiceSampleId = cue.voiceSampleId;

	if (!cue.valid())
		return false;
	if (!_validateSequentialVoiceMap)
		return true;
	if (cue.textRecordId < kSceneTextLargeRowBaseIndex || _stageVoiceSampleBase == 0)
		return false;

	const uint localRecordId = cue.textRecordId - kSceneTextLargeRowBaseIndex;
	const uint largeRowCount = stageLargeRows.size() / kSceneTextLargeRowSize;
	const uint lineCount = MAX<uint>(1, cue.continuationCount);
	if (localRecordId + lineCount > largeRowCount)
		return false;

	return cue.voiceSampleId == _stageVoiceSampleBase + localRecordId;
}

uint16 SceneTextStore::findStageVoiceSampleBase(uint largeRowCount) const {
	// Authored cues share one voice-to-text offset; stale table padding does not.
	Common::Array<uint16> baseCounts;
	baseCounts.resize(0x10000);
	memset(baseCounts.data(), 0, baseCounts.size() * sizeof(baseCounts[0]));

	uint16 bestBase = 0;
	uint bestCount = 0;
	for (uint offset = 0; offset + 5 <= stageBlock.size(); offset += 5) {
		const uint16 textRecordId = readUint16LE(stageBlock, offset);
		if (textRecordId < kSceneTextLargeRowBaseIndex)
			continue;

		const uint localRecordId = textRecordId - kSceneTextLargeRowBaseIndex;
		const uint lineCount = MAX<uint>(1, stageBlock[offset + 2]);
		const uint16 voiceSampleId = readUint16LE(stageBlock, offset + 3);
		if (localRecordId + lineCount > largeRowCount || voiceSampleId < localRecordId)
			continue;

		const uint16 base = voiceSampleId - localRecordId;
		const uint count = ++baseCounts[base];
		if (count > bestCount) {
			bestCount = count;
			bestBase = base;
		}
	}

	return bestBase;
}

bool SceneTextStore::getStaticSpeechCue(uint16 rowIndex, byte frameIndex,
		uint16 &textRecordId, byte &continuationCount, uint16 &voiceSampleId) const {
	const SceneSpeechCue cue = staticSpeechCue(rowIndex, frameIndex);
	textRecordId = cue.textRecordId;
	continuationCount = cue.continuationCount;
	voiceSampleId = cue.voiceSampleId;
	return cue.valid();
}

Common::String SceneTextStore::largeTextRecord(uint16 recordId) const {
	if (recordId < kSceneTextLargeRowBaseIndex) {
		const uint offset = (uint)recordId * kSceneTextLargeRowSize;
		if (recordId == 0 || offset >= inventoryOwnerLargeRows.size())
			return Common::String();

		const byte *row = inventoryOwnerLargeRows.data() + offset;
		uint length = 0;
		while (length < kSceneTextLargeRowSize && row[length] != 0)
			++length;

		return Common::String((const char *)row, length);
	}

	const uint localRecordId = recordId - kSceneTextLargeRowBaseIndex;
	const uint offset = localRecordId * kSceneTextLargeRowSize;
	if (offset >= stageLargeRows.size())
		return Common::String();

	const byte *row = stageLargeRows.data() + offset;
	uint length = 0;
	while (length < kSceneTextLargeRowSize && row[length] != 0)
		++length;

	return Common::String((const char *)row, length);
}

} // End of namespace Hollywood
