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
#include "scumm/he/mixer_he.h"
#include "scumm/he/intern_he.h"
#include "scumm/util.h"

#include "common/config-manager.h"
#include "common/memstream.h"
#include "common/timer.h"
#include "common/util.h"
#include "common/translation.h"

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
	_mutex(mutex),
	_overrideFreq(0),
	_heSpoolingMusicTable(nullptr),
	_heSpoolingMusicCount(0) {

	_createSndId = 0;
	_createSndLastAppend = 0;
	_createSndLastPos = 0;
	_baseSndSize = 0;

	memset(_heChannel, 0, sizeof(_heChannel));
	memset(_soundCallbackScripts, 0, sizeof(_soundCallbackScripts));

	bool useMilesSoundSystem =
		parent->_game.id == GID_MOONBASE ||
		parent->_game.id == GID_BASEBALL2003 ||
		parent->_game.id == GID_BASKETBALL ||
		parent->_game.id == GID_FOOTBALL2002 ||
		parent->_game.id == GID_SOCCER2004 ||
		parent->_game.id == GID_PJGAMES;

	_heMixer = new HEMixer(_mixer, _vm, useMilesSoundSystem);
}

SoundHE::~SoundHE() {
	free(_heSpoolingMusicTable);

	if (_heSpoolingMusicFile.isOpen())
		_heSpoolingMusicFile.close();

	delete _heMixer;
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

	if (_vm->_game.heversion < 95 && heChannel == -1)
		heChannel = 1;

	while (i--) {
		if (_soundQueue[i].sound == sound && !(heFlags & ScummEngine_v70he::HESndFlags::HE_SND_APPEND))
			// Sound is already queued
			return;
	}

	Sound::addSoundToQueue(sound, heOffset, heChannel, heFlags, heFreq, hePan, heVol);
}

void SoundHE::modifySound(int sound, int offset, int frequencyShift, int pan, int volume, int flags) {
	int channel = hsFindSoundChannel(sound);
	if (channel >= 0 && _heChannel[channel].sound) {
		// The implementation for this is only available for the Miles mixer
		if (_heMixer->isMilesActive()) {
			_heMixer->milesModifySound(channel, offset, HESoundModifiers(frequencyShift, pan, volume), flags);
		}
	}
}

void SoundHE::processSoundQueues() {
	int snd, heOffset, heChannel, heFlags, heFreq, hePan, heVol;

	for (int i = 0; i <_soundQueuePos; i++) {
		snd = _soundQueue[i].sound;
		heOffset = _soundQueue[i].offset;
		heChannel = _soundQueue[i].channel;
		heFlags = _soundQueue[i].flags;
		heFreq = _soundQueue[i].freq;
		hePan = _soundQueue[i].pan;
		heVol = _soundQueue[i].vol;

		HESoundModifiers modifiers =
			_vm->_game.heversion < 99 ? HESoundModifiers() : HESoundModifiers(heFreq, hePan, heVol);

		if (snd) {
			if (((_vm->_game.heversion >= 80) && (_vm->_game.heversion < 95)) &&
				(_soundQueue[i].flags & ScummEngine_v70he::HESndFlags::HE_SND_VOL)) {
				setSoundVolume(snd, _soundQueue[i].vol);
			} else {
				triggerSound(snd, heOffset, heChannel, heFlags, modifiers);
			}
		}
	}

	_soundQueuePos = 0;

	Sound::processSoundQueues();
}

int SoundHE::isSoundRunning(int sound) const {
	// If our sound is a channel number, search for it
	// between the currently playing sounds first, then
	// search the sound queue...

	if (_vm->_game.heversion >= 70 || (_vm->_game.heversion < 70 && sound > 0)) {
		if (sound >= HSND_CHANNEL_0) {
			int channel = sound - HSND_CHANNEL_0;
			sound = _heChannel[channel].sound;

			if (sound)
				return sound;

			for (int i = 0; i < _soundQueuePos; i++) {
				if (_soundQueue[i].channel == channel) {
					return _soundQueue[i].sound;
				}
			}

			return 0;
		}

		// ...otherwise the sound parameter is a proper
		//  sound number, so search the queue
		int i = _soundQueuePos;
		while (i) {
			if (sound == _soundQueue[--i].sound)
				return sound;
		}
	}

	// If it's not in the queue, go check if it is actually playing
	if (_vm->_game.heversion >= 70 || sound == HSND_TALKIE_SLOT) {
		if (sound >= HSND_CHANNEL_0) {
			sound = _heChannel[sound - HSND_CHANNEL_0].sound;
		}

		if (hsFindSoundChannel(sound) != -1) {
			return sound;
		}

		return 0;
	} else {
		if (sound == -2) {
			sound = _heChannel[0].sound;
			if (hsFindSoundChannel(sound) != -1) {
				return sound;
			}
		} else if (sound == -1) {
			sound = _currentMusic;

			if (_vm->_musicEngine && _vm->_musicEngine->getSoundStatus(sound)) {
				return sound;
			}

			if (is3DOSound(sound) && hsFindSoundChannel(sound) != -1) {
				return sound;
			}
		} else if (sound > 0) {
			if (hsFindSoundChannel(sound) != -1) {
				return sound;
			}

			if (_vm->_musicEngine && _vm->_musicEngine->getSoundStatus(sound)) {
				return sound;
			}
		}

		return 0;
	}
}

bool SoundHE::isSoundInUse(int sound) const {

	return isSoundRunning(sound) != 0;
}

void SoundHE::stopSound(int sound) {
	int channel = -1;

	if (_vm->_game.heversion == 60) {
		if (sound == -2) {
			sound = _heChannel[0].sound;
		} else if (sound == -1) {
			sound = _currentMusic;
			Sound::stopSound(sound);

			for (int i = 0; i < ARRAYSIZE(_heChannel); i++) {
				if (_heChannel[i].sound == sound) {
					_heChannel[i].clearChannel();
				}
			}
			return;
		}
	}

	if (sound >= HSND_CHANNEL_0 && sound <= HSND_CHANNEL_7) {
		channel = sound - HSND_CHANNEL_0;

		if (_heChannel[channel].sound) {
			sound = _heChannel[channel].sound;
			stopDigitalSound(sound);
		}

		for (int i = 0; i < ARRAYSIZE(_soundQueue); i++) {
			if (_soundQueue[i].channel == channel)
				_soundQueue[i].sound = 0;
		}
	} else {
		if (_vm->_game.heversion >= 95 && sound == HSND_DYN_SOUND_CHAN) {
			for (int i = 0; i < ARRAYSIZE(_soundQueue); i++) {
				if (_soundQueue[i].channel == HSND_DYN_SOUND_CHAN) {
					_soundQueue[i].sound = 0;
				}
			}
		} else {
			if (hsFindSoundChannel(sound) != -1) {
				stopDigitalSound(sound);
			}

			for (int i = 0; i < ARRAYSIZE(_soundQueue); i++) {
				if (_soundQueue[i].sound == sound)
					_soundQueue[i].sound = 0;
			}
		}
	}

	if ((sound == HSND_TALKIE_SLOT) || (_vm->VAR_TALK_CHANNEL != 0xFF && channel == _vm->VAR(_vm->VAR_TALK_CHANNEL))) {
		_vm->_talkDelay = 0;
	}
}

void SoundHE::stopAllSounds() {
	if (_vm->_game.heversion >= 95)
		stopSound(HSND_DYN_SOUND_CHAN);

	for (int i = HSND_CHANNEL_0; i <= HSND_CHANNEL_7; i++) {
		stopSound(i);
	}

	// Empty the sound queue
	_soundQueuePos = 0;
	memset(_soundQueue, 0, sizeof(_soundQueue));
}

