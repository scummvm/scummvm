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

#ifndef SCUMM_SOUNDSE_H
#define SCUMM_SOUNDSE_H

#include "common/scummsys.h"
#include "audio/mixer.h"
#include "scumm/file.h"

namespace Common {
class SeekableSubReadStream;
}

namespace Audio {
class SeekableAudioStream;
}

namespace Scumm {

class ScummEngine;

enum SoundSEType {
	kSoundSETypeMusic,
	kSoundSETypeSpeech,
	kSoundSETypeSFX
};

class SoundSE {

protected:
	ScummEngine *_vm;
	Audio::Mixer *_mixer;

public:
	SoundSE(ScummEngine *parent, Audio::Mixer *mixer);
	~SoundSE() = default;

	Audio::SeekableAudioStream *getXWBTrack(int track);
	Audio::AudioStream *getAudioStream(uint32 offset, SoundSEType type);
	uint32 getAudioOffsetForMI(int32 room, int32 script, int32 localScriptOffset, int32 messageIndex);

private:
	enum AudioCodec {
		kXWBCodecPCM = 0,
		kXWBCodecXMA = 1,
		kXWBCodecADPCM = 2,
		kXWBCodecWMA = 3,
		kFSBCodecMP3 = 4
	};

	enum XWBSegmentType {
		kXWBSegmentBankData = 0,
		kXWBSegmentEntryMetaData = 1,
		kXWBSegmentSeekTables = 2,
		kXWBSegmentEntryNames = 3,
		kXWBSegmentEntryWaveData = 4
	};

	struct AudioEntry {
		uint64 offset;
		uint32 length;
		AudioCodec codec;
		byte channels;
		uint16 rate;
		uint16 align;
		byte bits;
		Common::String name;
	};

	Common::HashMap<Common::String, uint32> _audioNameToOriginalOffsetMap;

	typedef Common::Array<AudioEntry> AudioIndex;
	typedef Common::HashMap<uint32, uint32> OffsetToIndexMap;

	OffsetToIndexMap _offsetToIndex;

	AudioIndex _musicEntries;
	Common::String _musicFilename;
	AudioIndex _speechEntries;
	Common::String _speechFilename;
	AudioIndex _sfxEntries;
	Common::String _sfxFilename;

	// Used in MI1 + MI2
	struct AudioEntryMI {
		uint32 hash;
		uint16 room;
		uint16 script;
		uint16 localScriptOffset;
		uint16 messageIndex;        // message index, used in messages split with wait()
		uint16 isEgoTalking;        // 1 if ego is talking, 0 otherwise
		uint16 wait;                // wait time in ms
		Common::String textEnglish; // 256 bytes, English text
		Common::String textFrench;  // 256 bytes, French text
		Common::String textItalian; // 256 bytes, Italian text
		Common::String textGerman;  // 256 bytes, German text
		Common::String textSpanish; // 256 bytes, Spanish text
		Common::String speechFile;  // 32 bytes
	};

	//typedef Common::Array<AudioEntryMI> AudioIndexMI;
	//AudioIndexMI _audioEntriesMI;

	int32 getSoundIndexFromOffset(uint32 offset);

	void initAudioMappingMI();
	void initAudioMapping();
	void initSoundFiles();
	void indexXWBFile(const Common::String &filename, AudioIndex *audioIndex);
	Audio::SeekableAudioStream *createSoundStream(Common::SeekableSubReadStream *stream, AudioEntry entry);
	void indexFSBFile(const Common::String &filename, AudioIndex *audioIndex);
};


} // End of namespace Scumm

#endif
