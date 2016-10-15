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

#include "common/events.h"
#include "common/system.h"
#include "engines/engine.h"
#include "graphics/palette.h"
#include "video/flic_decoder.h"

#include "chewy/sound.h"
#include "chewy/video/cfo_decoder.h"

namespace Chewy {

enum CustomSubChunk {
	kChunkFadeIn = 0,				// unused
	kChunkFadeOut = 1,
	kChunkLoadMusic = 2,
	kChunkLoadRaw = 3,				// unused
	kChunkLoadVoc = 4,
	kChunkPlayMusic = 5,
	kChunkPlaySeq = 6,				// unused
	kChunkPlayPattern = 7,			// unused
	kChunkStopMusic = 8,
	kChunkWaitMusicEnd = 9,
	kChunkSetMusicVolume = 10,
	kChunkSetLoopMode = 11,			// unused
	kChunkPlayRaw = 12,				// unused
	kChunkPlayVoc = 13,
	kChunkSetSoundVolume = 14,
	kChunkSetChannelVolume = 15,
	kChunkFreeSoundEffect = 16,
	kChunkMusicFadeIn = 17,			// unused
	kChunkMusicFadeOut = 18,
	kChunkSetBalance = 19,
	kChunkSetSpeed = 20,			// unused
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

	addTrack(new CfoVideoTrack(stream, frameCount, width, height, _sound));
	return true;
}

CfoDecoder::CfoVideoTrack::CfoVideoTrack(Common::SeekableReadStream *stream, uint16 frameCount, uint16 width, uint16 height, Sound *sound) :
	Video::FlicDecoder::FlicVideoTrack(stream, frameCount, width, height, true), _sound(sound) {
	readHeader();

	for (int i = 0; i < MAX_SOUND_EFFECTS; i++) {
		_soundEffects[i] = nullptr;
		_soundEffectSize[i] = 0;
	}

	_musicData = nullptr;
	_musicSize = 0;
}

CfoDecoder::CfoVideoTrack::~CfoVideoTrack() {
	_sound->stopAll();

	for (int i = 0; i < MAX_SOUND_EFFECTS; i++) {
		delete[] _soundEffects[i];
	}

	delete[] _musicData;
}

void CfoDecoder::CfoVideoTrack::readHeader() {
	_frameDelay = _startFrameDelay = _fileStream->readUint32LE();
	_offsetFrame1 = _fileStream->readUint32LE();
	_offsetFrame2 = 0;	// doesn't exist, as CFO videos aren't rewindable

	_fileStream->seek(_offsetFrame1);
}

#define FRAME_TYPE 0xF1FA
#define CUSTOM_FRAME_TYPE 0xFAF1

const ::Graphics::Surface *CfoDecoder::CfoVideoTrack::decodeNextFrame() {
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
			error("CfoDecoder::decodeNextFrame(): unknown subchunk type (type = 0x%02X)", frameType);
			break;
		}

		delete[] data;
	}
}

void CfoDecoder::CfoVideoTrack::handleCustomFrame() {
	uint16 chunkCount = _fileStream->readUint16LE();

	uint16 number, channel, volume, repeat, balance;

	// Read subchunks
	for (uint32 i = 0; i < chunkCount; ++i) {
		uint32 frameSize = _fileStream->readUint32LE();
		uint16 frameType = _fileStream->readUint16LE();

		switch (frameType) {
		case kChunkFadeIn:
			error("Unused chunk kChunkFadeIn found");
			break;
		case kChunkFadeOut:
			// Used in video 0
			_fileStream->skip(2);	// delay, unused
			fadeOut();
			break;
		case kChunkLoadMusic:
			// Used in videos 0, 18, 34, 71
			_musicSize = frameSize;
			_musicData = new byte[frameSize];
			_fileStream->read(_musicData, frameSize);
			break;
		case kChunkLoadRaw:
			error("Unused chunk kChunkLoadRaw found");
			break;
		case kChunkLoadVoc:
			number = _fileStream->readUint16LE();
			assert(number < MAX_SOUND_EFFECTS);
			delete[] _soundEffects[number];

			_soundEffectSize[number] = frameSize - 2;
			_soundEffects[number] = new byte[frameSize - 2];
			_fileStream->read(_soundEffects[number], frameSize - 2);
			break;
		case kChunkPlayMusic:
			// Used in videos 0, 18, 34, 71
			_sound->playMusic(_musicData, _musicSize, false, DisposeAfterUse::NO);
			break;
		case kChunkPlaySeq:
			error("Unused chunk kChunkPlaySeq found");
			break;
		case kChunkPlayPattern:
			error("Unused chunk kChunkPlayPattern found");
			break;
		case kChunkStopMusic:
			_sound->stopMusic();

			// Game videos do not restart music after stopping it
			delete[] _musicData;
			_musicSize = 0;
			break;
		case kChunkWaitMusicEnd:
			do {
				Common::Event event;
				while (g_system->getEventManager()->pollEvent(event)) {}	// ignore events
				g_system->updateScreen();
				g_system->delayMillis(10);
			} while (_sound->isMusicActive());
			break;
		case kChunkSetMusicVolume:
			volume = _fileStream->readUint16LE() * Audio::Mixer::kMaxChannelVolume / 63;
			_sound->setMusicVolume(volume);
			break;
		case kChunkSetLoopMode:
			error("Unused chunk kChunkSetLoopMode found");
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

			_sound->setSoundVolume(volume);
			_sound->playSound(_soundEffects[number], _soundEffectSize[number], repeat, channel, DisposeAfterUse::NO);
			break;
		case kChunkSetSoundVolume:
			volume = _fileStream->readUint16LE() * Audio::Mixer::kMaxChannelVolume / 63;
			_sound->setSoundVolume(volume);
			break;
		case kChunkSetChannelVolume:
			channel = _fileStream->readUint16LE();
			volume = _fileStream->readUint16LE() * Audio::Mixer::kMaxChannelVolume / 63;

			_sound->setSoundChannelVolume(channel, volume);
			break;
		case kChunkFreeSoundEffect:
			number = _fileStream->readUint16LE();
			assert(number < MAX_SOUND_EFFECTS);

			delete[] _soundEffects[number];
			_soundEffects[number] = nullptr;
			break;
		case kChunkMusicFadeIn:
			error("Unused chunk kChunkMusicFadeIn found");
			break;
		case kChunkMusicFadeOut:
			// Used in videos 0, 71
			warning("kChunkMusicFadeOut");
			// TODO
			_fileStream->skip(frameSize);
			break;
		case kChunkSetBalance:
			channel = _fileStream->readUint16LE();
			balance = (_fileStream->readUint16LE() * 2) - 127;
			_sound->setSoundChannelBalance(channel, balance);
			break;
		case kChunkSetSpeed:
			error("Unused chunk kChunkSetSpeed found");
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

void CfoDecoder::CfoVideoTrack::fadeOut() {
	for (int j = 0; j < 64; j++) {
		for (int i = 0; i < 256; i++) {
			if (_palette[i * 3 + 0] > 0)
				--_palette[i * 3 + 0];
			if (_palette[i * 3 + 1] > 0)
				--_palette[i * 3 + 1];
			if (_palette[i * 3 + 2] > 0)
				--_palette[i * 3 + 2];
		}

		g_system->getPaletteManager()->setPalette(_palette, 0, 256);
		g_system->updateScreen();
		g_system->delayMillis(10);
	}
}

} // End of namespace Chewy
