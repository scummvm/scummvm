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

#ifndef HOPKINS_SOUND_H
#define HOPKINS_SOUND_H

#include "common/scummsys.h"
#include "common/str.h"
#include "audio/mixer.h"

namespace Audio {
class RewindableAudioStream;
}

namespace Common {
class SeekableReadStream;
}

namespace Hopkins {

class VoiceItem {
public:
	VoiceItem() : _status(false), _wavIndex(0) {}

	bool _status;
	int _wavIndex;
};

class SwavItem {
public:
	SwavItem() : _active(false), _audioStream(NULL), _freeSampleFl(false) {}

	bool _active;
	Audio::RewindableAudioStream *_audioStream;
	Audio::SoundHandle _soundHandle;
	bool _freeSampleFl;
};

class MusicItem {
public:
	MusicItem() : _active(false) {}

	bool _active;
};

class SoundItem {
public:
	SoundItem() : _active(false) {}

	bool _active;
};

#define VOICE_COUNT 3
#define SWAV_COUNT 50
#define SOUND_COUNT 10

class HopkinsEngine;

class SoundManager {
private:
	HopkinsEngine *_vm;

	Audio::SoundHandle _musicHandle;
	int _currentSoundIndex;
	bool _modPlayingFl;
	int _oldSoundNumber;

	VoiceItem _voice[VOICE_COUNT];
	SwavItem _sWav[SWAV_COUNT];
	SoundItem _sound[SOUND_COUNT];
	MusicItem _music;

	void playMod(const Common::String &file);
	void loadMusic(const Common::String &file);
	void playMusic();
	void stopMusic();
	void delMusic();
	bool checkVoiceStatus(int voiceIndex);
	bool loadVoice(const Common::String &filename, size_t fileOffset, size_t entryLength, SwavItem &item);
	void stopVoice(int voiceIndex);
	void playVoice();
	void delWav(int wavIndex);
	void checkVoiceActivity();
	Common::String setExtension(const Common::String &str, const Common::String &ext);
	Audio::RewindableAudioStream *makeSoundStream(Common::SeekableReadStream *stream);
	bool removeWavSample(int wavIndex);
	void loadWavSample(int wavIndex, const Common::String &filename, bool freeSample);
	void playWavSample(int voiceIndex, int wavIndex);

public:
	bool _musicOffFl;
	bool _soundOffFl;
	bool _voiceOffFl;
	bool _textOffFl;
	bool _soundFl;
	bool _skipRefreshFl;
	int _musicVolume;
	int _soundVolume;
	int _voiceVolume;
	int _specialSoundNum;
public:
	SoundManager(HopkinsEngine *vm);
	~SoundManager();

	void loadAnimSound();
	void playAnimSound(int animFrame);

	void loadSample(int wavIndex, const Common::String &file);
	void playSample(int wavIndex, int voiceMode = 9);
	void removeSample(int soundIndex);

	void checkSoundEnd();
	void checkSounds();
	void playSoundFile(const Common::String &file);
	void playSound(int soundNumber);
	void stopSound();

	void updateScummVMSoundSettings();
	void syncSoundSettings();
	bool mixVoice(int voiceId, int voiceMode, bool displTxtFl = false);

	void setMODMusicVolume(int volume);
	void setMODSampleVolume();
	void setMODVoiceVolume();

	void loadWav(const Common::String &file, int wavIndex);
	void playWav(int wavIndex);
	void directPlayWav(const Common::String &file2);
};

} // End of namespace Hopkins

#endif /* HOPKINS_SOUND_H */
