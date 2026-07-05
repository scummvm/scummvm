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

#ifndef HOLLYWOOD_SCENES_INTRO_INTRO_TEXT_H
#define HOLLYWOOD_SCENES_INTRO_INTRO_TEXT_H

#include "common/array.h"
#include "common/str.h"
#include "common/types.h"

namespace Hollywood {

// Minimal RESOURCE.003 text/cue loader for non-playable intro cutscenes.
class IntroTextStore {
public:
	IntroTextStore();

	bool loadStage(const char *archiveName, const char *sceneDebugName, uint stageIndex);
	bool loadStaticSpeechCues(const char *archiveName, const char *sceneDebugName,
		uint32 speechCueDescriptorOffset);
	bool getStageCue(uint16 rowIndex, byte frameIndex, uint16 &textRecordId,
		byte &continuationCount, uint16 &voiceSampleId) const;
	bool getStaticSpeechCue(uint16 rowIndex, byte frameIndex, uint16 &textRecordId,
		byte &continuationCount, uint16 &voiceSampleId) const;
	Common::String largeTextRecord(uint16 recordId) const;

private:
	Common::Array<byte> _decodeKey;
	Common::Array<byte> _stageBlock;
	Common::Array<byte> _stageLargeRows;
	Common::Array<byte> _staticSpeechCueDescriptors;
};

} // End of namespace Hollywood

#endif // HOLLYWOOD_SCENES_INTRO_INTRO_TEXT_H
