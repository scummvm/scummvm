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

#ifndef VCRUISE_SAMPLELOOP_H
#define VCRUISE_SAMPLELOOP_H

#include "common/ptr.h"
#include "common/mutex.h"

#include "audio/audiostream.h"

namespace Common {

class ReadStream;

} // End of namespace Common

namespace VCruise {

struct SampleLoop {
	SampleLoop();

	bool read(Common::ReadStream &stream, uint &availableBytes);

	uint32 identifier;
	uint32 type;
	uint32 start;
	uint32 end;
	uint32 fraction;
	uint32 playCount;
};

struct SampleChunk {
	SampleChunk();

	bool read(Common::ReadStream &stream, uint &availableBytes);

	uint32 manufacturer;
	uint32 product;
	uint32 samplePeriod;
	uint32 midiUnityNote;
	uint32 midiPitchFraction;
	uint32 smpteFormat;
	uint32 smpteOffset;

	// uint32 numSampleLoops;
	// uint32 sizeOfSamplerData;

	Common::Array<SampleLoop> loops;
	Common::Array<byte> samplerData;
};

struct SoundLoopInfo {
	SampleChunk _sampleChunk;

	static Common::SharedPtr<SoundLoopInfo> readFromWaveFile(Common::SeekableReadStream &stream);
};

class SampleLoopAudioStream : public Audio::AudioStream {
public:
	SampleLoopAudioStream(Audio::SeekableAudioStream *baseStream, const SoundLoopInfo *loopInfo);
	virtual ~SampleLoopAudioStream();

	void stopLooping();

	int readBuffer(int16 *buffer, const int numSamples) override;
	bool isStereo() const override;
	int getRate() const override;
	bool endOfData() const override;

private:
	struct LoopRange {
		LoopRange();

		Audio::Timestamp _restartTimestamp;

		uint _startSampleInclusive;
		uint _endSampleExclusive;
		uint _playCount;
	};

	Common::Mutex _mutex;

	int _currentSampleOffset;
	int _currentLoop;
	uint _currentLoopIteration;
	int _streamFrames;
	int _streamSamples;
	bool _terminated;
	bool _ignoreLoops;

	Common::Array<LoopRange> _loopRanges;

	Audio::SeekableAudioStream *_baseStream;
};

} // End of namespace VCruise

#endif
