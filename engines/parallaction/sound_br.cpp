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

#include "sound/mixer.h"
#include "common/stream.h"
#include "common/util.h"

#include "sound/mixer.h"
#include "sound/mididrv.h"
#include "sound/midiparser.h"
#include "sound/mods/protracker.h"

#include "parallaction/disk.h"
#include "parallaction/parallaction.h"
#include "parallaction/sound.h"


namespace Parallaction {


/*
 * List of calls to the original music driver.
 *
 *
 * 1 set music buffer segment
 * 2 set music buffer offset
 * 3 set music buffer size
 * 4 play/resume
 * 5 stop
 * 6 pause
 * 7 set channel volume
 * 8 set byte_11C5A (boolean flag for ??)
 * 9 toggle fade
 * 10 set volume
 * 11 shutdown
 * 12 get status
 * 13 set byte_11C4D (used for fade??)
 * 14 get volume
 * 15 get X??
 * 16 get fade flag
 * 17 set tempo
 * 18 get tempo
 * 19 set Y??
 * 20 get looping flag
 * 21 toggle looping flag
 * 22 get version??
 * 23 get version??
 * 24 get busy flag
 */

class MidiParser_MSC : public MidiParser {
protected:
	virtual void parseNextEvent(EventInfo &info);
	virtual bool loadMusic(byte *data, uint32 size);

	uint8  read1(byte *&data) {
		return *data++;
	}

	void parseMetaEvent(EventInfo &info);
	void parseMidiEvent(EventInfo &info);

	bool byte_11C5A;
	uint8 _beats;
	uint8 _lastEvent;
	byte *_trackEnd;

public:
	MidiParser_MSC() : byte_11C5A(false) {
	}
};

void MidiParser_MSC::parseMetaEvent(EventInfo &info) {
	uint8 type = read1(_position._play_pos);
	uint8 len = read1(_position._play_pos);
	info.ext.type = type;
	info.length = len;
	info.ext.data = 0;

	if (type == 0x51) {
		info.ext.data = _position._play_pos;
	} else {
		warning("unknown meta event 0x%02X", type);
		info.ext.type = 0;
	}

	_position._play_pos += len;
}

void MidiParser_MSC::parseMidiEvent(EventInfo &info) {
	uint8 type = info.command();

	switch (type) {
	case 0x8:
	case 0x9:
	case 0xA:
	case 0xB:
	case 0xE:
		info.basic.param1 = read1(_position._play_pos);
		info.basic.param2 = read1(_position._play_pos);
		break;

	case 0xC:
	case 0xD:
		info.basic.param1 = read1(_position._play_pos);
		info.basic.param2 = 0;
		break;

	default:
		warning("Unexpected midi event 0x%02X in midi data.", info.event);
	}

	//if ((type == 0xB) && (info.basic.param1 == 64)) info.basic.param2 = 127;

}

void MidiParser_MSC::parseNextEvent(EventInfo &info) {
	info.start = _position._play_pos;

	if (_position._play_pos >= _trackEnd) {
		// fake an end-of-track meta event
		info.delta = 0;
		info.event = 0xFF;
		info.ext.type = 0x2F;
		info.length = 0;
		return;
	}

	info.delta = readVLQ(_position._play_pos);
	info.event = read1(_position._play_pos);

	if (info.event == 0xFF) {
		parseMetaEvent(info);
		return;
	}

	if (info.event < 0x80) {
		_position._play_pos--;
		info.event = _lastEvent;
	}

	parseMidiEvent(info);
	_lastEvent = info.event;

}

bool MidiParser_MSC::loadMusic(byte *data, uint32 size) {
	unloadMusic();

	byte *pos = data;

	uint32 signature = read4high(pos);
	if (memcmp("tCSM", &signature, 4)) {
		warning("Expected header not found in music file.");
		return false;
	}

	_beats = read1(pos);
	_ppqn = read2low(pos);

	if (byte_11C5A) {
		// do something with byte_11C4D
	}

	_lastEvent = 0;
	_trackEnd = data + size;

	_num_tracks = 1;
	_tracks[0] = pos;

	setTempo(500000);
	setTrack(0);
	return true;
}


MidiParser *createParser_MSC() {
	return new MidiParser_MSC;
}


class MidiPlayer_MSC : public MidiDriver {
public:

