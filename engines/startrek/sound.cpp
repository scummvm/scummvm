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
 */

#include "startrek/resource.h"
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
	_midiDevice = MT_AUTO;
	_midiDriver = nullptr;
	_loopingMidiTrack = false;

	if (_vm->getPlatform() == Common::kPlatformDOS || _vm->getPlatform() == Common::kPlatformMacintosh) {
		_midiDevice = MidiDriver::detectDevice(MDT_PCSPK | MDT_ADLIB | MDT_MIDI | MDT_PREFER_MT32);
		_midiDriver = MidiDriver::createMidi(_midiDevice);
		_midiDriver->open();

		for (int i = 0; i < NUM_MIDI_SLOTS; i++) {
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

		_midiDriver->setTimerCallback(this, Sound::midiDriverCallback);
	}

	_soundHandle = new Audio::SoundHandle();
	loadedSoundData = nullptr;

	for (int i = 1; i < NUM_MIDI_SLOTS; i++) {
		_midiSlotList.push_back(&_midiSlots[i]);
	}

	if (!(_vm->getFeatures() & GF_CDROM))
		_vm->_sfxWorking = false;
	else if (!SearchMan.hasFile("voc/speech.mrk")) {
		warning("Couldn't find 'voc/speech.mrk'. The 'trekcd/voc/' directory should be dumped from the CD. Continuing without CD audio");
		_vm->_sfxWorking = false;
	}

	_playingSpeech = false;
}

Sound::~Sound() {
	for (int i = 0; i < NUM_MIDI_SLOTS; i++)
		delete _midiSlots[i].midiParser;
	delete _midiDriver;
	delete _soundHandle;
	delete[] loadedSoundData;
}


void Sound::clearAllMidiSlots() {
	for (int i = 0; i < NUM_MIDI_SLOTS; i++) {
		clearMidiSlot(i);
	}
}

void Sound::playMidiTrack(int track) {
	if (!_vm->_musicEnabled || !_vm->_musicWorking)
		return;

	// TODO: Demo music
	if (_vm->getFeatures() & GF_DEMO)
		return;

	assert(loadedSoundData != nullptr);

	// Check if a midi slot for this track exists already
	for (int i = 1; i < NUM_MIDI_SLOTS; i++) {
		if (_midiSlots[i].track == track) {
			debugC(6, kDebugSound, "Playing MIDI track %d (slot %d)", track, i);
			_midiSlots[i].midiParser->loadMusic(loadedSoundData, sizeof(loadedSoundData));
			_midiSlots[i].midiParser->setTrack(track);

			// Shift this to the back (most recently used)
			_midiSlotList.remove(&_midiSlots[i]);
			_midiSlotList.push_back(&_midiSlots[i]);
			return;
		}
	}

	// Take the least recently used slot and use that for the sound effect
	MidiPlaybackSlot *slot = _midiSlotList.front();
	_midiSlotList.pop_front();
	_midiSlotList.push_back(slot);
	playMidiTrackInSlot(slot->slot, track);
}

void Sound::playMidiTrackInSlot(int slot, int track) {
	assert(loadedSoundData != nullptr);
	debugC(6, kDebugSound, "Playing MIDI track %d (slot %d)", track, slot);

	clearMidiSlot(slot);

	if (track != -1) {
		_midiSlots[slot].track = track;
		_midiSlots[slot].midiParser->loadMusic(loadedSoundData, sizeof(loadedSoundData));
		_midiSlots[slot].midiParser->setTrack(track);
	}
}

bool Sound::isMidiPlaying() {
	if (!_vm->_musicWorking)
		return false;

	for (int i = 0; i < NUM_MIDI_SLOTS; i++) {
		if (_midiSlots[i].midiParser->isPlaying())
			return true;
	}

	return false;
}

void Sound::loadMusicFile(const Common::String &baseSoundName) {
	bool isDemo = _vm->getFeatures() & GF_DEMO;

	clearAllMidiSlots();

	if (baseSoundName == _loadedMidiFilename)
		return;

	_loadedMidiFilename = baseSoundName;

	if (_vm->getPlatform() == Common::kPlatformDOS && !isDemo) {
		loadPCMusicFile(baseSoundName);
	} else if (_vm->getPlatform() == Common::kPlatformDOS && isDemo) {
		//playSMFSound(baseSoundName);
	} else if (_vm->getPlatform() == Common::kPlatformAmiga) {
		//playAmigaSound(baseSoundName);
	} else if (_vm->getPlatform() == Common::kPlatformMacintosh) {
		//playMacSMFSound(baseSoundName);
	}
}

