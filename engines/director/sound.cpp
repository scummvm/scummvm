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

#include "director/sound.h"
#include "audio/decoders/wave.h"
#include "common/file.h"
#include "audio/decoders/aiff.h"
#include "common/system.h"
#include "common/debug.h"

namespace Director {

DirectorSound::DirectorSound() {
	_soundHandle = new Audio::SoundHandle();
    _mixer = g_system->getMixer();
}

void DirectorSound::playWAV(Common::String filename) {
	Common::File *file = new Common::File();

	if (!file->open(filename)) {
		warning("Failed to open %s", filename.c_str());
		delete file;
		return;
	}

    Audio::RewindableAudioStream *sound = Audio::makeWAVStream(file, DisposeAfterUse::YES);
    _mixer->playStream(Audio::Mixer::kSFXSoundType, _soundHandle, sound);
}

void DirectorSound::playAIFF(Common::String filename) {

	Common::File *file = new Common::File();
	if (!file->open(filename)) {
		warning("Failed to open %s", filename.c_str());
		delete file;
		return;
	}

    Audio::RewindableAudioStream *sound = Audio::makeAIFFStream(file, DisposeAfterUse::YES);
    _mixer->playStream(Audio::Mixer::kSFXSoundType, _soundHandle, sound);
}

void DirectorSound::stopSound() {
	_mixer->stopHandle(*_soundHandle);
}

} //End of namespace Director
