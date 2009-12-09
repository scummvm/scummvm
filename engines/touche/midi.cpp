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

#include "common/config-manager.h"
#include "common/stream.h"

#include "sound/midiparser.h"

#include "touche/midi.h"

namespace Touche {

MidiPlayer::MidiPlayer()
	: _driver(0), _parser(0), _midiData(0), _isLooping(false), _isPlaying(false), _masterVolume(0) {
	memset(_channelsTable, 0, sizeof(_channelsTable));
	memset(_channelsVolume, 0, sizeof(_channelsVolume));
	open();
}

MidiPlayer::~MidiPlayer() {
	close();
}

void MidiPlayer::play(Common::ReadStream &stream, int size, bool loop) {
	stop();
	_midiData = (uint8 *)malloc(size);
	if (_midiData) {
		stream.read(_midiData, size);
		_mutex.lock();
		_parser->loadMusic(_midiData, size);
		_parser->setTrack(0);
		_isLooping = loop;
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
	MidiDriverType midiDriver = MidiDriver::detectMusicDriver(MDT_MIDI | MDT_ADLIB | MDT_PREFER_MIDI);
	_nativeMT32 = ((midiDriver == MD_MT32) || ConfMan.getBool("native_mt32"));
	_driver = MidiDriver::createMidi(midiDriver);
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
	default:
		if ((b & 0xF0) == 0xC0 && _nativeMT32) { // program change
			b = (b & 0xFFFF00FF) | (_gmToRol[(b >> 8) & 0x7F] << 8);
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

} // Touche namespace
