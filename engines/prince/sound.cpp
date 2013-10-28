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

 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.

 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.
 *
 */

/*
 * This code is based on original Soltys source code
 * Copyright (c) 1994-1995 Janus B. Wisniewski and L.K. Avalon
 */

#include "prince/prince.h"
#include "prince/sound.h"
#include "common/config-manager.h"
#include "common/memstream.h"
#include "common/archive.h"
#include "audio/decoders/raw.h"
#include "audio/audiostream.h"

namespace Prince {

const char * MusicPlayer::_musTable[] = {
	"",
	"Battlfld.mid",
	"Cave.mid",
	"Cemetery.mid",
	"Credits.mid",
	"Fjord.mid",
	"Guitar.mid",
	"Hell.mid",
	"Jingle.mid",
	"Main.mid",
	"Night.mid",
	"Reality.mid",
	"Sunlord.mid",
	"Tavern.mid",
	"Temple.mid",
	"Boruta.mid",
	"Intro.mid"
};

const uint8 MusicPlayer::_musRoomTable[] = {
	0,
	3,
	9,
	9,
	9,
	13,
	9,
	9 
};


MusicPlayer::MusicPlayer(PrinceEngine *vm) : _vm(vm) {
	_data = NULL;
	_isGM = false;

	MidiPlayer::createDriver();

	int ret = _driver->open();
	if (ret == 0) {
		if (_nativeMT32)
			_driver->sendMT32Reset();
		else
			_driver->sendGMReset();

		// TODO: Load cmf.ins with the instrument table.  It seems that an
		// interface for such an operation is supported for AdLib.	Maybe for
		// this card, setting instruments is necessary.

		_driver->setTimerCallback(this, &timerCallback);
	}
}

MusicPlayer::~MusicPlayer() {
	killMidi();
}

void MusicPlayer::killMidi() {
	Audio::MidiPlayer::stop();

	free(_data);
	_data = NULL;
}

void MusicPlayer::loadMidi(const char * name) {
	Common::SeekableReadStream * stream = SearchMan.createReadStreamForMember(name);
	if (!stream)
		return;

	// Stop any currently playing MIDI file
	killMidi();

	// Read in the data for the file
	_dataSize = stream->size();
	_data = (byte *)malloc(_dataSize);
	stream->read(_data, _dataSize);

	// Start playing the music
	sndMidiStart();
}

void MusicPlayer::sndMidiStart() {
	_isGM = true;

	MidiParser *parser = MidiParser::createParser_SMF();
	if (parser->loadMusic(_data, _dataSize)) {
		parser->setTrack(0);
		parser->setMidiDriver(this);
		parser->setTimerRate(_driver->getBaseTempo());
		parser->property(MidiParser::mpCenterPitchWheelOnUnload, 1);

		_parser = parser;

		syncVolume();

		// Al the tracks are supposed to loop
		_isLooping = true;
		_isPlaying = true;
	}
}

void MusicPlayer::send(uint32 b) {
	if ((b & 0xF0) == 0xC0 && !_isGM && !_nativeMT32) {
		b = (b & 0xFFFF00FF) | MidiDriver::_mt32ToGm[(b >> 8) & 0xFF] << 8;
	}

	Audio::MidiPlayer::send(b);
}

void MusicPlayer::sendToChannel(byte channel, uint32 b) {
	if (!_channelsTable[channel]) {
		_channelsTable[channel] = (channel == 9) ? _driver->getPercussionChannel() : _driver->allocateChannel();
		// If a new channel is allocated during the playback, make sure
		// its volume is correctly initialized.
		if (_channelsTable[channel])
			_channelsTable[channel]->volume(_channelsVolume[channel] * _masterVolume / 255);
	}

	if (_channelsTable[channel])
		_channelsTable[channel]->send(b);
}

} // End of namespace CGE
