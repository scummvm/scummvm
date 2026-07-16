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
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program. If not, see <http://www.gnu.org/licenses/>.
 */

#include "ripper/media.h"

#include "audio/decoders/raw.h"
#include "audio/decoders/wave.h"
#include "audio/audiostream.h"
#include "common/array.h"
#include "common/debug.h"
#include "common/endian.h"
#include "common/file.h"
#include "common/hashmap.h"
#include "common/memstream.h"
#include "common/ptr.h"
#include "common/serializer.h"
#include "common/system.h"
#include "graphics/blit.h"
#include "graphics/paletteman.h"
#include "graphics/surface.h"
#include "video/smk_decoder.h"

#include "ripper/cursor.h"
#include "ripper/detection.h"
#include "ripper/input.h"
#include "ripper/resources.h"
#include "ripper/ripper.h"
#include "ripper/script.h"
#include "ripper/toolbar.h"

namespace Ripper {

namespace {

static const int kScenePresentationTop = 50;
static const uint kBlockingAudioCursor = 0x13;

struct SavedDisplayContext {
	Common::Array<byte> pixels;
	Common::Array<byte> palette;
	uint width;
	uint height;

	SavedDisplayContext() : width(0), height(0) {}
};

struct IavfDescriptor {
	uint16 opcode;
	uint32 arg0;
	uint32 arg1;
	uint32 arg2;
};

struct IavfSegment {
	Common::Array<byte> setup;
	Common::Array<uint32> frameSizes;
	Common::Array<Common::Array<byte> > framePayloads;
	Common::Array<uint32> frameAudioOffsets;
	uint32 expectedFrames;
	int x;
	int y;
	bool clearDisplayBefore;

	IavfSegment() : expectedFrames(0), x(-1), y(-1), clearDisplayBefore(false) {}
};

struct IavfMovie {
	uint32 declaredGateCount;
	uint16 sampleRate;
	byte channels;
	byte bitsPerSample;
	uint32 audioByteRate;
	Common::Array<byte> audio;
	uint32 audioPayloadBytes;
	Common::Array<IavfSegment> segments;
	uint presentationWidth;
	uint presentationHeight;
	uint displayScale;
	bool clearDisplayAfter;

