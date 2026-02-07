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

class SoundFX_DOS final : public Sound {
public:
	SoundFX_DOS(Audio::Mixer *mixer) : _mixer(mixer) {}

	~SoundFX_DOS() override {
		for (auto &it : _soundsFx) {
			if (it._value) {
				free(it._value->data);
				free(it._value);
			}
		}
	}

	void loadSounds(Common::SeekableReadStream *file, int offset, int number) {
		for (int i = 0; i < number; i++) {
			_soundsFx[i] = load1bPCM(file, offset);
			offset += (_soundsFx[i]->size / 8) + 4;
		}
	}

	void playSound(int index) override {
		if (_soundsFx.size() == 0) {
			debugC(1, kFreescapeDebugMedia, "WARNING: Sounds are not loaded");
			return;
		}

		int size = _soundsFx[index]->size;
		//int sampleRate = _soundsFx[index]->sampleRate;
		byte *data = _soundsFx[index]->data;

		Audio::SeekableAudioStream *stream = Audio::makeRawStream(data, size, 11025, Audio::FLAG_UNSIGNED, DisposeAfterUse::NO);
		_mixer->playStream(Audio::Mixer::kSFXSoundType, &_soundFxHandle, stream, -1, kFreescapeDefaultVolume / 10);
	}

	void stopSound() override {
		_mixer->stopHandle(_soundFxHandle);
	}

	bool isPlayingSound() const override {
		return _mixer->isSoundHandleActive(_soundFxHandle);
	}

private:
	Common::HashMap<uint16, soundFx *> _soundsFx;

	Audio::Mixer *_mixer;
	Audio::SoundHandle _soundFxHandle;

	soundFx *load1bPCM(Common::SeekableReadStream *file, int offset) {
		soundFx *sound = (soundFx *)malloc(sizeof(soundFx));
		file->seek(offset);
		sound->size = file->readUint16LE();
		debugC(1, kFreescapeDebugParser, "size: %d", sound->size);
		sound->sampleRate = file->readUint16LE();
		debugC(1, kFreescapeDebugParser, "sample rate?: %f", sound->sampleRate);

		uint8 *data = (uint8 *)malloc(sound->size * sizeof(uint8) * 8);
		for (int i = 0; i < sound->size; i++) {
			uint8 byte = file->readByte();
			for (int j = 0; j < 8; j++) {
				data[8 * i + j] = byte & 1 ? 255 : 0;
				byte = byte >> 1;
			}
		}
		sound->size = sound->size * 8;
		sound->data = (byte *)data;
		return sound;
	}
};


Sound *FreescapeEngine::loadSoundsFxDOS(Common::SeekableReadStream *file, int offset, int number) {
	SoundFX_DOS *sound = new SoundFX_DOS(_mixer);
	sound->loadSounds(file, offset, number);
	return sound;
}

} // End of namespace Freescape
