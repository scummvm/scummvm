/* ScummVM - Graphic Adventure Engine
 *
 * ScummVM is the legal property of its developers, whose names
 * are too numerous to list here. Please refer to the COPYRIGHT
 * file distributed with this source distribution.
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License
 * as published by the Free Software Foundation; either version 2
 * of the License, or (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.
 *
 */

#ifndef MOHAWK_SOUND_H
#define MOHAWK_SOUND_H

#include "common/array.h"
#include "common/scummsys.h"
#include "common/str.h"
#include "common/stream.h"

#include "audio/mixer.h"

class MidiDriver;
class MidiParser;

namespace Audio {
class RewindableAudioStream;
}

namespace Mohawk {

#define MAX_CHANNELS 2         // Can there be more than 2?

enum SndHandleType {
	kFreeHandle,
	kUsedHandle
};

struct SndHandle {
	Audio::SoundHandle handle;
	SndHandleType type;
	uint samplesPerSecond;
	uint16 id;
};

struct ADPCMStatus { // Holds ADPCM status data, but is irrelevant for us.
	uint32 size;
	uint16 itemCount;
	uint16 channels;

	struct StatusItem {
		uint32 sampleFrame;
		struct {
			int16 last;
			uint16 stepIndex;
		} channelStatus[MAX_CHANNELS];
	} *statusItems;
};

struct CueListPoint {
	uint32 sampleFrame;
	Common::String name;
};

struct CueList {
	uint32 size;
	uint16 pointCount;
	Common::Array<CueListPoint> points;
};

enum {
	kCodecRaw = 0,
	kCodecADPCM = 1,
	kCodecMPEG2 = 2
};

struct DataChunk {
	uint16 sampleRate;
	uint32 sampleCount;
	byte bitsPerSample;
	byte channels;
	uint16 encoding;
	uint16 loopCount; // 0 == no looping, 0xFFFF == infinite loop
	uint32 loopStart;
	uint32 loopEnd;
	Common::SeekableReadStream *audioData;
};

Audio::RewindableAudioStream *makeMohawkWaveStream(Common::SeekableReadStream *stream, CueList *cueList = nullptr);

class MohawkEngine;

class Sound {
public:
	explicit Sound(MohawkEngine *vm);
	~Sound();

	// Generic sound functions
	Audio::SoundHandle *playSound(uint16 id, byte volume = Audio::Mixer::kMaxChannelVolume, bool loop = false, CueList *cueList = NULL);
	void stopSound();
	void stopSound(uint16 id);
	bool isPlaying(uint16 id);
	bool isPlaying();
	uint getNumSamplesPlayed(uint16 id);

private:
	MohawkEngine *_vm;

	static Audio::RewindableAudioStream *makeLivingBooksWaveStream_v1(Common::SeekableReadStream *stream);

	Common::Array<SndHandle> _handles;
	SndHandle *getHandle();
	Audio::RewindableAudioStream *makeAudioStream(uint16 id, CueList *cueList = nullptr);
};

} // End of namespace Mohawk

#endif
