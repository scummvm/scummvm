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

#include "audio/midiparser.h"

#include "touche/midi.h"

namespace Touche {

MidiPlayer::MidiPlayer()
	: _midiData(0) {

	// FIXME: Necessary?
	memset(_channelsVolume, 0, sizeof(_channelsVolume));

	MidiDriver::DeviceHandle dev = MidiDriver::detectDevice(MDT_MIDI | MDT_ADLIB | MDT_PREFER_GM);
	_nativeMT32 = ((MidiDriver::getMusicType(dev) == MT_MT32) || ConfMan.getBool("native_mt32"));
	_driver = MidiDriver::createMidi(dev);
	int ret = _driver->open();
	if (ret == 0) {
		_parser = MidiParser::createParser_SMF();
		_parser->setMidiDriver(this);
		_parser->setTimerRate(_driver->getBaseTempo());
		_driver->setTimerCallback(this, &timerCallback);

		if (_nativeMT32)
			_driver->sendMT32Reset();
		else
			_driver->sendGMReset();
	}
}

MidiPlayer::~MidiPlayer() {
	stop();

	Common::StackLock lock(_mutex);
	_driver->setTimerCallback(NULL, NULL);
	_driver->close();
	delete _driver;
	_driver = 0;
	_parser->setMidiDriver(NULL);
	delete _parser;
}

void MidiPlayer::play(Common::ReadStream &stream, int size, bool loop) {
	stop();
	_midiData = (uint8 *)malloc(size);
	if (_midiData) {
		stream.read(_midiData, size);

		Common::StackLock lock(_mutex);
		_parser->loadMusic(_midiData, size);
		_parser->setTrack(0);
		_isLooping = loop;
		_isPlaying = true;
	}
}

void MidiPlayer::stop() {
	Audio::MidiPlayer::stop();
	free(_midiData);
	_midiData = 0;
}

void MidiPlayer::updateTimer() {
	Common::StackLock lock(_mutex);
	if (_isPlaying) {
		_parser->onTimer();
	}
}

void MidiPlayer::adjustVolume(int diff) {
	setVolume(_masterVolume + diff);
}

void MidiPlayer::setVolume(int volume) {
	// FIXME: This is almost identical to Audio::MidiPlayer::setVolume,
	// the only difference is that this implementation will always
	// transmit the volume change, even if the current _masterVolume
	// equals the new master volume. This *could* make a difference in
	// some situations.
	// So, we should determine whether Touche requires this behavioral
	// difference; and maybe also if other engines could benefit from it
	// (as hypothetically, it might fix some subtle bugs?)
	_masterVolume = CLIP(volume, 0, 255);
	Common::StackLock lock(_mutex);
	for (int i = 0; i < kNumChannels; ++i) {
		if (_channelsTable[i]) {
			_channelsTable[i]->volume(_channelsVolume[i] * _masterVolume / 255);
		}
	}
}

void MidiPlayer::send(uint32 b) {
	if ((b & 0xF0) == 0xC0 && _nativeMT32) { // program change
		b = (b & 0xFFFF00FF) | (_gmToRol[(b >> 8) & 0x7F] << 8);
	}
	Audio::MidiPlayer::send(b);
}

void MidiPlayer::timerCallback(void *p) {
	MidiPlayer *player = (MidiPlayer *)p;
	player->updateTimer();
}

} // Touche namespace
