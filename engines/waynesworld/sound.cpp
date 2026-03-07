/* ScummVM - Graphic Adventure Engine
 *
 * ScummVM is the legal property of its developers, whose names
 * are too numerous to list here. Please refer to the COPYRIGHT
 * file distributed with this source distribution.
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 *
 */

#include "common/config-manager.h"
#include "common/memstream.h"
#include "audio/mixer.h"
#include "audio/audiostream.h"
#include "audio/mididrv.h"
#include "audio/midiparser.h"
#include "audio/decoders/raw.h"

// Miles Audio
#include "audio/miles.h"
#include "waynesworld/waynesworld.h"
#include "waynesworld/sound.h"

namespace WaynesWorld {

SoundManager::SoundManager(WaynesWorldEngine *vm, Audio::Mixer *mixer) : _vm(vm), _mixer(mixer) {
	_effectsHandle = new Audio::SoundHandle();
	_stream = nullptr;
}

SoundManager::~SoundManager() {
	delete _effectsHandle;
}

void SoundManager::playSound(const char *filename, int flag) {
	while (isSFXPlaying())
		_vm->waitMillis(10);

	_filename = Common::String(filename);
	
	Common::File fd;
	if (!fd.open(Common::Path(filename))) {
		error("playSound : Enable to open %s", filename);
	}

	const int size = fd.size();
	byte *buffer = new byte[size];
	fd.read(buffer, size);

	Common::SeekableReadStream *rawStream = new Common::MemoryReadStream(buffer, size, DisposeAfterUse::YES);
	Audio::RewindableAudioStream *audioStream = Audio::makeRawStream(rawStream, 9000, Audio::FLAG_UNSIGNED);

/*	
	if (loop)
		Audio::LoopingAudioStream *loopingStream = new Audio::LoopingAudioStream(audioStream, 0, DisposeAfterUse::NO));
*/
	if (!_mixer->isSoundHandleActive(*_effectsHandle)) {
		_mixer->playStream(Audio::Mixer::kSFXSoundType, _effectsHandle, audioStream, -1, _mixer->kMaxChannelVolume, 0, DisposeAfterUse::NO);
	}

	if (flag) {
		while (isSFXPlaying())
			_vm->waitMillis(10);
	}
}

bool SoundManager::isSFXPlaying() {
	return _mixer->isSoundHandleActive(*_effectsHandle);
}

void SoundManager::syncVolume() {
	const int sfxVol = CLIP(ConfMan.getInt("sfx_volume"), 0, 255);
	_mixer->setVolumeForSoundType(Audio::Mixer::kSFXSoundType, sfxVol);
}


void SoundManager::stopSound() {
	_mixer->stopHandle(*_effectsHandle);
}

/******************************************************************************************/

MusicManager::MusicManager(WaynesWorldEngine *vm) : _vm(vm) {
	_music = nullptr;
	_isLooping = false;
	_driver = nullptr;

	const MidiDriver::DeviceHandle dev = MidiDriver::detectDevice(MDT_MIDI | MDT_ADLIB | MDT_PREFER_MT32);
	MusicType musicType = MidiDriver::getMusicType(dev);

	switch (musicType) {
	case MT_ADLIB: {
		MidiPlayer::createDriver();
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

	if (_driver) {
		const int retValue = _driver->open();
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
	delete[] _music;
}

void MusicManager::send(uint32 b) {
	// Pass data directly to driver
	_driver->send(b);
}

bool MusicManager::checkMidiDone() {
	return (!_isPlaying);
}

void MusicManager::midiRepeat() {
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
	if (!_driver)
		return;

	stop();
}

void MusicManager::playMusic(const char *filename, bool loop) {
	if (!_driver)
		return;

	Common::File fd;
	if (!fd.open(Common::Path(filename)))
		error("playMusic : Unable to open %s", filename);

	const int size = fd.size();
	if (size < 4)
		error("playMusic() wrong music resource size");

	delete[] _music;
	_music = new byte[size];
	fd.read(_music, size);

	stop();

	const uint32 magic = READ_BE_UINT32(_music);
	if (magic == MKTAG('F', 'O', 'R', 'M')) {
		_parser = MidiParser::createParser_XMIDI();

		if (!_parser->loadMusic(_music, size))
			error("midiPlay() wrong music resource");

		_parser->setTrack(0);
		_parser->setMidiDriver(this);
		_parser->setTimerRate(_driver->getBaseTempo());
		_parser->property(MidiParser::mpCenterPitchWheelOnUnload, 1);
		_parser->property(MidiParser::mpSendSustainOffOnNotesOff, 1);

		// Handle music looping
		_parser->property(MidiParser::mpAutoLoop, loop);
		syncVolume();
		_isPlaying = true;
	} else {
		warning("playMusic() Unexpected signature 0x%08x, expected 'FORM'", magic);
		_isPlaying = false;
	}
}

void MusicManager::setLoop(bool loop) {
	_isLooping = loop;
	if (_parser)
		_parser->property(MidiParser::mpAutoLoop, _isLooping);
}
} // End of namespace WaynesWorld
