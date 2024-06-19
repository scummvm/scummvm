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
 * $URL: https://scummvm.svn.sourceforge.net/svnroot/scummvm/scummvm/trunk/engines/m4/midi.cpp $
 * $Id: midi.cpp 47705 2010-01-30 09:21:07Z dreammaster $
 *
 */

// FIXME: This is cribbed together from the SAGA music player. It needs cleanup
// and testing.

#include "prisoner/prisoner.h"
#include "prisoner/midi.h"

#include "common/stream.h"

namespace Prisoner {

MidiPlayer::MidiPlayer(PrisonerEngine *vm, MidiDriver *driver) : _vm(vm), _driver(driver), _isPlaying(false), _passThrough(false), _isGM(false) {
	memset(_channel, 0, sizeof(_channel));
	_masterVolume = 0;
	_parser = MidiParser::createParser_SMF();
	_parser->setMidiDriver(this);
	_parser->setTimerRate(getBaseTempo());
	open();
}

MidiPlayer::~MidiPlayer() {
	_driver->setTimerCallback(NULL, NULL);
	_parser->setMidiDriver(NULL);
	stopMusic();
	close();
	delete _parser;
}

void MidiPlayer::setVolume(int volume) {
	Common::StackLock lock(_mutex);

	if (volume < 0)
		volume = 0;
	else if (volume > 255)
		volume = 255;

	if (_masterVolume == volume)
		return;

	_masterVolume = volume;

	for (int i = 0; i < 16; ++i) {
		if (_channel[i]) {
			_channel[i]->volume(_channelVolume[i] * _masterVolume / 255);
		}
	}
}

bool MidiPlayer::isOpen() const {
	return _driver != 0;
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
	stopMusic();
	if (_driver)
		_driver->close();
	_driver = 0;
}

void MidiPlayer::send(uint32 b) {
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

void MidiPlayer::metaEvent(byte type, byte *data, uint16 length) {
	switch (type) {
	case 0x2F:
		// End of track. (Not called when auto-looping.)
		stopMusic();
		break;
	case 0x51:
		// Set tempo. Handled by the standard MIDI parser already.
		break;
	default:
		warning("Unhandled meta event: %02x", type);
		break;
	}
}

void MidiPlayer::onTimer(void *refCon) {
	MidiPlayer *midi = (MidiPlayer *)refCon;
	Common::StackLock lock(midi->_mutex);

	if (midi->_isPlaying)
		midi->_parser->onTimer();
}

void MidiPlayer::playMusic(MidiResource *midiResource, int32 vol, bool loop) {
	stopMusic();

	_parser->loadMusic(midiResource->getMidiData(), midiResource->getMidiSize());
	_parser->property(MidiParser::mpAutoLoop, loop);

	setVolume(255);

	_isPlaying = true;
}

void MidiPlayer::stopMusic() {
	Common::StackLock lock(_mutex);

	_isPlaying = false;
	if (_parser) {
		_parser->unloadMusic();
	}

}

} // End of namespace Prisoner
