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

#include "audio/chip.h"
#include "audio/mixer.h"

#include "common/timer.h"

namespace Audio {

void Chip::start(TimerCallback *callback, int timerFrequency) {
	_callback.reset(callback);
	startCallbacks(timerFrequency);
}

void Chip::stop() {
	stopCallbacks();
	_callback.reset();
}

RealChip::RealChip() : _baseFreq(0), _remainingTicks(0) {
}

RealChip::~RealChip() {
	// Stop callbacks, just in case. If it's still playing at this
	// point, there's probably a bigger issue, though. The subclass
	// needs to call stop() or the pointer can still use be used in
	// the mixer thread at the same time.
	stop();
}

void RealChip::setCallbackFrequency(int timerFrequency) {
	stopCallbacks();
	startCallbacks(timerFrequency);
}

void RealChip::startCallbacks(int timerFrequency) {
	_baseFreq = timerFrequency;
	assert(_baseFreq > 0);

	// We can't request more a timer faster than 100Hz. We'll handle this by calling
	// the proc multiple times in onTimer() later on.
	if (timerFrequency > kMaxFreq)
		timerFrequency = kMaxFreq;

	_remainingTicks = 0;
	g_system->getTimerManager()->installTimerProc(timerProc, 1000000 / timerFrequency, this, "RealChip");
}

void RealChip::stopCallbacks() {
	g_system->getTimerManager()->removeTimerProc(timerProc);
	_baseFreq = 0;
	_remainingTicks = 0;
}

void RealChip::timerProc(void *refCon) {
	static_cast<RealChip *>(refCon)->onTimer();
}

void RealChip::onTimer() {
	uint callbacks = 1;

	if (_baseFreq > kMaxFreq) {
		// We run faster than our max, so run the callback multiple
		// times to approximate the actual timer callback frequency.
		uint totalTicks = _baseFreq + _remainingTicks;
		callbacks = totalTicks / kMaxFreq;
		_remainingTicks = totalTicks % kMaxFreq;
	}

	// Call the callback multiple times. The if is on the inside of the
	// loop in case the callback removes itself.
	for (uint i = 0; i < callbacks; i++)
		if (_callback && _callback->isValid())
			(*_callback)();
}

EmulatedChip::EmulatedChip() :
	_nextTick(0),
	_samplesPerTick(0),
	_baseFreq(0),
	_handle(new Audio::SoundHandle()) { }

EmulatedChip::~EmulatedChip() {
	// Stop callbacks, just in case. If it's still playing at this
	// point, there's probably a bigger issue, though. The subclass
	// needs to call stop() or the pointer can still use be used in
	// the mixer thread at the same time.
	stop();

	delete _handle;
}

int EmulatedChip::readBuffer(int16 *buffer, const int numSamples) {
	const int stereoFactor = isStereo() ? 2 : 1;
	int len = numSamples / stereoFactor;
	int step;

	do {
		step = len;
		if (step > (_nextTick >> FIXP_SHIFT))
			step = (_nextTick >> FIXP_SHIFT);

		generateSamples(buffer, step * stereoFactor);

		_nextTick -= step << FIXP_SHIFT;
		if (!(_nextTick >> FIXP_SHIFT)) {
			if (_callback && _callback->isValid())
				(*_callback)();

			_nextTick += _samplesPerTick;
		}

		buffer += step * stereoFactor;
		len -= step;
	} while (len);

	return numSamples;
}

int EmulatedChip::getRate() const {
	return g_system->getMixer()->getOutputRate();
}

void EmulatedChip::startCallbacks(int timerFrequency) {
	setCallbackFrequency(timerFrequency);
	g_system->getMixer()->playStream(Audio::Mixer::kPlainSoundType, _handle, this, -1, Audio::Mixer::kMaxChannelVolume, 0, DisposeAfterUse::NO, true);
}

void EmulatedChip::stopCallbacks() {
	g_system->getMixer()->stopHandle(*_handle);
}

void EmulatedChip::setCallbackFrequency(int timerFrequency) {
	_baseFreq = timerFrequency;
	assert(_baseFreq != 0);

	int d = getRate() / _baseFreq;
	int r = getRate() % _baseFreq;

	// This is equivalent to (getRate() << FIXP_SHIFT) / BASE_FREQ
	// but less prone to arithmetic overflow.

	_samplesPerTick = (d << FIXP_SHIFT) + (r << FIXP_SHIFT) / _baseFreq;
}

} // End of namespace Audio
