/* ScummVM - Graphic Adventure Engine
 *
 * ScummVM is the legal property of its developers, whose names
 * are too numerous to list here. Please refer to the COPYRIGHT
 * file distributed with this source distribution.
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License
 * as published by the Free Software Foundation; either version 2
 * of the License, or (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.
 *
 */

#ifndef CHEWY_SOUND_H
#define CHEWY_SOUND_H

#include "audio/mixer.h"
#include "chewy/chewy.h"

namespace Chewy {

class SoundResource;

#define MAX_SOUND_EFFECTS 14

class Sound {
public:
	Sound(Audio::Mixer *mixer);
	virtual ~Sound();

	void playSound(int num, bool loop = false, uint channel = 0);
	void playSound(byte *data, uint32 size, bool loop = false, uint channel = 0, DisposeAfterUse::Flag dispose = DisposeAfterUse::YES);
	void pauseSound(uint channel);
	void resumeSound(uint channel);
	void stopSound(uint channel);
	bool isSoundActive(uint channel);
	void setSoundVolume(uint volume);
	void setSoundChannelVolume(uint channel, uint volume);
	void setSoundChannelBalance(uint channel, uint balance);

	void playMusic(int num, bool loop = false);
	void playMusic(byte *data, uint32 size, bool loop = false, DisposeAfterUse::Flag dispose = DisposeAfterUse::YES);
	void pauseMusic();
	void resumeMusic();
	void stopMusic();
	bool isMusicActive();
	void setMusicVolume(uint volume);

	void playSpeech(int num);
	void pauseSpeech();
	void resumeSpeech();
	void stopSpeech();
	bool isSpeechActive();
	void setSpeechVolume(uint volume);

	void stopAll();

private:
	Audio::Mixer *_mixer;
	Audio::SoundHandle _soundHandle[MAX_SOUND_EFFECTS];
	Audio::SoundHandle _musicHandle;
	Audio::SoundHandle _speechHandle;

	SoundResource *_speechRes;
	SoundResource *_soundRes;

	void convertTMFToMod(byte *tmfData, uint32 tmfSize, byte *modData, uint32 &modSize);
};

} // End of namespace Chewy

#endif
