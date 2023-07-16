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

#include "scumm/actor.h"
#include "scumm/file.h"
#include "scumm/imuse/imuse.h"
#include "scumm/resource.h"
#include "scumm/scumm.h"
#include "scumm/he/sound_he.h"
#include "scumm/he/intern_he.h"
#include "scumm/util.h"

#include "common/config-manager.h"
#include "common/memstream.h"
#include "common/timer.h"
#include "common/util.h"

#include "audio/audiostream.h"
#include "audio/decoders/adpcm.h"
#include "audio/mixer.h"
#include "audio/decoders/raw.h"
#include "audio/decoders/wave.h"
#include "audio/decoders/mp3.h"
#include "audio/decoders/vorbis.h"
#include "audio/decoders/flac.h"

namespace Scumm {

SoundHE::SoundHE(ScummEngine *parent, Audio::Mixer *mixer, Common::Mutex *mutex)
	:
	Sound(parent, mixer, false),
	_vm((ScummEngine_v60he *)parent),
	_overrideFreq(0),
	_heSpoolingMusicTable(nullptr),
	_heMusicTracks(0),
	_mutex(mutex) {

	memset(_heChannel, 0, sizeof(_heChannel));
	_heSoundChannels = new Audio::SoundHandle[8]();
}

SoundHE::~SoundHE() {
	free(_heSpoolingMusicTable);
	delete[] _heSoundChannels;

	if (_spoolingMusicFile.isOpen())
		_spoolingMusicFile.close();
}

void SoundHE::startSound(int sound, int heOffset, int heChannel, int heFlags, int heFreq, int hePan, int heVol) {
	if (_vm->VAR_LAST_SOUND != 0xFF)
		_vm->VAR(_vm->VAR_LAST_SOUND) = sound;

	if (heFlags & ScummEngine_v70he::HESndFlags::HE_SND_QUICK_START) {
		triggerDigitalSound(sound, heOffset, heChannel, heFlags);
	} else {
		Sound::startSound(sound, heOffset, heChannel, heFlags, heFreq, hePan, heVol);
	}
}

void SoundHE::addSoundToQueue(int sound, int heOffset, int heChannel, int heFlags, int heFreq, int hePan, int heVol) {
	int i = _soundQueuePos;
	while (i--) {
		if (_soundQueue[i].sound == sound && !(heFlags & ScummEngine_v70he::HESndFlags::HE_SND_APPEND))
			// Sound is already queued
			return;
	}

	Sound::addSoundToQueue(sound, heOffset, heChannel, heFlags, heFreq, hePan, heVol);
}

void SoundHE::modifySound(int sound, int offset, int frequencyShift, int pan, int volume, int flags) {
	int chan = findSoundChannel(sound);
	if (chan >= 0 && _heChannel[chan].sound) {
		// Modify the current playing sound
		if (flags & ScummEngine_v70he::HESndFlags::HE_SND_VOL)
			_mixer->setChannelVolume(_heSoundChannels[chan], volume);

		// Convert the pan range from (0, 127) to (-127, 127)
		int scaledPan = (pan != 64) ? 2 * pan - 127 : 0;
		if (flags & ScummEngine_v70he::HESndFlags::HE_SND_PAN)
			_mixer->setChannelBalance(_heSoundChannels[chan], scaledPan);

		if (flags & ScummEngine_v70he::HESndFlags::HE_SND_FREQUENCY) {
			int newFrequency = (_heChannel[chan].frequency * frequencyShift) / HSND_SOUND_FREQ_BASE;
			if (newFrequency)
				_mixer->setChannelRate(_heSoundChannels[chan], newFrequency);
		}
	}
	// TODO: Implement spooled sound case
}

void SoundHE::processSoundQueues() {
	int snd, heOffset, heChannel, heFlags, heFreq, hePan, heVol;

	if (_vm->_game.heversion >= 72) {
		for (int i = 0; i <_soundQueuePos; i++) {
			snd = _soundQueue[i].sound;
			heOffset = _soundQueue[i].offset;
			heChannel = _soundQueue[i].channel;
			heFlags = _soundQueue[i].flags;
			heFreq = _soundQueue[_soundQueuePos].freq;
			hePan = _soundQueue[_soundQueuePos].pan;
			heVol = _soundQueue[_soundQueuePos].vol;
			if (snd)
				triggerSound(snd, heOffset, heChannel, heFlags, heFreq, hePan, heVol);
		}
		_soundQueuePos = 0;
	} else {
		while (_soundQueuePos) {
			_soundQueuePos--;
			snd = _soundQueue[_soundQueuePos].sound;
			heOffset = _soundQueue[_soundQueuePos].offset;
			heChannel = _soundQueue[_soundQueuePos].channel;
			heFlags = _soundQueue[_soundQueuePos].flags;
			heFreq = _soundQueue[_soundQueuePos].freq;
			hePan = _soundQueue[_soundQueuePos].pan;
			heVol = _soundQueue[_soundQueuePos].vol;
			if (snd)
				triggerSound(snd, heOffset, heChannel, heFlags, heFreq, hePan, heVol);
		}
	}

	Sound::processSoundQueues();
}

int SoundHE::isSoundRunning(int sound) const {
	if (_vm->_game.heversion >= 70) {
		if (sound >= 10000) {
			return _mixer->getSoundID(_heSoundChannels[sound - 10000]);
		}
	} else if (_vm->_game.heversion >= 60) {
		if (sound == -2) {
			sound = _heChannel[0].sound;
		} else if (sound == -1) {
			sound = _currentMusic;
		}
	}

	if (_mixer->isSoundIDActive(sound))
		return sound;

	if (isSoundInQueue(sound))
		return sound;

	if (_vm->_musicEngine && _vm->_musicEngine->getSoundStatus(sound))
		return sound;

	return 0;
}

void SoundHE::stopSound(int sound) {
	if (_vm->_game.heversion >= 70) {
		if ( sound >= 10000) {
			stopSoundChannel(sound - 10000);
		}
	} else if (_vm->_game.heversion >= 60) {
		if (sound == -2) {
			sound = _heChannel[0].sound;
		} else if (sound == -1) {
			sound = _currentMusic;
		}
	}

	Sound::stopSound(sound);

	for (int i = 0; i < ARRAYSIZE(_heChannel); i++) {
		if (_heChannel[i].sound == sound) {
			_heChannel[i].sound = 0;
			_heChannel[i].priority = 0;
			_heChannel[i].frequency = 0;
			_heChannel[i].timer = 0;
			_heChannel[i].hasSoundTokens = false;
			_heChannel[i].codeOffs = 0;
			memset(_heChannel[i].soundVars, 0, sizeof(_heChannel[i].soundVars));
		}
	}

	if (_vm->_game.heversion >= 70 && sound == 1) {
		_vm->_haveMsg = 3;
		_vm->_talkDelay = 0;
	}
}

void SoundHE::stopAllSounds() {
	// Clear sound channels for HE games
	memset(_heChannel, 0, sizeof(_heChannel));

	Sound::stopAllSounds();
}

int SoundHE::findSoundChannel(int sound) {
	if (sound >= HSND_CHANNEL_0) {
		int channel = sound - HSND_CHANNEL_0;

		if (channel < 0 || channel > HSND_MAX_CHANNELS - 1) {
			error("SoundHE::findSoundChannel(): ERROR: Channel %d out of range (%d-%d)",
				channel, 0, HSND_MAX_CHANNELS - 1);
		}

		return channel;
	}

	for (int i = 0; i < ARRAYSIZE(_heChannel); ++i) {
		if (sound == _heChannel[i].sound) {
			return i;
		}
	}

	return -1;
}

void SoundHE::setupSound() {
	Sound::setupSound();

	if (_vm->_game.heversion >= 70) {
		setupHEMusicFile();
	}
}

void SoundHE::stopSoundChannel(int chan) {
	if (_heChannel[chan].sound == HSND_TALKIE_SLOT || chan == _vm->VAR(_vm->VAR_TALK_CHANNEL)) {
		_vm->_haveMsg = 3;
		_vm->_talkDelay = 0;
	}

	_mixer->stopHandle(_heSoundChannels[chan]);

	_heChannel[chan].sound = 0;
	_heChannel[chan].priority = 0;
	_heChannel[chan].frequency = 0;
	_heChannel[chan].timer = 0;
	_heChannel[chan].hasSoundTokens = false;
	_heChannel[chan].codeOffs = 0;
	memset(_heChannel[chan].soundVars, 0, sizeof(_heChannel[chan].soundVars));

	for (int i = 0; i < ARRAYSIZE(_soundQueue); i++) {
		if (_soundQueue[i].channel == chan) {
			_soundQueue[i].sound = 0;
			_soundQueue[i].offset = 0;
			_soundQueue[i].channel = 0;
			_soundQueue[i].flags = 0;
		}
	}
}

int SoundHE::getNextDynamicChannel() {
	int firstChan, minAge, chosenChan;

	firstChan = _vm->VAR(_vm->VAR_START_DYN_SOUND_CHANNELS);

	if (firstChan < 0) {
		_vm->VAR(_vm->VAR_START_DYN_SOUND_CHANNELS) = HSND_MAX_CHANNELS;
		firstChan = HSND_MAX_CHANNELS;
	}

	if (firstChan < HSND_MAX_CHANNELS) {
		for (int i = firstChan; i < HSND_MAX_CHANNELS; i++) {
			if (!_heChannel[i].sound)
				return i;
		}

		minAge = INT_MAX;
		chosenChan = -1;

		for (int i = firstChan; i < HSND_MAX_CHANNELS; i++) {

			if (_heChannel[i].age <= minAge) {
				minAge = _heChannel[i].age;
				chosenChan = i;
			}
		}

		// Found it!
		if (chosenChan != -1)
			return chosenChan;

		// ...just get the first one, then :-)
		return firstChan;
	}

	return 1;
}

bool SoundHE::isSoundCodeUsed(int sound) {
	int chan = -1;
	for (int i = 0; i < ARRAYSIZE(_heChannel); i ++) {
		if (_heChannel[i].sound == sound)
			chan = i;
	}

	if (chan != -1 && _mixer->isSoundHandleActive(_heSoundChannels[chan])) {
		return _heChannel[chan].hasSoundTokens;
	} else {
		return false;
	}
}

int SoundHE::getSoundPos(int sound) {
	int chan = -1;
	for (int i = 0; i < ARRAYSIZE(_heChannel); i ++) {
		if (_heChannel[i].sound == sound)
			chan = i;
	}

	if (chan != -1 && _mixer->isSoundHandleActive(_heSoundChannels[chan])) {
		int time = _vm->getHETimer(chan + 4) * _heChannel[chan].frequency / 1000;
		return time;
	} else {
		return 0;
	}
}

int SoundHE::getSoundVar(int sound, int var) {
	if (_vm->_game.heversion >= 90 && var == 26) {
		return isSoundCodeUsed(sound);
	}

	assertRange(0, var, 25, "sound variable");

	int chan = -1;
	for (int i = 0; i < ARRAYSIZE(_heChannel); i ++) {
		if (_heChannel[i].sound == sound)
			chan = i;
	}

	if (chan != -1 && _mixer->isSoundHandleActive(_heSoundChannels[chan])) {
		debug(5, "getSoundVar: sound %d var %d result %d", sound, var, _heChannel[chan].soundVars[var]);
		return _heChannel[chan].soundVars[var];
	} else {
		return 0;
	}
}

void SoundHE::setSoundVar(int sound, int var, int val) {
	assertRange(0, var, 25, "sound variable");

	int chan = -1;
	for (int i = 0; i < ARRAYSIZE(_heChannel); i ++) {
		if (_heChannel[i].sound == sound)
			chan = i;
	}

	if (chan != -1) {
		debug(5, "setSoundVar: sound %d var %d val %d", sound, var, val);
		_heChannel[chan].soundVars[var] = val;
	}
}

void SoundHE::setOverrideFreq(int freq) {
	_overrideFreq = freq;
}

void SoundHE::setupHEMusicFile() {
	uint32 id, len;
	Common::String musicFilename(_vm->generateFilename(-4));

	if (_spoolingMusicFile.open(musicFilename)) {

		id = _spoolingMusicFile.readUint32BE();
		len = _spoolingMusicFile.readUint32BE();
		if (id == MKTAG('S', 'O', 'N', 'G')) {

			// Older versions had a much simpler file structure
			if (_vm->_game.heversion < 80) {
				// Skip header wrapping file
				_spoolingMusicFile.seek(16, SEEK_SET);
				_heMusicTracks = _spoolingMusicFile.readUint32LE();
			} else {
				// HE 80 and above
				id = _spoolingMusicFile.readUint32BE();
				len = _spoolingMusicFile.readUint32BE();
				if (id == MKTAG('S', 'G', 'H', 'D')) {
					_heMusicTracks = _spoolingMusicFile.readUint32LE();
					_spoolingMusicFile.seek(len - 8 - 4, SEEK_CUR);
				} else {
					_spoolingMusicFile.close();
					debug(5, "setupHEMusicFile(): Invalid spooling file '%s', couldn't find SGHD tag, found %s", musicFilename.c_str(), tag2str(id));
					return;
				}
			}

			debug(5, "setupHEMusicFile(): music files count = %d", _heMusicTracks);
			_heSpoolingMusicTable = (HESpoolingMusicItem *)malloc(_heMusicTracks * sizeof(HESpoolingMusicItem));

			if (_heSpoolingMusicTable != nullptr) {
				for (int i = 0; i < _heMusicTracks; i++) {

					// For later versions we check that we are actually reading a SGEN section...
					if (_vm->_game.heversion >= 80) {
						id = _spoolingMusicFile.readUint32BE();
						len = _spoolingMusicFile.readUint32BE();
						if (id != MKTAG('S', 'G', 'E', 'N')) {
							_spoolingMusicFile.close();
							debug(5, "setupHEMusicFile(): Invalid spooling file '%s', couldn't find SGEN tag, found %s", musicFilename.c_str(), tag2str(id));
							return;
						}
					}

					_heSpoolingMusicTable[i].song = _spoolingMusicFile.readSint32LE();
					_heSpoolingMusicTable[i].offset = _spoolingMusicFile.readSint32LE();
					_heSpoolingMusicTable[i].size = _spoolingMusicFile.readSint32LE();

					int amountToRead = _vm->_game.heversion >= 80 ? 9 : 13;
					int readAmount = 0;
					for (readAmount = 0; readAmount < amountToRead; readAmount++) {
						_heSpoolingMusicTable[i].filename[readAmount] = _spoolingMusicFile.readByte();

						// Early string termination
						if (_heSpoolingMusicTable[i].filename[readAmount] == '\0')
							break;
					}
					// Not all filenames have a string termination char, let's include it;
					// we don't need to do that if we had an early string termination
					if (readAmount == amountToRead)
						_heSpoolingMusicTable[i].filename[amountToRead] = '\0';

					debug(5, "setupHEMusicFile(): read music file '%s' song %d, offset %d, size %d",
							_heSpoolingMusicTable[i].filename,
							_heSpoolingMusicTable[i].song,
							_heSpoolingMusicTable[i].offset,
							_heSpoolingMusicTable[i].size);
				}
			} else {
				debug(5, "setupHEMusicFile(): Can't allocate table for spooling music file '%s'", musicFilename.c_str());
			}
		} else {
			debug(5, "setupHEMusicFile(): Invalid file '%s', couldn't find SONG tag, found %s", musicFilename.c_str(), tag2str(id));
		}
	} else {
		debug(5, "setupHEMusicFile(): Can't open spooling music file '%s'", musicFilename.c_str());
	}
}

bool SoundHE::getHEMusicDetails(int id, int &musicOffs, int &musicSize) {
	int i;

	for (i = 0; i < _heMusicTracks; i++) {
		if (_heSpoolingMusicTable[i].song == id) {
			musicOffs = _heSpoolingMusicTable[i].offset;
			musicSize = _heSpoolingMusicTable[i].size;
			return 1;
		}
	}

	return 0;
}

void SoundHE::handleSoundFrame() {
	_soundsDebugFrameCounter++;

	if (_stopActorTalkingFlag) {
		_vm->stopTalk();
		_stopActorTalkingFlag = false;
	}

	unqueueSoundCallbackScripts();

	runSoundCode();
	checkSoundTimeouts();
}

void SoundHE::unqueueSoundCallbackScripts() {
	Common::StackLock lock(*_mutex);

	for (int i = 0; i < _soundCallbacksQueueSize; i++) {

		if (_soundCallbackScripts[i].sound) {
			int args[NUM_SCRIPT_LOCAL];

			memset(args, 0, sizeof(args));
			args[0] = _soundCallbackScripts[i].sound;
			args[1] = _soundCallbackScripts[i].channel;
			args[2] = 0;
			args[3] = 0;

			_vm->runScript(_vm->VAR(_vm->VAR_SOUND_CALLBACK_SCRIPT), 0, 0, args);
		}

		_soundCallbackScripts[i].sound = 0;
		_soundCallbackScripts[i].channel = 0;
		_soundCallbackScripts[i].whatFrame = 0;
	}

	_soundCallbacksQueueSize = 0;
}

void SoundHE::checkSoundTimeouts() {
	for (int chan = 0; chan < ARRAYSIZE(_heChannel); chan++) {
		if (_heChannel[chan].sound == 0 || _heChannel[chan].timer == 0)
			continue;

		if (_vm->getHETimer(chan + HSND_TIMER_SLOT) > _heChannel[chan].timer) {
			digitalSoundCallback(HSND_SOUND_TIMEOUT, chan);
		}
	}
}

void SoundHE::digitalSoundCallback(int message, int channel) {
	// The action done for each sound is always the same;
	// it's useful to keep track of the message for debugging
	// purposes though...
	switch (message) {
	case HSND_SOUND_TIMEOUT:
		debug(5, "SoundHE::digitalSoundCallback(): TIMEOUT, channel %d", channel);
		break;
	case HSND_SOUND_ENDED:
		debug(5, "SoundHE::digitalSoundCallback(): ENDED, channel %d", channel);
		break;
	case HSND_SOUND_STOPPED:
		debug(5, "SoundHE::digitalSoundCallback(): STOPPED, channel %d", channel);
		break;
	default:
		warning("SoundHE::digitalSoundCallback(): WARNING: invalid message (%d), channel = %d", message, channel);
		break;
	}

	_inSoundCallbackFlag = true;

	switch (message) {
	case HSND_SOUND_TIMEOUT:
	case HSND_SOUND_ENDED:
	case HSND_SOUND_STOPPED:

		if (_heChannel[channel].sound == HSND_TALKIE_SLOT) {
			// In the original this was used to prevent an edge case bug
			// which caused recursive resource accesses/allocations.
			// I think there's no harm in doing that ourselves too...
			if (_vm->_insideCreateResource == 0) {
				_vm->stopTalk();
			} else {
				_stopActorTalkingFlag = true;
			}
		}

		_heChannel[channel].clearChannel();

		queueSoundCallbackScript(_heChannel[channel].sound, channel, message);
		break;
	}

	_inSoundCallbackFlag = false;
}

void SoundHE::queueSoundCallbackScript(int sound, int channel, int message) {
	// Avoid queueing up a sound callback if the mixer is not available.
	if (!_mixer->isReady())
		return;

	// Check if we are about to duplicate this event...
	for (int i = 0; i < _soundCallbacksQueueSize; i++) {

		if ((sound == _soundCallbackScripts[i].sound) &&
			(channel == _soundCallbackScripts[i].channel)) {

			_soundAlreadyInQueueCount++;

			debug(5, "SoundHE::queueSoundCallbackScript(): callback for channel %d, sound %d, already in list.",
				channel, sound);

			return;
		}
	}

	// Finally queue up sound...
	_soundCallbackScripts[_soundCallbacksQueueSize].sound = sound;
	_soundCallbackScripts[_soundCallbacksQueueSize].channel = channel;
	_soundCallbackScripts[_soundCallbacksQueueSize].whatFrame = _soundsDebugFrameCounter;

	_soundCallbacksQueueSize++;

	if (_soundCallbacksQueueSize >= HSND_MAX_CALLBACK_SCRIPTS) {
		error("SoundHE::queueSoundCallbackScript(): ERROR: Got too many sound callbacks (got %d, max %d), message %d",
			_soundCallbacksQueueSize,
			HSND_MAX_CALLBACK_SCRIPTS,
			message);
	}
}

void SoundHE::runSoundCode() {
	byte *soundPtr;
	int chan, soundPos, len, freq;

	for (chan = 0; chan < ARRAYSIZE(_heChannel); chan++) {
		if (_heChannel[chan].sound == 0) {
			continue;
		}

		if (_heChannel[chan].codeOffs == -1) {
			continue;
		}

		soundPos = _vm->getHETimer(chan + HSND_TIMER_SLOT) * _heChannel[chan].frequency / 1000;
		soundPos += _vm->VAR(_vm->VAR_SOUND_TOKEN_OFFSET);

		if (soundPos < 0)
			soundPos = 0;

		if (_heChannel[chan].codeBuffer == nullptr) {
			soundPtr = _vm->getResourceAddress(rtSound, _heChannel[chan].sound);
		} else {
			soundPtr = _heChannel[chan].codeBuffer;
		}

		assert(soundPtr);

		soundPtr += _heChannel[chan].codeOffs;

		len = READ_LE_UINT16(soundPtr);
		freq = READ_LE_UINT32(soundPtr + sizeof(uint16));

		while (soundPos > freq) {
			debug(5, "Channel %d Timer %d Time %d", chan, soundPos, freq);

			processSoundOpcodes(_heChannel[chan].sound, soundPtr + sizeof(uint16) + sizeof(uint32), _heChannel[chan].soundVars);

			_heChannel[chan].codeOffs += len;

			soundPtr += len;

			len = READ_LE_UINT16(soundPtr);
			freq = READ_LE_UINT32(soundPtr + sizeof(uint16));

			if (len == 0) {
				_heChannel[chan].codeOffs = -1;
				break;
			}
		}
	}
}

void SoundHE::processSoundOpcodes(int sound, byte *codePtr, int *soundVars) {
	int arg, opcode, var, val;

	while (READ_LE_UINT16(codePtr) != 0) {
		codePtr += 2;
		opcode = READ_LE_UINT16(codePtr); codePtr += 2;
		opcode = (opcode & ~HSND_SBNG_MAGIC_MASK) >> 4;
		arg = opcode & HSND_SBNG_VARORVAL;
		opcode &= ~HSND_SBNG_VARORVAL;

		debug(5, "processSoundOpcodes: sound %d opcode %d", sound, opcode);

		switch (opcode) {
		case HSND_SBNG_END: // Continue
			break;
		case HSND_SBNG_FACE: // Set talk state
			val = READ_LE_UINT16(codePtr); codePtr += 2;
			setSoundVar(sound, 19, val);
			break;
		case HSND_SBNG_SET_SET: // Set var
			var = READ_LE_UINT16(codePtr); codePtr += 2;
			val = READ_LE_UINT16(codePtr); codePtr += 2;
			if (arg == 2) {
				val = getSoundVar(sound, val);
			}
			setSoundVar(sound, var, val);
			break;
		case HSND_SBNG_SET_ADD: // Add
			var = READ_LE_UINT16(codePtr); codePtr += 2;
			val = READ_LE_UINT16(codePtr); codePtr += 2;
			if (arg == 2) {
				val = getSoundVar(sound, val);
			}
			val = getSoundVar(sound, var) + val;
			setSoundVar(sound, var, val);
			break;
		case HSND_SBNG_SET_SUB: // Subtract
			var = READ_LE_UINT16(codePtr); codePtr += 2;
			val = READ_LE_UINT16(codePtr); codePtr += 2;
			if (arg == 2) {
				val = getSoundVar(sound, val);
			}
			val = getSoundVar(sound, var) - val;
			setSoundVar(sound, var, val);
			break;
		case HSND_SBNG_SET_MUL: // Multiple
			var = READ_LE_UINT16(codePtr); codePtr += 2;
			val = READ_LE_UINT16(codePtr); codePtr += 2;
			if (arg == 2) {
				val = getSoundVar(sound, val);
			}
			val = getSoundVar(sound, var) * val;
			setSoundVar(sound, var, val);
			break;
		case HSND_SBNG_SET_DIV: // Divide
			var = READ_LE_UINT16(codePtr); codePtr += 2;
			val = READ_LE_UINT16(codePtr); codePtr += 2;
			if (arg == 2) {
				val = getSoundVar(sound, val);
			}
			if (!val) {
				val = 1; // Safeguard for division by zero
				warning("Incorrect value 0 for processSoundOpcodes() kludge DIV");
			}
			val = getSoundVar(sound, var) / val;
			setSoundVar(sound, var, val);
			break;
		case HSND_SBNG_SET_INC: // Increment
			var = READ_LE_UINT16(codePtr); codePtr += 2;
			val = getSoundVar(sound, var) + 1;
			setSoundVar(sound, var, val);
			break;
		case HSND_SBNG_SET_DEC: // Decrement
			var = READ_LE_UINT16(codePtr); codePtr += 2;
			val = getSoundVar(sound, var) - 1;
			setSoundVar(sound, var, val);
			break;
		default:
			error("Illegal sound %d opcode %d", sound, opcode);
		}
	}
}

byte *findSoundTag(uint32 tag, byte *ptr) {
	byte *endPtr;
	uint32 offset, size;

	if (READ_BE_UINT32(ptr) == MKTAG('W','S','O','U')) {
		ptr += 8;
	}

	if (READ_BE_UINT32(ptr) != MKTAG('R','I','F','F'))
		return nullptr;

	endPtr = (ptr + 12);
	size = READ_LE_UINT32(ptr + 4);

	while (endPtr < ptr + size) {
		offset = READ_LE_UINT32(endPtr + 4);

		if (offset <= 0)
			error("Illegal chunk length - %d bytes.", offset);

		if (offset > size)
			error("Chunk extends beyond file end - %d versus %d.", offset, size);

		if (READ_BE_UINT32(endPtr) == tag)
			return endPtr;

		endPtr = endPtr + offset + 8;
	}

	return nullptr;
}

void SoundHE::triggerSound(int soundId, int heOffset, int heChannel, int heFlags, int heFreq, int hePan, int heVol) {
	if (_vm->_game.heversion >= 95) {
		if (heChannel == HSND_DYN_SOUND_CHAN) {
			heChannel = getNextDynamicChannel();
		}
	}

	if (soundId >= _vm->_numSounds) {
		triggerSpoolingSound(soundId, heOffset, heChannel, heFlags, heFreq, hePan, heVol);
		return;
	}

	byte *soundAddr = (byte *)_vm->getResourceAddress(rtSound, soundId);

	if ((READ_BE_UINT32(soundAddr) == MKTAG('D', 'I', 'G', 'I')) || (READ_BE_UINT32(soundAddr) == MKTAG('T', 'A', 'L', 'K'))) {
		triggerDigitalSound(soundId, heOffset, heChannel, heFlags);
	} else if (READ_BE_UINT32(soundAddr) == MKTAG('M', 'I', 'D', 'I')) {
		// Nothing happens in the original as well
		return;
	} else if (READ_BE_UINT32(soundAddr) == MKTAG('W', 'S', 'O', 'U')) {
		triggerRIFFSound(soundId, heOffset, heChannel, heFlags, heFreq, hePan, heVol);
	} else if (READ_BE_UINT32(soundAddr) == MKTAG('X', 'S', 'O', 'U')) {
		triggerXSOUSound(soundId, heOffset, heChannel, heFlags);
	} else {
		error("SoundHE::triggerSound(): Illegal sound %d type %s", soundId, tag2str(READ_BE_UINT32(soundAddr)));
	}
}

void SoundHE::triggerSpoolingSound(int soundId, int heOffset, int heChannel, int heFlags, int heFreq, int hePan, int heVol) {

}

void SoundHE::triggerRIFFSound(int soundId, int heOffset, int heChannel, int heFlags, int heFreq, int hePan, int heVol) {

}

void SoundHE::triggerXSOUSound(int soundId, int heOffset, int heChannel, int heFlags) {
	error("SoundHE::triggerXSOUSound(): unimplemented XSOU format for sound %d", soundId);
}

void SoundHE::triggerDigitalSound(int soundId, int heOffset, int heChannel, int heFlags) {
	Audio::RewindableAudioStream *stream = nullptr;
	byte *ptr, *spoolPtr;
	int size = -1;
	int priority, rate;
	byte flags = Audio::FLAG_UNSIGNED;

	Audio::Mixer::SoundType type = Audio::Mixer::kSFXSoundType;
	if (soundId > _vm->_numSounds)
		type = Audio::Mixer::kMusicSoundType;
	else if (soundId == 1)
		type = Audio::Mixer::kSpeechSoundType;


	if (heChannel == -1)
		heChannel = (_vm->VAR_START_DYN_SOUND_CHANNELS != 0xFF) ? getNextDynamicChannel() : 1;

	debug(5,"triggerDigitalSound: soundId %d heOffset %d heChannel %d heFlags %d", soundId, heOffset, heChannel, heFlags);

	if (soundId >= 10000) {
		// Special codes, used in pjgames
		return;
	}

	if (soundId > _vm->_numSounds) {
		int music_offs;
		Common::File musicFile;
		Common::String buf(_vm->generateFilename(-4));

		if (musicFile.open(buf) == false) {
			warning("triggerDigitalSound: Can't open music file %s", buf.c_str());
			return;
		}
		if (!getHEMusicDetails(soundId, music_offs, size)) {
			debug(0, "triggerDigitalSound: musicID %d not found", soundId);
			return;
		}

		musicFile.seek(music_offs, SEEK_SET);

		_mixer->stopHandle(_heSoundChannels[heChannel]);
		spoolPtr = _vm->_res->createResource(rtSpoolBuffer, heChannel, size);
		assert(spoolPtr);
		musicFile.read(spoolPtr, size);
		musicFile.close();

		if (_vm->_game.heversion == 70) {
			// Try to load high quality audio file if found
			stream = tryLoadAudioOverride(soundId);

			if (!stream) {
				stream = Audio::makeRawStream(spoolPtr, size, 11025, flags, DisposeAfterUse::NO);
			}

			_mixer->playStream(type, &_heSoundChannels[heChannel], stream, soundId);
			return;
		}
	}

	if (soundId > _vm->_numSounds) {
		ptr = _vm->getResourceAddress(rtSpoolBuffer, heChannel);
	} else {
		ptr = _vm->getResourceAddress(rtSound, soundId);
	}

	if (!ptr) {
		return;
	}

	// Support for sound in later HE games
	if (READ_BE_UINT32(ptr) == MKTAG('R','I','F','F') || READ_BE_UINT32(ptr) == MKTAG('W','S','O','U')) {
		uint16 compType;
		int blockAlign;
		int samplesPerBlock;
		int codeOffs = -1;

		priority = (soundId > _vm->_numSounds) ? 255 : *(ptr + 18);

		byte *sbngPtr = findSoundTag(MKTAG('S','B','N','G'), ptr);
		if (sbngPtr != nullptr) {
			codeOffs = sbngPtr - ptr + 8;
		}

		if (_mixer->isSoundHandleActive(_heSoundChannels[heChannel])) {
			int curSnd = _heChannel[heChannel].sound;
			if (curSnd == 1 && soundId != 1)
				return;
			if (curSnd != 0 && curSnd != 1 && soundId != 1 && _heChannel[heChannel].priority > priority)
				return;
		}

		if (READ_BE_UINT32(ptr) == MKTAG('W','S','O','U'))
			ptr += 8;

		size = READ_LE_UINT32(ptr + 4);
		Common::MemoryReadStream memStream(ptr, size);

		if (!Audio::loadWAVFromStream(memStream, size, rate, flags, &compType, &blockAlign, &samplesPerBlock)) {
			error("triggerDigitalSound: Not a valid WAV file (%d)", soundId);
		}

		assert(heOffset >= 0 && heOffset < size);

		// FIXME: Disabled sound offsets, due to asserts been triggered
		heOffset = 0;

		_vm->setHETimer(heChannel + 4);
		_heChannel[heChannel].sound = soundId;
		_heChannel[heChannel].priority = priority;
		_heChannel[heChannel].frequency = rate;
		_heChannel[heChannel].hasSoundTokens = (codeOffs != -1);
		_heChannel[heChannel].codeOffs = codeOffs;
		memset(_heChannel[heChannel].soundVars, 0, sizeof(_heChannel[heChannel].soundVars));

		// TODO: Extra sound flags
		if (heFlags & 1) {
			_heChannel[heChannel].timer = 0;
		} else {
			_heChannel[heChannel].timer = size * 1000 / (rate * blockAlign);
		}

		_mixer->stopHandle(_heSoundChannels[heChannel]);
		if (compType == 17) {
			int nChan = (flags & Audio::FLAG_STEREO) ? 2 : 1;
			Audio::AudioStream *voxStream = Audio::makeADPCMStream(&memStream, DisposeAfterUse::NO, size, Audio::kADPCMMSIma, rate, nChan, blockAlign);

			// FIXME: Get rid of this crude hack to turn a ADPCM stream into a raw stream.
			// It seems it is only there to allow looping -- if that is true, we certainly
			// can do without it, using a LoopingAudioStream.

			if (_heChannel[heChannel].timer)
				_heChannel[heChannel].timer = (int)(((int64)size * samplesPerBlock * 1000) / ((int64)rate * blockAlign * nChan));

			byte *sound = (byte *)malloc(size * 4);
			/* On systems where it matters, malloc will return
			 * even addresses, so the use of (void *) in the
			 * following cast shuts the compiler from warning
			 * unnecessarily. */
			size = voxStream->readBuffer((int16 *)(void *)sound, size * 2);
			size *= 2; // 16bits.
			delete voxStream;

			// makeADPCMStream returns a stream in native endianness, but RawMemoryStream
			// defaults to big endian. If we're on a little endian system, set the LE flag.
#ifdef SCUMM_LITTLE_ENDIAN
			flags |= Audio::FLAG_LITTLE_ENDIAN;
#endif
			stream = Audio::makeRawStream(sound + heOffset, size - heOffset, rate, flags);
		} else {
			stream = Audio::makeRawStream(ptr + memStream.pos() + heOffset, size - heOffset, rate, flags, DisposeAfterUse::NO);
		}
		_mixer->playStream(type, &_heSoundChannels[heChannel],
						Audio::makeLoopingAudioStream(stream, (heFlags & 1) ? 0 : 1), soundId);
	}
	// Support for sound in Humongous Entertainment games
	else if (READ_BE_UINT32(ptr) == MKTAG('D','I','G','I') || READ_BE_UINT32(ptr) == MKTAG('T','A','L','K')) {
		byte *sndPtr = ptr;
		int codeOffs = -1;

		priority = (soundId > _vm->_numSounds) ? 255 : *(ptr + 18);
		rate = READ_LE_UINT16(ptr + 22);

		// Skip DIGI/TALK (8) and HSHD (24) blocks
		ptr += 32;

		if (_mixer->isSoundHandleActive(_heSoundChannels[heChannel])) {
			int curSnd = _heChannel[heChannel].sound;
			if (curSnd == 1 && soundId != 1)
				return;
			if (curSnd != 0 && curSnd != 1 && soundId != 1 && _heChannel[heChannel].priority > priority)
				return;
		}

		if (READ_BE_UINT32(ptr) == MKTAG('S','B','N','G')) {
			codeOffs = ptr - sndPtr + 8;
			ptr += READ_BE_UINT32(ptr + 4);
		}

		assert(READ_BE_UINT32(ptr) == MKTAG('S','D','A','T'));
		size = READ_BE_UINT32(ptr + 4) - 8;
		if (heOffset < 0 || heOffset > size) {
			// Occurs when making fireworks in puttmoon
			heOffset = 0;
		}
		size -= heOffset;

		if (_overrideFreq) {
			// Used by the piano in Fatty Bear's Birthday Surprise
			rate = _overrideFreq;
			_overrideFreq = 0;
		}

		// Try to load high quality audio file if found
		int newDuration;
		stream = tryLoadAudioOverride(soundId, &newDuration);
		if (stream != nullptr && soundId == 1) {
			// Disable lip sync if the speech audio was overriden
			codeOffs = -1;
		}

		_vm->setHETimer(heChannel + 4);
		_heChannel[heChannel].sound = soundId;
		_heChannel[heChannel].priority = priority;
		_heChannel[heChannel].frequency = rate;
		_heChannel[heChannel].hasSoundTokens = (codeOffs != -1);
		_heChannel[heChannel].codeOffs = codeOffs;
		memset(_heChannel[heChannel].soundVars, 0, sizeof(_heChannel[heChannel].soundVars));

		// TODO: Extra sound flags
		if (heFlags & 1) {
			_heChannel[heChannel].timer = 0;
		} else {
			if (stream != nullptr) {
				_heChannel[heChannel].timer = newDuration;
			} else {
				_heChannel[heChannel].timer = size * 1000 / rate;
			}
		}

		_mixer->stopHandle(_heSoundChannels[heChannel]);

		if (!stream) {
			stream = Audio::makeRawStream(ptr + heOffset + 8, size, rate, flags, DisposeAfterUse::NO);
		}
		_mixer->playStream(type, &_heSoundChannels[heChannel],
						Audio::makeLoopingAudioStream(stream, (heFlags & 1) ? 0 : 1), soundId);
	}
	// Support for PCM music in 3DO versions of Humongous Entertainment games
	else if (READ_BE_UINT32(ptr) == MKTAG('M','R','A','W')) {
		priority = *(ptr + 18);
		rate = READ_LE_UINT16(ptr + 22);

		// Skip DIGI (8) and HSHD (24) blocks
		ptr += 32;

		assert(READ_BE_UINT32(ptr) == MKTAG('S','D','A','T'));
		size = READ_BE_UINT32(ptr + 4) - 8;

		byte *sound = (byte *)malloc(size);
		memcpy(sound, ptr + 8, size);

		_mixer->stopID(_currentMusic);
		_currentMusic = soundId;

		stream = Audio::makeRawStream(sound, size, rate, 0);
		_mixer->playStream(Audio::Mixer::kMusicSoundType, nullptr, stream, soundId);
	}
	else if (READ_BE_UINT32(ptr) == MKTAG('M','I','D','I')) {
		if (_vm->_imuse) {
			// This is used in the DOS version of Fatty Bear's
			// Birthday Surprise to change the note on the piano
			// when not using a digitized instrument.
			_vm->_imuse->stopSound(_currentMusic);
			_currentMusic = soundId;
			_vm->_imuse->startSoundWithNoteOffset(soundId, heOffset);
		} else if (_vm->_musicEngine) {
			_vm->_musicEngine->stopSound(_currentMusic);
			_currentMusic = soundId;
			_vm->_musicEngine->startSoundWithTrackID(soundId, heOffset);
		}
	}
}

Audio::RewindableAudioStream *SoundHE::tryLoadAudioOverride(int soundId, int *duration) {
	if (!_vm->_enableAudioOverride) {
		return nullptr;
	}

	const char *formats[] = {
#ifdef USE_FLAC
	    "flac",
#endif
	    "wav",
#ifdef USE_VORBIS
		"ogg",
#endif
#ifdef USE_MAD
	    "mp3",
#endif
	};

	Audio::SeekableAudioStream *(*formatDecoders[])(Common::SeekableReadStream *, DisposeAfterUse::Flag) = {
#ifdef USE_FLAC
	    Audio::makeFLACStream,
#endif
	    Audio::makeWAVStream,
#ifdef USE_VORBIS
	    Audio::makeVorbisStream,
#endif
#ifdef USE_MAD
	    Audio::makeMP3Stream,
#endif
	};

	STATIC_ASSERT(
	    ARRAYSIZE(formats) == ARRAYSIZE(formatDecoders),
	    formats_formatDecoders_must_have_same_size
	);

	const char *type;
	if (soundId == 1) {
		// Speech audio doesn't have a unique ID,
		// so we use the file offset instead.
		// _heTalkOffset is set at startHETalkSound.
		type = "speech";
		soundId = _heTalkOffset;
	} else {
		// Music and sfx share the same prefix.
		type = "sound";
	}

	for (int i = 0; i < ARRAYSIZE(formats); i++) {
		Common::Path pathDir(Common::String::format("%s%d.%s", type, soundId, formats[i]));
		Common::Path pathSub(Common::String::format("%s/%d.%s", type, soundId, formats[i]));

		debug(5, "tryLoadAudioOverride: %s or %s", pathSub.toString().c_str(), pathDir.toString().c_str());

		// First check if the file exists before opening it to
		// reduce the amount of "opening %s failed" in the console.
		// Prefer files in subdirectory.
		Common::File soundFileOverride;
		bool foundFile = (soundFileOverride.exists(pathSub) && soundFileOverride.open(pathSub)) ||
						 (soundFileOverride.exists(pathDir) && soundFileOverride.open(pathDir));
		if (foundFile) {
			soundFileOverride.seek(0, SEEK_SET);
			Common::SeekableReadStream *oStr = soundFileOverride.readStream(soundFileOverride.size());
			soundFileOverride.close();

			Audio::SeekableAudioStream *seekStream = formatDecoders[i](oStr, DisposeAfterUse::YES);
			if (duration != nullptr) {
				*duration = seekStream->getLength().msecs();
			}

			debug(5, "tryLoadAudioOverride: %s loaded from %s", formats[i], soundFileOverride.getName());

			return seekStream;
		}
	}

	debug(5, "tryLoadAudioOverride: file not found");

	return nullptr;
}

void SoundHE::startHETalkSound(uint32 offset) {
	byte *ptr;
	int32 size;

	if (ConfMan.getBool("speech_mute"))
		return;

	if (_sfxFilename.empty()) {
		// This happens in the Pajama Sam's Lost & Found demo, on the
		// main menu screen, so don't make it a fatal error.
		warning("startHETalkSound: Speech file is not found");
		return;
	}

	ScummFile file(_vm);
	if (!_vm->openFile(file, _sfxFilename)) {
		warning("startHETalkSound: Could not open speech file %s", _sfxFilename.c_str());
		return;
	}
	file.setEnc(_sfxFileEncByte);

	// Speech audio doesn't have a unique ID,
	// so we use the file offset instead.
	// _heTalkOffset is used at tryLoadSoundOverride.
	_heTalkOffset = offset;

	_digiSndMode |= DIGI_SND_MODE_TALKIE;
	_vm->_res->nukeResource(rtSound, 1);

	file.seek(offset + 4, SEEK_SET);
	size = file.readUint32BE();
	file.seek(offset, SEEK_SET);

	_vm->_res->createResource(rtSound, 1, size);
	ptr = _vm->getResourceAddress(rtSound, 1);
	file.read(ptr, size);

	int channel = (_vm->VAR_TALK_CHANNEL != 0xFF) ? _vm->VAR(_vm->VAR_TALK_CHANNEL) : 0;
	addSoundToQueue(1, 0, channel, 0);
}

#ifdef ENABLE_HE
void ScummEngine_v80he::createSound(int baseSound, int sound) {
	byte *baseSoundPtr, *soundPtr;
	byte *sbng1Ptr, *sbng2Ptr;
	byte *sdat1Ptr, *sdat2Ptr;
	byte *src, *dst;
	int len, offs, size;
	int sdat1size, sdat2size;

	sbng1Ptr = NULL;
	sbng2Ptr = NULL;

	if (sound == -1) {
		_sndPtrOffs = 0;
		_sndTmrOffs = 0;
		_sndDataSize = 0;
		return;
	}

	if (baseSound != _curSndId) {
		_curSndId = baseSound;
		_sndPtrOffs = 0;
		_sndTmrOffs = 0;
		_sndDataSize = 0;
	}

	baseSoundPtr = getResourceAddress(rtSound, baseSound);
	assert(baseSoundPtr);
	soundPtr = getResourceAddress(rtSound, sound);
	assert(soundPtr);

	int chan = ((SoundHE *)_sound)->findSoundChannel(baseSound);

	if (!findSoundTag(MKTAG('d','a','t','a'), baseSoundPtr)) {
		sbng1Ptr = heFindResource(MKTAG('S','B','N','G'), baseSoundPtr);
		sbng2Ptr = heFindResource(MKTAG('S','B','N','G'), soundPtr);
	}

	if (sbng1Ptr != NULL && sbng2Ptr != NULL) {
		if (chan != -1 && ((SoundHE *)_sound)->_heChannel[chan].codeOffs > 0) {
			// Copy any code left over to the beginning of the code block
			int curOffs = ((SoundHE *)_sound)->_heChannel[chan].codeOffs;

			src = baseSoundPtr + curOffs;
			dst = sbng1Ptr + 8;
			size = READ_BE_UINT32(sbng1Ptr + 4);
			len = sbng1Ptr - baseSoundPtr + size - curOffs;

			memmove(dst, src, len);

			// Now seek to the end of this code block
			dst = sbng1Ptr + 8;
			while ((size = READ_LE_UINT16(dst)) != 0)
				dst += size;
		} else {
			// We're going to overwrite the code block completely
			dst = sbng1Ptr + 8;
		}

		// Reset the current code offset to the beginning of the code block
		((SoundHE *)_sound)->_heChannel[chan].codeOffs = sbng1Ptr - baseSoundPtr + 8;

		// Seek to the end of the code block for sound 2
		byte *tmp = sbng2Ptr + 8;
		while ((offs = READ_LE_UINT16(tmp)) != 0) {
			tmp += offs;
		}

		// Copy the code block for sound 2 to the code block for sound 1
		src = sbng2Ptr + 8;
		len = tmp - sbng2Ptr - 6;
		memcpy(dst, src, len);

		// Rewrite the time for this new code block to be after the sound 1 code block
		int32 time;
		while ((size = READ_LE_UINT16(dst)) != 0) {
			time = READ_LE_UINT32(dst + 2);
			time += _sndTmrOffs;
			WRITE_LE_UINT32(dst + 2, time);
			dst += size;
		}
	}

	// Find the data pointers and sizes
	if (findSoundTag(MKTAG('d','a','t','a'), baseSoundPtr)) {
		sdat1Ptr = findSoundTag(MKTAG('d','a','t','a'), baseSoundPtr);
		assert(sdat1Ptr);
		sdat2Ptr = findSoundTag(MKTAG('d','a','t','a'), soundPtr);
		assert(sdat2Ptr);

		if (!_sndDataSize)
			_sndDataSize = READ_LE_UINT32(sdat1Ptr + 4) - 8;

		sdat2size = READ_LE_UINT32(sdat2Ptr + 4) - 8;
	} else {
		sdat1Ptr = heFindResource(MKTAG('S','D','A','T'), baseSoundPtr);
		assert(sdat1Ptr);
		sdat2Ptr = heFindResource(MKTAG('S','D','A','T'), soundPtr);
		assert(sdat2Ptr);

		_sndDataSize = READ_BE_UINT32(sdat1Ptr + 4) - 8;

		sdat2size = READ_BE_UINT32(sdat2Ptr + 4) - 8;
	}

	sdat1size = _sndDataSize - _sndPtrOffs;
	if (sdat2size < sdat1size) {
		// We have space leftover at the end of sound 1
		// -> Just append sound 2
		src = sdat2Ptr + 8;
		dst = sdat1Ptr + 8 + _sndPtrOffs;
		len = sdat2size;

		memcpy(dst, src, len);

		_sndPtrOffs += sdat2size;
		_sndTmrOffs += sdat2size;
	} else {
		// We might not have enough space leftover at the end of sound 1
		// -> Append as much of possible of sound 2 to sound 1
		src = sdat2Ptr + 8;
		dst = sdat1Ptr + 8 + _sndPtrOffs;
		len = sdat1size;

		memcpy(dst, src, len);

		if (sdat2size != sdat1size) {
			// We don't have enough space
			// -> Start overwriting the beginning of the sound again
			src = sdat2Ptr + 8 + sdat1size;
			dst = sdat1Ptr + 8;
			len = sdat2size - sdat1size;

			memcpy(dst, src, len);
		}

		_sndPtrOffs = sdat2size - sdat1size;
		_sndTmrOffs += sdat2size;
	}

	// TODO: PX_SoftRemixAllChannels()
}
#endif

} // End of namespace Scumm
