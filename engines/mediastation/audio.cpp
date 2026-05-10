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

#include "audio/decoders/adpcm.h"
#include "audio/decoders/raw.h"

#include "mediastation/audio.h"
#include "mediastation/debugchannels.h"
#include "mediastation/mediastation.h"

namespace MediaStation {

AudioSequence::~AudioSequence() {
	stop();

	for (Audio::SeekableAudioStream *stream : _streams) {
		delete stream;
	}
	_streams.clear();
}

void AudioSequence::start() {
	if (_state != kSoundPlayStateStopped) {
		return;
	}

	g_engine->registerAudioSequence(this);
	playStateChanged(kSoundPlayStatePlaying);

	_handle = Audio::SoundHandle();
	if (!_streams.empty()) {
		Audio::QueuingAudioStream *audio = Audio::makeQueuingAudioStream(22050, false);
		for (Audio::SeekableAudioStream *stream : _streams) {
			stream->rewind();
			audio->queueAudioStream(stream, DisposeAfterUse::NO);
		}
		g_engine->_mixer->playStream(Audio::Mixer::kPlainSoundType, &_handle, audio, -1, Audio::Mixer::kMaxChannelVolume, DisposeAfterUse::YES);
		audio->finish();
	}
}

void AudioSequence::pause() {
	playStateChanged(kSoundPlayStatePaused);
	g_engine->_mixer->pauseHandle(_handle, true);
}

void AudioSequence::resume() {
	playStateChanged(kSoundPlayStatePlaying);
	g_engine->_mixer->pauseHandle(_handle, false);
}

void AudioSequence::stop() {
	g_engine->_mixer->stopHandle(_handle);
	_handle = Audio::SoundHandle();
	g_engine->unregisterAudioSequence(this);
	playStateChanged(kSoundPlayStateStopped, kSoundStopForScriptStop);
}

void AudioSequence::sleep() {
	g_engine->unregisterAudioSequence(this);
	playStateChanged(kSoundPlayStatePaused);
}

void AudioSequence::awake() {
	// Not much happens here because the original CD-ROM streaming/caching
	// logic has not been reimplemented.
	playStateChanged(kSoundPlayStateAwake);
}

void AudioSequence::service() {
	bool soundActuallyPlaying = g_engine->_mixer->isSoundHandleActive(_handle);
	if (_state == kSoundPlayStatePlaying && !soundActuallyPlaying) {
		makeSoundIdle(kSoundStopForEnd);
	}
}

void AudioSequence::makeSoundIdle(SoundStopReason stopReason) {
	g_engine->unregisterAudioSequence(this);
	playStateChanged(kSoundPlayStateStopped, stopReason);
}

void AudioSequence::playStateChanged(SoundPlayState state, SoundStopReason why) {
	_state = state;
	_client->soundPlayStateChanged(state, why);
}

void AudioSequence::readParameters(Chunk &chunk) {
	_chunkCount = chunk.readTypedUint16();
	_rate = chunk.readTypedUint32();
	_channelCount = chunk.readTypedUint16();
	_bitsPerSample = chunk.readTypedUint16();
}

void AudioSequence::readChunk(Chunk &chunk) {
	Common::SeekableReadStream *buffer = chunk.readStream(chunk._length);
	Audio::SeekableAudioStream *stream = nullptr;
	switch (_bitsPerSample) {
	case 16:
		stream = Audio::makeRawStream(buffer, _rate, Audio::FLAG_16BITS | Audio::FLAG_LITTLE_ENDIAN);
		break;

	case 4: // IMA ADPCM-encoded, raw nibbles (no headers).
		stream = Audio::makeADPCMStream(buffer, DisposeAfterUse::YES, 0, Audio::kADPCMDVI, _rate, 1, 8);
		break;

	default:
		error("%s: Unknown audio encoding 0x%x", __func__, static_cast<uint>(_bitsPerSample));
	}
	_streams.push_back(stream);
}

} // End of namespace MediaStation
