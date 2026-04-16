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
#include "graphics/surface.h"
#include "harvester/harvester.h"
#include "harvester/resources.h"
#include "image/pcx.h"

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
	uint16 bitsPerSample = 0;
	uint16 unknown2 = 0;
};

struct FstFrameEntry {
	uint32 videoSize = 0;
	uint16 audioSize = 0;
};

struct FstCensorshipToggleEntry {
	const char *sequenceName;
	int32 toggleFrameIndices[6];
};

struct FstOverlayFrame {
	uint16 width = 0;
	uint16 height = 0;
	Common::Array<byte> pixels;
	Common::Array<byte> palette;
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
static const uint32 kFstAltMagic = MKTAG('2', 'T', 'S', 'F');
static const FstCensorshipToggleEntry kFstCensorshipToggleEntries[] = {
	{ "C001B.FST", { 170, 369, -1, -1, -1, -1 } },
	{ "C004.FST", { 23, 174, 860, 995, 1834, -1 } },
	{ "C005.FST", { 475, 550, -1, -1, -1, -1 } },
	{ "C006.FST", { 699, 851, -1, -1, -1, -1 } },
	{ "C028.FST", { 0, -1, -1, -1, -1, -1 } },
	{ "C057.FST", { 0, -1, -1, -1, -1, -1 } },
	{ "C086.FST", { 48, 65, -1, -1, -1, -1 } },
	{ "C109.FST", { 44, -1, -1, -1, -1, -1 } },
	{ "C125.FST", { 7, -1, -1, -1, -1, -1 } },
	{ "C136.FST", { 128, -1, -1, -1, -1, -1 } },
	{ "CLOKBLST.FST", { 0, -1, -1, -1, -1, -1 } },
	{ "HEADTRAP.FST", { 0, -1, -1, -1, -1, -1 } },
	{ "PCPLANT.FST", { 20, -1, -1, -1, -1, -1 } },
	{ "C072.FST", { 0, -1, -1, -1, -1, -1 } },
	{ "C073.FST", { 30, -1, -1, -1, -1, -1 } },
	{ "C145A.FST", { 9, -1, -1, -1, -1, -1 } },
	{ "C145B.FST", { 9, -1, -1, -1, -1, -1 } },
	{ "C119.FST", { 0, -1, -1, -1, -1, -1 } },
	{ "C119A.FST", { 0, -1, -1, -1, -1, -1 } },
	{ "C119B.FST", { 0, -1, -1, -1, -1, -1 } },
	{ "C119C.FST", { 0, -1, -1, -1, -1, -1 } },
	{ "C117.FST", { 0, -1, -1, -1, -1, -1 } },
	{ "C065.FST", { 0, -1, -1, -1, -1, -1 } },
	{ "C010.FST", { 47, 109, 169, -1, -1, -1 } },
	{ "RANGSHOT.FST", { 0, -1, -1, -1, -1, -1 } }
};

static byte expand6BitColor(byte value) {
	return (value * 255 + 31) / 63;
}

static void convertVgaPalette(const byte *source, byte *dest) {
	for (uint i = 0; i < 256 * 3; ++i)
		dest[i] = expand6BitColor(source[i]);
}

static bool isValidFstMagic(uint32 magic) {
	// The published format docs describe the header as "2TSF", while the
	// recovered Harvester samples and native notes use the little-endian FST2 tag.
	return magic == kFstMagic || magic == kFstAltMagic;
}

static uint32 getTrailingFstAudioFrameSkip(const Common::Array<FstFrameEntry> &frames) {
	if (frames.size() < 2)
		return 0;

	const uint16 firstFrameAudioSize = frames[0].audioSize;
	const uint16 steadyStateAudioSize = frames[1].audioSize;
	if (firstFrameAudioSize == 0 || steadyStateAudioSize == 0 || firstFrameAudioSize <= steadyStateAudioSize)
		return 0;
	if ((firstFrameAudioSize % steadyStateAudioSize) != 0)
		return 0;

	const uint32 skipFrameCount = firstFrameAudioSize / steadyStateAudioSize - 1;
	if (skipFrameCount == 0 || skipFrameCount >= frames.size())
		return 0;

	// FutureVision docs note that some files front-load multiple frames of PCM
	// into frame 0 and expect the corresponding tail chunks to be ignored.
	for (uint32 i = frames.size() - skipFrameCount; i < frames.size(); ++i) {
		if (frames[i].audioSize != steadyStateAudioSize)
			return 0;
	}

	return skipFrameCount;
}

static const FstCensorshipToggleEntry *findCensorshipToggleEntry(const Common::String &path) {
	const Common::String basename = Common::lastPathComponent(normalizeHarvesterResourcePath(path), '/');

	for (const FstCensorshipToggleEntry &entry : kFstCensorshipToggleEntries) {
		if (basename.equalsIgnoreCase(entry.sequenceName))
			return &entry;
	}

	return nullptr;
}

static bool loadIndexedPcx(ResourceManager &resources, const Common::String &path, FstOverlayFrame &overlay) {
	Common::ScopedPtr<Common::SeekableReadStream> stream(resources.openFile(path));
	if (!stream)
		return false;

	Image::PCXDecoder decoder;
	if (!decoder.loadStream(*stream))
		return false;

	const Graphics::Surface *surface = decoder.getSurface();
	if (!surface || surface->format.bytesPerPixel != 1)
		return false;

	const Graphics::Palette &palette = decoder.getPalette();
	if (palette.size() != 256)
		return false;

	overlay.width = surface->w;
	overlay.height = surface->h;
	overlay.pixels.resize((uint32)overlay.width * overlay.height);
	for (uint16 y = 0; y < overlay.height; ++y)
		memcpy(overlay.pixels.data() + y * overlay.width, (const byte *)surface->getBasePtr(0, y), overlay.width);

	overlay.palette.resize(256 * 3);
	memcpy(overlay.palette.data(), palette.data(), overlay.palette.size());
	return true;
}

static void blitIndexedFrame(const FstOverlayFrame &overlay) {
	if (overlay.palette.size() != 256 * 3 || overlay.pixels.empty())
		return;

	g_system->getPaletteManager()->setPalette(overlay.palette.data(), 0, 256);
	g_system->copyRectToScreen(overlay.pixels.data(), overlay.width, 0, 0, overlay.width, overlay.height);
	g_system->updateScreen();
}

static bool extractFramePalette(const byte *frameData, uint32 frameSize, Common::Array<byte> &palette, bool &hasPalette) {
	hasPalette = false;
	if (!frameData || frameSize < 2)
		return false;

	const uint16 bitCount = READ_LE_UINT16(frameData);
	const uint32 bitstreamSize = (bitCount >> 3) + 1;
	if (frameSize < 2 + bitstreamSize)
		return false;

	FstBitReader bitReader(frameData + 2, bitCount);
	if (!bitReader.readBit())
		return true;

	const uint32 paletteOffset = 2 + bitstreamSize;
	if (frameSize < paletteOffset + 256 * 3)
		return false;

	palette.resize(256 * 3);
	convertVgaPalette(frameData + paletteOffset, palette.data());
	hasPalette = true;
	return true;
}

static bool clearPendingVideoInput(HarvesterEngine &vm) {
	Common::Event event;
	while (g_system->getEventManager()->pollEvent(event)) {
		switch (event.type) {
		case Common::EVENT_QUIT:
		case Common::EVENT_RETURN_TO_LAUNCHER:
			return false;
		default:
			break;
		}
	}

	return !vm.shouldQuit();
}

static bool pumpVideoEvents(HarvesterEngine &vm, bool &skipRequested, bool &deferredSkipRequested, bool allowSkipInput) {
	Common::Event event;
	while (g_system->getEventManager()->pollEvent(event)) {
		switch (event.type) {
		case Common::EVENT_QUIT:
		case Common::EVENT_RETURN_TO_LAUNCHER:
			return false;
		case Common::EVENT_KEYDOWN:
		case Common::EVENT_LBUTTONDOWN:
		case Common::EVENT_RBUTTONDOWN:
			if (allowSkipInput) {
				skipRequested = true;
			} else {
				deferredSkipRequested = true;
			}
			break;
		default:
			break;
		}
	}

	return !vm.shouldQuit();
}

static bool waitForAudioSync(HarvesterEngine &vm, Audio::Mixer &mixer, Audio::SoundHandle audioHandle,
		uint64 targetPlaybackBytes, uint32 audioByteRate, bool &skipRequested,
		bool &deferredSkipRequested, bool allowSkipInput) {
	if (audioByteRate == 0)
		return true;

	const uint32 targetPlaybackMs = (uint32)((targetPlaybackBytes * 1000ULL) / audioByteRate);
	while (!skipRequested) {
		if (!pumpVideoEvents(vm, skipRequested, deferredSkipRequested, allowSkipInput))
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

	for (int y = 0; y < 4; ++y) {
		byte *row = dest + y * pitch;
		const byte rowMask = (y < 2) ? (byte)(mask >> 8) : (byte)mask;
		const byte firstBit = (y & 1) ? 0x08 : 0x80;
		for (int x = 0; x < 4; ++x) {
			const byte bit = firstBit >> x;
			row[x] = (rowMask & bit) ? color1 : color0;
		}
	}
}

static bool decodeFrame(const FstHeader &header, const byte *frameData, uint32 frameSize, Common::Array<byte> &pixels, byte *palette) {
	if (!frameData || frameSize < 2)
		return false;

	const uint16 bitCount = READ_LE_UINT16(frameData);
	const uint32 bitstreamSize = (bitCount >> 3) + 1;
	if (frameSize < 2 + bitstreamSize)
		return false;

	FstBitReader bitReader(frameData + 2, bitCount);
	uint32 payloadOffset = 2 + bitstreamSize;

	if (bitReader.readBit()) {
		if (frameSize < payloadOffset + 256 * 3)
			return false;

		convertVgaPalette(frameData + payloadOffset, palette);
		payloadOffset += 256 * 3;
	}

	const int width = (int)header.width;
	const int height = (int)header.height;
	const int blocksX = width / 4;
	// Native playback leaves the last four scanlines unchanged.
	const int blocksY = MAX(0, height / 4 - 1);
	bool truncatedTail = false;

	for (int blockY = 0; blockY < blocksY; ++blockY) {
		for (int blockX = 0; blockX < blocksX; ++blockX) {
			if (!bitReader.readBit())
				continue;

			const uint32 blockOffset = blockY * 4 * width + blockX * 4;
			byte *dest = pixels.data() + blockOffset;

			if (bitReader.readBit()) {
				// Some shipped FST frames stop before the full 320x200 block grid.
				// Treat the missing tail blocks as unchanged instead of reading into PCM data.
				if (frameSize < payloadOffset + 4) {
					truncatedTail = true;
					break;
				}

				decodeMaskBlock(dest, width, frameData + payloadOffset);
				payloadOffset += 4;
			} else {
				if (frameSize < payloadOffset + 16) {
					truncatedTail = true;
					break;
				}

				for (int y = 0; y < 4; ++y)
					memcpy(dest + y * width, frameData + payloadOffset + y * 4, 4);
				payloadOffset += 16;
			}
		}

		if (truncatedTail)
			break;
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
	header.bitsPerSample = stream->readUint16LE();
	header.unknown2 = stream->readUint16LE();

	if (!isValidFstMagic(header.magic) || header.width == 0 || header.height == 0 || header.frameCount == 0 || header.frameRate == 0) {
		warning("Harvester: invalid FST header in '%s'", path.c_str());
		return false;
	}

	Common::Array<FstFrameEntry> frames;
	frames.resize(header.frameCount);
	uint32 maxTotalFrameSize = 0;
	for (uint32 i = 0; i < header.frameCount; ++i) {
		frames[i].videoSize = stream->readUint32LE();
		frames[i].audioSize = stream->readUint16LE();
		maxTotalFrameSize = MAX(maxTotalFrameSize, frames[i].videoSize + (uint32)frames[i].audioSize);
	}

	Common::Array<byte> pixels;
	pixels.resize(header.width * header.height);
	Common::fill(pixels.begin(), pixels.end(), 0);

	// The original player keeps one payload buffer for the full sequence instead of
	// reallocating per frame during playback.
	Common::Array<byte> frameData;
	frameData.resize(maxTotalFrameSize);

	byte palette[256 * 3] = { 0 };
	g_system->getPaletteManager()->setPalette(palette, 0, 256);

	if (!clearPendingVideoInput(_vm))
		return false;

	const bool shouldSwitchToMovieDisplay =
		header.width == 320 && header.height == 200 &&
		(_vm.getDisplayWidth() != 320 || _vm.getDisplayHeight() != 200);
	const int previousDisplayWidth = _vm.getDisplayWidth();
	const int previousDisplayHeight = _vm.getDisplayHeight();
	if (shouldSwitchToMovieDisplay)
		_vm.setDisplayMode(320, 200);

	const bool resumeMusicAfterPlayback = _vm.isMusicPlaying();
	if (resumeMusicAfterPlayback)
		_vm.pauseMusic(true);

	const bool censorshipEnabled = !_vm.isGoreEnabled();
	const FstCensorshipToggleEntry *censorshipEntry = censorshipEnabled ? findCensorshipToggleEntry(path) : nullptr;
	FstOverlayFrame censorshipOverlay;
	if (censorshipEntry && !loadIndexedPcx(*_vm.getResources(), "GRAPHIC/OTHER/CENSORED.PCX", censorshipOverlay)) {
		warning("Harvester: unable to load censorship overlay for '%s'", path.c_str());
		censorshipEntry = nullptr;
	}
	Common::Array<byte> savedMoviePalette;
	bool haveSavedMoviePalette = false;
	bool censorshipActive = false;
	uint nextCensorshipToggleIndex = 0;

	Audio::SoundHandle audioHandle;
	Audio::QueuingAudioStream *audioStream = nullptr;
	const bool hasPcmAudio = header.sampleRate != 0 && header.bitsPerSample == 16;
	const uint32 trailingAudioFrameSkip = hasPcmAudio ? getTrailingFstAudioFrameSkip(frames) : 0;
	if (hasPcmAudio)
		audioStream = Audio::makeQueuingAudioStream(header.sampleRate, false);
	else if (header.sampleRate != 0 && header.bitsPerSample != 0)
		warning("Harvester: unsupported FST audio format %u-bit in '%s'", header.bitsPerSample, path.c_str());

	bool audioStarted = false;
	bool skipRequested = false;
	bool deferredSkipRequested = false;
	uint32 nextFrameTime = g_system->getMillis();
	const uint32 audioByteRate = hasPcmAudio ? header.sampleRate * (header.bitsPerSample / 8) : 0;
	const uint32 audioBytesPerFrame = (audioByteRate != 0 && header.frameRate != 0) ? audioByteRate / header.frameRate : 0;
	uint64 nextAudioSyncBytes = 0;
	Audio::Mixer *mixer = g_system->getMixer();

	for (uint32 frameIndex = 0; frameIndex < header.frameCount; ++frameIndex) {
		if (frameIndex != 0 && deferredSkipRequested)
			skipRequested = true;
		if (skipRequested)
			break;

		const bool allowSkipInput = frameIndex != 0;
		const uint32 totalFrameSize = frames[frameIndex].videoSize + frames[frameIndex].audioSize;
		if (stream->read(frameData.data(), totalFrameSize) != totalFrameSize) {
			warning("Harvester: short read while decoding '%s' frame %u", path.c_str(), frameIndex);
			break;
		}

		if (audioStream && frames[frameIndex].audioSize != 0 &&
				frameIndex + trailingAudioFrameSkip < header.frameCount) {
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

		if (audioStarted &&
				!waitForAudioSync(_vm, *mixer, audioHandle, nextAudioSyncBytes, audioByteRate,
					skipRequested, deferredSkipRequested, allowSkipInput))
			break;

		const byte *videoData = frameData.data();
		const uint32 videoSize = frames[frameIndex].videoSize;
		Common::Array<byte> framePalette;
		bool frameHasPalette = false;
		if (censorshipEntry && !extractFramePalette(videoData, videoSize, framePalette, frameHasPalette)) {
			warning("Harvester: unable to extract palette from '%s' frame %u", path.c_str(), frameIndex);
			break;
		}

		if (frameHasPalette) {
			savedMoviePalette = framePalette;
			haveSavedMoviePalette = true;
		}

		bool restoreSavedPalette = false;
		while (censorshipEntry && nextCensorshipToggleIndex < ARRAYSIZE(censorshipEntry->toggleFrameIndices) &&
				censorshipEntry->toggleFrameIndices[nextCensorshipToggleIndex] >= 0 &&
				(uint32)censorshipEntry->toggleFrameIndices[nextCensorshipToggleIndex] == frameIndex) {
			censorshipActive = !censorshipActive;
			if (censorshipActive) {
				blitIndexedFrame(censorshipOverlay);
			} else if (!frameHasPalette) {
				restoreSavedPalette = true;
			}

			++nextCensorshipToggleIndex;
		}

		if (restoreSavedPalette && haveSavedMoviePalette)
			memcpy(palette, savedMoviePalette.data(), sizeof(palette));

		if (censorshipActive) {
			if (audioStarted && audioBytesPerFrame != 0) {
				nextAudioSyncBytes += audioBytesPerFrame;
			} else {
				nextFrameTime += 1000 / header.frameRate;
				while (!skipRequested && g_system->getMillis() < nextFrameTime) {
					if (!pumpVideoEvents(_vm, skipRequested, deferredSkipRequested, allowSkipInput))
						break;
					g_system->delayMillis(1);
				}
			}

			if (!pumpVideoEvents(_vm, skipRequested, deferredSkipRequested, allowSkipInput) || skipRequested)
				break;
			continue;
		}

		if (!decodeFrame(header, videoData, videoSize, pixels, palette)) {
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
				if (!pumpVideoEvents(_vm, skipRequested, deferredSkipRequested, allowSkipInput))
					break;
				g_system->delayMillis(1);
			}
		}

		if (!pumpVideoEvents(_vm, skipRequested, deferredSkipRequested, allowSkipInput) || skipRequested)
			break;
	}

	if (audioStarted)
		mixer->stopHandle(audioHandle);
	delete audioStream;
	if (resumeMusicAfterPlayback)
		_vm.pauseMusic(false);
	if (shouldSwitchToMovieDisplay)
		_vm.setDisplayMode(previousDisplayWidth, previousDisplayHeight);

	return !_vm.shouldQuit();
}

} // End of namespace Harvester
