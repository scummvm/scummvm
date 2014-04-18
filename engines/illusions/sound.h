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

 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.

 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.
 *
 */

#ifndef ILLUSIONS_SOUND_H
#define ILLUSIONS_SOUND_H

#include "illusions/graphics.h"
#include "audio/audiostream.h"
#include "audio/mixer.h"
#include "audio/decoders/wave.h"
#include "common/list.h"

namespace Illusions {

class IllusionsEngine;

class MusicPlayer {
public:
	MusicPlayer(Audio::Mixer *mixer);
	~MusicPlayer();
	void play(uint32 musicId, bool looping, int16 volume, int16 pan);
	void stop();
	bool isPlaying();
protected:
	Audio::Mixer *_mixer;
	Audio::SoundHandle _soundHandle;
	uint32 _musicId;
	uint _flags;
};

class VoicePlayer {
public:
	VoicePlayer(Audio::Mixer *mixer);
	~VoicePlayer();
	bool cue(const char *voiceName);
	void stopCueing();
	void start(int16 volume, int16 pan);
	void stop();
	bool isPlaying();
	bool isEnabled();
	bool isCued();
protected:
	Audio::Mixer *_mixer;
	Audio::SoundHandle _soundHandle;
	Common::String _voiceName;
	uint _voiceStatus;
};

class SoundMan {
public:
	SoundMan(IllusionsEngine *vm);
	~SoundMan();
	void update();

	void playMusic(uint32 musicId, int16 type, int16 volume, int16 pan, uint32 notifyThreadId);
	void stopMusic();

	bool cueVoice(const char *voiceName);
	void stopCueingVoice();
	void startVoice(int16 volume, int16 pan);
	void stopVoice();
	bool isVoicePlaying();
	bool isVoiceEnabled();
	bool isVoiceCued();

protected:
	IllusionsEngine *_vm;
	uint32 _musicNotifyThreadId;
	MusicPlayer *_musicPlayer;
	VoicePlayer *_voicePlayer;
};

} // End of namespace Illusions

#endif // ILLUSIONS_SOUND_H
