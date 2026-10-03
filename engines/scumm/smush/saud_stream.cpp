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

#include "scumm/smush/saud_stream.h"

#include "audio/decoders/raw.h"
#include "audio/mixer.h"
#include "audio/rate.h"
#include "common/textconsole.h"
#include "common/util.h"

namespace Scumm {

SaudStreamState::SaudStreamState() {
	reset();
}

void SaudStreamState::reset() {
	Common::StackLock lock(_mutex);
	memset(_variables, 0, sizeof(_variables));
	_masterVolume = 127;
}

byte SaudStreamState::variable(byte id) const {
	Common::StackLock lock(_mutex);
	return id == SAUD_VALUEID_ALL_VOLS ? _masterVolume : _variables[id];
}

SaudStream::SaudStream(const Common::Array<byte> &samples, const Common::Array<byte> &script,
	uint rate, SaudStreamState &state, byte volume, int8 pan) :
	_samples(samples), _state(state), _raw(nullptr), _converter(nullptr), _volume(volume), _pan(pan), _ended(false) {
	_script.reset(script.begin(), script.size(), kRate);
	if (script.empty()) {
		SaudSpan span;
		span.length = samples.size();
		span.rate = rate;
		_ended = !startSpan(span);
	}
}

SaudStream::~SaudStream() {
	delete _raw;
	delete _converter;
}

bool SaudStream::startSpan(const SaudSpan &span) {
	if (span.offset >= _samples.size() || !span.length) {
		warning("SAUD: sample span %u + %u exceeds SDAT size %u", span.offset, span.length, _samples.size());
		return false;
	}
	const uint32 length = MIN<uint32>(span.length, _samples.size() - span.offset);
	if (length < span.length) {
		// DUST.SAN contains a speech script extending past its SDAT.
		warning("SAUD: truncating sample span %u + %u to SDAT size %u", span.offset, span.length, _samples.size());
		_script.reset(nullptr, 0, kRate);
	}
	_raw = Audio::makeRawStream(_samples.begin() + span.offset, length, span.rate, Audio::FLAG_UNSIGNED, DisposeAfterUse::NO);
	if (!_converter || _converter->getInputRate() != span.rate) {
		delete _converter;
		_converter = Audio::makeRateConverter(span.rate, kRate, false, true, false);
	}
	return true;
}

int SaudStream::readBuffer(int16 *buffer, int numSamples) {
	Common::StackLock lock(_state._mutex);
	const int frames = numSamples / 2;
	memset(buffer, 0, frames * 2 * sizeof(*buffer));
	int done = 0;
	uint emptySpans = 0;
	while (done < frames && !_ended) {
		if (!_raw) {
			if (++emptySpans > 65536) {
				warning("SAUD: script loop without output samples");
				_ended = true;
				break;
			}
			SaudSpan span;
			if (!_script.nextSpan(span, _state._variables, _state._masterVolume, _volume, _pan) || !startSpan(span)) {
				_ended = true;
				break;
			}
		}
		const int volume = CLIP<int>(_volume * _state._masterVolume / 127, 0, 127) * Audio::Mixer::kMaxMixerVolume / 127;
		const int pan = CLIP<int>(int8(_pan), -127, 127);
		const int left = pan > 0 ? volume * (127 - pan) / 127 : volume;
		const int right = pan < 0 ? volume * (127 + pan) / 127 : volume;
		const int count = _converter->convert(*_raw, (byte *)(buffer + done * 2), sizeof(*buffer), frames - done, left, right, Audio::MIX_ADD);
		done += count;
		if (count == 0) {
			delete _raw;
			_raw = nullptr;
		} else {
			emptySpans = 0;
		}
	}
	return done * 2;
}

void SaudStream::skip(uint64 frames) {
	int16 buffer[2048];
	while (frames && !_ended) {
		const int count = readBuffer(buffer, MIN<uint64>(frames, ARRAYSIZE(buffer) / 2) * 2) / 2;
		if (!count)
			break;
		frames -= count;
	}
}

} // End of namespace Scumm
