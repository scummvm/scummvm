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

#include "common/algorithm.h"
#include "audio/audiostream.h"
#include "access/access.h"
#include "access/sound.h"

namespace Access {

SoundManager::SoundManager(AccessEngine *vm, Audio::Mixer *mixer) : 
		_vm(vm), _mixer(mixer) {
	_music = nullptr;
	_tempMusic = nullptr;
	_musicRepeat = false;
	_playingSound = false;
	_isVoice = false;
}

SoundManager::~SoundManager() {
	clearSounds();
	delete _music;
	delete _tempMusic;
}

void SoundManager::clearSounds() {
	for (uint i = 0; i < _soundTable.size(); ++i)
		delete _soundTable[i];
	_soundTable.clear();
	_soundPriority.clear();
}

void SoundManager::queueSound(int idx, int fileNum, int subfile) {
	delete _soundTable[idx];
	_soundTable[idx] = _vm->_files->loadFile(fileNum, subfile);
}

Resource *SoundManager::loadSound(int fileNum, int subfile) {
	return _vm->_files->loadFile(fileNum, subfile);
}

void SoundManager::playSound(int soundIndex) {
	int priority = _soundPriority[soundIndex];
	playSound(_soundTable[soundIndex], priority);
}

void SoundManager::playSound(Resource *res, int priority) {
	/*
	Audio::QueuingAudioStream *audioStream = Audio::makeQueuingAudioStream(22050, false);
	audioStream->queueBuffer(data, size, DisposeAfterUse::YES, 0);
	_mixer->playStream(Audio::Mixer::kPlainSoundType, &_soundHandle, audioStream, -1, 
		Audio::Mixer::kMaxChannelVolume, 0, DisposeAfterUse::YES, false);
		*/
}

void SoundManager::loadSounds(Common::Array<RoomInfo::SoundIdent> &sounds) {
	clearSounds();

	for (uint i = 0; i < sounds.size(); ++i) {
		_soundTable.push_back(loadSound(sounds[i]._fileNum, sounds[i]._subfile));
		_soundPriority.push_back(sounds[i]._priority);
	}
}

void SoundManager::midiPlay() {
	// TODO
}

void SoundManager::midiRepeat() {
	// TODO
}

void SoundManager::stopSong() {
	// TODO
}

void SoundManager::stopSound() {
	// TODO: REALSTOPSND or BLASTSTOPSND or STOP_SOUNDG
}

void SoundManager::freeSounds() {
	stopSound();
	clearSounds();
}

void SoundManager::newMusic(int musicId, int mode) {
	if (mode == 1) {
		stopSong();
		freeMusic();
		_music = _tempMusic;
		_tempMusic = nullptr;
		_musicRepeat = true;
		if (_music)
			midiPlay();
	} else {
		_musicRepeat = (mode == 2);
		_tempMusic = _music;
		stopSong();
		_music = loadSound(97, musicId);
	}
}

void SoundManager::freeMusic() {
	delete _music;
	_music = nullptr;
}

} // End of namespace Access
