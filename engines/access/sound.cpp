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
	Common::fill(&_soundPriority[0], &_soundPriority[MAX_SOUNDS], 0);
	for (int i = 0; i < MAX_SOUNDS; ++i)
		_soundTable[i]._data = nullptr;
}

SoundManager::~SoundManager() {
	for (int i = 0; i < MAX_SOUNDS; ++i)
		delete _soundTable[i]._data;
}

void SoundManager::queueSound(int idx, int fileNum, int subfile) {
	/*
	_soundTable[idx]._data = _vm->_files->loadFile(fileNum, subfile);
	_soundTable[idx]._size = _vm->_files->_filesize;
	*/
}

byte *SoundManager::loadSound(int fileNum, int subfile) {
	return _vm->_files->loadFile(fileNum, subfile);
}

void SoundManager::playSound(int soundIndex) {
	int idx = _soundPriority[soundIndex - 1] - 1;
	playSound(_soundTable[idx]._data, _soundTable[idx]._size);
}

void SoundManager::playSound(byte *data, uint32 size) {
	/*
	Audio::QueuingAudioStream *audioStream = Audio::makeQueuingAudioStream(22050, false);
	audioStream->queueBuffer(data, size, DisposeAfterUse::YES, 0);
	_mixer->playStream(Audio::Mixer::kPlainSoundType, &_soundHandle, audioStream, -1, 
		Audio::Mixer::kMaxChannelVolume, 0, DisposeAfterUse::YES, false);
		*/
}

} // End of namespace Access
