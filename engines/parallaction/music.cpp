/* ScummVM - Scumm Interpreter
 * Copyright (C) 2006 The ScummVM project
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
 * $URL$
 * $Id$
 *
 */

#include "common/stdafx.h"
#include "common/file.h"

#include "common/stream.h"

#include "sound/mixer.h"
#include "sound/midiparser.h"
#include "sound/mods/protracker.h"

#include "parallaction/music.h"
#include "parallaction/parallaction.h"


namespace Parallaction {

class MidiPlayer : public MidiDriver {
public:

	enum {
		NUM_CHANNELS = 16
	};

	MidiPlayer(MidiDriver *driver);
	~MidiPlayer();

	void play(const char *filename);
	void stop();
	void updateTimer();
	void adjustVolume(int diff);
	void setVolume(int volume);
	int getVolume() const { return _masterVolume; }
	void setLooping(bool loop) { _isLooping = loop; }

	// MidiDriver interface
	int open();
	void close();
	void send(uint32 b);
	void metaEvent(byte type, byte *data, uint16 length);
	void setTimerCallback(void *timerParam, void (*timerProc)(void *)) { }
	uint32 getBaseTempo() { return _driver ? _driver->getBaseTempo() : 0; }
	MidiChannel *allocateChannel() { return 0; }
	MidiChannel *getPercussionChannel() { return 0; }

private:

	static void timerCallback(void *p);

