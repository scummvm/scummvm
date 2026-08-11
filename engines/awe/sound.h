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
#include "audio/audiostream.h"
#include "awe/intern.h"
#include "awe/sfx_player.h"

namespace Awe {

#define MAX_CHANNELS 8

class SfxMusicStream : public Audio::AudioStream {
private:
	SfxPlayer *_player;

public:
	explicit SfxMusicStream(SfxPlayer *player) : _player(player) {}

	bool isStereo() const override {
		return true;
	}

	virtual int getRate() const override { return _player->_rate; }
	virtual bool endOfData() const override { return false; }

	int readBuffer(int16 *buffer, const int numSamples) override {
		assert(_player != nullptr);
		memset(buffer, 0, numSamples * sizeof(int16));
		_player->readSamples(buffer, numSamples);

		return numSamples;
	}
};

class Sound {
private:
	Audio::Mixer *_mixer;
	Audio::SoundHandle _musicHandle;
	SfxPlayer *_sfx = nullptr;
	SfxMusicStream *_sfxStream = nullptr;
	Audio::SoundHandle _channels[MAX_CHANNELS];

public:
	explicit Sound(Audio::Mixer *mixer) : _mixer(mixer) {
	}

	void setPlayer(SfxPlayer *player);

	void stopAll();
	void playMusic(const char *path, int loops);
	void playAifcMusic(const char *path, uint32 offset);
	void playSfxMusic(int num);
	void stopMusic();
	void stopAifcMusic();
	void stopSfxMusic();

	void preloadSoundAiff(byte num, const byte *data);
	void playSoundRaw(byte channel, const byte *data, size_t size,
		int freq, byte volume);
	void playSoundWav(byte channel, const byte *data, size_t size,
		uint16 freq, byte volume, byte loop);
	void playSoundAiff(byte channel, byte num, byte volume);
	void stopSound(byte channel);
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

	void playSoundRaw(byte channel, const byte *data, uint16 freq, byte volume);
	void playSoundWav(byte channel, const byte *data, uint16 freq, byte volume, byte loop);
	void stopSound(byte channel);
	void setChannelVolume(byte channel, byte volume);
	void playMusic(const char *path, byte loop);
	void stopMusic();
	void playAifcMusic(const char *path, uint32 offset);
	void stopAifcMusic();
	void playSfxMusic(int num);
	void stopSfxMusic();
	void stopAll();
	void preloadSoundAiff(byte num, const byte *data);
	void playSoundAiff(byte channel, byte num, byte volume);
};
#endif

} // namespace Awe

#endif
