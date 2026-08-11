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

#include "darkseed/music.h"
#include "darkseed/darkseed.h"

namespace Darkseed {

MusicPlayer::MusicPlayer(DarkseedEngine* vm, bool useFloppyMusic, bool useFloppySfx) :
		_vm(vm),
		_driver(nullptr),
		_floppyAdLibDriver(nullptr),
		_paused(false),
		_deviceType(MT_NULL),
		_musicParser(nullptr),
		_musicData(nullptr),
		_sfxParserSbr(nullptr),
		_sfxData(nullptr),
		_tosInstrumentBankData(nullptr),
		_tosInstrumentBankLoaded(false),
		_useFloppyMusic(useFloppyMusic),
		_useFloppySfx(useFloppySfx) {
	for (int i = 0; i < NUM_SFX_PARSERS; i++) {
		_sfxParsers[i] = nullptr;
	}
}

MusicPlayer::~MusicPlayer() {
	stopMusic();
	stopAllSfx();
	if (_driver != nullptr) {
		_driver->setTimerCallback(nullptr, nullptr);
		_driver->close();
	}

	Common::StackLock lock(_mutex);

	if (_musicParser != nullptr)
		delete _musicParser;
	if (_musicData != nullptr)
		delete[] _musicData;
	for (int i = 0; i < NUM_SFX_PARSERS; i++) {
		if (_sfxParsers[i] != nullptr)
			delete _sfxParsers[i];
	}
	if (_sfxData != nullptr)
		delete _sfxData;
	if (_tosInstrumentBankData != nullptr)
		delete[] _tosInstrumentBankData;
	if (_driver != nullptr) {
		delete _driver;
		_driver = nullptr;
	}
}

int MusicPlayer::open() {
	assert(!_driver);

	int devFlags = MDT_ADLIB; // | MDT_PCSPK;
	MidiDriver::DeviceHandle dev = MidiDriver::detectDevice(devFlags);
	_deviceType = MidiDriver::getMusicType(dev);

	if (_useFloppyMusic) {
		switch (_deviceType) {
			case MT_ADLIB:
				_floppyAdLibDriver = new MidiDriver_DarkSeedFloppy_AdLib(OPL::Config::kOpl2);
				_driver = _floppyAdLibDriver;
				break;
			case MT_PCSPK:
				// TODO Implement PC speaker driver
			default:
				_driver = new MidiDriver_NULL_Multisource();
				break;
		}

		_musicParser = new MidiParser_SBR(0);
		if (_useFloppySfx) {
			for (int i = 0; i < NUM_SFX_PARSERS; i++) {
				if (_sfxParserSbr == nullptr) {
					_sfxParserSbr = new MidiParser_SBR(i + 1, true);
					_sfxParsers[i] = _sfxParserSbr;
				}
				else {
					_sfxParsers[i] = new MidiParser_SBR(i + 1, true);
				}
			}
		}
	} else {
		switch (_deviceType) {
			case MT_ADLIB:
				_driver = new MidiDriver_Worx_AdLib(OPL::Config::kOpl2);
				// Some tracks do not set instruments and expect instrument 0
				// to be set on each channel. Make sure this is done every time
				// a track starts.
				_driver->setControllerDefault(MidiDriver_Multisource::CONTROLLER_DEFAULT_PROGRAM);
				break;
			case MT_PCSPK:
				// TODO Implement PC speaker driver
			default:
				_driver = new MidiDriver_NULL_Multisource();
				break;
		}

		// CD version uses SMF data
		_musicParser = MidiParser::createParser_SMF(0);
	}

	_driver->property(MidiDriver::PROP_USER_VOLUME_SCALING, true);
	if (_musicParser != nullptr)
		_musicParser->property(MidiParser::mpDisableAutoStartPlayback, true);
	for (int i = 0; i < NUM_SFX_PARSERS; i++) {
		if (_sfxParsers[i] != nullptr) {
			_sfxParsers[i]->property(MidiParser::mpDisableAutoStartPlayback, true);
		}
	}

	int returnCode = _driver->open();
	if (returnCode != 0) {
		error("MusicPlayer::open - Failed to open MIDI driver - error code %d.", returnCode);
		return 1;
	}

	syncSoundSettings();

	if (_musicParser != nullptr) {
		_musicParser->setMidiDriver(_driver);
		_musicParser->setTimerRate(_driver->getBaseTempo());
	}
	for (int i = 0; i < NUM_SFX_PARSERS; i++) {
		if (_sfxParsers[i] != nullptr) {
			_sfxParsers[i]->setMidiDriver(_driver);
			_sfxParsers[i]->setTimerRate(_driver->getBaseTempo());
		}
	}
	_driver->setTimerCallback(this, &onTimer);

	return 0;
}

void MusicPlayer::onTimer(void *data) {
	MusicPlayer *p = (MusicPlayer *)data;

	Common::StackLock lock(p->_mutex);

	if (p->_musicParser != nullptr)
		p->_musicParser->onTimer();
	for (int i = 0; i < NUM_SFX_PARSERS; i++) {
		if (p->_sfxParsers[i] != nullptr) {
			p->_sfxParsers[i]->onTimer();
		}
	}
}

bool MusicPlayer::isPlayingMusic() {
	Common::StackLock lock(_mutex);

	return _musicParser != nullptr && _musicParser->isPlaying();
}

void MusicPlayer::stopMusic() {
	Common::StackLock lock(_mutex);

	if (_musicParser != nullptr) {
		_musicParser->stopPlaying();
		if (_driver != nullptr) {
			_driver->deinitSource(0);
		}
	}
}

void MusicPlayer::pauseMusic(bool pause) {
	Common::StackLock lock(_mutex);

	if (_paused == pause || _musicParser == nullptr)
		return;

	_paused = pause;

	if (_paused) {
		_musicParser->pausePlaying();
	} else {
		_musicParser->resumePlaying();
	}
}

bool MusicPlayer::isPlayingSfx() {
	Common::StackLock lock(_mutex);

	for (int i = 0; i < NUM_SFX_PARSERS; i++) {
		if (_sfxParsers[i] != nullptr && _sfxParsers[i]->isPlaying()) {
			return true;
		}
	}

	return false;
}

bool MusicPlayer::isPlayingSfx(uint8 sfxId) {
	Common::StackLock lock(_mutex);

	for (int i = 0; i < NUM_SFX_PARSERS; i++) {
		if (_sfxParsers[i] != nullptr && _sfxParsers[i]->isPlaying() && _sfxParsers[i]->getActiveTrack() == sfxId) {
			return true;
		}
	}

	return false;
}

void MusicPlayer::playSfx(uint8 sfxId, uint8 priority) {
	Common::StackLock lock(_mutex);

	if (_sfxParsers[0] == nullptr)
		return;

	if (!_tosInstrumentBankLoaded) {
		warning("Attempt to play floppy sound effect %i while TOS instrument bank is not loaded", sfxId);
		return;
	}

	uint8 sfxParserIdx = assignSfxParser();
	if (sfxParserIdx == 0xFF) {
		warning("All SFX parsers in use when trying to play SFX %i", sfxId);
		return;
	}

	if (_driver != nullptr) {
		_driver->resetSourceVolume(sfxParserIdx + 1);
	}
	if (_floppyAdLibDriver != nullptr)
		_floppyAdLibDriver->setSourcePriority(sfxParserIdx + 1, priority);

	_sfxParsers[sfxParserIdx]->setTrack(sfxId);
	_sfxParsers[sfxParserIdx]->startPlaying();
}

uint8 MusicPlayer::assignSfxParser() {
	Common::StackLock lock(_mutex);

	int parserIdx = 0xFF;

	for (int i = 0; i < NUM_SFX_PARSERS; i++) {
		if (_sfxParsers[i] != nullptr && !_sfxParsers[i]->isPlaying()) {
			// Make sure all inactive SFX parsers have released their resources
			if (_driver != nullptr)
				_driver->deinitSource(i + 1);

			if (parserIdx == 0xFF)
				parserIdx = i;
		}
	}
	// If all SFX parsers are already playing, parserIdx is still 0xFF

	return parserIdx;
}

void MusicPlayer::stopAllSfx() {
	Common::StackLock lock(_mutex);

	for (int i = 0; i < NUM_SFX_PARSERS; i++) {
		if (_sfxParsers[i] != nullptr) {
			_sfxParsers[i]->stopPlaying();
			if (_driver != nullptr) {
				_driver->deinitSource(i + 1);
			}
		}
	}
}

bool MusicPlayer::stopSfx(uint8 sfxId) {
	Common::StackLock lock(_mutex);

	bool stoppedSfx = false;

	for (int i = 0; i < NUM_SFX_PARSERS; i++) {
		if (_sfxParsers[i] != nullptr && _sfxParsers[i]->getActiveTrack() == sfxId) {
			_sfxParsers[i]->stopPlaying();
			if (_driver != nullptr) {
				_driver->deinitSource(i + 1);
			}
			stoppedSfx = true;
		}
	}

	return stoppedSfx;
}

void MusicPlayer::syncSoundSettings() {
	if (_driver)
		_driver->syncSoundSettings();
}

void MusicPlayer::setLoopMusic(bool loop) {
	Common::StackLock lock(_mutex);

	if (_musicParser)
		_musicParser->property(MidiParser::mpAutoLoop, loop);
}

void MusicPlayer::load(Common::SeekableReadStream *in, int32 size, bool sfx) {
	Common::StackLock lock(_mutex);

	MidiParser *parser = sfx ? _sfxParsers[0] : _musicParser;
	byte **dataPtr = sfx ? &_sfxData : &_musicData;

	if (parser == nullptr)
		return;

	if (size < 0) {
		// Use the parser to determine the size of the MIDI data.
		int64 startPos = in->pos();
		size = parser->determineDataSize(in);
		if (size < 0) {
			warning("MusicPlayer::load - Could not determine size of music data");
			return;
		}
		// determineDataSize might move the stream position, so return it to
		// the original position.
		in->seek(startPos);
	}

	if (!sfx && isPlayingMusic()) {
		stopMusic();
		_musicParser->unloadMusic();
	}
	if (sfx && isPlayingSfx()) {
		stopAllSfx();
		for (int i = 0; i < NUM_SFX_PARSERS; i++) {
			_sfxParsers[i]->unloadMusic();
		}
	}
	if (*dataPtr != nullptr) {
		delete[] *dataPtr;
	}

	*dataPtr = new byte[size];
	in->read(*dataPtr, size);

	if (!sfx) {
		_musicParser->loadMusic(*dataPtr, size);
	}
	else {
		for (int i = 0; i < NUM_SFX_PARSERS; i++) {
			_sfxParsers[i]->loadMusic(*dataPtr, size);
		}
	}
}

void MusicPlayer::loadTosInstrumentBankData(Common::SeekableReadStream* in, int32 size) {
	if (size != 4096) {
		warning("MusicPlayer::loadTosInstrumentBankData - Specified instrument bank has unexpected size %d", size);
		return;
	}

	if (_tosInstrumentBankData != nullptr)
		delete[] _tosInstrumentBankData;

	_tosInstrumentBankData = new byte[size];
	in->read(_tosInstrumentBankData, size);
}

void MusicPlayer::loadTosInstrumentBank() {
	if (_floppyAdLibDriver == nullptr) {
		warning("MusicPlayer::loadTosInstrumentBank - Driver does not support instrument banks");
		return;
	}
	if (_tosInstrumentBankData == nullptr) {
		warning("MusicPlayer::loadTosInstrumentBank - TOS instrument bank data has not been loaded");
		return;
	}

	if (!_tosInstrumentBankLoaded) {
		if (isPlayingMusic())
			stopMusic();
		_floppyAdLibDriver->loadInstrumentBank(_tosInstrumentBankData);
		_tosInstrumentBankLoaded = true;
	}
}

void MusicPlayer::loadInstrumentBank(Common::SeekableReadStream *in, int32 size) {
	if (_floppyAdLibDriver == nullptr) {
		warning("MusicPlayer::loadInstrumentBank - Driver does not support instrument banks");
		return;
	}
	if (size != 4096) {
		warning("MusicPlayer::loadInstrumentBank - Specified instrument bank has unexpected size %d", size);
		return;
	}

	byte *instrumentBankData = new byte[size];
	in->read(instrumentBankData, size);

	if (isPlayingMusic())
		stopMusic();
	_floppyAdLibDriver->loadInstrumentBank(instrumentBankData);
	_tosInstrumentBankLoaded = false;
}

bool MusicPlayer::isSampleSfx(uint8 sfxId) {
	if (_sfxParserSbr == nullptr)
		return false;
	return _sfxParserSbr->isSampleSfx(sfxId);
}

void MusicPlayer::playMusic(uint8 priority, bool loop) {
	Common::StackLock lock(_mutex);

	if (_musicParser == nullptr || _driver == nullptr)
		return;

	if (!_useFloppyMusic)
		_musicParser->property(MidiParser::mpAutoLoop, loop);
	if (_floppyAdLibDriver != nullptr)
		_floppyAdLibDriver->setSourcePriority(0, priority);

	if (_driver->isFading())
		_driver->abortFade();
	_driver->resetSourceVolume(0);

	_musicParser->startPlaying();
}

void MusicPlayer::startFadeOutMusic() {
	_driver->startFade(0, 1100, 0);
}

bool MusicPlayer::isFadingMusic() {
	return _driver->isFading();
}

} // namespace Darkseed
