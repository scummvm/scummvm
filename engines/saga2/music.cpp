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

// MIDI music class

#include "saga2/saga2.h"
#include "saga2/fta.h"

#include "saga2/music.h"
#include "saga2/hresmgr.h"

#include "audio/mididrv.h"
#include "audio/mididrv_ms.h"
#include "audio/midiparser.h"
#include "audio/miles.h"
#include "common/config-manager.h"
#include "common/file.h"
#include "common/substream.h"

namespace Saga2 {

Music::Music(hResContext *musicRes) : _musicContext(musicRes), _parser(nullptr) {
	static const char *opl2InstDefFilename = "SAMPLE.AD";
	static const char *opl3InstDefFilename = "SAMPLE.OPL";

	// TODO Confirm this
	_musicType = MT_GM;

	MidiDriver::DeviceHandle dev = MidiDriver::detectDevice(MDT_MIDI | MDT_ADLIB | MDT_PREFER_GM);
	_driverType = MidiDriver::getMusicType(dev);
	if (_driverType == MT_GM && ConfMan.getBool("native_mt32"))
		_driverType = MT_MT32;

	switch (_driverType) {
	case MT_ADLIB:
		if (Common::File::exists(opl2InstDefFilename) && Common::File::exists(opl3InstDefFilename)) {
			_driver = (MidiDriver_Multisource *)Audio::MidiDriver_Miles_AdLib_create(opl2InstDefFilename, opl3InstDefFilename);
		} else {
			error("Could not find AdLib instrument definition files %s and %s", opl2InstDefFilename, opl3InstDefFilename);
		}
		break;
	case MT_MT32:
	case MT_GM:
		_driver = Audio::MidiDriver_Miles_MIDI_create(_musicType, "");
		break;
	default:
		_driver = new MidiDriver_NULL_Multisource();
		break;
	}

	if (_driver) {
		_driver->property(MidiDriver::PROP_USER_VOLUME_SCALING, true);
		_driver->property(MidiDriver::PROP_MILES_VERSION, Audio::MILES_VERSION_3);
		if (_driver->open() != 0)
			error("Failed to open MIDI driver.");

		_driver->setTimerCallback(this, &timerCallback);
		_driver->setSourceNeutralVolume(255);
	}

	_currentVolume = 255;
	_currentMusicBuffer = nullptr;

	_trackNumber = 0;

	syncSoundSettings();
}

Music::~Music() {
	if (_parser) {
		_parser->stopPlaying();
		delete _parser;
	}
	if (_driver) {
		_driver->setTimerCallback(nullptr, nullptr);
		_driver->close();
		delete _driver;
	}
}

void Music::setVolume(int volume) {
	if (_driver)
		_driver->setSourceVolume(0, volume);

	_currentVolume = volume;
}

bool Music::isPlaying() {
	return _parser ? _parser->isPlaying() : false;
}

void Music::play(uint32 resourceId, MusicFlags flags) {
	debug(2, "Music::play %d, %d", resourceId, flags);

	if (isPlaying() && _trackNumber == resourceId)
		return;

	_trackNumber = resourceId;
	if (_parser) {
		_parser->unloadMusic();
	} else {
		_parser = MidiParser::createParser_XMIDI(nullptr, nullptr, 0);

		_parser->setMidiDriver(_driver);
		_parser->setTimerRate(_driver->getBaseTempo());
		_parser->property(MidiParser::mpCenterPitchWheelOnUnload, 1);
		_parser->property(MidiParser::mpSendSustainOffOnNotesOff, 1);
	}

	free(_currentMusicBuffer);

	_currentMusicBuffer = (byte *)LoadResource(_musicContext, resourceId, "music data");
	uint32 size = _musicContext->size(resourceId);

	// Handle music looping
	_parser->property(MidiParser::mpAutoLoop, flags & MUSIC_LOOP);
	if (!_parser->loadMusic(_currentMusicBuffer, size))
		error("Music::play() wrong music resource");
}

void Music::pause() {
	if (_parser)
		_parser->pausePlaying();
}

void Music::resume() {
	if (_parser)
		_parser->resumePlaying();
}

void Music::stop() {
	if (_parser)
		_parser->stopPlaying();
}

void Music::syncSoundSettings() {
	if (_driver)
		_driver->syncSoundSettings();
}

void Music::onTimer() {
	if (_parser)
		_parser->onTimer();
}

void Music::timerCallback(void *data) {
	((Music *)data)->onTimer();
}
} // End of namespace Saga
