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

#include "common/config-manager.h"
#include "sound/audiocd.h"

namespace Groovie {

// MusicPlayer

MusicPlayer::MusicPlayer(GroovieEngine *vm) :
	_vm(vm), _isPlaying(false), _backgroundFileRef(0), _gameVolume(100),
	_prevCDtrack(0), _backgroundDelay(0) {
}

MusicPlayer::~MusicPlayer() {
	AudioCD.stop();
}

void MusicPlayer::playSong(uint32 fileref) {
	Common::StackLock lock(_mutex);

	// Set the volumes
	_fadingEndVolume = 100;
	_gameVolume = 100;

	// Play the referenced file once
	play(fileref, false);
}

void MusicPlayer::setBackgroundSong(uint32 fileref) {
	Common::StackLock lock(_mutex);

	debugC(1, kGroovieDebugMIDI | kGroovieDebugAll, "Groovie::Music: Changing the background song: %04X", fileref);
	_backgroundFileRef = fileref;
}

void MusicPlayer::frameTick() {
	if (_backgroundDelay > 0) {
		_backgroundDelay--;
		if (_backgroundDelay == 0)
			playSong(_backgroundFileRef);
	}
}

void MusicPlayer::setBackgroundDelay(uint16 delay) {
	_backgroundDelay = delay;
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
	// It was in the original interpreter, but it introduces a big delay
	// in the middle of the introduction, so it's disabled right now
	/*
	AudioCD.updateCD();
	while (AudioCD.isPlaying()) {
		// Wait a bit and try again
		_vm->_system->delayMillis(100);
		AudioCD.updateCD();
	}
	*/

	// Play the track starting at the requested offset (1000ms = 75 frames)
	AudioCD.play(track - 1, 1, startms * 75 / 1000, 0);
}

void MusicPlayer::startBackground() {
	debugC(3, kGroovieDebugMIDI | kGroovieDebugAll, "Groovie::Music: startBackground()");
	if (!_isPlaying && _backgroundFileRef) {
		debugC(3, kGroovieDebugMIDI | kGroovieDebugAll, "Groovie::Music: Starting the background song (0x%4X)", _backgroundFileRef);
		play(_backgroundFileRef, true);
	}
}

void MusicPlayer::setUserVolume(uint16 volume) {
	Common::StackLock lock(_mutex);

	// Save the new user volume
	_userVolume = volume;
	if (_userVolume > 0x100)
		_userVolume = 0x100;

	// Apply it
	updateVolume();
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

bool MusicPlayer::play(uint32 fileref, bool loop) {
	// Unload the previous song
	unload();

	// Set the new state
	_isPlaying = true;

	// Load the new file
	return load(fileref, loop);
}

void MusicPlayer::applyFading() {
	debugC(6, kGroovieDebugMIDI | kGroovieDebugAll, "Groovie::Music: applyFading() _fadingStartTime = %d, _fadingDuration = %d, _fadingStartVolume = %d, _fadingEndVolume = %d", _fadingStartTime, _fadingDuration, _fadingStartVolume, _fadingEndVolume);
	Common::StackLock lock(_mutex);

	// Calculate the passed time
	uint32 time = _vm->_system->getMillis() - _fadingStartTime;
	debugC(6, kGroovieDebugMIDI | kGroovieDebugAll, "Groovie::Music: time = %d, _gameVolume = %d", time, _gameVolume);
	if (time >= _fadingDuration) {
		// Set the end volume
		_gameVolume = _fadingEndVolume;
	} else {
		// Calculate the interpolated volume for the current time
		_gameVolume = (_fadingStartVolume * (_fadingDuration - time) +
			_fadingEndVolume * time) / _fadingDuration;
	}
	if (_gameVolume == _fadingEndVolume) {
		// If we were fading to 0, stop the playback and restore the volume
		if (_fadingEndVolume == 0) {
			debugC(1, kGroovieDebugMIDI | kGroovieDebugAll, "Groovie::Music: Faded to zero: end of song. _fadingEndVolume set to 100");
			unload();
		}
	}

	// Apply it
	updateVolume();
}

void MusicPlayer::onTimer(void *refCon) {
	debugC(9, kGroovieDebugMIDI | kGroovieDebugAll, "Groovie::Music: onTimer()");
	MusicPlayer *music = (MusicPlayer *)refCon;
	Common::StackLock lock(music->_mutex);

	// Apply the game volume fading
	if (music->_gameVolume != music->_fadingEndVolume) {
		// Apply the next step of the fading
		music->applyFading();
	}

	// Handle internal timed events
	music->onTimerInternal();
}

void MusicPlayer::unload() {
	debugC(1, kGroovieDebugMIDI | kGroovieDebugAll, "Groovie::Music: Stopping the playback");

	// Set the new state
	_isPlaying = false;
}


// MusicPlayerMidi

MusicPlayerMidi::MusicPlayerMidi(GroovieEngine *vm) :
	MusicPlayer(vm), _midiParser(NULL), _data(NULL), _driver(NULL) {
	// Initialize the channel volumes
	for (int i = 0; i < 0x10; i++) {
		_chanVolumes[i] = 0x7F;
	}
}

MusicPlayerMidi::~MusicPlayerMidi() {
	// Stop the callback
	if (_driver)
		_driver->setTimerCallback(NULL, NULL);

	Common::StackLock lock(_mutex);

	// Unload the parser
	unload();
	delete _midiParser;

	// Unload the MIDI Driver
	if (_driver)
		_driver->close();
	delete _driver;
}

int MusicPlayerMidi::open() {
	// Don't ever call open without first setting the output driver!
	if (!_driver)
		return 255;

	int ret = _driver->open();
	if (ret)
		return ret;

	return 0;
}

void MusicPlayerMidi::close() {}

void MusicPlayerMidi::send(uint32 b) {
	if ((b & 0xFFF0) == 0x07B0) { // Volume change
		// Save the specific channel volume
		byte chan = b & 0xF;
		_chanVolumes[chan] = (b >> 16) & 0x7F;

		// Send the updated value
		updateChanVolume(chan);

		return;
	}
	if (_driver)
		_driver->send(b);
}

void MusicPlayerMidi::metaEvent(byte type, byte *data, uint16 length) {
	switch (type) {
	case 0x2F:
		// End of Track, play the background song
		endTrack();
		break;
	default:
		if (_driver)
			_driver->metaEvent(type, data, length);
		break;
	}
}

void MusicPlayerMidi::setTimerCallback(void *timer_param, Common::TimerManager::TimerProc timer_proc) {
	if (_driver)
		_driver->setTimerCallback(timer_param, timer_proc);
}

uint32 MusicPlayerMidi::getBaseTempo(void) {
	if (_driver)
		return _driver->getBaseTempo();
	else
		return 0;
}

MidiChannel *MusicPlayerMidi::allocateChannel() {
	if (_driver)
		return _driver->allocateChannel();
	else
		return 0;
}

MidiChannel *MusicPlayerMidi::getPercussionChannel() {
	if (_driver)
		return _driver->getPercussionChannel();
	else
		return 0;
}

void MusicPlayerMidi::updateChanVolume(byte channel) {
	// Generate a MIDI Control change message for the volume
	uint32 b = 0x7B0;

	// Specify the channel
	b |= (channel & 0xF);

	// Scale by the user and game volumes
	uint32 val = (_chanVolumes[channel] * _userVolume * _gameVolume) / 0x100 / 100;
	val &= 0x7F;

	// Send it to the driver
	if (_driver)
		_driver->send(b | (val << 16));
}

void MusicPlayerMidi::endTrack() {
	debugC(3, kGroovieDebugMIDI | kGroovieDebugAll, "Groovie::Music: endTrack()");
	unload();
}

void MusicPlayerMidi::onTimerInternal() {
	// TODO: We really only need to call this while music is playing.
	if (_midiParser)
		_midiParser->onTimer();
}

void MusicPlayerMidi::updateVolume() {
	// Apply it to all the channels
	for (int i = 0; i < 0x10; i++) {
		updateChanVolume(i);
	}
}

void MusicPlayerMidi::unload() {
	MusicPlayer::unload();

	// Unload the parser data
	if (_midiParser)
		_midiParser->unloadMusic();

	// Unload the data
	delete[] _data;
	_data = NULL;
}

bool MusicPlayerMidi::loadParser(Common::SeekableReadStream *stream, bool loop) {
	if (!_midiParser)
		return false;

	// Read the whole file to memory
	int length = stream->size();
	_data = new byte[length];
	stream->read(_data, length);
	delete stream;

	// Set the looping option
	_midiParser->property(MidiParser::mpAutoLoop, loop);

	// Start parsing the data
	if (!_midiParser->loadMusic(_data, length)) {
		error("Groovie::Music: Couldn't parse the data");
		return false;
	}

	// Activate the timer source
	if (_driver)
		_driver->setTimerCallback(this, &onTimer);

	return true;
}


// MusicPlayerXMI

MusicPlayerXMI::MusicPlayerXMI(GroovieEngine *vm, const Common::String &gtlName) :
	MusicPlayerMidi(vm) {
	// Create the parser
	_midiParser = MidiParser::createParser_XMIDI();

	// Create the driver
	int driver = detectMusicDriver(MDT_MIDI | MDT_ADLIB | MDT_PREFER_MIDI);
	_driver = createMidi(driver);
	this->open();

	// Set the parser's driver
	_midiParser->setMidiDriver(this);

	// Set the timer rate
	_midiParser->setTimerRate(_driver->getBaseTempo());

	// Initialize the channel banks
	for (int i = 0; i < 0x10; i++) {
		_chanBanks[i] = 0;
	}

	// Load the Global Timbre Library
	if (driver == MD_ADLIB) {
		// MIDI through AdLib
		_musicType = MD_ADLIB;
		loadTimbres(gtlName + ".ad");

		// Setup the percussion channel
		for (unsigned int i = 0; i < _timbres.size(); i++) {
			if (_timbres[i].bank == 0x7F)
				setTimbreAD(9, _timbres[i]);
		}
	} else if ((driver == MD_MT32) || ConfMan.getBool("native_mt32")) {
		// MT-32
		_musicType = MD_MT32;
		loadTimbres(gtlName + ".mt");
	} else {
		// GM
		_musicType = 0;
	}
}

MusicPlayerXMI::~MusicPlayerXMI() {
	//~MusicPlayer();

	// Unload the timbres
	clearTimbres();
}

void MusicPlayerXMI::send(uint32 b) {
	if ((b & 0xFFF0) == 0x72B0) { // XMIDI Patch Bank Select 114
		// From AIL2's documentation: XMIDI Patch Bank Select controller (114)
		// selects a bank to be used when searching the next patches
		byte chan = b & 0xF;
		byte bank = (b >> 16) & 0xFF;

		debugC(5, kGroovieDebugMIDI | kGroovieDebugAll, "Groovie::Music: Selecting bank %X for channel %X", bank, chan);
		_chanBanks[chan] = bank;
		return;
	} else if ((b & 0xF0) == 0xC0) { // Program change
		// We intercept the program change when using AdLib or MT32 drivers,
		// since we have custom timbres for them.  The command is sent
		// unchanged to GM drivers.
		if (_musicType != 0) {
			byte chan = b & 0xF;
			byte patch = (b >> 8) & 0xFF;

			debugC(5, kGroovieDebugMIDI | kGroovieDebugAll, "Groovie::Music: Setting custom patch %X from bank %X to channel %X", patch, _chanBanks[chan], chan);

			// Try to find the requested patch from the previously
			// specified bank
			int numTimbres = _timbres.size();
			for (int i = 0; i < numTimbres; i++) {
				if ((_timbres[i].bank == _chanBanks[chan]) &&
					(_timbres[i].patch == patch)) {
					if (_musicType == MD_ADLIB) {
						setTimbreAD(chan, _timbres[i]);
					} else if (_musicType == MD_MT32) {
						setTimbreMT(chan, _timbres[i]);
					}
					return;
				}
			}

			// If we got here we couldn't find the patch, and the
			// received message will be sent unchanged.
		}
	}
	MusicPlayerMidi::send(b);
}

bool MusicPlayerXMI::load(uint32 fileref, bool loop) {
	debugC(1, kGroovieDebugMIDI | kGroovieDebugAll, "Groovie::Music: Starting the playback of song: %04X", fileref);

	// Open the song resource
	Common::SeekableReadStream *file = _vm->_resMan->open(fileref);
	if (!file) {
		error("Groovie::Music: Couldn't find resource 0x%04X", fileref);
		return false;
	}

	return loadParser(file, loop);
}

void MusicPlayerXMI::loadTimbres(const Common::String &filename) {
	// Load the Global Timbre Library format as documented in AIL2
	debugC(1, kGroovieDebugMIDI | kGroovieDebugAll, "Groovie::Music: Loading the GTL file %s", filename.c_str());

	// Does it exist?
	if (!Common::File::exists(filename)) {
		error("Groovie::Music: %s not found", filename.c_str());
		return;
	}

	// Open the GTL
	Common::File *gtl = new Common::File();
	if (!gtl->open(filename.c_str())) {
		delete gtl;
		error("Groovie::Music: Couldn't open %s", filename.c_str());
		return;
	}

	// Clear the old timbres before loading the new ones
	clearTimbres();

	// Get the list of timbres
	while (true) {
		Timbre t;
		t.patch = gtl->readByte();
		t.bank = gtl->readByte();
		if ((t.patch == 0xFF) && (t.bank == 0xFF)) {
			// End of list
			break;
		}
		// We temporarily use the size field to store the offset
		t.size = gtl->readUint32LE();

		// Add it to the list
		_timbres.push_back(t);
	}

	// Read the timbres
	for (unsigned int i = 0; i < _timbres.size(); i++) {
		// Seek to the start of the timbre
		gtl->seek(_timbres[i].size);

		// Read the size
		_timbres[i].size = gtl->readUint16LE() - 2;

		// Allocate memory for the timbre data
		_timbres[i].data = new byte[_timbres[i].size];

		// Read the timbre data
		gtl->read(_timbres[i].data, _timbres[i].size);
		debugC(5, kGroovieDebugMIDI | kGroovieDebugAll, "Groovie::Music: Loaded patch %x in bank %x with size %d",
			_timbres[i].patch, _timbres[i].bank, _timbres[i].size);
	}

	// Close the file
	delete gtl;
}

void MusicPlayerXMI::clearTimbres() {
	// Delete the allocated data
	int num = _timbres.size();
	for (int i = 0; i < num; i++) {
		delete[] _timbres[i].data;
	}

	// Erase the array entries
	_timbres.clear();
}

void MusicPlayerXMI::setTimbreAD(byte channel, const Timbre &timbre) {
	// Verify the timbre size
	if (timbre.size != 12) {
		error("Groovie::Music: Invalid size for an AdLib timbre: %d", timbre.size);
	}

	// Prepare the AdLib Instrument array from the GTL entry
	byte data[13];
	data[2] = timbre.data[1];        // mod_characteristic
	data[3] = timbre.data[2] ^ 0x3F; // mod_scalingOutputLevel
	data[4] = ~timbre.data[3];       // mod_attackDecay
	data[5] = ~timbre.data[4];       // mod_sustainRelease
	data[6] = timbre.data[5];        // mod_waveformSelect
	data[7] = timbre.data[7];        // car_characteristic
	data[8] = timbre.data[8] ^ 0x3F; // car_scalingOutputLevel
	data[9] = ~timbre.data[9];       // car_attackDecay
	data[10] = ~timbre.data[10];     // car_sustainRelease
	data[11] = timbre.data[11];      // car_waveformSelect
	data[12] = timbre.data[6];       // feedback

	// Send the instrument to the driver
	if (timbre.bank == 0x7F) {
		// This is a Percussion instrument, this will always be set on the same note
		data[0] = timbre.patch;

		// From AIL2's documentation: If the instrument is to be played in MIDI
		// channel 10, num specifies its desired absolute MIDI note number.
		data[1] = timbre.data[0];

		_driver->getPercussionChannel()->sysEx_customInstrument('ADLP', data);
	} else {
		// Some tweaks for non-percussion instruments
		byte mult1 = timbre.data[1] & 0xF;
		if (mult1 < 4)
			mult1 = 1 << mult1;
		data[2] = (timbre.data[1] & 0xF0) + (mult1 & 0xF);
		byte mult2 = timbre.data[7] & 0xF;
		if (mult2 < 4)
			mult2 = 1 << mult2;
		data[7] = (timbre.data[7] & 0xF0) + (mult2 & 0xF);
		// TODO: Fix CHARACTERISTIC: 0xF0: pitch_vib, amp_vib, sustain_sound, env_scaling  0xF: freq_mult
		// TODO: Fix KSL_TL: 0xC: key_scale_lvl  0x3F: out_lvl

		// From AIL2's documentation: num specifies the number of semitones
		// by which to transpose notes played with the instrument.
		if (timbre.data[0] != 0)
			warning("Groovie::Music: AdLib instrument's transposing not supported");

		_driver->sysEx_customInstrument(channel, 'ADL ', data + 2);
	}
}

void MusicPlayerXMI::setTimbreMT(byte channel, const Timbre &timbre) {
	// Verify the timbre size
	if (timbre.size != 0xF6) {
		error("Groovie::Music: Invalid size for an MT-32 timbre: %d", timbre.size);
	}

	// Show the timbre name as extra debug information
	Common::String name((char*)timbre.data, 10);
	debugC(5, kGroovieDebugMIDI | kGroovieDebugAll, "Groovie::Music: Using MT32 timbre: %s", name.c_str());

	// TODO: Support MT-32 custom instruments
	warning("Groovie::Music: Setting MT32 custom instruments isn't supported yet");
}


// MusicPlayerMac

MusicPlayerMac::MusicPlayerMac(GroovieEngine *vm) :
	MusicPlayerMidi(vm) {
}

bool MusicPlayerMac::load(uint32 fileref, bool loop) {
	debugC(1, kGroovieDebugMIDI | kGroovieDebugAll, "Groovie::Music: Starting the playback of song: %04X", fileref);
	debug("Groovie::Music: Starting the playback of song: %04X %d", fileref, fileref);

	// Open the song resource
	/*
	Common::SeekableReadStream *file = _vm->_resMan->open(fileref);
	if (!file) {
		error("Groovie::Music: Couldn't find resource 0x%04X", fileref);
		return false;
	}
	*/

	//return loadParser(file, loop);
	return false;
}

} // End of Groovie namespace
