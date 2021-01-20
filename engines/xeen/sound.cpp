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
#include "backends/audiocd/audiocd.h"
#include "common/config-manager.h"
#include "xeen/sound.h"
#include "xeen/sound_driver_adlib.h"
#include "xeen/xeen.h"

namespace Xeen {

Sound::Sound(Audio::Mixer *mixer) : _mixer(mixer), _fxOn(true), _musicOn(true), _subtitles(false),
		_songData(nullptr), _effectsData(nullptr), _musicSide(0), _musicPercent(100),
		_musicVolume(0), _sfxVolume(0) {
	_SoundDriver = new SoundDriverAdlib();
	if (g_vm->getIsCD())
		g_system->getAudioCDManager()->open();
}

Sound::~Sound() {
	stopAllAudio();
	if (g_vm->getIsCD())
		g_system->getAudioCDManager()->close();

	delete _SoundDriver;
	delete[] _effectsData;
	delete[] _songData;
}

void Sound::playSound(Common::SeekableReadStream &s, int unused) {
	stopSound();
	if (!_fxOn)
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

void Sound::playVoice(const Common::String &name, int ccMode) {
	File f;
	bool result = (ccMode == -1) ? f.open(name) : f.open(name, ccMode);
	if (!result)
		error("Could not open sound - %s", name.c_str());

	stopSound();

	Common::SeekableReadStream *srcStream = f.readStream(f.size());
	Audio::SeekableAudioStream *stream = Audio::makeVOCStream(srcStream,
		Audio::FLAG_UNSIGNED, DisposeAfterUse::YES);
	_mixer->playStream(Audio::Mixer::kSpeechSoundType, &_soundHandle, stream);
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
	setMusicPercent(100);
}

void Sound::setFxOn(bool isOn) {
	ConfMan.setBool("sfx_mute", !isOn);
	if (isOn)
		ConfMan.setBool("mute", false);

	g_vm->syncSoundSettings();
}

void Sound::loadEffectsData() {
	// Stop any prior FX
	stopFX();

	if (!_effectsData) {
		// Load in an entire driver so we have quick access to the effects data that's hardcoded within it
		const char *name = "blastmus";
		File file(name);
		size_t size = file.size();
		byte *effectsData = new byte[size];

		if (file.read(effectsData, size) != size) {
			delete[] effectsData;
			error("Failed to read %zu bytes from '%s'", size, name);
		}

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

int Sound::songCommand(uint commandId, byte musicVolume, byte sfxVolume) {
	int result = _SoundDriver->songCommand(commandId, musicVolume, sfxVolume);
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

	if (!stream.seek(0))
		error("Failed to seek to 0 for song data");

	size_t size = stream.size();
	byte *songData = new byte[size];

	if (stream.read(songData, size) != size) {
		delete[] songData;
		error("Failed to read %zu bytes of song data", size);
	}

	assert(!_songData);
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

void Sound::setMusicPercent(byte percent) {
	assert(percent <= 100);
	_musicPercent = percent;

	updateVolume();
}

void Sound::updateSoundSettings() {
	_fxOn = !ConfMan.getBool("sfx_mute");
	if (!_fxOn)
		stopFX();

	_musicOn = !ConfMan.getBool("music_mute");
	if (!_musicOn)
		stopSong();

	_subtitles = ConfMan.hasKey("subtitles") ? ConfMan.getBool("subtitles") : true;
	_musicVolume = CLIP(ConfMan.getInt("music_volume"), 0, 255);
	_sfxVolume = CLIP(ConfMan.getInt("sfx_volume"), 0, 255);
	updateVolume();
}

void Sound::updateVolume() {
	songCommand(SET_VOLUME, _musicPercent * _musicVolume / 100, _sfxVolume);
}

} // End of namespace Xeen
