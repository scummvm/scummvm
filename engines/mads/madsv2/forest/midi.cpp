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

#include "common/config-manager.h"
#include "common/textconsole.h"
#include "audio/adlib_hmisos.h"
#include "audio/midiparser_hmp.h"
#include "audio/mt32gm.h"
#include "mads/madsv2/forest/midi.h"
#include "mads/madsv2/core/env.h"
#include "mads/madsv2/forest/forest.h"

namespace MADS {
namespace MADSV2 {
namespace Forest {

bool midi_playing;

MidiPlayer::MidiPlayer() {
	_driver = nullptr;
	_deviceType = MT_NULL;
	_parser = nullptr;
	_data = nullptr;
	_paused = false;
}

MidiPlayer::~MidiPlayer() {
	stop();

	if (_driver) {
		_driver->setTimerCallback(nullptr, nullptr);
		_driver->close();
	}

	Common::StackLock lock(_mutex);

	if (_parser) {
		delete _parser;
		_parser = nullptr;
	}
	if (_data) {
		delete[] _data;
		_data = nullptr;
	}
	if (_driver) {
		delete _driver;
		_driver = nullptr;
	}
}

int MidiPlayer::open() {
	// Don't call open() twice!
	assert(!_driver);

	// Check the type of device that the user has configured.
	MidiDriver::DeviceHandle dev = MidiDriver::detectDevice(MDT_ADLIB | MDT_MIDI | MDT_PREFER_GM);
	_deviceType = MidiDriver::getMusicType(dev);
	if (_deviceType == MT_GM && ConfMan.getBool("native_mt32"))
		_deviceType = MT_MT32;

	OPL::Config::OplType oplType;
	switch (_deviceType) {
	case MT_ADLIB:
		oplType = MidiDriver_ADLIB_Multisource::detectOplType(OPL::Config::kOpl3) ? OPL::Config::kOpl3 : OPL::Config::kOpl2;
		MidiDriver_ADLIB_HMISOS *adLibDriver;
		adLibDriver = new MidiDriver_ADLIB_HMISOS(oplType);
		_driver = adLibDriver;

		Common::SeekableReadStream *instrumentBankStream;
		instrumentBankStream = SearchMan.createReadStreamForMember(Common::Path("MELODIC.BNK"));
		Common::SeekableReadStream *rhythmBankStream;
		rhythmBankStream = SearchMan.createReadStreamForMember(Common::Path("DRUM.BNK"));

		adLibDriver->loadInstrumentBanks(instrumentBankStream, rhythmBankStream);

		break;
	case MT_GM:
	case MT_MT32:
		_driver = new MidiDriver_MT32GM(MusicType::MT_GM);
		break;
	default:
		_driver = new MidiDriver_NULL_Multisource();
		break;
	}

	_parser = new MidiParser_HMP(0);
	_driver->property(MidiDriver::PROP_USER_VOLUME_SCALING, true);
	_parser->property(MidiParser::mpDisableAutoStartPlayback, true);

	// Open the MIDI driver.
	const int returnCode = _driver->open();
	if (returnCode != 0)
		error("MidiPlayer::open - Failed to open MIDI music driver - error code %d.", returnCode);

	syncSoundSettings();

	// Connect the driver and the parser.
	_parser->setMidiDriver(_driver);
	_parser->setTimerRate(_driver->getBaseTempo());
	_driver->setTimerCallback(this, &onTimer);

	return 0;
}

void MidiPlayer::load(Common::SeekableReadStream *in, int64 size) {
	Common::StackLock lock(_mutex);

	if (size < 0) {
		// Use the parser to determine the size of the MIDI data.
		int64 startPos = in->pos();
		size = _parser->determineDataSize(in);
		if (size < 0) {
			warning("MidiPlayer::load - Could not determine size of music data");
			return;
		}
		// determineDataSize might move the stream position, so return it to
		// the original position.
		in->seek(startPos);
	}

	_parser->unloadMusic();
	delete[] _data;

	// Copy the music data
	_data = new byte[size];
	in->read(_data, size);

	// Finally, load the data into the parser.
	if (!_parser->loadMusic(_data, size)) {
		warning("MidiPlayer::load - Failed to parse music data");
	}
	_parser->setTrack(0);
}

void MidiPlayer::play() {
	Common::StackLock lock(_mutex);

	_parser->startPlaying();
}

void MidiPlayer::pause(bool pause) {
	if (_paused == pause || !_driver)
		return;

	_paused = pause;

	Common::StackLock lock(_mutex);

	if (_paused) {
		_parser->pausePlaying();
	} else {
		_parser->resumePlaying();
	}
}

void MidiPlayer::stop() {
	Common::StackLock lock(_mutex);

	_parser->stopPlaying();
	if (_driver)
		_driver->deinitSource(0);
}

void MidiPlayer::setLoop(bool loop) {
	Common::StackLock lock(_mutex);

	_parser->property(MidiParser::mpAutoLoop, loop);
}

bool MidiPlayer::isPlaying() {
	Common::StackLock lock(_mutex);

	return _parser->isPlaying();
}

void MidiPlayer::syncSoundSettings() {
	if (_driver)
		_driver->syncSoundSettings();
}

void MidiPlayer::onTimer(void *data) {
	MidiPlayer *p = (MidiPlayer *)data;
	Common::StackLock lock(p->_mutex);

	p->_parser->onTimer();
}

void midi_play(const char *name) {
	// Open up the MIDI file
	Common::SeekableReadStream *f = env_open(name);
	if (!f) {
		warning("MIDI not found - %s", name);
		return;
	}
	g_engine->_midiPlayer.load(f, f->size());
	g_engine->_midiPlayer.play();
	midi_playing = true;
}

void midi_stop() {
	g_engine->_midiPlayer.stop();
	midi_playing = false;
}

void midi_loop() {
	// This is probably what this function is supposed to do.
	// Better to use setLoop to control looping instead of
	// setting global 9 and calling this function repeatedly.
	if (midi_playing && !g_engine->_midiPlayer.isPlaying())
		g_engine->_midiPlayer.play();
}

} // namespace Forest
} // namespace MADSV2
} // namespace MADS
