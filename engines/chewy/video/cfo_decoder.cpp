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

#include "common/stream.h"
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
	kChunkSetStero = 19,
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

	// Read subchunks
	for (uint32 i = 0; i < chunkCount; ++i) {
		uint32 frameSize = _fileStream->readUint32LE();
		uint16 frameType = _fileStream->readUint16LE();
		uint8 *data = new uint8[frameSize];
		_fileStream->read(data, frameSize);

		switch (frameType) {
		case kChunkFadeIn:
			warning("kChunkFadeIn");
			// TODO
			break;
		case kChunkFadeOut:
			warning("kChunkFadeOut");
			// TODO
			break;
		case kChunkLoadMusic:
			warning("kChunkLoadMusic");
			// TODO
			break;
		case kChunkLoadRaw:
			warning("kChunkLoadRaw");
			// TODO
			break;
		case kChunkLoadVoc:
			warning("kChunkLoadVoc");
			// TODO
			break;
		case kChunkPlayMusic:
			warning("kChunkPlayMusic");
			break;
		case kChunkPlaySeq:
			warning("kChunkPlaySeq");
			// TODO
			break;
		case kChunkPlayPattern:
			warning("kChunkPlayPattern");
			// TODO
			break;
		case kChunkStopMusic:
			warning("kChunkStopMusic");
			// TODO
			break;
		case kChunkWaitMusicEnd:
			warning("kChunkWaitMusicEnd");
			// TODO
			break;
		case kChunkSetMusicVolume:
			warning("kChunkSetMusicVolume");
			// TODO
			break;
		case kChunkSetLoopMode:
			warning("kChunkSetLoopMode");
			// TODO
			break;
		case kChunkPlayRaw:
			warning("kChunkPlayRaw");
			// TODO
			break;
		case kChunkPlayVoc:
			warning("kChunkPlayVoc");
			// TODO
			break;
		case kChunkSetSoundVolume:
			warning("kChunkSetSoundVolume");
			// TODO
			break;
		case kChunkSetChannelVolume:
			warning("kChunkSetChannelVolume");
			// TODO
			break;
		case kChunkFreeSoundEffect:
			warning("kChunkFreeSoundEffect");
			// TODO
			break;
		case kChunkMusicFadeIn:
			warning("kChunkMusicFadeIn");
			// TODO
			break;
		case kChunkMusicFadeOut:
			warning("kChunkMusicFadeOut");
			// TODO
			break;
		case kChunkSetStero:
			warning("kChunkSetStero");
			// TODO
			break;
		case kChunkSetSpeed:
			warning("kChunkSetSpeed");
			// TODO
			break;
		case kChunkClearScreen:
			warning("kChunkClearScreen");
			// TODO
			break;
		default:
			error("Unknown subchunk: %d", frameType);
			break;
		}

		delete[] data;
	}
}

} // End of namespace Chewy
