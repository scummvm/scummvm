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

#include "common/endian.h"
#include "common/mutex.h"
#include "freescape/games/3dck/3dck.h"
#include "freescape/sound/3dck.h"

namespace Freescape {

class KitSpeakerSound : public Sound, public Audio::AudioStream {
public:
	KitSpeakerSound(Audio::Mixer *mixer, const Common::Array<byte> &data);
	~KitSpeakerSound() override { _mixer->stopHandle(_handle); }

	void playSound(int index, Type type) override;
	void stopSound(Type type) override;
	bool isPlayingSound(Type type) const override;
	bool isSoundAvailable(int index) const override;
	int readBuffer(int16 *buffer, int samples) override;
	int getRate() const override { return _mixer->getOutputRate(); }
	bool isStereo() const override { return false; }
	bool endOfData() const override { return false; }

private:
	void loadPart();
	void tick();

	struct Part {
		byte steps, duration;
		uint16 delta;
	};
	struct Effect {
		uint16 divisor;
		byte repeats;
		Common::Array<Part> parts;
	};
	Common::Array<Effect> _effects;
	Audio::Mixer *_mixer;
	Audio::SoundHandle _handle;
	mutable Common::Mutex _mutex;
	const Effect *_effect = nullptr;
	uint _part = 0;
	byte _steps = 0, _duration = 0, _repeats = 0;
	uint16 _divisor = 0;
	uint64 _phase = 0, _timer = 0;
	Type _type = kTypeNormal;
};

KitSpeakerSound::KitSpeakerSound(Audio::Mixer *mixer, const Common::Array<byte> &data) : _mixer(mixer) {
	uint count = READ_LE_UINT16(&data[0x323]);
	if (0x325 + count * 4 > 0x375)
		error("Invalid 3D Construction Kit speaker table");
	_effects.resize(count);
	for (uint i = 0; i < count; i++) {
		const byte *entry = &data[0x325 + i * 4];
		if (entry[0] == 0xff)
			continue;
		uint offset = 0x375 + entry[0] * 5;
		if (offset >= data.size() || !data[offset] || offset + 1 + data[offset] * 4 > data.size())
			error("Invalid 3D Construction Kit speaker pattern");
		Effect &effect = _effects[i];
		effect.divisor = READ_LE_UINT16(entry + 1);
		effect.repeats = entry[3];
		effect.parts.resize(data[offset++]);
		for (Part &part : effect.parts) {
			part.steps = data[offset];
			part.delta = READ_LE_UINT16(&data[offset + 1]);
			part.duration = data[offset + 3];
			offset += 4;
		}
	}
	_mixer->playStream(Audio::Mixer::kSFXSoundType, &_handle, this, -1,
		Audio::Mixer::kMaxChannelVolume, 0, DisposeAfterUse::NO);
}

bool KitSpeakerSound::isSoundAvailable(int index) const {
	return index > 0 && uint(index) <= _effects.size() && !_effects[index - 1].parts.empty();
}

void KitSpeakerSound::playSound(int index, Type type) {
	if (!isSoundAvailable(index))
		return;
	Common::StackLock lock(_mutex);
	_effect = &_effects[index - 1];
	_type = type;
	_part = 0;
	_repeats = _effect->repeats;
	_divisor = _effect->divisor;
	_phase = 0;
	loadPart();
}

void KitSpeakerSound::loadPart() {
	_steps = _effect->parts[_part].steps;
	_duration = _effect->parts[_part].duration;
}

void KitSpeakerSound::tick() {
	if (!_effect || --_duration)
		return;
	_duration = _effect->parts[_part].duration;
	_divisor += _effect->parts[_part].delta;
	_phase = 0;
	if (--_steps)
		return;
	if (++_part == _effect->parts.size()) {
		if (!--_repeats) {
			_effect = nullptr;
			return;
		}
		_part = 0;
	}
	loadPart();
}

int KitSpeakerSound::readBuffer(int16 *buffer, int samples) {
	Common::StackLock lock(_mutex);
	// The DOS IRQ uses PIT reload 3637; channel 2 holds an unsigned divisor.
	uint64 interval = uint64(getRate()) * 3637;
	for (int i = 0; i < samples; i++) {
		if (_effect) {
			uint64 period = uint64(_divisor ? _divisor : 65536) * getRate();
			_phase %= period;
			buffer[i] = _phase < period / 2 ? 2540 : -2540;
			_phase += kKitPITClock;
		} else {
			buffer[i] = 0;
		}
		_timer += kKitPITClock;
		while (_timer >= interval) {
			_timer -= interval;
			tick();
		}
	}
	return samples;
}

void KitSpeakerSound::stopSound(Type type) {
	Common::StackLock lock(_mutex);
	if (type == kTypeNormal || _type == type)
		_effect = nullptr;
}

bool KitSpeakerSound::isPlayingSound(Type type) const {
	Common::StackLock lock(_mutex);
	return _effect && (type == kTypeNormal || _type == type);
}

void KitEngine::loadSounds(Common::SeekableReadStream &file) {
	Common::Array<byte> data;
	data.resize(file.size());
	if (data.size() < 0x108 || file.read(data.data(), data.size()) != data.size() ||
			memcmp(data.data(), "3D Construction Kit (c) Incentive Software.", 42))
		error("Invalid 3D Construction Kit sound driver");
	uint16 init = READ_LE_UINT16(&data[0x100]);
	uint16 play = READ_LE_UINT16(&data[0x104]);
	if (init == 0x124 && play == 0x1c5 && data.size() >= 0x375)
		_sound = new KitSpeakerSound(_mixer, data);
	else if (init == 0x79a && play == 0x864 && data.size() >= 0x4480)
		_sound = createKitAdLibSound(_mixer, data);
	else if (init != 0x115 || play != 0x1a7)
		warning("Unsupported 3D Construction Kit sound driver");
}

void KitEngine::playPendingSound() {
	if (_pendingSound >= 0 && _sound)
		_sound->playSound(_pendingSound, Sound::kTypeNormal);
	_pendingSound = -1;
}

} // namespace Freescape
