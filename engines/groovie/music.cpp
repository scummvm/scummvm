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

MusicPlayer::MusicPlayer(GroovieEngine *vm) :
	_vm(vm), _midiParser(NULL), _data(NULL), _driver(NULL),
	_backgroundFileRef(0), _gameVolume(100), _prevCDtrack(0) {
	// Create the parser
	_midiParser = MidiParser::createParser_XMIDI();

	// Create the driver
	int driver = detectMusicDriver(MDT_MIDI | MDT_ADLIB | MDT_PREFER_MIDI);
	_driver = createMidi(driver);
	this->open();

	// Initialize the channel volumes and banks
	for (int i = 0; i < 0x10; i++) {
		_chanVolumes[i] = 0x7F;
		_chanBanks[i] = 0;
	}

	// Load the Global Timbre Library
	if (driver == MD_ADLIB) {
		// MIDI through AdLib
		_musicType = MD_ADLIB;
		loadTimbres("fat.ad");

		// Setup the percussion channel
		for (unsigned int i = 0; i < _timbres.size(); i++) {
			if (_timbres[i].bank == 0x7F)
				setTimbreAD(9, _timbres[i]);
		}
	} else if ((driver == MD_MT32) || ConfMan.getBool("native_mt32")) {
		// MT-32
		_musicType = MD_MT32;
		loadTimbres("fat.mt");
	} else {
		// GM
		_musicType = 0;
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

	// Unload the timbres
	clearTimbres();
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
		error("Groovie::Music: Couldn't find resource 0x%04X", fileref);
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
	} else if ((b & 0xFFF0) == 0x07B0) { // Volume change
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

void MusicPlayer::loadTimbres(const Common::String &filename) {
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

void MusicPlayer::clearTimbres() {
	// Delete the allocated data
	int num = _timbres.size();
	for (int i = 0; i < num; i++) {
		delete[] _timbres[i].data;
	}

	// Erase the array entries
	_timbres.clear();
}

void MusicPlayer::setTimbreAD(byte channel, const Timbre &timbre) {
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

void MusicPlayer::setTimbreMT(byte channel, const Timbre &timbre) {
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

} // End of Groovie namespace
