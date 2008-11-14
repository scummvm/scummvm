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

#include "groovie/music.h"
#include "groovie/resource.h"

namespace Groovie {

MusicPlayer::MusicPlayer(GroovieEngine *vm) :
	_vm(vm), _midiParser(NULL), _data(NULL), _driver(NULL),
	_backgroundFileRef(0) {
	// Create the parser
	_midiParser = MidiParser::createParser_XMIDI();

	// Create the driver
	int driver = detectMusicDriver(MDT_MIDI | MDT_ADLIB | MDT_PREFER_MIDI);
	_driver = createMidi(driver);
	_driver->open();

	// Initialize the channel volumes
	for (int i = 0; i < 0x10; i++) {
		_chanVolumes[i] = 0x7F;
	}

	// Set the parser's driver
	_midiParser->setMidiDriver(this);

	// Set the timer rate
	_midiParser->setTimerRate(_driver->getBaseTempo());
}

MusicPlayer::~MusicPlayer() {
	// Unload the parser
	unload();
	delete _midiParser;

	// Unload the MIDI Driver
	_driver->close();
	delete _driver;
}

void MusicPlayer::playSong(uint16 fileref) {
	// Play the referenced file once
	play(fileref, false);
}

void MusicPlayer::setBackgroundSong(uint16 fileref) {
	_backgroundFileRef = fileref;
}

void MusicPlayer::setUserVolume(uint16 volume) {
	// Save the new user volume
	_userVolume = volume;
	if (_userVolume > 0x100) _userVolume = 0x100;

	// Apply it to all the channels
	for (int i = 0; i < 0x10; i++) {
		updateChanVolume(i);
	}
	//FIXME: AdlibPercussionChannel::controlChange() is empty
	//(can't set the volume for the percusion channel)
}

void MusicPlayer::setGameVolume(uint16 volume, uint16 time) {
	//TODO: Implement volume fading
	debugC(5, kGroovieDebugMIDI | kGroovieDebugAll, "setting game volume: %d, %d\n", volume, time);

	// Save the new game volume
	_gameVolume = volume;
	if (_gameVolume > 100) _gameVolume = 100;

	// Apply it to all the channels
	for (int i = 0; i < 0x10; i++) {
		updateChanVolume(i);
	}
}

void MusicPlayer::updateChanVolume(byte channel) {
	// Generate a MIDI Control change message for the volume
	uint32 b = 0x7B0;

	// Specify the channel
	b |= (channel & 0xF);

	// Scale by the user and game volumes
	uint32 val = (_chanVolumes[channel] * _userVolume * _gameVolume) / 0x100 / 100;
	val &= 0x7F;

	// Send it to the driver
	_driver->send(b | (val << 16));
}

bool MusicPlayer::play(uint16 fileref, bool loop) {
	// Unload the previous song
	unload();

	// Set the looping option
	_midiParser->property(MidiParser::mpAutoLoop, loop);

	// Load the new file
	return load(fileref);
}

bool MusicPlayer::load(uint16 fileref) {
	// Open the song resource
	Common::SeekableReadStream *xmidiFile = _vm->_resMan->open(fileref);
	if (!xmidiFile) {
		error("Groovie::Music: Couldn't resource 0x%04X", fileref);
		return false;
	}

	// Read the whole file to memory
	int length = xmidiFile->size();
	_data = new byte[length];
	xmidiFile->read(_data, length);
	delete xmidiFile;

	// Start parsing the data
	if (!_midiParser->loadMusic(_data, length)) {
		error("Groovie::Music: Invalid XMI file");
		return false;
	}

	// Activate the timer source
	_driver->setTimerCallback(_midiParser, MidiParser::timerCallback);

	return true;
}

void MusicPlayer::unload() {
	// Unload the parser
	_midiParser->unloadMusic();

	// Unload the xmi file
	delete[] _data;
	_data = NULL;
}

int MusicPlayer::open() {
	return 0;
}

void MusicPlayer::close() {}

void MusicPlayer::send(uint32 b) {
	if ((b & 0xFFF0) == 0x07B0) { // Volume change
		// Save the specific channel volume
		byte chan = b & 0xF;
		_chanVolumes[chan] = (b >> 16) & 0x7F;

		// Send the updated value
		updateChanVolume(chan);

		return;
	}
	_driver->send(b);
}

void MusicPlayer::metaEvent(byte type, byte *data, uint16 length) {
	switch (type) {
	case 0x2F:
		// End of Track, play the background song
		if (_backgroundFileRef) {
			play(_backgroundFileRef, true);
		}
		break;
	default:
		_driver->metaEvent(type, data, length);
		break;
	}
}

void MusicPlayer::setTimerCallback(void *timer_param, Common::TimerManager::TimerProc timer_proc) {
	_driver->setTimerCallback(timer_param, timer_proc);
}

uint32 MusicPlayer::getBaseTempo(void) {
	return _driver->getBaseTempo();
}

MidiChannel *MusicPlayer::allocateChannel() {
	return _driver->allocateChannel();
}

MidiChannel *MusicPlayer::getPercussionChannel() {
	return _driver->getPercussionChannel();
}

} // End of Groovie namespace