	enum {
		NUM_CHANNELS = 16
	};

	MidiPlayer_MSC(MidiDriver *driver);
	~MidiPlayer_MSC();

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
	void setVolumeInternal(int volume);

	Common::Mutex _mutex;
	MidiDriver *_driver;
	MidiParser *_parser;
	uint8 *_midiData;
	bool _isLooping;
	bool _isPlaying;
	bool _paused;

	int _masterVolume;
	MidiChannel *_channels[NUM_CHANNELS];
	uint8 _volume[NUM_CHANNELS];
};



MidiPlayer_MSC::MidiPlayer_MSC(MidiDriver *driver)
	: _driver(driver), _parser(0), _midiData(0), _isLooping(false), _isPlaying(false), _paused(false), _masterVolume(0) {
	assert(_driver);
	memset(_channels, 0, sizeof(_channels));
	for (int i = 0; i < NUM_CHANNELS; i++) {
		_volume[i] = 127;
	}

	open();
}

MidiPlayer_MSC::~MidiPlayer_MSC() {
	close();
}

void MidiPlayer_MSC::play(Common::SeekableReadStream *stream) {
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

void MidiPlayer_MSC::stop() {
	_mutex.lock();
	if (_isPlaying) {
		_isPlaying = false;
		_parser->unloadMusic();
		free(_midiData);
		_midiData = 0;
	}
	_mutex.unlock();
}

void MidiPlayer_MSC::pause(bool p) {
	_paused = p;
	setVolumeInternal(_paused ? 0 : _masterVolume);
}

void MidiPlayer_MSC::updateTimer() {
	if (_paused) {
		return;
	}

	Common::StackLock lock(_mutex);
	if (_isPlaying) {
		_parser->onTimer();
	}
}

void MidiPlayer_MSC::adjustVolume(int diff) {
	setVolume(_masterVolume + diff);
}

void MidiPlayer_MSC::setVolume(int volume) {
	_masterVolume = CLIP(volume, 0, 255);
	setVolumeInternal(_masterVolume);
}

void MidiPlayer_MSC::setVolumeInternal(int volume) {
	Common::StackLock lock(_mutex);
	for (int i = 0; i < NUM_CHANNELS; ++i) {
		if (_channels[i]) {
			_channels[i]->volume(_volume[i] * volume / 255);
		}
	}
}

int MidiPlayer_MSC::open() {
	int ret = _driver->open();
	if (ret == 0) {
		_parser = createParser_MSC();
		_parser->setMidiDriver(this);
		_parser->setTimerRate(_driver->getBaseTempo());
		_driver->setTimerCallback(this, &timerCallback);
	}
	return ret;
}

void MidiPlayer_MSC::close() {
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

void MidiPlayer_MSC::send(uint32 b) {
	const byte ch = b & 0x0F;
	byte param2 = (b >> 16) & 0xFF;

	switch (b & 0xFFF0) {
	case 0x07B0: // volume change
		_volume[ch] = param2;
		break;
	}

	if (!_channels[ch]) {
		_channels[ch] = (ch == 9) ? _driver->getPercussionChannel() : _driver->allocateChannel();
	}
	if (_channels[ch]) {
		_channels[ch]->send(b);
	}
}

void MidiPlayer_MSC::metaEvent(byte type, byte *data, uint16 length) {
	switch (type) {
	case 0x2F: // end of Track
		if (_isLooping) {
			_parser->jumpToTick(0);
		} else {
			stop();
		}
		break;
	default:
		break;
	}
}

void MidiPlayer_MSC::timerCallback(void *p) {
	MidiPlayer_MSC *player = (MidiPlayer_MSC *)p;

	player->updateTimer();
}

DosSoundMan_br::DosSoundMan_br(Parallaction_br *vm, MidiDriver *driver) : SoundMan_br(vm) {
	_midiPlayer = new MidiPlayer_MSC(driver);
	assert(_midiPlayer);
}

DosSoundMan_br::~DosSoundMan_br() {
	delete _midiPlayer;
}

Audio::AudioStream *DosSoundMan_br::loadChannelData(const char *filename, Channel *ch, bool looping) {
	Common::SeekableReadStream *stream = _vm->_disk->loadSound(filename);

	ch->dataSize = stream->size();
	ch->data = (int8*)malloc(ch->dataSize);
	if (stream->read(ch->data, ch->dataSize) != ch->dataSize)
		error("DosSoundMan_br::loadChannelData: Read failed");

	ch->dispose = true;
	delete stream;

	// TODO: Confirm sound rate
	ch->header.samplesPerSec = 11025;

	uint32 loopStart = 0, loopEnd = 0, flags = Audio::Mixer::FLAG_UNSIGNED;
	if (looping) {
		loopEnd = ch->dataSize;
		flags |= Audio::Mixer::FLAG_LOOP;
	}

	// Create the input stream
	return Audio::makeLinearInputStream((byte *)ch->data, ch->dataSize, ch->header.samplesPerSec, flags, loopStart, loopEnd);
}

void DosSoundMan_br::playSfx(const char *filename, uint channel, bool looping, int volume) {
	stopSfx(channel);

	if (!_sfxEnabled) {
		return;
	}

	debugC(1, kDebugAudio, "DosSoundMan_br::playSfx(%s, %u, %i, %i)", filename, channel, looping, volume);

	Channel *ch = &_channels[channel];
	Audio::AudioStream *input = loadChannelData(filename, ch, looping);
	_mixer->playInputStream(Audio::Mixer::kSFXSoundType, &ch->handle, input, -1, volume);
}

void DosSoundMan_br::playMusic() {
	if (_musicFile.empty()) {
		return;
	}

	if (!_musicEnabled) {
		return;
	}

	Common::SeekableReadStream *s = _vm->_disk->loadMusic(_musicFile.c_str());
	assert(s);
	_midiPlayer->play(s);
}

void DosSoundMan_br::stopMusic() {
	_midiPlayer->stop();
}

void DosSoundMan_br::pause(bool p) {
	_midiPlayer->pause(p);
}

AmigaSoundMan_br::AmigaSoundMan_br(Parallaction_br *vm) : SoundMan_br(vm)  {
	_musicStream = 0;
}

AmigaSoundMan_br::~AmigaSoundMan_br() {
	stopMusic();
}

Audio::AudioStream *AmigaSoundMan_br::loadChannelData(const char *filename, Channel *ch, bool looping) {
	Common::SeekableReadStream *stream = _vm->_disk->loadSound(filename);
	Audio::AudioStream *input = 0;

	if (_vm->getFeatures() & GF_DEMO) {
		ch->dataSize = stream->size();
		ch->data = (int8*)malloc(ch->dataSize);
		if (stream->read(ch->data, ch->dataSize) != ch->dataSize)
			error("DosSoundMan_br::loadChannelData: Read failed");

		// TODO: Confirm sound rate
		ch->header.samplesPerSec = 11025;

		uint32 loopStart = 0, loopEnd = 0, flags = 0;
		if (looping) {
			loopEnd = ch->header.oneShotHiSamples + ch->header.repeatHiSamples;
			flags = Audio::Mixer::FLAG_LOOP;
		}

		input = Audio::makeLinearInputStream((byte *)ch->data, ch->dataSize, ch->header.samplesPerSec, flags, loopStart, loopEnd);
	} else {
		input = Audio::make8SVXStream(*stream, looping);
		delete stream;
	}

	return input;
}

void AmigaSoundMan_br::playSfx(const char *filename, uint channel, bool looping, int volume) {
	if (channel >= NUM_SFX_CHANNELS) {
		warning("unknown sfx channel");
		return;
	}

	stopSfx(channel);

	if (!_sfxEnabled) {
		return;
	}

	debugC(1, kDebugAudio, "AmigaSoundMan_ns::playSfx(%s, %i)", filename, channel);

	Channel *ch = &_channels[channel];
	Audio::AudioStream *input = loadChannelData(filename, ch, looping);

	if (volume == -1) {
		volume = ch->header.volume;
	}

	_mixer->playInputStream(Audio::Mixer::kSFXSoundType, &ch->handle, input, -1, volume);
}

void AmigaSoundMan_br::playMusic() {
	stopMusic();

	if (!_musicEnabled) {
		return;
	}

	debugC(1, kDebugAudio, "AmigaSoundMan_ns::playMusic()");

	Common::SeekableReadStream *stream = _vm->_disk->loadMusic(_musicFile.c_str());
	// NOTE: Music files don't always exist
	if (!stream)
		return;

	_musicStream = Audio::makeProtrackerStream(stream);
	delete stream;

	debugC(3, kDebugAudio, "AmigaSoundMan_ns::playMusic(): created new music stream");

	_mixer->playInputStream(Audio::Mixer::kMusicSoundType, &_musicHandle, _musicStream, -1, 255, 0, false, false);
}

void AmigaSoundMan_br::stopMusic() {
	debugC(1, kDebugAudio, "AmigaSoundMan_ns::stopMusic()");

	if (_mixer->isSoundHandleActive(_musicHandle)) {
		_mixer->stopHandle(_musicHandle);
		delete _musicStream;
		_musicStream = 0;
	}
}

void AmigaSoundMan_br::pause(bool p) {
	_mixer->pauseHandle(_musicHandle, p);
}

SoundMan_br::SoundMan_br(Parallaction_br *vm) : _vm(vm) {
	_mixer = _vm->_mixer;

	_channels[0].data = 0;
	_channels[0].dispose = false;
	_channels[1].data = 0;
	_channels[1].dispose = false;
	_channels[2].data = 0;
	_channels[2].dispose = false;
	_channels[3].data = 0;
	_channels[3].dispose = false;

	_musicEnabled = true;
	_sfxEnabled = true;
}

SoundMan_br::~SoundMan_br() {
	stopAllSfx();
}

void SoundMan_br::stopAllSfx() {
	stopSfx(0);
	stopSfx(1);
	stopSfx(2);
	stopSfx(3);
}

void SoundMan_br::setMusicFile(const char *name) {
	stopMusic();
	_musicFile = name;
}

void SoundMan_br::stopSfx(uint channel) {
	if (channel >= NUM_SFX_CHANNELS) {
		warning("unknown sfx channel");
		return;
	}

	if (_channels[channel].dispose) {
		debugC(1, kDebugAudio, "SoundMan_br::stopSfx(%i)", channel);
		_mixer->stopHandle(_channels[channel].handle);
		free(_channels[channel].data);
		_channels[channel].data = 0;
	}
}

void SoundMan_br::execute(int command, const char *parm) {
	uint32 n = parm ? strtoul(parm, 0, 10) : 0;
	bool b = (n == 1) ? true : false;

	switch (command) {
	case SC_PLAYMUSIC:
		playMusic();
		break;
	case SC_STOPMUSIC:
		stopMusic();
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

void SoundMan_br::enableSfx(bool enable) {
	if (!enable) {
		stopAllSfx();
	}
	_sfxEnabled = enable;
}

void SoundMan_br::enableMusic(bool enable) {
	if (enable) {
		playMusic();
	} else {
		stopMusic();
	}
	_musicEnabled = enable;
}

bool SoundMan_br::isSfxEnabled() const {
	return _sfxEnabled;
}

bool SoundMan_br::isMusicEnabled() const {
	return _musicEnabled;
}

} // namespace Parallaction
