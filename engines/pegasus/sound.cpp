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

#include "pegasus/sound.h"

#include "common/file.h"
#include "audio/decoders/aiff.h"

namespace Pegasus {

SoundManager::SoundManager(PegasusEngine *vm) : _vm(vm) {
}
        
void SoundManager::playSound(Common::String filename, bool loop) {
	SndHandle *handle = getHandle();
	handle->type = kUsedHandle;

	Common::File *file = new Common::File();
	if (!file->open(filename.c_str()))
		error("Could not open file \'%s\'", filename.c_str());

	Audio::AudioStream* audStream = Audio::makeAIFFStream(file, DisposeAfterUse::YES);

	if (loop)
		audStream = Audio::makeLoopingAudioStream((Audio::RewindableAudioStream*)audStream, 0);
	
	if (audStream)
		_vm->_mixer->playStream(Audio::Mixer::kPlainSoundType, &handle->handle, audStream);
}

SndHandle *SoundManager::getHandle() {
	for (int i = 0; i < SOUND_HANDLES; i++) {
		if (_handles[i].type == kFreeHandle)
			return &_handles[i];

		if (!_vm->_mixer->isSoundHandleActive(_handles[i].handle)) {
			_handles[i].type = kFreeHandle;
			return &_handles[i];
		}
	}

	error("SoundManager::getHandle(): Too many sound handles");
	return NULL;
}

bool SoundManager::isPlaying() {
	for (int i = 0; i < SOUND_HANDLES; i++)
		if (_handles[i].type == kUsedHandle)
			if (_vm->_mixer->isSoundHandleActive(_handles[i].handle))
				return true;
	return false;
}

void SoundManager::stopSound() {
	_vm->_mixer->stopAll();
}

void SoundManager::pauseSound() {
	_vm->_mixer->pauseAll(true);
}

void SoundManager::resumeSound() {
	_vm->_mixer->pauseAll(false);
}

} // End of namespace Pegasus
