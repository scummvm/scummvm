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
 * $URL: https://scummvm-startrek.googlecode.com/svn/trunk/sound.cpp $
 * $Id: sound.cpp 15 2010-06-27 06:13:42Z clone2727 $
 *
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
		// The main PC versions use XMIDI. ST25 Demo and Macintosh versions use SMF.
		if ((_vm->getGameType() == GType_ST25 && _vm->getFeatures() & GF_DEMO) || _vm->getPlatform() == Common::kPlatformMacintosh)
			_midiParser = MidiParser::createParser_SMF();
		else
			_midiParser = MidiParser::createParser_XMIDI();
			
		_midiDevice = MidiDriver::detectDevice(MDT_PCSPK|MDT_ADLIB|MDT_MIDI);
		_midiDriver = MidiDriver::createMidi(_midiDevice);
		_midiDriver->open();
		_midiParser->setMidiDriver(_midiDriver);
		_midiParser->setTimerRate(_midiDriver->getBaseTempo());
	}

	if (_vm->getPlatform() == Common::kPlatformMacintosh) {
		_macAudioResFork = new Common::MacResManager();
		if (!_macAudioResFork->open("Star Trek Audio"))
			error("Could not open 'Star Trek Audio'");
		assert(_macAudioResFork->hasResFork());
	} else
		_macAudioResFork = 0;

	_soundHandle = new Audio::SoundHandle();
}

Sound::~Sound() {
	delete _midiParser;
	delete _midiDriver;
	delete _soundHandle;
	delete _macAudioResFork;
}

void Sound::playSound(const char *baseSoundName) {
	if (_vm->getPlatform() == Common::kPlatformAmiga)
		playAmigaSound(baseSoundName);
	else if (_vm->getPlatform() == Common::kPlatformMacintosh)
		playMacSMFSound(baseSoundName);
	else if (_vm->getFeatures() & GF_DEMO)
		playSMFSound(baseSoundName);
	else
		playXMIDISound(baseSoundName);
}

void Sound::playSoundEffect(const char *baseSoundName) {
	if (_vm->getPlatform() == Common::kPlatformAmiga)
		playAmigaSoundEffect(baseSoundName);
	else if (_vm->getPlatform() == Common::kPlatformMacintosh)
		playMacSoundEffect(baseSoundName);
	else
		error("PC Sound Effects Not Supported");
}

// PC Functions

void Sound::playSMFSound(const char *baseSoundName) {
	Common::String soundName = baseSoundName;
	
	soundName += '.';
	
	switch (MidiDriver::getMusicType(_midiDevice)) {
		case MT_MT32:
			soundName += "ROL";
			break;
		case MT_PCSPK:
			return; // Not supported...
		default:
			soundName += "ADL";
			break;
	}
	
	debug(0, "Playing sound \'%s\'\n", soundName.c_str());
	SharedPtr<Common::SeekableReadStream> soundStream = _vm->openFile(soundName.c_str());
	
	byte *soundData = (byte *)malloc(soundStream->size());
	soundStream->read(soundData, soundStream->size());
	_midiParser->loadMusic(soundData, soundStream->size());
	
	_midiDriver->setTimerCallback(_midiParser, MidiParser::timerCallback);
}

void Sound::playXMIDISound(const char *baseSoundName) {
	Common::String soundName = baseSoundName;
	
	soundName += '.';
	
	switch (MidiDriver::getMusicType(_midiDevice)) {
		case MT_MT32:
			soundName += "MT";
			break;
		case MT_PCSPK:
			soundName += "PC";
			break;
		default:
			soundName += "AD";
			break;
	}
	
	debug(0, "Playing sound \'%s\'\n", soundName.c_str());
	SharedPtr<Common::SeekableReadStream> soundStream = _vm->openFile(soundName.c_str());
	
	byte *soundData = (byte *)malloc(soundStream->size());
	soundStream->read(soundData, soundStream->size());
	_midiParser->loadMusic(soundData, soundStream->size());
	
	_midiDriver->setTimerCallback(_midiParser, MidiParser::timerCallback);
}

// Amiga Functions

void Sound::playAmigaSound(const char *baseSoundName) {
	// Nope, this is wrong... see http://protracker.de/files/amiga/formats/theplayer_41_format.txt
#if 0
	Common::String soundName = baseSoundName;
	soundName += ".SNG";
	if (_vm->_mixer->isSoundHandleActive(*_soundHandle))
		_vm->_mixer->stopHandle(*_soundHandle);
	_vm->_mixer->playInputStream(Audio::Mixer::kMusicSoundType, _soundHandle, Audio::makeProtrackerStream(_vm->openFile(soundName.c_str())));
#endif
}

void Sound::playAmigaSoundEffect(const char *baseSoundName) {
	Common::String soundName = baseSoundName;
	soundName += ".SFX";

	if (_vm->_mixer->isSoundHandleActive(*_soundHandle))
		_vm->_mixer->stopHandle(*_soundHandle);

	Audio::AudioStream *audStream = (Audio::AudioStream *)Audio::makeRawStream(_vm->openFile(soundName.c_str()).get(), 11025, 0);
	_vm->_mixer->playStream(Audio::Mixer::kSFXSoundType, _soundHandle, audStream);
}

// Macintosh Functions

void Sound::playMacSMFSound(const char *baseSoundName) {
	Common::SeekableReadStream *soundStream = _macAudioResFork->getResource(baseSoundName);
	byte *soundData = (byte *)malloc(soundStream->size());
	soundStream->read(soundData, soundStream->size());
	_midiParser->loadMusic(soundData, soundStream->size());
	delete soundStream;
	
	_midiDriver->setTimerCallback(_midiParser, MidiParser::timerCallback);
}

void Sound::playMacSoundEffect(const char *baseSoundName) {
	if (_vm->_mixer->isSoundHandleActive(*_soundHandle))
		_vm->_mixer->stopHandle(*_soundHandle);

	Audio::AudioStream *audStream = (Audio::AudioStream *)Audio::makeRawStream(_macAudioResFork->getResource(baseSoundName), 11025, 0);
	_vm->_mixer->playStream(Audio::Mixer::kSFXSoundType, _soundHandle, audStream);
}

} // End of namespace StarTrek
