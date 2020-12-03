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

#include "sci/sound/audio32.h"
#include "audio/audiostream.h"      // for SeekableAudioStream
#include "audio/decoders/aiff.h"    // for makeAIFFStream
#include "audio/decoders/mac_snd.h" // for makeMacSndStream
#include "audio/decoders/raw.h"     // for makeRawStream, RawFlags::FLAG_16BITS
#include "audio/decoders/wave.h"    // for makeWAVStream
#include "audio/rate.h"             // for RateConverter, makeRateConverter
#include "audio/timestamp.h"        // for Timestamp
#include "common/config-manager.h"  // for ConfMan
#include "common/endian.h"          // for MKTAG
#include "common/memstream.h"       // for MemoryReadStream
#include "common/str.h"             // for String
#include "common/stream.h"          // for SeekableReadStream
#include "common/system.h"          // for OSystem, g_system
#include "common/textconsole.h"     // for warning
#include "common/types.h"           // for Flag::NO
#include "engines/engine.h"         // for Engine, g_engine
#include "sci/console.h"            // for Console
#include "sci/engine/features.h"    // for GameFeatures
#include "sci/engine/guest_additions.h" // for GuestAdditions
#include "sci/engine/vm_types.h"    // for reg_t, make_reg, NULL_REG
#include "sci/resource/resource.h"  // for ResourceId, ResourceType::kResour...
#include "sci/sci.h"                // for SciEngine, g_sci, getSciVersion
#include "sci/sound/decoders/sol.h" // for makeSOLStream

namespace Sci {

bool detectSolAudio(Common::SeekableReadStream &stream) {
	const size_t initialPosition = stream.pos();

	byte header[6];
	if (stream.read(header, sizeof(header)) != sizeof(header)) {
		stream.seek(initialPosition);
		return false;
	}

	stream.seek(initialPosition);

	if ((header[0] & 0x7f) != kResourceTypeAudio || READ_BE_UINT32(header + 2) != MKTAG('S', 'O', 'L', 0)) {
		return false;
	}

	return true;
}

bool detectWaveAudio(Common::SeekableReadStream &stream) {
	const size_t initialPosition = stream.pos();

	byte blockHeader[8];
	if (stream.read(blockHeader, sizeof(blockHeader)) != sizeof(blockHeader)) {
		stream.seek(initialPosition);
		return false;
	}

	stream.seek(initialPosition);
	const uint32 headerType = READ_BE_UINT32(blockHeader);

	if (headerType != MKTAG('R', 'I', 'F', 'F')) {
		return false;
	}

	return true;
}

bool detectAIFFAudio(Common::SeekableReadStream &stream) {
	const size_t initialPosition = stream.pos();

	byte blockHeader[8];
	if (stream.read(blockHeader, sizeof(blockHeader)) != sizeof(blockHeader)) {
		stream.seek(initialPosition);
		return false;
	}

	stream.seek(initialPosition);
	const uint32 headerType = READ_BE_UINT32(blockHeader);

	if (headerType != MKTAG('F', 'O', 'R', 'M')) {
		return false;
	}

	return true;
}

bool detectMacSndAudio(Common::SeekableReadStream &stream) {
	const size_t initialPosition = stream.pos();

	byte header[14];
	if (stream.read(header, sizeof(header)) != sizeof(header)) {
		stream.seek(initialPosition);
		return false;
	}

	stream.seek(initialPosition);

	return (READ_BE_UINT16(header) == 1 &&
		READ_BE_UINT16(header + 2) == 1 &&
		READ_BE_UINT16(header + 4) == 5 &&
		READ_BE_UINT32(header + 10) == 0x00018051);
}

#pragma mark -
#pragma mark MutableLoopAudioStream

class MutableLoopAudioStream : public Audio::AudioStream {
public:
	MutableLoopAudioStream(Audio::RewindableAudioStream *stream, const bool loop_, const DisposeAfterUse::Flag dispose = DisposeAfterUse::YES) :
		_stream(stream, dispose),
		_loop(loop_) {}

	int readBuffer(int16 *buffer, int numSamples) override {
		int totalSamplesRead = 0;
		int samplesRead;
		do {
			if (_loop && _stream->endOfStream()) {
				_stream->rewind();
			}

			samplesRead = _stream->readBuffer(buffer, numSamples);
			totalSamplesRead += samplesRead;
			numSamples -= samplesRead;
			buffer += samplesRead;
		} while (samplesRead > 0 && _loop && numSamples > 0);
		return totalSamplesRead;
	}

	bool isStereo() const override {
		return _stream->isStereo();
	}

	int getRate() const override {
		return _stream->getRate();
	}

	bool endOfData() const override {
		return !_loop && _stream->endOfData();
	}

	bool endOfStream() const override {
		return !_loop && _stream->endOfStream();
	}

	bool &loop() {
		return _loop;
	}

	bool loop() const {
		return _loop;
	}

	virtual Audio::Timestamp getLength() const {
		Audio::SeekableAudioStream *stream = dynamic_cast<Audio::SeekableAudioStream *>(_stream.get());
		if (stream == nullptr) {
			error("Cannot get length from a non-seekable stream");
		}
		return stream->getLength();
	}

private:
	Common::DisposablePtr<Audio::RewindableAudioStream> _stream;
	bool _loop;
};

#pragma mark -

Audio32::Audio32(ResourceManager *resMan) :
	_resMan(resMan),
	_mixer(g_system->getMixer()),
	_handle(),
	_mutex(),

	_channels(getSciVersion() < SCI_VERSION_2_1_EARLY ? 10 : getSciVersion() < SCI_VERSION_3 ? 5 : 8),
	_numActiveChannels(0),
	_inAudioThread(false),

	_globalSampleRate(44100),
	_maxAllowedSampleRate(44100),
	_globalBitDepth(16),
	_maxAllowedBitDepth(16),
	_globalNumOutputChannels(2),
	_maxAllowedOutputChannels(2),
	_preload(0),

	_robotAudioPaused(false),

	_pausedAtTick(0),
	_startedAtTick(0),

	_attenuatedMixing(true),
	_useModifiedAttenuation(g_sci->_features->usesModifiedAudioAttenuation()),

