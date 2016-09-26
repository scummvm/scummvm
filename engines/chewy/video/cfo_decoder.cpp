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

#include "audio/audiostream.h"
#include "audio/mixer.h"
#include "audio/decoders/raw.h"
#include "common/stream.h"
#include "common/system.h"
#include "engines/engine.h"
#include "video/flic_decoder.h"

#include "chewy/video/cfo_decoder.h"

namespace Chewy {

enum CustomSubChunk {
	kChunkFadeIn = 0,
	kChunkFadeOut = 1,
	kChunkLoadMusic = 2,
	kChunkLoadRaw = 3,
	kChunkLoadVoc = 4,
	kChunkPlayMusic = 5,
	kChunkPlaySeq = 6,
	kChunkPlayPattern = 7,
	kChunkStopMusic = 8,
	kChunkWaitMusicEnd = 9,
	kChunkSetMusicVolume = 10,
	kChunkSetLoopMode = 11,
	kChunkPlayRaw = 12,
	kChunkPlayVoc = 13,
	kChunkSetSoundVolume = 14,
	kChunkSetChannelVolume = 15,
	kChunkFreeSoundEffect = 16,
	kChunkMusicFadeIn = 17,
	kChunkMusicFadeOut = 18,
	kChunkSetBalance = 19,
	kChunkSetSpeed = 20,
	kChunkClearScreen = 21
};

bool CfoDecoder::loadStream(Common::SeekableReadStream *stream) {
	close();

	if (stream->readUint32BE() != MKTAG('C', 'F', 'O', '\0'))
		error("Corrupt video resource");

	stream->readUint32LE();	// always 0

	uint16 frameCount = stream->readUint16LE();
	uint16 width = stream->readUint16LE();
	uint16 height = stream->readUint16LE();

	addTrack(new CfoVideoTrack(stream, frameCount, width, height));
	return true;
}

CfoDecoder::CfoVideoTrack::CfoVideoTrack(Common::SeekableReadStream *stream, uint16 frameCount, uint16 width, uint16 height) :
	Video::FlicDecoder::FlicVideoTrack(stream, frameCount, width, height, true) {
	readHeader();

	for (int i = 0; i < MAX_SOUND_EFFECTS; i++) {
		_soundEffects[i] = nullptr;
		_soundEffectSize[i] = 0;
	}
}

CfoDecoder::CfoVideoTrack::~CfoVideoTrack() {
	g_engine->_mixer->stopAll();

	for (int i = 0; i < MAX_SOUND_EFFECTS; i++) {
		delete[] _soundEffects[i];
	}
}

void CfoDecoder::CfoVideoTrack::readHeader() {
	_frameDelay = _startFrameDelay = _fileStream->readUint32LE();
	_offsetFrame1 = _fileStream->readUint32LE();
	_offsetFrame2 = 0;	// doesn't exist, as CFO videos aren't rewindable

	_fileStream->seek(_offsetFrame1);
}

#define FRAME_TYPE 0xF1FA
#define CUSTOM_FRAME_TYPE 0xFAF1

const Graphics::Surface *CfoDecoder::CfoVideoTrack::decodeNextFrame() {
	uint16 frameType;

	// Read chunk
	/*uint32 frameSize =*/ _fileStream->readUint32LE();
	frameType = _fileStream->readUint16LE();

	switch (frameType) {
	case FRAME_TYPE:
		handleFrame();
		break;
	case CUSTOM_FRAME_TYPE:
		handleCustomFrame();
		break;
	default:
		error("CfoDecoder::decodeFrame(): unknown main chunk type (type = 0x%02X)", frameType);
		break;
	}

	_curFrame++;
	_nextFrameStartTime += _frameDelay;

	return _surface;
}

#define FLI_SETPAL 4
#define FLI_SS2    7
#define FLI_BRUN   15
#define FLI_COPY   16
#define PSTAMP     18

void CfoDecoder::CfoVideoTrack::handleFrame() {
	uint16 chunkCount = _fileStream->readUint16LE();

	// Read subchunks
	for (uint32 i = 0; i < chunkCount; ++i) {
		uint32 frameSize = _fileStream->readUint32LE();
		uint16 frameType = _fileStream->readUint16LE();
		uint8 *data = new uint8[frameSize - 6];
		_fileStream->read(data, frameSize - 6);

		switch (frameType) {
		case FLI_SETPAL:
			unpackPalette(data);
			_dirtyPalette = true;
			break;
		case FLI_SS2:
			decodeDeltaFLC(data);
			break;
		case FLI_BRUN:
			decodeByteRun(data);
			break;
		case FLI_COPY:
			copyFrame(data);
			break;
		case PSTAMP:
			/* PSTAMP - skip for now */
			break;
		default:
			error("FlicDecoder::decodeNextFrame(): unknown subchunk type (type = 0x%02X)", frameType);
			break;
		}

		delete[] data;
	}
}

void CfoDecoder::CfoVideoTrack::handleCustomFrame() {
	uint16 chunkCount = _fileStream->readUint16LE();

	uint16 delay, number, channel, volume, repeat, balance;
	Audio::AudioStream *stream;

	// Read subchunks
	for (uint32 i = 0; i < chunkCount; ++i) {
		uint32 frameSize = _fileStream->readUint32LE();
		uint16 frameType = _fileStream->readUint16LE();

		switch (frameType) {
		case kChunkFadeIn:
			delay = _fileStream->readUint16LE();

			warning("kChunkFadeIn, delay %d", delay);
			// TODO
			break;
		case kChunkFadeOut:
			delay = _fileStream->readUint16LE();

			warning("kChunkFadeOut, delay %d", delay);
			// TODO
			break;
		case kChunkLoadMusic:
			warning("kChunkLoadMusic");
			// TODO
			_fileStream->skip(frameSize);
			break;
		case kChunkLoadRaw:
			error("Unused chunk kChunkLoadRaw found");
			break;
		case kChunkLoadVoc:
			number = _fileStream->readUint16LE();
			assert(number < MAX_SOUND_EFFECTS);
			delete[] _soundEffects[number];

			_soundEffectSize[number] = frameSize - 2;
			_soundEffects[number] = (byte *)malloc(frameSize - 2);
			_fileStream->read(_soundEffects[number], frameSize - 2);
			break;
		case kChunkPlayMusic:
			warning("kChunkPlayMusic");
			// TODO
			_fileStream->skip(frameSize);
			break;
		case kChunkPlaySeq:
			warning("kChunkPlaySeq");
			// TODO
			_fileStream->skip(frameSize);
			break;
		case kChunkPlayPattern:
			warning("kChunkPlayPattern");
			// TODO
			_fileStream->skip(frameSize);
			break;
		case kChunkStopMusic:
			g_engine->_mixer->stopHandle(_musicHandle);
			break;
		case kChunkWaitMusicEnd:
			do {
				g_system->delayMillis(10);
			} while (g_engine->_mixer->isSoundHandleActive(_musicHandle));
			break;
		case kChunkSetMusicVolume:
			volume = _fileStream->readUint16LE() * Audio::Mixer::kMaxChannelVolume / 63;

			g_engine->_mixer->setVolumeForSoundType(Audio::Mixer::SoundType::kMusicSoundType, volume);
			break;
		case kChunkSetLoopMode:
			warning("kChunkSetLoopMode");
			// TODO
			_fileStream->skip(frameSize);
			break;
		case kChunkPlayRaw:
			error("Unused chunk kChunkPlayRaw found");
			break;
		case kChunkPlayVoc:
			number = _fileStream->readUint16LE();
			channel = _fileStream->readUint16LE();
			volume = _fileStream->readUint16LE() * Audio::Mixer::kMaxChannelVolume / 63;
			repeat = _fileStream->readUint16LE();
			assert(number < MAX_SOUND_EFFECTS);
			assert(channel < MAX_SOUND_EFFECTS);

			stream = Audio::makeLoopingAudioStream(
				Audio::makeRawStream(_soundEffects[number],
				_soundEffectSize[number], 22050, Audio::FLAG_UNSIGNED,
				DisposeAfterUse::NO),
				(repeat == 0) ? 1 : repeat);

			g_engine->_mixer->setVolumeForSoundType(Audio::Mixer::SoundType::kSFXSoundType, volume);
			g_engine->_mixer->playStream(Audio::Mixer::kSFXSoundType, &_soundHandle[channel], stream);
			break;
		case kChunkSetSoundVolume:
			volume = _fileStream->readUint16LE() * Audio::Mixer::kMaxChannelVolume / 63;

			g_engine->_mixer->setVolumeForSoundType(Audio::Mixer::SoundType::kSFXSoundType, volume);
			break;
		case kChunkSetChannelVolume:
			channel = _fileStream->readUint16LE();
			volume = _fileStream->readUint16LE() * Audio::Mixer::kMaxChannelVolume / 63;
			assert(channel < MAX_SOUND_EFFECTS);

			g_engine->_mixer->setChannelVolume(_soundHandle[channel], volume);
			break;
		case kChunkFreeSoundEffect:
			number = _fileStream->readUint16LE();
			assert(number < MAX_SOUND_EFFECTS);

			delete[] _soundEffects[number];
			_soundEffects[number] = nullptr;
			break;
		case kChunkMusicFadeIn:
			warning("kChunkMusicFadeIn");
			// TODO
			_fileStream->skip(frameSize);
			break;
		case kChunkMusicFadeOut:
			warning("kChunkMusicFadeOut");
			// TODO
			_fileStream->skip(frameSize);
			break;
		case kChunkSetBalance:
			channel = _fileStream->readUint16LE();
			balance = (_fileStream->readUint16LE() * 2) - 127;
			assert(channel < MAX_SOUND_EFFECTS);

			g_engine->_mixer->setChannelBalance(_soundHandle[channel], balance);
			break;
		case kChunkSetSpeed:
			warning("kChunkSetSpeed");
			// TODO
			_fileStream->skip(frameSize);
			break;
		case kChunkClearScreen:
			g_system->fillScreen(0);
			break;
		default:
			error("Unknown subchunk: %d", frameType);
			break;
		}
	}
}

} // End of namespace Chewy
