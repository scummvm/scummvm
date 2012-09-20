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

#include "graphics/palette.h"
#include "neverhood/sound.h"

namespace Neverhood {

// TODO Put more stuff into the constructors/destructors of the item structs
// TODO Some parts are quite bad here, but my priority is to get sound working at all

MusicResource::MusicResource(NeverhoodEngine *vm)
	: _vm(vm) {
}

bool MusicResource::isPlaying() { 
	return false; 
}

void MusicResource::load(uint32 fileHash) {
	// TODO
}

void MusicResource::unload() {
	// TODO
}

void MusicResource::play(int16 fadeVolumeStep) {
	// TODO
}

void MusicResource::stop(int16 fadeVolumeStep) {
	// TODO
}

SoundMan::SoundMan(NeverhoodEngine *vm)
	: _vm(vm),
	_soundIndex1(-1), _soundIndex2(-1), _soundIndex3(-1) {
}

SoundMan::~SoundMan() {
	// TODO Clean up
}

void SoundMan::addMusic(uint32 nameHash, uint32 musicFileHash) {
	MusicItem *musicItem = new MusicItem();
	musicItem->_nameHash = nameHash;
	musicItem->_musicFileHash = musicFileHash;
	musicItem->_play = false;
	musicItem->_stop = false;
	musicItem->_fadeVolumeStep = 0;
	musicItem->_countdown = 24;
	musicItem->_musicResource = new MusicResource(_vm);
	musicItem->_musicResource->load(musicFileHash);
	// TODO Is this needed? musicItem->_musicResource->init();
	_musicItems.push_back(musicItem);
}

void SoundMan::deleteMusic(uint32 musicFileHash) {
	for (uint i = 0; i < _musicItems.size(); ++i) {
		MusicItem *musicItem = _musicItems[i];
		if (musicItem->_musicFileHash == musicFileHash) {
			musicItem->_musicResource->unload();
			delete musicItem->_musicResource;
			delete musicItem;
			_musicItems.remove_at(i);
			break;
		}
	}
}

void SoundMan::startMusic(uint32 musicFileHash, int16 countdown, int16 fadeVolumeStep) {
	for (uint i = 0; i < _musicItems.size(); ++i) {
		MusicItem *musicItem = _musicItems[i];
		if (musicItem->_musicFileHash == musicFileHash) {
			musicItem->_play = true;
			musicItem->_stop = false;
			musicItem->_countdown = countdown;
			musicItem->_fadeVolumeStep = fadeVolumeStep;
			break;
		}
	}
}

void SoundMan::stopMusic(uint32 musicFileHash, int16 countdown, int16 fadeVolumeStep) {
	for (uint i = 0; i < _musicItems.size(); ++i) {
		MusicItem *musicItem = _musicItems[i];
		if (musicItem->_musicFileHash == musicFileHash) {
			musicItem->_play = false;
			musicItem->_stop = true;
			musicItem->_countdown = countdown;
			musicItem->_fadeVolumeStep = fadeVolumeStep;
			break;
		}
	}
}

void SoundMan::addSound(uint32 nameHash, uint32 soundFileHash) {
	SoundItem *soundItem = new SoundItem();
	soundItem->_nameHash = nameHash;
	soundItem->_soundFileHash = soundFileHash;
	soundItem->_playOnceAfterRandomCountdown = false;
	soundItem->_minCountdown = 50;
	soundItem->_maxCountdown = 600;
	soundItem->_playOnceAfterCountdown = false;
	soundItem->_initialCountdown = 0;
	soundItem->_playLooping = false;
	soundItem->_currCountdown = 0;
	soundItem->_soundResource = new SoundResource(_vm);
	soundItem->_soundResource->load(soundFileHash);
	_soundItems.push_back(soundItem);
}

void SoundMan::addSoundList(uint32 nameHash, uint32 *soundFileHashList) {
	while (*soundFileHashList)
		addSound(nameHash, *soundFileHashList++);
}

void SoundMan::deleteSound(uint32 soundFileHash) {
	for (uint i = 0; i < _soundItems.size(); ++i) {
		SoundItem *soundItem = _soundItems[i];
		if (soundItem->_soundFileHash == soundFileHash) {
			soundItem->_soundResource->unload();
			delete soundItem->_soundResource;
			delete soundItem;
			_soundItems.remove_at(i);
			break;
		}
	}
}

void SoundMan::setSoundParams(uint32 soundFileHash, bool playOnceAfterRandomCountdown,
	int16 minCountdown, int16 maxCountdown, int16 firstMinCountdown, int16 firstMaxCountdown) {

	SoundItem *soundItem = getSoundItemByHash(soundFileHash);
	if (soundItem) {
		soundItem->_playOnceAfterCountdown = false;
		soundItem->_playLooping = false;
		soundItem->_playOnceAfterRandomCountdown = playOnceAfterRandomCountdown;
		if (minCountdown > 0)
			soundItem->_minCountdown = minCountdown;
		if (maxCountdown > 0)
			soundItem->_maxCountdown = maxCountdown;
		if (firstMinCountdown >= firstMaxCountdown)
			soundItem->_currCountdown = firstMinCountdown;
		else if (firstMinCountdown > 0 && firstMaxCountdown > 0 && firstMinCountdown < firstMaxCountdown)
			soundItem->_currCountdown = _vm->_rnd->getRandomNumberRng(firstMinCountdown, firstMaxCountdown);
	}
}

void SoundMan::setSoundListParams(uint32 *soundFileHashList, bool playOnceAfterRandomCountdown,
	int16 minCountdown, int16 maxCountdown, int16 firstMinCountdown, int16 firstMaxCountdown) {

	while (*soundFileHashList)
		setSoundParams(*soundFileHashList++, playOnceAfterRandomCountdown,
			minCountdown, maxCountdown, firstMinCountdown, firstMaxCountdown);
}

void SoundMan::playSoundLooping(uint32 soundFileHash) {
	SoundItem *soundItem = getSoundItemByHash(soundFileHash);
	if (soundItem) {
		soundItem->_playOnceAfterRandomCountdown = false;
		soundItem->_playOnceAfterCountdown = false;
		soundItem->_playLooping = true;
	}
}

void SoundMan::stopSound(uint32 soundFileHash) {
	SoundItem *soundItem = getSoundItemByHash(soundFileHash);
	if (soundItem) {
		soundItem->_playOnceAfterRandomCountdown = false;
		soundItem->_playOnceAfterCountdown = false;
		soundItem->_playLooping = false;
		soundItem->_soundResource->stop();
	}
}

void SoundMan::setSoundVolume(uint32 soundFileHash, int volume) {
	SoundItem *soundItem = getSoundItemByHash(soundFileHash);
	if (soundItem)
		soundItem->_soundResource->setVolume(volume);
}

void SoundMan::update() {
	// TODO Check if active
	
	for (uint i = 0; i < _soundItems.size(); ++i) {
		SoundItem *soundItem = _soundItems[i];
		if (soundItem->_playOnceAfterCountdown) {
			if (soundItem->_currCountdown == 0) {
				soundItem->_currCountdown = soundItem->_initialCountdown;
			} else if (--soundItem->_currCountdown == 0) {
				soundItem->_soundResource->play();
			}
		} else if (soundItem->_playOnceAfterRandomCountdown) {
			if (soundItem->_currCountdown == 0) {
				if (soundItem->_minCountdown > 0 && soundItem->_maxCountdown > 0 && soundItem->_minCountdown < soundItem->_maxCountdown)
					soundItem->_currCountdown = _vm->_rnd->getRandomNumberRng(soundItem->_minCountdown, soundItem->_maxCountdown);
			} else if (--soundItem->_currCountdown == 0) {
				soundItem->_soundResource->play();
			}
		} else if (soundItem->_playLooping && !soundItem->_soundResource->isPlaying()) {
			soundItem->_soundResource->play(); // TODO Looping parameter?
		}
	}

	for (uint i = 0; i < _musicItems.size(); ++i) {
		MusicItem *musicItem = _musicItems[i];
		if (musicItem->_countdown) {
			--musicItem->_countdown;
		} else if (musicItem->_play && !musicItem->_musicResource->isPlaying()) {
			musicItem->_musicResource->play(musicItem->_fadeVolumeStep);
			musicItem->_fadeVolumeStep = 0;
		} else if (musicItem->_stop) {
			musicItem->_musicResource->stop(musicItem->_fadeVolumeStep);
			musicItem->_fadeVolumeStep = 0;
			musicItem->_stop = false;
		}
	}

}

void SoundMan::deleteGroup(uint32 nameHash) {
	deleteMusicGroup(nameHash);
	deleteSoundGroup(nameHash);
}

void SoundMan::deleteMusicGroup(uint32 nameHash) {
	for (int index = _musicItems.size() - 1; index >= 0; --index) {
		MusicItem *musicItem = _musicItems[index];
		if (musicItem->_nameHash == nameHash) {
			musicItem->_musicResource->unload();
			delete musicItem->_musicResource;
			delete musicItem;
			_musicItems.remove_at(index);
		}
	}
}

void SoundMan::deleteSoundGroup(uint32 nameHash) {

	SoundItem *soundItem;

	if (_soundIndex1 != -1 && _soundItems[_soundIndex1]->_nameHash == nameHash) {
		soundItem = _soundItems[_soundIndex1];
		soundItem->_soundResource->unload();
		delete soundItem->_soundResource;
		delete soundItem;
		_soundItems.remove_at(_soundIndex1);
		_soundIndex1 = -1;
	}

	if (_soundIndex2 != -1 && _soundItems[_soundIndex2]->_nameHash == nameHash) {
		soundItem = _soundItems[_soundIndex2];
		soundItem->_soundResource->unload();
		delete soundItem->_soundResource;
		delete soundItem;
		_soundItems.remove_at(_soundIndex2);
		_soundIndex2 = -1;
	}

	for (int index = _soundItems.size() - 1; index >= 0; --index) {
		soundItem = _soundItems[index];
		if (soundItem->_nameHash == nameHash) {
			soundItem->_soundResource->unload();
			delete soundItem->_soundResource;
			delete soundItem;
			_soundItems.remove_at(index);
		}
	}

}

void SoundMan::playTwoSounds(uint32 nameHash, uint32 soundFileHash1, uint32 soundFileHash2, int16 initialCountdown) {

	SoundItem *soundItem;
	int16 currCountdown1 = _initialCountdown;
	int16 currCountdown2 = _initialCountdown / 2;

	if (_soundIndex1 != -1) {
		soundItem = _soundItems[_soundIndex1];
		currCountdown1 = soundItem->_currCountdown;
		soundItem->_soundResource->unload();
		delete soundItem->_soundResource;
		delete soundItem;
		_soundItems.remove_at(_soundIndex1);
		_soundIndex1 = -1;
	}

	if (_soundIndex2 != -1) {
		soundItem = _soundItems[_soundIndex2];
		currCountdown2 = soundItem->_currCountdown;
		soundItem->_soundResource->unload();
		delete soundItem->_soundResource;
		delete soundItem;
		_soundItems.remove_at(_soundIndex2);
		_soundIndex2 = -1;
	}

	if (initialCountdown > 0)
		_initialCountdown = initialCountdown;

	if (soundFileHash1 != 0) {
		soundItem = new SoundItem();
		soundItem->_nameHash = nameHash;
		soundItem->_soundFileHash = soundFileHash1;
		soundItem->_playOnceAfterRandomCountdown = false;
		soundItem->_minCountdown = 0;
		soundItem->_maxCountdown = 0;
		soundItem->_playOnceAfterCountdown = _playOnceAfterCountdown;
		soundItem->_initialCountdown = _initialCountdown;
		soundItem->_playLooping = false;
		soundItem->_currCountdown = currCountdown1;
		soundItem->_soundResource = new SoundResource(_vm);
		soundItem->_soundResource->load(soundFileHash1);
		soundItem->_soundResource->setVolume(80);
		_soundIndex1 = _soundItems.size();
		_soundItems.push_back(soundItem);
	}

	if (soundFileHash2 != 0) {
		soundItem = new SoundItem();
		soundItem->_nameHash = nameHash;
		soundItem->_soundFileHash = soundFileHash2;
		soundItem->_playOnceAfterRandomCountdown = false;
		soundItem->_minCountdown = 0;
		soundItem->_maxCountdown = 0;
		soundItem->_playOnceAfterCountdown = _playOnceAfterCountdown;
		soundItem->_initialCountdown = _initialCountdown;
		soundItem->_playLooping = false;
		soundItem->_currCountdown = currCountdown2;
		soundItem->_soundResource = new SoundResource(_vm);
		soundItem->_soundResource->load(soundFileHash2);
		soundItem->_soundResource->setVolume(80);
		_soundIndex2 = _soundItems.size();
		_soundItems.push_back(soundItem);
	}

}

void SoundMan::playSoundThree(uint32 nameHash, uint32 soundFileHash) {

	SoundItem *soundItem;

	if (_soundIndex3 != -1) {
		soundItem = _soundItems[_soundIndex3];
		soundItem->_soundResource->unload();
		delete soundItem->_soundResource;
		delete soundItem;
		_soundItems.remove_at(_soundIndex3);
		_soundIndex3 = -1;
	}

	if (soundFileHash != 0) {
		soundItem = new SoundItem();
		soundItem->_nameHash = nameHash;
		soundItem->_soundFileHash = soundFileHash;
		soundItem->_playOnceAfterRandomCountdown = false;
		soundItem->_minCountdown = 0;
		soundItem->_maxCountdown = 0;
		soundItem->_playOnceAfterCountdown = false;
		soundItem->_initialCountdown = _initialCountdown3;
		soundItem->_playLooping = false;
		soundItem->_currCountdown = 0;
		soundItem->_soundResource = new SoundResource(_vm);
		soundItem->_soundResource->load(soundFileHash);
		_soundIndex2 = _soundItems.size();
		_soundItems.push_back(soundItem);
	}
	
}

void SoundMan::setTwoSoundsPlayFlag(bool playOnceAfterCountdown) {
	if (_soundIndex1 != -1)
		_soundItems[_soundIndex1]->_playOnceAfterCountdown = playOnceAfterCountdown;
	if (_soundIndex2 != -1)
		_soundItems[_soundIndex2]->_playOnceAfterCountdown = playOnceAfterCountdown;
	_playOnceAfterCountdown = playOnceAfterCountdown;
}

void SoundMan::setSoundThreePlayFlag(bool playOnceAfterCountdown) {
	if (_soundIndex3 != -1)
		_soundItems[_soundIndex3]->_playOnceAfterCountdown = playOnceAfterCountdown;
	_playOnceAfterCountdown3 = playOnceAfterCountdown;
}

SoundItem *SoundMan::getSoundItemByHash(uint32 soundFileHash) {
	for (uint i = 0; i < _soundItems.size(); ++i)
		if (_soundItems[i]->_soundFileHash == soundFileHash)
			return _soundItems[i];
	return NULL;
}

} // End of namespace Neverhood
