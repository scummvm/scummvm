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

#include "common/config-manager.h"
#include "sherlock/sherlock.h"
#include "sherlock/music.h"

namespace Sherlock {

#define NUM_SONGS 45

/* This tells which song to play in each room, 0 = no song played */
static const char ROOM_SONG[62] = {
	 0, 20, 43,  6, 11,  2,  8, 15,  6, 28,
	 6, 38,  7, 32, 16,  5,  8, 41,  9, 22,
	10, 23,  4, 39, 19, 24, 13, 27,  0, 30,
	 3, 21, 26, 25, 16, 29,  1,  1, 18, 12,
	 1, 17, 17, 31, 17, 34, 36,  7, 20, 20,
	33,  8, 44, 40, 42, 35,  0,  0,  0, 12,
	12
};

static const char *const SONG_NAMES[NUM_SONGS] = {
	"SINGERF",  "CHEMIST",  "TOBAC",   "EQUEST",  "MORTUARY", "DOCKS",    "LSTUDY",
	"LORD",     "BOY",      "PERFUM1", "BAKER1",  "BAKER2",   "OPERA1",   "HOLMES",
	"FFLAT",    "OP1FLAT",  "ZOO",     "SROOM",   "FLOWERS",  "YARD",     "TAXID",
	"PUB1",     "VICTIM",   "RUGBY",   "DORM",    "SHERMAN",  "LAWYER",   "THEATRE",
	"DETECT",   "OPERA4",   "POOL",    "SOOTH",   "ANNA1",    "ANNA2",    "PROLOG3",
	"PAWNSHOP", "MUSICBOX", "MOZART1", "ROBHUNT", "PANCRAS1", "PANCRAS2", "LORDKILL",
	"BLACKWEL", "RESCUE",   "MAP"
};

MidiParser_SH::MidiParser_SH() {
	_ppqn = 1;
	setTempo(16667);
	_data = nullptr;
	_beats = 0;
	_lastEvent = 0;
	_trackEnd = nullptr;
}

void MidiParser_SH::parseNextEvent(EventInfo &info) {
	warning("parseNextEvent");

	// An attempt to remap MT32 instruments to GMIDI. Only partially successful, it still
	// does not sound even close to the real MT32. Oddly enough, on the actual hardware MT32
	// and SB sound very differently.
	static const byte mt32Map[128] = {
		0,     1,   0,   2,   4,   4,   5,   3, /* 0-7 */
		16,   17,  18,  16,  16,  19,  20,  21, /* 8-15 */
		6,     6,   6,   7,   7,   7,   8, 112, /* 16-23 */
		62,   62,  63,  63 , 38,  38,  39,  39, /* 24-31 */
		88,   95,  52,  98,  97,  99,  14,  54, /* 32-39 */
		102,  96,  53, 102,  81, 100,  14,  80, /* 40-47 */
		48,   48,  49,  45,  41,  40,  42,  42, /* 48-55 */
		43,   46,  45,  24,  25,  28,  27, 104, /* 56-63 */
		32,   32,  34,  33,  36,  37,  35,  35, /* 64-71 */
		79,   73,  72,  72,  74,  75,  64,  65, /* 72-79 */
		66,   67,  71,  71,  68,  69,  70,  22, /* 80-87 */
		56,   59,  57,  57,  60,  60,  58,  61, /* 88-95 */
		61,   11,  11,  98,  14,   9,  14,  13, /* 96-103 */
		12,  107, 107,  77,  78,  78,  76,  76, /* 104-111 */
		47,  117, 127, 118, 118, 116, 115, 119, /* 112-119 */
		115, 112,  55, 124, 123,   0,  14, 117  /* 120-127 */
	};


	info.start = _position._playPos;
	info.delta = 0;

	info.event = *_position._playPos++;
	warning("Event %x", info.event);
	_position._runningStatus = info.event;

	switch (info.command()) {
	case 0xC: {
		int idx = *_position._playPos++;
		info.basic.param1 = mt32Map[idx & 0x7f]; // remap MT32 to GM
		info.basic.param2 = 0;
		}
		break;
	case 0xD:
		info.basic.param1 = *_position._playPos++;
		info.basic.param2 = 0;
		break;

	case 0xB:
		info.basic.param1 = *_position._playPos++;
		info.basic.param2 = *_position._playPos++;
		info.length = 0;
		break;

	case 0x8:
	case 0x9:
	case 0xA:
	case 0xE:
		info.basic.param1 = *(_position._playPos++);
		info.basic.param2 = *(_position._playPos++);
		if (info.command() == 0x9 && info.basic.param2 == 0) {
			// NoteOn with param2==0 is a NoteOff
			info.event = info.channel() | 0x80;
		}
		info.length = 0;
		break;
	case 0xF:
		if (info.event == 0xFF) {
			byte type = *(_position._playPos++);
			switch(type) {
			case 0x2F:
				// End of Track
				allNotesOff();
				stopPlaying();
				unloadMusic();
				return;
			case 0x51:
				warning("TODO: 0xFF / 0x51");
				return;
			default:
				warning("TODO: 0xFF / %x Unknown", type);
				break;
			}
		} else if (info.event == 0xFC) {
			allNotesOff();
			stopPlaying();
			unloadMusic();
			return;
		} else {
			warning("TODO: %x / Unknown", info.event);
			break;
		}
		break;
	default:
		warning("MidiParser_SH::parseNextEvent: Unsupported event code %x", info.event);
		break;
	}// switch (info.command())

	info.delta = *(_position._playPos++);
}

bool MidiParser_SH::loadMusic(byte *data, uint32 size) {
	warning("loadMusic");
	unloadMusic();

	byte *pos = data;

	if (memcmp("            ", pos, 12)) {
		warning("Expected header not found in music file");
		return false;
	}
	pos += 12;
	byte headerSize = *pos;
	pos += headerSize;

	_lastEvent = 0;
	_trackEnd = data + size;

	_numTracks = 1;
	_tracks[0] = pos;
	
	_ppqn = 1;
	setTempo(16667);
	setTrack(0);

	return true;
}

/*----------------------------------------------------------------*/

Music::Music(SherlockEngine *vm, Audio::Mixer *mixer) : _vm(vm), _mixer(mixer) {
	if (_vm->_interactiveFl)
		_vm->_res->addToCache("MUSIC.LIB");

	MidiDriver::DeviceHandle dev = MidiDriver::detectDevice(MDT_MIDI | MDT_ADLIB | MDT_PREFER_GM);
	_driver = MidiDriver::createMidi(dev);
	assert(_driver);

	int ret = _driver->open();
	if (ret == 0) {
		_driver->sendGMReset();
		_driver->setTimerCallback(&_midiParser, &_midiParser.timerCallback);
	}
	_midiParser.setMidiDriver(_driver);
	_midiParser.setTimerRate(_driver->getBaseTempo());

	_musicPlaying = false;
	_musicOn = true;
}

bool Music::loadSong(int songNumber) {
	warning("loadSong");

	if(songNumber == 100)
		songNumber = 55;
	else if(songNumber == 70)
		songNumber = 54;

	if((songNumber > 60) || (songNumber < 1))
		return false;

	songNumber = ROOM_SONG[songNumber];

	if(songNumber == 0)
		songNumber = 12;

	if((songNumber > NUM_SONGS) || (songNumber < 1))
		return false;

	Common::String songName = Common::String(SONG_NAMES[songNumber - 1]) + ".MUS";

	freeSong();  // free any song that is currently loaded
	
	if (!playMusic(songName))
		return false;

	stopMusic();
	startSong();
	return true;
}

void Music::syncMusicSettings() {
	_musicOn = !ConfMan.getBool("mute") && !ConfMan.getBool("music_mute");
}

bool Music::playMusic(const Common::String &name) {
	if (!_musicOn)
		return false;

	warning("Sound::playMusic %s", name.c_str());
	Common::SeekableReadStream *stream = _vm->_res->load(name, "MUSIC.LIB");

	byte *data = new byte[stream->size()];
	byte *ptr = data;

	stream->read(ptr, stream->size());
	Common::DumpFile outFile;
	outFile.open(name + ".RAW");
	outFile.write(data, stream->size());
	outFile.flush();
	outFile.close();

	_midiParser.loadMusic(data, stream->size());

	return true;
}

void Music::stopMusic() {
	// TODO
	warning("TODO: Sound::stopMusic");

	_musicPlaying = false;
}

void Music::startSong() {
	if (!_musicOn)
		return;

	// TODO
	warning("TODO: Sound::startSong");
	_musicPlaying = true;
}

void Music::freeSong() {
	// TODO
	warning("TODO: Sound::freeSong");
}

void Music::waitTimerRoland(uint time) {
	// TODO
	warning("TODO: Sound::waitTimerRoland");
}} // End of namespace Sherlock

