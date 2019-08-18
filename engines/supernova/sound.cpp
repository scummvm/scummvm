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

#include "audio/audiostream.h"
#include "audio/mixer.h"
#include "audio/decoders/raw.h"
#include "audio/mods/protracker.h"
#include "common/system.h"

#include "supernova/resman.h"
#include "supernova/sound.h"
#include "supernova/supernova.h"

namespace Supernova {

Sound::Sound(Audio::Mixer *mixer, ResourceManager *resMan)
	: _mixer(mixer)
	, _resMan(resMan) {
}

void Sound::play(AudioId index) {
	Audio::AudioStream *stream = _resMan->getSoundStream(index);

	stop();
	_mixer->playStream(Audio::Mixer::kPlainSoundType, &_soundHandle, stream,
					   -1, Audio::Mixer::kMaxChannelVolume, 0, DisposeAfterUse::NO);
}

void Sound::play(MusicId index) {
	Audio::AudioStream *stream = _resMan->getSoundStream(index);

	stop();
	_mixer->playStream(Audio::Mixer::kMusicSoundType, &_soundHandle, stream,
					   -1, Audio::Mixer::kMaxChannelVolume, 0, DisposeAfterUse::NO);
}

void Sound::playSiren() {
	Audio::AudioStream *stream = _resMan->getSirenStream();

	stop();
	_mixer->playStream(Audio::Mixer::kPlainSoundType, &_soundHandle, stream,
					   -1, Audio::Mixer::kMaxChannelVolume, 0, DisposeAfterUse::NO);
}

bool Sound::isPlaying() {
	return _mixer->isSoundHandleActive(_soundHandle);
}

void Sound::stop() {
	if (_mixer->isSoundHandleActive(_soundHandle))
		_mixer->stopHandle(_soundHandle);
}

int Sound::getVolume() {
	return _mixer->getChannelVolume(_soundHandle);
}

void Sound::setVolume(int volume) {
	_mixer->setChannelVolume(_soundHandle, volume);
}

}
