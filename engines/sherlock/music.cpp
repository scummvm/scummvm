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
#include "sherlock/scalpel/drivers/mididriver.h"

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

MidiParser_SH::~MidiParser_SH() {
	unloadMusic();
	_driver = NULL;
}

void MidiParser_SH::parseNextEvent(EventInfo &info) {
//	warning("parseNextEvent");

	// there is no delta right at the start of the music data
	// this order is essential, otherwise notes will get delayed or even go missing
	if (_position._playPos != _tracks[0]) {
		info.delta = *(_position._playPos++);
	} else {
		info.delta = 0;
	}

	info.start = _position._playPos;

	info.event = *_position._playPos++;
	//warning("Event %x", info.event);
	_position._runningStatus = info.event;

	switch (info.command()) {
	case 0xC: { // program change
		int idx = *_position._playPos++;
		info.basic.param1 = idx & 0x7f;
		// don't do this here, it breaks adlib
		//info.basic.param1 = mt32Map[idx & 0x7f]; // remap MT32 to GM
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
			error("SysEx META event 0xFF");

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
			// Official End-Of-Track signal
			debugC(kDebugLevelMusic, "Music: System META event 0xFC");

			byte type = *(_position._playPos++);
			switch (type) {
			case 0x80: // end of track, triggers looping
				debugC(kDebugLevelMusic, "Music: META event triggered looping");
				jumpToTick(0, true, true, false);
				break;
			case 0x81: // end of track, stop playing
				debugC(kDebugLevelMusic, "Music: META event triggered music stop");
				stopPlaying();
				unloadMusic();
				break;
			default:
				error("MidiParser_SH::parseNextEvent: Unknown META event 0xFC type %x", type);
				break;
			}
		} else {
			warning("TODO: %x / Unknown", info.event);
			break;
		}
		break;
	default:
		warning("MidiParser_SH::parseNextEvent: Unsupported event code %x", info.event);
		break;
	}// switch (info.command())
}

bool MidiParser_SH::loadMusic(byte *data, uint32 size) {
	debugC(kDebugLevelMusic, "Music: loadMusic()");
	unloadMusic();

	byte  *headerPtr  = data;
	byte  *pos        = data;
	uint16 headerSize = READ_LE_UINT16(headerPtr);
	assert(headerSize == 0x7F);

	// Skip over header
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
	_musicPlaying = false;
	_musicOn = true;

	if (_vm->_interactiveFl)
		_vm->_res->addToCache("MUSIC.LIB");

	_midiParser = new MidiParser_SH();

	MidiDriver::DeviceHandle dev = MidiDriver::detectDevice(MDT_MIDI | MDT_ADLIB | MDT_PREFER_MT32);
	_musicType = MidiDriver::getMusicType(dev);

	_driver = NULL;

	switch (_musicType) {
	case MT_ADLIB:
		_driver = MidiDriver_AdLib_create();
		break;
	case MT_MT32:
		_driver = MidiDriver_MT32_create();
		break;
	case MT_GM:
		if (ConfMan.getBool("native_mt32")) {
			_driver = MidiDriver_MT32_create();
			_musicType = MT_MT32;
		}
		break;
	default:
		// Create default one
		// I guess we shouldn't do this anymore
		//_driver = MidiDriver::createMidi(dev);
		break;
	}

	if (_driver) {
		assert(_driver);

		int ret = _driver->open();
		if (ret == 0) {
			// Reset is done inside our MIDI driver
			_driver->setTimerCallback(_midiParser, &_midiParser->timerCallback);
		}
		_midiParser->setMidiDriver(_driver);
		_midiParser->setTimerRate(_driver->getBaseTempo());

		if (_musicType == MT_MT32) {
			// Upload patches
			Common::SeekableReadStream *MT32driverStream = _vm->_res->load("MTHOM.DRV", "MUSIC.LIB");

			byte *MT32driverData = new byte[MT32driverStream->size()];
			int32 MT32driverDataSize = MT32driverStream->size();
			assert(MT32driverData);

			MT32driverStream->read(MT32driverData, MT32driverDataSize);
			delete MT32driverStream;

			assert(MT32driverDataSize > 12);
			byte *MT32driverDataPtr = MT32driverData + 12;
			MT32driverDataSize -= 12;

			MidiDriver_MT32_uploadPatches(_driver, MT32driverDataPtr, MT32driverDataSize);
			delete[] MT32driverData;
		}
	} else {
		// no driver, bye bye music
		_musicOn = false;
	}
}

