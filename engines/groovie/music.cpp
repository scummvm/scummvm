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
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.
 *
 */

#include "audio/mididrv.h"
#include "audio/mixer.h"

#include "groovie/music.h"
#include "groovie/groovie.h"
#include "groovie/resource.h"

#include "backends/audiocd/audiocd.h"
#include "common/config-manager.h"
#include "common/debug.h"
#include "common/file.h"
#include "common/macresman.h"
#include "common/memstream.h"
#include "common/textconsole.h"
#include "audio/audiostream.h"
#include "audio/midiparser.h"
#include "audio/miles.h"

namespace Groovie {

// MusicPlayer

MusicPlayer::MusicPlayer(GroovieEngine *vm) :
	_vm(vm), _isPlaying(false), _backgroundFileRef(0), _gameVolume(100),
	_prevCDtrack(0), _backgroundDelay(0), _fadingStartTime(0), _fadingStartVolume(0),
	_fadingEndVolume(0), _fadingDuration(0), _midiInit(false), _userVolume(0) {
}

MusicPlayer::~MusicPlayer() {
	g_system->getAudioCDManager()->stop();
}

void MusicPlayer::playSong(uint32 fileref) {
	Common::StackLock lock(_mutex);

	// Set the volumes
	_fadingEndVolume = 100;
	_gameVolume = 100;
	updateVolume();

	// Play the referenced file once
	play(fileref, false);
}

void MusicPlayer::setBackgroundSong(uint32 fileref) {
	Common::StackLock lock(_mutex);

	debugC(1, kDebugMIDI, "Groovie::Music: Changing the background song: %04X", fileref);
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

	debugC(1, kDebugMIDI, "Groovie::Music: Playing CD track %d", track);

	if (track == 3) {
		// This is the credits song, start at 23:20
		startms = 1400000;
		// TODO: If we want to play it directly from the CD, we should decrement
		// the song number (it's track 2 on the 2nd CD)
	} else if ((track == 98) && (_prevCDtrack == 3)) {
		// Track 98 is used as a hack to stop the credits song
		g_system->getAudioCDManager()->stop();
		stopCreditsIOS();
		return;
	}

	// Save the playing track in order to be able to stop the credits song
	_prevCDtrack = track;

	// Wait until the CD stops playing the current song
	// It was in the original interpreter, but it introduces a big delay
	// in the middle of the introduction, so it's disabled right now
	/*
	g_system->getAudioCDManager()->updateCD();
	while (g_system->getAudioCDManager()->isPlaying()) {
		// Wait a bit and try again
		_vm->_system->delayMillis(100);
		g_system->getAudioCDManager()->updateCD();
	}
	*/

	// Play the track starting at the requested offset (1000ms = 75 frames)
	g_system->getAudioCDManager()->play(track - 1, 1, startms * 75 / 1000, 0);

	// If the audio is not playing from the CD, play the "fallback" MIDI.
	// The Mac version has no CD tracks, so it will always use the MIDI.
	if (!g_system->getAudioCDManager()->isPlaying()) {
		if (track == 2) {
			// Intro MIDI fallback
			if (_vm->getPlatform() == Common::kPlatformMacintosh)
				playSong(70);
			else
				playSong((19 << 10) | 36); // XMI.GJD, file 36
		} else if (track == 3) {
			// TODO: Credits MIDI fallback
			if (_vm->getPlatform() == Common::kPlatformIOS)
				playCreditsIOS();
		}
	}
}

void MusicPlayer::startBackground() {
	debugC(3, kDebugMIDI, "Groovie::Music: startBackground()");
	if (!_isPlaying && _backgroundFileRef) {
		debugC(3, kDebugMIDI, "Groovie::Music: Starting the background song (0x%4X)", _backgroundFileRef);
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

	debugC(1, kDebugMIDI, "Groovie::Music: Setting game volume from %d to %d in %dms", _gameVolume, volume, time);

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

void MusicPlayer::stop() {
	_backgroundFileRef = 0;
	unload();
}

void MusicPlayer::applyFading() {
	debugC(6, kDebugMIDI, "Groovie::Music: applyFading() _fadingStartTime = %d, _fadingDuration = %d, _fadingStartVolume = %d, _fadingEndVolume = %d", _fadingStartTime, _fadingDuration, _fadingStartVolume, _fadingEndVolume);
	Common::StackLock lock(_mutex);

	// Calculate the passed time
	uint32 time = _vm->_system->getMillis() - _fadingStartTime;
	debugC(6, kDebugMIDI, "Groovie::Music: time = %d, _gameVolume = %d", time, _gameVolume);
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
			debugC(1, kDebugMIDI, "Groovie::Music: Faded to zero: end of song. _fadingEndVolume set to 100");
			// WORKAROUND The original interpreter would keep playing a track
			// at volume 0 after it has faded out. When a new track was
			// started, it would restore the volume first and a short part of
			// the old track would be heard before the new track would start.
			// To prevent this, playback is actually stopped after fading out,
			// but _isPlaying remains true. This keeps the original
			// interpreter behavior of not starting the background music, but 
			// it prevents the issue when starting playback of a new track.
			unload(false);
		}
	}

	// Apply it
	updateVolume();
}

void MusicPlayer::onTimer(void *refCon) {
	debugC(9, kDebugMIDI, "Groovie::Music: onTimer()");
	MusicPlayer *music = (MusicPlayer *)refCon;
	Common::StackLock lock(music->_mutex);

	// Apply the game volume fading
	if (music->_gameVolume != music->_fadingEndVolume) {
		// Apply the next step of the fading
		music->applyFading();
	}

	// If the game is accepting user input, start the background music if necessary
	if (music->_vm->isWaitingForInput())
		music->startBackground();

	// Handle internal timed events
	music->onTimerInternal();
}

void MusicPlayer::unload(bool updateState) {
	debugC(1, kDebugMIDI, "Groovie::Music: Stopping the playback");

	if (updateState)
		// Set the new state
		_isPlaying = false;
}

void MusicPlayer::playCreditsIOS() {
	Audio::AudioStream *stream = Audio::SeekableAudioStream::openStreamFile("7th_Guest_Dolls_from_Hell_OC_ReMix");

	if (!stream) {
		warning("Could not find '7th_Guest_Dolls_from_Hell_OC_ReMix' audio file");
		return;
	}

	_vm->_system->getMixer()->playStream(Audio::Mixer::kMusicSoundType, &_handleCreditsIOS, stream);
}

void MusicPlayer::stopCreditsIOS() {
	_vm->_system->getMixer()->stopHandle(_handleCreditsIOS);
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
	if (_driver) {
		_driver->close();
		delete _driver;
	}
}

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

void MusicPlayerMidi::sysEx(const byte *msg, uint16 length) {
	if (_driver)
		_driver->sysEx(msg, length);
}

uint16 MusicPlayerMidi::sysExNoDelay(const byte *msg, uint16 length) {
	return _driver ? _driver->sysExNoDelay(msg, length) : 0;
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

void MusicPlayerMidi::pause(bool pause) {
	if (_midiParser) {
		if (pause) {
			_midiParser->pausePlaying();
		} else {
			_midiParser->resumePlaying();
		}
	}
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
	debugC(3, kDebugMIDI, "Groovie::Music: endTrack()");
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

void MusicPlayerMidi::unload(bool updateState) {
	MusicPlayer::unload(updateState);

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
	MusicPlayerMidi(vm),
	_milesMidiDriver(NULL) {

	// Create the driver
	MidiDriver::DeviceHandle dev = MidiDriver::detectDevice(MDT_MIDI | MDT_ADLIB | MDT_PREFER_GM);
	MusicType musicType = MidiDriver::getMusicType(dev);
	if (musicType == MT_GM && ConfMan.getBool("native_mt32"))
		musicType = MT_MT32;
	_driver = NULL;

	_musicType = 0;

	// 7th Guest uses FAT.AD/FAT.OPL/FAT.MT
	// 11th Hour uses SAMPLE.AD/SAMPLE.OPL/SAMPLE.MT
	switch (musicType) {
	case MT_ADLIB:
		// TODO Would be nice if the Miles AdLib and MIDI drivers shared
		// a common interface, then we can use only _milesMidiDriver in
		// this class.
		_driver = Audio::MidiDriver_Miles_AdLib_create(gtlName + ".AD", gtlName + ".OPL");
		break;
	case MT_MT32:
		_driver = _milesMidiDriver = Audio::MidiDriver_Miles_MIDI_create(musicType, gtlName + ".MT");
		break;
	case MT_GM:
		_driver = _milesMidiDriver = Audio::MidiDriver_Miles_MIDI_create(musicType, "");
		break;
	case MT_NULL:
		_driver = MidiDriver::createMidi(dev);
		break;
	default:
		break;
	}
	_musicType = musicType;

	assert(_driver);

	// Create the parser
	_midiParser = MidiParser::createParser_XMIDI(NULL, NULL, 0);

	int result = _driver->open();
	if (result > 0 && result != MidiDriver::MERR_ALREADY_OPEN)
		error("Opening MidiDriver failed with error code %i", result);

	// Set the parser's driver
	_midiParser->setMidiDriver(this);

	// Set the timer rate
	_midiParser->setTimerRate(_driver->getBaseTempo());
}

MusicPlayerXMI::~MusicPlayerXMI() {
	_midiParser->stopPlaying();
}

void MusicPlayerXMI::send(int8 source, uint32 b) {
	if (_milesMidiDriver) {
		_milesMidiDriver->send(source, b);
	} else {
		MusicPlayerMidi::send(b);
	}
}

void MusicPlayerXMI::metaEvent(int8 source, byte type, byte *data, uint16 length) {
	if (_milesMidiDriver) {
		if (type == 0x2F) // End Of Track
			MusicPlayerMidi::endTrack();
		_milesMidiDriver->metaEvent(source, type, data, length);
	} else {
		MusicPlayerMidi::metaEvent(type, data, length);
	}
}

void MusicPlayerXMI::stopAllNotes(bool stopSustainedNotes) {
	if (_driver)
		_driver->stopAllNotes(stopSustainedNotes);
}

bool MusicPlayerXMI::isReady() {
	return _driver ? _driver->isReady() : false;
}

void MusicPlayerXMI::updateVolume() {
	if (_milesMidiDriver) {
		uint16 val = (_userVolume * _gameVolume) / 100;
		_milesMidiDriver->setSourceVolume(0, val);
	} else {
		MusicPlayerMidi::updateVolume();
	}
}

bool MusicPlayerXMI::load(uint32 fileref, bool loop) {
	debugC(1, kDebugMIDI, "Groovie::Music: Starting the playback of song: %04X", fileref);

	// Open the song resource
	Common::SeekableReadStream *file = _vm->_resMan->open(fileref);
	if (!file) {
		error("Groovie::Music: Couldn't find resource 0x%04X", fileref);
		return false;
	}

	return loadParser(file, loop);
}

void MusicPlayerXMI::unload(bool updateState) {
	MusicPlayerMidi::unload(updateState);
	if (_milesMidiDriver) {
		_milesMidiDriver->deinitSource(0);
	}
}

// MusicPlayerMac_t7g

MusicPlayerMac_t7g::MusicPlayerMac_t7g(GroovieEngine *vm) : MusicPlayerMidi(vm) {
	// Create the parser
	_midiParser = MidiParser::createParser_SMF();

	// Create the driver
	MidiDriver::DeviceHandle dev = MidiDriver::detectDevice(MDT_MIDI | MDT_ADLIB | MDT_PREFER_GM);
	_driver = MidiDriver::createMidi(dev);
	assert(_driver);

	_driver->open();	// TODO: Handle return value != 0 (indicating an error)

	// Set the parser's driver
	_midiParser->setMidiDriver(this);

	// Set the timer rate
	_midiParser->setTimerRate(_driver->getBaseTempo());

	// Sanity check
	assert(_vm->_macResFork);
}

bool MusicPlayerMac_t7g::load(uint32 fileref, bool loop) {
	debugC(1, kDebugMIDI, "Groovie::Music: Starting the playback of song: %04X", fileref);

	// First try for compressed MIDI
	Common::SeekableReadStream *file = _vm->_macResFork->getResource(MKTAG('c','m','i','d'), fileref & 0x3FF);

	if (file) {
		// Found the resource, decompress it
		Common::SeekableReadStream *tmp = decompressMidi(file);
		delete file;
		file = tmp;
	} else {
		// Otherwise, it's uncompressed
		file = _vm->_macResFork->getResource(MKTAG('M','i','d','i'), fileref & 0x3FF);
		if (!file)
			error("Groovie::Music: Couldn't find resource 0x%04X", fileref);
	}

	return loadParser(file, loop);
}

Common::SeekableReadStream *MusicPlayerMac_t7g::decompressMidi(Common::SeekableReadStream *stream) {
	// Initialize an output buffer of the given size
	uint32 size = stream->readUint32BE();
	byte *output = (byte *)malloc(size);

	byte *current = output;
	uint32 decompBytes = 0;
	while ((decompBytes < size) && !stream->eos()) {
		// 8 flags
		byte flags = stream->readByte();

		for (byte i = 0; (i < 8) && !stream->eos(); i++) {
			if (flags & 1) {
				// 1: Next byte is a literal
				*(current++) = stream->readByte();
				if (stream->eos())
					continue;
				decompBytes++;
			} else {
				// 0: It's a reference to part of the history
				uint16 args = stream->readUint16BE();
				if (stream->eos())
					continue;

				// Length = 4bit unsigned (3 minimal)
				uint8 length = (args >> 12) + 3;

				// Offset = 12bit signed (all values are negative)
				int16 offset = (args & 0xFFF) | 0xF000;

				// Copy from the past decompressed bytes
				decompBytes += length;
				while (length > 0) {
					*(current) = *(current + offset);
					current++;
					length--;
				}
			}
			flags = flags >> 1;
		}
	}

	// Return the output buffer wrapped in a MemoryReadStream
	return new Common::MemoryReadStream(output, size, DisposeAfterUse::YES);
}

// MusicPlayerMac_v2

MusicPlayerMac_v2::MusicPlayerMac_v2(GroovieEngine *vm) : MusicPlayerMidi(vm) {
	// Create the parser
	_midiParser = MidiParser::createParser_QT();

	// Create the driver
	MidiDriver::DeviceHandle dev = MidiDriver::detectDevice(MDT_MIDI | MDT_ADLIB | MDT_PREFER_GM);
	_driver = MidiDriver::createMidi(dev);
	assert(_driver);

	_driver->open();	// TODO: Handle return value != 0 (indicating an error)

	// Set the parser's driver
	_midiParser->setMidiDriver(this);

	// Set the timer rate
	_midiParser->setTimerRate(_driver->getBaseTempo());
}

bool MusicPlayerMac_v2::load(uint32 fileref, bool loop) {
	debugC(1, kDebugMIDI, "Groovie::Music: Starting the playback of song: %04X", fileref);

	// Find correct filename
	ResInfo info;
	_vm->_resMan->getResInfo(fileref, info);
	uint len = info.filename.size();
	if (len < 4)
		return false;	// This shouldn't actually occur

	// Remove the extension and add ".mov"
	info.filename.deleteLastChar();
	info.filename.deleteLastChar();
	info.filename.deleteLastChar();
	info.filename += "mov";

	Common::SeekableReadStream *file = SearchMan.createReadStreamForMember(info.filename);

	if (!file) {
		warning("Could not find file '%s'", info.filename.c_str());
		return false;
	}

	return loadParser(file, loop);
}

MusicPlayerIOS::MusicPlayerIOS(GroovieEngine *vm) : MusicPlayer(vm) {
	vm->getTimerManager()->installTimerProc(&onTimer, 50 * 1000, this, "groovieMusic");
}

MusicPlayerIOS::~MusicPlayerIOS() {
	_vm->getTimerManager()->removeTimerProc(&onTimer);
}

void MusicPlayerIOS::updateVolume() {
	// Just set the mixer volume for the music sound type
	_vm->_system->getMixer()->setVolumeForSoundType(Audio::Mixer::kMusicSoundType, _userVolume * _gameVolume / 100);
}

void MusicPlayerIOS::unload(bool updateState) {
	MusicPlayer::unload(updateState);

	_vm->_system->getMixer()->stopHandle(_handle);
}

bool MusicPlayerIOS::load(uint32 fileref, bool loop) {
	// Find correct filename
	ResInfo info;
	_vm->_resMan->getResInfo(fileref, info);
	uint len = info.filename.size();
	if (len < 4)
		return false;	// This shouldn't actually occur
	/*
	19462 door
	19463 ??
	19464 ??
	19465 puzzle?
	19466 cake
	19467 maze
	19468 ambient  (but not 69, amb b.  odd)
	19470 puzzle
	19471
	19473
	19475 coffins or blood pump
	19476 blood pump or coffins
	19493
	19499 chapel
	19509 downstair ambient
	19510 bedroom 'skip 3 and 5' puzzle (should loop from partway?)
	19514
	19515 bathroom drain teeth
	*/
	if ((fileref >= 19462 && fileref <= 19468) ||
		fileref == 19470 || fileref == 19471 ||
		fileref == 19473 || fileref == 19475 ||
		fileref == 19476 || fileref == 19493 ||
		fileref == 19499 || fileref == 19509 ||
		fileref == 19510 || fileref == 19514 ||
		fileref == 19515)
		loop = true; // XMIs for these refs self-loop

	// iOS port provides alternative intro sequence music
	if (info.filename == "gu39.xmi") {
		info.filename = "intro";
	} else if (info.filename == "gu32.xmi") {
		info.filename = "foyer";
	} else {
		// Remove the extension
		info.filename.deleteLastChar();
		info.filename.deleteLastChar();
		info.filename.deleteLastChar();
		info.filename.deleteLastChar();
	}

	// Create the audio stream
	Audio::SeekableAudioStream *seekStream = Audio::SeekableAudioStream::openStreamFile(info.filename);

	if (!seekStream) {
		warning("Could not play audio file '%s'", info.filename.c_str());
		return false;
	}

	Audio::AudioStream *audStream = seekStream;

	// Loop if requested
	if (loop)
		audStream = Audio::makeLoopingAudioStream(seekStream, 0);

	// MIDI player handles volume reset on load, IOS player doesn't - force update here
	updateVolume();

	// Play!
	_vm->_system->getMixer()->playStream(Audio::Mixer::kMusicSoundType, &_handle, audStream);
	return true;
}

} // End of Groovie namespace
