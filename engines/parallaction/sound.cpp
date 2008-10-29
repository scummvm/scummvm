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
 * $URL$
 * $Id$
 *
 */

#include "common/file.h"

#include "common/stream.h"

#include "sound/mixer.h"
#include "sound/midiparser.h"
#include "sound/mods/protracker.h"

#include "parallaction/sound.h"
#include "parallaction/parallaction.h"


namespace Parallaction {

class MidiPlayer : public MidiDriver {
public:

	enum {
		NUM_CHANNELS = 16
	};

	MidiPlayer(MidiDriver *driver);
	~MidiPlayer();

	void play(Common::SeekableReadStream *stream);
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

void MidiPlayer::play(Common::SeekableReadStream *stream) {
	if (!stream) {
		stop();
		return;
	}

	int size = stream->size();

	_midiData = (uint8 *)malloc(size);
	if (_midiData) {
		stream->read(_midiData, size);
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
	delete _driver;
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
	debugC(1, kDebugAudio, "DosSoundMan::playMusic()");

	delete _midiPlayer;
}

bool DosSoundMan::isLocationSilent(const char *locationName) {

	// these are the prefixes for location names with no background midi music
	const char *noMusicPrefix[] = { "museo", "intgrottadopo", "caveau", "estgrotta", "plaza1", "endtgz", "common", 0 };
	Common::String s(locationName);

	for (int i = 0; noMusicPrefix[i]; i++) {
		if (s.hasPrefix(noMusicPrefix[i])) {
			return true;
		}
	}

	return false;
}

void DosSoundMan::playMusic() {
	debugC(1, kDebugAudio, "DosSoundMan::playMusic()");

	if (isLocationSilent(_vm->_location._name)) {
		// just stop the music if this location is silent
		_midiPlayer->stop();
		return;
	}

	Common::SeekableReadStream *stream = _vm->_disk->loadMusic(_musicFile);
	_midiPlayer->play(stream);
}

void DosSoundMan::stopMusic() {
	_midiPlayer->stop();
}

void DosSoundMan::playCharacterMusic(const char *character) {
	if (character == NULL) {
		return;
	}

	if (!scumm_stricmp(_vm->_location._name, "night") ||
		!scumm_stricmp(_vm->_location._name, "intsushi")) {
		return;
	}

	char *name = const_cast<char*>(character);

	if (!scumm_stricmp(name, _dinoName)) {
		setMusicFile("dino");
	} else
	if (!scumm_stricmp(name, _donnaName)) {
		setMusicFile("donna");
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
		playCharacterMusic(_vm->_char.getBaseName());
		_musicData1 = 0;
		debugC(2, kDebugExec, "changeLocation: started character specific music");
	}

	if (!scumm_stricmp(location, "night") || !scumm_stricmp(location, "intsushi")) {
		setMusicFile("nuts");
		playMusic();

		debugC(2, kDebugExec, "changeLocation: started music 'soft'");
	}

	if (isLocationSilent(location)) {
		stopMusic();
		_musicData1 = 1;

		debugC(2, kDebugExec, "changeLocation: music stopped");
	}
}

AmigaSoundMan::AmigaSoundMan(Parallaction *vm) : SoundMan(vm) {
	_musicStream = 0;
	_channels[0].data = 0;
	_channels[1].data = 0;
	_channels[2].data = 0;
	_channels[3].data = 0;
}

AmigaSoundMan::~AmigaSoundMan() {
	stopMusic();
	stopSfx(0);
	stopSfx(1);
	stopSfx(2);
	stopSfx(3);
}

#define AMIGABEEP_SIZE	16
#define NUM_REPEATS		60

static int8 res_amigaBeep[AMIGABEEP_SIZE] = {
	0, 20, 40, 60, 80, 60, 40, 20, 0, -20, -40, -60, -80, -60, -40, -20
};


void AmigaSoundMan::loadChannelData(const char *filename, Channel *ch) {
	if (!scumm_stricmp("beep", filename)) {
		ch->header.oneShotHiSamples = 0;
		ch->header.repeatHiSamples = 0;
		ch->header.samplesPerHiCycle = 0;
		ch->header.samplesPerSec = 11934;
		ch->header.volume = 160;
		ch->data = new int8[AMIGABEEP_SIZE * NUM_REPEATS];
		int8* odata = ch->data;
		for (uint i = 0; i < NUM_REPEATS; i++) {
			memcpy(odata, res_amigaBeep, AMIGABEEP_SIZE);
			odata += AMIGABEEP_SIZE;
		}
		ch->dataSize = AMIGABEEP_SIZE * NUM_REPEATS;
		ch->dispose = true;
		return;
	}

	Common::ReadStream *stream = _vm->_disk->loadSound(filename);
	Audio::A8SVXDecoder decoder(*stream, ch->header, ch->data, ch->dataSize);
	decoder.decode();
	ch->dispose = true;
	delete stream;
}

void AmigaSoundMan::playSfx(const char *filename, uint channel, bool looping, int volume, int rate) {
	if (channel >= NUM_AMIGA_CHANNELS) {
		warning("unknown sfx channel");
		return;
	}

	debugC(1, kDebugAudio, "AmigaSoundMan::playSfx(%s, %i)", filename, channel);

	Channel *ch = &_channels[channel];
	loadChannelData(filename, ch);

	uint32 loopStart, loopEnd, flags;
	if (looping) {
		// the standard way to loop 8SVX audio implies use of the oneShotHiSamples and
		// repeatHiSamples fields, but Nippon Safes handles loops according to flags
		// set in its location scripts and always operates on the whole data.
		loopStart = 0;
		loopEnd = ch->header.oneShotHiSamples + ch->header.repeatHiSamples;
		flags = Audio::Mixer::FLAG_LOOP;
	} else {
		loopStart = loopEnd = 0;
		flags = 0;
	}

	if (volume == -1) {
		volume = ch->header.volume;
	}

	if (rate == -1) {
		rate = ch->header.samplesPerSec;
	}

	_mixer->playRaw(Audio::Mixer::kSFXSoundType, &ch->handle, ch->data, ch->dataSize, rate, flags, -1, volume, 0, loopStart, loopEnd);
}

void AmigaSoundMan::stopSfx(uint channel) {
	if (channel >= NUM_AMIGA_CHANNELS) {
		warning("unknown sfx channel");
		return;
	}

	if (_channels[channel].dispose) {
		debugC(1, kDebugAudio, "AmigaSoundMan::stopSfx(%i)", channel);
		_mixer->stopHandle(_channels[channel].handle);
		free(_channels[channel].data);
		_channels[channel].data = 0;
	}
}

void AmigaSoundMan::playMusic() {
	stopMusic();

	debugC(1, kDebugAudio, "AmigaSoundMan::playMusic()");

	Common::SeekableReadStream *stream = _vm->_disk->loadMusic(_musicFile);
	_musicStream = Audio::makeProtrackerStream(stream);
	delete stream;

	debugC(3, kDebugAudio, "AmigaSoundMan::playMusic(): created new music stream");

	_mixer->playInputStream(Audio::Mixer::kMusicSoundType, &_musicHandle, _musicStream, -1, 255, 0, false, false);
}

void AmigaSoundMan::stopMusic() {
	debugC(1, kDebugAudio, "AmigaSoundMan::stopMusic()");

	if (_mixer->isSoundHandleActive(_musicHandle)) {
		_mixer->stopHandle(_musicHandle);
		delete _musicStream;
		_musicStream = 0;
	}
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
