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

#include "mediastation/debugchannels.h"
#include "mediastation/assets/sound.h"

namespace MediaStation {

Sound::Sound(AssetHeader *header) : Asset(header) {
	if (_header != nullptr) {
		_encoding = _header->_soundEncoding;
	}
}

Sound::~Sound() {
	delete _samples;
	_samples = nullptr;
	//for (Audio::SeekableAudioStream *stream : _streams) {
	//    delete stream;
	//}
}

Operand Sound::callMethod(BuiltInMethod methodId, Common::Array<Operand> &args) {
	switch (methodId) {
	default: {
		error("Got unimplemented method ID %d", methodId);
	}
	}
}

void Sound::process() {
	// TODO: Process more playing.
}

void Sound::readChunk(Chunk &chunk) {
	// TODO: Can we read the chunk directly into the audio stream?
	debugC(5, kDebugLoading, "Sound::readChunk(): (encoding = 0x%x) Reading audio chunk (@0x%llx)", (uint)_encoding, static_cast<long long int>(chunk.pos()));
	byte *buffer = (byte *)malloc(chunk.length);
	chunk.read((void *)buffer, chunk.length);

	switch (_encoding) {
	case AssetHeader::SoundEncoding::PCM_S16LE_MONO_22050: {
		// Audio::SeekableAudioStream *stream = Audio::makeRawStream(buffer, chunk.length, Sound::RATE, Sound::FLAGS, DisposeAfterUse::NO);
		//_streams.push_back(stream);
		break;
	}

	case AssetHeader::SoundEncoding::IMA_ADPCM_S16LE_MONO_22050: {
		// TODO: Support ADPCM decoding.
		// Audio::SeekableAudioStream *stream = nullptr; // Audio::makeADPCMStream(buffer, chunk.length, DisposeAfterUse::NO, Audio::ADPCMType::kADPCMMSIma, Sound::RATE, 1, 4);
		//_streams.push_back(stream);
		break;
	}

	default: {
		error("Sound::readChunk(): Unknown audio encoding 0x%x", (uint)_encoding);
		break;
	}
	}
	debugC(5, kDebugLoading, "Sound::readChunk(): Finished reading audio chunk (@0x%llx)", static_cast<long long int>(chunk.pos()));
}

void Sound::readSubfile(Subfile &subfile, Chunk &chunk) {
	//if (_streams.size() != 0) {
	//    warning("Sound::readSubfile(): Some audio has already been read.");
	//}
	uint32 totalChunks = _header->_chunkCount;
	uint32 expectedChunkId = chunk.id;

	readChunk(chunk);
	for (uint i = 0; i < totalChunks; i++) {
		chunk = subfile.nextChunk();
		if (chunk.id != expectedChunkId) {
			// TODO: Make this show the chunk IDs as strings, not numbers.
			error("Sound::readSubfile(): Expected chunk %s, got %s", tag2str(expectedChunkId), tag2str(chunk.id));
		}
		readChunk(chunk);
	}
}

}