void Sound::playMidiMusicTracks(int startTrack, int loopTrack) {
	if (!_vm->_musicWorking || !_vm->_musicEnabled)
		return;

	if (loopTrack == -3)
		_loopingMidiTrack = startTrack;
	else if (loopTrack != -2)
		_loopingMidiTrack = loopTrack;

	if (startTrack != -2 && _vm->_musicEnabled)
		playMidiTrackInSlot(0, startTrack);
}

/**
 * TODO: original game had some caching of loaded voc files.
 */
void Sound::playVoc(const Common::String &baseSoundName) {
	/*
	if (_vm->getPlatform() == Common::kPlatformAmiga)
		playAmigaSoundEffect(baseSoundName);
	else if (_vm->getPlatform() == Common::kPlatformMacintosh)
		playMacSoundEffect(baseSoundName);
	else
	*/
	bool loop = false;
	if (baseSoundName.size() == 8 && baseSoundName.hasSuffixIgnoreCase("loop")) {
		_loopingAudioName = baseSoundName;
		loop = true;
	}

	if (!_vm->_sfxEnabled || !_vm->_sfxWorking)
		return;

	/*
	// This is probably just driver initialization stuff...
	if (word_5113a == 0)
		sub_2aaa3();
	*/

	for (int i = 0; i < MAX_SFX_PLAYING; i++) {
		if (_vm->_system->getMixer()->isSoundHandleActive(_sfxHandles[i]))
			continue;

		Common::String soundName = Common::String("voc/sfx/") + baseSoundName + ".voc";
		Common::SeekableReadStream *readStream = SearchMan.createReadStreamForMember(soundName);
		if (readStream == nullptr)
			error("Couldn't open '%s'", soundName.c_str());

		debugC(5, kDebugSound, "Playing sound effect '%s'", soundName.c_str());

		Audio::RewindableAudioStream *srcStream = Audio::makeVOCStream(readStream, Audio::FLAG_UNSIGNED, DisposeAfterUse::YES);
		Audio::AudioStream *audioStream;
		if (loop)
			audioStream = new Audio::LoopingAudioStream(srcStream, 0, DisposeAfterUse::YES);
		else
			audioStream = srcStream;
		_vm->_system->getMixer()->playStream(Audio::Mixer::kSFXSoundType, &_sfxHandles[i], audioStream);
		return;
	}

	debugC(3, kDebugSound, "No sound slot to play '%s'", baseSoundName.c_str());
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

		Common::String filename = "voc/" + Common::String(name.c_str(), name.c_str() + i) + ".voc";
		debugC(5, kDebugSound, "Playing speech '%s'", filename.c_str());
		Common::SeekableReadStream *readStream = SearchMan.createReadStreamForMember(filename);
		if (readStream == nullptr)
			error("Couldn't open '%s'", filename.c_str());

		Audio::AudioStream *audioStream = Audio::makeVOCStream(readStream, Audio::FLAG_UNSIGNED, DisposeAfterUse::YES);
		if (audioStream != nullptr) {
			if (audioQueue == nullptr)
				audioQueue = Audio::makeQueuingAudioStream(audioStream->getRate(), audioStream->isStereo());
			audioQueue->queueAudioStream(audioStream, DisposeAfterUse::YES);
		}

		name.erase(0, i + 1);
	}

	if (audioQueue != nullptr) {
		audioQueue->finish();
		_vm->_system->getMixer()->playStream(Audio::Mixer::kSpeechSoundType, &_speechHandle, audioQueue);
		_playingSpeech = true;
	}
}

void Sound::stopAllVocSounds() {
	stopPlayingSpeech();

	for (int i = 0; i < MAX_SFX_PLAYING; i++) {
		_vm->_system->getMixer()->stopHandle(_sfxHandles[i]);
	}
}