Music::~Music() {
	stopMusic();
	if (_midiParser) {
		_midiParser->stopPlaying();
		delete _midiParser;
	}
	if (_driver) {
		_driver->close();
		delete _driver;
	}
}

bool Music::loadSong(int songNumber) {
	debugC(kDebugLevelMusic, "Music: loadSong()");

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

bool Music::loadSong(const Common::String &songName) {
	warning("TODO: Music::loadSong");
	return false;
}

void Music::syncMusicSettings() {
	_musicOn = !ConfMan.getBool("mute") && !ConfMan.getBool("music_mute");
}

bool Music::playMusic(const Common::String &name) {
	if (!_musicOn)
		return false;

	debugC(kDebugLevelMusic, "Music: playMusic('%s')", name.c_str());
	Common::SeekableReadStream *stream = _vm->_res->load(name, "MUSIC.LIB");

	byte *data = new byte[stream->size()];
	int32 dataSize = stream->size();
	assert(data);

	stream->read(data, dataSize);
	delete stream;

	// for dumping the music tracks
#if 0
	Common::DumpFile outFile;
	outFile.open(name + ".RAW");
	outFile.write(data, stream->size());
	outFile.flush();
	outFile.close();
#endif

	if (dataSize < 14) {
		warning("Music: not enough data in music file");
		return false;
	}

	byte *dataPos = data;
	if (memcmp("            ", dataPos, 12)) {
		warning("Music: expected header not found in music file");
		return false;
	}
	dataPos += 12;
	dataSize -= 12;

	uint16 headerSize = READ_LE_UINT16(dataPos);
	if (headerSize != 0x7F) {
		warning("Music: header is not as expected");
		return false;
	}

	if (_driver) {
		switch (_musicType) {
		case MT_ADLIB:
			MidiDriver_AdLib_newMusicData(_driver, dataPos, dataSize);
			break;

		case MT_MT32:
			MidiDriver_MT32_newMusicData(_driver, dataPos, dataSize);
			break;

		default:
			// should never happen
			break;
		}
	}

	_midiParser->loadMusic(dataPos, dataSize);
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
}

// This is used to wait for the music in certain situations like especially the intro
// Note: the original game didn't do this, instead it just waited for certain amounts of time
//       We do this, so that the intro graphics + music work together even on faster/slower hardware.
bool Music::waitUntilTick(uint32 tick, uint32 maxTick, uint32 additionalDelay, uint32 noMusicDelay) {
	uint32 currentTick = 0;

	if (!_midiParser->isPlaying()) {
		return _vm->_events->delay(noMusicDelay, true);
	}
	while (1) {
		if (!_midiParser->isPlaying()) { // Music has stopped playing -> we are done
			if (additionalDelay > 0) {
				if (!_vm->_events->delay(additionalDelay, true))
					return false;
			}
			return true;
		}

		currentTick = _midiParser->getTick();
		//warning("waitUntilTick: %lx", currentTick);

		if (currentTick <= maxTick) {
			if (currentTick >= tick) {
				if (additionalDelay > 0) {
					if (!_vm->_events->delay(additionalDelay, true))
						return false;
				}
				return true;
			}
		}
		if (!_vm->_events->delay(10, true))
			return false;
	}
}

} // End of namespace Sherlock

