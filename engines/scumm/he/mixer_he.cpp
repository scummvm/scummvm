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

HEMixer::HEMixer(Audio::Mixer *mixer, bool useMilesSoundSystem) {
	_mixer = mixer;
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
	return false;
}

void HEMixer::stopAllChannels() {
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

bool HEMixer::startChannelNew(
	int channel, int globType, int globNum, uint32 soundData, uint32 offset,
	int sampleLen, int frequency, int bitsPerSample, int sampleChannels,
	const HESoundModifiers &modifiers, int callbackID, int32 flags, ...) {
	va_list params;
	bool retValue;

	if (!_useMilesSoundSystem) {
		if (bitsPerSample != 8) {
			debug(5, "HEMixer::PX_StartChannel(): Glob(%d, %d) is %d bits per channel, must be 8 for software mixer", globType, globNum, bitsPerSample);
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
}

int HEMixer::hsFindSoundQueue(int sound) {
	return 0;
}

bool HEMixer::mixerStartChannel(int channel, int globType, int globNum, uint32 sampleDataOffset, int sampleLen, int frequency, int volume, int callbackID, uint32 flags, ...) {
	return false;
}

bool HEMixer::milesStartChannel(int channel, int globType, int globNum, uint32 sound_data, uint32 offset, int sampleLen, int bitsPerSample, int sampleChannels, int frequency, HESoundModifiers modifiers, int callbackID, uint32 flags, ...) {
	return false;
}

void HEMixer::milesModifySound(int channel, int offset, HESoundModifiers modifiers, int flags) {
}

} // End of namespace Scumm
