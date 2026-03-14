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

#include "harvester/fst_player.h"

#include "audio/audiostream.h"
#include "audio/decoders/raw.h"
#include "audio/mixer.h"
#include "common/debug.h"
#include "common/endian.h"
#include "common/events.h"
#include "common/ptr.h"
#include "common/stream.h"
#include "common/system.h"
#include "common/textconsole.h"
#include "graphics/paletteman.h"
#include "harvester/harvester.h"
#include "harvester/resources.h"

namespace Harvester {

namespace {

struct FstHeader {
	uint32 magic = 0;
	uint32 width = 0;
	uint32 height = 0;
	uint32 maxFrameSize = 0;
	uint32 frameCount = 0;
	uint32 frameRate = 0;
	uint32 sampleRate = 0;
	uint32 bitsPerSample = 0;
};

struct FstFrameEntry {
	uint32 videoSize = 0;
	uint16 audioSize = 0;
};

class FstBitReader {
public:
	FstBitReader(const byte *data, uint32 bitCount) : _data(data), _bitCount(bitCount) {}

	bool readBit() {
		if (!_data || _bitIndex >= _bitCount)
			return false;

		const bool value = (_data[_bitIndex >> 3] & (0x80 >> (_bitIndex & 7))) != 0;
		++_bitIndex;
		return value;
	}

private:
	const byte *_data = nullptr;
	uint32 _bitCount = 0;
	uint32 _bitIndex = 0;
};

static const uint32 kFstMagic = MKTAG('F', 'S', 'T', '2');

static byte expand6BitColor(byte value) {
	return (value * 255 + 31) / 63;
}

static void convertVgaPalette(const byte *source, byte *dest) {
	for (uint i = 0; i < 256 * 3; ++i)
		dest[i] = expand6BitColor(source[i]);
}

static bool pumpVideoEvents(HarvesterEngine &vm, bool &skipRequested) {
	Common::Event event;
	while (g_system->getEventManager()->pollEvent(event)) {
		switch (event.type) {
		case Common::EVENT_QUIT:
		case Common::EVENT_RETURN_TO_LAUNCHER:
			return false;
		case Common::EVENT_KEYDOWN:
		case Common::EVENT_LBUTTONDOWN:
		case Common::EVENT_RBUTTONDOWN:
			skipRequested = true;
			break;
		default:
			break;
		}
	}

	return !vm.shouldQuit();
}

static bool waitForAudioSync(HarvesterEngine &vm, Audio::Mixer &mixer, Audio::SoundHandle audioHandle,
		uint64 targetPlaybackBytes, uint32 audioByteRate, bool &skipRequested) {
	if (audioByteRate == 0)
		return true;

	const uint32 targetPlaybackMs = (uint32)((targetPlaybackBytes * 1000ULL) / audioByteRate);
	while (!skipRequested) {
		if (!pumpVideoEvents(vm, skipRequested))
			return false;
		if (skipRequested || !mixer.isSoundHandleActive(audioHandle))
			return true;
		if (mixer.getSoundElapsedTime(audioHandle) >= targetPlaybackMs)
			return true;

		g_system->delayMillis(1);
	}

	return true;
}

static void decodeMaskBlock(byte *dest, int pitch, const byte *source) {
	const byte color0 = source[0];
	const byte color1 = source[1];
	const uint16 mask = READ_LE_UINT16(source + 2);
	const byte maskRows[4] = {
		(byte)((mask >> 8) & 0xf0),
		(byte)((mask >> 8) & 0x0f),
		(byte)(mask & 0xf0),
		(byte)(mask & 0x0f)
	};

	for (int y = 0; y < 4; ++y) {
		byte *row = dest + y * pitch;
		const byte rowMask = maskRows[y];
		for (int x = 0; x < 4; ++x) {
			const byte bit = (y & 1) ? (1 << x) : (0x10 << x);
			row[x] = (rowMask & bit) ? color1 : color0;
		}
	}
}

static bool decodeFrame(const FstHeader &header, const Common::Array<byte> &frameData, Common::Array<byte> &pixels, byte *palette) {
	if (frameData.size() < 2)
		return false;

	const uint16 bitCount = READ_LE_UINT16(frameData.data());
	const uint32 bitstreamSize = (bitCount >> 3) + 1;
	if (frameData.size() < 2 + bitstreamSize)
		return false;

	FstBitReader bitReader(frameData.data() + 2, bitCount);
	uint32 payloadOffset = 2 + bitstreamSize;

	if (bitReader.readBit()) {
		if (frameData.size() < payloadOffset + 256 * 3)
			return false;

		convertVgaPalette(frameData.data() + payloadOffset, palette);
		payloadOffset += 256 * 3;
	}

	const int width = (int)header.width;
	const int height = (int)header.height;
	const int blocksX = width / 4;
	const int blocksY = height / 4;

	for (int blockY = 0; blockY < blocksY; ++blockY) {
		for (int blockX = 0; blockX < blocksX; ++blockX) {
			if (!bitReader.readBit())
				continue;

			const uint32 blockOffset = blockY * 4 * width + blockX * 4;
			byte *dest = pixels.data() + blockOffset;

			if (bitReader.readBit()) {
				if (frameData.size() < payloadOffset + 4)
					return false;

				decodeMaskBlock(dest, width, frameData.data() + payloadOffset);
				payloadOffset += 4;
			} else {
				if (frameData.size() < payloadOffset + 16)
					return false;

				for (int y = 0; y < 4; ++y)
					memcpy(dest + y * width, frameData.data() + payloadOffset + y * 4, 4);
				payloadOffset += 16;
			}
		}
	}

	return true;
}

} // End of anonymous namespace

FstPlayer::FstPlayer(HarvesterEngine &vm) : _vm(vm) {}

bool FstPlayer::play(const Common::String &path) {
	Common::ScopedPtr<Common::SeekableReadStream> stream(_vm.getResources()->openFile(path));
	if (!stream) {
		warning("Harvester: unable to open FST '%s'", path.c_str());
		return false;
	}

	FstHeader header;
	header.magic = stream->readUint32LE();
	header.width = stream->readUint32LE();
	header.height = stream->readUint32LE();
	header.maxFrameSize = stream->readUint32LE();
	header.frameCount = stream->readUint32LE();
	header.frameRate = stream->readUint32LE();
	header.sampleRate = stream->readUint32LE();
	header.bitsPerSample = stream->readUint32LE();

	if (header.magic != kFstMagic || header.width == 0 || header.height == 0 || header.frameCount == 0 || header.frameRate == 0) {
		warning("Harvester: invalid FST header in '%s'", path.c_str());
		return false;
	}

	Common::Array<FstFrameEntry> frames;
	frames.resize(header.frameCount);
	for (uint32 i = 0; i < header.frameCount; ++i) {
		frames[i].videoSize = stream->readUint32LE();
		frames[i].audioSize = stream->readUint16LE();
	}

	Common::Array<byte> pixels;
	pixels.resize(header.width * header.height);
	Common::fill(pixels.begin(), pixels.end(), 0);

	byte palette[256 * 3] = { 0 };
	g_system->getPaletteManager()->setPalette(palette, 0, 256);

	Audio::SoundHandle audioHandle;
	Audio::QueuingAudioStream *audioStream = nullptr;
	const bool hasPcmAudio = header.sampleRate != 0 && header.bitsPerSample == 16;
	if (hasPcmAudio)
		audioStream = Audio::makeQueuingAudioStream(header.sampleRate, false);

	bool audioStarted = false;
	bool skipRequested = false;
	uint32 nextFrameTime = g_system->getMillis();
	const uint32 audioByteRate = hasPcmAudio ? header.sampleRate * 2 : 0;
	const uint32 audioBytesPerFrame = (audioByteRate != 0 && header.frameRate != 0) ? audioByteRate / header.frameRate : 0;
	uint64 nextAudioSyncBytes = 0;
	Audio::Mixer *mixer = g_system->getMixer();

	for (uint32 frameIndex = 0; frameIndex < header.frameCount; ++frameIndex) {
		const uint32 totalFrameSize = frames[frameIndex].videoSize + frames[frameIndex].audioSize;
		Common::Array<byte> frameData;
		frameData.resize(totalFrameSize);
		if (stream->read(frameData.data(), totalFrameSize) != totalFrameSize) {
			warning("Harvester: short read while decoding '%s' frame %u", path.c_str(), frameIndex);
			break;
		}

		if (audioStream && frames[frameIndex].audioSize != 0) {
			byte *audioBuffer = (byte *)malloc(frames[frameIndex].audioSize);
			if (!audioBuffer)
				error("Harvester: unable to allocate audio buffer");

			memcpy(audioBuffer, frameData.data() + frames[frameIndex].videoSize, frames[frameIndex].audioSize);
			audioStream->queueBuffer(audioBuffer, frames[frameIndex].audioSize, DisposeAfterUse::YES,
				Audio::FLAG_16BITS | Audio::FLAG_LITTLE_ENDIAN);

			if (!audioStarted) {
				mixer->playStream(Audio::Mixer::kSFXSoundType, &audioHandle, audioStream, -1,
					Audio::Mixer::kMaxChannelVolume, 0, DisposeAfterUse::NO);
				audioStarted = true;
			}
		}

		if (audioStarted && !waitForAudioSync(_vm, *mixer, audioHandle, nextAudioSyncBytes, audioByteRate, skipRequested))
			break;

		if (!decodeFrame(header, frameData, pixels, palette)) {
			warning("Harvester: unable to decode '%s' frame %u", path.c_str(), frameIndex);
			break;
		}

		g_system->getPaletteManager()->setPalette(palette, 0, 256);
		g_system->copyRectToScreen(pixels.data(), header.width, 0, 0, header.width, header.height);
		g_system->updateScreen();

		if (audioStarted && audioBytesPerFrame != 0) {
			nextAudioSyncBytes += audioBytesPerFrame;
		} else {
			nextFrameTime += 1000 / header.frameRate;
			while (!skipRequested && g_system->getMillis() < nextFrameTime) {
				if (!pumpVideoEvents(_vm, skipRequested))
					break;
				g_system->delayMillis(1);
			}
		}

		if (!pumpVideoEvents(_vm, skipRequested) || skipRequested)
			break;
	}

	if (audioStarted)
		mixer->stopHandle(audioHandle);
	delete audioStream;

	return !_vm.shouldQuit();
}

} // End of namespace Harvester
