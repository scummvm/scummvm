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
 *
 */

#include "audio/softsynth/ay8912.h"
#include "common/endian.h"
#include "common/mutex.h"

#include "freescape/games/3dck/8bit.h"

namespace Freescape {

// Standard CPC runner bank at 0x9a9f, also used for editor data without a runner.
static const byte kKitCPCSounds[13][16] = {
	{0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x3f, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00}, // Silence
	{0x0f, 0x00, 0x0f, 0x00, 0x0f, 0x00, 0x00, 0x38, 0x0f, 0x0f, 0x0f, 0x01, 0x00, 0xff, 0x00, 0x1e}, // Ping
	{0x00, 0x0c, 0x01, 0x0c, 0x02, 0x0c, 0x00, 0x38, 0x0f, 0x0f, 0x0f, 0x00, 0x10, 0x00, 0x00, 0x40}, // Buzz
	{0x11, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x3e, 0x0f, 0x0f, 0x0f, 0x00, 0x14, 0x00, 0x00, 0x10}, // Fire
	{0x40, 0x01, 0x80, 0x01, 0xc0, 0x01, 0x00, 0x38, 0x0f, 0x0f, 0x0f, 0x01, 0xa0, 0x00, 0x00, 0x10}, // Activate
	{0xc0, 0x00, 0x00, 0x09, 0x00, 0x07, 0x10, 0x38, 0x0f, 0x0f, 0x0f, 0x00, 0x7f, 0x00, 0xa0, 0x0a}, // Bump
	{0xf0, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x36, 0x0f, 0x0f, 0x0f, 0x01, 0x08, 0xff, 0x01, 0x1c}, // Fall
	{0x00, 0x10, 0x10, 0x10, 0x20, 0x10, 0x00, 0x38, 0x0f, 0x0f, 0x0f, 0x00, 0x04, 0x00, 0x00, 0x90}, // Fail
	{0x00, 0x08, 0x10, 0x08, 0x20, 0x08, 0x00, 0x38, 0x0f, 0x0f, 0x0f, 0x00, 0xf0, 0x00, 0x00, 0x80}, // Bonus 1
	{0x00, 0x08, 0x40, 0x08, 0x80, 0x08, 0x00, 0x30, 0x0f, 0x0f, 0x0f, 0x00, 0xe0, 0x00, 0x04, 0x41}, // Bonus 2
	{0x00, 0x01, 0x00, 0x02, 0x00, 0x03, 0x00, 0x38, 0x0f, 0x0f, 0x0f, 0x00, 0xf0, 0x00, 0xff, 0x34}, // Bonus 3
	{0x00, 0x08, 0x10, 0x08, 0x20, 0x08, 0x20, 0x36, 0x0f, 0x0f, 0x0f, 0x00, 0x01, 0x00, 0x01, 0x20}, // Door open
	{0x00, 0x08, 0x10, 0x08, 0x20, 0x08, 0x20, 0x36, 0x0f, 0x0f, 0x0f, 0x00, 0xff, 0x00, 0xff, 0x20}  // Door close
};

class KitCPCSound : public Sound, public Audio::AudioStream {
public:
	KitCPCSound(Audio::Mixer *mixer, const byte *data);
	~KitCPCSound() override { _mixer->stopHandle(_handle); }

	void playSound(int index, Type type) override;
	void stopSound(Type type) override;
	bool isPlayingSound(Type type) const override;
	bool isSoundAvailable(int index) const override { return index >= 0 && index < ARRAYSIZE(_effects); }
	int readBuffer(int16 *buffer, int samples) override;
	int getRate() const override { return _psg.getRate(); }
	bool isStereo() const override { return true; }
	bool endOfData() const override { return false; }

private:
	struct Effect {
		uint16 tone[3];
		byte noise, mixer, volume[3], delay;
		int8 toneStep, volumeStep, noiseStep;
		byte duration;
	};
	void tick();
	void silence();
	void writeRegisters();

