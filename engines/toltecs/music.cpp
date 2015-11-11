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

#include "audio/midiparser.h"
#include "audio/miles.h"
#include "common/textconsole.h"

#include "toltecs/toltecs.h"
#include "toltecs/music.h"
#include "toltecs/resource.h"

namespace Toltecs {

MusicPlayer::MusicPlayer(bool isGM) : _isGM(isGM), _buffer(NULL) {
	MidiDriver::DeviceHandle dev = MidiDriver::detectDevice(MDT_MIDI | MDT_ADLIB | MDT_PREFER_GM);
	MusicType musicType = MidiDriver::getMusicType(dev);

	switch (musicType) {
	case MT_ADLIB:
		_milesAudioMode = true;
		_driver = Audio::MidiDriver_Miles_AdLib_create("SAMPLE.AD", "SAMPLE.OPL");
		break;
	case MT_MT32:
		// Not recommended since it sounds awful, but apparently the
		// original sounded just as bad. I guess MT-32 support was
		// added by default, not because anyone actually put any work
		// into it.
		_milesAudioMode = true;
		_driver = Audio::MidiDriver_Miles_MT32_create("");
		break;
	default:
		_milesAudioMode = false;
		MidiPlayer::createDriver();
		break;
	}

	int ret = _driver->open();
	if (ret == 0) {
		if (musicType != MT_ADLIB) {
			if (musicType == MT_MT32 || _nativeMT32)
				_driver->sendMT32Reset();
			else
				_driver->sendGMReset();
		}

		_driver->setTimerCallback(this, &timerCallback);
	}
}

void MusicPlayer::send(uint32 b) {
	if (_milesAudioMode) {
		_driver->send(b);
		return;
	}

	if ((b & 0xF0) == 0xC0 && !_isGM && !_nativeMT32) {
		b = (b & 0xFFFF00FF) | MidiDriver::_mt32ToGm[(b >> 8) & 0xFF] << 8;
	}

	Audio::MidiPlayer::send(b);
}

void MusicPlayer::playMIDI(const byte *data, uint32 size, bool loop) {
	Common::StackLock lock(_mutex);

	stopAndClear();

	_buffer = new byte[size];
	memcpy(_buffer, data, size);

	MidiParser *parser;

	if (!memcmp(data, "FORM", 4))
		parser = MidiParser::createParser_XMIDI(NULL);
	else
		parser = MidiParser::createParser_SMF();

	if (parser->loadMusic(_buffer, size)) {
		parser->setTrack(0);
		parser->setMidiDriver(this);
		parser->setTimerRate(_driver->getBaseTempo());
		parser->property(MidiParser::mpCenterPitchWheelOnUnload, 1);
		parser->property(MidiParser::mpSendSustainOffOnNotesOff, 1);

		_parser = parser;

		syncVolume();

		_isLooping = loop;
		_isPlaying = true;
	} else {
		delete parser;
	}
}

void MusicPlayer::stopAndClear() {
	Common::StackLock lock(_mutex);
	stop();

	delete[] _buffer;
	_buffer = NULL;
}

Music::Music(ArchiveReader *arc) : MusicPlayer(true), _arc(arc) {
	_sequenceResIndex = -1;
}

void Music::playSequence(int16 sequenceResIndex) {
	_sequenceResIndex = sequenceResIndex;

	int32 resourceSize = _arc->getResourceSize(sequenceResIndex);
	byte *data = new byte[resourceSize];
	_arc->openResource(sequenceResIndex);
	_arc->read(data, resourceSize);
	_arc->closeResource();

	if (!memcmp(data, "FORM", 4))
		playMIDI(data, resourceSize, true);	// music tracks are always looping
	else
		// Sanity check: this should never occur
		error("playSequence: resource %d isn't XMIDI", sequenceResIndex);

	delete[] data;
}

void Music::stopSequence() {
	_sequenceResIndex = -1;
	stopAndClear();
}

void Music::saveState(Common::WriteStream *out) {
	out->writeSint16LE(_sequenceResIndex);
}

void Music::loadState(Common::ReadStream *in) {
	_sequenceResIndex = in->readSint16LE();

	if (_sequenceResIndex >= 0)
		playSequence(_sequenceResIndex);
}

} // End of namespace Made
