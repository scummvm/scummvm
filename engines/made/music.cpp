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

// FIXME: This code is taken from SAGA and needs more work (e.g. setVolume).

// MIDI and digital music class

#include "sound/audiostream.h"
#include "sound/mididrv.h"
#include "sound/midiparser.h"
#include "common/config-manager.h"
#include "common/file.h"

#include "made/music.h"

namespace Made {

MusicPlayer::MusicPlayer(MidiDriver *driver) : _parser(0), _driver(driver), _looping(false), _isPlaying(false), _passThrough(false), _isGM(false) {
	memset(_channel, 0, sizeof(_channel));
	_masterVolume = 0;
	this->open();
}

MusicPlayer::~MusicPlayer() {
	_driver->setTimerCallback(NULL, NULL);
	stopMusic();
	this->close();
}

void MusicPlayer::setVolume(int volume) {
	volume = CLIP(volume, 0, 255);

	if (_masterVolume == volume)
		return;

	_masterVolume = volume;

	for (int i = 0; i < 16; ++i) {
		if (_channel[i]) {
			_channel[i]->volume(_channelVolume[i] * _masterVolume / 255);
		}
	}
}

int MusicPlayer::open() {
	// Don't ever call open without first setting the output driver!
	if (!_driver)
		return 255;

	int ret = _driver->open();
	if (ret)
		return ret;

	_driver->setTimerCallback(this, &onTimer);
	return 0;
}

void MusicPlayer::close() {
	stopMusic();
	if (_driver)
		_driver->close();
	_driver = 0;
}

void MusicPlayer::send(uint32 b) {
	if (_passThrough) {
		_driver->send(b);
		return;
	}

	byte channel = (byte)(b & 0x0F);
	if ((b & 0xFFF0) == 0x07B0) {
		// Adjust volume changes by master volume
		byte volume = (byte)((b >> 16) & 0x7F);
		_channelVolume[channel] = volume;
		volume = volume * _masterVolume / 255;
		b = (b & 0xFF00FFFF) | (volume << 16);
	} else if ((b & 0xF0) == 0xC0 && !_isGM && !_nativeMT32) {
		b = (b & 0xFFFF00FF) | MidiDriver::_mt32ToGm[(b >> 8) & 0xFF] << 8;
	}
	else if ((b & 0xFFF0) == 0x007BB0) {
		//Only respond to All Notes Off if this channel
		//has currently been allocated
		if (_channel[b & 0x0F])
			return;
	}

	if (!_channel[channel])
		_channel[channel] = (channel == 9) ? _driver->getPercussionChannel() : _driver->allocateChannel();

	if (_channel[channel])
		_channel[channel]->send(b);
}

void MusicPlayer::metaEvent(byte type, byte *data, uint16 length) {

	switch (type) {
	case 0x2F:	// End of Track
		if (_looping)
			_parser->jumpToTick(0);
		else
			stopMusic();
		break;
	default:
		//warning("Unhandled meta event: %02x", type);
		break;
	}
}

void MusicPlayer::onTimer(void *refCon) {
	MusicPlayer *music = (MusicPlayer *)refCon;
	Common::StackLock lock(music->_mutex);

	if (music->_isPlaying)
		music->_parser->onTimer();
}

void MusicPlayer::playMusic() {
	_isPlaying = true;
}

void MusicPlayer::stopMusic() {
	Common::StackLock lock(_mutex);

	_isPlaying = false;
	if (_parser) {
		_parser->unloadMusic();
		_parser = NULL;
	}
}

Music::Music(MidiDriver *driver, int enabled) : _enabled(enabled), _adlib(false) {
	_player = new MusicPlayer(driver);
	_currentVolume = 0;

	xmidiParser = MidiParser::createParser_XMIDI();

	_songTableLen = 0;
	_songTable = 0;

	_midiMusicData = NULL;
}

Music::~Music() {
	delete _player;
	xmidiParser->setMidiDriver(NULL);
	delete xmidiParser;

	free(_songTable);
	if (_midiMusicData)
		delete[] _midiMusicData;
}

void Music::setVolume(int volume) {
	if (volume == -1) // Set Full volume
		volume = 255;

	_player->setVolume(volume);
	_currentVolume = volume;
}

bool Music::isPlaying() {
	return _player->isPlaying();
}

void Music::play(XmidiResource *midiResource, MusicFlags flags) {
	MidiParser *parser = 0;
	byte *resourceData;
	size_t resourceSize;

	debug(2, "Music::play %d", flags);

	if (!_enabled || isPlaying()) {
		return;
	}

	_player->stopMusic();

	/*
	if (!_vm->_musicVolume) {
		return;
	}
	*/

	if (flags == MUSIC_DEFAULT) {
		flags = MUSIC_NORMAL;
	}

	// Load MIDI/XMI resource data

	_player->setGM(true);

	resourceSize = midiResource->getSize();
	resourceData = new byte[resourceSize];
	memcpy(resourceData, midiResource->getData(), resourceSize);

	if (resourceSize < 4) {
		error("Music::play() wrong music resource size");
	}

	if (xmidiParser->loadMusic(resourceData, resourceSize)) {
		//_player->setGM(false);
		parser = xmidiParser;
	}

	parser->setTrack(0);
	parser->setMidiDriver(_player);
	parser->setTimerRate(_player->getBaseTempo());
	parser->property(MidiParser::mpCenterPitchWheelOnUnload, 1);

	_player->_parser = parser;
	//setVolume(_vm->_musicVolume == 10 ? 255 : _vm->_musicVolume * 25);
	setVolume(255);

	if (flags & MUSIC_LOOP)
		_player->setLoop(true);
	else
		_player->setLoop(false);

	_player->playMusic();
	if (_midiMusicData)
		delete[] _midiMusicData;
	_midiMusicData = resourceData;
}

void Music::pause(void) {
	_player->setVolume(-1);
	_player->setPlaying(false);
}

void Music::resume(void) {
	//_player->setVolume(_vm->_musicVolume == 10 ? 255 : _vm->_musicVolume * 25);
	setVolume(255);
	_player->setPlaying(true);
}

void Music::stop(void) {
	_player->stopMusic();
}

} // End of namespace Made
