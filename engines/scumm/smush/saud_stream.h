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
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program. If not, see <http://www.gnu.org/licenses/>.
 */

#ifndef SCUMM_SMUSH_SAUD_STREAM_H
#define SCUMM_SMUSH_SAUD_STREAM_H

#include "audio/audiostream.h"
#include "common/array.h"
#include "common/mutex.h"
#include "scumm/smush/saud.h"

namespace Audio {
class RateConverter;
}

namespace Scumm {

class SaudStreamState {
public:
	SaudStreamState();
	void reset();
	byte variable(byte id) const;

private:
	friend class SaudStream;
	Common::Mutex _mutex;
	byte _variables[256];
	int _masterVolume;
};

// Plays an in-memory SAUD through the same STRK interpreter as RebelAudio.
// The caller keeps samples, script and shared state alive until playback stops.
class SaudStream : public Audio::AudioStream {
public:
	enum { kRate = 22050 };
	SaudStream(const Common::Array<byte> &samples, const Common::Array<byte> &script,
		uint rate, SaudStreamState &state, byte volume, int8 pan);
	~SaudStream() override;

	int readBuffer(int16 *buffer, int numSamples) override;
	bool isStereo() const override { return true; }
	int getRate() const override { return kRate; }
	bool endOfData() const override { return _ended; }
	// Advance the script and audio together before handing the stream to the mixer.
	void skip(uint64 frames);

private:
	bool startSpan(const SaudSpan &span);

	const Common::Array<byte> &_samples;
	SaudStreamState &_state;
	SaudScript _script;
	Audio::AudioStream *_raw;
	Audio::RateConverter *_converter;
	byte _volume, _pan;
	bool _ended;
};

} // End of namespace Scumm

#endif
