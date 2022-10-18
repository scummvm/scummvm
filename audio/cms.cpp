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

#include "audio/cms.h"

namespace CMS {

CMS *Config::create() {
	// For now this is fixed to the DOSBox emulator.
	return new DOSBoxCMS();
}

bool CMS::_hasInstance = false;

CMS::CMS() {
	if (_hasInstance)
		error("There are multiple CMS output instances running.");
	_hasInstance = true;
}

CMS::~CMS() {
	_hasInstance = false;
}

void CMS::start(TimerCallback *callback, int timerFrequency) {
	_callback.reset(callback);
	startCallbacks(timerFrequency);
}

void CMS::stop() {
	stopCallbacks();
	_callback.reset();
}

EmulatedCMS::EmulatedCMS() :
	_nextTick(0),
	_samplesPerTick(0),
	_baseFreq(0),
	_handle(new Audio::SoundHandle()) { }

EmulatedCMS::~EmulatedCMS() {
	// Stop callbacks, just in case. If it's still playing at this
	// point, there's probably a bigger issue, though. The subclass
	// needs to call stop() or the pointer can still use be used in
	// the mixer thread at the same time.
	stop();

	delete _handle;
}

int EmulatedCMS::readBuffer(int16 *buffer, const int numSamples) {
	int len = numSamples / 2;
	int step;

	do {
		step = len;
		if (step > (_nextTick >> FIXP_SHIFT))
			step = (_nextTick >> FIXP_SHIFT);

		generateSamples(buffer, step);

		_nextTick -= step << FIXP_SHIFT;
		if (!(_nextTick >> FIXP_SHIFT)) {
			if (_callback && _callback->isValid())
				(*_callback)();

			_nextTick += _samplesPerTick;
		}

		buffer += step * 2;
		len -= step;
	} while (len);

	return numSamples;
}

int EmulatedCMS::getRate() const {
	return g_system->getMixer()->getOutputRate();
}

bool EmulatedCMS::endOfData() const {
	return false;
}

bool EmulatedCMS::isStereo() const {
	return true;
}

void EmulatedCMS::startCallbacks(int timerFrequency) {
	setCallbackFrequency(timerFrequency);
	g_system->getMixer()->playStream(Audio::Mixer::kMusicSoundType, _handle, this, -1, Audio::Mixer::kMaxChannelVolume, 0, DisposeAfterUse::NO, true);
}

void EmulatedCMS::stopCallbacks() {
	g_system->getMixer()->stopHandle(*_handle);
}

void EmulatedCMS::setCallbackFrequency(int timerFrequency) {
	_baseFreq = timerFrequency;
	assert(_baseFreq != 0);

	int d = getRate() / _baseFreq;
	int r = getRate() % _baseFreq;

	// This is equivalent to (getRate() << FIXP_SHIFT) / BASE_FREQ
	// but less prone to arithmetic overflow.

	_samplesPerTick = (d << FIXP_SHIFT) + (r << FIXP_SHIFT) / _baseFreq;
}

DOSBoxCMS::DOSBoxCMS() : _cms(nullptr) { }

DOSBoxCMS::~DOSBoxCMS() {
	if (_cms)
		delete _cms;
}

bool DOSBoxCMS::init() {
	_cms = new CMSEmulator(getRate());
	return _cms != nullptr;
}

void DOSBoxCMS::reset() {
	_cms->reset();
}

void DOSBoxCMS::write(int a, int v) {
	_cms->portWrite(a, v);
}

void DOSBoxCMS::writeReg(int r, int v) {
	int address = 0x220;
	if (r >= 0x100)
		address += 0x002;

	_cms->portWrite(address + 1, r & 0x1F);
	_cms->portWrite(address, v);
}

void DOSBoxCMS::generateSamples(int16 *buffer, int numSamples) {
	_cms->readBuffer(buffer, numSamples);
}

} // End of namespace CMS