	MidiDriver *_driver;
	MidiParser *_parser;
	uint8 *_midiData;
	bool _isLooping;
	bool _isPlaying;
	int _masterVolume;
	MidiChannel *_channelsTable[NUM_CHANNELS];
	uint8 _channelsVolume[NUM_CHANNELS];
	Common::Mutex _mutex;
};

MidiPlayer::MidiPlayer(MidiDriver *driver)
	: _driver(driver), _parser(0), _midiData(0), _isLooping(false), _isPlaying(false), _masterVolume(0) {
	assert(_driver);
	memset(_channelsTable, 0, sizeof(_channelsTable));
	memset(_channelsVolume, 0, sizeof(_channelsVolume));

	open();
}

MidiPlayer::~MidiPlayer() {
	close();
}

void MidiPlayer::play(const char *filename) {
	stop();

	if (!scumm_strnicmp(_vm->_location._name, "museo", 5)) return;
	if (!scumm_strnicmp(_vm->_location._name, "intgrottadopo", 13)) return;
	if (!scumm_strnicmp(_vm->_location._name, "caveau", 6)) return;
	if (!scumm_strnicmp(_vm->_location._name, "estgrotta", 9)) return;
	if (!scumm_strnicmp(_vm->_location._name, "plaza1", 6)) return;
	if (!scumm_strnicmp(_vm->_location._name, "endtgz", 6)) return;

	char path[PATH_LEN];
	sprintf(path, "%s.mid", filename);

	Common::File stream;

	if (!stream.open(path))
		return;

	int size = stream.size();

	_midiData = (uint8 *)malloc(size);
	if (_midiData) {
		stream.read(_midiData, size);
		_mutex.lock();
		_parser->loadMusic(_midiData, size);
		_parser->setTrack(0);
		_isLooping = true;
		_isPlaying = true;
		_mutex.unlock();
	}
}

void MidiPlayer::stop() {
	_mutex.lock();
	if (_isPlaying) {
		_isPlaying = false;
		_parser->unloadMusic();
		free(_midiData);
		_midiData = 0;
	}
	_mutex.unlock();
}

void MidiPlayer::updateTimer() {
	_mutex.lock();
	if (_isPlaying) {
		_parser->onTimer();
	}
	_mutex.unlock();
}

void MidiPlayer::adjustVolume(int diff) {
	setVolume(_masterVolume + diff);
}

void MidiPlayer::setVolume(int volume) {
	_masterVolume = CLIP(volume, 0, 255);
	_mutex.lock();
	for (int i = 0; i < NUM_CHANNELS; ++i) {
		if (_channelsTable[i]) {
			_channelsTable[i]->volume(_channelsVolume[i] * _masterVolume / 255);
		}
	}
	_mutex.unlock();
}

int MidiPlayer::open() {
	int ret = _driver->open();
	if (ret == 0) {
		_parser = MidiParser::createParser_SMF();
		_parser->setMidiDriver(this);
		_parser->setTimerRate(_driver->getBaseTempo());
		_driver->setTimerCallback(this, &timerCallback);
	}
	return ret;
}

void MidiPlayer::close() {
	stop();
	_mutex.lock();
	_driver->setTimerCallback(NULL, NULL);
	_driver->close();
	_driver = 0;
	_parser->setMidiDriver(NULL);
	delete _parser;
	_mutex.unlock();
}

void MidiPlayer::send(uint32 b) {
	byte volume, ch = (byte)(b & 0xF);
	switch (b & 0xFFF0) {
	case 0x07B0: // volume change
		volume = (byte)((b >> 16) & 0x7F);
		_channelsVolume[ch] = volume;
		volume = volume * _masterVolume / 255;
		b = (b & 0xFF00FFFF) | (volume << 16);
		break;
	case 0x7BB0: // all notes off
		if (!_channelsTable[ch]) {
			// channel not yet allocated, no need to send the event
			return;
		}
		break;
	}

	if (!_channelsTable[ch]) {
		_channelsTable[ch] = (ch == 9) ? _driver->getPercussionChannel() : _driver->allocateChannel();
	}
	if (_channelsTable[ch]) {
		_channelsTable[ch]->send(b);
	}
}

void MidiPlayer::metaEvent(byte type, byte *data, uint16 length) {
	switch (type) {
	case 0x2F: // end of Track
		if (_isLooping) {
			_parser->jumpToTick(0);
		} else {
			stop();
		}
		break;
	default:
//		warning("Unhandled meta event: %02x", type);
		break;
	}
}

void MidiPlayer::timerCallback(void *p) {
	MidiPlayer *player = (MidiPlayer *)p;

	player->updateTimer();
}


DosSoundMan::DosSoundMan(Parallaction *vm, MidiDriver *midiDriver) : SoundMan(vm), _musicData1(0) {
	_midiPlayer = new MidiPlayer(midiDriver);
}

DosSoundMan::~DosSoundMan() {
	delete _midiPlayer;
}

void DosSoundMan::playMusic() {
	_midiPlayer->play(_musicFile);
}

void DosSoundMan::stopMusic() {
	_midiPlayer->stop();
}

void DosSoundMan::playCharacterMusic(const char *character) {

	if (!scumm_stricmp(_vm->_location._name, "night") ||
		!scumm_stricmp(_vm->_location._name, "intsushi")) {
		return;
	}

	char *name = const_cast<char*>(character);

	if (IS_MINI_CHARACTER(name))
		name+=4;

	if (!scumm_stricmp(name, _dinoName)) {
		setMusicFile("dino");
	} else
	if (!scumm_stricmp(name, _donnaName)) {
		setMusicFile("dough");
	} else
	if (!scumm_stricmp(name, _doughName)) {
		setMusicFile("nuts");
	} else {
		warning("unknown character '%s' in DosSoundMan::playCharacterMusic", character);
		return;
	}

	playMusic();
}

void DosSoundMan::playLocationMusic(const char *location) {
	if (_musicData1 != 0) {
		playCharacterMusic(_vm->_characterName);
		_musicData1 = 0;
		debugC(2, kDebugLocation, "changeLocation: started character specific music");
	}

	if (!scumm_stricmp(location, "night") || !scumm_stricmp(location, "intsushi")) {
		setMusicFile("nuts");
		playMusic();

		debugC(2, kDebugLocation, "changeLocation: started music 'soft'");
	}

	if (!scumm_stricmp(location, "museo") ||
		!scumm_stricmp(location, "caveau") ||
		!scumm_strnicmp(location, "plaza1", 6) ||
		!scumm_stricmp(location, "estgrotta") ||
		!scumm_stricmp(location, "intgrottadopo") ||
		!scumm_stricmp(location, "endtgz") ||
		!scumm_stricmp(location, "common")) {

		stopMusic();
		_musicData1 = 1;

		debugC(2, kDebugLocation, "changeLocation: music stopped");
	}
}

AmigaSoundMan::AmigaSoundMan(Parallaction *vm) : SoundMan(vm) {
	_musicStream = 0;
}

AmigaSoundMan::~AmigaSoundMan() {
	stopMusic();
}

void AmigaSoundMan::playMusic() {
	stopMusic();

	Common::ReadStream *stream = _vm->_disk->loadMusic(_musicFile);
	_musicStream = Audio::makeProtrackerStream(stream);
	delete stream;

	_mixer->playInputStream(Audio::Mixer::kMusicSoundType, &_musicHandle, _musicStream, -1, 255, 0, false, true);
}

void AmigaSoundMan::stopMusic() {
	_mixer->stopHandle(_musicHandle);
	delete _musicStream;
}

void AmigaSoundMan::playCharacterMusic(const char *character) {
}

void AmigaSoundMan::playLocationMusic(const char *location) {
}


SoundMan::SoundMan(Parallaction *vm) : _vm(vm) {
	_mixer = _vm->_mixer;
}

void SoundMan::setMusicVolume(int value) {
	_mixer->setVolumeForSoundType(Audio::Mixer::kMusicSoundType, value);
}

void SoundMan::setMusicFile(const char *filename) {
	strcpy(_musicFile, filename);
}


} // namespace Parallaction
