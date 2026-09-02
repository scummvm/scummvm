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

#ifndef HOLLYWOOD_SCENES_SCENE_TEXT_STORE_H
#define HOLLYWOOD_SCENES_SCENE_TEXT_STORE_H

#include "common/array.h"
#include "common/str.h"
#include "common/types.h"

namespace Common {
class SeekableReadStream;
}

namespace Hollywood {

struct SceneSpeechCue {
	SceneSpeechCue() :
			textRecordId(0),
			continuationCount(0),
			voiceSampleId(0) {
	}

	SceneSpeechCue(uint16 newTextRecordId, byte newContinuationCount,
			uint16 newVoiceSampleId) :
			textRecordId(newTextRecordId),
			continuationCount(newContinuationCount),
			voiceSampleId(newVoiceSampleId) {
	}

	bool valid() const { return textRecordId != 0; }

	uint16 textRecordId;
	byte continuationCount;
	uint16 voiceSampleId;
};

// Owns the RESOURCE.003 rows and cue tables required by a scene.
class SceneTextStore {
public:
	SceneTextStore();

	bool load(const char *archiveName, const char *sceneDebugName, uint stageIndex,
		uint inventoryRowsOffsetIndex, uint32 speechCueDescriptorOffset,
		bool validateSequentialVoiceMap = false);
	bool loadStage(const char *archiveName, const char *sceneDebugName, uint stageIndex);
	bool loadStaticSpeechCues(const char *archiveName, const char *sceneDebugName,
		uint32 speechCueDescriptorOffset);

	Common::String inventoryItemName(byte itemId) const;
	Common::String dialogueMenuText(byte stageId, byte textRowId) const;
	SceneSpeechCue stageCue(uint16 rowIndex, byte frameIndex) const;
	bool getStageCue(uint16 rowIndex, byte frameIndex, uint16 &textRecordId,
		byte &continuationCount, uint16 &voiceSampleId) const;
	bool getStaticSpeechCue(uint16 rowIndex, byte frameIndex, uint16 &textRecordId,
		byte &continuationCount, uint16 &voiceSampleId) const;
	Common::String largeTextRecord(uint16 recordId) const;

	Common::Array<byte> _decodeKey;
	Common::Array<byte> _stageBlock;
	Common::Array<byte> _stageSmallRows;
	Common::Array<byte> _stageLargeRows;
	Common::Array<byte> _staticSpeechCueDescriptors;
	Common::Array<byte> _inventoryOwnerSmallRows;
	Common::Array<byte> _inventoryOwnerLargeRows;

private:
	SceneSpeechCue staticSpeechCue(uint16 rowIndex, byte frameIndex) const;

	bool readDecodeKey(Common::SeekableReadStream &stream, const char *archiveName);
	bool readStage(Common::SeekableReadStream &stream, const char *archiveName,
		uint stageIndex);
	bool readStaticSpeechCues(Common::SeekableReadStream &stream,
		const char *archiveName, uint32 speechCueDescriptorOffset,
		uint trailingByteCount = 0);
	bool readInventoryRows(Common::SeekableReadStream &stream, const char *archiveName,
		uint inventoryRowsOffsetIndex, byte smallRowCount, uint16 largeRowCount);
	uint16 findStageVoiceSampleBase(uint largeRowCount) const;

	uint16 _stageVoiceSampleBase;
	bool _validateSequentialVoiceMap;
};

} // End of namespace Hollywood

#endif // HOLLYWOOD_SCENES_SCENE_TEXT_STORE_H