int SoundHE::hsFindSoundChannel(int sound) const {
	if (sound >= HSND_CHANNEL_0) {
		int channel = sound - HSND_CHANNEL_0;

		if (channel < 0 || channel > HSND_MAX_CHANNELS - 1) {
			error("SoundHE::hsFindSoundChannel(): ERROR: Channel %d out of range (%d-%d)",
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

void SoundHE::pauseSounds(bool pause) {
	// For MIDI audio
	if (_vm->_imuse)
		_vm->_imuse->pause(pause);

	_heMixer->pauseMixerSubSystem(pause);
}

void SoundHE::stopDigitalSound(int sound) {
	if (sound == HSND_TALKIE_SLOT) {
		_vm->_haveMsg = 3;
		_vm->_talkDelay = 0;
	}

	hsStopDigitalSound(sound);
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
	int chan;

	if ((chan = hsFindSoundChannel(sound)) != -1) {
		if (_heChannel[chan].hasSoundTokens) {
			return true;
		}
	}

	return false;
}

int SoundHE::getChannelPosition(int channel) {
	int soundPos;

	int frequency = _vm->_game.heversion >= 95 ? _heChannel[channel].frequency : HSND_DEFAULT_FREQUENCY;
	soundPos = (int)(((uint64)_vm->getHETimer(HSND_TIMER_SLOT + channel) * (uint64)frequency) / 1000);

	return soundPos;
}

int SoundHE::getSoundPosition(int sound) {
	int channel = hsFindSoundChannel(sound);

	if (channel != -1) {
		return getChannelPosition(channel);
	} else {
		return (_vm->_game.heversion > 72) ? 0 : channel;
	}
}

int SoundHE::getSoundVar(int sound, int var) {
	if (_vm->_game.heversion >= 90 && var == HSND_SNDVAR_TOKENS) {
		return isSoundCodeUsed(sound);
	}

	assertRange(0, var, HSND_MAX_SOUND_VARS - 1, "sound variable");

	int chan = hsFindSoundChannel(sound);

	if (chan != -1) {
		debug(5, "SoundHE::getSoundVar(): sound %d var %d result %d", sound, var, _heChannel[chan].soundVars[var]);
		return _heChannel[chan].soundVars[var];
	} else {
		return 0;
	}
}

void SoundHE::setSoundVar(int sound, int var, int val) {
	assertRange(0, var, HSND_MAX_SOUND_VARS - 1, "sound variable");

	int chan = -1;
	for (int i = 0; i < ARRAYSIZE(_heChannel); i ++) {
		if (_heChannel[i].sound == sound)
			chan = i;
	}

	if (chan != -1) {
		debug(5, "SoundHE::setSoundVar(): sound %d var %d val %d", sound, var, val);
		_heChannel[chan].soundVars[var] = val;
	}
}

void SoundHE::setSoundVolume(int sound, int volume) {
	int channel;

	if (sound < HSND_CHANNEL_0) {
		if ((channel = hsFindSoundChannel(sound)) == -1) {
			return;
		}
	} else {
		channel = sound - HSND_CHANNEL_0;
	}

	_heMixer->changeChannelVolume(channel, volume, true);
}

void SoundHE::setOverrideFreq(int freq) {
	_overrideFreq = freq;
}

void SoundHE::setupHEMusicFile() {
	uint32 id, len;
	Common::Path musicFilename(_vm->generateFilename(-4));

	// For engine restarts
	if (_heSpoolingMusicFile.isOpen())
		_heSpoolingMusicFile.close();

	if (_heSpoolingMusicFile.open(musicFilename)) {

		id = _heSpoolingMusicFile.readUint32BE();
		len = _heSpoolingMusicFile.readUint32BE();
		if (id == MKTAG('S', 'O', 'N', 'G')) {

			// Older versions had a much simpler file structure
			if (_vm->_game.heversion < 80) {
				// Skip header wrapping file
				_heSpoolingMusicFile.seek(16, SEEK_SET);
				_heSpoolingMusicCount = _heSpoolingMusicFile.readUint32LE();
			} else {
				// HE 80 and above
				id = _heSpoolingMusicFile.readUint32BE();
				len = _heSpoolingMusicFile.readUint32BE();
				if (id == MKTAG('S', 'G', 'H', 'D')) {
					_heSpoolingMusicCount = _heSpoolingMusicFile.readUint32LE();
					_heSpoolingMusicFile.seek(len - 8 - 4, SEEK_CUR);
				} else {
					_heSpoolingMusicFile.close();
					debug(5, "setupHEMusicFile(): Invalid spooling file '%s', couldn't find SGHD tag, found %s", musicFilename.toString().c_str(), tag2str(id));
					return;
				}
			}

			debug(5, "setupHEMusicFile(): music files count = %d", _heSpoolingMusicCount);
			_heSpoolingMusicTable = (HESpoolingMusicItem *)malloc(_heSpoolingMusicCount * sizeof(HESpoolingMusicItem));

			if (_heSpoolingMusicTable != nullptr) {
				for (int i = 0; i < _heSpoolingMusicCount; i++) {

					// For later versions we check that we are actually reading a SGEN section...
					if (_vm->_game.heversion >= 80) {
						id = _heSpoolingMusicFile.readUint32BE();
						len = _heSpoolingMusicFile.readUint32BE();
						if (id != MKTAG('S', 'G', 'E', 'N')) {
							_heSpoolingMusicFile.close();
							debug(5, "setupHEMusicFile(): Invalid spooling file '%s', couldn't find SGEN tag, found %s", musicFilename.toString().c_str(), tag2str(id));
							return;
						}
					}

					_heSpoolingMusicTable[i].song = _heSpoolingMusicFile.readSint32LE();
					_heSpoolingMusicTable[i].offset = _heSpoolingMusicFile.readSint32LE();
					_heSpoolingMusicTable[i].size = _heSpoolingMusicFile.readSint32LE();

					int amountToRead = _vm->_game.heversion >= 80 ? 9 : 13;
					int readAmount = 0;
					for (readAmount = 0; readAmount < amountToRead; readAmount++) {
						_heSpoolingMusicTable[i].filename[readAmount] = _heSpoolingMusicFile.readByte();

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

				_heMixer->setSpoolingSongsTable(_heSpoolingMusicTable, _heSpoolingMusicCount);
			} else {
				debug(5, "setupHEMusicFile(): Can't allocate table for spooling music file '%s'", musicFilename.toString().c_str());
			}
		} else {
			debug(5, "setupHEMusicFile(): Invalid file '%s', couldn't find SONG tag, found %s", musicFilename.toString().c_str(), tag2str(id));
		}
	} else {
		debug(5, "setupHEMusicFile(): Can't open spooling music file '%s'", musicFilename.toString().c_str());
	}
}

bool SoundHE::getHEMusicDetails(int id, int &musicOffs, int &musicSize) {
	for (int i = 0; i < _heSpoolingMusicCount; i++) {
		if (_heSpoolingMusicTable[i].song == id) {
			musicOffs = _heSpoolingMusicTable[i].offset;
			musicSize = _heSpoolingMusicTable[i].size;
			return true;
		}
	}

	return false;
}

void SoundHE::handleSoundFrame() {
	if (_vm->_game.heversion < 80)
		return;

	_soundsDebugFrameCounter++;

	if (_stopActorTalkingFlag) {
		_vm->stopTalk();
		_vm->_haveMsg = 3;
		_stopActorTalkingFlag = false;
	}

	if (_vm->_game.heversion >= 95)
		unqueueSoundCallbackScripts();

	runSoundCode();
	checkSoundTimeouts();
}

void SoundHE::feedMixer() {
	_heMixer->feedMixer();
}

void SoundHE::unqueueSoundCallbackScripts() {
	if (_inUnqueueCallbackScripts)
		return;

	_inUnqueueCallbackScripts++;

	for (int i = 0; i < _soundCallbacksQueueSize; i++) {

		if (_soundCallbackScripts[i].sound) {
			int args[NUM_SCRIPT_LOCAL];

			memset(args, 0, sizeof(args));
			args[0] = _soundCallbackScripts[i].sound;
			args[1] = _soundCallbackScripts[i].channel;
			args[2] = 0;
			args[3] = 0;

			debug(5, "SoundHE::unqueueSoundCallbackScripts(): dequeued callback for sound %d in channel %d",
				_soundCallbackScripts[i].sound,
				_soundCallbackScripts[i].channel);

			_vm->runScript(_vm->VAR(_vm->VAR_SOUND_CALLBACK_SCRIPT), 0, 0, args);
		}

		_soundCallbackScripts[i].sound = 0;
		_soundCallbackScripts[i].channel = 0;
		_soundCallbackScripts[i].whatFrame = 0;
	}

	_soundCallbacksQueueSize = 0;
	_inUnqueueCallbackScripts--;
}

void SoundHE::checkSoundTimeouts() {
	for (int chan = 0; chan < ARRAYSIZE(_heChannel); chan++) {
		if (_heChannel[chan].sound == 0 || _heChannel[chan].timeout == 0)
			continue;

		if (_vm->getHETimer(chan + HSND_TIMER_SLOT) > _heChannel[chan].timeout) {
			digitalSoundCallback(HSND_SOUND_TIMEOUT, chan);
		}
	}
}

void SoundHE::digitalSoundCallback(int message, int channel, bool earlyCallback) {
	// The action done for each sound is always the same;
	// it's useful to keep track of the message for debugging
	// purposes though...
	switch (message) {
	case HSND_SOUND_TIMEOUT:
		debug(5, "SoundHE::digitalSoundCallback(): TIMEOUT, channel %d", channel);
		break;
	case HSND_SOUND_ENDED:
		if (earlyCallback)
			debug(5, "SoundHE::digitalSoundCallback(): ENDED with EARLY CALLBACK, channel %d", channel);
		else
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
	int sound = _heChannel[channel].sound;

	switch (message) {
	case HSND_SOUND_TIMEOUT:
	case HSND_SOUND_ENDED:
	case HSND_SOUND_STOPPED:

		if (sound == HSND_TALKIE_SLOT) {
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

		if (_vm->_game.heversion >= 80)
			queueSoundCallbackScript(sound, channel, message);

		break;
	}

	_inSoundCallbackFlag = false;
}

void SoundHE::queueSoundCallbackScript(int sound, int channel, int message) {
	// Avoid queueing up a sound callback if the mixer is not available.
	if (!_mixer->isReady())
		return;

	debug(5, "SoundHE::queueSoundCallbackScript(): callback for channel %d, sound %d, attempting queueing...", channel, sound);
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

		if (_heChannel[chan].codeOffset == -1) {
			continue;
		}

		soundPos = getChannelPosition(chan) + _vm->VAR(_vm->VAR_SOUND_TOKEN_OFFSET);
		soundPos = MAX<int>(0, soundPos);

		if (_heChannel[chan].codeBuffer == nullptr) {
			soundPtr = _vm->getResourceAddress(rtSound, _heChannel[chan].sound);
		} else {
			soundPtr = _heChannel[chan].codeBuffer;
		}

		assert(soundPtr);

		soundPtr += _heChannel[chan].codeOffset;

		len = READ_LE_UINT16(soundPtr);
		freq = READ_LE_UINT32(soundPtr + sizeof(uint16));

		while (soundPos > freq) {
			debug(5, "SoundHE::runSoundCode(): Channel %d Timer %d Time %d", chan, soundPos, freq);

			processSoundOpcodes(_heChannel[chan].sound, soundPtr + sizeof(uint16) + sizeof(uint32), _heChannel[chan].soundVars);

			_heChannel[chan].codeOffset += len;

			// The original runs the following section again on purpose
			if (_heChannel[chan].codeBuffer == nullptr) {
				soundPtr = _vm->getResourceAddress(rtSound, _heChannel[chan].sound);
			} else {
				soundPtr = _heChannel[chan].codeBuffer;
			}

			assert(soundPtr);

			soundPtr += _heChannel[chan].codeOffset;

			len = READ_LE_UINT16(soundPtr);
			freq = READ_LE_UINT32(soundPtr + sizeof(uint16));

			if (len == 0) {
				_heChannel[chan].codeOffset = -1;
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

		debug(5, "SoundHE::processSoundOpcodes(): sound %d opcode %d", sound, opcode);

		switch (opcode) {
		case HSND_SBNG_END: // Continue - 0
			break;
		case HSND_SBNG_FACE: // Set talk state - 16
			val = READ_LE_UINT16(codePtr); codePtr += 2;
			setSoundVar(sound, 19, val);
			break;
		case HSND_SBNG_SET_SET: // Set var - 32
			var = READ_LE_UINT16(codePtr); codePtr += 2;
			val = READ_LE_UINT16(codePtr); codePtr += 2;
			if (arg == 2) {
				val = getSoundVar(sound, val);
			}
			setSoundVar(sound, var, val);
			break;
		case HSND_SBNG_SET_ADD: // Add - 48
			var = READ_LE_UINT16(codePtr); codePtr += 2;
			val = READ_LE_UINT16(codePtr); codePtr += 2;
			if (arg == 2) {
				val = getSoundVar(sound, val);
			}
			val = getSoundVar(sound, var) + val;
			setSoundVar(sound, var, val);
			break;
		case HSND_SBNG_SET_SUB: // Subtract - 56
			var = READ_LE_UINT16(codePtr); codePtr += 2;
			val = READ_LE_UINT16(codePtr); codePtr += 2;
			if (arg == 2) {
				val = getSoundVar(sound, val);
			}
			val = getSoundVar(sound, var) - val;
			setSoundVar(sound, var, val);
			break;
		case HSND_SBNG_SET_MUL: // Multiple - 64
			var = READ_LE_UINT16(codePtr); codePtr += 2;
			val = READ_LE_UINT16(codePtr); codePtr += 2;
			if (arg == 2) {
				val = getSoundVar(sound, val);
			}
			val = getSoundVar(sound, var) * val;
			setSoundVar(sound, var, val);
			break;
		case HSND_SBNG_SET_DIV: // Divide - 80
			var = READ_LE_UINT16(codePtr); codePtr += 2;
			val = READ_LE_UINT16(codePtr); codePtr += 2;
			if (arg == 2) {
				val = getSoundVar(sound, val);
			}
			if (!val) {
				val = 1; // Safeguard for division by zero
				warning("SoundHE::processSoundOpcodes(): Incorrect value 0 for processSoundOpcodes() kludge DIV");
			}
			val = getSoundVar(sound, var) / val;
			setSoundVar(sound, var, val);
			break;
		case HSND_SBNG_SET_INC: // Increment - 96
			var = READ_LE_UINT16(codePtr); codePtr += 2;
			val = getSoundVar(sound, var) + 1;
			setSoundVar(sound, var, val);
			break;
		case HSND_SBNG_SET_DEC: // Decrement - 104
			var = READ_LE_UINT16(codePtr); codePtr += 2;
			val = getSoundVar(sound, var) - 1;
			setSoundVar(sound, var, val);
			break;
		default:
			error("SoundHE::processSoundOpcodes(): Illegal sound %d opcode %d", sound, opcode);
		}
	}
}

void SoundHE::triggerSound(int soundId, int heOffset, int heChannel, int heFlags, HESoundModifiers modifiers) {
	_dynamicSoundAgeCounter++;

	if (_vm->_game.heversion >= 95) {
		if (heChannel == HSND_DYN_SOUND_CHAN) {
			heChannel = getNextDynamicChannel();
		}
	}

	if (soundId >= _vm->_numSounds) {
		triggerSpoolingSound(soundId, heOffset, heChannel, heFlags, modifiers);
		return;
	}

	byte *soundAddr = (byte *)_vm->getResourceAddress(rtSound, soundId);

	if ((READ_BE_UINT32(soundAddr) == MKTAG('D', 'I', 'G', 'I')) ||
		(READ_BE_UINT32(soundAddr) == MKTAG('T', 'A', 'L', 'K')) ||
		(READ_BE_UINT32(soundAddr) == MKTAG('M', 'R', 'A', 'W'))) {
		triggerDigitalSound(soundId, heOffset, heChannel, heFlags);
	} else if (READ_BE_UINT32(soundAddr) == MKTAG('M', 'I', 'D', 'I')) {
		triggerMidiSound(soundId, heOffset);
	} else if (READ_BE_UINT32(soundAddr) == MKTAG('W', 'S', 'O', 'U')) {
		triggerRIFFSound(soundId, heOffset, heChannel, heFlags, modifiers);
	} else if (READ_BE_UINT32(soundAddr) == MKTAG('X', 'S', 'O', 'U')) {
#ifdef ENABLE_HE
		triggerXSOUSound(soundId, heOffset, heChannel, heFlags);
#endif
	} else {
		error("SoundHE::triggerSound(): Illegal sound %d type %s", soundId, tag2str(READ_BE_UINT32(soundAddr)));
	}
}

void SoundHE::triggerSpoolingSound(int song, int offset, int channel, int flags, HESoundModifiers modifiers) {
	if (_heSpoolingMusicCount != 0) {
		for (int i = 0; i < _heSpoolingMusicCount; i++) {
			if (_heSpoolingMusicTable[i].song == song) {
				debug(5, "SoundHE::triggerSpoolingSound(): Starting spooling sound %d with offset %d, on channel %d with flags %d",
					song, offset, channel, flags);

				Common::Path filename(_vm->generateFilename(-4));
				int fileOffset = 0;
				int songsize = 0;
				uint32 id, len;

				hsStopDigitalSound(_heChannel[channel].sound);

				if (_heSpoolingMusicTable[i].offset != 0) {
					fileOffset = _heSpoolingMusicTable[i].offset;

					if (_vm->_game.heversion < 80)
						fileOffset += HSND_RES_OFFSET_SOUND_DATA;

					_heSpoolingMusicFile.seek(fileOffset, SEEK_SET);
					songsize = _heSpoolingMusicTable[i].size;

					if (_vm->_game.heversion < 80)
						songsize -= HSND_RES_OFFSET_SOUND_DATA;
				} else {
					_heSpoolingMusicFile.close();

					filename = _heSpoolingMusicTable[i].filename;

					if (!_heSpoolingMusicFile.open(filename)) {
						debug("SoundHE::triggerSpoolingSound(): Can't open music file '%s'", filename.toString().c_str());
						if (_vm->_game.heversion < 95) {
							_vm->VAR(_vm->VAR_ERROR_FLAG) = -1;
						} else {
							_vm->VAR(_vm->VAR_OPERATION_FAILURE) = -1;
						}

						return;
					} else if (_vm->_game.heversion < 80) {
						_heSpoolingMusicFile.seek(HSND_RES_OFFSET_SOUND_DATA, SEEK_SET);
						songsize = _heSpoolingMusicFile.size() - HSND_RES_OFFSET_SOUND_DATA;
					}
				}

				// Old spooled music format (raw audio)
				if (_vm->_game.heversion < 80) {
					_heMixer->startSpoolingChannel(
						channel, song, _heSpoolingMusicFile, songsize,
						HSND_DEFAULT_FREQUENCY, HSND_MAX_VOLUME, channel,
						CHANNEL_ACTIVE);

					_heChannel[channel].sound = song;
					_heChannel[channel].priority = 255;

					_vm->setHETimer(channel + HSND_TIMER_SLOT);

					_vm->VAR(_vm->VAR_ERROR_FLAG) = 0;
					return;
				}

				_heChannel[channel].codeOffset = -1;
				_heChannel[channel].codeBuffer = nullptr;
				_heChannel[channel].hasSoundTokens = false;

				id = _heSpoolingMusicFile.readUint32BE();

				if (id == MKTAG('R', 'I', 'F', 'F')) {
					len = _heSpoolingMusicFile.readUint32LE();
					id = _heSpoolingMusicFile.readUint32BE();

					if (id != MKTAG('W', 'A', 'V', 'E'))
						error("SoundHE::triggerSpoolingSound(): Illegal music .wav file %d", song);

					while (true) {
						id = _heSpoolingMusicFile.readUint32BE();
						len = _heSpoolingMusicFile.readUint32LE();

						if (id == MKTAG('d', 'a', 't', 'a')) {
							id = MKTAG('S', 'D', 'A', 'T'); // Convert it to a SDAT tag
							len += 8; // Compensate the length of the header

							break;
						} else if (id == MKTAG('S', 'B', 'N', 'G')) {
							if (len > ARRAYSIZE(_heSpoolingCodeBuffer)) {
								error("SoundHE::triggerSpoolingSound(): Spooling sound %d code too large", song);
							}

							_heSpoolingMusicFile.read(_heSpoolingCodeBuffer, len);

							_heChannel[channel].codeOffset = 0;
							_heChannel[channel].codeBuffer = _heSpoolingCodeBuffer;
							_heChannel[channel].hasSoundTokens = true;
						} else if (id == MKTAG('f', 'm', 't', ' ')) {
							PCMWaveFormat pcm;

							if (len < sizeof(pcm))
								error("SoundHE::triggerSpoolingSound(): Illegal .wav format length in song %d", song);

							pcm.wFormatTag = _heSpoolingMusicFile.readUint16LE();
							pcm.wChannels = _heSpoolingMusicFile.readUint16LE();
							pcm.dwSamplesPerSec = _heSpoolingMusicFile.readUint32LE();
							pcm.dwAvgBytesPerSec = _heSpoolingMusicFile.readUint32LE();
							pcm.wBlockAlign = _heSpoolingMusicFile.readUint16LE();
							pcm.wBitsPerSample = _heSpoolingMusicFile.readUint16LE();

							if (_heMixer->isMilesActive()) {
								if (pcm.wFormatTag != WAVE_FORMAT_PCM && pcm.wFormatTag != WAVE_FORMAT_IMA_ADPCM) {
									error("SoundHE::triggerSpoolingSound(): Illegal .wav format for Miles mixer, song %d - %d",
										  song, pcm.wFormatTag);
								}
							} else {
								if (pcm.wFormatTag != WAVE_FORMAT_PCM ||
									pcm.wChannels != 1 ||
									pcm.dwSamplesPerSec != HSND_DEFAULT_FREQUENCY ||
									pcm.wBitsPerSample != 8) {
									error("SoundHE::triggerSpoolingSound(): Illegal .wav format for software mixer, song %d - %d, %d, %d, %d",
											   song, pcm.wFormatTag, pcm.wChannels, pcm.dwSamplesPerSec, pcm.wBitsPerSample);
								}
							}

							// Skip over the rest of this block.
							_heSpoolingMusicFile.seek(len - sizeof(pcm), SEEK_CUR);
						} else {
							_heSpoolingMusicFile.seek(len, SEEK_CUR);
						}
					}
				} else if (id == MKTAG('D', 'I', 'G', 'I')) {
					len = _heSpoolingMusicFile.readUint32BE();
					id = _heSpoolingMusicFile.readUint32BE();
					len = _heSpoolingMusicFile.readUint32BE();

					if (id == MKTAG('H', 'S', 'H', 'D')) {
						_heSpoolingMusicFile.seek(len - 8, SEEK_CUR);
					} else {
						error("SoundHE::triggerSpoolingSound(): Illegal spooling sound %d, id %s", song, tag2str(id));
					}

					id = _heSpoolingMusicFile.readUint32BE();
					len = _heSpoolingMusicFile.readUint32BE();

					if (id == MKTAG('S', 'B', 'N', 'G')) {
						if (len > sizeof(_heSpoolingCodeBuffer)) {
							error("SoundHE::triggerSpoolingSound(): Spooling sound %d code too large", song);
						}

						_heSpoolingMusicFile.read(_heSpoolingCodeBuffer, len - 8);
						_heChannel[channel].codeOffset = 0;
						_heChannel[channel].codeBuffer = _heSpoolingCodeBuffer;
						_heChannel[channel].hasSoundTokens = true;

						id = _heSpoolingMusicFile.readUint32BE();
						len = _heSpoolingMusicFile.readUint32BE();
					}
				} else {
					error("SoundHE::triggerSpoolingSound(): Illegal spooling sound %d, id %s", song, tag2str(id));
				}

				if (id == MKTAG('S', 'D', 'A', 'T')) {
					songsize = len - 8;
				} else {
					error("SoundHE::triggerSpoolingSound(): Illegal spooling sound %d, id %s", song, tag2str(id));
				}

				if (_heMixer->isMilesActive()) {
					if (offset)
						debug("SoundHE::triggerSpoolingSound(): Starting offsets into music files not supported with Miles currently");

					_heMixer->milesStartSpoolingChannel(channel, filename.toString('/').c_str(), fileOffset, flags, modifiers);
				} else {
					// Start the music track at a specified offset
					if (offset) {
						_heSpoolingMusicFile.seek(offset, SEEK_CUR);

						if (offset >= songsize || offset < 0) {
							error("SoundHE::triggerSpoolingSound(): Invalid offset %d for sound %d (song size %d)", offset, song, songsize);
						}

						songsize -= offset;
					}

					_heMixer->startSpoolingChannel(channel, song, _heSpoolingMusicFile, songsize,
											HSND_DEFAULT_FREQUENCY, HSND_MAX_VOLUME, channel, CHANNEL_ACTIVE);
				}

				_vm->setHETimer(channel + HSND_TIMER_SLOT);

				_heChannel[channel].age = _dynamicSoundAgeCounter;
				_heChannel[channel].sound = song;
				_heChannel[channel].priority = 255;
				_heChannel[channel].frequency = HSND_DEFAULT_FREQUENCY;

				if (_vm->_game.heversion < 95) {
					_vm->VAR(_vm->VAR_ERROR_FLAG) = 0;
				} else {
					_vm->VAR(_vm->VAR_OPERATION_FAILURE) = 0;
				}

				return;
			}
		}
	}

	// Error situation
	if (_vm->_game.heversion < 95) {
		_vm->VAR(_vm->VAR_ERROR_FLAG) = -1;
	} else {
		_vm->VAR(_vm->VAR_OPERATION_FAILURE) = -1;
	}
}

void SoundHE::triggerRIFFSound(int soundId, int heOffset, int heChannel, int heFlags, HESoundModifiers modifiers) {
	PCMWaveFormat pFmt;
	uint8 *soundDataPtr = nullptr;
	int sampleCount = 0;
	int soundPriority = 128;
	int soundCodeOffset = -1;
	bool parsedFmt = false;

	// For uninit var warning...
	pFmt.wFormatTag = 0;
	pFmt.wChannels = 0;
	pFmt.dwSamplesPerSec = 0;
	pFmt.dwAvgBytesPerSec = 0;
	pFmt.wBlockAlign = 0;
	pFmt.wBitsPerSample = 0;


	// Let's begin by fetching the sound address...
	uint8 *wsouPtr = (byte *)_vm->getResourceAddress(rtSound, soundId);

	// We only accept the WSOU format compliant files,
	// which are WAV files with a WSOU header wrapped around.
	// Still, let's not use an assertion like the original does,
	// and let's bail out gracefully instead....
	if (READ_BE_UINT32(wsouPtr) != MKTAG('W', 'S', 'O', 'U')) {
		debug("SoundHE::triggerRIFFSound(): Couldn't find WSOU tag for sound %d, bailing out...", soundId);
		return;
	}

	// Skip over the WSOU header and hope to find a RIFF header...
	uint8 *soundPtr = wsouPtr + 8;
	if (READ_BE_UINT32(soundPtr) != MKTAG('R', 'I', 'F', 'F')) {
		debug("SoundHE::triggerRIFFSound(): Couldn't find RIFF tag for sound %d, bailing out...", soundId);
		return;
	}

	// Since all sub-blocks must be padded to be even, we want the RIFF block length to be even...
	int riffLength = READ_LE_UINT32(soundPtr + 4);
	if ((riffLength & 1) != 0) {
		debug("SoundHE::triggerRIFFSound(): RIFF block length not even (%d) for sound %d, bailing out...", riffLength, soundId);
		return;
	}

	uint8 *wavePtr = soundPtr + 8;
	if (READ_BE_UINT32(wavePtr) != MKTAG('W', 'A', 'V', 'E')) {
		debug("SoundHE::triggerRIFFSound(): Couldn't find WAVE tag for sound %d, bailing out...", soundId);
		return;
	}

	// Now that we've made all the safety checks needed, fetch the sound data...
	wavePtr += 4;
	riffLength -= 4;

	// Parse the various blocks...
	while (riffLength > 0) {
		int chunkId = READ_BE_UINT32(wavePtr);
		int chunkLength = READ_LE_UINT32(wavePtr + 4);
		wavePtr += 8;
		riffLength -= 8;

		if (chunkLength < 0) {
			debug("SoundHE::triggerRIFFSound(): Illegal chunk length - %d bytes, bailing out...", chunkLength);
			return;
		}

		if (chunkLength > riffLength) {
			debug("SoundHE::triggerRIFFSound(): Chunk extends beyond file end - %d versus %d, bailing out...", chunkLength, riffLength);
			return;
		}

		switch (chunkId) {
		case MKTAG('f', 'm', 't', ' '):
		{
			uint8 *tempPtr = wavePtr;
			pFmt.wFormatTag = READ_LE_UINT16(tempPtr); tempPtr += 2;
			pFmt.wChannels = READ_LE_UINT16(tempPtr); tempPtr += 2;
			pFmt.dwSamplesPerSec = READ_LE_UINT32(tempPtr); tempPtr += 4;
			pFmt.dwAvgBytesPerSec = READ_LE_UINT32(tempPtr); tempPtr += 4;
			pFmt.wBlockAlign = READ_LE_UINT16(tempPtr); tempPtr += 2;
			pFmt.wBitsPerSample = READ_LE_UINT16(tempPtr); tempPtr += 2;

			if (pFmt.wFormatTag != WAVE_FORMAT_PCM && pFmt.wFormatTag != WAVE_FORMAT_IMA_ADPCM) {
				debug("SoundHE::triggerRIFFSound(): Unsupported .wav sound type %d - only PCM and IMA ADPCM are supported.", pFmt.wFormatTag);
				return;
			}

			parsedFmt = true;

			break;
		}
		case MKTAG('d', 'a', 't', 'a'):
			assert(parsedFmt);
			soundDataPtr = wavePtr;
			sampleCount = (chunkLength * 8) / (pFmt.wChannels * pFmt.wBitsPerSample);

			break;
		case MKTAG('X', 'S', 'H', '2'):
		{
			// Check for the optional sound flag block
			int optionalBlockFlags = READ_LE_UINT32(wavePtr);
			if (optionalBlockFlags & XSH2_FLAG_HAS_PRIORITY)
				soundPriority = READ_LE_UINT32(wavePtr + 4);

			break;
		}
		case MKTAG('S', 'B', 'N', 'G'):
			soundCodeOffset = wavePtr - wsouPtr;
			break;
		default:
			break;
		}

		// Round up to the next multiple of two
		chunkLength = (chunkLength + 1) & ~1;
		wavePtr += chunkLength;
		riffLength -= chunkLength;
	}

	if (riffLength != 0 || !parsedFmt || soundDataPtr == nullptr)
		return;

	int sampleFrequency = pFmt.dwSamplesPerSec;
	int bitsPerSample = pFmt.wBitsPerSample;
	int sampleChannelCount = pFmt.wChannels;

	int soundDataOffset = soundDataPtr - soundPtr;

	// Make sure that the sound has a high enough priority to play
	if (_heChannel[heChannel].sound && soundId != HSND_TALKIE_SLOT && _heChannel[heChannel].sound != HSND_TALKIE_SLOT) {
		if (soundPriority < _heChannel[heChannel].priority)
			return;
	}

	// Finally start the sound
	hsStartDigitalSound(soundId, heOffset, soundPtr, soundDataOffset, rtSound, soundId,
						sampleCount, sampleFrequency, heChannel, soundPriority, soundCodeOffset,
						heFlags, bitsPerSample, sampleChannelCount, modifiers);
}

#ifdef ENABLE_HE

void SoundHE::triggerXSOUSound(int heSound, int heOffset, int heChannel, int heFlags) {
	int sampleFrequency, bitsPerSample, sampleChannelCount, soundPriority;
	int soundCodeOffset, soundDataOffset, sampleCount;
	byte *soundCodeBlockPtr, *soundHeaderBlock, *soundDataPtr, *soundPtr;
	byte *optionalHeaderBlock;
	int32 optionalBlockFlags;

	soundPtr = (byte *)_vm->getResourceAddress(rtSound, heSound);

	// Fetch the sound data format...
	soundHeaderBlock = (byte *)((ScummEngine_v71he *)_vm)->findWrappedBlock(MKTAG('X', 'S', 'H', 'D'), soundPtr, 0, true);
	soundHeaderBlock += 8;

	sampleCount = READ_LE_UINT32(soundHeaderBlock + 0);
	sampleFrequency = READ_LE_UINT32(soundHeaderBlock + 4);
	bitsPerSample = READ_LE_UINT32(soundHeaderBlock + 8);
	sampleChannelCount = READ_LE_UINT32(soundHeaderBlock + 12);

	soundDataPtr = (byte *)((ScummEngine_v71he *)_vm)->findWrappedBlock(MKTAG('X', 'D', 'A', 'T'), soundPtr, 0, true);
	soundDataOffset = ((soundDataPtr - soundPtr) + 8);

	// Check for the optional sound flag block containing the priority...
	soundPriority = 128;

	optionalHeaderBlock = (byte *)((ScummEngine_v71he *)_vm)->findWrappedBlock(MKTAG('X', 'S', 'H', '2'), soundPtr, 0, false);

	if (optionalHeaderBlock) {
		optionalHeaderBlock += 8;
		optionalBlockFlags = READ_LE_UINT32(optionalHeaderBlock);
		optionalHeaderBlock += 4;

		if (optionalBlockFlags & XSH2_FLAG_HAS_PRIORITY) {
			soundPriority = READ_LE_UINT32(optionalHeaderBlock);
			optionalHeaderBlock += 4;
		}
	}

	// Check for the optional SBNG block...
	soundCodeBlockPtr = (byte *)((ScummEngine_v71he *)_vm)->findWrappedBlock(MKTAG('S', 'B', 'N', 'G'), soundPtr, 0, false);

	if (soundCodeBlockPtr == nullptr) {
		soundCodeOffset = -1;
	} else {
		soundCodeOffset = (soundCodeBlockPtr - soundPtr) + 8;
	}

	// Make sure that the sound has a high enough priority to play
	if (_heChannel[heChannel].sound && heSound != HSND_TALKIE_SLOT && _heChannel[heChannel].sound != HSND_TALKIE_SLOT) {
		if (soundPriority < _heChannel[heChannel].priority)
			return;
	}

	// Finally start the sound
	hsStartDigitalSound(
		heSound, heOffset, soundPtr, soundDataOffset, rtSound, heSound,
		sampleCount, sampleFrequency, heChannel, soundPriority, soundCodeOffset,
		heFlags, bitsPerSample, sampleChannelCount, HESoundModifiers());
}

#endif

void SoundHE::hsStartDigitalSound(int sound, int offset, byte *addr, int soundData,
	int globType, int globNum, int sampleCount, int frequency, int channel, int priority,
	int soundCode, int flags, int bitsPerSample, int soundChannelCount, HESoundModifiers modifiers) {
	int earlyCallbackByteCount = 0;
	int index;
	uint32 hflags;

	debug(5, "SoundHE::hsStartDigitalSound(): Starting sound %d with offset %d, on channel %d with flags %d", sound, offset, channel, flags);
	hflags = (flags & ScummEngine_v70he::HESndFlags::HE_SND_LOOP) ? CHANNEL_LOOPING : 0;
	hflags |= (flags & ScummEngine_v70he::HESndFlags::HE_SND_SOFT_SOUND) ? CHANNEL_SOFT_REMIX : 0;

	// If there's an early channel callback value, fetch it...
	if (_vm->_game.heversion >= 95) {
		index = _vm->VAR_EARLY_CHAN_1_CALLBACK + channel;

		if ((index >= _vm->VAR_EARLY_CHAN_1_CALLBACK) && (index <= _vm->VAR_EARLY_CHAN_3_CALLBACK)) {
			earlyCallbackByteCount = _vm->VAR(index);
		} else {
			earlyCallbackByteCount = 0;
		}
	}

	if (_vm->_game.heversion >= 95) {
		_heMixer->startChannelNew(
			channel, globType, globNum, soundData, offset,
			(sampleCount - offset), frequency, bitsPerSample, soundChannelCount,
			modifiers, channel, CHANNEL_ACTIVE | CHANNEL_CALLBACK_EARLY | hflags,
			earlyCallbackByteCount);
	} else if (_vm->_game.heversion >= 80) {
		// HE 80 doesn't make checks on the range for the early callback channel
		_heMixer->startChannel(
			channel, globType, globNum, soundData + offset,
			(sampleCount - offset), HSND_DEFAULT_FREQUENCY, HSND_MAX_VOLUME, channel,
			CHANNEL_ACTIVE | CHANNEL_CALLBACK_EARLY | hflags,
			_vm->VAR(_vm->VAR_EARLY_CHAN_1_CALLBACK + channel));
	} else {
		// Sub HE 80 codepath, simpler than the newer versions

		// If we're trying to start a speech sound, we have to signal an early callback
		if (_vm->_game.heversion >= 70 && channel == _vm->VAR(_vm->VAR_TALK_CHANNEL)) {
			earlyCallbackByteCount = ((_vm->VAR(_vm->VAR_TIMER_NEXT) * frequency) / 60);
			earlyCallbackByteCount *= _vm->VAR(_vm->VAR_EARLY_TALKIE_CALLBACK);

			_heMixer->startChannel(
				channel, globType, globNum, HSND_RES_OFFSET_SOUND_DATA + offset,
				sampleCount - offset, frequency, HSND_MAX_VOLUME, channel,
				CHANNEL_ACTIVE | CHANNEL_CALLBACK_EARLY | hflags,
				earlyCallbackByteCount);
		} else {
			_heMixer->startChannel(
				channel, globType, globNum, HSND_RES_OFFSET_SOUND_DATA + offset,
				sampleCount - offset, frequency, HSND_MAX_VOLUME, channel, CHANNEL_ACTIVE | hflags);
		}

		_heChannel[channel].sound = sound;
		_heChannel[channel].priority = priority;

		_vm->setHETimer(channel + HSND_TIMER_SLOT);

		return;
	}

	_vm->setHETimer(channel + HSND_TIMER_SLOT);

	_heChannel[channel].age = _dynamicSoundAgeCounter;
	_heChannel[channel].sound = sound;
	_heChannel[channel].priority = priority;
	_heChannel[channel].codeOffset = soundCode;
	_heChannel[channel].hasSoundTokens = (soundCode != -1);
	_heChannel[channel].frequency = frequency;

	if (flags & ScummEngine_v70he::HESndFlags::HE_SND_LOOP) {
		_heChannel[channel].timeout = 0;
	} else {
		if (_vm->_game.heversion >= 95) {
			uint64 timeOut;
			timeOut = (uint64)(sampleCount - offset) * (uint64)1000;
			_heChannel[channel].timeout = (int)(timeOut / frequency) + 2000;
		} else {
			uint32 timeOut;
			timeOut = (uint32)(sampleCount - offset) * (uint32)1000;
			_heChannel[channel].timeout = (int)(timeOut / frequency) + 2000;
		}
	}

	int overrideDuration;
	if (_heMixer->audioOverrideExists(globNum, true, &overrideDuration)) {
		_heChannel[channel].timeout = overrideDuration;
		_heChannel[channel].codeOffset = -1;
		_heChannel[channel].hasSoundTokens = false;
	}

	for (int i = 0; i < HSND_MAX_SOUND_VARS; i++) {
		_heChannel[channel].soundVars[i] = 0;
	}
}

void SoundHE::hsStopDigitalSound(int sound) {
	int channel;
	if ((channel = hsFindSoundChannel(sound)) != -1) {
		_heMixer->stopChannel(channel);
		_heChannel[channel].sound = 0;
		memset(&_heChannel[channel], 0, sizeof(_heChannel[channel]));
	}
}

void SoundHE::triggerDigitalSound(int sound, int offset, int channel, int flags) {
	byte *soundAddr;
	int soundCode, soundData, bitsPerSample, sampleChannels;
	const byte *soundResPtr;
	uint32 soundLength, soundFrequency;
	int soundPriority;

	debug(5, "SoundHE::triggerDigitalSound(sound=%d, offset=%d, channel=%d, flags=%08x)", sound, offset, channel, flags);

	soundAddr = (byte *)_vm->getResourceAddress(rtSound, sound);

	// Is this a MRAW music file from the 3DO games? Then update _currentMusic
	// and throw the sound on the last channel, since otherwise speech will interrupt it...
	if (READ_BE_UINT32(soundAddr) == MKTAG('M', 'R', 'A', 'W')) {
		_currentMusic = sound;
		channel = HSND_MAX_CHANNELS - 1;
	}

	// Don't let digital sounds interrupt speech...
	if (_heChannel[channel].sound == HSND_TALKIE_SLOT && sound != HSND_TALKIE_SLOT) {
		return;
	}

	soundPriority = soundAddr[HSND_RES_OFFSET_KILL_PRIO];

	if (_vm->_game.heversion < 95 && _overrideFreq) {
		soundFrequency = _overrideFreq;
		_overrideFreq = 0;
	} else {
		soundFrequency = READ_LE_UINT16(&soundAddr[HSND_RES_OFFSET_FREQUENCY]);
	}

	bitsPerSample = 8;
	sampleChannels = 1;

	// Check sound priority
	if (_heChannel[channel].sound && (_heChannel[channel].sound != HSND_TALKIE_SLOT) && (sound != HSND_TALKIE_SLOT)) {
		if (soundPriority < _heChannel[channel].priority) {
			return; // Don't start new sound, prio too low
		}
	}

	soundResPtr = _vm->findResource(MKTAG('S', 'B', 'N', 'G'), soundAddr);
	if (soundResPtr == nullptr) {
		soundCode = -1;
	} else {
		soundCode = soundResPtr - soundAddr + 8;
	}

	soundResPtr = _vm->findResource(MKTAG('S', 'D', 'A', 'T'), soundAddr);
	if (soundResPtr == nullptr)
		error("SoundHE::triggerDigitalSound(): Can't find SDAT section in sound %d", sound);

	soundData = soundResPtr - soundAddr + 8;
	soundLength = READ_BE_UINT32(soundResPtr + 4) - 8;

	// Check for a valid offset...
	if ((uint32)offset >= soundLength) {
		debug(5, "SoundHE::triggerDigitalSound(): WARNING: Sound %d started past end offset %d size %d", sound, offset, soundLength);
		offset = 0;
	}

	hsStartDigitalSound(
		sound, offset, soundAddr, soundData, rtSound, sound,
		soundLength, soundFrequency, channel, soundPriority, soundCode, flags,
		bitsPerSample, sampleChannels, HESoundModifiers());
}

void SoundHE::triggerMidiSound(int soundId, int heOffset) {
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

void SoundHE::playVoice(uint32 offset, uint32 length) {
	byte *ptr;
	int talkieChannel = (_vm->VAR_TALK_CHANNEL != 0xFF) ? _vm->VAR(_vm->VAR_TALK_CHANNEL) : 0;

	if (offset == 0 || length == 0) {
		debug(5, "SoundHE::playVoice(): Attempted playing 0 offset/length talkie from scripts, bailing out...");
		return;
	}

	if (ConfMan.getBool("speech_mute"))
		return;

	if (_sfxFilename.empty()) {
		// This happens in the Pajama Sam's Lost & Found demo, on the
		// main menu screen, so don't make it a fatal error.
		warning("SoundHE::playVoice(): Speech file is not found");
		return;
	}

	ScummFile file(_vm);
	if (!_vm->openFile(file, Common::Path(_sfxFilename))) {
		warning("SoundHE::playVoice(): Could not open speech file %s", _sfxFilename.c_str());
		return;
	}

	file.setEnc(_sfxFileEncByte);

	// Speech audio doesn't have a unique ID,
	// so we use the file offset instead.
	// _heTalkOffset is used at tryLoadSoundOverride.
	_heTalkOffset = offset;

	_digiSndMode |= DIGI_SND_MODE_TALKIE;
	_heMixer->stopChannel(talkieChannel);
	_vm->_res->nukeResource(rtSound, HSND_TALKIE_SLOT);

	file.seek(offset, SEEK_SET);

	if (_vm->hasLocalizer()) {
		file.seek(offset + 4, SEEK_SET);
		length = file.readUint32BE();
		file.seek(offset, SEEK_SET);
	}

	_vm->_res->createResource(rtSound, 1, length);
	ptr = _vm->getResourceAddress(rtSound, 1);
	file.read(ptr, length);

	addSoundToQueue(HSND_TALKIE_SLOT, 0, talkieChannel, 0, HSND_BASE_FREQ_FACTOR, HSND_SOUND_PAN_CENTER, HSND_MAX_VOLUME);
}

void SoundHE::playVoiceFile(char *filename) {
	// This is unimplemented on purpose! I haven't found a game which uses this;
	// after all, this is a development path and it doesn't seem to affect any game
	// version I had the chance to test. Emphasis on "had the chance to test":
	// I don't know if there's actually a game which uses this!
	//
	// Originally this served the purpose of fetching voice lines from separate files
	// instead of using a single .HE2 bundle.
	//
	// Again, we should never end up in here, but IF WE DO we issue a message box.
	GUIErrorMessageWithURL(_(
			"Unimplemented development codepath encountered within the sound engine,\n"
			"please file a ticket at https://bugs.scummvm.org."),
		"https://bugs.scummvm.org");

	warning("SoundHE::playVoiceFile(): Unimplemented development codepath");
}

#ifdef ENABLE_HE
void SoundHE::createSound(int baseSound, int sound) {
	byte *baseSndPtr, *sndPtr, *baseSndSbngPtr, *sndSbngPtr, *dep, *sep;
	int baseSndLeft, sndSize, channel;

	if (sound == -1) {
		debug(5, "SoundHE::createSound(): Resetting append position...");
		_createSndLastAppend = 0;
		_createSndLastPos = 0;
		_baseSndSize = 0;
		return;
	}

	if (baseSound != _createSndId) {
		_createSndId = baseSound;
		_createSndLastAppend = 0;
		_createSndLastPos = 0;
		_baseSndSize = 0;
	}

	debug(5, "SoundHE::createSound(): Appending sound %d to base sound %d", sound, baseSound);

	_vm->ensureResourceLoaded(rtSound, baseSound);
	_vm->ensureResourceLoaded(rtSound, sound);
	_vm->_res->lock(rtSound, baseSound);
	_vm->_res->lock(rtSound, sound);

	baseSndPtr = _vm->getResourceAddress(rtSound, baseSound);
	sndPtr = _vm->getResourceAddress(rtSound, sound);

	channel = hsFindSoundChannel(baseSound);

	bool sndIsWav = findWavBlock(MKTAG('d', 'a', 't', 'a'), baseSndPtr) != nullptr;

	if (!sndIsWav) {
		// For non-WAV files we have to deal with sound variables (i.e. skip them :-) )
		baseSndSbngPtr = ((ScummEngine_v71he *)_vm)->heFindResource(MKTAG('S', 'B', 'N', 'G'), baseSndPtr);

		if (baseSndSbngPtr != nullptr) {
			sndSbngPtr = ((ScummEngine_v71he *)_vm)->heFindResource(MKTAG('S', 'B', 'N', 'G'), sndPtr);

			if (sndSbngPtr != nullptr) {
				if (channel != -1 && (_heChannel[channel].codeOffset > 0)) {
					memcpy(baseSndSbngPtr + 8,
						baseSndPtr + _heChannel[channel].codeOffset,
						(READ_BE_UINT32(baseSndSbngPtr + 4) - 8) - (_heChannel[channel].codeOffset - (baseSndSbngPtr - baseSndPtr + 8)));

					_heChannel[channel].codeOffset = baseSndSbngPtr + 8 - baseSndPtr;
					dep = baseSndSbngPtr + 8;

					while (READ_LE_UINT16(dep) != 0)
						dep += READ_LE_UINT16(dep);
				} else {
					_heChannel[channel].codeOffset = baseSndSbngPtr + 8 - baseSndPtr;
					dep = baseSndSbngPtr + 8;
				}

				sep = sndSbngPtr + 8;
				while (READ_LE_UINT16(sep) != 0)
					sep += READ_LE_UINT16(sep);

				memcpy(dep, sndSbngPtr + 8, sep - sndSbngPtr - 8 + 2);

				while (READ_LE_UINT16(dep)) {
					WRITE_LE_UINT32(dep + 2, READ_LE_UINT32(dep + 2) + _createSndLastPos);
					dep += READ_LE_UINT16(dep);
				}
			}
		}
	}

	// Find where the actual sound data is located...
	if (sndIsWav) {
		baseSndPtr = const_cast<byte *>(findWavBlock(MKTAG('d', 'a', 't', 'a'), baseSndPtr));
		if (baseSndPtr == nullptr)
			error("SoundHE::createSound(): Bad format for sound %d, couldn't find data tag", baseSound);

		sndPtr = const_cast<byte *>(findWavBlock(MKTAG('d', 'a', 't', 'a'), sndPtr));
		if (sndPtr == nullptr)
			error("SoundHE::createSound(): Bad format for sound %d, couldn't find data tag", sound);

		if (_baseSndSize == 0) {
			_baseSndSize = READ_LE_UINT32(baseSndPtr + sizeof(uint32)) - 8;
		}

		sndSize = READ_LE_UINT32(sndPtr + sizeof(uint32)) - 8;
	} else {
		baseSndPtr = ((ScummEngine_v71he *)_vm)->heFindResource(MKTAG('S', 'D', 'A', 'T'), baseSndPtr);
		if (baseSndPtr == nullptr)
			error("SoundHE::createSound(): Bad format for sound %d, couldn't find SDAT tag", baseSound);

		sndPtr = ((ScummEngine_v71he *)_vm)->heFindResource(MKTAG('S', 'D', 'A', 'T'), sndPtr);
		if (sndPtr == nullptr)
			error("SoundHE::createSound(): Bad format for sound %d, couldn't find SDAT tag", sound);

		_baseSndSize = READ_BE_UINT32(baseSndPtr + sizeof(uint32)) - 8;
		sndSize = READ_BE_UINT32(sndPtr + sizeof(uint32)) - 8;
	}

	// Finally perform the merging of the sound data
	baseSndLeft = _baseSndSize - _createSndLastAppend;

	baseSndPtr += 8;
	sndPtr += 8;

	if (sndSize < baseSndLeft) {
		memcpy(baseSndPtr + _createSndLastAppend, sndPtr, sndSize);
		_createSndLastAppend += sndSize;
	} else {
		memcpy(baseSndPtr + _createSndLastAppend, sndPtr, baseSndLeft);
		if (sndSize - baseSndLeft) {
			memcpy(baseSndPtr, sndPtr + baseSndLeft, sndSize - baseSndLeft);
		}
		_createSndLastAppend = sndSize - baseSndLeft;
	}

	_createSndLastPos += sndSize;

	_vm->_res->unlock(rtSound, baseSound);
	_vm->_res->unlock(rtSound, sound);
}

const byte *SoundHE::findWavBlock(uint32 tag, const byte *block) {
	const byte *wsouPtr = block;

	// For compatibility reason with old sound formats this
	// doesn't error out, and instead gracefully returns a nullptr.
	if (READ_BE_UINT32(wsouPtr) != MKTAG('W', 'S', 'O', 'U'))
		return nullptr;

	// Skip over the WSOU header...
	const byte *soundPtr = wsouPtr + 8;
	if (READ_BE_UINT32(soundPtr) != MKTAG('R', 'I', 'F', 'F'))
		error("SoundHE::findWavBlock(): Expected RIFF block");

	int riffLength = READ_LE_UINT32(soundPtr + 4);
	assert((riffLength & 1) == 0); // It must be even, since all sub-blocks must be padded to even.

	// Skip over RIFF and length and go to the actual sound data...
	const byte *wavePtr = soundPtr + 8;
	assert(READ_BE_UINT32(wavePtr) == MKTAG('W', 'A', 'V', 'E'));
	wavePtr += 4; // Skip over the WAVE tag
	riffLength -= 4;

	// Walk the nested blocks of the .wav file...
	while (riffLength > 0) {
		uint32 chunkID = READ_BE_UINT32(wavePtr);
		uint32 chunkLength = READ_LE_UINT32(wavePtr + 4);
		if ((int)chunkLength < 0)
			error("SoundHE::findWavBlock(): Illegal chunk length - %d bytes", chunkLength);
		if ((int)chunkLength > (int)riffLength)
			error("SoundHE::findWavBlock(): Chunk extends beyond file end - %d versus %d", chunkLength, riffLength);

		riffLength -= 8;

		if (chunkID == tag)
			return wavePtr;

		wavePtr += 8;

		// Round up to the next multiple of two.
		chunkLength = (chunkLength + 1) & ~1;
		wavePtr += chunkLength;
		riffLength -= chunkLength;
	}

	return nullptr;
}

#endif

int SoundHE::getCurrentSpeechOffset() {
	return _heTalkOffset;
}

bool SoundHE::is3DOSound(int sound) const {
	byte *soundAddr = _vm->getResourceAddress(rtSound, sound);
	if (soundAddr == nullptr)
		return false;

	return READ_BE_UINT32(soundAddr) == MKTAG('M', 'R', 'A', 'W');
}

} // End of namespace Scumm
