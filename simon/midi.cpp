/* ScummVM - Scumm Interpreter
 * Copyright (C) 2001-2003 The ScummVM project
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
 * Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.
 *
 * $Header$
 *
 */


#include "stdafx.h"
#include "scummsys.h"
#include "common/system.h"
#include "common/file.h"
#include "sound/mixer.h"
#include "simon/simon.h"

MidiPlayer::MidiPlayer (OSystem *system) {
	// Since initialize() is called every time the music changes,
	// this is where we'll initialize stuff that must persist
	// between songs.
	_system = system;
	_mutex = system->create_mutex();
	_data = 0;
	_masterVolume = 255;
	_num_songs = 0;
	_currentTrack = 255;
}

MidiPlayer::~MidiPlayer() {
	_system->lock_mutex (_mutex);
	close();
	_system->unlock_mutex (_mutex);
	_system->delete_mutex (_mutex);
}

int MidiPlayer::open() {
	// Don't ever call open without first setting the output driver!
	if (!_driver)
		return 255;

	int ret = _driver->open();
	if (ret)
		return ret;
	_driver->setTimerCallback (this, &onTimer);
	return 0;
}

void MidiPlayer::close() {
	_system->lock_mutex (_mutex);
	stop();
	if (_driver)
		_driver->close();
	_driver = NULL;
	clearConstructs();
	_system->unlock_mutex (_mutex);
}

void MidiPlayer::send (uint32 b) {
	byte volume;

	// Only thing we care about is volume control changes.
	if ((b & 0xFFF0) == 0x07B0) {
		volume = (byte) ((b >> 16) & 0xFF) * _masterVolume / 255;
		_volumeTable [b & 0xF] = volume;
		b = (b & 0xFF00FFFF) | (volume << 16);
	}

	_driver->send (b);
}

void MidiPlayer::metaEvent (byte type, byte *data, uint16 length) {
	// Only thing we care about is End of Track.
	if (type != 0x2F)
		return;

	_parser->jumpToTick (0);
}

void MidiPlayer::onTimer (void *data) {
	MidiPlayer *player = (MidiPlayer *) data;
	if (player->_paused)
		return;
	if (player->_parser) {
		player->_system->lock_mutex (player->_mutex);
		player->_parser->onTimer();
		player->_system->unlock_mutex (player->_mutex);
	}
}

void MidiPlayer::jump (uint16 track, uint16 tick) {
	if (track == _currentTrack)
		return;

	if (_num_songs > 0) {
		if (track >= _num_songs)
			return;

		_system->lock_mutex (_mutex);

		if (_parser) {
			delete _parser;
			_parser = 0;
		}

		MidiParser *parser = MidiParser::createParser_SMF();
		parser->setMidiDriver (this);
		parser->setTimerRate (_driver->getBaseTempo());
		if (!parser->loadMusic (_songs[track], _song_sizes[track])) {
			printf ("Error reading track!\n");
			delete parser;
			parser = 0;
		}

		_currentTrack = (byte) track;
		for (int i = ARRAYSIZE (_volumeTable); i; --i)
			_volumeTable[i-1] = 127;
		_parser = parser; // That plugs the power cord into the wall

		_system->unlock_mutex (_mutex);
	} else if (_parser) {
		_system->lock_mutex (_mutex);
		_currentTrack = (byte) track;
		_parser->setTrack ((byte) track);
		_parser->jumpToTick (tick - 1);
		_system->unlock_mutex (_mutex);
	}
}

void MidiPlayer::stop() {
	_system->lock_mutex (_mutex);
	if (_parser)
		_parser->unloadMusic();
	_currentTrack = 255;
	_system->unlock_mutex (_mutex);
}

void MidiPlayer::pause (bool b) {
	if (_paused == b)
		return;
	_paused = b;

	_system->lock_mutex (_mutex);
	for (int i = ARRAYSIZE (_volumeTable); i; --i)
		_driver->send (((_paused ? 0 : (_volumeTable[i-1] * _masterVolume / 255)) << 16) | (7 << 8) | 0xB0 | i);
	_system->unlock_mutex (_mutex);
}

void MidiPlayer::set_volume (int volume) {
	if (volume < 0)
		volume = 0;
	else if (volume > 255)
		volume = 255;

	if (_masterVolume == volume)
		return;

	_masterVolume = volume;

	// Now tell all the channels this.
	_system->lock_mutex (_mutex);
	if (_driver && !_paused) {
		for (int i = ARRAYSIZE (_volumeTable); i; --i) {
			_driver->send (((_volumeTable[i-1] * _masterVolume / 255) << 16) | (7 << 8) | 0xB0 | i);
		}
	}
	_system->unlock_mutex (_mutex);
}

