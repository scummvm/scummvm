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

#ifndef NEVERHOOD_SOUND_H
#define NEVERHOOD_SOUND_H

#include "audio/audiostream.h"
#include "common/array.h"
#include "neverhood/resourceman.h"

namespace Common {
class SeekableReadStream;
}

namespace Audio {
class SoundHandle;
}

namespace Neverhood {

class NeverhoodEngine;
class AudioResourceManSoundItem;
class AudioResourceManMusicItem;

class SoundResource {
public:
	SoundResource(NeverhoodEngine *vm);
	~SoundResource();
	bool isPlaying();
	void load(uint32 fileHash);
	void unload();
	void play(uint32 fileHash);
	void play();
	void playLooping();
	void stop();
	void setVolume(int16 volume);
	void setPan(int16 pan);
protected:
	NeverhoodEngine *_vm;
	int16 _soundIndex;
	AudioResourceManSoundItem *getSoundItem();
};

class MusicResource {
public:
	MusicResource(NeverhoodEngine *vm);
	bool isPlaying();
	void load(uint32 fileHash);
	void unload();
	void play(int16 fadeVolumeStep);
	void stop(int16 fadeVolumeStep);
	void setVolume(int16 volume);
protected:
	NeverhoodEngine *_vm;
	int16 _musicIndex;
	AudioResourceManMusicItem *getMusicItem();
};

class MusicItem {
public:
	MusicItem(NeverhoodEngine *vm, uint32 groupNameHash, uint32 musicFileHash);
	~MusicItem();
	void startMusic(int16 countdown, int16 fadeVolumeStep);
	void stopMusic(int16 countdown, int16 fadeVolumeStep);
	void update();
	uint32 getGroupNameHash() const { return _groupNameHash; }
	uint32 getFileHash() const { return _fileHash; }
protected:
	NeverhoodEngine *_vm;
	uint32 _groupNameHash;
	uint32 _fileHash;
	bool _play;
	bool _stop;
	int16 _fadeVolumeStep;
	int16 _countdown;
	MusicResource *_musicResource;
};

class SoundItem {
public:
	SoundItem(NeverhoodEngine *vm, uint32 groupNameHash, uint32 soundFileHash,
		bool playOnceAfterRandomCountdown, int16 minCountdown, int16 maxCountdown,
		bool playOnceAfterCountdown, int16 initialCountdown, bool playLooping, int16 currCountdown);
	~SoundItem();
	void setSoundParams(bool playOnceAfterRandomCountdown, int16 minCountdown, int16 maxCountdown,
		int16 firstMinCountdown, int16 firstMaxCountdown);
	void playSoundLooping();
	void stopSound();
	void setVolume(int volume);
	void update();
	void setPlayOnceAfterCountdown(bool playOnceAfterCountdown) { _playOnceAfterCountdown = playOnceAfterCountdown; }
	uint32 getGroupNameHash() const { return _groupNameHash; }
	uint32 getFileHash() const { return _fileHash; }
	int16 getCurrCountdown() const { return _currCountdown; }
protected:
	NeverhoodEngine *_vm;
	uint32 _groupNameHash;
	uint32 _fileHash;
	bool _playOnceAfterRandomCountdown;
	int16 _minCountdown;
	int16 _maxCountdown;
	bool _playOnceAfterCountdown;
	int16 _initialCountdown;
	bool _playLooping;
	int16 _currCountdown;
	SoundResource *_soundResource;
};

class SoundMan {
public:
	SoundMan(NeverhoodEngine *vm);
	~SoundMan();

	void stopAllMusic();
	void stopAllSounds();

	// Music
	void addMusic(uint32 groupNameHash, uint32 musicFileHash);
	void deleteMusic(uint32 musicFileHash);
	void startMusic(uint32 musicFileHash, int16 countdown, int16 fadeVolumeStep);
	void stopMusic(uint32 musicFileHash, int16 countdown, int16 fadeVolumeStep);

	// Sound
	void addSound(uint32 groupNameHash, uint32 soundFileHash);
	void addSoundList(uint32 groupNameHash, const uint32 *soundFileHashList);
	void deleteSound(uint32 soundFileHash);
	void setSoundParams(uint32 soundFileHash, bool playOnceAfterRandomCountdown,
		int16 minCountdown, int16 maxCountdown, int16 firstMinCountdown, int16 firstMaxCountdown);
	void setSoundListParams(const uint32 *soundFileHashList, bool playOnceAfterRandomCountdown,
		int16 minCountdown, int16 maxCountdown, int16 firstMinCountdown, int16 firstMaxCountdown);
	void playSoundLooping(uint32 soundFileHash);
	void stopSound(uint32 soundFileHash);
	void setSoundVolume(uint32 soundFileHash, int volume);