	Effect _effects[13], _effect = {};
	Audio::Mixer *_mixer;
	Audio::SoundHandle _handle;
	Audio::AY8912Stream _psg;
	mutable Common::Mutex _mutex;
	Type _type = kTypeNormal;
	bool _active = false;
	byte _delay = 0;
	int _samplesLeft = 0;
};

KitCPCSound::KitCPCSound(Audio::Mixer *mixer, const byte *data) : _mixer(mixer), _psg(62500, 1000000) {
	// An integral AY clock divisor keeps pitch exact with AY8912Stream.
	for (Effect &effect : _effects) {
		for (int channel = 0; channel < 3; channel++) {
			effect.tone[channel] = READ_LE_UINT16(data + 2 * channel);
			effect.volume[channel] = data[8 + channel];
		}
		effect.noise = data[6];
		effect.mixer = data[7];
		effect.delay = data[11];
		effect.toneStep = int8(data[12]);
		effect.volumeStep = int8(data[13]);
		effect.noiseStep = int8(data[14]);
		effect.duration = data[15];
		data += 16;
	}
	silence();
	_mixer->playStream(Audio::Mixer::kSFXSoundType, &_handle, this, -1,
		kFreescapeDefaultVolume, 0, DisposeAfterUse::NO);
}

void KitCPCSound::silence() {
	_active = false;
	_psg.setReg(7, 0x3f);
	for (int channel = 0; channel < 3; channel++)
		_psg.setReg(8 + channel, 0);
}

void KitCPCSound::playSound(int index, Type type) {
	if (!isSoundAvailable(index))
		return;
	Common::StackLock lock(_mutex);
	_effect = _effects[index];
	_type = type;
	_delay = 0;
	_active = _effect.mixer != 0x3f;
	if (!_active)
		silence();
}

void KitCPCSound::stopSound(Type type) {
	Common::StackLock lock(_mutex);
	if (type == kTypeNormal || type == _type)
		silence();
}

bool KitCPCSound::isPlayingSound(Type type) const {
	Common::StackLock lock(_mutex);
	return _active && (type == kTypeNormal || type == _type);
}

void KitCPCSound::writeRegisters() {
	for (int channel = 0; channel < 3; channel++) {
		_psg.setReg(2 * channel, _effect.tone[channel]);
		_psg.setReg(2 * channel + 1, _effect.tone[channel] >> 8);
		_psg.setReg(8 + channel, _effect.volume[channel]);
	}
	_psg.setReg(6, _effect.noise);
	_psg.setReg(7, _effect.mixer);
	// The runner writes its modulation bytes to envelope registers too.
	_psg.setReg(11, _effect.delay);
	_psg.setReg(12, _effect.toneStep);
	_psg.setReg(13, _effect.volumeStep);
}

void KitCPCSound::tick() {
	if (!_active)
		return;
	// CPC routine 0x9a14 decrements duration before the modulation delay.
	if (!--_effect.duration) {
		silence();
		return;
	}
	if (_delay) {
		--_delay;
		return;
	}
	_delay = _effect.delay;
	writeRegisters();
	for (int channel = 0; channel < 3; channel++) {
		_effect.tone[channel] += _effect.toneStep;
		_effect.volume[channel] += _effect.volumeStep;
	}
	_effect.noise += _effect.noiseStep;
}

int KitCPCSound::readBuffer(int16 *buffer, int samples) {
	Common::StackLock lock(_mutex);
	assert(!(samples & 1));
	int done = 0;
	while (done < samples) {
		if (!_samplesLeft) {
			tick();
			_samplesLeft = 2 * getRate() / 50;
		}
		int count = MIN(samples - done, _samplesLeft);
		_psg.readBuffer(buffer + done, count);
		done += count;
		_samplesLeft -= count;
	}
	return samples;
}

void Kit8Engine::loadSounds() {
	if (isC64()) {
		warning("3D Construction Kit C64 sound effects are not implemented");
		return;
	}
	if (isSpectrum()) {
		loadSoundsZX();
		return;
	}
	byte data[sizeof(kKitCPCSounds)];
	memcpy(data, kKitCPCSounds, sizeof(data));
	Common::File file;
	const char *runner = _gameDescription->filesDescriptions[1].fileName;
	if (file.open(runner ? runner : "DISC.BIN") && file.size() == 25216) {
		byte header[128];
		file.read(header, sizeof(header));
		if (READ_LE_UINT16(header + 21) == 0x3e00 && READ_LE_UINT16(header + 24) == 25088) {
			file.seek(128 + 0x9a9f - 0x3e00);
			if (file.read(data, sizeof(data)) != sizeof(data))
				error("Truncated 3D Construction Kit CPC sound bank");
		}
	}
	_sound = new KitCPCSound(_mixer, data);
}

void Kit8Engine::playSound(int index, bool sync, Sound::Type type) {
	if (!_sound || !_sound->isSoundAvailable(index))
		return;
	if (type == Sound::kTypeMovement) {
		// A bump must not replace an already pending shot or scripted effect.
		if (index != _soundIndexCollide || !_pendingSound)
			_pendingSound = index;
	} else if (sync) {
		// SYNCSND 0 cancels the pending effect in the CPC runner.
		_pendingSound = index;
	} else
		_sound->playSound(index, type);
}

void Kit8Engine::playPendingSound() {
	if (!_soundSyncReady)
		return;
	if (_pendingSound && _sound)
		_sound->playSound(_pendingSound, Sound::kTypeNormal);
	_pendingSound = 0;
	_soundSyncReady = false;
}

} // namespace Freescape
