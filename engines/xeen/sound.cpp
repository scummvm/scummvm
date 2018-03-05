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

Sound::Sound(Audio::Mixer *mixer) : _mixer(mixer), _soundOn(true), _musicOn(true),
		_songData(nullptr), _effectsData(nullptr), _musicSide(0) {
	_SoundDriver = new AdlibSoundDriver();
}

Sound::~Sound() {
	stopAllAudio();

	delete _SoundDriver;
	delete[] _effectsData;
	delete[] _songData;
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

bool Sound::isSoundPlaying() const {
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

	_musicOn = !ConfMan.getBool("music_mute");
	if (!_musicOn)
		stopSong();
}

void Sound::loadEffectsData() {
	// Stop any prior FX
	stopFX();
	delete[] _effectsData;

	// Load in an entire driver so we have quick access to the effects data
	// that's hardcoded within it
	File file("blastmus");
	byte *effectsData = new byte[file.size()];
	file.seek(0);
	file.read(effectsData, file.size());
	file.close();
	_effectsData = effectsData;

	// Locate the playFX routine
	const byte *fx = effectsData + READ_LE_UINT16(effectsData + 10) + 12;
	assert(READ_BE_UINT16(fx + 28) == 0x81FB);
	uint numEffects = READ_LE_UINT16(fx + 30);

	assert(READ_BE_UINT16(fx + 36) == 0x8B87);
	const byte *table = effectsData + READ_LE_UINT16(fx + 38);

	// Extract the effects offsets
	_effectsOffsets.resize(numEffects);
	for (uint idx = 0; idx < numEffects; ++idx)
		_effectsOffsets[idx] = READ_LE_UINT16(&table[idx * 2]);
}

void Sound::playFX(uint effectId) {
	stopFX();
	loadEffectsData();

	if (effectId < _effectsOffsets.size()) {
		const byte *dataP = &_effectsData[_effectsOffsets[effectId]];
		_SoundDriver->playFX(effectId, dataP);
	}
}

void Sound::stopFX() {
	_SoundDriver->stopFX();
}

int Sound::songCommand(uint commandId, byte volume) {
	int result = _SoundDriver->songCommand(commandId, volume);
	if (commandId == STOP_SONG) {
		delete[] _songData;
		_songData = nullptr;
	}

	return result;
}

void Sound::playSong(Common::SeekableReadStream &stream) {
	stopSong();
	if (!_musicOn)
		return;

	byte *songData = new byte[stream.size()];
	stream.seek(0);
	stream.read(songData, stream.size());
	_songData = songData;

	_SoundDriver->playSong(_songData);
}

void Sound::playSong(const Common::String &name, int param) {
	_priorMusic = _currentMusic;
	_currentMusic = name;

	Common::File mf;
	if (mf.open(name)) {
		playSong(mf);
	} else {
		File f(name, _musicSide);
		playSong(f);
	}
}

void Sound::setMusicOn(bool isOn) {
	ConfMan.setBool("music_mute", !isOn);
	if (isOn)
		ConfMan.setBool("mute", false);

	g_vm->syncSoundSettings();
}

bool Sound::isMusicPlaying() const {
	return _SoundDriver->isPlaying();
}

} // End of namespace Xeen
