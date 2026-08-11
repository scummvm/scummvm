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

#include "audio/midiparser.h"
#include "audio/adlib_ctmidi.h"
#include "common/config-manager.h"

#include "tot/midi.h"

namespace Tot {

MidiPlayer::MidiPlayer() {
	_driver = nullptr;

	_paused = false;
	_musicData = nullptr;
	_parserMusic = nullptr;
}

MidiPlayer::~MidiPlayer() {
	stop();

	if (_driver) {
		_driver->setTimerCallback(nullptr, nullptr);
		_driver->close();
	}
	if (_parserMusic)
		delete _parserMusic;
	if (_driver) {
		delete _driver;
		_driver = nullptr;
	}
}
int MidiPlayer::open() {

	// Don't call open() twice!
	assert(!_driver);

	OPL::Config::OplType oplType =
		(MidiDriver_ADLIB_Multisource::detectOplType(OPL::Config::kOpl3) && ConfMan.getBool("opl3_mode")) ?
			OPL::Config::kOpl3 : OPL::Config::kOpl2;

	_driver = new MidiDriver_ADLIB_CTMIDI(oplType);

	_parserMusic = MidiParser::createParser_SMF(0);

	_driver->property(MidiDriver::PROP_USER_VOLUME_SCALING, true);
	_driver->setControllerDefault(MidiDriver_Multisource::CONTROLLER_DEFAULT_PITCH_BEND);
	_driver->setControllerDefault(MidiDriver_Multisource::CONTROLLER_DEFAULT_PANNING);

	int returnCode = _driver->open();
	if (returnCode != 0)
		error("MidiPlayer::open - Failed to open MIDI music driver - error code %d.", returnCode);

	syncSoundSettings();
	// Connect the driver(s) and the parser(s).
	_parserMusic->setMidiDriver(_driver);
	_parserMusic->setTimerRate(_driver->getBaseTempo());
	_driver->setTimerCallback(this, &onTimer);
	return 0;
}

void MidiPlayer::onTimer(void *data) {
	MidiPlayer *p = (MidiPlayer *)data;
	if (p->_parserMusic) {
		p->_parserMusic->onTimer();
	}
}

bool MidiPlayer::isPlaying() {
	return _parserMusic->isPlaying();
}

void MidiPlayer::stop() {
	if (_parserMusic) {
		_parserMusic->stopPlaying();
		if (_driver)
			_driver->deinitSource(0);
	}
}

void MidiPlayer::pause(bool b) {
	if (_paused == b || !_driver)
		return;

	_paused = b;
	if (_paused) {
		if (_parserMusic)
			_parserMusic->pausePlaying();
	} else {
		if (_parserMusic)
			_parserMusic->resumePlaying();
	}
}

void MidiPlayer::syncSoundSettings() {
	if (_driver)
		_driver->syncSoundSettings();
}

void MidiPlayer::setLoop(bool loop) {
	if (_parserMusic)
		_parserMusic->property(MidiParser::mpAutoLoop, loop);
}

void MidiPlayer::setSourceVolume(uint8 volume) {
	if (_driver)
		_driver->setSourceVolume(0, volume);
}

void MidiPlayer::startFadeOut() {
	if (_driver)
		// Note: 40 ms is almost imperceptibly short
		_driver->startFade(40, 0);
}

void MidiPlayer::startFadeIn() {
	if (_driver)
		_driver->startFade(40, 255);
}

bool MidiPlayer::isFading() {
	return _driver ? _driver->isFading() : false;
}

void MidiPlayer::load(Common::SeekableReadStream *in, int32 size) {

	MidiParser *parser = _parserMusic;
	if (!parser)
		return;

	if (size < 0) {
		// Use the parser to determine the size of the MIDI data.
		int64 startPos = in->pos();
		size = parser->determineDataSize(in);
		if (size < 0) {
			warning("MidiPlayer::load - Could not determine size of music data");
			return;
		}
		// determineDataSize might move the stream position, so return it to
		// the original position.
		in->seek(startPos);
	}

	if (isPlaying())
		stop();
	parser->unloadMusic();

	byte **dataPtr = &_musicData;
	if (*dataPtr) {
		delete[] *dataPtr;
	}

	*dataPtr = new byte[size];
	in->read(*dataPtr, size);

	parser->loadMusic(*dataPtr, size);
}

void MidiPlayer::play(int track) {
	MidiParser *parser = _parserMusic;
	if (!parser)
		return;
	if (parser->setTrack(track)) {
		if (_driver)
			// Reset the source volume to neutral (in case the previous track
			// was faded out).
			_driver->resetSourceVolume(0);
		parser->startPlaying();
	} else {
		parser->stopPlaying();
		warning("MidiPlayer::play - Could not play %s track %i", "music", track);
	}
}

} // End of namespace Tot
