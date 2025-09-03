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
#include "common/file.h"

#include "tot/midi.h"

namespace Tot {

MidiPlayer::MidiPlayer() {
	_driver = nullptr;
	_driverMsMusic = nullptr;

	_paused = false;
	_musicData = nullptr;
	_parserMusic = nullptr;
}

MidiPlayer::~MidiPlayer() {
	stop();

	if (_driverMsMusic) {
		_driverMsMusic->setTimerCallback(nullptr, nullptr);
		_driverMsMusic->close();
	} else if (_driver) {
		_driver->setTimerCallback(nullptr, nullptr);
		_driver->close();
	}
	if (_parserMusic)
		delete _parserMusic;
	if (_driverMsMusic) {
		delete _driverMsMusic;
		_driverMsMusic = nullptr;
	} else if (_driver) {
		delete _driver;
		_driver = nullptr;
	}
}
int MidiPlayer::open() {

	// Don't call open() twice!
	assert(!_driver);

	OPL::Config::OplType oplType =
		MidiDriver_ADLIB_Multisource::detectOplType(OPL::Config::kOpl3) ? OPL::Config::kOpl3 : OPL::Config::kOpl2;

	_driverMsMusic = new MidiDriver_AdLib(oplType);

	_parserMusic = MidiParser::createParser_SMF();
	_driver = _driverMsMusic;

	if (_driverMsMusic)
		_driverMsMusic->property(MidiDriver::PROP_USER_VOLUME_SCALING, true);
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
		if (_driverMsMusic)
			_driverMsMusic->deinitSource(0);
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
	if (_driverMsMusic)
		_driverMsMusic->syncSoundSettings();
}
void MidiPlayer::setLoop(bool loop) {
	if (_parserMusic)
		_parserMusic->property(MidiParser::mpAutoLoop, loop);
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
		if (_driverMsMusic)
			// Reset the volume to neutral (in case the previous track was
			// faded out).
			_driverMsMusic->resetSourceVolume(0);
		parser->startPlaying();
	} else {
		parser->stopPlaying();
		warning("MidiPlayer::play - Could not play %s track %i", "music", track);
	}
}

MidiDriver_AdLib::MidiDriver_AdLib(OPL::Config::OplType oplType, int timerFrequency) : MidiDriver_ADLIB_Multisource::MidiDriver_ADLIB_Multisource(oplType, timerFrequency) {
	_dsfInstrumentBank = new OplInstrumentDefinition[128];
	loadInstrumentBankFromDriver(11048);
}

MidiDriver_AdLib::~MidiDriver_AdLib() {
	delete[] _dsfInstrumentBank;
}

void MidiDriver_AdLib::loadInstrumentBankFromDriver(int32 offset) {
	Common::File driverFile;
	if (!driverFile.open("CTMIDI.DRV")) {
		error("Couldnt find midi file!");
	}
	driverFile.seek(offset, SEEK_SET);
	uint8 *data = (uint8 *)malloc(128 * (11 + 21));
	driverFile.read(data, 128 * (11 + 21));
	loadInstrumentBank(data);
	driverFile.close();

	_instrumentBank = _dsfInstrumentBank;
	_rhythmBank = _dsfInstrumentBank;
}

void MidiDriver_AdLib::loadInstrumentBank(uint8 *instrumentBankData) {
	for (int i = 0; i < 128; i++) {
		AdLibIbkInstrumentDefinition instrument;

		instrument.o0FreqMultMisc = *instrumentBankData++;
		instrument.o1FreqMultMisc = *instrumentBankData++;

		instrument.o0Level = *instrumentBankData++;
		instrument.o1Level = *instrumentBankData++;

		instrument.o0DecayAttack = *instrumentBankData++;
		instrument.o1DecayAttack = *instrumentBankData++;

		instrument.o0ReleaseSustain = *instrumentBankData++;
		instrument.o1ReleaseSustain = *instrumentBankData++;

		instrument.o0WaveformSelect = *instrumentBankData++;
		instrument.o1WaveformSelect = *instrumentBankData++;

		instrument.connectionFeedback = *instrumentBankData++;

		instrument.rhythmType = *instrumentBankData++;
		instrument.transpose = *instrumentBankData++;
		instrument.rhythmNote = *instrumentBankData++;
		instrument.padding1 = *instrumentBankData++;
		instrument.padding2 = *instrumentBankData++;
		instrument.toOplInstrumentDefinition(_dsfInstrumentBank[i]);
	}
}

} // End of namespace Tot
