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

#include "sound/audiocd.h"

namespace Groovie {

MusicPlayer::MusicPlayer(GroovieEngine *vm) :
	_vm(vm), _midiParser(NULL), _data(NULL), _driver(NULL),
	_backgroundFileRef(0), _gameVolume(100), _prevCDtrack(0) {
	// Create the parser
	_midiParser = MidiParser::createParser_XMIDI();

	// Create the driver
	int driver = detectMusicDriver(MDT_MIDI | MDT_ADLIB | MDT_PREFER_MIDI);
	_driver = createMidi(driver);
	this->open();

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
	_driver->setTimerCallback(NULL, NULL);

	Common::StackLock lock(_mutex);

	// Unload the parser
	unload();
	delete _midiParser;

	// Unload the MIDI Driver
	_driver->close();
	delete _driver;
}

void MusicPlayer::playSong(uint16 fileref) {
	Common::StackLock lock(_mutex);

	// Play the referenced file once
	play(fileref, false);
}

void MusicPlayer::setBackgroundSong(uint16 fileref) {
	Common::StackLock lock(_mutex);

	debugC(1, kGroovieDebugMIDI | kGroovieDebugAll, "Groovie::Music: Changing the background song: %04X", fileref);
	_backgroundFileRef = fileref;
}

void MusicPlayer::playCD(uint8 track) {
	int startms = 0;

	// Stop the MIDI playback
	unload();

	debugC(1, kGroovieDebugMIDI | kGroovieDebugAll, "Groovie::Music: Playing CD track %d", track);

	if (track == 3) {
		// This is the credits song, start at 23:20
		startms = 1400000;
		// TODO: If we want to play it directly from the CD, we should decrement
		// the song number (it's track 2 on the 2nd CD)
	} else if ((track == 98) && (_prevCDtrack == 3)) {
		// Track 98 is used as a hack to stop the credits song
		AudioCD.stop();
		return;
	}

	// Save the playing track in order to be able to stop the credits song
	_prevCDtrack = track;

	// Wait until the CD stops playing the current song
	AudioCD.updateCD();
	while (AudioCD.isPlaying()) {
		// Wait a bit and try again
		_vm->_system->delayMillis(100);
		AudioCD.updateCD();
	}

	// Play the track starting at the requested offset (1000ms = 75 frames)
	AudioCD.play(track - 1, 1, startms * 75 / 1000, 0);
}

void MusicPlayer::setUserVolume(uint16 volume) {
	Common::StackLock lock(_mutex);

	// Save the new user volume
	_userVolume = volume;
	if (_userVolume > 0x100)
		_userVolume = 0x100;

	// Apply it to all the channels
	for (int i = 0; i < 0x10; i++) {
		updateChanVolume(i);
	}
	//FIXME: AdlibPercussionChannel::controlChange() is empty
	//(can't set the volume for the percusion channel)
}

void MusicPlayer::setGameVolume(uint16 volume, uint16 time) {
	Common::StackLock lock(_mutex);

	debugC(1, kGroovieDebugMIDI | kGroovieDebugAll, "Groovie::Music: Setting game volume from %d to %d in %dms", _gameVolume, volume, time);

	// Save the start parameters of the fade
	_fadingStartTime = _vm->_system->getMillis();
	_fadingStartVolume = _gameVolume;
	_fadingDuration = time;

	// Save the new game volume
	_fadingEndVolume = volume;
	if (_fadingEndVolume > 100)
		_fadingEndVolume = 100;
}

void MusicPlayer::applyFading() {
	Common::StackLock lock(_mutex);

	// Calculate the passed time
	uint32 time = _vm->_system->getMillis() - _fadingStartTime;
	if (time >= _fadingDuration) {
		// If we were fading to 0, stop the playback and restore the volume
		if (_fadingEndVolume == 0) {
			unload();
			_fadingEndVolume = 100;
		}

		// Set the end volume
		_gameVolume = _fadingEndVolume;
	} else {
		// Calculate the interpolated volume for the current time
		_gameVolume = (_fadingStartVolume * (_fadingDuration - time) +
			_fadingEndVolume * time) / _fadingDuration;
	}

	// Apply the new volume to all the channels
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
	debugC(1, kGroovieDebugMIDI | kGroovieDebugAll, "Groovie::Music: Starting the playback of song: %04X", fileref);

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
	_driver->setTimerCallback(this, &onTimer);

	return true;
}

void MusicPlayer::unload() {
	debugC(1, kGroovieDebugMIDI | kGroovieDebugAll, "Groovie::Music: Stopping the playback");

	// Unload the parser
	_midiParser->unloadMusic();

	// Unload the xmi file
	delete[] _data;
	_data = NULL;
}

int MusicPlayer::open() {
	// Don't ever call open without first setting the output driver!
	if (!_driver)
		return 255;

	int ret = _driver->open();
	if (ret)
		return ret;

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
		debugC(1, kGroovieDebugMIDI | kGroovieDebugAll, "Groovie::Music: End of song");
		if (_backgroundFileRef) {
			play(_backgroundFileRef, true);
		}
		break;
	default:
		_driver->metaEvent(type, data, length);
		break;
	}
}

void MusicPlayer::onTimer(void *refCon) {
	MusicPlayer *music = (MusicPlayer *)refCon;
	Common::StackLock lock(music->_mutex);

	// Apply the game volume fading
	if (music->_gameVolume != music->_fadingEndVolume) {
		// Apply the next step of the fading
		music->applyFading();
	}

	// TODO: We really only need to call this while music is playing.
	music->_midiParser->onTimer();
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