	IavfMovie() : declaredGateCount(0), sampleRate(0), channels(0), bitsPerSample(0),
		audioByteRate(0), audioPayloadBytes(0),
		presentationWidth(0), presentationHeight(0), displayScale(1), clearDisplayAfter(false) {}
};

static bool readExact(Common::SeekableReadStream &stream, void *data, uint32 size) {
	return stream.read(data, size) == size;
}

static bool captureDisplayContext(SavedDisplayContext &context) {
	Graphics::Surface *screen = g_system->lockScreen();
	if (!screen || screen->format.bytesPerPixel != 1) {
		if (screen)
			g_system->unlockScreen();
		return false;
	}

	context.width = screen->w;
	context.height = screen->h;
	context.pixels.resize(context.width * context.height);
	for (uint y = 0; y < context.height; ++y)
		memcpy(context.pixels.data() + y * context.width, screen->getBasePtr(0, y), context.width);
	g_system->unlockScreen();

	context.palette.resize(256 * 3);
	g_system->getPaletteManager()->grabPalette(context.palette.data(), 0, 256);
	return true;
}

static bool restoreDisplayContext(const SavedDisplayContext &context) {
	Graphics::Surface *screen = g_system->lockScreen();
	if (!screen || screen->format.bytesPerPixel != 1 ||
		(uint)screen->w != context.width || (uint)screen->h != context.height) {
		if (screen)
			g_system->unlockScreen();
		return false;
	}

	for (uint y = 0; y < context.height; ++y)
		memcpy(screen->getBasePtr(0, y), context.pixels.data() + y * context.width, context.width);
	g_system->unlockScreen();
	g_system->getPaletteManager()->setPalette(context.palette.data(), 0, 256);
	g_system->updateScreen();
	return true;
}

static bool readBlob(Common::SeekableReadStream &stream, uint32 size, Common::Array<byte> &data) {
	if ((uint64)stream.pos() + size > (uint64)stream.size())
		return false;
	data.resize(size);
	return size == 0 || readExact(stream, data.data(), size);
}

static bool validateSmackerSetup(const Common::Array<byte> &setup, uint32 &frameCount) {
	if (setup.size() < 104 || READ_BE_UINT32(setup.data()) != MKTAG('S', 'M', 'K', '2'))
		return false;
	frameCount = READ_LE_UINT32(setup.data() + 12);
	const uint32 flags = READ_LE_UINT32(setup.data() + 20);
	const uint32 treesSize = READ_LE_UINT32(setup.data() + 52);
	if ((flags & 1) != 0)
		return false;
	return (uint64)104 + frameCount + treesSize == setup.size();
}

static bool readDescriptor(Common::SeekableReadStream &stream, IavfDescriptor &descriptor) {
	if ((uint64)stream.pos() + 14 > (uint64)stream.size())
		return false;
	descriptor.opcode = stream.readUint16LE();
	descriptor.arg0 = stream.readUint32LE();
	descriptor.arg1 = stream.readUint32LE();
	descriptor.arg2 = stream.readUint32LE();
	return true;
}

static bool parseIavf(Common::SeekableReadStream &stream, const Common::String &name, IavfMovie &movie) {
	if (stream.size() < 0x91 || (uint64)stream.size() > 0xffffffffULL)
		return false;

	Common::Array<byte> header;
	if (!readBlob(stream, 0x91, header) || memcmp(header.data(), "IAVF2.00", 8) != 0)
		return false;
	movie.declaredGateCount = READ_LE_UINT32(header.data() + 0x10);
	movie.sampleRate = READ_LE_UINT16(header.data() + 0x1c);
	movie.channels = header[0x1e];
	movie.bitsPerSample = header[0x1f];
	const uint32 headerByteRate = READ_LE_UINT32(header.data() + 0x20);
	const uint32 headerBlockAlign = READ_LE_UINT32(header.data() + 0x24);
	// The retail assets store height before width, contrary to the field order
	// previously documented for IAVF.
	movie.presentationHeight = READ_LE_UINT16(header.data() + 0x2f);
	movie.presentationWidth = READ_LE_UINT16(header.data() + 0x31);
	if (movie.channels != 1 || movie.bitsPerSample != 16 || movie.sampleRate == 0) {
		warning("Ripper: unsupported IAVF audio format rate=%u channels=%u bits=%u in '%s'",
			movie.sampleRate, movie.channels, movie.bitsPerSample, name.c_str());
		return false;
	}
	const uint32 expectedBlockAlign = movie.channels * movie.bitsPerSample / 8;
	movie.audioByteRate = movie.sampleRate * expectedBlockAlign;
	if (headerBlockAlign != expectedBlockAlign || headerByteRate != movie.audioByteRate) {
		warning("Ripper: inconsistent IAVF audio header rate=%u byteRate=%u blockAlign=%u in '%s'",
			movie.sampleRate, headerByteRate, headerBlockAlign, name.c_str());
	}
	if (movie.presentationWidth == 0 || movie.presentationHeight == 0)
		return false;
	movie.displayScale = movie.presentationHeight < 0xc9 && movie.presentationWidth < 0x141 ? 2 : 1;
	movie.audio.reserve((uint32)stream.size());

	Common::HashMap<uint32, Common::Array<byte> > setupCache;
	Common::HashMap<uint32, uint32> audioEndOffsets;
	IavfSegment *activeSegment = nullptr;
	Common::Array<byte> pendingFramePayload;
	bool reachedEnd = false;
	bool hasPendingAudioOffset = false;
	bool hasPendingFrame = false;
	bool pendingDisplayClear = false;
	uint32 descriptorIndex = 0;
	uint32 observedGateCount = 0;
	uint32 pendingAudioOffset = 0;
	uint32 pendingFrameAudioOffset = 0;
	while (stream.pos() < stream.size() && !reachedEnd) {
		IavfDescriptor descriptor;
		if (!readDescriptor(stream, descriptor))
			return false;
		debugC(11, kDebugVideo,
			"Ripper: IAVF '%s' descriptor=%u opcode=0x%02x arg0=%u arg1=%u arg2=%u offset=%lld",
			name.c_str(), descriptorIndex++, descriptor.opcode, descriptor.arg0, descriptor.arg1,
			descriptor.arg2, stream.pos() - 14);

		switch (descriptor.opcode) {
		case 0x66: {
			if (descriptor.arg2 > descriptor.arg1 ||
				(uint64)movie.audio.size() + descriptor.arg1 > 0xffffffffULL ||
				(uint64)movie.audioPayloadBytes + descriptor.arg2 > 0xffffffffULL)
				return false;
			const uint32 outputOffset = movie.audio.size();
			movie.audio.resize(outputOffset + descriptor.arg1);
			if (descriptor.arg2 != 0 && !readExact(stream, movie.audio.data() + outputOffset, descriptor.arg2))
				return false;
			if (descriptor.arg1 > descriptor.arg2) {
				memset(movie.audio.data() + outputOffset + descriptor.arg2, 0,
					descriptor.arg1 - descriptor.arg2);
				debugC(11, kDebugVideo,
					"Ripper: IAVF '%s' audio tag=%u payload=%u timeline=%u silence=%u",
					name.c_str(), descriptor.arg0, descriptor.arg2, descriptor.arg1,
					descriptor.arg1 - descriptor.arg2);
			}
			movie.audioPayloadBytes += descriptor.arg2;
			audioEndOffsets[descriptor.arg0] = movie.audio.size();
			break;
		}

		case 0x67: {
			Common::HashMap<uint32, uint32>::const_iterator audioOffset = audioEndOffsets.find(descriptor.arg0);
			if (audioOffset == audioEndOffsets.end() || audioOffset->_value != descriptor.arg1)
				return false;
			pendingAudioOffset = audioOffset->_value;
			hasPendingAudioOffset = true;
			++observedGateCount;
			break;
		}

		case 0x68:
			if (descriptor.arg0 != 0 || descriptor.arg1 != 0 || descriptor.arg2 != 0)
				return false;
			pendingDisplayClear = true;
			break;

		case 0x75:
			if (descriptor.arg0 != 0 || descriptor.arg1 != 0 || descriptor.arg2 != 0)
				return false;
			// RunPacketizedMediaPlaybackCore at 0x5b592 uses this command to
			// prebuffer the packet stream and arm managed-audio control. IAVF is
			// parsed eagerly here, so neither operation needs a ScummVM analogue.
			debugC(2, kDebugVideo, "Ripper: IAVF '%s' reached prebuffer command 0x75", name.c_str());
			break;

		case 0x78: {
			if (descriptor.arg0 == 0 || descriptor.arg1 == 0 || descriptor.arg2 != 0)
				return false;
			Common::Array<byte> setup;
			uint32 frameCount = 0;
			if (!readBlob(stream, descriptor.arg1, setup) || !validateSmackerSetup(setup, frameCount))
				return false;
			setupCache[descriptor.arg0] = setup;
			break;
		}

		case 0x6a: {
			if (hasPendingFrame ||
				(activeSegment && activeSegment->frameSizes.size() != activeSegment->expectedFrames))
				return false;
			int32 branch[5];
			for (uint i = 0; i < ARRAYSIZE(branch); ++i)
				branch[i] = stream.readSint32LE();
			if (stream.err())
				return false;

			IavfSegment segment;
			segment.x = branch[0];
			segment.y = branch[1];
			segment.clearDisplayBefore = pendingDisplayClear;
			pendingDisplayClear = false;
			if (descriptor.arg1 == 0) {
				if (!readBlob(stream, descriptor.arg0, segment.setup))
					return false;
			} else {
				Common::HashMap<uint32, Common::Array<byte> >::const_iterator cached = setupCache.find(descriptor.arg1);
				if (cached == setupCache.end() || cached->_value.size() != descriptor.arg0)
					return false;
				segment.setup = cached->_value;
			}
			if (!validateSmackerSetup(segment.setup, segment.expectedFrames))
				return false;
			movie.segments.push_back(Common::move(segment));
			activeSegment = &movie.segments.back();
			debugC(2, kDebugVideo,
				"Ripper: IAVF '%s' segment=%u frames=%u x=%d y=%d setup=%u",
				name.c_str(), movie.segments.size() - 1, activeSegment->expectedFrames,
				activeSegment->x, activeSegment->y, activeSegment->setup.size());
			break;
		}

		case 0x6c: {
			if (!activeSegment || descriptor.arg1 != 0 || descriptor.arg2 != 0 ||
				activeSegment->frameSizes.size() >= activeSegment->expectedFrames ||
				!hasPendingAudioOffset || hasPendingFrame)
				return false;
			if (!readBlob(stream, descriptor.arg0, pendingFramePayload))
				return false;
			pendingFrameAudioOffset = pendingAudioOffset;
			hasPendingAudioOffset = false;
			hasPendingFrame = true;
			break;
		}

		case 0x77:
			if (descriptor.arg0 != 0 || descriptor.arg1 != 0 || descriptor.arg2 != 0 ||
				!activeSegment || !hasPendingFrame ||
				activeSegment->frameSizes.size() >= activeSegment->expectedFrames)
				return false;
			// LoadCustomPacketPaletteStateBlock at 0x6c430 consumes opcode 0x6c,
			// while opcode 0x77 reaches RenderCustomPacketFrameAndOverlays at
			// 0x6c486. Commit the reconstructed Smacker frame at the render command
			// so packet loading and presentation retain their original boundary.
			activeSegment->frameSizes.push_back(pendingFramePayload.size());
			activeSegment->framePayloads.push_back(Common::move(pendingFramePayload));
			activeSegment->frameAudioOffsets.push_back(pendingFrameAudioOffset);
			hasPendingFrame = false;
			break;

		case 0x70:
			if (descriptor.arg0 != 0 || descriptor.arg1 != 0 || descriptor.arg2 != 0 || hasPendingFrame)
				return false;
			movie.clearDisplayAfter = pendingDisplayClear;
			reachedEnd = true;
			break;

		default:
			warning("Ripper: unsupported IAVF opcode 0x%02x at 0x%llx in '%s'",
				descriptor.opcode, stream.pos() - 14, name.c_str());
			return false;
		}
	}

	if (!reachedEnd || movie.segments.empty())
		return false;
	if (observedGateCount != movie.declaredGateCount) {
		warning("Ripper: IAVF '%s' declares %u playback gates but contains %u",
			name.c_str(), movie.declaredGateCount, observedGateCount);
	}
	for (uint i = 0; i < movie.segments.size(); ++i) {
		if (movie.segments[i].frameSizes.size() != movie.segments[i].expectedFrames ||
			movie.segments[i].frameAudioOffsets.size() != movie.segments[i].expectedFrames)
			return false;
		for (uint frame = 0; frame < movie.segments[i].frameAudioOffsets.size(); ++frame) {
			if (movie.segments[i].frameAudioOffsets[frame] > movie.audio.size())
				return false;
		}
	}
	debugC(1, kDebugVideo,
		"Ripper: parsed IAVF '%s' canvas=%ux%u scale=%u segments=%u gates=%u audioPayloadBytes=%u audioTimelineBytes=%u audioMs=%u",
		name.c_str(), movie.presentationWidth, movie.presentationHeight, movie.displayScale,
		movie.segments.size(), observedGateCount, movie.audioPayloadBytes, movie.audio.size(),
		(uint32)((uint64)movie.audio.size() * 1000 / movie.audioByteRate));
	return true;
}

static Common::SeekableReadStream *rebuildSmackerStream(const IavfSegment &segment) {
	uint64 totalSize = 104 + (uint64)segment.frameSizes.size() * 4 + segment.setup.size() - 104;
	for (uint i = 0; i < segment.framePayloads.size(); ++i)
		totalSize += segment.framePayloads[i].size();
	if (totalSize > 0xffffffffULL)
		return nullptr;

	byte *output = new byte[(uint32)totalSize];
	uint32 cursor = 0;
	memcpy(output + cursor, segment.setup.data(), 104);
	cursor += 104;
	for (uint i = 0; i < segment.frameSizes.size(); ++i) {
		WRITE_LE_UINT32(output + cursor, segment.frameSizes[i]);
		cursor += 4;
	}
	memcpy(output + cursor, segment.setup.data() + 104, segment.setup.size() - 104);
	cursor += segment.setup.size() - 104;
	for (uint i = 0; i < segment.framePayloads.size(); ++i) {
		memcpy(output + cursor, segment.framePayloads[i].data(), segment.framePayloads[i].size());
		cursor += segment.framePayloads[i].size();
	}
	return new Common::MemoryReadStream(output, cursor, DisposeAfterUse::YES);
}

} // End of anonymous namespace

MediaPlayer::MediaPlayer(RipperEngine *engine, InputManager *input, Audio::Mixer *mixer) :
		_engine(engine), _input(input), _mixer(mixer), _sceneAudioVolumePercent(100),
		_sceneAudioLoop(false), _stopSceneOnMouse(false) {
}

MediaPlayer::~MediaPlayer() {
	_mixer->stopHandle(_sceneAudioHandle);
}

bool MediaPlayer::loadAudio(const Common::String &path) {
	Common::File file;
	if (!file.open(Common::Path(path))) {
		warning("Ripper: could not load audio '%s'", path.c_str());
		return false;
	}
	_loadedAudioPath = path;
	uint start = 0;
	for (uint i = 0; i < path.size(); ++i) {
		if (path[i] == '/' || path[i] == '\\' || path[i] == ':')
			start = i + 1;
	}
	uint end = path.size();
	for (uint i = start; i < path.size(); ++i) {
		if (path[i] == '.') {
			end = i;
			break;
		}
	}
	_loadedAudioKey = path.substr(start, end - start);
	debugC(2, kDebugAudio, "Ripper: loaded audio slot key='%s' path='%s'",
		_loadedAudioKey.c_str(), _loadedAudioPath.c_str());
	return true;
}

bool MediaPlayer::startLoadedAudio(const Common::String &key, uint volumePercent, bool loop) {
	if (!_loadedAudioKey.equalsIgnoreCase(key))
		return false;
	Common::File *file = new Common::File();
	if (!file->open(Common::Path(_loadedAudioPath))) {
		delete file;
		return false;
	}
	Audio::SeekableAudioStream *wavStream = Audio::makeWAVStream(file, DisposeAfterUse::YES);
	if (!wavStream)
		return false;
	Audio::AudioStream *stream = wavStream;
	if (loop)
		stream = Audio::makeLoopingAudioStream(wavStream, 0);
	_mixer->stopHandle(_sceneAudioHandle);
	const byte volume = (byte)(MIN<uint>(volumePercent, 100) * Audio::Mixer::kMaxChannelVolume / 100);
	_mixer->playStream(Audio::Mixer::kSFXSoundType, &_sceneAudioHandle, stream, -1, volume);
	_sceneAudioVolumePercent = MIN<uint>(volumePercent, 100);
	_sceneAudioLoop = loop;
	debugC(1, kDebugAudio, "Ripper: started audio key='%s' volume=%u loop=%d active=%d",
		key.c_str(), volumePercent, loop, _mixer->isSoundHandleActive(_sceneAudioHandle));
	return true;
}

bool MediaPlayer::isSceneAudioActive() const {
	return _mixer->isSoundHandleActive(_sceneAudioHandle);
}

bool MediaPlayer::syncGame(Common::Serializer &serializer) {
	Common::String audioPath = serializer.isSaving() ? _loadedAudioPath : Common::String();
	byte active = isSceneAudioActive() ? 1 : 0;
	uint32 volumePercent = _sceneAudioVolumePercent;
	byte loop = _sceneAudioLoop ? 1 : 0;
	serializer.syncString(audioPath);
	serializer.syncAsByte(active);
	serializer.syncAsUint32LE(volumePercent);
	serializer.syncAsByte(loop);
	if (serializer.err() || audioPath.size() > 256 || volumePercent > 100)
		return false;
	if (serializer.isSaving())
		return true;

	_mixer->stopHandle(_sceneAudioHandle);
	_loadedAudioPath.clear();
	_loadedAudioKey.clear();
	_sceneAudioVolumePercent = volumePercent;
	_sceneAudioLoop = loop != 0;
	if (!audioPath.empty() && !loadAudio(audioPath))
		return false;
	if (active != 0 && !startLoadedAudio(_loadedAudioKey, volumePercent, loop != 0))
		return false;
	debugC(1, kDebugSaveLoad,
		"Ripper: restored scene audio path='%s' active=%d volume=%u loop=%d",
		audioPath.c_str(), active != 0, volumePercent, loop != 0);
	return true;
}

bool MediaPlayer::servicePlaybackInput(Video::SmackerDecoder &decoder, bool allowEscSpace,
		bool &paused, bool toolbarPaused, bool &skipToEnd,
		Audio::SoundHandle *externalAudio, bool suppressSceneMouseStop) {
	skipToEnd = false;
	if (_input->pollEvents()) {
		_engine->quitGame();
		return false;
	}
	if (!allowEscSpace || !_input->hasPendingKey())
		return !(_stopSceneOnMouse && !suppressSceneMouseStop &&
			_input->peekMouseState().pressed != 0);

	const uint16 command = _input->consumeKey();
	if (command == 0x1b) {
		skipToEnd = true;
		debugC(2, kDebugVideo, "Ripper: Escape advanced skippable presentation to end");
		return false;
	}
	if (command == 0x20) {
		paused = !paused;
		const bool effectivePause = paused || toolbarPaused;
		decoder.pauseVideo(effectivePause);
		if (externalAudio)
			_mixer->pauseHandle(*externalAudio, effectivePause);
		debugC(2, kDebugVideo, "Ripper: Space %s presentation", paused ? "paused" : "resumed");
	}
	return true;
}

bool MediaPlayer::playSmacker(Common::SeekableReadStream *stream, const Common::String &name,
		bool allowEscSpace, int x, int y, Audio::SoundHandle *externalAudio, bool *stoppedByUser,
		const Common::Array<uint32> *frameAudioOffsets, uint32 audioByteRate,
		uint32 timelineStartMillis, uint displayScale, bool patchInterfacePalette,
		uint frameLimit, int originY, bool presentFinalFrameOnEsc, bool patchWacMediaPalette) {
	if (stoppedByUser)
		*stoppedByUser = false;
	Video::SmackerDecoder decoder;
	if (!decoder.loadStream(stream)) {
		warning("Ripper: invalid Smacker stream '%s'", name.c_str());
		return false;
	}
	const uint outputWidth = decoder.getWidth() * displayScale;
	const uint outputHeight = decoder.getHeight() * displayScale;
	if (x < 0)
		x = (g_system->getWidth() - outputWidth) / 2;
	else
		x *= displayScale;
	if (y < 0)
		y = (g_system->getHeight() - outputHeight) / 2;
	else
		y = y * displayScale + originY;
	debugC(1, kDebugVideo,
		"Ripper: playing Smacker '%s' frames=%u source=%ux%u output=%ux%u at %d,%d controls=%d frameLimit=%u",
		name.c_str(), decoder.getFrameCount(), decoder.getWidth(), decoder.getHeight(),
		outputWidth, outputHeight, x, y, allowEscSpace, frameLimit);
	debugC(2, kDebugVideo, "Ripper: Smacker '%s' interfacePalettePatch=%d",
		name.c_str(), patchInterfacePalette);
	if (patchWacMediaPalette)
		debugC(2, kDebugVideo, "Ripper: Smacker '%s' WAC palette patch=10..149", name.c_str());
	const bool synchronizeToTimeline = frameAudioOffsets &&
		frameAudioOffsets->size() == decoder.getFrameCount() && audioByteRate != 0;
	if (synchronizeToTimeline) {
		debugC(2, kDebugVideo, "Ripper: Smacker '%s' follows IAVF timeline %u..%u ms clock=%s",
			name.c_str(),
			(uint32)((uint64)(*frameAudioOffsets)[0] * 1000 / audioByteRate),
			(uint32)((uint64)frameAudioOffsets->back() * 1000 / audioByteRate),
			externalAudio ? "mixer" : "system");
	}

	bool paused = false;
	bool toolbarPaused = false;
	bool completed = true;
	uint presentedFrames = 0;
	auto presentFrame = [&](const Graphics::Surface *frame, bool forcePalette) {
		if (forcePalette || decoder.hasDirtyPalette()) {
			byte palette[256 * 3];
			if (patchWacMediaPalette) {
				g_system->getPaletteManager()->grabPalette(palette, 0, 256);
				memcpy(palette + 10 * 3, decoder.getPalette() + 10 * 3, 140 * 3);
			} else {
				memcpy(palette, decoder.getPalette(), sizeof(palette));
			}
			if (patchInterfacePalette && !patchWacMediaPalette)
				_engine->getToolbar()->applySharedPalettePatch(palette, 256);
			g_system->getPaletteManager()->setPalette(palette, 0, 256);
		}
		if (displayScale == 1) {
			g_system->copyRectToScreen(frame->getPixels(), frame->pitch, x, y, frame->w, frame->h);
		} else {
			Graphics::Surface *screen = g_system->lockScreen();
			Graphics::scaleBlit((byte *)screen->getBasePtr(x, y),
				(const byte *)frame->getPixels(), screen->pitch, frame->pitch,
				outputWidth, outputHeight, frame->w, frame->h, frame->format);
			g_system->unlockScreen();
		}
		_engine->getScripts()->drawDialogueOverlay();
		g_system->updateScreen();
	};
	decoder.start();
	while (!_engine->shouldQuit() && !decoder.endOfVideo()) {
		// ExecuteSceneFrameAndInteractions at 0x13277 passes
		// PollInteractionAndResolveSelection at 0x13c8d as RunMediaSequence's
		// per-frame callback. RunFrontEndActionMenu blocks that callback while the
		// pointer remains in the toolbar band, so no Smacker frame advances.
		const bool toolbarOwnsInput =
			_engine->getScripts()->updateInteractiveCursor(_input->peekMouseState().position);
		if (toolbarOwnsInput != toolbarPaused) {
			toolbarPaused = toolbarOwnsInput;
			const bool effectivePause = paused || toolbarPaused;
			decoder.pauseVideo(effectivePause);
			if (externalAudio)
				_mixer->pauseHandle(*externalAudio, effectivePause);
			debugC(2, kDebugVideo,
				"Ripper: interactive scene media '%s' toolbarPause=%d keyboardPause=%d",
				name.c_str(), toolbarPaused, paused);
			g_system->updateScreen();
		}
		bool skipToEnd = false;
		if (!servicePlaybackInput(decoder, allowEscSpace, paused, toolbarPaused,
				skipToEnd, externalAudio, toolbarOwnsInput)) {
			completed = false;
			if (skipToEnd && presentFinalFrameOnEsc && decoder.getFrameCount() != 0) {
				const uint finalFrame = decoder.getFrameCount() - 1;
				const Graphics::Surface *frame = decoder.forceSeekToFrame(finalFrame);
				if (frame) {
					presentFrame(frame, true);
					completed = true;
					debugC(2, kDebugVideo,
						"Ripper: Escape presented final scene-transition frame '%s' frame=%u",
						name.c_str(), finalFrame);
				}
			}
			if (stoppedByUser && !_engine->shouldQuit())
				*stoppedByUser = true;
			break;
		}
		uint32 audioElapsedMs = 0;
		uint32 targetAudioMs = 0;
		bool frameDue = !synchronizeToTimeline && decoder.needsUpdate();
		if (synchronizeToTimeline && !paused && !toolbarPaused) {
			const uint32 nextFrame = decoder.getCurFrame() + 1;
			targetAudioMs = (uint32)((uint64)(*frameAudioOffsets)[nextFrame] * 1000 / audioByteRate);
			if (externalAudio && _mixer->isSoundHandleActive(*externalAudio))
				audioElapsedMs = _mixer->getSoundElapsedTime(*externalAudio);
			else
				audioElapsedMs = g_system->getMillis(true) - timelineStartMillis;
			frameDue = audioElapsedMs >= targetAudioMs;
		}
		if (!paused && !toolbarPaused && frameDue) {
			const Graphics::Surface *frame = decoder.decodeNextFrame();
			if (frame) {
				presentFrame(frame, false);
				++presentedFrames;
				if (synchronizeToTimeline) {
					debugC(11, kDebugVideo,
						"Ripper: Smacker '%s' frame=%d audioTargetMs=%u audioElapsedMs=%u driftMs=%d",
						name.c_str(), decoder.getCurFrame(), targetAudioMs, audioElapsedMs,
						(int32)audioElapsedMs - (int32)targetAudioMs);
				} else {
					debugC(11, kDebugVideo, "Ripper: Smacker '%s' frame=%d", name.c_str(), decoder.getCurFrame());
				}
				if (frameLimit != 0 && presentedFrames >= frameLimit) {
					debugC(2, kDebugVideo,
						"Ripper: stopped Smacker '%s' after bounded preview frame=%u",
						name.c_str(), presentedFrames);
					break;
				}
			}
		}
		if (frameLimit != 0 && presentedFrames >= frameLimit)
			break;
		if (toolbarPaused) {
			g_system->updateScreen();
			g_system->delayMillis(10);
		} else if (synchronizeToTimeline) {
			if (!paused && !toolbarPaused && !frameDue)
				g_system->delayMillis(MIN<uint32>(targetAudioMs - audioElapsedMs, 10));
			else
				g_system->delayMillis(1);
		} else {
			g_system->delayMillis(MIN<uint32>(decoder.getTimeToNextFrame(), 10));
		}
	}
	if (synchronizeToTimeline && completed) {
		const uint32 elapsedMs = externalAudio && _mixer->isSoundHandleActive(*externalAudio) ?
			_mixer->getSoundElapsedTime(*externalAudio) : g_system->getMillis(true) - timelineStartMillis;
		debugC(2, kDebugVideo, "Ripper: completed IAVF Smacker '%s' targetMs=%u audioElapsedMs=%u",
			name.c_str(),
			(uint32)((uint64)frameAudioOffsets->back() * 1000 / audioByteRate),
			elapsedMs);
	}
	decoder.close();
	return completed || (allowEscSpace && !_engine->shouldQuit());
}

bool MediaPlayer::playIavf(Common::SeekableReadStream &stream, const Common::String &name, bool allowEscSpace) {
	IavfMovie movie;
	if (!parseIavf(stream, name, movie)) {
		warning("Ripper: invalid IAVF presentation '%s'", name.c_str());
		return false;
	}

	Audio::SoundHandle audioHandle;
	bool audioActive = false;
	const uint32 timelineStartMillis = g_system->getMillis(true);
	if (!movie.audio.empty()) {
		Audio::SeekableAudioStream *audioStream = Audio::makeRawStream(
			movie.audio.data(), movie.audio.size(), movie.sampleRate,
			Audio::FLAG_16BITS | Audio::FLAG_LITTLE_ENDIAN, DisposeAfterUse::NO);
		if (audioStream) {
			_mixer->playStream(Audio::Mixer::kSpeechSoundType, &audioHandle, audioStream);
			audioActive = _mixer->isSoundHandleActive(audioHandle);
			debugC(2, kDebugVideo, "Ripper: started IAVF audio rate=%u bytes=%u active=%d",
				movie.sampleRate, movie.audio.size(), audioActive);
		}
	}

	bool result = true;
	const uint32 audioByteRate = movie.audioByteRate;
	uint completedSegments = 0;
	for (uint i = 0; i < movie.segments.size() && !_engine->shouldQuit(); ++i) {
		if (movie.segments[i].clearDisplayBefore) {
			// RunPacketizedMediaPlaybackCore at 0x5b592 handles IAVF opcode 0x68 by
			// clearing the active logical page through display command 0x14.
			g_system->fillScreen(0);
			g_system->updateScreen();
			debugC(2, kDebugVideo,
				"Ripper: IAVF '%s' cleared display before segment=%u from opcode 0x68",
				name.c_str(), i);
		}
		Common::SeekableReadStream *smacker = rebuildSmackerStream(movie.segments[i]);
		bool stoppedByUser = false;
		if (!smacker || !playSmacker(smacker, Common::String::format("%s#%u", name.c_str(), i),
			allowEscSpace, movie.segments[i].x, movie.segments[i].y,
			audioActive ? &audioHandle : nullptr, &stoppedByUser,
			&movie.segments[i].frameAudioOffsets, audioByteRate, timelineStartMillis,
			movie.displayScale, false)) {
			result = false;
			break;
		}
		if (stoppedByUser)
			break;
		++completedSegments;
	}
	if (audioActive)
		_mixer->stopHandle(audioHandle);
	if (movie.clearDisplayAfter && completedSegments == movie.segments.size()) {
		g_system->fillScreen(0);
		g_system->updateScreen();
		debugC(2, kDebugVideo,
			"Ripper: IAVF '%s' cleared display after final segment from opcode 0x68",
			name.c_str());
	}
	return result;
}

bool MediaPlayer::play(const Common::String &path, bool allowEscSpace, int x, int y) {
	Common::File *file = new Common::File();
	if (!file->open(Common::Path(path))) {
		warning("Ripper: could not open media '%s'", path.c_str());
		delete file;
		return false;
	}

	byte magic[8];
	if (!readExact(*file, magic, sizeof(magic))) {
		delete file;
		return false;
	}
	file->seek(0);

	// RunMediaPresentation at 0x168af preserves the current logical display
	// page and palette while packetized IAVF media owns the physical presentation.
	const bool isSmacker = memcmp(magic, "SMK2", 4) == 0 || memcmp(magic, "SMK4", 4) == 0;
	const bool isIavf = memcmp(magic, "IAVF2.00", 8) == 0;
	SavedDisplayContext displayContext;
	const bool displayContextCaptured = isIavf && captureDisplayContext(displayContext);
	if (isIavf) {
		debugC(2, kDebugVideo,
			"Ripper: captured script media display context media='%s' valid=%d size=%ux%u",
			path.c_str(), displayContextCaptured, displayContext.width, displayContext.height);
	}

	bool result = false;
	if (isSmacker) {
		result = playSmacker(file, path, allowEscSpace, x, y);
	} else if (isIavf) {
		result = playIavf(*file, path, allowEscSpace);
		delete file;
	} else {
		warning("Ripper: unsupported media magic for '%s'", path.c_str());
		delete file;
	}
	if (displayContextCaptured && !_engine->shouldQuit()) {
		const bool restored = restoreDisplayContext(displayContext);
		debugC(restored ? 2 : 1, kDebugVideo,
			"Ripper: restored script media display context media='%s' success=%d size=%ux%u",
			path.c_str(), restored, displayContext.width, displayContext.height);
		if (!restored)
			result = false;
	}

	_input->drainKeys();
	return result;
}

bool MediaPlayer::playWacMedia(const Common::String &path, int x, int y) {
	Common::File *file = new Common::File();
	if (!file->open(Common::Path(path))) {
		warning("Ripper: could not open WAC media '%s'", path.c_str());
		delete file;
		return false;
	}

	byte magic[4];
	if (!readExact(*file, magic, sizeof(magic))) {
		delete file;
		return false;
	}
	file->seek(0);
	if (memcmp(magic, "SMK2", 4) != 0 && memcmp(magic, "SMK4", 4) != 0) {
		warning("Ripper: unsupported WAC media magic for '%s'", path.c_str());
		delete file;
		return false;
	}

	debugC(1, kDebugVideo,
		"Ripper: entering WAC media presentation media='%s' position=%d,%d palette=10..149",
		path.c_str(), x, y);
	const bool result = playSmacker(file, path, false, x, y, nullptr, nullptr, nullptr,
		0, 0, 1, false, 0, 0, false, true);
	_input->drainKeys();
	return result;
}

bool MediaPlayer::playBlockingAudio(const Common::String &path) {
	Common::SeekableReadStream *audioStream = nullptr;
	Common::String source;
	Common::File *file = new Common::File();
	if (file->open(Common::Path(path))) {
		audioStream = file;
		source = "filesystem";
	} else {
		delete file;
		ResourceManager *resources = _engine->getResources();
		if (resources && resources->sound().hasMember(path)) {
			audioStream = resources->sound().createReadStreamForMember(path);
			source = "sound-library";
		}
	}
	if (!audioStream) {
		warning("Ripper: could not open blocking audio '%s' from the filesystem or sound library",
			path.c_str());
		return false;
	}
	Audio::SeekableAudioStream *stream = Audio::makeWAVStream(audioStream, DisposeAfterUse::YES);
	if (!stream)
		return false;

	Audio::SoundHandle handle;
	_mixer->playStream(Audio::Mixer::kSFXSoundType, &handle, stream);
	_engine->getCursor()->update(kBlockingAudioCursor);
	g_system->updateScreen();
	debugC(2, kDebugAudio,
		"Ripper: started blocking audio '%s' source=%s cursor=%u input=keyboard-only presentation=serviced",
		path.c_str(), source.c_str(), kBlockingAudioCursor);
	bool stoppedByEscape = false;
	while (!_engine->shouldQuit() && _mixer->isSoundHandleActive(handle)) {
		_engine->getCursor()->update(kBlockingAudioCursor);
		if (_input->pollEvents()) {
			_engine->quitGame();
			break;
		}
		while (_input->hasPendingKey()) {
			const uint16 command = _input->consumeKey();
			if (command == 0x1b) {
				stoppedByEscape = true;
				break;
			}
		}
		if (stoppedByEscape)
			break;
		g_system->updateScreen();
		g_system->delayMillis(10);
	}
	_mixer->stopHandle(handle);
	_input->discardMouseTransitions();
	debugC(2, kDebugAudio,
		"Ripper: completed blocking audio '%s' source=%s stoppedByEscape=%d",
		path.c_str(), source.c_str(), stoppedByEscape);
	return !_engine->shouldQuit();
}

bool MediaPlayer::playScene(const Common::String &path, int x, int y, bool firstFrameOnly,
		bool loopUntilInput, bool allowEscSpace) {
	debugC(1, kDebugVideo,
		"Ripper: entering scene presentation media='%s' firstFrameOnly=%d loopUntilInput=%d controls=%d",
		path.c_str(), firstFrameOnly, loopUntilInput, allowEscSpace);
	Common::File *file = new Common::File();
	if (!file->open(Common::Path(path))) {
		warning("Ripper: could not open scene media '%s'", path.c_str());
		delete file;
		return false;
	}

	byte magic[8];
	if (!readExact(*file, magic, sizeof(magic))) {
		delete file;
		return false;
	}
	file->seek(0);

	// HandleSceneEntryMediaPreviewOrPrompt at 0x15b03 supplies a target of one to
	// MediaSequenceCounterEqualsTarget at 0x15ac8. RunMediaSequence at 0x1e516
	// calls it after presenting the frame, so this path retains frame one onscreen.
	const uint frameLimit = firstFrameOnly ? 1 : 0;
	debugC(2, kDebugVideo,
		"Ripper: scene media '%s' mode=%s scriptPosition=%d,%d originY=%d controls=%d",
		path.c_str(), firstFrameOnly ? "first-frame-preview" : "sequence", x, y,
		kScenePresentationTop, allowEscSpace);
	bool result = false;
	_stopSceneOnMouse = loopUntilInput && !firstFrameOnly;
	const bool loop = loopUntilInput && !firstFrameOnly;
	uint pass = 0;
	do {
		if (pass++ != 0) {
			file = new Common::File();
			if (!file->open(Common::Path(path)))
				return false;
		}
	if (memcmp(magic, "SMK2", 4) == 0 || memcmp(magic, "SMK4", 4) == 0) {
		result = playSmacker(file, path, allowEscSpace, x, y, nullptr, nullptr, nullptr,
			0, 0, 1, true, frameLimit, kScenePresentationTop, allowEscSpace);
		if (!result && _stopSceneOnMouse && _input->peekMouseState().pressed != 0) {
			result = true;
			debugC(1, kDebugVideo,
				"Ripper: interactive scene media stopped by mouse; returning to hotspot polling");
		}
	} else if (memcmp(magic, "IAVF2.00", 8) == 0 && !firstFrameOnly) {
		result = playIavf(*file, path, allowEscSpace);
		delete file;
	} else {
		warning("Ripper: unsupported scene media mode for '%s'", path.c_str());
		delete file;
	}
	} while (loop && !_input->peekMouseState().pressed && !_engine->shouldQuit() && result);
	_input->drainKeys();
	_stopSceneOnMouse = false;
	return result;
}

} // End of namespace Ripper
