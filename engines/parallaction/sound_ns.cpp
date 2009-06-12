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
	void pause(bool p);
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
	bool _paused;
	int _masterVolume;
	MidiChannel *_channelsTable[NUM_CHANNELS];
	uint8 _channelsVolume[NUM_CHANNELS];
	Common::Mutex _mutex;
};

MidiPlayer::MidiPlayer(MidiDriver *driver)
	: _driver(driver), _parser(0), _midiData(0), _isLooping(false), _isPlaying(false), _paused(false), _masterVolume(0) {
	assert(_driver);
	memset(_channelsTable, 0, sizeof(_channelsTable));
	for (int i = 0; i < NUM_CHANNELS; i++) {
		_channelsVolume[i] = 127;
	}

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
		delete stream;
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

void MidiPlayer::pause(bool p) {
	_paused = p;

	for (int i = 0; i < NUM_CHANNELS; ++i) {
		if (_channelsTable[i]) {
			_channelsTable[i]->volume(_paused ? 0 : _channelsVolume[i] * _masterVolume / 255);
		}
	}
}

void MidiPlayer::updateTimer() {
	if (_paused) {
		return;
	}

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

DosSoundMan_ns::DosSoundMan_ns(Parallaction_ns *vm, MidiDriver *midiDriver) : SoundMan_ns(vm), _musicData1(0) {
	_midiPlayer = new MidiPlayer(midiDriver);
}

DosSoundMan_ns::~DosSoundMan_ns() {
	debugC(1, kDebugAudio, "DosSoundMan_ns_ns::playMusic()");

	delete _midiPlayer;
}

bool DosSoundMan_ns::isLocationSilent(const char *locationName) {

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

void DosSoundMan_ns::playMusic() {
	debugC(1, kDebugAudio, "DosSoundMan_ns_ns::playMusic()");

	if (isLocationSilent(_vm->_location._name)) {
		// just stop the music if this location is silent
		_midiPlayer->stop();
		return;
	}

	Common::SeekableReadStream *stream = _vm->_disk->loadMusic(_musicFile);
	_midiPlayer->play(stream);
	_midiPlayer->setVolume(255);
}

void DosSoundMan_ns::stopMusic() {
	_midiPlayer->stop();
}

void DosSoundMan_ns::pause(bool p) {
	SoundMan_ns::pause(p);
	_midiPlayer->pause(p);
}

void DosSoundMan_ns::playCharacterMusic(const char *character) {
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
		warning("unknown character '%s' in DosSoundMan_ns_ns::playCharacterMusic", character);
		return;
	}

	playMusic();
}

void DosSoundMan_ns::playLocationMusic(const char *location) {
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

AmigaSoundMan_ns::AmigaSoundMan_ns(Parallaction_ns *vm) : SoundMan_ns(vm) {
	_musicStream = 0;
	_channels[0].data = 0;
	_channels[0].dispose = false;
	_channels[1].data = 0;
	_channels[1].dispose = false;
	_channels[2].data = 0;
	_channels[2].dispose = false;
	_channels[3].data = 0;
	_channels[3].dispose = false;
}

AmigaSoundMan_ns::~AmigaSoundMan_ns() {
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

Audio::AudioStream *AmigaSoundMan_ns::loadChannelData(const char *filename, Channel *ch, bool looping) {
	Audio::AudioStream *input = 0;

	if (!scumm_stricmp("beep", filename)) {
		ch->header.oneShotHiSamples = 0;
		ch->header.repeatHiSamples = 0;
		ch->header.samplesPerHiCycle = 0;
		ch->header.samplesPerSec = 11934;
		ch->header.volume = 160;
		ch->data = (int8*)malloc(AMIGABEEP_SIZE * NUM_REPEATS);
		int8* odata = ch->data;
		for (uint i = 0; i < NUM_REPEATS; i++) {
			memcpy(odata, res_amigaBeep, AMIGABEEP_SIZE);
			odata += AMIGABEEP_SIZE;
		}
		ch->dataSize = AMIGABEEP_SIZE * NUM_REPEATS;
		ch->dispose = true;

		uint32 loopStart = 0, loopEnd = 0, flags = 0;
		if (looping) {
			loopEnd = ch->header.oneShotHiSamples + ch->header.repeatHiSamples;
			flags = Audio::Mixer::FLAG_LOOP;
		}

		input = Audio::makeLinearInputStream((byte *)ch->data, ch->dataSize, ch->header.samplesPerSec, flags, loopStart, loopEnd);
	} else {
		Common::SeekableReadStream *stream = _vm->_disk->loadSound(filename);
		input = Audio::make8SVXStream(*stream, looping);
		ch->dispose = true;
		delete stream;
	}

	return input;
}

void AmigaSoundMan_ns::playSfx(const char *filename, uint channel, bool looping, int volume) {
	if (channel >= NUM_SFX_CHANNELS) {
		warning("unknown sfx channel");
		return;
	}

	stopSfx(channel);

	debugC(1, kDebugAudio, "AmigaSoundMan_ns::playSfx(%s, %i)", filename, channel);

	Channel *ch = &_channels[channel];
	Audio::AudioStream *input = loadChannelData(filename, ch, looping);

	if (volume == -1) {
		volume = ch->header.volume;
	}

	_mixer->playInputStream(Audio::Mixer::kSFXSoundType, &ch->handle, input, -1, volume);
}

void AmigaSoundMan_ns::stopSfx(uint channel) {
	if (channel >= NUM_SFX_CHANNELS) {
		warning("unknown sfx channel");
		return;
	}

	if (_channels[channel].dispose) {
		debugC(1, kDebugAudio, "AmigaSoundMan_ns::stopSfx(%i)", channel);
		_mixer->stopHandle(_channels[channel].handle);
		free(_channels[channel].data);
		_channels[channel].data = 0;
	}
}

void AmigaSoundMan_ns::playMusic() {
	stopMusic();

	debugC(1, kDebugAudio, "AmigaSoundMan_ns::playMusic()");

	Common::SeekableReadStream *stream = _vm->_disk->loadMusic(_musicFile);
	_musicStream = Audio::makeProtrackerStream(stream);
	delete stream;

	debugC(3, kDebugAudio, "AmigaSoundMan_ns::playMusic(): created new music stream");

	_mixer->playInputStream(Audio::Mixer::kMusicSoundType, &_musicHandle, _musicStream, -1, 255, 0, false, false);
}

void AmigaSoundMan_ns::stopMusic() {
	debugC(1, kDebugAudio, "AmigaSoundMan_ns::stopMusic()");

	if (_mixer->isSoundHandleActive(_musicHandle)) {
		_mixer->stopHandle(_musicHandle);
		delete _musicStream;
		_musicStream = 0;
	}
}

void AmigaSoundMan_ns::playCharacterMusic(const char *character) {
}

void AmigaSoundMan_ns::playLocationMusic(const char *location) {
}


SoundMan_ns::SoundMan_ns(Parallaction_ns *vm) : _vm(vm) {
	_mixer = _vm->_mixer;
}

void SoundMan_ns::setMusicVolume(int value) {
	_mixer->setVolumeForSoundType(Audio::Mixer::kMusicSoundType, value);
}

void SoundMan_ns::setMusicFile(const char *filename) {
	strcpy(_musicFile, filename);
}

void SoundMan_ns::execute(int command, const char *parm = 0) {
	uint32 n = strtoul(parm, 0, 10);
	bool b = (n == 1) ? true : false;

	switch (command) {
	case SC_PLAYMUSIC:
		if (_musicType == MUSIC_CHARACTER) playCharacterMusic(parm);
		else if (_musicType == MUSIC_LOCATION) playLocationMusic(parm);
		else playMusic();
		break;
	case SC_STOPMUSIC:
		stopMusic();
		break;
	case SC_SETMUSICTYPE:
		_musicType = n;
		break;
	case SC_SETMUSICFILE:
		setMusicFile(parm);
		break;

	case SC_PLAYSFX:
		playSfx(parm, _sfxChannel, _sfxLooping, _sfxVolume);
		break;
	case SC_STOPSFX:
		stopSfx(n);
		break;

	case SC_SETSFXCHANNEL:
		_sfxChannel = n;
		break;
	case SC_SETSFXLOOPING:
		_sfxLooping = b;
		break;
	case SC_SETSFXVOLUME:
		_sfxVolume = n;
		break;

	case SC_PAUSE:
		pause(b);
		break;
	}
}

} // namespace Parallaction