	// Misc
	void update();
	void deleteGroup(uint32 groupNameHash);
	void deleteMusicGroup(uint32 groupNameHash);
	void deleteSoundGroup(uint32 groupNameHash);
	void playTwoSounds(uint32 groupNameHash, uint32 soundFileHash1, uint32 soundFileHash2, int16 initialCountdown);
	void playSoundThree(uint32 groupNameHash, uint32 soundFileHash);
	void setTwoSoundsPlayFlag(bool playOnceAfterCountdown);
	void setSoundThreePlayFlag(bool playOnceAfterCountdown);

protected:
	NeverhoodEngine *_vm;

	// TODO Find out what these special sounds are used for (door sounds?)
	int _soundIndex1, _soundIndex2;
	int16 _initialCountdown;
	bool _playOnceAfterCountdown;

	int _soundIndex3;
	int16 _initialCountdown3;
	bool _playOnceAfterCountdown3;

	Common::Array<MusicItem*> _musicItems;
	Common::Array<SoundItem*> _soundItems;

	MusicItem *getMusicItemByHash(uint32 musicFileHash);
	SoundItem *getSoundItemByHash(uint32 soundFileHash);
	int16 addMusicItem(MusicItem *musicItem);
	int16 addSoundItem(SoundItem *soundItem);
	void deleteSoundByIndex(int index);

};

class NeverhoodAudioStream : public Audio::AudioStream {
public:
	NeverhoodAudioStream(int rate, byte shiftValue, bool isLooping, DisposeAfterUse::Flag disposeStream, Common::SeekableReadStream *stream);
	~NeverhoodAudioStream() override;
	int readBuffer(int16 *buffer, const int numSamples) override;
	bool isStereo() const override  { return _isStereo; }
	bool endOfData() const override { return _endOfData; }
	int getRate() const override { return _rate; }
private:
	const int _rate;
	const bool _isLooping;
	const bool _isStereo;
	const byte _shiftValue;
	const bool _isCompressed;
	int16 _prevValue;
	Common::DisposablePtr<Common::SeekableReadStream> _stream;
	bool _endOfData;
	byte *_buffer;
	enum {
		kSampleBufferLength = 2048
	};
	int fillBuffer(int maxSamples);
};

// TODO Rename these

class AudioResourceManSoundItem {
public:
	AudioResourceManSoundItem(NeverhoodEngine *vm, uint32 fileHash);
	~AudioResourceManSoundItem();
	void loadSound();
	void unloadSound();
	void setVolume(int16 volume);
	void setPan(int16 pan);
	void playSound(bool looping);
	void stopSound();
	bool isPlaying();
protected:
	NeverhoodEngine *_vm;
	uint32 _fileHash;
	ResourceHandle _resourceHandle;
	const byte *_data;
	bool _isLoaded;
	bool _isPlaying;
	int16 _volume;
	int16 _panning;
	Audio::SoundHandle *_soundHandle;
};

class AudioResourceManMusicItem {
public:
	AudioResourceManMusicItem(NeverhoodEngine *vm, uint32 fileHash);
	~AudioResourceManMusicItem();
	void playMusic(int16 fadeVolumeStep);
	void stopMusic(int16 fadeVolumeStep);
	void unloadMusic();
	void setVolume(int16 volume);
	void restart();
	void update();
	bool isPlaying() const { return _isPlaying; }
	bool canRestart() const { return _canRestart; }
	bool isTerminated() const { return _terminate; }
	uint32 getFileHash() const { return _fileHash; }
protected:
	NeverhoodEngine *_vm;
	uint32 _fileHash;
	bool _isPlaying;
	bool _canRestart;
	bool _terminate;
	int16 _volume;
	int16 _panning;
	bool _start;
	bool _isFadingIn;
	bool _isFadingOut;
	int16 _fadeVolume;
	int16 _fadeVolumeStep;
	Audio::SoundHandle *_soundHandle;
};

class AudioResourceMan {
public:
	AudioResourceMan(NeverhoodEngine *vm);
	~AudioResourceMan();

	void stopAllMusic();
	void stopAllSounds();

	int16 addSound(uint32 fileHash);
	void removeSound(int16 soundIndex);

	int16 loadMusic(uint32 fileHash);
	void updateMusic();

	AudioResourceManSoundItem *getSoundItem(int16 index);
	AudioResourceManMusicItem *getMusicItem(int16 index);

protected:
	NeverhoodEngine *_vm;

	Common::Array<AudioResourceManMusicItem*> _musicItems;
	Common::Array<AudioResourceManSoundItem*> _soundItems;

	int16 addSoundItem(AudioResourceManSoundItem *soundItem);

};

} // End of namespace Neverhood

#endif /* NEVERHOOD_SOUND_H */
