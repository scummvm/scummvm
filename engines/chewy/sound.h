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

#ifndef CHEWY_SOUND_H
#define CHEWY_SOUND_H

#include "audio/mixer.h"
#include "chewy/atds.h"
#include "chewy/chewy.h"

namespace Chewy {

class SoundResource;

#define MAX_SOUND_EFFECTS 14

class Sound {
private:
	static const int TMF_NUM_INSTRUMENTS = 31;
	static const uint8 TMF_MOD_SONG_NAME[20];
	static const uint8 TMF_MOD_INSTRUMENT_NAME[22];
	static const uint16 TMF_MOD_PERIODS[36];

public:
	Sound(Audio::Mixer *mixer);
	virtual ~Sound();

	void playSound(int num, uint channel = 0, bool loop = false);
	void playSound(uint8 *data, uint32 size, uint channel = 0, bool loop = false, DisposeAfterUse::Flag dispose = DisposeAfterUse::YES);
	void pauseSound(uint channel);
	void resumeSound(uint channel);
	void stopSound(uint channel = 0);
	void stopAllSounds();
	bool isSoundActive(uint channel);
	void setSoundVolume(uint volume);
	void setSoundChannelVolume(uint channel, uint volume);
	void setSoundChannelBalance(uint channel, int8 balance);

	void playMusic(int num, bool loop = false);
	void playMusic(uint8 *data, uint32 size, bool loop = false, DisposeAfterUse::Flag dispose = DisposeAfterUse::YES);
	void pauseMusic();
	void resumeMusic();
	void stopMusic();
	bool isMusicActive();
	void setMusicVolume(uint volume);

	void playSpeech(int num, bool waitForFinish);
	void pauseSpeech();
	void resumeSpeech();
	void stopSpeech();
	bool isSpeechActive();
	void setSpeechVolume(uint volume);

	void stopAll();

	/**
	 * Helper method to wait until any playing speech is finished
	 */
	void waitForSpeechToFinish();

	bool soundEnabled() const;
	void toggleSound(bool enable);

	bool musicEnabled() const;
	void toggleMusic(bool enable);

	bool speechEnabled() const;
	void toggleSpeech(bool enable);

	bool subtitlesEnabled() const;
	void toggleSubtitles(bool enable);

private:
	Audio::Mixer *_mixer;
	Audio::SoundHandle _soundHandle[MAX_SOUND_EFFECTS];
	Audio::SoundHandle _musicHandle;
	Audio::SoundHandle _speechHandle;

	SoundResource *_speechRes;
	SoundResource *_soundRes;
};

} // End of namespace Chewy

#endif
