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

#ifndef AWE_SOUND_H
#define AWE_SOUND_H

#include "audio/mixer.h"
#include "awe/intern.h"

namespace Awe {

enum MixerType {
	kMixerTypeRaw,
	kMixerTypeWav,
	kMixerTypeAiff
};

class Sound {
private:
	Audio::Mixer *_mixer;

public:
	Sound(Audio::Mixer *mixer) : _mixer(mixer) {
	}

	void stopAll() {
		_mixer->stopAll();
	}
};

#ifdef TODO
struct AifcPlayer;
struct SfxPlayer;
struct Mixer_impl;

struct Mixer {
	AifcPlayer *_aifc;
	SfxPlayer *_sfx;
	Mixer_impl *_impl;

	Mixer(SfxPlayer *sfx);
	void init(MixerType mixerType);
	void quit();
	void update();

	void playSoundRaw(uint8_t channel, const uint8_t *data, uint16_t freq, uint8_t volume);
	void playSoundWav(uint8_t channel, const uint8_t *data, uint16_t freq, uint8_t volume, uint8_t loop);
	void stopSound(uint8_t channel);
	void setChannelVolume(uint8_t channel, uint8_t volume);
	void playMusic(const char *path, uint8_t loop);
	void stopMusic();
	void playAifcMusic(const char *path, uint32_t offset);
	void stopAifcMusic();
	void playSfxMusic(int num);
	void stopSfxMusic();
	void stopAll();
	void preloadSoundAiff(uint8_t num, const uint8_t *data);
	void playSoundAiff(uint8_t channel, uint8_t num, uint8_t volume);
};
#endif

} // namespace Awe

#endif
