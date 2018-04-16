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

#include "common/config-manager.h"
#include "audio/mixer.h"
#include "audio/audiostream.h"
#include "audio/mididrv.h"
#include "audio/midiparser.h"
#include "audio/decoders/raw.h"
#include "audio/decoders/wave.h"
// Miles Audio
#include "audio/miles.h"
#include "access/access.h"
#include "access/sound.h"

namespace Access {

SoundManager::SoundManager(AccessEngine *vm, Audio::Mixer *mixer) : _vm(vm), _mixer(mixer) {
	_effectsHandle = new Audio::SoundHandle();
}

SoundManager::~SoundManager() {
	clearSounds();
	delete _effectsHandle;
}

void SoundManager::clearSounds() {
	debugC(1, kDebugSound, "clearSounds()");

	for (uint i = 0; i < _soundTable.size(); ++i)
		delete _soundTable[i]._res;

	_soundTable.clear();

	if (_mixer->isSoundHandleActive(*_effectsHandle))
		_mixer->stopHandle(*_effectsHandle);

	while (_queue.size()) {
		delete _queue[0]._stream;
		_queue.remove_at(0);
	}
}

bool SoundManager::isSoundQueued(int soundId) const {
	for (uint idx = 0; idx < _queue.size(); ++idx) {
		if (_queue[idx]._soundId == soundId)
			return true;
	}

	return false;
}

void SoundManager::loadSoundTable(int idx, int fileNum, int subfile, int priority) {
	debugC(1, kDebugSound, "loadSoundTable(%d, %d, %d)", idx, fileNum, subfile);

	Resource *soundResource;

	if (idx >= (int)_soundTable.size())
		_soundTable.resize(idx + 1);

	delete _soundTable[idx]._res;
	soundResource = _vm->_files->loadFile(fileNum, subfile);
	_soundTable[idx]._res = soundResource;
	_soundTable[idx]._priority = priority;
}

Resource *SoundManager::loadSound(int fileNum, int subfile) {
	debugC(1, kDebugSound, "loadSound(%d, %d)", fileNum, subfile);
	return _vm->_files->loadFile(fileNum, subfile);
}

void SoundManager::playSound(int soundIndex, bool loop) {
	debugC(1, kDebugSound, "playSound(%d, %d)", soundIndex, loop);
	if (isSoundQueued(soundIndex))
		// Prevent duplicate copies of a sound from being queued
		return;

	int priority = _soundTable[soundIndex]._priority;
	playSound(_soundTable[soundIndex]._res, priority, loop, soundIndex);
}

void SoundManager::playSound(Resource *res, int priority, bool loop, int soundIndex) {
	debugC(1, kDebugSound, "playSound");

	byte *resourceData = res->data();

	assert(res->_size >= 32);

	Audio::RewindableAudioStream *audioStream;

	if (READ_BE_UINT32(resourceData) == MKTAG('R','I','F','F')) {
		// CD version uses WAVE-files
		Common::SeekableReadStream *waveStream = new Common::MemoryReadStream(resourceData, res->_size, DisposeAfterUse::NO);
		audioStream = Audio::makeWAVStream(waveStream, DisposeAfterUse::YES);
	} else if (READ_BE_UINT32(resourceData) == MKTAG('S', 'T', 'E', 'V')) {
		// sound files have a fixed header of 32 bytes in total
		//  header content:
		//   "STEVE" - fixed header
		//   byte    - sample rate
		//              01h mapped internally to 3Ch
		//              02h mapped internally to 78h
		//              03h mapped internally to B5h
		//              04h mapped internally to F1h
		//   byte    - unknown
		//   word    - actual sample size (should be resource-size - 32)
		byte internalSampleRate = resourceData[5];
		int sampleSize = READ_LE_UINT16(resourceData + 7);

		assert( (sampleSize + 32) <= res->_size);

		int sampleRate = 0;
		switch (internalSampleRate) {
		case 1: // NEG(3Ch) -> C4h time constant
			sampleRate = 16666;
			break;

		case 2: // NEG(78h) -> 88h time constant
			sampleRate = 8334;
			break;

		case 3: // NEG(B5h) -> 4Bh time constant
			sampleRate = 5525;
			break;

		case 4: // NEG(F1h) -> 0Fh time constant
			sampleRate = 4150;
			break;

		default:
			error("Unexpected internal Sample Rate %d", internalSampleRate);
			return;
		}

		audioStream = Audio::makeRawStream(resourceData + 32, sampleSize, sampleRate, 0, DisposeAfterUse::NO);
	} else
		error("Unknown format");

	if (loop) {
		_queue.push_back(QueuedSound(new Audio::LoopingAudioStream(audioStream, 0,
			DisposeAfterUse::NO), soundIndex));
	} else {
		_queue.push_back(QueuedSound(audioStream, soundIndex));
	}

	if (!_mixer->isSoundHandleActive(*_effectsHandle))
		_mixer->playStream(Audio::Mixer::kSFXSoundType, _effectsHandle,
						_queue[0]._stream, -1, _mixer->kMaxChannelVolume, 0,
						DisposeAfterUse::NO);
}

void SoundManager::checkSoundQueue() {
	debugC(5, kDebugSound, "checkSoundQueue");

	if (_queue.empty() || _mixer->isSoundHandleActive(*_effectsHandle))
		return;

	delete _queue[0]._stream;
	_queue.remove_at(0);

	if (_queue.size() && _queue[0]._stream)
		_mixer->playStream(Audio::Mixer::kSFXSoundType, _effectsHandle,
		   _queue[0]._stream, -1, _mixer->kMaxChannelVolume, 0,
		   DisposeAfterUse::NO);
}

bool SoundManager::isSFXPlaying() {
	return _mixer->isSoundHandleActive(*_effectsHandle);
}

void SoundManager::loadSounds(Common::Array<RoomInfo::SoundIdent> &sounds) {
	debugC(1, kDebugSound, "loadSounds");

	clearSounds();

	for (uint i = 0; i < sounds.size(); ++i) {
		Resource *sound = loadSound(sounds[i]._fileNum, sounds[i]._subfile);
		_soundTable.push_back(SoundEntry(sound, sounds[i]._priority));
	}
}

void SoundManager::stopSound() {
	debugC(3, kDebugSound, "stopSound");

	_mixer->stopHandle(*_effectsHandle);
}

void SoundManager::freeSounds() {
	debugC(3, kDebugSound, "freeSounds");

	stopSound();
	clearSounds();
}

/******************************************************************************************/

MusicManager::MusicManager(AccessEngine *vm) : _vm(vm) {
	_music = nullptr;
	_tempMusic = nullptr;
	_isLooping = false;
	_driver = nullptr;
	_byte1F781 = false;

	MidiDriver::DeviceHandle dev = MidiDriver::detectDevice(MDT_MIDI | MDT_ADLIB | MDT_PREFER_MT32);
	MusicType musicType = MidiDriver::getMusicType(dev);

	// Amazon Guardians of Eden uses MIDPAK inside MIDIDRV.AP
	// AdLib patches are inside MIDIDRV.AP too, 2nd resource file
	//
	// Amazon Guardians of Eden (demo) seems to use another type of driver, possibly written by Access themselves
	// Martian Memorandum uses this other type of driver as well, which means it makes sense to reverse engineer it.
	//
	switch (musicType) {
	case MT_ADLIB: {
		if (_vm->getGameID() == GType_Amazon && !_vm->isDemo()) {
			Resource   *midiDrvResource = _vm->_files->loadFile(92, 1);
			Common::MemoryReadStream *adLibInstrumentStream = new Common::MemoryReadStream(midiDrvResource->data(), midiDrvResource->_size);

			_driver = Audio::MidiDriver_Miles_AdLib_create("", "", adLibInstrumentStream);

			delete midiDrvResource;
			delete adLibInstrumentStream;
		} else {
			MidiPlayer::createDriver();
		}
		break;
	}
	case MT_MT32:
		_driver = Audio::MidiDriver_Miles_MT32_create("");
		_nativeMT32 = true;
		break;
	case MT_GM:
		if (ConfMan.getBool("native_mt32")) {
			_driver = Audio::MidiDriver_Miles_MT32_create("");
			_nativeMT32 = true;
		}
		break;

	default:
		break;
	}

#if 0
	MidiPlayer::createDriver();
	MidiDriver::detectDevice(MDT_MIDI | MDT_ADLIB | MDT_PREFER_GM);
#endif

	if (_driver) {
		int retValue = _driver->open();
		if (retValue == 0) {
			if (_nativeMT32)
				_driver->sendMT32Reset();
			else
				_driver->sendGMReset();

			_driver->setTimerCallback(this, &timerCallback);
		}
	}
}

MusicManager::~MusicManager() {
	delete _music;
	delete _tempMusic;
}

void MusicManager::send(uint32 b) {
	// Pass data directly to driver
	_driver->send(b);
#if 0
	if ((b & 0xF0) == 0xC0 && !_nativeMT32) {
		b = (b & 0xFFFF00FF) | MidiDriver::_mt32ToGm[(b >> 8) & 0xFF] << 8;
	}

	Audio::MidiPlayer::send(b);
#endif
}

void MusicManager::midiPlay() {
	debugC(1, kDebugSound, "midiPlay");

	if (!_driver)
		return;

	if (_music->_size < 4) {
		error("midiPlay() wrong music resource size");
	}

	stop();

	if (READ_BE_UINT32(_music->data()) != MKTAG('F', 'O', 'R', 'M')) {
		warning("midiPlay() Unexpected signature");
		_isPlaying = false;
	} else {
		_parser = MidiParser::createParser_XMIDI();

		if (!_parser->loadMusic(_music->data(), _music->_size))
			error("midiPlay() wrong music resource");

		_parser->setTrack(0);
		_parser->setMidiDriver(this);
		_parser->setTimerRate(_driver->getBaseTempo());
		_parser->property(MidiParser::mpCenterPitchWheelOnUnload, 1);
		_parser->property(MidiParser::mpSendSustainOffOnNotesOff, 1);

		// Handle music looping
		_parser->property(MidiParser::mpAutoLoop, _isLooping);

		setVolume(127);
		_isPlaying = true;
	}
}

bool MusicManager::checkMidiDone() {
	debugC(1, kDebugSound, "checkMidiDone");
	return (!_isPlaying);
}

void MusicManager::midiRepeat() {
	debugC(1, kDebugSound, "midiRepeat");

	if (!_driver)
		return;
	if (!_parser)
		return;

	_isLooping = true;
	_parser->property(MidiParser::mpAutoLoop, _isLooping);
	if (!_isPlaying)
		_parser->setTrack(0);
}

void MusicManager::stopSong() {
	debugC(1, kDebugSound, "stopSong");

	if (!_driver)
		return;

	stop();
}

void MusicManager::loadMusic(int fileNum, int subfile) {
	debugC(1, kDebugSound, "loadMusic(%d, %d)", fileNum, subfile);

	_music = _vm->_files->loadFile(fileNum, subfile);
}

void MusicManager::loadMusic(FileIdent file) {
	debugC(1, kDebugSound, "loadMusic(%d, %d)", file._fileNum, file._subfile);

	_music = _vm->_files->loadFile(file);
}

void MusicManager::newMusic(int musicId, int mode) {
	debugC(1, kDebugSound, "newMusic(%d, %d)", musicId, mode);

	if (!_driver)
		return;

	if (mode == 1) {
		stopSong();
		freeMusic();
		_music = _tempMusic;
		_tempMusic = nullptr;
		_isLooping = true;
	} else {
		_isLooping = (mode == 2);
		_tempMusic = _music;
		stopSong();
		loadMusic(97, musicId);
	}

	if (_music)
		midiPlay();
}

void MusicManager::freeMusic() {
	debugC(3, kDebugSound, "freeMusic");

	delete _music;
	_music = nullptr;
}

void MusicManager::setLoop(bool loop) {
	debugC(3, kDebugSound, "setLoop");

	_isLooping = loop;
	if (_parser)
		_parser->property(MidiParser::mpAutoLoop, _isLooping);
}
} // End of namespace Access
