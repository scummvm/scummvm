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
 */
 
#include "startrek/sound.h"

#include "common/file.h"
#include "common/macresman.h"

#include "audio/mods/protracker.h"
#include "audio/decoders/raw.h"

namespace StarTrek {

// Main Sound Functions

Sound::Sound(StarTrekEngine *vm) : _vm(vm) {
	if (_vm->getPlatform() == Common::kPlatformDOS || _vm->getPlatform() == Common::kPlatformMacintosh) {
		_midiDevice = MidiDriver::detectDevice(MDT_PCSPK|MDT_ADLIB|MDT_MIDI);
		_midiDriver = MidiDriver::createMidi(_midiDevice);
		_midiDriver->open();
		_midiDriver->setTimerCallback(this, Sound::midiDriverCallback);

		for (int i=0; i<8; i++) {
			_midiSlots[i].slot = i;
			_midiSlots[i].track = -1;

			// The main PC versions use XMIDI. ST25 Demo and Macintosh versions use SMF.
			if ((_vm->getGameType() == GType_ST25 && _vm->getFeatures() & GF_DEMO) || _vm->getPlatform() == Common::kPlatformMacintosh)
				_midiSlots[i].midiParser = MidiParser::createParser_SMF();
			else
				_midiSlots[i].midiParser = MidiParser::createParser_XMIDI();

			_midiSlots[i].midiParser->setMidiDriver(_midiDriver);
			_midiSlots[i].midiParser->setTimerRate(_midiDriver->getBaseTempo());
		}
	}

	_soundHandle = new Audio::SoundHandle();
	loadedSoundData = nullptr;

	for (int i=1; i<8; i++) {
		_sfxSlotList.push_back(&_midiSlots[i]);
	}
}

Sound::~Sound() {
	for (int i=0; i<8; i++)
		delete _midiSlots[i].midiParser;
	delete _midiDriver;
	delete _soundHandle;
	delete[] loadedSoundData;
}


void Sound::playMidiTrack(int track) {
	if (!_vm->_midiAudioEnabled)
		return;
	/*
	if (!_vm->_word_467a8)
		return;
	*/

	assert(loadedSoundData != NULL);

	// Check if a midi slot for this track exists already
	for (int i=1; i<8; i++) {
		if (_midiSlots[i].track == track) {
			_midiSlots[i].midiParser->loadMusic(loadedSoundData, sizeof(loadedSoundData));
			_midiSlots[i].midiParser->setTrack(track);

			// Shift this to the back (most recently used)
			_sfxSlotList.remove(&_midiSlots[i]);
			_sfxSlotList.push_back(&_midiSlots[i]);
			return;
		}
	}

	// Take the least recently used slot and use that for the sound effect
	MidiSlot *slot = _sfxSlotList.front();
	_sfxSlotList.pop_front();
	_sfxSlotList.push_back(slot);
	slot->track = track;
	slot->midiParser->loadMusic(loadedSoundData, sizeof(loadedSoundData));
	slot->midiParser->setTrack(track);
}

void Sound::loadMusicFile(const char *baseSoundName) {
	clearAllMidiSlots();
	/*
	if (_vm->getPlatform() == Common::kPlatformAmiga)
		playAmigaSound(baseSoundName);
	else if (_vm->getPlatform() == Common::kPlatformMacintosh)
		playMacSMFSound(baseSoundName);
	else if (_vm->getFeatures() & GF_DEMO)
		playSMFSound(baseSoundName);
	else
	*/
	loadPCMusicFile(baseSoundName);
}

void Sound::playSoundEffect(const char *baseSoundName) {
	/*
	if (_vm->getPlatform() == Common::kPlatformAmiga)
		playAmigaSoundEffect(baseSoundName);
	else if (_vm->getPlatform() == Common::kPlatformMacintosh)
		playMacSoundEffect(baseSoundName);
	else
	*/
	error("PC Sound Effects Not Supported");
}

// PC Functions


// XMIDI or SM sound
void Sound::loadPCMusicFile(const char *baseSoundName) {
	Common::String soundName = baseSoundName;
	
	soundName += '.';
	
	switch (MidiDriver::getMusicType(_midiDevice)) {
		case MT_MT32:
			if (_vm->getFeatures() & GF_DEMO)
				soundName += "ROL";
			else
				soundName += "MT";
			break;
		case MT_PCSPK:
			if (_vm->getFeatures() & GF_DEMO)
				return; // Not supported...
			else
				soundName += "PC";
			break;
		default:
			if (_vm->getFeatures() & GF_DEMO)
				soundName += "ADL";
			else
				soundName += "AD";
			break;
	}
	
	debug(0, "Playing sound \'%s\'\n", soundName.c_str());
	SharedPtr<Common::SeekableReadStream> soundStream = _vm->openFile(soundName.c_str());
	
	if (loadedSoundData != nullptr)
		delete[] loadedSoundData;
	loadedSoundData = new byte[soundStream->size()];
	soundStream->read(loadedSoundData, soundStream->size());
	_midiSlots[0].midiParser->loadMusic(loadedSoundData, soundStream->size());
}

void Sound::clearMidiSlot(int slot) {
	_midiSlots[slot].midiParser->stopPlaying();
	_midiSlots[slot].midiParser->unloadMusic();
	_midiSlots[slot].track = -1;
}

void Sound::clearAllMidiSlots() {
	for (int i=0; i<8; i++) {
		clearMidiSlot(i);
	}
}

void Sound::midiDriverCallback(void *data) {
	Sound *s = (Sound*)data;
	for (int i=0; i<8; i++)
		s->_midiSlots[i].midiParser->onTimer();
}


} // End of namespace StarTrek
