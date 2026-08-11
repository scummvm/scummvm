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

#include "got/sound.h"

#include "got/got.h"
#include "got/musicdriver_adlib.h"
#include "got/utils/file.h"

#include "common/config-manager.h"
#include "common/memstream.h"
#include "audio/mididrv.h"
#include "audio/decoders/raw.h"
#include "audio/decoders/voc.h"

namespace Got {

static const byte SOUND_PRIORITY[] = {1, 2, 3, 3, 3, 1, 4, 4, 4, 5, 4, 3, 1, 2, 2, 5, 1, 3, 1};

static const char *MUSIC_MENU_DATA_FILENAME = "GOT.AUD";

Sound::Sound() {
	for (int i = 0; i < 3; i++)
		_bossSounds[i] = nullptr;
}

Sound::~Sound() {
	delete[] _soundData;
	for (int i = 0; i < ARRAYSIZE(_bossSounds); i++) {
		delete[] _bossSounds[i];
	}

	if (_musicParser != nullptr) {
		musicStop();
	}

	if (_musicDriver != nullptr) {
		_musicDriver->setTimerCallback(nullptr, nullptr);
		_musicDriver->close();
	}

	delete _musicParser;
	delete _musicDriver;
	delete[] _musicData;
}

void Sound::load() {
	File f("DIGSOUND");

	// Load index
	for (int i = 0; i < 16; ++i)
		_digiSounds[i].load(&f);

	// Allocate memory and load sound data
	_soundData = new byte[f.size() - 16 * 8];
	f.read(_soundData, f.size() - 16 * 8);

	// Initialize music.

	// Check the type of music device that the user has configured.
	MidiDriver::DeviceHandle dev = MidiDriver::detectDevice(MDT_ADLIB);
	MusicType deviceType = MidiDriver::getMusicType(dev);

	// Initialize the appropriate driver.
	switch (deviceType) {
		case MT_ADLIB:
			_musicDriver = new MusicDriver_Got_AdLib(MUSIC_TIMER_FREQUENCY_GAME);
			break;
		default:
			// No support for music other than AdLib.
			_musicDriver = new MusicDriver_Got_NULL(MUSIC_TIMER_FREQUENCY_GAME);
			break;
	}

	// Initialize the parser.
	_musicParser = new MusicParser_Got();

	// Open the driver.
	int returnCode = _musicDriver->open();
	if (returnCode != 0) {
		warning("Sound::load - Failed to open music driver - error code %d.", returnCode);
		return;
	}

	// Apply user volume settings.
	syncSoundSettings();

	// Connect the driver and the parser.
	_musicParser->setMusicDriver(_musicDriver);
	_musicDriver->setTimerCallback(_musicParser, &_musicParser->timerCallback);
}

void Sound::setupBoss(const int num) {
	if (_currentBossLoaded == num)
		// Boss sounds are already loaded
		return;

	if (_currentBossLoaded) {
		// Boss sounds have already been loaded. Delete them to avoid memory leak
		for (int i = 0; i < 3; i++)
			delete(_bossSounds[i]);
	}

	// Information concerning boss sounds is stored in _digiSounds 16 to 18, but the data is stored in _bossSounds
	for (int i = 0; i < 3; ++i) {
		Common::String resourceName = Common::String::format("BOSSV%d%d", num, i + 1);
		File f(resourceName);
		const int size = f.size();
		_bossSounds[i] = new byte[size];
		_digiSounds[16 + i]._length = size;
		_digiSounds[16 + i]._offset = 0;
		f.read(_bossSounds[i], size);
		f.close();
	}

	_currentBossLoaded = num;
}

void Sound::playSound(const int index, const bool override) {
	if (index >= NUM_SOUNDS)
		return;

	byte newPriority = SOUND_PRIORITY[index];

	// If a sound is playing, stop it unless there is a priority override
	if (soundPlaying()) {
		if (!override && _currentPriority < newPriority)
			return;

		g_engine->_mixer->stopHandle(_soundHandle);
	}

	_currentPriority = newPriority;

	Common::MemoryReadStream *stream;
	if (index >= 16) {
		// Boss sounds are not stored in the normal sound data, it's in 3 buffers in _bossSounds.
		stream = new Common::MemoryReadStream(_bossSounds[index - 16], _digiSounds[index]._length);
	} else {
		// Normal digital sound
		stream = new Common::MemoryReadStream(_soundData + _digiSounds[index]._offset, _digiSounds[index]._length);
	}
	// Play the new sound
	Audio::AudioStream *audioStream = Audio::makeVOCStream(stream, Audio::FLAG_UNSIGNED, DisposeAfterUse::YES);
	g_engine->_mixer->playStream(Audio::Mixer::kSFXSoundType, &_soundHandle, audioStream);
}

void Sound::playSound(const Gfx::GraphicChunk &src) {
	if (soundPlaying())
		g_engine->_mixer->stopHandle(_soundHandle);

	// Play the new sound
	Common::MemoryReadStream *stream = new Common::MemoryReadStream(
		src._data, src._uncompressedSize);
	Audio::AudioStream *audioStream = Audio::makeVOCStream(stream, Audio::FLAG_UNSIGNED,
														   DisposeAfterUse::YES);
	g_engine->_mixer->playStream(Audio::Mixer::kSFXSoundType, &_soundHandle, audioStream);
}

bool Sound::soundPlaying() const {
	return g_engine->_mixer->isSoundHandleActive(_soundHandle);
}

void Sound::musicPlay(const char *name, bool override) {
	if (_currentMusic == nullptr || strcmp(name, _currentMusic) || override) {
		_musicParser->stopPlaying();
		_musicParser->unloadMusic();
		delete[] _musicData;

		_currentMusic = name;

		File file;
		if (!strcmp(name, "MENU")) {
			// Title menu music is embedded in the executable.
			// It has been extracted and included with ScummVM.
			if (!file.exists(MUSIC_MENU_DATA_FILENAME)) {
				warning("Could not find %s", MUSIC_MENU_DATA_FILENAME);
				return;
			}
			file.open(MUSIC_MENU_DATA_FILENAME);

			// Title music uses an alternate timer frequency.
			_musicDriver->setTimerFrequency(MUSIC_TIMER_FREQUENCY_TITLE);
		} else {
			file.open(name);

			_musicDriver->setTimerFrequency(MUSIC_TIMER_FREQUENCY_GAME);
		}

		// Copy music data to local buffer and load it into the parser.
		_musicData = new byte[file.size()];
		file.read(_musicData, file.size());

		if (!_musicParser->loadMusic(_musicData, file.size())) {
			warning("Could not load music track %s", name);
			return;
		}

		//debug("Playing music track %s", name);
		_musicParser->startPlaying();
	}
}

void Sound::musicPause() {
	_musicParser->pausePlaying();
}

void Sound::musicResume() {
	_musicParser->resumePlaying();
}

void Sound::musicStop() {
	_musicParser->stopPlaying();
	_currentMusic = nullptr;
}

bool Sound::musicIsOn() const {
	return _musicParser->isPlaying();
}

const char *Sound::getMusicName(const int num) const {
	const char *name = nullptr;

	switch (_G(area)) {
	case 1:
		switch (num) {
		case 0:
			name = "SONG1";
			break;
		case 1:
			name = "SONG2";
			break;
		case 2:
			name = "SONG3";
			break;
		case 3:
			name = "SONG4";
			break;
		case 4:
			name = "WINSONG";
			break;
		case 5:
			name = "BOSSSONG";
			break;
		default:
			break;
		}
		break;

	case 2:
		switch (num) {
		case 0:
			name = "SONG21";
			break;
		case 1:
			name = "SONG22";
			break;
		case 2:
			name = "SONG23";
			break;
		case 3:
			name = "SONG24";
			break;
		case 4:
			name = "SONG35";
			break;
		case 5:
			name = "SONG25";
			break;
		case 6:
			name = "WINSONG";
			break;
		case 7:
			name = "BOSSSONG";
			break;
		default:
			break;
		}
		break;

	case 3:
		switch (num) {
		case 0:
			name = "SONG31";
			break;
		case 1:
			name = "SONG32";
			break;
		case 2:
			name = "SONG33";
			break;
		case 3:
			name = "SONG34";
			break;
		case 4:
			name = "SONG35";
			break;
		case 5:
			name = "SONG36";
			break;
		case 6:
			name = "WINSONG";
			break;
		case 7:
			name = "BOSSSONG";
			break;
		default:
			break;
		}
		break;

	default:
		break;
	}

	if (!name)
		error("Invalid music");

	return name;
}

void Sound::syncSoundSettings() {
	g_engine->_mixer->muteSoundType(Audio::Mixer::kSFXSoundType, ConfMan.getBool("sfx_mute") || ConfMan.getBool("mute"));

	if (_musicDriver)
		_musicDriver->syncSoundSettings();
}

void playSound(const int index, const bool override) {
	_G(sound).playSound(index, override);
}

void playSound(const Gfx::GraphicChunk &src) {
	_G(sound).playSound(src);
}

bool soundPlaying() {
	return _G(sound).soundPlaying();
}

void musicPlay(const int num, const bool override) {
	_G(sound).musicPlay(num, override);
}

void musicPlay(const char *name, const bool override) {
	_G(sound).musicPlay(name, override);
}

void musicPause() {
	_G(sound).musicPause();
}

void musicResume() {
	_G(sound).musicResume();
}

void setupBoss(int num) {
	_G(sound).setupBoss(num);
}

} // namespace Got
