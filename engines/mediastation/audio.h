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

#ifndef MEDIASTATION_AUDIO_H
#define MEDIASTATION_AUDIO_H

#include "common/array.h"
#include "audio/audiostream.h"
#include "audio/mixer.h"

#include "mediastation/datafile.h"

namespace MediaStation {

class AudioSequence {
public:
	AudioSequence() {};
	~AudioSequence();

	void play();
	void stop();

	void readParameters(Chunk &chunk);
	void readChunk(Chunk &chunk);
	bool isActive();
	bool isEmpty() { return _streams.empty(); }

	uint _rate = 0;
	uint _channelCount = 0;
	uint _bitsPerSample = 0;
	uint _chunkCount = 0;

private:
	Common::Array<Audio::SeekableAudioStream *> _streams;
	Audio::SoundHandle _handle;
};

} // End of namespace MediaStation

#endif