void MidiPlayer::set_driver(MidiDriver *md) {
	// Don't try to set this more than once.
	if (_driver)
		return;
	_driver = md;
}

void MidiPlayer::clearConstructs() {
	if (_num_songs > 0) {
		byte i;
		for (i = 0; i < _num_songs; ++i) {
			free (_songs [i]);
		}
		_num_songs = 0;
	}

	if (_data) {
		free (_data);
		_data = 0;
	}

	if (_parser) {
		delete _parser;
		_parser = 0;
	}
}

void MidiPlayer::playSMF (File *in) {
	_system->lock_mutex (_mutex);
	clearConstructs();
	uint32 size = in->size() - in->pos();
	if (size > 64000)
		size = 64000;
	_data = (byte *) calloc (size, 1);
	in->read (_data, size);

	MidiParser *parser = MidiParser::createParser_SMF();
	parser->setMidiDriver (this);
	parser->setTimerRate (_driver->getBaseTempo());
	if (!parser->loadMusic (_data, size)) {
		printf ("Error reading track!\n");
		delete parser;
		parser = 0;
	}

	_currentTrack = 0;
	for (int i = ARRAYSIZE (_volumeTable); i; --i)
		_volumeTable[i-1] = 127;
	_parser = parser; // That plugs the power cord into the wall
	_system->unlock_mutex (_mutex);
}

void MidiPlayer::playMultipleSMF (File *in) {
	// This is a special case for Simon 2 Windows.
	// Instead of having multiple sequences as
	// separate tracks in a Type 2 file, simon2win
	// has multiple songs, each of which is a Type 1
	// file. Thus, preceding the songs is a single
	// byte specifying how many songs are coming.
	// We need to load ALL the songs and then
	// treat them as separate tracks -- for the
	// purpose of jumps, anyway.
	_system->lock_mutex (_mutex);
	clearConstructs();
	_num_songs = in->readByte();
	if (_num_songs > 16) {
		printf ("playMultipleSMF: %d is too many songs to keep track of!\n", (int) _num_songs);
		_system->unlock_mutex (_mutex);
		return;
	}

	byte i;
	for (i = 0; i < _num_songs; ++i) {
		byte buf[5];
		uint32 pos = in->pos();

		// Make sure there's a MThd
		in->read (buf, 4);
		if (memcmp (buf, "MThd", 4)) {
			printf ("Expected MThd but found '%c%c%c%c' instead!\n", buf[0], buf[1], buf[2], buf[3]);
			_system->unlock_mutex (_mutex);
			return;
		}
		in->seek (in->readUint32BE() + in->pos(), SEEK_SET);

		// Now skip all the MTrk blocks
		while (true) {
			in->read (buf, 4);
			if (memcmp (buf, "MTrk", 4))
				break;
			in->seek (in->readUint32BE() + in->pos(), SEEK_SET);
		}

		uint32 pos2 = in->pos() - 4;
		uint32 size = pos2 - pos;
		_songs[i] = (byte *) calloc (size, 1);
		in->seek (pos, SEEK_SET);
		in->read (_songs[i], size);
		_song_sizes[i] = size;
	}

	_system->unlock_mutex (_mutex);
	jump (0, 1);
}

void MidiPlayer::playXMIDI (File *in) {
	_system->lock_mutex (_mutex);
	clearConstructs();

	char buf[4];
	uint32 pos = in->pos();
	uint32 size = 4;
	in->read (buf, 4);
	if (!memcmp (buf, "FORM", 4)) {
		while (memcmp (buf, "CAT ", 4)) { size += 4; in->read (buf, 4); }
		size += 4 + in->readUint32BE();
		in->seek (pos, 0);
		_data = (byte *) calloc (size, 1);
		in->read (_data, size);
	} else {
		printf ("ERROR! Expected 'FORM' tag but found '%c%c%c%c' instead!\n", buf[0], buf[1], buf[2], buf[3]);
		_system->unlock_mutex (_mutex);
		return;
	}

	MidiParser *parser = MidiParser::createParser_XMIDI();
	parser->setMidiDriver (this);
	parser->setTimerRate (_driver->getBaseTempo());
	if (!parser->loadMusic (_data, size)) {
		printf ("Error reading track!\n");
		delete parser;
		parser = 0;
	}

	_currentTrack = 0;
	for (int i = ARRAYSIZE (_volumeTable); i; --i)
		_volumeTable[i-1] = 127;
	_parser = parser; // That plugs the power cord into the wall
	_system->unlock_mutex (_mutex);
}
