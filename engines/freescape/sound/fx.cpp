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

#include "audio/audiostream.h"
#include "audio/decoders/raw.h"

#include "freescape/freescape.h"

namespace Freescape {

struct soundFx {
	int size;
	float sampleRate;
	int repetitions;
	byte *data;
};

class SoundFX final : public Sound {
public:
	SoundFX(Audio::Mixer *mixer) : _mixer(mixer) {}

	~SoundFX() override {
		for (auto &it : _soundsFx) {
			if (it._value) {
				free(it._value->data);
				free(it._value);
			}
		}
	}

	void loadSounds(Common::SeekableReadStream *file, int offset, int number) {
		file->seek(offset);
		soundFx *sound = nullptr;
		_soundsFx[0] = sound;
		for (int i = 1; i < number + 1; i++) {
			sound = (soundFx *)malloc(sizeof(soundFx));
			int zero = file->readUint16BE();
			assert(zero == 0);
			int size = file->readUint16BE();
			float sampleRate = float(file->readUint16BE()) / 2;
			debugC(1, kFreescapeDebugParser, "Loading sound: %d (size: %d, sample rate: %f) at %" PRIx64, i, size, sampleRate, file->pos());
			byte *data = (byte *)malloc(size * sizeof(byte));
			file->read(data, size);
			sound->sampleRate = sampleRate;
			sound->size = size;
			sound->data = (byte *)data;
			sound->repetitions = 1;
			_soundsFx[i] = sound;
		}
	}

	void playSound(int index, Type type) override {
		if (_soundsFx.size() == 0) {
			debugC(1, kFreescapeDebugMedia, "WARNING: Sounds are not loaded");
			return;
		}

		if (index < 0 || index >= int(_soundsFx.size())) {
			debugC(1, kFreescapeDebugMedia, "WARNING: Sound %d not available", index);
			return;
		}

		int size = _soundsFx[index]->size;
		int sampleRate = _soundsFx[index]->sampleRate;
		int repetitions = _soundsFx[index]->repetitions;
		byte *data = _soundsFx[index]->data;

		if (size > 4) {
		Audio::SeekableAudioStream *s = Audio::makeRawStream(data, size, sampleRate, Audio::FLAG_16BITS, DisposeAfterUse::NO);
			Audio::AudioStream *stream = new Audio::LoopingAudioStream(s, repetitions);
			_mixer->playStream(Audio::Mixer::kSFXSoundType, &_soundFxHandle, stream);
		} else
			debugC(1, kFreescapeDebugMedia, "WARNING: Sound %d is empty", index);
	}

	void stopSound(Type type) override {
		_mixer->stopHandle(_soundFxHandle);
	}

	bool isPlayingSound(Type type) const override {
		return _mixer->isSoundHandleActive(_soundFxHandle);
	}

private:
	Common::HashMap<uint16, soundFx *> _soundsFx;

	Audio::Mixer *_mixer;
	Audio::SoundHandle _soundFxHandle;
};

Sound *FreescapeEngine::loadSoundsFx(Common::SeekableReadStream *file, int offset, int number) {
	SoundFX *sound = new SoundFX(_mixer);
	sound->loadSounds(file, offset, number);
	return sound;
}

} // namespace Freescape
