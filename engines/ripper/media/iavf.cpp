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

#include "ripper/media/media.h"

#include "audio/audiostream.h"
#include "audio/decoders/raw.h"
#include "common/debug.h"
#include "common/ptr.h"
#include "common/stream.h"
#include "common/system.h"

#include "ripper/detection.h"
#include "ripper/display.h"
#include "ripper/media/iavf_parser.h"
#include "ripper/input.h"
#include "ripper/ripper.h"
#include "ripper/script/script.h"

namespace Ripper {

bool MediaPlayer::playIavf(Common::SeekableReadStream &stream, const Common::String &name,
		const IavfPlaybackOptions &options) {
	IavfMovie movie;
	if (!parseIavf(stream, name, movie)) {
		warning("Ripper: invalid IAVF presentation '%s'", name.c_str());
		return false;
	}
	Common::Array<Common::ScopedPtr<Common::SeekableReadStream> > smackerStreams;
	smackerStreams.reserve(movie.segments.size());
	uint64 bufferedVideoBytes = 0;
	for (uint i = 0; i < movie.segments.size(); ++i) {
		Common::ScopedPtr<Common::SeekableReadStream> smacker(
			rebuildSmackerStream(movie.segments[i]));
		if (!smacker) {
			warning("Ripper: could not prebuffer IAVF Smacker branch '%s#%u'",
				name.c_str(), i);
			return false;
		}
		bufferedVideoBytes += smacker->size();
		smackerStreams.push_back(Common::move(smacker));
	}
	debugC(2, kDebugVideo,
		"Ripper: prebuffered IAVF '%s' videoBranches=%u videoBytes=%llu audioBytes=%u",
		name.c_str(), smackerStreams.size(),
		(unsigned long long)bufferedVideoBytes,
		movie.audio.size());

	Audio::SoundHandle audioHandle;
	bool audioActive = false;
	uint32 audioTimelineOffset = 0;
	uint32 timelineStartMillis = g_system->getMillis(true);
	auto startAudioAtOffset = [&](uint32 offset) {
		if (audioActive)
			_mixer->stopHandle(audioHandle);
		audioActive = false;
		if (movie.audio.empty())
			return true;
		if (offset >= movie.audio.size())
			return false;
		Audio::SeekableAudioStream *audioStream = Audio::makeRawStream(
			movie.audio.data() + offset, movie.audio.size() - offset, movie.sampleRate,
			Audio::FLAG_16BITS | Audio::FLAG_LITTLE_ENDIAN, DisposeAfterUse::NO);
		if (!audioStream)
			return false;
		_mixer->playStream(Audio::Mixer::kSpeechSoundType, &audioHandle, audioStream);
		audioActive = _mixer->isSoundHandleActive(audioHandle);
		audioTimelineOffset = offset;
		timelineStartMillis = g_system->getMillis(true);
		debugC(2, kDebugVideo,
			"Ripper: started IAVF audio rate=%u offset=%u bytes=%u active=%d",
			movie.sampleRate, offset, movie.audio.size() - offset, audioActive);
		return true;
	};
	if (!startAudioAtOffset(0))
		return false;

	bool result = true;
	const uint32 audioByteRate = movie.audioByteRate;
	bool completedFinalSegment = false;
	if (options.command)
		*options.command = 0;
	for (uint i = 0; i < movie.segments.size() && !_engine->shouldQuit(); ++i) {
		if (movie.segments[i].clearDisplayBefore) {
			g_system->fillScreen(0);
			presentScreen();
			debugC(2, kDebugVideo,
				"Ripper: IAVF '%s' cleared display before segment=%u from opcode 0x68",
				name.c_str(), i);
		}
		// Sequence zero describes the IAVF header; media branch ids start at one.
		const uint sequenceId = i + 1;
		IavfSegment &segment = movie.segments[i];
		Common::SeekableReadStream *smacker = smackerStreams[i].release();
		uint branchWidth = movie.presentationWidth;
		uint branchHeight = movie.presentationHeight;
		if (smacker && smacker->size() >= 12) {
			smacker->seek(4);
			branchWidth = smacker->readUint32LE();
			branchHeight = smacker->readUint32LE();
			smacker->seek(0);
		}
		if (options.callback)
			options.callback->beginIavfSegment(sequenceId,
				segment.expectedFrames, branchWidth, branchHeight);
		debugC(2, kDebugVideo,
			"Ripper: IAVF '%s' entered packetized branch segment=%u "
			"sequence=%u callback=%d size=%ux%u",
			name.c_str(), i, sequenceId, options.callback != nullptr,
			branchWidth, branchHeight);
		const int segmentX = options.x != -1 ? options.x : segment.x;
		const int segmentY = options.y != -1 ? options.y : segment.y;
		const int segmentOriginY = options.y != -1 ? options.originY : 0;
		if (options.x != -1 || options.y != -1) {
			debugC(2, kDebugVideo,
				"Ripper: IAVF '%s' segment=%u coordinate override raw=%d,%d caller=%d,%d originY=%d effective=%d,%d",
				name.c_str(), i, segment.x, segment.y,
				options.x, options.y, segmentOriginY, segmentX,
				segmentY + segmentOriginY);
		}
		Common::Array<uint32> relativeAudioOffsets;
		const Common::Array<uint32> *frameAudioOffsets = &segment.frameAudioOffsets;
		if (audioTimelineOffset != 0) {
			relativeAudioOffsets.reserve(frameAudioOffsets->size());
			for (uint frame = 0; frame < frameAudioOffsets->size(); ++frame) {
				relativeAudioOffsets.push_back((*frameAudioOffsets)[frame] >= audioTimelineOffset ?
					(*frameAudioOffsets)[frame] - audioTimelineOffset : 0);
			}
			frameAudioOffsets = &relativeAudioOffsets;
		}
		SmackerPlaybackPlan plan;
		plan.input.allowEscSpace = options.allowEscSpace;
		plan.placement.x = segmentX;
		plan.placement.y = segmentY;
		plan.timeline.externalAudio = audioActive ? &audioHandle : nullptr;
		plan.input.stopOnMouse = options.stopOnMouse;
		plan.timeline.frameAudioOffsets = frameAudioOffsets;
		plan.timeline.audioByteRate = audioByteRate;
		plan.timeline.timelineStartMillis = timelineStartMillis;
		plan.placement.displayScale = options.displayScale;
		plan.palette.patchInterfacePalette = false;
		plan.placement.originY = segmentOriginY;
		plan.input.serviceSceneUi = options.serviceSceneUi;
		plan.palette.rememberVideoPalette = options.rememberVideoPalette;
		plan.input.allowSegmentAdvance = true;
		plan.callback.sequenceCallback = options.callback;
		SmackerPlaybackResult playback;
		if (smacker)
			playback = playSmacker(smacker,
				Common::String::format("%s#%u", name.c_str(), i), plan);
		if (options.command)
			*options.command = playback.command;
		if (!playback.completed) {
			result = false;
			break;
		}
		if (playback.stoppedByUser)
			break;
		if (options.command && *options.command != 0)
			break;
		if (_engine->getScripts()->hasPendingSceneTransition())
			break;
		if (playback.advanceSegment) {
			if (i + 1 >= movie.segments.size()) {
				debugC(1, kDebugVideo,
					"Ripper: Right Arrow completed final IAVF segment '%s#%u'",
					name.c_str(), i);
				break;
			}
			const IavfSegment &nextSegment = movie.segments[i + 1];
			if (nextSegment.frameAudioOffsets.empty() ||
					!startAudioAtOffset(nextSegment.frameAudioOffsets[0])) {
				warning("Ripper: could not seek IAVF audio for '%s' segment=%u",
					name.c_str(), i + 1);
				result = false;
				break;
			}
			debugC(1, kDebugVideo,
				"Ripper: Right Arrow advanced IAVF '%s' segment=%u->%u audioOffset=%u",
				name.c_str(), i, i + 1, audioTimelineOffset);
			continue;
		}
		completedFinalSegment = i + 1 == movie.segments.size();
	}
	// Keep the final frame until managed audio ends; KA_BOOK.AVI relies on it.
	if (result && completedFinalSegment && audioActive &&
			!_engine->shouldQuit() && !_engine->getScripts()->hasPendingSceneTransition() &&
			_mixer->isSoundHandleActive(audioHandle)) {
		RipperEngine::SaveLoadScope saveLoad(*_engine);
		const uint32 targetAudioMs = (uint32)((uint64)(movie.audio.size() - audioTimelineOffset) *
			1000 / audioByteRate);
		debugC(2, kDebugVideo,
			"Ripper: retaining final IAVF display '%s' for managed-audio tail targetMs=%u elapsedMs=%u",
			name.c_str(), targetAudioMs, _mixer->getSoundElapsedTime(audioHandle));
		bool paused = false;
		bool skipped = false;
		while (!_engine->shouldQuit() && _mixer->isSoundHandleActive(audioHandle)) {
			if (_input->pollEvents()) {
				result = false;
				break;
			}
			if (options.allowEscSpace && _input->hasPendingKey()) {
				const uint16 inputCommand = _input->consumeKey();
				if (inputCommand == 0x1b || inputCommand == 0x4d00) {
					skipped = true;
					debugC(2, kDebugVideo,
						"Ripper: %s completed final IAVF managed-audio tail '%s'",
						inputCommand == 0x1b ? "Escape" : "Right Arrow", name.c_str());
					break;
				}
				if (inputCommand == 0x20) {
					paused = !paused;
					_mixer->pauseHandle(audioHandle, paused);
					debugC(2, kDebugVideo, "Ripper: Space %s IAVF managed-audio tail '%s'",
						paused ? "paused" : "resumed", name.c_str());
				}
			}
			g_system->delayMillis(10);
		}
		if (paused)
			_mixer->pauseHandle(audioHandle, false);
		debugC(2, kDebugVideo,
			"Ripper: completed final IAVF managed-audio tail '%s' targetMs=%u elapsedMs=%u skipped=%d",
			name.c_str(), targetAudioMs, _mixer->getSoundElapsedTime(audioHandle), skipped);
	}
	if (audioActive)
		_mixer->stopHandle(audioHandle);
	if (movie.clearDisplayAfter && completedFinalSegment) {
		g_system->fillScreen(0);
		presentScreen();
		debugC(2, kDebugVideo,
			"Ripper: IAVF '%s' cleared display after final segment from opcode 0x68",
			name.c_str());
	}
	return result;
}

} // End of namespace Ripper