void Sound::stopPlayingSpeech() {
	if (_playingSpeech) {
		debugC(5, kDebugSound, "Canceled speech playback");
		_playingSpeech = false;
		_vm->_system->getMixer()->stopHandle(_speechHandle);
	}
}

void Sound::playSoundEffectIndex(int index) {
	if (!(_vm->getFeatures() & GF_CDROM))
		playMidiTrack(index);
	else {
		switch (index) {
		case 0x04:
			playVoc("tricorde");
			break;
		case 0x05:
			playVoc("STDOOR1");
			break;
		case 0x06:
			playVoc("PHASSHOT");
			break;
		case 0x07:
			playMidiTrack(index);
			break;
		case 0x08:
			playVoc("TRANSDEM");
			break;
		case 0x09: // Beaming in?
			playVoc("TRANSMAT");
			break;
		case 0x0a: // Beaming out?
			playVoc("TRANSENE");
			break;
		case 0x10: // Menu selection sound
			playMidiTrack(index);
			break;
		case 0x22:
			playVoc("HAILING");
			break;
		case 0x24:
			playVoc("PHASSHOT");
			break;
		case 0x25:
			playVoc("PHOTSHOT");
			break;
		case 0x26:
			playVoc("HITSHIEL");
			break;
		case 0x27:
			playMidiTrack(index);
			break;
		case 0x28:
			playVoc("REDALERT");
			break;
		case 0x29:
			playVoc("WARP");
			break;
		default:
			debugC(kDebugSound, 6, "Unmapped sound 0x%x", index);
			break;
		}
	}
}

void Sound::toggleMusic() {
	setMusicEnabled(!_vm->_musicEnabled);
}

void Sound::setMusicEnabled(bool enable) {
	if (!_vm->_musicWorking || _vm->_musicEnabled == enable)
		return;

	_vm->_musicEnabled = enable;

	if (enable)
		playMidiMusicTracks(_loopingMidiTrack, _loopingMidiTrack);
	else
		clearMidiSlot(0);
}

void Sound::toggleSfx() {
	setSfxEnabled(!_vm->_sfxEnabled);
}

void Sound::setSfxEnabled(bool enable) {
	if (!_vm->_sfxWorking || _vm->_sfxEnabled == enable)
		return;

	_vm->_sfxEnabled = enable;

	if (!enable) {
		for (int i = 1; i < NUM_MIDI_SLOTS; i++)
			clearMidiSlot(i);
	}

	if (!enable) {
		stopAllVocSounds();
	} else if (!_loopingAudioName.empty()) {
		playVoc(_loopingAudioName);
	}
}

void Sound::checkLoopMusic() {
	// TODO
	// It might be better to get rid of this altogether and deal with it in callbacks...
}


// XMIDI or SM sound
void Sound::loadPCMusicFile(const Common::String &baseSoundName) {
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

	debugC(5, kDebugSound, "Loading midi \'%s\'\n", soundName.c_str());
	Common::MemoryReadStreamEndian *soundStream = _vm->_resource->loadFile(soundName.c_str());

	if (loadedSoundData != nullptr)
		delete[] loadedSoundData;
	loadedSoundData = new byte[soundStream->size()];
	soundStream->read(loadedSoundData, soundStream->size());

	// FIXME: should music start playing when this is called?
	//_midiSlots[0].midiParser->loadMusic(loadedSoundData, soundStream->size());

	delete soundStream;
}

void Sound::clearMidiSlot(int slot) {
	_midiSlots[slot].midiParser->stopPlaying();
	_midiSlots[slot].midiParser->unloadMusic();
	_midiSlots[slot].track = -1;
}

// Static callback method
void Sound::midiDriverCallback(void *data) {
	Sound *s = (Sound *)data;
	for (int i = 0; i < NUM_MIDI_SLOTS; i++)
		s->_midiSlots[i].midiParser->onTimer();

	// TODO: put this somewhere other than the midi callback...
	if (s->_playingSpeech && !s->_vm->_system->getMixer()->isSoundHandleActive(s->_speechHandle)) {
		s->stopPlayingSpeech();
		s->_vm->_finishedPlayingSpeech = true;
	}
}

} // End of namespace StarTrek