	_monitoredChannelIndex(-1),
	_numMonitoredSamples(0) {
	// In games where scripts premultiply master audio volumes into the volumes
	// of the individual audio channels sent to the mixer, Audio32 needs to use
	// the kPlainSoundType so that the master SFX volume is not applied twice.
	// Otherwise, we simply pass along master volume changes to the ScummVM
	// mixer for the kSFXSoundType and allow it to control master volume.
	// (The volume of the kSFXSoundType in the mixer still needs to be updated
	// for games that control master volumes themselves so that videos will play
	// at the same volume as the rest of the game.)
	const Audio::Mixer::SoundType soundType = g_sci->_features->gameScriptsControlMasterVolume() ? Audio::Mixer::kPlainSoundType : Audio::Mixer::kSFXSoundType;

	_mixer->playStream(soundType, &_handle, this, -1, Audio::Mixer::kMaxChannelVolume, 0, DisposeAfterUse::NO, true);
}

Audio32::~Audio32() {
	stop(kAllChannels);
	_mixer->stopHandle(_handle);
}

#pragma mark -
#pragma mark AudioStream implementation

int Audio32::writeAudioInternal(Audio::AudioStream &sourceStream, Audio::RateConverter &converter, Audio::st_sample_t *targetBuffer, const int numSamples, const Audio::st_volume_t leftVolume, const Audio::st_volume_t rightVolume) {
	const int samplePairsToRead = numSamples >> 1;
	const int samplePairsWritten = converter.flow(sourceStream, targetBuffer, samplePairsToRead, leftVolume, rightVolume);
	return samplePairsWritten << 1;
}

int16 Audio32::getNumChannelsToMix() const {
	Common::StackLock lock(_mutex);
	int16 numChannels = 0;
	for (int16 channelIndex = 0; channelIndex < _numActiveChannels; ++channelIndex) {
		const AudioChannel &channel = getChannel(channelIndex);
		if (channelShouldMix(channel)) {
			++numChannels;
		}
	}
	return numChannels;
}

bool Audio32::channelShouldMix(const AudioChannel &channel) const {
	if (channel.pausedAtTick ||
		(channel.robot && (_robotAudioPaused || channel.stream->endOfStream()))) {

		return false;
	}

	if (channel.fadeStartTick) {
		const uint32 fadeElapsed = g_sci->getTickCount() - channel.fadeStartTick;
		if (fadeElapsed > channel.fadeDuration && channel.stopChannelOnFade) {
			return false;
		}
	}

	return true;
}

// In earlier versions of SCI32 engine, audio mixing is split into three
// different functions.
//
// The first function is called from the main game thread in AsyncEventCheck;
// later versions of SSCI also call it when getting the playback position. This
// function is responsible for cleaning up finished channels and filling active
// channel buffers with decompressed audio matching the hardware output audio
// format so they can just be copied into the main DAC buffer directly later.
//
// The second function is called by the audio hardware when the DAC buffer needs
// to be filled, and by `play` when there is only one active sample (so it can
// just blow away whatever was already in the DAC buffer). It merges all active
// channels into the DAC buffer and then updates the offset into the DAC buffer.
//
// Finally, a third function is called by the second function, and it actually
// puts data into the DAC buffer, performing volume, distortion, and balance
// adjustments.
//
// Since we only have one callback from the audio thread, and should be able to
// do all audio processing in real time, and we have streams, and we do not need
// to completely fill the audio buffer, the functionality of all these original
// functions is combined here and simplified.
int Audio32::readBuffer(Audio::st_sample_t *const buffer, const int numSamples) {
	Common::StackLock lock(_mutex);

	if (_pausedAtTick != 0 || _numActiveChannels == 0) {
		return 0;
	}

	// ResourceManager is not thread-safe so we need to avoid calling into it
	// from the audio thread, but at the same time we need to be able to clear
	// out any finished channels on a regular basis
	_inAudioThread = true;
	freeUnusedChannels();

	const bool playOnlyMonitoredChannel = getSciVersion() != SCI_VERSION_3 && _monitoredChannelIndex != -1;

	// The caller of `readBuffer` is a rate converter, which reuses (without
	// clearing) an intermediate buffer, so we need to zero the intermediate
	// buffer to prevent mixing into audio data from the last callback.
	memset(buffer, 0, numSamples * sizeof(Audio::st_sample_t));

	// This emulates the attenuated mixing mode of SSCI engine, which reduces
	// the volume of the target buffer when each new channel is mixed in.
	// Instead of manipulating the content of the target buffer when mixing
	// (which would either require modification of RateConverter or an expensive
	// second pass against the entire target buffer), we just scale the volume
	// for each channel in advance, with the earliest (lowest) channel having
	// the highest amount of attenuation (lowest volume).
	int8 attenuationAmount;
	int8 attenuationStepAmount;
	if (_useModifiedAttenuation) {
		// Divides samples in target buffer by 4, and samples in source buffer
		// by 0, when adding each channel to the output buffer.
		// 1 channel:  0 >>0
		// 2 channels: 0 >>2, 1 >>0
		// 3 channels: 0 >>4, 1 >>2, 2 >>0
		// 4 channels: 0 >>6, 1 >>4, 2 >>2, 3 >>0 ...
		// Attenuation amounts are shift values.
		attenuationAmount = (getNumChannelsToMix() - 1) * 2;
		attenuationStepAmount = 2;
	} else {
		// Divides samples in both target & source buffers by 2 when adding each
		// channel to the output buffer.
		// 1 channel:  0 >>0
		// 2 channels: 0 >>1, 1 >>1
		// 3 channels: 0 >>2, 1 >>2, 2 >>1
		// 4 channels: 0 >>3, 1 >>3, 2 >>2, 3 >>1 ...
		// Attenuation amounts are shift values.
		attenuationAmount = getNumChannelsToMix() - 1;
		attenuationStepAmount = 1;
	}

	int maxSamplesWritten = 0;
	bool firstChannelWritten = false;

	for (int16 channelIndex = 0; channelIndex < _numActiveChannels; ++channelIndex) {
		const AudioChannel &channel = getChannel(channelIndex);

		if (channel.pausedAtTick || (channel.robot && _robotAudioPaused)) {
			continue;
		}

		if (channel.robot && channel.stream->endOfStream()) {
			stop(channelIndex--);
			continue;
		}

		// Channel finished fading and had the stopChannelOnFade flag set, so no
		// longer exists
		if (channel.fadeStartTick && processFade(channelIndex)) {
			--channelIndex;
			continue;
		}

		Audio::st_volume_t leftVolume, rightVolume;

		if (channel.pan == -1 || !isStereo()) {
			int volume = channel.volume;
			if (getSciVersion() == SCI_VERSION_2) {
				// In SSCI, audio is decompressed into a temporary buffer, then
				// the samples in that buffer are looped over, shifting each
				// sample right 3, 2, or 1 bits to reduce the volume within the
				// ranges given here
				if (volume > 0 && volume <= 42) {
					volume = 15;
				} else if (volume > 42 && volume <= 84) {
					volume = 31;
				} else if (volume > 84 && volume < kMaxVolume) {
					volume = 63;
				}
			} else if (getSciVersion() == SCI_VERSION_3 && volume != kMaxVolume) {
				// In SCI3, granularity of the non-maximum volumes is 1/32
				volume &= ~4;

				// In the SSCI DOS interpreter, non-maximum volumes are divided
				// by 8 which puts them in a range of [0, 16). That reduced
				// volume range gets passed into a volume function which expects
				// values [0, 32). So, effectively, all non-maximum volumes are
				// half-volume in DOS in SCI3 due to a programming bug. In
				// Windows, volumes [120, 124) are the same as 127 due to
				// another programming bug. We do not emulate either of these
				// incorrect behaviors.
			}

			leftVolume = rightVolume = volume * Audio::Mixer::kMaxChannelVolume / kMaxVolume;
		} else {
			leftVolume = channel.volume * (100 - channel.pan) / 100 * Audio::Mixer::kMaxChannelVolume / kMaxVolume;
			rightVolume = channel.volume * channel.pan / 100 * Audio::Mixer::kMaxChannelVolume / kMaxVolume;
		}

		if (!playOnlyMonitoredChannel && _attenuatedMixing) {
			assert(attenuationAmount >= 0);
			leftVolume >>= attenuationAmount;
			rightVolume >>= attenuationAmount;
			if (!_useModifiedAttenuation && !firstChannelWritten) {
				firstChannelWritten = true;
			} else {
				attenuationAmount -= attenuationStepAmount;
			}
		}

		if (channelIndex == _monitoredChannelIndex) {
			if (numSamples > (int)_monitoredBuffer.size()) {
				_monitoredBuffer.resize(numSamples);
			}
			memset(_monitoredBuffer.data(), 0, _monitoredBuffer.size() * sizeof(Audio::st_sample_t));
			_numMonitoredSamples = writeAudioInternal(*channel.stream, *channel.converter, _monitoredBuffer.data(), numSamples, leftVolume, rightVolume);

			Audio::st_sample_t *sourceBuffer = _monitoredBuffer.data();
			Audio::st_sample_t *targetBuffer = buffer;
			const Audio::st_sample_t *const end = _monitoredBuffer.data() + _numMonitoredSamples;
			while (sourceBuffer != end) {
				Audio::clampedAdd(*targetBuffer++, *sourceBuffer++);
			}

			if (_numMonitoredSamples > maxSamplesWritten) {
				maxSamplesWritten = _numMonitoredSamples;
			}
		} else if (!channel.stream->endOfStream()) {
			if (playOnlyMonitoredChannel) {
				// Audio that is not on the monitored channel is silent
				// when the monitored channel is active, but the stream still
				// needs to be read in order to ensure that sound effects sync
				// up once the monitored channel is turned off. The easiest
				// way to guarantee this is to just do the normal channel read,
				// but set the channel volume to zero so nothing is mixed in
				leftVolume = rightVolume = 0;
			}

			const int channelSamplesWritten = writeAudioInternal(*channel.stream, *channel.converter, buffer, numSamples, leftVolume, rightVolume);
			if (channelSamplesWritten > maxSamplesWritten) {
				maxSamplesWritten = channelSamplesWritten;
			}
		}
	}

	_inAudioThread = false;

	return maxSamplesWritten;
}

#pragma mark -
#pragma mark Channel management

uint8 Audio32::getNumUnlockedChannels() const {
	Common::StackLock lock(_mutex);

	uint8 numChannels = 0;
	for (uint i = 0; i < _numActiveChannels; ++i) {
		const AudioChannel &channel = getChannel(i);
		if (!channel.robot && Common::find(_lockedResourceIds.begin(), _lockedResourceIds.end(), channel.id) == _lockedResourceIds.end()) {
			++numChannels;
		}
	}

	return numChannels;
}

int16 Audio32::findChannelByArgs(EngineState *s, int argc, const reg_t *argv, const int startIndex, const reg_t soundNode) const {
	// SSCI takes extra steps to skip the subop argument here, but argc/argv are
	// already reduced by one in our engine by the kernel since these calls are
	// always subops so we do not need to do anything extra

	argc -= startIndex;
	if (argc <= 0) {
		return kAllChannels;
	}

	Common::StackLock lock(_mutex);

	if (_numActiveChannels == 0) {
		return kNoExistingChannel;
	}

	ResourceId searchId;

	if (argc < 5) {
		searchId = ResourceId(kResourceTypeAudio, argv[startIndex].toUint16());
	} else if (argc == 6 && argv[startIndex + 5].isPointer()) {
		// LSL6 hires Mac plays external AIFF files by passing filenames as strings.
		//  All other parameters are ignored.
		const Common::String audioName = s->_segMan->getString(argv[startIndex + 5]);
		uint16 audioNumber = atoi(audioName.c_str());
		if (audioNumber == 0) {
			// script passed a dummy value such as "XXXX" to indicate
			//  that all sounds should be stopped
			return kAllChannels;
		}
		searchId = ResourceId(kResourceTypeAudio, audioNumber);
	} else {
		searchId = ResourceId(
			kResourceTypeAudio36,
			argv[startIndex].toUint16(),
			argv[startIndex + 1].toUint16(),
			argv[startIndex + 2].toUint16(),
			argv[startIndex + 3].toUint16(),
			argv[startIndex + 4].toUint16()
		);
	}

	return findChannelById(searchId, soundNode);
}

int16 Audio32::findChannelById(const ResourceId resourceId, const reg_t soundNode) const {
	Common::StackLock lock(_mutex);

	if (_numActiveChannels == 0) {
		return kNoExistingChannel;
	}

	if (resourceId.getType() == kResourceTypeAudio) {
		for (int16 i = 0; i < _numActiveChannels; ++i) {
			const AudioChannel &candidate = _channels[i];
			if (
				candidate.id == resourceId &&
				(soundNode.isNull() || soundNode == candidate.soundNode)
			) {
				return i;
			}
		}
	} else if (resourceId.getType() == kResourceTypeAudio36) {
		for (int16 i = 0; i < _numActiveChannels; ++i) {
			const AudioChannel &candidate = getChannel(i);
			if (!candidate.robot && candidate.id == resourceId) {
				return i;
			}
		}
	} else {
		error("Audio32::findChannelById: Unknown resource type %d", resourceId.getType());
	}

	return kNoExistingChannel;
}

void Audio32::lockResource(const ResourceId resourceId, const bool lock) {
	Common::StackLock slock(_mutex);

	LockList::iterator it = Common::find(_lockedResourceIds.begin(), _lockedResourceIds.end(), resourceId);
	if (it != _lockedResourceIds.end()) {
		if (!lock) {
			_lockedResourceIds.erase(it);
		}
	} else {
		if (lock) {
			_lockedResourceIds.push_back(resourceId);
		}
	}
}

void Audio32::freeUnusedChannels() {
	Common::StackLock lock(_mutex);
	for (int16 channelIndex = 0; channelIndex < _numActiveChannels; ++channelIndex) {
		const AudioChannel &channel = getChannel(channelIndex);
		if (!channel.robot && channel.stream->endOfStream()) {
			stop(channelIndex--);
		}
	}

	if (!_inAudioThread) {
		unlockResources();
	}
}

void Audio32::freeChannel(const int16 channelIndex) {
	// SSCI did this:
	// 1. Unlock memory-cached resource, if one existed
	// 2. Close patched audio file descriptor, if one existed
	// 3. Free decompression memory buffer, if one existed
	// 4. Clear monitored memory buffer, if one existed
	Common::StackLock lock(_mutex);
	AudioChannel &channel = getChannel(channelIndex);

	// Robots have no corresponding resource to free
	if (channel.robot) {
		channel.stream.reset();
		channel.robot = false;
	} else {
		// We cannot unlock resources from the audio thread because
		// ResourceManager is not thread-safe; instead, we just record that the
		// resource needs unlocking and unlock it whenever we are on the main
		// thread again
		if (_inAudioThread) {
			_resourcesToUnlock.push_back(channel.resource);
		} else {
			_resMan->unlockResource(channel.resource);
		}

		channel.resource = nullptr;
		channel.stream.reset();
	}

	channel.converter.reset();

	if (_monitoredChannelIndex == channelIndex) {
		_monitoredChannelIndex = -1;
	}
}

void Audio32::unlockResources() {
	Common::StackLock lock(_mutex);
	assert(!_inAudioThread);

	for (UnlockList::const_iterator it = _resourcesToUnlock.begin(); it != _resourcesToUnlock.end(); ++it) {
		_resMan->unlockResource(*it);
	}
	_resourcesToUnlock.clear();
}

#pragma mark -
#pragma mark Script compatibility

void Audio32::setSampleRate(uint16 rate) {
	if (rate > _maxAllowedSampleRate) {
		rate = _maxAllowedSampleRate;
	}

	_globalSampleRate = rate;
}

void Audio32::setBitDepth(uint8 depth) {
	if (depth > _maxAllowedBitDepth) {
		depth = _maxAllowedBitDepth;
	}

	_globalBitDepth = depth;
}

void Audio32::setNumOutputChannels(int16 numChannels) {
	if (numChannels > _maxAllowedOutputChannels) {
		numChannels = _maxAllowedOutputChannels;
	}

	_globalNumOutputChannels = numChannels;
}

#pragma mark -
#pragma mark Robot

int16 Audio32::findRobotChannel() const {
	Common::StackLock lock(_mutex);
	for (int16 i = 0; i < _numActiveChannels; ++i) {
		if (_channels[i].robot) {
			return i;
		}
	}

	return kNoExistingChannel;
}

bool Audio32::playRobotAudio(const RobotAudioStream::RobotAudioPacket &packet) {
	// Stop immediately
	if (packet.dataSize == 0) {
		warning("Stopping robot stream by zero-length packet");
		return stopRobotAudio();
	}

	// Flush and then stop
	if (packet.dataSize == -1) {
		warning("Stopping robot stream by negative-length packet");
		return finishRobotAudio();
	}

	Common::StackLock lock(_mutex);
	int16 channelIndex = findRobotChannel();

	bool isNewChannel = false;
	if (channelIndex == kNoExistingChannel) {
		if (_numActiveChannels == _channels.size()) {
			return false;
		}

		channelIndex = _numActiveChannels++;
		isNewChannel = true;
	}

	AudioChannel &channel = getChannel(channelIndex);

	if (isNewChannel) {
		channel.id = ResourceId();
		channel.resource = nullptr;
		channel.robot = true;
		channel.fadeStartTick = 0;
		channel.pausedAtTick = 0;
		channel.soundNode = NULL_REG;
		channel.volume = kMaxVolume;
		channel.pan = -1;
		channel.converter.reset(Audio::makeRateConverter(RobotAudioStream::kRobotSampleRate, getRate(), false));
		// The RobotAudioStream buffer size is
		// ((bytesPerSample * channels * sampleRate * 2000ms) / 1000ms) & ~3
		// where bytesPerSample = 2, channels = 1, and sampleRate = 22050
		channel.stream.reset(new RobotAudioStream(88200));
		_robotAudioPaused = false;

		if (_numActiveChannels == 1) {
			_startedAtTick = g_sci->getTickCount();
		}
	}

	return static_cast<RobotAudioStream *>(channel.stream.get())->addPacket(packet);
}

bool Audio32::queryRobotAudio(RobotAudioStream::StreamState &status) const {
	Common::StackLock lock(_mutex);

	const int16 channelIndex = findRobotChannel();
	if (channelIndex == kNoExistingChannel) {
		status.bytesPlaying = 0;
		return false;
	}

	status = static_cast<RobotAudioStream *>(getChannel(channelIndex).stream.get())->getStatus();
	return true;
}

bool Audio32::finishRobotAudio() {
	Common::StackLock lock(_mutex);

	const int16 channelIndex = findRobotChannel();
	if (channelIndex == kNoExistingChannel) {
		return false;
	}

	static_cast<RobotAudioStream *>(getChannel(channelIndex).stream.get())->finish();
	return true;
}

bool Audio32::stopRobotAudio() {
	Common::StackLock lock(_mutex);

	const int16 channelIndex = findRobotChannel();
	if (channelIndex == kNoExistingChannel) {
		return false;
	}

	stop(channelIndex);
	return true;
}

#pragma mark -
#pragma mark Playback

uint16 Audio32::play(int16 channelIndex, const ResourceId resourceId, const bool autoPlay, const bool loop, const int16 volume, const reg_t soundNode, const bool monitor) {
	Common::StackLock lock(_mutex);

	freeUnusedChannels();

	if (channelIndex != kNoExistingChannel) {
		AudioChannel &channel = getChannel(channelIndex);
		MutableLoopAudioStream *stream = dynamic_cast<MutableLoopAudioStream *>(channel.stream.get());
		if (stream == nullptr) {
			error("[Audio32::play]: Unable to cast stream for resource %s", resourceId.toString().c_str());
		}

		if (channel.pausedAtTick) {
			resume(channelIndex);
			return MIN(65534, 1 + stream->getLength().msecs() * 60 / 1000);
		}

		warning("Tried to resume channel %s that was not paused", channel.id.toString().c_str());
		return MIN(65534, 1 + stream->getLength().msecs() * 60 / 1000);
	}

	if (_numActiveChannels == _channels.size()) {
		warning("Audio mixer is full when trying to play %s", resourceId.toString().c_str());
		return 0;
	}

	// SSCI normally searches in this order:
	//
	// For Audio36:
	//
	// 1. First, request a FD using Audio36 name and use it as the source FD for
	//    reading the audio resource data.
	// 2a. If the returned FD is -1, or equals the audio map, or equals the
	//     audio bundle, try to get the offset of the data from the audio map,
	//     using the Audio36 name.
	//
	//     If the returned offset is -1, this is not a valid resource; return 0.
	//     Otherwise, set the read offset for the FD to the returned offset.
	// 2b. Otherwise, use the FD as-is (it is a patch file), with zero offset,
	//     and record it separately so it can be closed later.
	//
	// For plain audio:
	//
	// 1. First, request an Audio resource from the resource cache. If one does
	//    not exist, make the same request for a Wave resource.
	// 2a. If an audio resource was discovered, record its memory ID and clear
	//     the streaming FD
	// 2b. Otherwise, request an Audio FD. If one does not exist, make the same
	//     request for a Wave FD. If neither exist, this is not a valid
	//     resource; return 0. Otherwise, use the returned FD as the streaming
	//     ID and set the memory ID to null.
	//
	// Once these steps are complete, the audio engine either has a file
	// descriptor + offset that it can use to read streamed audio, or it has a
	// memory ID that it can use to read cached audio.
	//
	// Here in ScummVM we just ask the resource manager to give us the resource
	// and we get a seekable stream.

	// TODO: This should be fixed to use streaming, which means fixing the
	// resource manager to allow streaming, which means probably rewriting a
	// bunch of the resource manager.
	Resource *resource = _resMan->findResource(resourceId, true);
	if (resource == nullptr) {
		warning("[Audio32::play]: %s could not be found", resourceId.toString().c_str());
		return 0;
	}

	channelIndex = _numActiveChannels++;

	AudioChannel &channel = getChannel(channelIndex);
	channel.id = resourceId;
	channel.resource = resource;
	channel.robot = false;
	channel.fadeStartTick = 0;
	channel.soundNode = soundNode;
	channel.volume = volume < 0 || volume > kMaxVolume ? (int)kMaxVolume : volume;
	channel.pan = -1;

	if (monitor) {
		_monitoredChannelIndex = channelIndex;
	}

	Common::SeekableReadStream *dataStream = resource->makeStream();

	Audio::RewindableAudioStream *audioStream;

	if (detectSolAudio(*dataStream)) {
		audioStream = makeSOLStream(dataStream, DisposeAfterUse::YES);
	} else if (detectWaveAudio(*dataStream)) {
		audioStream = Audio::makeWAVStream(dataStream, DisposeAfterUse::YES);
	} else if (detectAIFFAudio(*dataStream)) {
		audioStream = Audio::makeAIFFStream(dataStream, DisposeAfterUse::YES);
	} else if (detectMacSndAudio(*dataStream)) {
		audioStream = Audio::makeMacSndStream(dataStream, DisposeAfterUse::YES);
	} else {
		byte flags = Audio::FLAG_LITTLE_ENDIAN;
		if (_globalBitDepth == 16) {
			flags |= Audio::FLAG_16BITS;
		} else {
			flags |= Audio::FLAG_UNSIGNED;
		}

		if (_globalNumOutputChannels == 2) {
			flags |= Audio::FLAG_STEREO;
		}

		audioStream = Audio::makeRawStream(dataStream, _globalSampleRate, flags, DisposeAfterUse::YES);
	}

	channel.stream.reset(new MutableLoopAudioStream(audioStream, loop));
	channel.converter.reset(Audio::makeRateConverter(channel.stream->getRate(), getRate(), channel.stream->isStereo(), false));

	// SSCI sets up a decompression buffer here for the audio stream, plus
	// writes information about the sample to the channel to convert to the
	// correct hardware output format, and allocates the monitoring buffer to
	// match the bitrate/samplerate/channels of the original stream. We do not
	// need to do any of these things since we use audio streams, and allocate
	// and fill the monitoring buffer when reading audio data from the stream.

	MutableLoopAudioStream *stream = dynamic_cast<MutableLoopAudioStream *>(channel.stream.get());
	if (stream == nullptr) {
		error("[Audio32::play]: Unable to cast stream for resource %s", resourceId.toString().c_str());
	}

	channel.duration = /* round up */ 1 + (stream->getLength().msecs() * 60 / 1000);

	const uint32 now = g_sci->getTickCount();
	channel.pausedAtTick = autoPlay ? 0 : now;
	channel.startedAtTick = now;

	if (_numActiveChannels == 1) {
		if (_pausedAtTick) {
			_pausedAtTick = now;
		}
		_startedAtTick = now;
	}

	return channel.duration;
}

bool Audio32::resume(const int16 channelIndex) {
	if (channelIndex == kNoExistingChannel) {
		return false;
	}

	Common::StackLock lock(_mutex);
	const uint32 now = g_sci->getTickCount();

	if (channelIndex == kAllChannels) {
		// Global pause in SSCI is an extra layer over
		// individual channel pauses, so only unpause channels
		// if there was not a global pause in place
		if (_pausedAtTick == 0) {
			return false;
		}

		for (int i = 0; i < _numActiveChannels; ++i) {
			AudioChannel &channel = getChannel(i);
			if (!channel.pausedAtTick) {
				channel.startedAtTick += now - _pausedAtTick;
				if (channel.startedAtTick > now) {
					warning("%s is being resumed in the future", channel.id.toString().c_str());
				}
			}
		}

		_startedAtTick += now - _pausedAtTick;
		if (_startedAtTick > now) {
			warning("Audio32 is being resumed in the future");
		}
		_pausedAtTick = 0;
		return true;
	} else if (channelIndex == kRobotChannel) {
		for (int i = 0; i < _numActiveChannels; ++i) {
			AudioChannel &channel = getChannel(i);
			if (channel.robot) {
				if (channel.pausedAtTick) {
					channel.startedAtTick += now - channel.pausedAtTick;
					if (channel.startedAtTick > now) {
						warning("Robot audio is being resumed in the future");
					}
					channel.pausedAtTick = 0;
				}
				return true;
			}
		}
	} else {
		AudioChannel &channel = getChannel(channelIndex);
		if (channel.pausedAtTick) {
			channel.startedAtTick += now - channel.pausedAtTick;
			if (channel.startedAtTick > now) {
				warning("%s is being resumed in the future", channel.id.toString().c_str());
			}
			channel.pausedAtTick = 0;
			return true;
		}
	}

	return false;
}

bool Audio32::pause(const int16 channelIndex) {
	if (channelIndex == kNoExistingChannel) {
		return false;
	}

	Common::StackLock lock(_mutex);
	const uint32 now = g_sci->getTickCount();
	bool didPause = false;

	if (channelIndex == kAllChannels) {
		if (_pausedAtTick == 0) {
			_pausedAtTick = now;
			didPause = true;
		}
	} else if (channelIndex == kRobotChannel) {
		_robotAudioPaused = true;
		for (int16 i = 0; i < _numActiveChannels; ++i) {
			AudioChannel &channel = getChannel(i);
			if (channel.robot) {
				channel.pausedAtTick = now;
			}
		}

		// SSCI returns false here regardless of whether or not channels were
		// paused, so we emulate this behaviour
	} else {
		AudioChannel &channel = getChannel(channelIndex);

		if (channel.pausedAtTick == 0) {
			channel.pausedAtTick = now;
			didPause = true;
		}
	}

	return didPause;
}

int16 Audio32::stop(const int16 channelIndex) {
	Common::StackLock lock(_mutex);
	const int16 oldNumChannels = _numActiveChannels;

	if (channelIndex == kNoExistingChannel || oldNumChannels == 0) {
		return 0;
	}

	if (channelIndex == kAllChannels) {
		for (int i = 0; i < oldNumChannels; ++i) {
			freeChannel(i);
		}
		_numActiveChannels = 0;
	} else {
		freeChannel(channelIndex);
		--_numActiveChannels;
		for (int i = channelIndex; i < oldNumChannels - 1; ++i) {
			_channels[i] = _channels[i + 1];
			if (i + 1 == _monitoredChannelIndex) {
				_monitoredChannelIndex = i;
			}
		}
	}

	// SSCI stops the DSP interrupt and frees the global decompression buffer
	// here if there are no more active channels, which we do not need to do
	// since the system manages audio callbacks and we have no static
	// decompression buffer

	return oldNumChannels;
}

uint16 Audio32::restart(const ResourceId resourceId, const bool autoPlay, const bool loop, const int16 volume, const reg_t soundNode, const bool monitor) {
	Common::StackLock lock(_mutex);
	stop(resourceId, soundNode);
	return play(kNoExistingChannel, resourceId, autoPlay, loop, volume, soundNode, monitor);
}

int16 Audio32::getPosition(const int16 channelIndex) const {
	Common::StackLock lock(_mutex);
	if (channelIndex == kNoExistingChannel || _numActiveChannels == 0) {
		return -1;
	}

	// SSCI treats this as an unsigned short except for when the value is 65535,
	// then it treats it as signed
	int position = -1;
	const uint32 now = g_sci->getTickCount();

	// SSCI also queried the audio driver to see whether it thought that there
	// was audio playback occurring via driver opcode 9, but we have no analogue
	// to query
	if (channelIndex == kAllChannels) {
		if (_pausedAtTick) {
			position = _pausedAtTick - _startedAtTick;
		} else {
			position = now - _startedAtTick;
		}
	} else {
		const AudioChannel &channel = getChannel(channelIndex);

		if (channel.pausedAtTick) {
			position = channel.pausedAtTick - channel.startedAtTick;
		} else if (_pausedAtTick) {
			position = _pausedAtTick - channel.startedAtTick;
		} else {
			position = now - channel.startedAtTick;
		}
	}

	return MIN(position, 65534);
}

void Audio32::setLoop(const int16 channelIndex, const bool loop) {
	Common::StackLock lock(_mutex);

	if (channelIndex < 0 || channelIndex >= _numActiveChannels) {
		return;
	}

	AudioChannel &channel = getChannel(channelIndex);

	MutableLoopAudioStream *stream = dynamic_cast<MutableLoopAudioStream *>(channel.stream.get());
	assert(stream);
	stream->loop() = loop;
}

#pragma mark -
#pragma mark Effects

int16 Audio32::getVolume(const int16 channelIndex) const {
	bool getGlobalVolume = false;
	switch (getSciVersion()) {
	case SCI_VERSION_3:
		getGlobalVolume = (channelIndex == kAllChannels);
		break;
	default:
		getGlobalVolume = (channelIndex < 0 || channelIndex >= _numActiveChannels);
	}

	if (getGlobalVolume) {
		return (_mixer->getVolumeForSoundType(Audio::Mixer::kSFXSoundType) + 1) * kMaxVolume / Audio::Mixer::kMaxMixerVolume;
	}

	if (channelIndex < 0 || channelIndex >= _numActiveChannels) {
		return -1;
	}

	Common::StackLock lock(_mutex);
	return getChannel(channelIndex).volume;
}

void Audio32::setVolume(const int16 channelIndex, int16 volume) {
	volume = MIN<int16>(kMaxVolume, volume);
	if (channelIndex == kAllChannels) {
		if (!g_sci->_guestAdditions->audio32SetVolumeHook(channelIndex, volume)) {
			setMasterVolume(volume);
		}
	} else if (channelIndex != kNoExistingChannel) {
		Common::StackLock lock(_mutex);
		getChannel(channelIndex).volume = volume;
	}
}

bool Audio32::fadeChannel(const int16 channelIndex, const int16 targetVolume, const int16 speed, const int16 steps, const bool stopAfterFade) {
	Common::StackLock lock(_mutex);

	if (channelIndex < 0 || channelIndex >= _numActiveChannels) {
		return false;
	}

	AudioChannel &channel = getChannel(channelIndex);

	if (channel.id.getType() != kResourceTypeAudio || channel.volume == targetVolume) {
		return false;
	}

	if (steps && speed) {
		channel.fadeStartTick = g_sci->getTickCount();
		channel.fadeStartVolume = channel.volume;
		channel.fadeTargetVolume = targetVolume;
		channel.fadeDuration = speed * steps;
		channel.stopChannelOnFade = stopAfterFade;
	} else {
		setVolume(channelIndex, targetVolume);
	}

	return true;
}

bool Audio32::processFade(const int16 channelIndex) {
	Common::StackLock lock(_mutex);
	AudioChannel &channel = getChannel(channelIndex);

	if (channel.fadeStartTick) {
		const uint32 fadeElapsed = g_sci->getTickCount() - channel.fadeStartTick;
		if (fadeElapsed > channel.fadeDuration) {
			channel.fadeStartTick = 0;
			if (channel.stopChannelOnFade) {
				stop(channelIndex);
				return true;
			} else {
				setVolume(channelIndex, channel.fadeTargetVolume);
			}
			return false;
		}

		int volume;
		if (channel.fadeStartVolume > channel.fadeTargetVolume) {
			volume = channel.fadeStartVolume - fadeElapsed * (channel.fadeStartVolume - channel.fadeTargetVolume) / channel.fadeDuration;
		} else {
			volume = channel.fadeStartVolume + fadeElapsed * (channel.fadeTargetVolume - channel.fadeStartVolume) / channel.fadeDuration;
		}

		setVolume(channelIndex, volume);
		return false;
	}

	return false;
}

#pragma mark -
#pragma mark Signal monitoring

bool Audio32::hasSignal() const {
	Common::StackLock lock(_mutex);

	if (_monitoredChannelIndex == -1) {
		return false;
	}

	const Audio::st_sample_t *buffer = _monitoredBuffer.data();
	const Audio::st_sample_t *const end = _monitoredBuffer.data() + _numMonitoredSamples;

	while (buffer != end) {
		const Audio::st_sample_t sample = *buffer++;
		if (sample > 1280 || sample < -1280) {
			return true;
		}
	}

	return false;
}

#pragma mark -
#pragma mark Kernel

reg_t Audio32::kernelPlay(const bool autoPlay, EngineState *s, const int argc, const reg_t *const argv) {
	Common::StackLock lock(_mutex);

	int16 channelIndex = findChannelByArgs(s, argc, argv, 0, NULL_REG);
	ResourceId resourceId;
	bool loop;
	int16 volume;
	bool monitor = false;
	reg_t soundNode = NULL_REG;

	if (argc == 6 && argv[5].isPointer()) {
		// LSL6 hires Mac plays external AIFF files by passing filenames as strings.
		//  All other parameters are ignored.
		const Common::String audioName = s->_segMan->getString(argv[5]);
		uint16 audioNumber = atoi(audioName.c_str());
		resourceId = ResourceId(kResourceTypeAudio, audioNumber);
		loop = false;
		volume = Audio32::kMaxVolume;
		
		// SSCI only plays one AIFF file at a time using this method. The game scripts
		//  rely on this and don't stop the previous AIFF before playing another.
		//  This entire scheme is only used in two rooms, so rather than track the
		//  AIFF channel, just stop all channels when an AIFF is played.
		stop(kAllChannels);
		channelIndex = kNoExistingChannel;
	} else if (argc >= 5) {
		resourceId = ResourceId(kResourceTypeAudio36, argv[0].toUint16(), argv[1].toUint16(), argv[2].toUint16(), argv[3].toUint16(), argv[4].toUint16());

		if (argc < 6 || argv[5].toSint16() == 1) {
			loop = false;
		} else {
			// SSCI uses -1 for infinite loop. Presumably the engine was
			// supposed to allow counter loops at one point, but ended up only
			// using loop as a boolean.
			loop = (bool)argv[5].toSint16();
		}

		if (getSciVersion() == SCI_VERSION_3) {
			if (argc < 7) {
				volume = Audio32::kMaxVolume;
			} else {
				volume = argv[6].toSint16() & Audio32::kMaxVolume;
				monitor = argv[6].toSint16() & Audio32::kMonitorAudioFlagSci3;
			}
		} else {
			if (argc < 7 || argv[6].toSint16() < 0 || argv[6].toSint16() > Audio32::kMaxVolume) {
				volume = Audio32::kMaxVolume;

				if (argc >= 7) {
					monitor = true;
				}
			} else {
				volume = argv[6].toSint16();
			}
		}
	} else {
		resourceId = ResourceId(kResourceTypeAudio, argv[0].toUint16());

		if (argc < 2 || argv[1].toSint16() == 1) {
			loop = false;
		} else {
			loop = (bool)argv[1].toSint16();
		}

		if (getSciVersion() == SCI_VERSION_3) {
			if (argc < 3) {
				volume = Audio32::kMaxVolume;
			} else {
				volume = argv[2].toSint16() & Audio32::kMaxVolume;
				monitor = argv[2].toSint16() & Audio32::kMonitorAudioFlagSci3;
			}
		} else {
			if (argc < 3 || argv[2].toSint16() < 0 || argv[2].toSint16() > Audio32::kMaxVolume) {
				volume = Audio32::kMaxVolume;

				if (argc >= 3) {
					monitor = true;
				}
			} else {
				volume = argv[2].toSint16();
			}
		}

		soundNode = argc == 4 ? argv[3] : NULL_REG;
	}

	return make_reg(0, play(channelIndex, resourceId, autoPlay, loop, volume, soundNode, monitor));
}

reg_t Audio32::kernelStop(EngineState *s, const int argc, const reg_t *const argv) {
	Common::StackLock lock(_mutex);
	const int16 channelIndex = findChannelByArgs(s, argc, argv, 0, argc > 1 ? argv[1] : NULL_REG);
	return make_reg(0, stop(channelIndex));
}

reg_t Audio32::kernelPause(EngineState *s, const int argc, const reg_t *const argv) {
	Common::StackLock lock(_mutex);
	const int16 channelIndex = findChannelByArgs(s, argc, argv, 0, argc > 1 ? argv[1] : NULL_REG);
	return make_reg(0, pause(channelIndex));
}

reg_t Audio32::kernelResume(EngineState *s, const int argc, const reg_t *const argv) {
	Common::StackLock lock(_mutex);
	const int16 channelIndex = findChannelByArgs(s, argc, argv, 0, argc > 1 ? argv[1] : NULL_REG);
	return make_reg(0, resume(channelIndex));
}

reg_t Audio32::kernelPosition(EngineState *s, const int argc, const reg_t *const argv) {
	Common::StackLock lock(_mutex);
	const int16 channelIndex = findChannelByArgs(s, argc, argv, 0, argc > 1 ? argv[1] : NULL_REG);
	return make_reg(0, getPosition(channelIndex));
}

reg_t Audio32::kernelVolume(EngineState *s, const int argc, const reg_t *const argv) {
	Common::StackLock lock(_mutex);

	const int16 volume = argc > 0 ? argv[0].toSint16() : -1;
	int16 channelIndex;

	if (getSciVersion() == SCI_VERSION_3 && argc < 2) {
		channelIndex = kAllChannels;
	} else {
		channelIndex = findChannelByArgs(s, argc, argv, 1, argc > 2 ? argv[2] : NULL_REG);
	}

	if (volume != -1) {
		setVolume(channelIndex, volume);
	}

	return make_reg(0, getVolume(channelIndex));
}

reg_t Audio32::kernelMixing(const int argc, const reg_t *const argv) {
	Common::StackLock lock(_mutex);

	if (argc > 0) {
		setAttenuatedMixing(argv[0].toUint16());
	}

	return make_reg(0, getAttenuatedMixing());
}

reg_t Audio32::kernelFade(EngineState *s, const int argc, const reg_t *const argv) {
	if (argc < 4) {
		return make_reg(0, 0);
	}

	Common::StackLock lock(_mutex);

	// In SSCI, this call to find the channel is hacked up; argc is set to 2
	// before the call, and then restored after the call. We just implemented
	// findChannelByArgs in a manner that allows us to pass this information
	// without messing with argc/argv instead
	const int16 channelIndex = findChannelByArgs(s, 2, argv, 0, argc > 5 ? argv[5] : NULL_REG);
	const int16 volume = argv[1].toSint16();
	const int16 speed = argv[2].toSint16();
	const int16 steps = argv[3].toSint16();
	const bool stopAfterFade = argc > 4 ? (bool)argv[4].toUint16() : false;

	return make_reg(0, fadeChannel(channelIndex, volume, speed, steps, stopAfterFade));
}

void Audio32::kernelLoop(EngineState *s, const int argc, const reg_t *const argv) {
	Common::StackLock lock(_mutex);

	const int16 channelIndex = findChannelByArgs(s, argc, argv, 0, argc == 3 ? argv[2] : NULL_REG);
	const bool loop = argv[0].toSint16() != 0 && argv[0].toSint16() != 1;

	setLoop(channelIndex, loop);
}

void Audio32::kernelPan(EngineState *s, const int argc, const reg_t *const argv) {
	Common::StackLock lock(_mutex);

	const int16 channelIndex = findChannelByArgs(s, argc, argv, 1, argc == 3 ? argv[2] : NULL_REG);
	const int16 pan = argv[0].toSint16();
	if (channelIndex != kNoExistingChannel) {
		setPan(channelIndex, pan);
	} else {
		warning("Attempt to pan a channel that does not exist");
	}
}

void Audio32::kernelPanOff(EngineState *s, const int argc, const reg_t *const argv) {
	Common::StackLock lock(_mutex);

	const int16 channelIndex = findChannelByArgs(s, argc, argv, 0, argc == 2 ? argv[1] : NULL_REG);
	if (channelIndex != kNoExistingChannel) {
		setPan(channelIndex, -1);
	}
}

#pragma mark -
#pragma mark Debugging

void Audio32::printAudioList(Console *con) const {
	Common::StackLock lock(_mutex);
	for (int i = 0; i < _numActiveChannels; ++i) {
		const AudioChannel &channel = _channels[i];
		const MutableLoopAudioStream *stream = dynamic_cast<MutableLoopAudioStream *>(channel.stream.get());
		con->debugPrintf("  %d[%04x:%04x]: %s, started at %d, pos %d/%d, vol %d, pan %d%s%s\n",
						 i,
						 PRINT_REG(channel.soundNode),
						 channel.robot ? "robot" : channel.resource->name().c_str(),
						 channel.startedAtTick,
						 (g_sci->getTickCount() - channel.startedAtTick) % channel.duration,
						 channel.duration,
						 channel.volume,
						 channel.pan,
						 stream && stream->loop() ? ", looping" : "",
						 channel.pausedAtTick ? ", paused" : "");
		if (channel.fadeStartTick) {
			con->debugPrintf("                fade: vol %d -> %d, started at %d, pos %d/%d%s\n",
							 channel.fadeStartVolume,
							 channel.fadeTargetVolume,
							 channel.fadeStartTick,
							 (g_sci->getTickCount() - channel.fadeStartTick) % channel.duration,
							 channel.fadeDuration,
							 channel.stopChannelOnFade ? ", stopping" : "");
		}
	}

	if (g_sci->_features->hasSci3Audio()) {
		con->debugPrintf("\nLocks: ");
		if (_lockedResourceIds.size()) {
			const char *separator = "";
			for (LockList::const_iterator it = _lockedResourceIds.begin(); it != _lockedResourceIds.end(); ++it) {
				con->debugPrintf("%s%s", separator, it->toString().c_str());
				separator = ", ";
			}
		} else {
			con->debugPrintf("none");
		}
		con->debugPrintf("\n");
	}
}

} // End of namespace Sci
