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

#ifndef SCI_SOUND_DECODERS_SOL_H
#define SCI_SOUND_DECODERS_SOL_H

#include "audio/audiostream.h"
#include "common/stream.h"

namespace Sci {

enum SOLFlags {
	kCompressed = 1,
	k16Bit      = 4,
	kStereo     = 16
};

template <bool STEREO, bool S16BIT, bool OLDDPCM8>
class SOLStream : public Audio::SeekableAudioStream {
private:
	/**
	 * Read stream containing possibly-compressed SOL audio.
	 */
	Common::DisposablePtr<Common::SeekableReadStream> _stream;

	/**
	 * Sample rate of audio data.
	 */
	uint16 _sampleRate;

	/**
	 * The raw (possibly-compressed) size of audio data in the stream.
	 */
	int32 _rawDataSize;

	/**
	 * DPCM8 pop (overflow) fix working data:
	 *  - Whether or not the fix is enabled, set once on construction.
	 *  - The current state of the repair (0 = inactive, 1 = positive, 2 = negative).
	 *  - The sample state without repair, to detect where repairing should stop.
	 */
	struct PopFixData {
		const bool enabled;
		uint8 state;
		uint8 preRepairSample;

		PopFixData(const bool e):
			enabled(e), state(0), preRepairSample(0) {}
	} _popfixDPCM8;

	/**
	 * The last sample from the previous DPCM decode.
	 */
	union {
		struct { int16 l; int16 r; } _dpcmCarry16;
		struct { uint8 l; uint8 r; } _dpcmCarry8;
	};

	/**
	 * The calculated length of the stream.
	 */
	Audio::Timestamp _length;

	bool seek(const Audio::Timestamp &where) override;
	Audio::Timestamp getLength() const override;
	int readBuffer(int16 *buffer, const int numSamples) override;
	bool isStereo() const override;
	int getRate() const override;
	bool endOfData() const override;
	bool rewind() override;

public:
	SOLStream(Common::SeekableReadStream *stream, const DisposeAfterUse::Flag disposeAfterUse, const uint16 sampleRate, const int32 rawDataSize);
};

Audio::SeekableAudioStream *makeSOLStream(Common::SeekableReadStream *stream, DisposeAfterUse::Flag disposeAfterUse);

} // End of namespace Sci

#endif // SCI_SOUND_DECODERS_SOL_H
