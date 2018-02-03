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

#include "audio/decoders/raw.h"
#include "audio/decoders/voc.h"
#include "common/config-manager.h"
#include "xeen/sound.h"
#include "xeen/xeen.h"

namespace Xeen {

/*------------------------------------------------------------------------*/

Sound::Sound(XeenEngine *vm, Audio::Mixer *mixer): Music(),
		_mixer(mixer), _soundOn(true) {
}

Sound::~Sound() {
	stopSound();
}

void Sound::playSound(Common::SeekableReadStream &s, int unused) {
	stopSound();
	if (!_soundOn)
		return;

	s.seek(0);
	Common::SeekableReadStream *srcStream = s.readStream(s.size());
	Audio::SeekableAudioStream *stream = Audio::makeVOCStream(srcStream,
		Audio::FLAG_UNSIGNED, DisposeAfterUse::YES);
	_mixer->playStream(Audio::Mixer::kSFXSoundType, &_soundHandle, stream);
}

void Sound::playSound(const Common::String &name, int unused) {
	File f;
	if (!f.open(name))
		error("Could not open sound - %s", name.c_str());

	playSound(f);
}

void Sound::playSound(const Common::String &name, int ccNum, int unused) {
	File f;
	if (!f.open(name, ccNum))
		error("Could not open sound - %s", name.c_str());

	playSound(f);
}

void Sound::stopSound() {
	_mixer->stopHandle(_soundHandle);
}

bool Sound::isPlaying() const {
	return _mixer->isSoundHandleActive(_soundHandle);
}

void Sound::stopAllAudio() {
	stopSong();
	stopFX();
	stopSound();
}

void Sound::setEffectsOn(bool isOn) {
	ConfMan.setBool("sfx_mute", !isOn);
	if (isOn)
		ConfMan.setBool("mute", false);

	g_vm->syncSoundSettings();
}

void Sound::updateSoundSettings() {
	_soundOn = !ConfMan.getBool("sfx_mute");
	if (!_soundOn)
		stopFX();

	Music::updateSoundSettings();
}

} // End of namespace Xeen
