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

#include "audio/miles.h"
#include "audio/midiparser.h"
#include "common/config-manager.h"
#include "common/file.h"
#include "ultima/ultima3/music.h"
#include "ultima/ultima3/data/savegame.h"

namespace Ultima {
namespace Ultima3 {

static const char *const SONG_NAMES[SONG_COUNT] = {
	"wander", "castles", "rulebrit", "towns", "shopping", "combat", "dungeon", "alive", "fanfare", "exodus"
};

// The instruments for the AdLib are the same as for the OPL3
static const char *const INSTRUMENT_BANK = "sounds/midpak.ad";

void Music::openDriver() {
	_driverOpened = true;

	MidiDriver::DeviceHandle dev = MidiDriver::detectDevice(MDT_MIDI | MDT_ADLIB | MDT_PREFER_GM);
	MusicType type = MidiDriver::getMusicType(dev);
	if (type == MT_GM && ConfMan.getBool("native_mt32"))
		type = MT_MT32;

	switch (type) {
	case MT_ADLIB:
		_miles = true;
		_driver = Audio::MidiDriver_Miles_AdLib_create(INSTRUMENT_BANK, INSTRUMENT_BANK);
		break;
	case MT_MT32:
		_miles = true;
		_driver = Audio::MidiDriver_Miles_MT32_create(Common::Path());
		break;
	default:
		createDriver();
		break;
	}

	if (!_driver)
		return;

	if (_driver->open() != 0) {
		delete _driver;
		_driver = nullptr;
		return;
	}

	// The Miles drivers reset themselves
	if (!_miles) {
		if (_nativeMT32)
			_driver->sendMT32Reset();
		else
			_driver->sendGMReset();
	}

	_driver->setTimerCallback(this, &timerCallback);
}

void Music::send(uint32 b) {
	// The Miles drivers mix the channels themselves
	if (_miles)
		_driver->send(b);
	else
		Audio::MidiPlayer::send(b);
}

void Music::start(int song, bool loop) {
	if (startFile(SONG_NAMES[song], loop))
		_current = song;
}

bool Music::startFile(const Common::String &name, bool loop) {
	Common::StackLock lock(_mutex);
	stop();
	_current = SONG_NONE;

	Common::File f;
	if (!f.open(Common::Path("sounds/" + name + ".xmi")))
		return false;

	const uint32 size = f.size();
	_midiData = (byte *)malloc(size);
	if (f.read(_midiData, size) != size) {
		stop();
		return false;
	}

	_parser = MidiParser::createParser_XMIDI(nullptr, nullptr, 0);
	_parser->setMidiDriver(this);
	_parser->setTimerRate(_driver->getBaseTempo());
	_parser->property(MidiParser::mpCenterPitchWheelOnUnload, 1);
	_parser->property(MidiParser::mpSendSustainOffOnNotesOff, 1);

	if (!_parser->loadMusic(_midiData, size)) {
		stop();
		return false;
	}

	_isLooping = loop;
	_parser->property(MidiParser::mpAutoLoop, loop ? 1 : 0);
	_parser->setTrack(0);

	syncVolume();
	_isPlaying = true;
	return true;
}

bool Music::playFile(const Common::String &name) {
	if (!_driverOpened)
		openDriver();
	if (!_driver)
		return false;

	// Nothing carries on from or after it
	_song = _jingle = SONG_NONE;
	return startFile(name, true);
}

void Music::setEnabled(bool enabled) {
	if (enabled == _enabled)
		return;

	_enabled = enabled;
	if (_enabled)
		resync();
	else
		silence();
}

void Music::silence() {
	_song = _jingle = _current = SONG_NONE;
	stop();
}

void Music::play(int song) {
	if (!_enabled)
		return;

	if (!_driverOpened)
		openDriver();
	if (!_driver)
		return;

	_song = song;

	// A jingle goes on to completion, and then the song takes over
	if (_jingle != SONG_NONE)
		return;

	if (song != _current || !isPlaying())
		start(song, true);
}

void Music::jingle(int song) {
	if (!_enabled)
		return;

	if (!_driverOpened)
		openDriver();
	if (!_driver)
		return;

	_jingle = song;
	start(song, false);
}

void Music::update() {
	if (!_driver || isPlaying())
		return;

	// A jingle has finished, or a song that doesn't loop has
	_jingle = SONG_NONE;
	if (_song != SONG_NONE)
		start(_song, true);
}

void Music::autoSelect(byte location, bool inExodus) {
	int song;

	switch (location) {
	case Data::LOCATION_SOSARIA:
		song = SONG_WANDER;
		break;
	case Data::LOCATION_DUNGEON:
		song = SONG_DUNGEON;
		break;
	case Data::LOCATION_TOWN:
		song = SONG_TOWNS;
		break;
	case Data::LOCATION_CASTLE:
		song = inExodus ? SONG_EXODUS : SONG_CASTLES;
		break;
	case MUSIC_LOCATION_COMBAT:
		song = SONG_COMBAT;
		break;
	case MUSIC_LOCATION_SCENE:
	case MUSIC_LOCATION_GEM:
		song = SONG_ALIVE;
		break;
	case Data::LOCATION_AMBROSIA:
		song = SONG_FANFARE;
		break;
	default:
		return;
	}

	if (song == _selected)
		return;

	_selected = song;
	play(song);
}

} // namespace Ultima3
} // namespace Ultima
