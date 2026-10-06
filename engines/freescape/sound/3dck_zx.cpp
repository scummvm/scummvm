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

#include "freescape/games/3dck/8bit.h"

namespace Freescape {

class KitZXSound : public Sound {
public:
	KitZXSound(Audio::Mixer *mixer, const byte *data, uint size);
	~KitZXSound() override { _mixer->stopHandle(_handle); }

	void playSound(int index, Type type) override;
	void stopSound(Type type) override;
	bool isPlayingSound(Type type) const override;
	bool isSoundAvailable(int index) const override { return index >= 0 && index <= 255; }

private:
	struct Sweep {
		byte steps;
		int8 step;
		byte duration;
	};
	struct Effect {
		uint16 period;
		int8 step;
		byte repeats;
		Common::Array<Sweep> sweeps;
	};
	Effect _effects[16];
	Audio::Mixer *_mixer;
	Audio::SoundHandle _handle;
	SizedPCSpeaker _speaker;
	Type _type = kTypeNormal;
};

KitZXSound::KitZXSound(Audio::Mixer *mixer, const byte *data, uint size) : _mixer(mixer) {
	for (uint index = 1; index < ARRAYSIZE(_effects); ++index) {
		uint offset = 16 + data[index];
		if (offset + 5 > size)
			error("Invalid 3D Construction Kit Spectrum sound offset");
		Effect &effect = _effects[index];
		effect.period = READ_LE_UINT16(data + offset);
		effect.step = int8(data[offset + 2]);
		effect.repeats = data[offset + 3];
		uint count = data[offset + 4] ? data[offset + 4] : 256;
		offset += 5;
		if (offset + 3 * count > size)
			error("Invalid 3D Construction Kit Spectrum sound sweep");
		for (uint i = 0; i < count; ++i) {
			Sweep sweep = {data[offset], int8(data[offset + 1]), data[offset + 2]};
			effect.sweeps.push_back(sweep);
			offset += 3;
		}
	}
}

void KitZXSound::playSound(int index, Type type) {
	if (!isSoundAvailable(index) || !(index & 15))
		return;
	stopSound(kTypeNormal);
	_type = type;
	const Effect &effect = _effects[index & 15];
	uint16 base = effect.period;
	uint repeats = effect.repeats ? effect.repeats : 256;
	for (uint repeat = 0; repeat < repeats; ++repeat) {
		uint16 period = base;
		for (const Sweep &sweep : effect.sweeps) {
			uint steps = sweep.steps ? sweep.steps : 256;
			for (uint step = 0; step < steps; ++step) {
				uint16 cycles = period ? (208 * sweep.duration) / period + 1 : 0;
				int16 delay = 7 * period - 30;
				if (delay < 0)
					delay = 1;
				// Runner 0xba90 uses 8 * HL + 236 T-states per beeper cycle.
				float frequency = 3500000.0f / (8 * delay + 236);
				uint32 duration = uint32(1000000.0f * (uint32(cycles) + 1) / frequency);
				_speaker.playQueue(Audio::PCSpeaker::kWaveFormSquare, frequency, duration);
				period = (period + sweep.step) & 0xfff;
			}
		}
		base += effect.step;
	}
	_mixer->playStream(Audio::Mixer::kSFXSoundType, &_handle, &_speaker, -1,
		kFreescapeDefaultVolume, 0, DisposeAfterUse::NO);
}

void KitZXSound::stopSound(Type type) {
	if (type == kTypeNormal || type == _type) {
		_mixer->stopHandle(_handle);
		_speaker.stop();
	}
}

bool KitZXSound::isPlayingSound(Type type) const {
	return (type == kTypeNormal || type == _type) && _speaker.isPlaying();
}

void Kit8Engine::loadSoundsZX() {
	Common::File file;
	if (!file.open("3dkit.zx.code"))
		error("Missing 3D Construction Kit Spectrum sounds");
	byte data[118];
	file.seek(0x1acc);
	if (file.read(data, sizeof(data)) != sizeof(data))
		error("Truncated 3D Construction Kit Spectrum sound bank");
	_sound = new KitZXSound(_mixer, data, sizeof(data));
}

} // namespace Freescape

