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

#ifndef HOLLYWOOD_SCENES_PLAYABLE_SCENE_TEXT_STORE_H
#define HOLLYWOOD_SCENES_PLAYABLE_SCENE_TEXT_STORE_H

#include "common/array.h"
#include "common/str.h"
#include "common/types.h"

namespace Hollywood {

// Owns RESOURCE.003 text rows and rejects stale entries in its padded cue table.
class SceneTextStore {
public:
	SceneTextStore();

	bool load(const char *archiveName, const char *sceneDebugName, uint stageIndex,
		uint inventoryRowsOffsetIndex, uint32 speechCueDescriptorOffset,
		bool validateSequentialVoiceMap = false);
	Common::String inventoryItemName(byte itemId) const;
	Common::String dialogueMenuText(byte stageId, byte textRowId) const;
	bool getStageCue(uint16 rowIndex, byte frameIndex, uint16 &textRecordId,
		byte &continuationCount, uint16 &voiceSampleId) const;
	bool getStaticSpeechCue(uint16 rowIndex, byte frameIndex, uint16 &textRecordId,
		byte &continuationCount, uint16 &voiceSampleId) const;
	Common::String largeTextRecord(uint16 recordId) const;

	Common::Array<byte> decodeKey;
	Common::Array<byte> stageBlock;
	Common::Array<byte> stageSmallRows;
	Common::Array<byte> stageLargeRows;
	Common::Array<byte> staticSpeechCueDescriptors;
	Common::Array<byte> inventoryOwnerSmallRows;
	Common::Array<byte> inventoryOwnerLargeRows;

private:
	uint16 findStageVoiceSampleBase(uint largeRowCount) const;

	uint16 _stageVoiceSampleBase;
	bool _validateSequentialVoiceMap;
};

} // End of namespace Hollywood

#endif // HOLLYWOOD_SCENES_PLAYABLE_SCENE_TEXT_STORE_H
