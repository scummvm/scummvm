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
#include "audio/midiplayer.h"
#include "audio/mixer.h"
#include "audio/decoders/wave.h"
#include "common/list.h"

namespace Illusions {

class IllusionsEngine;

class MusicPlayer {
public:
	MusicPlayer();
	~MusicPlayer();
	void play(uint32 musicId, bool looping, int16 volume, int16 pan);
	void stop();
	bool isPlaying();
protected:
	Audio::SoundHandle _soundHandle;
	uint32 _musicId;
	uint _flags;
};

class MidiPlayer : public Audio::MidiPlayer {
public:
	MidiPlayer();

	void pause(bool p);
	void play(const Common::String &filename);
	void fade(int16 finalVolume, int16 duration);

	// The following line prevents compiler warnings about hiding the pause()
	// method from the parent class.
	// FIXME: Maybe the pause(bool p) method should be removed and the
	// pause/resume methods of the parent class be used instead?
	virtual void pause() { Audio::MidiPlayer::pause(); }

	// Overload Audio::MidiPlayer method
	virtual void sendToChannel(byte channel, uint32 b);
	virtual void onTimer();

private:
	bool _paused;
};

class VoicePlayer {
public:
	VoicePlayer();
	~VoicePlayer();
	bool cue(const char *voiceName);
	void stopCueing();
	void start(int16 volume, int16 pan);
	void stop();
	bool isPlaying();
	bool isEnabled();
	bool isCued();
protected:
	Audio::SoundHandle _soundHandle;
	Common::String _voiceName;
	uint _voiceStatus;
};

class Sound {
public:
	Sound(uint32 soundEffectId, uint32 soundGroupId, bool looping);
	~Sound();
	void load();
	void unload();
	void play(int16 volume, int16 pan);
	void stop();
	bool isPlaying();
public:
	uint32 _soundEffectId;
	uint32 _soundGroupId;
protected:
	Audio::RewindableAudioStream *_stream;
	Audio::SoundHandle _soundHandle;
	bool _looping;
};

class SoundMan {
public:
	SoundMan(IllusionsEngine *vm);
	~SoundMan();
	void update();

	void playMusic(uint32 musicId, int16 type, int16 volume, int16 pan, uint32 notifyThreadId);
	void stopMusic();

	void playMidiMusic(uint32 musicId);
	void stopMidiMusic();
	void fadeMidiMusic(int16 finalVolume, int16 duration);

	bool cueVoice(const char *voiceName);
	void stopCueingVoice();
	void startVoice(int16 volume, int16 pan);
	void stopVoice();
	bool isVoicePlaying();
	bool isVoiceEnabled();
	bool isVoiceCued();

	void loadSound(uint32 soundEffectId, uint32 soundGroupId, bool looping);
	void playSound(uint32 soundEffectId, int16 volume, int16 pan);
	void stopSound(uint32 soundEffectId);
	void unloadSounds(uint32 soundGroupId);

protected:
	typedef Common::List<Sound*> SoundList;
	typedef SoundList::iterator SoundListIterator;
	IllusionsEngine *_vm;
	uint32 _musicNotifyThreadId;
	MusicPlayer *_musicPlayer;
	MidiPlayer *_midiPlayer;
	VoicePlayer *_voicePlayer;
	SoundList _sounds;
	Sound *getSound(uint32 soundEffectId);
};

} // End of namespace Illusions

#endif // ILLUSIONS_SOUND_H
