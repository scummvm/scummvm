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
#include "audio/mixer.h"
#include "audio/audiostream.h"
#include "audio/decoders/raw.h"
#include "audio/decoders/wave.h"
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
		delete _soundTable[i]._res;
	_soundTable.clear();
}

void SoundManager::queueSound(int idx, int fileNum, int subfile) {
	Resource *soundResource;

	if (idx >= (int)_soundTable.size())
		_soundTable.resize(idx + 1);

	delete _soundTable[idx]._res;
	soundResource = _vm->_files->loadFile(fileNum, subfile);
	_soundTable[idx]._res = soundResource;
	_soundTable[idx]._priority = 1;
}

Resource *SoundManager::loadSound(int fileNum, int subfile) {
	return _vm->_files->loadFile(fileNum, subfile);
}

void SoundManager::playSound(int soundIndex) {
	int priority = _soundTable[soundIndex]._priority;
	playSound(_soundTable[soundIndex]._res, priority);
}

void SoundManager::playSound(Resource *res, int priority) {
	byte *resourceData = res->data();
	Audio::SoundHandle audioHandle;
	Audio::RewindableAudioStream *audioStream = 0;
	
	assert(res->_size >= 32);

	if (READ_BE_UINT32(resourceData) == MKTAG('R','I','F','F')) {
		// CD version uses WAVE-files
		Common::SeekableReadStream *waveStream = new Common::MemoryReadStream(resourceData, res->_size, DisposeAfterUse::NO);
		audioStream = Audio::makeWAVStream(waveStream, DisposeAfterUse::YES);

	} else if (READ_BE_UINT32(resourceData) == MKTAG('S', 'T', 'E', 'V')) {
		// sound files have a fixed header of 32 bytes in total
		//  header content:
		//   "STEVE" - fixed header
		//   byte    - sample rate
		//              01h mapped internally to 3Ch ??
		//              02h mapped internally to 78h seems to be 11025Hz
		//              03h mapped internally to B5h
		//              04h mapped internally to F1h
		//   byte    - unknown
		//   word    - actual sample size (should be resource-size - 32)
		byte internalSampleRate = resourceData[5];
		int sampleSize = READ_LE_UINT16(resourceData + 7);
	
		assert( (sampleSize + 32) == res->_size);

		int sampleRate = 0;
		switch (internalSampleRate) {
		case 1: sampleRate = 16666; break; // 3Ch -> C4h time constant
		case 2: sampleRate = 8334; break;  // 78h -> 88h time constant
		case 3: sampleRate = 5525; break;  // B5h -> 4Bh time constant
		case 4: sampleRate = 4150; break;  // F1h -> 0Fh time constant
		default:
			error("Unexpected internal Sample Rate %d", internalSampleRate);
			return;
		}
	
		audioStream = Audio::makeRawStream(resourceData + 32, sampleSize, sampleRate, 0);

	} else
		error("Unknown format");
	
	audioHandle = Audio::SoundHandle();
	_mixer->playStream(Audio::Mixer::kSFXSoundType, &audioHandle,
						audioStream, -1, _mixer->kMaxChannelVolume, 0,
						DisposeAfterUse::NO);

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
		Resource *sound = loadSound(sounds[i]._fileNum, sounds[i]._subfile);
		_soundTable.push_back(SoundEntry(sound, sounds[i]._priority));
	}
}

void SoundManager::midiPlay() {
	// TODO
}

bool SoundManager::checkMidiDone() {
	// TODO
	return true;
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
