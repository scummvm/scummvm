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
#include "common/scummsys.h"
#include "common/system.h"
#include "common/file.h"
#include "sound/mixer.h"
#include "simon/simon.h"

namespace Simon {


// MidiParser_S1D is not considered part of the standard
// MidiParser suite, but we still try to mask its details
// and just provide a factory function.
extern MidiParser *MidiParser_createS1D();


// Instrument mapping for MT32 tracks emulated under GM.
static const byte mt32_to_gm[128] = {
//    0    1    2    3    4    5    6    7    8    9    A    B    C    D    E    F
	  0,   1,   0,   2,   4,   4,   5,   3,  16,  17,  18,  16,  16,  19,  20,  21, // 0x
	  6,   6,   6,   7,   7,   7,   8, 112,  62,  62,  63,  63,  38,  38,  39,  39, // 1x
	 88,  95,  52,  98,  97,  99,  14,  54, 102,  96,  53, 102,  81, 100,  14,  80, // 2x
	 48,  48,  49,  45,  41,  40,  42,  42,  43,  46,  45,  24,  25,  28,  27, 104, // 3x
	 32,  32,  34,  33,  36,  37,  35,  35,  79,  73,  72,  72,  74,  75,  64,  65, // 4x
	 66,  67,  71,  71,  68,  69,  70,  22,  56,  59,  57,  57,  60,  60,  58,  61, // 5x
	 61,  11,  11,  98,  14,   9,  14,  13,  12, 107, 107,  77,  78,  78,  76,  76, // 6x
	 47, 117, 127, 118, 118, 116, 115, 119, 115, 112,  55, 124, 123,   0,  14, 117  // 7x
};



MidiPlayer::MidiPlayer(OSystem *system) {
	// Since initialize() is called every time the music changes,
	// this is where we'll initialize stuff that must persist
	// between songs.
	_system = system;
	_mutex = system->create_mutex();
	_driver = 0;
	_map_mt32_to_gm = false;
	
	_enable_sfx = true;
	_current = 0;

	_masterVolume = 255;
	resetVolumeTable();
	_paused = false;

	_currentTrack = 255;
	_loopTrack = 0;
	_queuedTrack = 255;
	_loopQueuedTrack = 0;
}

MidiPlayer::~MidiPlayer() {
	_system->lock_mutex(_mutex);
	close();
	_system->unlock_mutex(_mutex);
	_system->delete_mutex(_mutex);
}

int MidiPlayer::open() {
	// Don't ever call open without first setting the output driver!
	if (!_driver)
		return 255;

	int ret = _driver->open();
	if (ret)
		return ret;
	_driver->setTimerCallback(this, &onTimer);
	return 0;
}

void MidiPlayer::close() {
	stop();
//	_system->lock_mutex (_mutex);
	if (_driver)
		_driver->close();
	_driver = NULL;
	clearConstructs();
//	_system->unlock_mutex (_mutex);
}

void MidiPlayer::send(uint32 b) {
	if (!_current)
		return;

	byte channel = (byte) (b & 0x0F);
	if ((b & 0xFFF0) == 0x07B0) {
		// Adjust volume changes by master volume.
		byte volume = (byte) ((b >> 16) & 0x7F);
		_current->volume[channel] = volume;
		volume = volume * _masterVolume / 255;
		b = (b & 0xFF00FFFF) | (volume << 16);
	} else if ((b & 0xF0) == 0xC0 && _map_mt32_to_gm) {
		b = (b & 0xFFFF00FF) | (mt32_to_gm[(b >> 8) & 0xFF] << 8);
	} else if ((b & 0xFFF0) == 0x007BB0) {
		// Only respond to an All Notes Off if this channel
		// has already been allocated.
		if (!_current->channel[b & 0x0F])
			return;
	}

	if (!_current->channel[channel])
		_current->channel[channel] = (channel == 9) ? _driver->getPercussionChannel() : _driver->allocateChannel();
	if (_current->channel[channel])
		_current->channel[channel]->send(b);
}

void MidiPlayer::metaEvent (byte type, byte *data, uint16 length) {
	// Only thing we care about is End of Track.
	if (!_current || type != 0x2F) {
		return;
	} else if (_current == &_sfx) {
		clearConstructs(_sfx);
	} else if (_loopTrack) {
		_current->parser->jumpToTick(0);
	} else if (_queuedTrack != 255) {
		_currentTrack = 255;
		byte destination = _queuedTrack;
		_queuedTrack = 255;
		_loopTrack = _loopQueuedTrack;
		_loopQueuedTrack = false;

		// Remember, we're still inside the locked mutex.
		// Have to unlock it before calling jump()
		// (which locks it itself), and then relock it
		// upon returning.
		_system->unlock_mutex(_mutex);
		startTrack (destination);
		_system->lock_mutex(_mutex);
	} else {
		stop();
	}
}

void MidiPlayer::onTimer (void *data) {
	MidiPlayer *p = (MidiPlayer *) data;
	p->_system->lock_mutex(p->_mutex);
	if (!p->_paused) {
		if (p->_music.parser && p->_currentTrack != 255) {
			p->_current = &p->_music;
			p->_music.parser->onTimer();
		}
	}
	if (p->_sfx.parser) {
		p->_current = &p->_sfx;
		p->_sfx.parser->onTimer();
	}
	p->_current = 0;
	p->_system->unlock_mutex(p->_mutex);
}

void MidiPlayer::startTrack (int track) {
	if (track == _currentTrack)
		return;

	if (_music.num_songs > 0) {
		if (track >= _music.num_songs)
			return;

		_system->lock_mutex(_mutex);

		if (_music.parser) {
			_current = &_music;
			delete _music.parser;
			_current = 0;
			_music.parser = 0;
		}

		MidiParser *parser = MidiParser::createParser_SMF();
		parser->property (MidiParser::mpMalformedPitchBends, 1);
		parser->setMidiDriver (this);
		parser->setTimerRate (_driver->getBaseTempo());
		if (!parser->loadMusic (_music.songs[track], _music.song_sizes[track])) {
			printf ("Error reading track!\n");
			delete parser;
			parser = 0;
		}

		_currentTrack = (byte) track;
		_music.parser = parser; // That plugs the power cord into the wall
	} else if (_music.parser) {
		_system->lock_mutex(_mutex);
		if (!_music.parser->setTrack(track)) {
			_system->unlock_mutex(_mutex);
			return;
		}
		_currentTrack = (byte) track;
		_current = &_music;
		_music.parser->jumpToTick(0);
		_current = 0;
	}

	_system->unlock_mutex(_mutex);
}

void MidiPlayer::stop() {
	_system->lock_mutex(_mutex);
	if (_music.parser) {
		_current = &_music;
		_music.parser->jumpToTick(0);
	}
	_current = 0;
	_currentTrack = 255;
	_system->unlock_mutex(_mutex);
}

void MidiPlayer::pause (bool b) {
	if (_paused == b || !_driver)
		return;
	_paused = b;

	_system->lock_mutex(_mutex);
	for (int i = 0; i < 16; ++i) {
		if (_music.channel[i])
			_music.channel[i]->volume (_paused ? 0 : (_music.volume[i] * _masterVolume / 255));
		if (_sfx.channel[i])
			_sfx.channel[i]->volume (_paused ? 0 : (_sfx.volume[i] * _masterVolume / 255));
	}
	_system->unlock_mutex(_mutex);
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
		for (int i = 0; i < 16; ++i) {
			if (_music.channel[i])
				_music.channel[i]->volume(_music.volume[i] * _masterVolume / 255);
			if (_sfx.channel[i])
				_sfx.channel[i]->volume(_sfx.volume[i] * _masterVolume / 255);
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

void MidiPlayer::mapMT32toGM (bool map) {
	_system->lock_mutex(_mutex);
	_map_mt32_to_gm = map;
	_system->unlock_mutex(_mutex);
}

void MidiPlayer::setLoop (bool loop) {
	_system->lock_mutex(_mutex);
	_loopTrack = loop;
	_system->unlock_mutex(_mutex);
}

void MidiPlayer::queueTrack (int track, bool loop) {
	_system->lock_mutex(_mutex);
	if (_currentTrack == 255) {
		_system->unlock_mutex(_mutex);
		setLoop(loop);
		startTrack(track);
	} else {
		_queuedTrack = track;
		_loopQueuedTrack = loop;
		_system->unlock_mutex(_mutex);
	}
}

void MidiPlayer::clearConstructs() {
	clearConstructs(_music);
	clearConstructs(_sfx);
}

void MidiPlayer::clearConstructs(MusicInfo &info) {
	int i;
	if (info.num_songs > 0) {
		for (i = 0; i < info.num_songs; ++i)
			free (info.songs[i]);
		info.num_songs = 0;
	}

	if (info.data) {
		free(info.data);
		info.data = 0;
	} // end if

	if (info.parser) {
		delete info.parser;
		info.parser = 0;
	}

	if (_driver) {
		for (i = 0; i < 16; ++i) {
			if (info.channel[i]) {
				info.channel[i]->allNotesOff();
				info.channel[i]->release();
			}
		}
	}
	info.clear();
}

void MidiPlayer::resetVolumeTable() {
	int i;
	for (i = 0; i < 16; ++i) {
		_music.volume[i] = _sfx.volume[i] = 127;
		if (_driver)
			_driver->send (((_masterVolume >> 1) << 16) | 0x7B0 | i);
	}
}

static int simon1_gmf_size[] = {
	8900, 12166, 2848, 3442, 4034, 4508, 7064, 9730, 6014, 4742, 3138,
	6570, 5384, 8909, 6457, 16321, 2742, 8968, 4804, 8442, 7717,
	9444, 5800, 1381, 5660, 6684, 2456, 4744, 2455, 1177, 1232,
	17256, 5103, 8794, 4884, 16
};

void MidiPlayer::loadSMF (File *in, int song, bool sfx) {
	_system->lock_mutex(_mutex);
	MusicInfo *p = sfx ? &_sfx : &_music;
	clearConstructs (*p);

	uint32 startpos = in->pos();
	byte header[4];
	in->read(header, 4);
	bool isGMF = !memcmp (header, "GMF\x1", 4);
	in->seek(startpos, SEEK_SET);

	uint32 size = in->size() - in->pos();
	if (isGMF) {
		if (sfx) {
			// Multiple GMF resources are stored in the SFX files,
			// but each one is referenced by a pointer at the
			// beginning of the file. Those pointers can be used
			// to determine file size.
			in->seek(0, SEEK_SET);
			uint16 value = in->readUint16LE() >> 2; // Number of resources
			if (song != value - 1) {
				in->seek(song * 2 + 2, SEEK_SET);
				value = in->readUint16LE();
				size = value - startpos;
			}
			in->seek(startpos, SEEK_SET);
		} else if (size >= 64000) {
			// For GMF resources not in separate
			// files, we're going to have to use
			// hardcoded size tables.
			size = simon1_gmf_size[song];
		}
	}

	// When allocating space, add 4 bytes in case
	// this is a GMF and we have to tack on our own
	// End of Track event.
	p->data = (byte *) calloc(size + 4, 1);
	in->read(p->data, size);

	if (!memcmp(p->data, "GMF\x1", 4)) {
		// BTW, here's what we know about the GMF header,
		// the 7 bytes preceding the actual MIDI events.
		// 3 BYTES: 'GMF'
		// 1 BYTE : Always seems to be 0x01
		// 1 BYTE : Always seems to be 0x00
		// 1 BYTE : Ranges from 0x02 to 0x08 (always 0x02 for SFX, though)
		// 1 BYTE : Loop control. 0 = no loop, 1 = loop
		if (!sfx)
			setLoop(p->data[6] != 0);

	}

	MidiParser *parser = MidiParser::createParser_SMF();
	parser->property(MidiParser::mpMalformedPitchBends, 1);
	parser->setMidiDriver(this);
	parser->setTimerRate(_driver->getBaseTempo());
	if (!parser->loadMusic(p->data, size)) {
		printf ("Error reading track!\n");
		delete parser;
		parser = 0;
	}

	if (!sfx) {
		_currentTrack = 255;
		resetVolumeTable();
	}
	p->parser = parser; // That plugs the power cord into the wall
	_system->unlock_mutex(_mutex);
}

void MidiPlayer::loadMultipleSMF (File *in, bool sfx) {
	// This is a special case for Simon 2 Windows.
	// Instead of having multiple sequences as
	// separate tracks in a Type 2 file, simon2win
	// has multiple songs, each of which is a Type 1
	// file. Thus, preceding the songs is a single
	// byte specifying how many songs are coming.
	// We need to load ALL the songs and then
	// treat them as separate tracks -- for the
	// purpose of jumps, anyway.
	_system->lock_mutex(_mutex);
	MusicInfo *p = sfx ? &_sfx : &_music;
	clearConstructs(*p);

	p->num_songs = in->readByte();
	if (p->num_songs > 16) {
		printf ("playMultipleSMF: %d is too many songs to keep track of!\n", (int) p->num_songs);
		_system->unlock_mutex(_mutex);
		return;
	}

	byte i;
	for (i = 0; i < p->num_songs; ++i) {
		byte buf[5];
		uint32 pos = in->pos();

		// Make sure there's a MThd
		in->read(buf, 4);
		if (memcmp (buf, "MThd", 4)) {
			printf ("Expected MThd but found '%c%c%c%c' instead!\n", buf[0], buf[1], buf[2], buf[3]);
			_system->unlock_mutex (_mutex);
			return;
		}
		in->seek(in->readUint32BE() + in->pos(), SEEK_SET);

		// Now skip all the MTrk blocks
		while (true) {
			in->read (buf, 4);
			if (memcmp(buf, "MTrk", 4))
				break;
			in->seek(in->readUint32BE() + in->pos(), SEEK_SET);
		}

		uint32 pos2 = in->pos() - 4;
		uint32 size = pos2 - pos;
		p->songs[i] = (byte *) calloc(size, 1);
		in->seek(pos, SEEK_SET);
		in->read(p->songs[i], size);
		p->song_sizes[i] = size;
	}

	if (!sfx) {
		_currentTrack = 255;
		resetVolumeTable();
	}
	_system->unlock_mutex(_mutex);
}

void MidiPlayer::loadXMIDI(File *in, bool sfx) {
	_system->lock_mutex(_mutex);
	MusicInfo *p = sfx ? &_sfx : &_music;
	clearConstructs(*p);

	char buf[4];
	uint32 pos = in->pos();
	uint32 size = 4;
	in->read(buf, 4);
	if (!memcmp(buf, "FORM", 4)) {
		int i;
		for (i = 0; i < 16; ++i) {
			if (!memcmp(buf, "CAT ", 4))
				break;
			size += 2;
			memcpy(buf, &buf[2], 2);
			in->read(&buf[2], 2);
		}
		if (memcmp(buf, "CAT ", 4)) {
			warning("Could not find 'CAT ' tag to determine resource size!");
			_system->unlock_mutex (_mutex);
			return;
		}
		size += 4 + in->readUint32BE();
		in->seek(pos, 0);
		p->data = (byte *) calloc(size, 1);
		in->read(p->data, size);
	} else {
		warning("Expected 'FORM' tag but found '%c%c%c%c' instead!", buf[0], buf[1], buf[2], buf[3]);
		_system->unlock_mutex(_mutex);
		return;
	}

	MidiParser *parser = MidiParser::createParser_XMIDI();
	parser->setMidiDriver(this);
	parser->setTimerRate(_driver->getBaseTempo());
	if (!parser->loadMusic(p->data, size)) {
		warning("Error reading track!");
		delete parser;
		parser = 0;
	}

	if (!sfx) {
		_currentTrack = 255;
		resetVolumeTable();
	}
	p->parser = parser; // That plugs the power cord into the wall
	_system->unlock_mutex(_mutex);
}

void MidiPlayer::loadS1D (File *in, bool sfx) {
	_system->lock_mutex(_mutex);
	MusicInfo *p = sfx ? &_sfx : &_music;
	clearConstructs(*p);

	uint16 size = in->readUint16LE();
	if (size != in->size() - 2) {
		warning("Size mismatch in simon1demo MUS file (%ld versus reported %d)", (long) in->size() - 2, (int) size);
		_system->unlock_mutex(_mutex);
		return;
	}

	p->data = (byte *) calloc(size, 1);
	in->read(p->data, size);

	MidiParser *parser = MidiParser_createS1D();
	parser->setMidiDriver(this);
	parser->setTimerRate(_driver->getBaseTempo());
	if (!parser->loadMusic(p->data, size)) {
		warning("Error reading track!");
		delete parser;
		parser = 0;
	}

	if (!sfx) {
		_currentTrack = 255;
		resetVolumeTable();
	}
	p->parser = parser; // That plugs the power cord into the wall
	_system->unlock_mutex(_mutex);
}

} // End of namespace Simon
