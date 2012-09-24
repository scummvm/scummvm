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

#ifndef NEVERHOOD_SOUND_H
#define NEVERHOOD_SOUND_H

#include "audio/audiostream.h"
#include "common/array.h"
#include "graphics/surface.h"
#include "neverhood/neverhood.h"
#include "neverhood/resource.h"

namespace Neverhood {

// Convert volume from percent to 0..255
#define VOLUME(volume) (Audio::Mixer::kMaxChannelVolume / 100 * (volume))

// Convert panning from percent (50% equals center) to -127..0..+127
#define PANNING(panning) (254 / 100 * (panning) - 127)

class SoundResource {
public:
	SoundResource(NeverhoodEngine *vm);
	~SoundResource();
	bool isPlaying();
	void load(uint32 fileHash);
	void unload();
	void play(uint32 fileHash);
	void play();
	void stop();
	void setVolume(int16 volume);
	void setPan(int16 pan);
protected:
	NeverhoodEngine *_vm;
	int16 _soundIndex;	
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
};

struct MusicItem {
	uint32 _nameHash;
	uint32 _musicFileHash;
	bool _play;
	bool _stop;
	int16 _fadeVolumeStep;
	int16 _countdown;
	MusicResource *_musicResource;
	MusicItem();
	~MusicItem();
};

struct SoundItem {
	uint32 _nameHash;
	uint32 _soundFileHash;
	bool _playOnceAfterRandomCountdown;
	int16 _minCountdown;
	int16 _maxCountdown;
	bool _playOnceAfterCountdown;
	int16 _initialCountdown;
	bool _playLooping;
	int16 _currCountdown;
	SoundResource *_soundResource;

	SoundItem(NeverhoodEngine *vm, uint32 nameHash, uint32 soundFileHash,
		bool playOnceAfterRandomCountdown, int16 minCountdown, int16 maxCountdown,
		bool playOnceAfterCountdown, int16 initialCountdown, bool playLooping, int16 currCountdown);
	~SoundItem();
};

// TODO Give this a better name

class SoundMan {
public:
	SoundMan(NeverhoodEngine *vm);
	~SoundMan();

	// Music
	void addMusic(uint32 nameHash, uint32 musicFileHash);
	void deleteMusic(uint32 musicFileHash);
	void startMusic(uint32 musicFileHash, int16 countdown, int16 fadeVolumeStep);
	void stopMusic(uint32 musicFileHash, int16 countdown, int16 fadeVolumeStep);
	
	// Sound
	void addSound(uint32 nameHash, uint32 soundFileHash);
	void addSoundList(uint32 nameHash, const uint32 *soundFileHashList);
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
	void deleteGroup(uint32 nameHash);
	void deleteMusicGroup(uint32 nameHash);
	void deleteSoundGroup(uint32 nameHash);
	void playTwoSounds(uint32 nameHash, uint32 soundFileHash1, uint32 soundFileHash2, int16 initialCountdown);
	void playSoundThree(uint32 nameHash, uint32 soundFileHash);
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
	~NeverhoodAudioStream();
	int readBuffer(int16 *buffer, const int numSamples);
	bool isStereo() const  { return _isStereo; }
	bool endOfData() const { return _endOfData; }
	int getRate() const { return _rate; }
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

struct AudioResourceManSoundItem {
	uint32 _fileHash;
	int _resourceHandle;
	byte *_data;
	bool _isLoaded;
	bool _isPlaying;
	int16 _volume;
	int16 _panning;
	Audio::SoundHandle _soundHandle;
};

struct AudioResourceManMusicItem {
	uint32 _fileHash;
	// streamIndex dw
	// needCreate db
	bool _isPlaying;
	bool _remove;
	int16 _volume;
	int16 _panning;
	bool _start;
	bool _isFadingIn;
	bool _isFadingOut;
	int16 _fadeVolume;
	int16 _fadeVolumeStep;
	Audio::SoundHandle _soundHandle;
	// status dw
	// updateCounter dd
};

class AudioResourceMan {
public:
	AudioResourceMan(NeverhoodEngine *vm);
	~AudioResourceMan();
	
	int16 addSound(uint32 fileHash);
	void removeSound(int16 soundIndex);
	void loadSound(int16 soundIndex);
	void unloadSound(int16 soundIndex);
	void setSoundVolume(int16 soundIndex, int16 volume);
	void setSoundPan(int16 soundIndex, int16 pan);
	void playSound(int16 soundIndex, bool looping);
	void stopSound(int16 soundIndex);
	bool isSoundPlaying(int16 soundIndex);
	
	int16 loadMusic(uint32 fileHash);
	void unloadMusic(int16 musicIndex);
	void setMusicVolume(int16 musicIndex, int16 volume);
	void playMusic(int16 musicIndex, int16 fadeVolumeStep);
	void stopMusic(int16 musicIndex, int16 fadeVolumeStep);
	bool isMusicPlaying(int16 musicIndex);
	void updateMusicItem(int16 musicIndex);
	
	void update();
	
protected:
	NeverhoodEngine *_vm;

	Common::Array<AudioResourceManMusicItem*> _musicItems;
	Common::Array<AudioResourceManSoundItem*> _soundItems;

	int16 addSoundItem(AudioResourceManSoundItem *soundItem);

};

} // End of namespace Neverhood

#endif /* NEVERHOOD_SOUND_H */
