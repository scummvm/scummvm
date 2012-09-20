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

#include "common/array.h"
#include "graphics/surface.h"
#include "neverhood/neverhood.h"
#include "neverhood/resource.h"

namespace Neverhood {

class MusicResource {
public:
	MusicResource(NeverhoodEngine *vm);
	bool isPlaying();
	void load(uint32 fileHash);
	void unload();
	void play(int16 fadeVolumeStep);
	void stop(int16 fadeVolumeStep);
protected:
	NeverhoodEngine *_vm;
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
	void addSoundList(uint32 nameHash, uint32 *soundFileHashList);
	void deleteSound(uint32 soundFileHash);
	void setSoundParams(uint32 soundFileHash, bool playOnceAfterRandomCountdown,
		int16 minCountdown, int16 maxCountdown, int16 firstMinCountdown, int16 firstMaxCountdown);
	void setSoundListParams(uint32 *soundFileHashList, bool playOnceAfterRandomCountdown,
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
	void deleteSoundByIndex(int index);
	
};

} // End of namespace Neverhood

#endif /* NEVERHOOD_SOUND_H */
