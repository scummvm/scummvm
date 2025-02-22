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
#include "darkseed/midiparser_sbr.h"

namespace Darkseed {

MusicPlayer::MusicPlayer(DarkseedEngine* vm, bool useFloppyMusic) :
	_vm(vm),
	_driver(nullptr),
	_floppyAdLibDriver(nullptr),
	_paused(false),
	_deviceType(MT_NULL),
	_parser(nullptr),
	_musicData(nullptr),
	_tosInstrumentBankData(nullptr),
	_tosInstrumentBankLoaded(false),
	_useFloppyMusic(useFloppyMusic) {
}

MusicPlayer::~MusicPlayer() {
	stop();
	if (_driver != nullptr) {
		_driver->setTimerCallback(nullptr, nullptr);
		_driver->close();
	}

	Common::StackLock lock(_mutex);

	if (_parser != nullptr)
		delete _parser;
	if (_musicData != nullptr)
		delete[] _musicData;
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

		_parser = new MidiParser_SBR(0);
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
		_parser = MidiParser::createParser_SMF(0);
	}

	_driver->property(MidiDriver::PROP_USER_VOLUME_SCALING, true);
	if (_parser)
		_parser->property(MidiParser::mpDisableAutoStartPlayback, true);

	int returnCode = _driver->open();
	if (returnCode != 0) {
		error("MusicPlayer::open - Failed to open MIDI driver - error code %d.", returnCode);
		return 1;
	}

	syncSoundSettings();

	if (_parser) {
		_parser->setMidiDriver(_driver);
		_parser->setTimerRate(_driver->getBaseTempo());
		_driver->setTimerCallback(_parser, &_parser->timerCallback);
	}

	return 0;
}

void MusicPlayer::onTimer(void *data) {
	MusicPlayer *p = (MusicPlayer *)data;

	Common::StackLock lock(p->_mutex);

	if (p->_parser) {
		p->_parser->onTimer();
	}
}

bool MusicPlayer::isPlaying() {
	Common::StackLock lock(_mutex);

	return _parser && _parser->isPlaying();
}

void MusicPlayer::stop() {
	Common::StackLock lock(_mutex);

	if (_parser) {
		_parser->stopPlaying();
		if (_driver) {
			_driver->deinitSource(0);
		}
	}
}

void MusicPlayer::pause(bool pause) {
	Common::StackLock lock(_mutex);

	if (_paused == pause || !_parser)
		return;

	_paused = pause;

	if (_paused) {
		_parser->pausePlaying();
	} else {
		_parser->resumePlaying();
	}
}

void MusicPlayer::syncSoundSettings() {
	if (_driver)
		_driver->syncSoundSettings();
}

void MusicPlayer::setLoop(bool loop) {
	Common::StackLock lock(_mutex);

	if (_parser)
		_parser->property(MidiParser::mpAutoLoop, loop);
}

void MusicPlayer::load(Common::SeekableReadStream *in, int32 size, bool sfx) {
	Common::StackLock lock(_mutex);

	if (!_parser)
		return;

	if (size < 0) {
		// Use the parser to determine the size of the MIDI data.
		int64 startPos = in->pos();
		size = _parser->determineDataSize(in);
		if (size < 0) {
			warning("MusicPlayer::load - Could not determine size of music data");
			return;
		}
		// determineDataSize might move the stream position, so return it to
		// the original position.
		in->seek(startPos);
	}

	if (isPlaying())
		stop();
	_parser->unloadMusic();
	if (_musicData) {
		delete[] _musicData;
	}

	_musicData = new byte[size];
	in->read(_musicData, size);

	_parser->loadMusic(_musicData, size);
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
		if (isPlaying())
			stop();
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

	if (isPlaying())
		stop();
	_floppyAdLibDriver->loadInstrumentBank(instrumentBankData);
	_tosInstrumentBankLoaded = false;
}

void MusicPlayer::play(uint8 priority, bool loop) {
	Common::StackLock lock(_mutex);

	if (!_parser)
		return;

	if (!_useFloppyMusic)
		_parser->property(MidiParser::mpAutoLoop, loop);
	if (_floppyAdLibDriver != nullptr)
		_floppyAdLibDriver->setSourcePriority(0, priority);

	if (_driver->isFading())
		_driver->abortFade();
	_driver->resetSourceVolume(0);

	_parser->startPlaying();
}

void MusicPlayer::startFadeOut() {
	_driver->startFade(0, 1100, 0);
}

bool MusicPlayer::isFading() {
	return _driver->isFading();
}

} // namespace Darkseed
