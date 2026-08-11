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

#include "audio/softsynth/pcspk.h"
#include "mm/shared/classic/pc_speaker.h"

namespace MM {
namespace Shared {
namespace Classic {

static const float kPitBaseFrequency = 1193182.0f;

PcSpeaker::PcSpeaker() {
	_speaker = new Audio::PCSpeaker();
}

PcSpeaker::~PcSpeaker() {
	delete _speaker;
}

bool PcSpeaker::init() {
	_ready = _speaker->init();
	return _ready;
}

void PcSpeaker::stop() {
	_loopingPitSequence = nullptr;
	_loopingPitTickMicros = 0;

	if (_ready)
		_speaker->stop();
}

bool PcSpeaker::isPlaying() const {
	return _ready && _speaker->isPlaying();
}

void PcSpeaker::playTone(int frequency, int32 lengthMs) {
	if (_ready)
		_speaker->play(Audio::PCSpeaker::kWaveFormSquare, frequency, lengthMs);
}

void PcSpeaker::queueTone(float frequency, uint32 lengthUs) {
	if (_ready)
		_speaker->playQueue(Audio::PCSpeaker::kWaveFormSquare, frequency, lengthUs);
}

void PcSpeaker::queueSilence(uint32 lengthUs) {
	if (_ready)
		_speaker->playQueue(Audio::PCSpeaker::kWaveFormSilence, 0.0f, lengthUs);
}

void PcSpeaker::queuePitSequence(const PitSequenceEntry *sequence, uint32 tickMicros) {
	for (const PitSequenceEntry *entry = sequence; entry->durationTicks != 0; ++entry) {
		const uint32 lengthUs = entry->durationTicks * tickMicros;
		if (entry->pitDivisor == 0) {
			queueSilence(lengthUs);
		} else {
			queueTone(kPitBaseFrequency / entry->pitDivisor, lengthUs);
		}
	}
}

void PcSpeaker::playPitSequence(const PitSequenceEntry *sequence, uint32 tickMicros, bool append, bool loop) {
	if (!_ready || !sequence)
		return;

	if (!append)
		stop();

	_loopingPitSequence = loop ? sequence : nullptr;
	_loopingPitTickMicros = loop ? tickMicros : 0;
	queuePitSequence(sequence, tickMicros);
}

void PcSpeaker::playFrequencySequence(const FrequencySequenceEntry *sequence) {
	if (!_ready || !sequence)
		return;

	stop();

	for (const FrequencySequenceEntry *entry = sequence; entry->lengthUs != 0; ++entry) {
		if (entry->frequency == 0.0f)
			queueSilence(entry->lengthUs);
		else
			queueTone(entry->frequency, entry->lengthUs);
	}
}

void PcSpeaker::update() {
	if (_loopingPitSequence && !isPlaying())
		queuePitSequence(_loopingPitSequence, _loopingPitTickMicros);
}

} // namespace Classic
} // namespace Shared
} // namespace MM
