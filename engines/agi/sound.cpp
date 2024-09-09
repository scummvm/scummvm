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

#include "agi/agi.h"

#include "agi/sound_2gs.h"
#include "agi/sound_a2.h"
#include "agi/sound_coco3.h"
#include "agi/sound_midi.h"
#include "agi/sound_sarien.h"
#include "agi/sound_pcjr.h"

#include "common/textconsole.h"
#include "audio/mixer.h"

namespace Agi {

SoundGen::SoundGen(AgiBase *vm, Audio::Mixer *pMixer) : _vm(vm), _mixer(pMixer) {
	_sampleRate = pMixer->getOutputRate();
	_soundHandle = new Audio::SoundHandle();
}

SoundGen::~SoundGen() {
	delete _soundHandle;
}

//
// TODO: add support for variable sampling rate in the output device
//

AgiSound *AgiSound::createFromRawResource(uint8 *data, uint32 len, int resnum, int soundemu) {
	if (data == nullptr || len < 2) // Check for too small resource or no resource at all
		return nullptr;

	// Handle platform-specific formats that can't be detected by contents.
	// These formats have no headers or predictable first bytes.
	if (soundemu == SOUND_EMU_APPLE2) {
		return new AgiSound(resnum, data, len, AGI_SOUND_APPLE2);
	} else if (soundemu == SOUND_EMU_COCO3) {
		return new AgiSound(resnum, data, len, AGI_SOUND_COCO3);
	}

	uint16 type = READ_LE_UINT16(data);

	// For V1 sound resources
	if (type != AGI_SOUND_SAMPLE && (type & 0xFF) == AGI_SOUND_4CHN)
		return new PCjrSound(resnum, data, len, AGI_SOUND_4CHN);

	switch (type) { // Create a sound object based on the type
	case AGI_SOUND_SAMPLE:
		return new IIgsSample(resnum, data, len, type);
	case AGI_SOUND_MIDI:
		return new IIgsMidi(resnum, data, len, type);
	case AGI_SOUND_4CHN:
		if (soundemu == SOUND_EMU_MIDI) {
			return new AgiSound(resnum, data, len, type);
		} else {
			return new PCjrSound(resnum, data, len, type);
		}
	default:
		break;
	}

	warning("Sound resource (%d) has unknown type (0x%04x). Not using the sound", resnum, type);
	return nullptr;
}

PCjrSound::PCjrSound(byte resourceNr, byte *data, uint32 length, uint16 type) :
	AgiSound(resourceNr, data, length, type) {

	bool isValid = (_type == AGI_SOUND_4CHN) && (_data != nullptr) && (_length >= 2);
	if (!isValid) // Check for errors
		warning("Error creating PCjr 4-channel sound from resource %d (Type %d, length %d)", _resourceNr, _type, _length);
}

const uint8 *PCjrSound::getVoicePointer(uint voiceNum) {
	assert(voiceNum < 4);
	uint16 voiceStartOffset = READ_LE_UINT16(_data + voiceNum * 2);

	return _data + voiceStartOffset;
}

void SoundMgr::unloadSound(int resnum) {
	if (_vm->_game.dirSound[resnum].flags & RES_LOADED) {
		if (_vm->_game.sounds[resnum]->isPlaying()) {
			_vm->_game.sounds[resnum]->stop();
		}

		// Release the sound resource's data
		delete _vm->_game.sounds[resnum];
		_vm->_game.sounds[resnum] = nullptr;
		_vm->_game.dirSound[resnum].flags &= ~RES_LOADED;
	}
}

/**
 * Start playing a sound resource. The logic here is that when the sound is
 * finished we set the given flag to be true. This way the condition can be
 * detected by the game. On the other hand, if the game wishes to start
 * playing a new sound before the current one is finished, we also let it
 * do that.
 * @param resnum  the sound resource number
 * @param flag    the flag that is wished to be set true when finished
 */
void SoundMgr::startSound(int resnum, int flag) {
	AgiSound *sound = _vm->_game.sounds[resnum];
	debugC(3, kDebugLevelSound, "startSound(resnum = %d, flag = %d, type = %d)", resnum, flag, sound ? sound->type() : 0);
	if (sound == nullptr) {
		warning("startSound: sound %d does not exist", resnum);
		return;
	}

	stopSound();

	// This check handles an Apple IIgs sample with an invalid header
	if (!sound->isValid()) {
		warning("startSound: sound %d is invalid", resnum);
		return;
	}

	sound->play();
	_playingSound = resnum;
	_soundGen->play(resnum);

	// Reset the flag
	_endflag = flag;

	_vm->setFlagOrVar(_endflag, false);
}

void SoundMgr::stopSound() {
	debugC(3, kDebugLevelSound, "stopSound() --> %d", _playingSound);

	if (_playingSound != -1) {
		if (_vm->_game.sounds[_playingSound]) // sanity checking
			_vm->_game.sounds[_playingSound]->stop();
		_soundGen->stop();
		_playingSound = -1;
	}

	// This is needed all the time, some games wait until music got played and when a sound/music got stopped early
	// it would otherwise block the game (for example Death Angel jingle in back door poker room in Police Quest 1, room 71)
	if (_endflag != -1) {
		_vm->setFlagOrVar(_endflag, true);
	}

	_endflag = -1;
}

// FIXME: This is called from SoundGen classes on unsynchronized background threads.
void SoundMgr::soundIsFinished() {
	if (_endflag != -1)
		_vm->setFlagOrVar(_endflag, true);

	if (_playingSound != -1)
		_vm->_game.sounds[_playingSound]->stop();
	_playingSound = -1;
	_endflag = -1;
}

SoundMgr::SoundMgr(AgiBase *agi, Audio::Mixer *pMixer) {
	_vm = agi;
	_endflag = -1;
	_playingSound = -1;

	switch (_vm->_soundemu) {
	default:
	case SOUND_EMU_NONE:
	case SOUND_EMU_AMIGA:
	case SOUND_EMU_MAC:
	case SOUND_EMU_PC:
		_soundGen = new SoundGenSarien(_vm, pMixer);
		break;
	case SOUND_EMU_PCJR:
		_soundGen = new SoundGenPCJr(_vm, pMixer);
		break;
	case SOUND_EMU_APPLE2:
		_soundGen = new SoundGenA2(_vm, pMixer);
		break;
	case SOUND_EMU_APPLE2GS:
		_soundGen = new SoundGen2GS(_vm, pMixer);
		break;
	case SOUND_EMU_COCO3:
		_soundGen = new SoundGenCoCo3(_vm, pMixer);
		break;
	case SOUND_EMU_MIDI:
		_soundGen = new SoundGenMIDI(_vm, pMixer);
		break;
	}
}

SoundMgr::~SoundMgr() {
	stopSound();

	delete _soundGen;
}

} // End of namespace Agi
