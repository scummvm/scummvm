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

#include "audio/audiostream.h"
#include "audio/decoders/raw.h"
#include "audio/decoders/voc.h"
#include "audio/mods/protracker.h"

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

	if (!SearchMan.hasFile("voc/speech.mrk")) {
		error("Couldn't find 'voc/speech.mrk'. The 'trekcd/voc/' directory must be dumped from the CD");
	}

	_playingSpeech = false;
}

Sound::~Sound() {
	for (int i=0; i<8; i++)
		delete _midiSlots[i].midiParser;
	delete _midiDriver;
	delete _soundHandle;
	delete[] loadedSoundData;
}


void Sound::playMidiTrack(int track) {
	if (!_vm->_musicEnabled)
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
	MidiPlaybackSlot *slot = _sfxSlotList.front();
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
	if (scumm_stricmp(baseSoundName+4, "loop") == 0)
		_loopingAudioName = Common::String(baseSoundName);

	if (!_vm->_sfxEnabled || !_vm->_audioEnabled)
		return;

	/*
	if (word_5113a == 0)
		sub_2aaa3();
	*/

	for (int i=0; i<MAX_SFX_PLAYING; i++) {
		if (_vm->_system->getMixer()->isSoundHandleActive(_sfxHandles[i]))
			continue;

		Common::String soundName = Common::String("voc/sfx/") + baseSoundName + ".voc";
		Common::SeekableReadStream *readStream = SearchMan.createReadStreamForMember(soundName);
		if (readStream == nullptr)
			error("Couldn't open '%s'", soundName.c_str());

		Audio::AudioStream *audioStream = Audio::makeVOCStream(readStream, Audio::FLAG_UNSIGNED, DisposeAfterUse::YES);
		_vm->_system->getMixer()->playStream(Audio::Mixer::kSFXSoundType, &_sfxHandles[i], audioStream);
		return;
	}
}

void Sound::playSpeech(const Common::String &basename) {
	stopPlayingSpeech();

	Audio::QueuingAudioStream *audioQueue = nullptr;
	Common::String name = basename;

	// Play a list of comma-separated audio files in sequence (usually there's only one)
	while (!name.empty()) {
		uint i = 0;
		while (i < name.size() && name[i] != ',') {
			if (name[i] == '\\')
				name.setChar('/', i);
			i++;
		}

		Common::String filename = "voc/" + Common::String(name.c_str(), name.c_str()+i) + ".voc";
		debug("Playing speech '%s'", filename.c_str());
		Common::SeekableReadStream *readStream = SearchMan.createReadStreamForMember(filename);
		if (readStream == nullptr)
			error("Couldn't open '%s'", filename.c_str());

		Audio::AudioStream *audioStream = Audio::makeVOCStream(readStream, Audio::FLAG_UNSIGNED, DisposeAfterUse::YES);
		if (audioQueue == nullptr)
			audioQueue = Audio::makeQueuingAudioStream(audioStream->getRate(), audioStream->isStereo());
		audioQueue->queueAudioStream(audioStream, DisposeAfterUse::YES);

		name.erase(0,i+1);
	}

	if (audioQueue != nullptr) {
		audioQueue->finish();
		_vm->_system->getMixer()->playStream(Audio::Mixer::kSpeechSoundType, &_speechHandle, audioQueue);
	}

	_playingSpeech = true;
}

void Sound::stopPlayingSpeech() {
	if (_playingSpeech) {
		_playingSpeech = false;
		_vm->_system->getMixer()->stopHandle(_speechHandle);
	}
}


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

// Static callback method
void Sound::midiDriverCallback(void *data) {
	Sound *s = (Sound*)data;
	for (int i=0; i<8; i++)
		s->_midiSlots[i].midiParser->onTimer();

	// TODO: put this somewhere other than the midi callback...
	if (s->_playingSpeech && !s->_vm->_system->getMixer()->isSoundHandleActive(s->_speechHandle)) {
		s->stopPlayingSpeech();
		s->_vm->_finishedPlayingSpeech = true;
	}
}


} // End of namespace StarTrek
