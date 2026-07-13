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
#include "common/system.h"
#include "graphics/blit.h"
#include "graphics/paletteman.h"
#include "graphics/surface.h"
#include "video/smk_decoder.h"

#include "ripper/detection.h"
#include "ripper/input.h"
#include "ripper/ripper.h"

namespace Ripper {

namespace {

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

	IavfSegment() : expectedFrames(0), x(-1), y(-1) {}
};

struct IavfMovie {
	uint16 sampleRate;
	byte channels;
	byte bitsPerSample;
	Common::Array<byte> audio;
	uint32 audioPayloadBytes;
	Common::Array<IavfSegment> segments;
	uint presentationWidth;
	uint presentationHeight;
	uint displayScale;

	IavfMovie() : sampleRate(0), channels(0), bitsPerSample(0), audioPayloadBytes(0),
		presentationWidth(0), presentationHeight(0), displayScale(1) {}
};

static bool readExact(Common::SeekableReadStream &stream, void *data, uint32 size) {
	return stream.read(data, size) == size;
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
	movie.sampleRate = READ_LE_UINT16(header.data() + 0x1c);
	movie.channels = header[0x1e];
	movie.bitsPerSample = header[0x1f];
	movie.presentationHeight = READ_LE_UINT16(header.data() + 0x2f);
	movie.presentationWidth = READ_LE_UINT16(header.data() + 0x31);
	if (movie.channels != 1 || movie.bitsPerSample != 16 || movie.sampleRate == 0) {
		warning("Ripper: unsupported IAVF audio format rate=%u channels=%u bits=%u in '%s'",
			movie.sampleRate, movie.channels, movie.bitsPerSample, name.c_str());
		return false;
	}
	if (movie.presentationWidth == 0 || movie.presentationHeight == 0)
		return false;
	movie.displayScale = movie.presentationHeight < 0xc9 && movie.presentationWidth < 0x141 ? 2 : 1;
	movie.audio.reserve((uint32)stream.size());

	Common::HashMap<uint32, Common::Array<byte> > setupCache;
	Common::HashMap<uint32, uint32> audioEndOffsets;
	IavfSegment *activeSegment = nullptr;
	bool reachedEnd = false;
	bool hasPendingAudioOffset = false;
	uint32 descriptorIndex = 0;
	uint32 pendingAudioOffset = 0;
	while (stream.pos() < stream.size() && !reachedEnd) {
		IavfDescriptor descriptor;
		if (!readDescriptor(stream, descriptor))
			return false;
		debugC(3, kDebugVideo,
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
				debugC(3, kDebugVideo,
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
			break;
		}

		case 0x68:
		case 0x75:
		case 0x77:
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
			if (activeSegment && activeSegment->frameSizes.size() != activeSegment->expectedFrames)
				return false;
			int32 branch[5];
			for (uint i = 0; i < ARRAYSIZE(branch); ++i)
				branch[i] = stream.readSint32LE();
			if (stream.err())
				return false;

			IavfSegment segment;
			segment.x = branch[0];
			segment.y = branch[1];
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
				!hasPendingAudioOffset)
				return false;
			Common::Array<byte> frame;
			if (!readBlob(stream, descriptor.arg0, frame))
				return false;
			activeSegment->frameSizes.push_back(descriptor.arg0);
			activeSegment->framePayloads.push_back(Common::move(frame));
			activeSegment->frameAudioOffsets.push_back(pendingAudioOffset);
			hasPendingAudioOffset = false;
			break;
		}

		case 0x70:
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
	for (uint i = 0; i < movie.segments.size(); ++i) {
		if (movie.segments[i].frameSizes.size() != movie.segments[i].expectedFrames ||
			movie.segments[i].frameAudioOffsets.size() != movie.segments[i].expectedFrames)
			return false;
		for (uint frame = 0; frame < movie.segments[i].frameAudioOffsets.size(); ++frame) {
			if (movie.segments[i].frameAudioOffsets[frame] > movie.audio.size())
				return false;
		}
	}
	const uint32 audioByteRate = movie.sampleRate * movie.channels * movie.bitsPerSample / 8;
	debugC(1, kDebugVideo,
		"Ripper: parsed IAVF '%s' canvas=%ux%u scale=%u segments=%u audioPayloadBytes=%u audioTimelineBytes=%u audioMs=%u",
		name.c_str(), movie.presentationWidth, movie.presentationHeight, movie.displayScale,
		movie.segments.size(), movie.audioPayloadBytes, movie.audio.size(),
		audioByteRate != 0 ? (uint32)((uint64)movie.audio.size() * 1000 / audioByteRate) : 0);
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
		_engine(engine), _input(input), _mixer(mixer) {
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
	debugC(2, kDebugVideo, "Ripper: loaded audio slot key='%s' path='%s'",
		_loadedAudioKey.c_str(), _loadedAudioPath.c_str());
	return true;
}

bool MediaPlayer::startLoadedAudio(const Common::String &key, uint volumePercent) {
	if (!_loadedAudioKey.equalsIgnoreCase(key))
		return false;
	Common::File *file = new Common::File();
	if (!file->open(Common::Path(_loadedAudioPath))) {
		delete file;
		return false;
	}
	Audio::SeekableAudioStream *stream = Audio::makeWAVStream(file, DisposeAfterUse::YES);
	if (!stream)
		return false;
	_mixer->stopHandle(_sceneAudioHandle);
	const byte volume = (byte)(MIN<uint>(volumePercent, 100) * Audio::Mixer::kMaxChannelVolume / 100);
	_mixer->playStream(Audio::Mixer::kSFXSoundType, &_sceneAudioHandle, stream, -1, volume);
	debugC(2, kDebugVideo, "Ripper: started audio key='%s' volume=%u", key.c_str(), volumePercent);
	return true;
}

bool MediaPlayer::servicePlaybackInput(Video::SmackerDecoder &decoder, bool allowEscSpace,
		bool &paused, Audio::SoundHandle *externalAudio) {
	if (_input->pollEvents()) {
		_engine->quitGame();
		return false;
	}
	if (!allowEscSpace || !_input->hasPendingKey())
		return true;

	const uint16 command = _input->consumeKey();
	if (command == 0x1b) {
		debugC(2, kDebugVideo, "Ripper: Escape stopped skippable presentation");
		return false;
	}
	if (command == 0x20) {
		paused = !paused;
		decoder.pauseVideo(paused);
		if (externalAudio)
			_mixer->pauseHandle(*externalAudio, paused);
		debugC(2, kDebugVideo, "Ripper: Space %s presentation", paused ? "paused" : "resumed");
	}
	return true;
}

bool MediaPlayer::playSmacker(Common::SeekableReadStream *stream, const Common::String &name,
		bool allowEscSpace, int x, int y, Audio::SoundHandle *externalAudio, bool *stoppedByUser,
		const Common::Array<uint32> *frameAudioOffsets, uint32 audioByteRate,
		uint32 timelineStartMillis, uint displayScale) {
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
		y *= displayScale;
	debugC(1, kDebugVideo,
		"Ripper: playing Smacker '%s' frames=%u source=%ux%u output=%ux%u at %d,%d controls=%d",
		name.c_str(), decoder.getFrameCount(), decoder.getWidth(), decoder.getHeight(),
		outputWidth, outputHeight, x, y, allowEscSpace);
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
	bool completed = true;
	decoder.start();
	while (!_engine->shouldQuit() && !decoder.endOfVideo()) {
		if (!servicePlaybackInput(decoder, allowEscSpace, paused, externalAudio)) {
			completed = false;
			if (stoppedByUser && !_engine->shouldQuit())
				*stoppedByUser = true;
			break;
		}
		uint32 audioElapsedMs = 0;
		uint32 targetAudioMs = 0;
		bool frameDue = !synchronizeToTimeline && decoder.needsUpdate();
		if (synchronizeToTimeline && !paused) {
			const uint32 nextFrame = decoder.getCurFrame() + 1;
			targetAudioMs = (uint32)((uint64)(*frameAudioOffsets)[nextFrame] * 1000 / audioByteRate);
			if (externalAudio && _mixer->isSoundHandleActive(*externalAudio))
				audioElapsedMs = _mixer->getSoundElapsedTime(*externalAudio);
			else
				audioElapsedMs = g_system->getMillis(true) - timelineStartMillis;
			frameDue = audioElapsedMs >= targetAudioMs;
		}
		if (!paused && frameDue) {
			const Graphics::Surface *frame = decoder.decodeNextFrame();
			if (frame) {
				if (decoder.hasDirtyPalette())
					g_system->getPaletteManager()->setPalette(decoder.getPalette(), 0, 256);
				if (displayScale == 1) {
					g_system->copyRectToScreen(frame->getPixels(), frame->pitch, x, y, frame->w, frame->h);
				} else {
					Graphics::Surface *screen = g_system->lockScreen();
					Graphics::scaleBlit((byte *)screen->getBasePtr(x, y),
						(const byte *)frame->getPixels(), screen->pitch, frame->pitch,
						outputWidth, outputHeight, frame->w, frame->h, frame->format);
					g_system->unlockScreen();
				}
				g_system->updateScreen();
				if (synchronizeToTimeline) {
					debugC(3, kDebugVideo,
						"Ripper: Smacker '%s' frame=%d audioTargetMs=%u audioElapsedMs=%u driftMs=%d",
						name.c_str(), decoder.getCurFrame(), targetAudioMs, audioElapsedMs,
						(int32)audioElapsedMs - (int32)targetAudioMs);
				} else {
					debugC(3, kDebugVideo, "Ripper: Smacker '%s' frame=%d", name.c_str(), decoder.getCurFrame());
				}
			}
		}
		if (synchronizeToTimeline) {
			if (!paused && !frameDue)
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
	const uint32 audioByteRate = movie.sampleRate * movie.channels * movie.bitsPerSample / 8;
	for (uint i = 0; i < movie.segments.size() && !_engine->shouldQuit(); ++i) {
		Common::SeekableReadStream *smacker = rebuildSmackerStream(movie.segments[i]);
		bool stoppedByUser = false;
		if (!smacker || !playSmacker(smacker, Common::String::format("%s#%u", name.c_str(), i),
			allowEscSpace, movie.segments[i].x, movie.segments[i].y,
			audioActive ? &audioHandle : nullptr, &stoppedByUser,
			&movie.segments[i].frameAudioOffsets, audioByteRate, timelineStartMillis,
			movie.displayScale)) {
			result = false;
			break;
		}
		if (stoppedByUser)
			break;
	}
	if (audioActive)
		_mixer->stopHandle(audioHandle);
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

	bool result = false;
	if (memcmp(magic, "SMK2", 4) == 0 || memcmp(magic, "SMK4", 4) == 0) {
		result = playSmacker(file, path, allowEscSpace, x, y);
	} else if (memcmp(magic, "IAVF2.00", 8) == 0) {
		result = playIavf(*file, path, allowEscSpace);
		delete file;
	} else {
		warning("Ripper: unsupported media magic for '%s'", path.c_str());
		delete file;
	}

	_input->drainKeys();
	return result;
}

} // End of namespace Ripper
