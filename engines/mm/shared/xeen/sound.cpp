/* ScummVM - Graphic Adventure Engine
 *
 * ScummVM is the legal property of its developers, whose names
 * are too numerous to list here. Please refer to the COPYRIGHT
 * file distributed with this source distribution.
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 *
 */
#include "audio/decoders/raw.h"
#include "audio/decoders/voc.h"
#include "backends/audiocd/audiocd.h"
#include "common/config-manager.h"
#include "mm/shared/xeen/sound.h"
#include "mm/shared/xeen/sound_driver_adlib.h"
#include "mm/shared/xeen/sound_driver_mt32.h"
#include "mm/xeen/xeen.h"
#include "mm/mm.h"

namespace MM {
namespace Shared {
namespace Xeen {

Sound::Sound(Audio::Mixer *mixer) : _mixer(mixer), _fxOn(true), _musicOn(true), _subtitles(false),
_songData(nullptr), _SoundDriver(nullptr), _effectsData(nullptr), _musicSide(0), _musicPercent(100),
_musicVolume(0), _sfxVolume(0) {
	MidiDriver::DeviceHandle dev = MidiDriver::detectDevice(MDT_ADLIB | MDT_MIDI | MDT_PREFER_MT32);
	musicType = MidiDriver::getMusicType(dev);

	switch (musicType) {
	case MT_MT32:
		_SoundDriver = new SoundDriverMT32();
		debugC(1, "Selected mt32 sound driver\n");
		break;
	case MT_ADLIB:
	default:
		_SoundDriver = new SoundDriverAdlib();
		debugC(1, "Selected adlib sound driver\n");
		break;
	}

	// force load effects early so custom instruments for mt32 are loaded before sound is played.
	loadEffectsData();

	assert(_SoundDriver);

	if (g_engine->getIsCD())
		g_system->getAudioCDManager()->open();
}

Sound::~Sound() {
	stopAllAudio();
	if (g_engine->getIsCD())
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

void Sound::playSound(const Common::Path &name, int unused) {
	File f;
	if (!f.open(name))
		error("Could not open sound - %s", name.toString().c_str());

	playSound(f);
}

#ifdef ENABLE_XEEN
void Sound::playSound(const Common::Path &name, int ccNum, int unused) {
	File f;
	if (!f.open(name, ccNum))
		error("Could not open sound - %s", name.toString().c_str());

	playSound(f);
}
#endif

#ifdef ENABLE_XEEN
void Sound::playVoice(const Common::Path &name, int ccMode) {
#else
void Sound::playVoice(const Common::Path &name) {
#endif
	stopSound();
	if (!_fxOn)
		return;
	File f;
#ifdef ENABLE_XEEN
	bool result = (ccMode == -1) ? f.open(name) : f.open(name, ccMode);
#else
	bool result = f.open(name);
#endif

	if (!result)
		error("Could not open sound - %s", name.toString().c_str());
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
	stopFX(true);
	stopSound();
	setMusicPercent(100);
}

void Sound::setFxOn(bool isOn) {
	ConfMan.setBool("sfx_mute", !isOn);
	if (isOn)
		ConfMan.setBool("mute", false);
	ConfMan.flushToDisk();

	g_engine->syncSoundSettings();
}

void Sound::loadEffectsData() {
	// Stop any prior FX
	stopFX();

	// Skip if the sound driver hasn't been loaded, or effects have already been loaded
	if (!_SoundDriver || _effectsData)
		return;

	if (musicType == MT_ADLIB) {
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
	} else if (musicType == MT_MT32) {
		// Load in an entire driver so we have quick access to the effects data that's hardcoded within it
		const char *name = "rolmus";
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
		assert(READ_BE_UINT16(fx + 36) == 0x81FB);
		// TODO: Investigate, additional 10 effects seem to exist in the table beyond the base 180. Not unique to rolmus as at least admus, blastmus and canmus have them too.
		uint numEffects = READ_LE_UINT16(fx + 38);

		assert(READ_BE_UINT16(fx + 80) == 0x8B87);
		const byte *table = effectsData + READ_LE_UINT16(fx + 82);

		// Extract the effects offsets
		_effectsOffsets.resize(numEffects);
		for (uint idx = 0; idx < numEffects; ++idx)
			_effectsOffsets[idx] = READ_LE_UINT16(&table[idx * 2]);

		// rolmus in intro.cc
		if (effectsData[1] == 0xBD) {
			_patchesOffsetsMT32 = {0x0A86, 0x0ABC, 0x10A3, 0x0BC1, 0x10AF, 0x0CBB, 0x10BB, 0x0DB5, 0x10C7, 0x0EAF, 0x10D3, 0x0FA9, 0x10DF};
			debugC(3, "intro rolmus");
		// rolmus in xeen.cc
		} else if (effectsData[1] == 0xB9) {
			_patchesOffsetsMT32 = {0x09A7, 0x09DD, 0x0FC4, 0x0AE2, 0x0FD0, 0x0BDC, 0x0FDC, 0x0CD6, 0x0FE8, 0x0DD0, 0x0FF4, 0x0ECA, 0x1000};
			debugC(3, "xeen rolmus");
		}

		assert(_patchesOffsetsMT32.size() == 13);

		for (uint idx = 0; idx < 13; idx++) {
			const byte *ptr = &effectsData[_patchesOffsetsMT32[idx]];

			_SoundDriver->sysExMessage(ptr);
		}
	}
}

void Sound::playFX(uint effectId) {
	stopFX();
	if (!_fxOn)
		return;
	loadEffectsData();

	if (_SoundDriver && effectId < _effectsOffsets.size()) {
		const byte *dataP = &_effectsData[_effectsOffsets[effectId]];
		_SoundDriver->playFX(effectId, dataP);
	}
}

void Sound::stopFX(bool force) {
	if (_SoundDriver)
		_SoundDriver->stopFX(force);
}

int Sound::songCommand(uint commandId, byte musicVolume, byte sfxVolume) {
	int result = 0;

	if (_SoundDriver)
		result = _SoundDriver->songCommand(commandId, musicVolume, sfxVolume);

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

	if (_SoundDriver)
		_SoundDriver->playSong(_songData);
}

void Sound::playSong(const Common::Path &name, int param) {
	if (isMusicPlaying() && name == _currentMusic)
		return;
	_currentMusic = name;

	Common::File mf;
	if (mf.open(name)) {
		playSong(mf);
#ifdef ENABLE_XEEN
	} else if (dynamic_cast<MM::Xeen::XeenEngine *>(g_engine)) {
		File f(name, _musicSide);
		playSong(f);
#endif
	} else {
		File f(name);
		playSong(f);
	}
}

void Sound::setMusicOn(bool isOn) {
	ConfMan.setBool("music_mute", !isOn);
	if (isOn)
		ConfMan.setBool("mute", false);
	ConfMan.flushToDisk();

	g_engine->syncSoundSettings();
}

bool Sound::isMusicPlaying() const {
	return _SoundDriver && _SoundDriver->isPlaying();
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
	else if (!_currentMusic.empty())
		playSong(_currentMusic);

	_subtitles = ConfMan.hasKey("subtitles") ? ConfMan.getBool("subtitles") : true;
	if (ConfMan.getBool("mute")) {
		_musicVolume = 0;
		_sfxVolume = 0;
	} else {
		_musicVolume = CLIP(ConfMan.getInt("music_volume"), 0, 255);
		_sfxVolume = CLIP(ConfMan.getInt("sfx_volume"), 0, 255);
	}
	updateVolume();
}

void Sound::updateVolume() {
	songCommand(SET_VOLUME, _musicPercent * _musicVolume / 100, _sfxVolume);
}

} // namespace Xeen
} // namespace Shared
} // namespace MM
