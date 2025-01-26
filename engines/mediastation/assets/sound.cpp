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

#include "audio/decoders/raw.h"
#include "audio/decoders/adpcm.h"

#include "mediastation/debugchannels.h"
#include "mediastation/assets/sound.h"
#include "mediastation/mediastation.h"

namespace MediaStation {

Sound::Sound(AssetHeader *header) : Asset(header) {
	if (_header != nullptr) {
		_encoding = _header->_soundEncoding;
	}
}

Sound::~Sound() {
	for (Audio::SeekableAudioStream *stream : _streams) {
		delete stream;
	}
	_streams.clear();
}

void Sound::process() {
	processTimeEventHandlers();
	if (!g_engine->_mixer->isSoundHandleActive(_handle)) {
		_isActive = false;
		_startTime = 0;
		_lastProcessedTime = 0;
		_handle = Audio::SoundHandle();

		runEventHandlerIfExists(kSoundEndEvent);
	}
}

Operand Sound::callMethod(BuiltInMethod methodId, Common::Array<Operand> &args) {
	switch (methodId) {
	case kTimePlayMethod: {
		assert(args.empty());
		timePlay();
		return Operand();
	}

	case kTimeStopMethod: {
		assert(args.empty());
		timeStop();
		return Operand();
	}

	default: {
		error("Sound::callMethod(): Got unimplemented method %s (%d)", builtInMethodToStr(methodId), methodId);
	}
	}
}

void Sound::readChunk(Chunk &chunk) {
	byte *buffer = (byte *)malloc(chunk._length);
	chunk.read((void *)buffer, chunk._length);
	Audio::SeekableAudioStream *stream = nullptr;
	switch (_header->_soundEncoding) {
	case SoundEncoding::PCM_S16LE_MONO_22050:
		stream = Audio::makeRawStream(buffer, chunk._length, 22050, Audio::FLAG_16BITS | Audio::FLAG_LITTLE_ENDIAN, DisposeAfterUse::NO);
		break;

	case SoundEncoding::IMA_ADPCM_S16LE_MONO_22050:
		// TODO: The interface here is different. We can't pass in the
		// buffers directly. We have to make a stream first.
		// stream = Audio::makeADPCMStream(buffer, chunk.length,
		// DisposeAfterUse::NO, Audio::ADPCMType::kADPCMMSIma, 22050, 1,
		// 4);
		warning("Sound::readSubfile(): ADPCM decoding not implemented yet");
		chunk.skip(chunk.bytesRemaining());
		break;

	default:
		error("Sound::readChunk(): Unknown audio encoding 0x%x", static_cast<uint>(_header->_soundEncoding));
	}
	_streams.push_back(stream);
	debugC(5, kDebugLoading, "Sound::readChunk(): Finished reading audio chunk (@0x%llx)", static_cast<long long int>(chunk.pos()));
}

void Sound::readSubfile(Subfile &subfile, Chunk &chunk) {
	uint32 totalChunks = _header->_chunkCount;
	uint32 expectedChunkId = chunk._id;

	debugC(5, kDebugLoading, "Sound::readSubfile(): Reading %d chunks", totalChunks);
	readChunk(chunk);
	for (uint i = 1; i < totalChunks; i++) {
		debugC(5, kDebugLoading, "Sound::readSubfile(): Reading chunk %d of %d", i, totalChunks);
		chunk = subfile.nextChunk();
		if (chunk._id != expectedChunkId) {
			error("Sound::readSubfile(): Expected chunk %s, got %s", tag2str(expectedChunkId), tag2str(chunk._id));
		}
		readChunk(chunk);
	}
}

void Sound::timePlay() {
	if (_isActive) {
		warning("Sound::timePlay(): Attempt to play a sound that is already playing");
		return;
	}
	_isActive = true;
	g_engine->addPlayingAsset(this);

	_startTime = g_system->getMillis();
	_lastProcessedTime = 0;
	_handle = Audio::SoundHandle();

	runEventHandlerIfExists(kSoundBeginEvent);

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

void Sound::timeStop() {
	if (!_isActive) {
		warning("Sound::timeStop(): Attempt to stop a sound that isn't playing");
		return;
	}

	_isActive = false;
	_startTime = 0;
	_lastProcessedTime = 0;

	g_engine->_mixer->stopHandle(_handle);
	_handle = Audio::SoundHandle();

	runEventHandlerIfExists(kSoundStoppedEvent);
}

} // End of namespace MediaStation
