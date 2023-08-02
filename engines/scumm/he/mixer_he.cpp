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

#include "mixer_he.h"

namespace Scumm {

HEMixer::HEMixer(Audio::Mixer *mixer, ScummEngine_v60he *vm, bool useMilesSoundSystem) {
	_mixer = mixer;
	_vm = vm;
	_useMilesSoundSystem = useMilesSoundSystem;
}

HEMixer::~HEMixer() {
}

void *HEMixer::getMilesSoundSystemObject() {
	return nullptr;
}

bool HEMixer::initSoftMixerSubSystem() {
	return false;
}

void HEMixer::deinitSoftMixerSubSystem() {
}

void HEMixer::endNeglectProcess() {
}

void HEMixer::startLongNeglectProcess() {
}

bool HEMixer::forceMusicBufferFill() {
	return false;
}

bool HEMixer::isMixerDisabled() {
	return false;
}

bool HEMixer::stopChannel(int channel) {
	if (_useMilesSoundSystem) {
		return milesStopChannel(channel);
	} else {
		//return mixerStopChannel(channel);
	}

	return true;
}

void HEMixer::stopAllChannels() {
	for (int i = 0; i < HSND_MAX_CHANNELS; i++) {
		stopChannel(i);
	}
}

bool HEMixer::changeChannelVolume(int channel, int volume, bool soft) {
	return false;
}

void HEMixer::softRemixAllChannels() {
}

void HEMixer::premixUntilCritical() {
}

bool HEMixer::pauseMixerSubSystem(bool paused) {
	return false;
}

void HEMixer::feedMixer() {
}

int HEMixer::getChannelCurrentPosition(int channel) {
	return 0;
}

bool HEMixer::startChannelNew(
	int channel, int globType, int globNum, uint32 soundData, uint32 offset,
	int sampleLen, int frequency, int bitsPerSample, int sampleChannels,
	const HESoundModifiers &modifiers, int callbackID, int32 flags, ...) {
	va_list params;
	bool retValue;

	if (!_useMilesSoundSystem) {
		if (bitsPerSample != 8) {
			debug(5, "HEMixer::startChannelNew(): Glob(%d, %d) is %d bits per channel, must be 8 for software mixer", globType, globNum, bitsPerSample);
			return false;
		}

		if (CHANNEL_CALLBACK_EARLY & flags) {
			va_start(params, flags);

			retValue = mixerStartChannel(
				channel, globType, globNum, soundData + offset, sampleLen,
				frequency, modifiers.volume, callbackID, flags, va_arg(params, int));

			va_end(params);
			return retValue;
		} else {
			return mixerStartChannel(
				channel, globType, globNum, soundData + offset, sampleLen,
				frequency, modifiers.volume, callbackID, flags);
		}

	} else {
		flags &= ~CHANNEL_CALLBACK_EARLY;

		return milesStartChannel(
			channel, globType, globNum, soundData, offset, sampleLen,
			bitsPerSample, sampleChannels, frequency, modifiers, callbackID, flags);
	}

	return true;
}

bool HEMixer::startChannel(int channel, int globType, int globNum, uint32 sampleDataOffset, int sampleLen, int frequency, int volume, int callbackId, int32 flags, ...) {
	return false;
}

bool HEMixer::startSpoolingChannel(int channel, Common::File &sampleFileIOHandle, int sampleLen, int frequency, int volume, int callbackID, int32 flags, ...) {
	return false;
}

bool HEMixer::isMilesActive() {
	return _useMilesSoundSystem;
}

bool HEMixer::changeChannelVolume(int channel, int newVolume, int soft_flag) {
	return false;
}

void HEMixer::milesStartSpoolingChannel(int channel, const char *filename, long offset, int flags, HESoundModifiers modifiers) {
	assert(channel >= 0 && channel < ARRAYSIZE(_milesChannels));

	if (channel >= 0 && channel < ARRAYSIZE(_milesChannels))
		_milesChannels[channel].startSpoolingChannel(filename, offset, flags, modifiers);
}

int HEMixer::hsFindSoundQueue(int sound) {
	return 0;
}

bool HEMixer::mixerStartChannel(int channel, int globType, int globNum, uint32 sampleDataOffset, int sampleLen, int frequency, int volume, int callbackID, uint32 flags, ...) {
	return false;
}

bool HEMixer::milesStartChannel(int channel, int globType, int globNum, uint32 soundData, uint32 offset, int sampleLen, int bitsPerSample, int sampleChannels, int frequency, HESoundModifiers modifiers, int callbackID, uint32 flags, ...) {
	// Stop any running sound on the target channel,
	// which is also going to trigger the appropriate callbacks
	milesStopChannel(channel);

	int audioDataLen = 0;
	int compType = WAVE_FORMAT_PCM;
	byte *audioData = milesGetAudioDataFromResource(globType, globNum, audioDataLen, compType);

	byte streamFlags = 0;
	if (bitsPerSample == 8) // 8 bit data is unsigned
		streamFlags |= Audio::FLAG_UNSIGNED;
	else if (bitsPerSample == 16) // 16 bit data is signed little endian
		streamFlags |= (Audio::FLAG_16BITS | Audio::FLAG_LITTLE_ENDIAN);
	else if (bitsPerSample == 24) // 24 bit data is signed little endian
		streamFlags |= (Audio::FLAG_24BITS | Audio::FLAG_LITTLE_ENDIAN);
	else if (bitsPerSample == 4 && (compType == WAVE_FORMAT_IMA_ADPCM))
		streamFlags |= Audio::FLAG_16BITS;

	if (sampleChannels == 2)
		streamFlags |= Audio::FLAG_STEREO;


	if (audioData) {
		_vm->_res->lock((ResType)globType, globNum);

		_milesChannels[channel].dataOffset = soundData + offset;
		_milesChannels[channel].lastPlayPosition = 0;
		_milesChannels[channel].globType = globType;
		_milesChannels[channel].globNum = globNum;

		// Actually play the sound
		if (flags & CHANNEL_LOOPING) {
			_milesChannels[channel].playFlags = CHANNEL_LOOPING;

			if (compType == WAVE_FORMAT_PCM) {
				Audio::RewindableAudioStream *stream = Audio::makeRawStream(audioData, audioDataLen, frequency, streamFlags);
				_mixer->playStream(Audio::Mixer::kPlainSoundType, &_milesChannels[channel].audioHandle,
								   Audio::makeLoopingAudioStream(stream, 0), channel, 255, 0, DisposeAfterUse::NO);
			} else {
				error("HEMixer::milesStartChannel(): Looping ADPCM not yet implemented!");
			}
		} else {
			_milesChannels[channel].playFlags = CHANNEL_EMPTY_FLAGS;

			int newFrequency = (frequency * modifiers.frequencyShift) / HSND_SOUND_FREQ_BASE;

			_milesChannels[channel].m_modifiers.frequencyShift = modifiers.frequencyShift;
			_milesChannels[channel].m_modifiers.volume = modifiers.volume;
			_milesChannels[channel].m_modifiers.pan = modifiers.pan;

			_milesChannels[channel].m_baseFrequency = frequency;

			int scaledPan = (modifiers.pan != 64) ? 2 * modifiers.pan - 127 : 0;
			if (compType == WAVE_FORMAT_PCM) {
				Audio::SeekableAudioStream *stream = Audio::makeRawStream(audioData + offset, audioDataLen - offset, newFrequency, streamFlags);
				_mixer->playStream(Audio::Mixer::kPlainSoundType, &_milesChannels[channel].audioHandle,
								   stream, channel, modifiers.volume, scaledPan, DisposeAfterUse::NO);
			} else {
				error("HEMixer::milesStartChannel(): ADPCM not yet implemented!");
			}
		}
	}

	return true;
}

bool HEMixer::milesStopChannel(int channel) {
	milesStopAndCallback(channel, HSND_SOUND_STOPPED);

	return true;
}

void HEMixer::milesStopAllSounds() {
	for (int i = 0; i < MILES_MAX_CHANNELS; i++) {
		milesStopChannel(i);
	}
}

void HEMixer::milesModifySound(int channel, int offset, HESoundModifiers modifiers, int flags) {
	Audio::SoundHandle audioHandle = _milesChannels[channel].audioHandle;

	debug(5, "HEMixer::milesModifySound(): modifying sound in channel %d, flags %d, vol %d, pan %d, freq %d",
		channel, flags, modifiers.volume, modifiers.pan, modifiers.frequencyShift);

	if (_mixer->isSoundHandleActive(audioHandle)) {
		if (flags & ScummEngine_v70he::HESndFlags::HE_SND_VOL)
			_milesChannels[channel].m_modifiers.volume = modifiers.volume;

		if (flags & ScummEngine_v70he::HESndFlags::HE_SND_PAN)
			_milesChannels[channel].m_modifiers.pan = modifiers.pan;

		if ((flags & ScummEngine_v70he::HESndFlags::HE_SND_VOL) || (flags & ScummEngine_v70he::HESndFlags::HE_SND_PAN)) {
			int scaledPan = (modifiers.pan != 64) ? 2 * modifiers.pan - 127 : 0;

			_mixer->setChannelVolume(_milesChannels[channel].audioHandle, _milesChannels[channel].m_modifiers.volume);
			_mixer->setChannelBalance(_milesChannels[channel].audioHandle, scaledPan);
		}

		if (flags & ScummEngine_v70he::HESndFlags::HE_SND_FREQUENCY) {
			_milesChannels[channel].m_modifiers.frequencyShift = modifiers.frequencyShift;
			int newFrequency = (_milesChannels[channel].m_baseFrequency * modifiers.frequencyShift) / HSND_SOUND_FREQ_BASE;
			if (newFrequency)
				_mixer->setChannelRate(_milesChannels[channel].audioHandle, newFrequency);
		}
	}
}

void HEMixer::milesStopAndCallback(int channel, int messageId) {
	if (!_mixer->isSoundHandleActive(_milesChannels[channel].audioHandle) &&
		!_milesChannels[channel].m_stream)
		return;

	if (_mixer->isSoundHandleActive(_milesChannels[channel].audioHandle)) {
		// Stop the sound, and then unload it...
		_mixer->stopHandle(_milesChannels[channel].audioHandle);
		int globType = _milesChannels[channel].globType;
		int globNum = _milesChannels[channel].globNum;

		if (!_vm->_res->isOffHeap((ResType)globType, globNum)) {
			_vm->_res->unlock((ResType)globType, globNum);

			if (globType == rtSound && globNum == HSND_TALKIE_SLOT) {
				// Voice files have to be manually purged
				_vm->_res->nukeResource((ResType)globType, globNum);
			}
		}
	} else {
		// TODO: Stop the spooling file stream...
	}

	_milesChannels[channel].clearChannelData();

	// Signal the sound engine that we've stopped a sound
	((SoundHE *)_vm->_sound)->digitalSoundCallback(messageId, channel);

}

void HEMixer::milesRestoreChannel(int channel) {
	milesStopAndCallback(channel, HSND_SOUND_TIMEOUT);
}

void HEMixer::milesFeedMixer() {
}

bool HEMixer::milesPauseMixerSubSystem(bool paused) {
	_mixerPaused = paused;

	_mixer->pauseAll(_mixerPaused);

	return true;
}

byte *HEMixer::milesGetAudioDataFromResource(int globType, int globNum, int &dataLength, int &compType) {
	byte *globPtr = _vm->getResourceAddress((ResType)globType, globNum);

	if (globPtr == nullptr) {
		error("HEMixer::milesGetAudioDataFromResource(): Glob(%d,%d) missing from heap", globType, globNum);
	}

	uint32 globId = READ_BE_UINT32(globPtr);

	if (globId != MKTAG('W', 'S', 'O', 'U')) {
		debug(5, "HEMixer::milesGetAudioDataFromResource(): Glob(%d,%d) - type '%s' - is not a WSOU (wrapped .wav) file", globType, globNum, tag2str(globId));
		return nullptr;
	}

	// WSOU tag found, skip to the RIFF header...
	globPtr += 8;
	globId = READ_BE_UINT32(globPtr);

	if (globId != MKTAG('R', 'I', 'F', 'F')) {
		debug(5, "HEMixer::milesGetAudioDataFromResource(): Glob(%d,%d) - '%s' - is not a .wav file", globType, globNum, tag2str(globId));
		return nullptr;
	}

	compType = READ_LE_UINT16(globPtr + 20); // Format type from the 'fmt ' block
	dataLength = READ_LE_UINT32(globPtr + 40); // Length field of the 'data' block

	if (compType != WAVE_FORMAT_PCM && compType != WAVE_FORMAT_IMA_ADPCM) {
		debug("HEMixer::milesGetAudioDataFromResource(): .wav files must be PCM or IMA ADPCM. Unsupported .wav sound type %d.", compType);
		return nullptr;
	}

	return globPtr + WAVE_RIFF_HEADER_LEN;
}

void HEMilesChannel::startSpoolingChannel(const char *filename, long offset, int flags, HESoundModifiers modifiers) {
	// TODO
}

void HEMilesChannel::clearChannelData() {
	lastPlayPosition = 0;
	playFlags = 0;
	dataOffset = 0;
	globType = 0;
	globNum = 0;
	m_fileHandle = nullptr;
	m_stream = nullptr;
	m_baseFrequency = 0;
	m_modifiers.volume = HSND_MAX_VOLUME;
	m_modifiers.pan = HSND_SOUND_PAN_CENTER;
	m_modifiers.frequencyShift = HSND_BASE_FREQ_FACTOR;
}

void HEMilesChannel::closeFileHandle() {
}

} // End of namespace Scumm
