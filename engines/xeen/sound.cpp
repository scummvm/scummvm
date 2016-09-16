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

#include "audio/decoders/voc.h"
#include "xeen/sound.h"
#include "xeen/xeen.h"

namespace Xeen {

Sound *Voc::_sound;

Voc::Voc(const Common::String &name) {
	if (!open(name))
		error("Could not open - %s", name.c_str());
}

void Voc::init(XeenEngine *vm) {
	_sound = vm->_sound;
}

void Voc::play() {
	_sound->playSound(this, _soundHandle);
}

void Voc::stop() {
	_sound->stopSound(_soundHandle);
}

/*------------------------------------------------------------------------*/

Sound::Sound(XeenEngine *vm, Audio::Mixer *mixer): Music(), _mixer(mixer) {
}

void Sound::proc2(Common::SeekableReadStream &f) {
	// TODO
}

void Sound::startMusic(int v1) {
	// TODO
}

void Sound::stopMusic(int id) {
	// TODO
}

void Sound::playSound(Common::SeekableReadStream *s, Audio::SoundHandle &soundHandle,
	Audio::Mixer::SoundType soundType) {
	Audio::SeekableAudioStream *stream = Audio::makeVOCStream(s, 0);
	_mixer->playStream(soundType, &soundHandle, stream);		
}

void Sound::stopSound(Audio::SoundHandle &soundHandle) {
	_mixer->stopHandle(soundHandle);
}

} // End of namespace Xeen
