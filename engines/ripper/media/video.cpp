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

#include "audio/audiostream.h"
#include "audio/decoders/raw.h"
#include "common/array.h"
#include "common/debug.h"
#include "common/system.h"
#include "graphics/blit.h"
#include "graphics/paletteman.h"
#include "graphics/surface.h"
#include "video/smk_decoder.h"

#include "ripper/cursor.h"
#include "ripper/detection.h"
#include "ripper/display.h"
#include "ripper/iavf.h"
#include "ripper/input.h"
#include "ripper/ripper.h"
#include "ripper/scene_audio.h"
#include "ripper/settings.h"
#include "ripper/script.h"
#include "ripper/toolbar.h"

namespace Ripper {

namespace {

static const int kScenePresentationTop = 50;
static const uint16 kHelpCommand = 0x3b00;
static const uint kAutoPacketizedDisplayScale = 0;

class RipperSmackerDecoder : public Video::SmackerDecoder {
public:
	bool loadStreamWithDataFixes(Common::SeekableReadStream *stream,
			const Common::String &name) {
		if (!Video::SmackerDecoder::loadStream(stream))
			return false;

		repairEfw4FrameTable(name);
		return true;
	}

	const Graphics::Surface *restartAtFrame(uint frame) {
		if (!isVideoLoaded() || frame >= getFrameCount() || !rewind())
			return nullptr;

		stopAudio();
		RipperSmackerVideoTrack *videoTrack =
			static_cast<RipperSmackerVideoTrack *>(getTrack(0));
		if (!videoTrack)
			return nullptr;
		videoTrack->clearSurface();

		const Graphics::Surface *surface = nullptr;
		while (getCurFrame() < (int)frame && !endOfVideo())
			surface = decodeNextFrame();
		if (!surface || getCurFrame() != (int)frame)
			return nullptr;

		_lastTimeChange = videoTrack->getFrameTime(frame);
		if (isPlaying())
			_startTime = g_system->getMillis() -
				(_lastTimeChange.msecs() / getRate()).toInt();
		resetPauseStartTime();
		return surface;
	}

protected:
	void repairEfw4FrameTable(const Common::String &name) {
		// LoadSmackerPlaybackState at 0x4f140 and
		// AdvanceSmackerPlaybackFrame at 0x4ffe8 consume each frame-table
		// DWORD as an ordinary byte count. The extracted EFW4.SMK has one
		// damaged entry: frame 62 says 0x2000a070, while 0x8070 makes the
		// declared payload end exactly at EOF and matches its neighboring
		// frame sizes. Restrict the repair to the complete known signature.
		const uint kFrame = 62;
		const uint32 kDamagedSize = 0x2000a070;
		const uint32 kRepairedSize = 0x8070;
		const uint32 kFrameCount = 110;
		const uint32 kFileSize = 0x2ce97c;
		if (!name.hasSuffixIgnoreCase("efw4.smk") ||
				getFrameCount() != kFrameCount || !_frameSizes ||
				_frameSizes[kFrame] != kDamagedSize ||
				_fileStream->size() != kFileSize)
			return;

		uint64 repairedEnd = _fileStream->pos();
		for (uint i = 0; i < kFrameCount; ++i)
			repairedEnd += (i == kFrame ? kRepairedSize : _frameSizes[i]) & ~3U;
		if (repairedEnd != kFileSize) {
			warning("Ripper: EFW4 Smacker frame-table repair rejected repairedEnd=%u fileSize=%u",
				(uint)repairedEnd, kFileSize);
			return;
		}

		_frameSizes[kFrame] = kRepairedSize;
		debugC(1, kDebugVideo,
			"Ripper: repaired EFW4 Smacker frame table frame=%u raw=0x%08x size=0x%04x fileSize=%u",
			kFrame, kDamagedSize, kRepairedSize, kFileSize);
	}

	class RipperSmackerVideoTrack : public SmackerVideoTrack {
	public:
		RipperSmackerVideoTrack(uint32 width, uint32 height, uint32 frameCount,
				const Common::Rational &frameRate, uint32 flags, uint32 version) :
			SmackerVideoTrack(width, height, frameCount, frameRate, flags, version) {}

		void clearSurface() {
			_surface->fillRect(_surface->getRect(), 0);
		}
	};

	SmackerVideoTrack *createVideoTrack(uint32 width, uint32 height, uint32 frameCount,
			const Common::Rational &frameRate, uint32 flags, uint32 version) const override {
		return new RipperSmackerVideoTrack(width, height, frameCount, frameRate,
			flags, version);
	}
};

} // End of anonymous namespace

MediaPlayer::MediaPlayer(RipperEngine *engine, InputManager *input, Audio::Mixer *mixer) :
		_engine(engine), _input(input), _mixer(mixer), _stopSceneOnMouse(false) {
}

MediaPlayer::~MediaPlayer() {
}

MediaPlayer::ActivePlaybackGuard::ActivePlaybackGuard(MediaPlayer *player,
		Video::SmackerDecoder *decoder, const Common::String &name,
		Audio::SoundHandle *externalAudio) : _player(player) {
	_player->_activePlaybacks.push_back(ActivePlayback(decoder, name, externalAudio));
}

MediaPlayer::ActivePlaybackGuard::~ActivePlaybackGuard() {
	assert(!_player->_activePlaybacks.empty());
	_player->_activePlaybacks.pop_back();
}

void MediaPlayer::logPlaybackPause(const char *source, bool pause,
		Video::SmackerDecoder &decoder, const Common::String &name,
		Audio::SoundHandle *externalAudio) const {
	const bool externalActive = externalAudio &&
		_mixer->isSoundHandleActive(*externalAudio);
	const uint32 externalElapsed = externalActive ?
		_mixer->getSoundElapsedTime(*externalAudio) : 0;
	debugC(1, kDebugVideo,
		"Ripper: %s %s media='%s' frame=%d decoderMs=%u "
		"decoderPaused=%d externalAudio=%d externalElapsedMs=%u sceneAudio=%d",
		source, pause ? "paused" : "resumed", name.c_str(), decoder.getCurFrame(),
		decoder.getTime(), decoder.isPaused(), externalActive, externalElapsed,
		_engine->getSceneAudio()->isActive());
}

void MediaPlayer::pauseActiveMedia(bool pause) {
	for (uint i = 0; i < _activePlaybacks.size(); ++i) {
		ActivePlayback &playback = _activePlaybacks[i];
		playback.decoder->pauseVideo(pause);
		logPlaybackPause("ScummVM", pause, *playback.decoder, playback.name,
			playback.externalAudio);
	}
	if (_activePlaybacks.empty()) {
		debugC(2, kDebugVideo, "Ripper: ScummVM %s without active media",
			pause ? "paused" : "resumed");
	}
}

bool MediaPlayer::servicePlaybackInput(Video::SmackerDecoder &decoder, bool allowEscSpace,
		bool allowSegmentAdvance, bool &paused, bool &skipToEnd,
		bool &advanceSegment,
		Audio::SoundHandle *externalAudio, bool suppressSceneMouseStop, bool allowSceneHelp,
		const Common::String &name) {
	skipToEnd = false;
	advanceSegment = false;
	if (_input->pollEvents()) {
		_engine->quitGame();
		return false;
	}
	if (allowSceneHelp && _input->peekKey() == kHelpCommand) {
		_input->consumeKey();
		decoder.pauseVideo(true);
		_mixer->pauseAll(true);
		logPlaybackPause("Help", true, decoder, name, externalAudio);
		const bool helpDisplayed = _engine->getScripts()->showHelp("interactive-media");
		_mixer->pauseAll(false);
		decoder.pauseVideo(false);
		logPlaybackPause("Help", false, decoder, name, externalAudio);
		return helpDisplayed;
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
		decoder.pauseVideo(paused);
		_mixer->pauseAll(paused);
		logPlaybackPause("Space", paused, decoder, name, externalAudio);
	}
	if (command == 0x4d00 && allowSegmentAdvance) {
		advanceSegment = true;
		debugC(2, kDebugVideo, "Ripper: Right Arrow requested next IAVF segment");
		return false;
	}
	return true;
}

bool MediaPlayer::playSmacker(Common::SeekableReadStream *stream, const Common::String &name,
		const SmackerPlaybackPlan &plan) {
	debugC(2, kDebugVideo, "Ripper: Smacker playback plan media='%s' %s",
		name.c_str(), describeSmackerPlaybackPlan(plan).c_str());
	const bool allowEscSpace = plan.input.allowEscSpace;
	int x = plan.placement.x;
	int y = plan.placement.y;
	const Common::Rect &centerBounds = plan.placement.centerBounds;
	Audio::SoundHandle *externalAudio = plan.timeline.externalAudio;
	bool *stoppedByUser = plan.input.stoppedByUser;
	const Common::Array<uint32> *frameAudioOffsets = plan.timeline.frameAudioOffsets;
	const uint32 audioByteRate = plan.timeline.audioByteRate;
	const uint32 timelineStartMillis = plan.timeline.timelineStartMillis;
	uint displayScale = plan.placement.displayScale;
	const bool patchInterfacePalette = plan.palette.patchInterfacePalette;
	const uint frameLimit = plan.frames.frameLimit;
	const int originY = plan.placement.originY;
	const bool patchWacMediaPalette = plan.palette.patchWacMediaPalette;
	const bool preserveDisplayPalette = plan.palette.preserveDisplayPalette;
	const bool serviceSceneUi = plan.input.serviceSceneUi;
	const bool loopFromStart = plan.loop.loopFromStart;
	bool *advanceSegment = plan.input.advanceSegment;
	const uint loopStartFrame = plan.loop.loopStartFrame;
	MediaSequenceCallback *sequenceCallback = plan.callback.sequenceCallback;
	uint16 *sequenceCommand = plan.callback.sequenceCommand;
	Common::Array<byte> *sourcePalette = plan.palette.sourcePalette;
	const bool rememberVideoPalette = plan.palette.rememberVideoPalette;
	const uint firstFrame = plan.frames.firstFrame;
	uint lastFrame = plan.frames.lastFrame;
	const uint boundedLoopStartFrame = plan.loop.boundedLoopStartFrame;
	const bool transparentFirstPixel = plan.rendering.transparentFirstPixel;
	const bool retainFinalTransparentFrame = plan.rendering.retainFinalTransparentFrame;
	if (stoppedByUser)
		*stoppedByUser = false;
	if (advanceSegment)
		*advanceSegment = false;
	if (sequenceCommand)
		*sequenceCommand = 0;
	RipperSmackerDecoder decoder;
	if (!decoder.loadStreamWithDataFixes(stream, name)) {
		warning("Ripper: invalid Smacker stream '%s'", name.c_str());
		return false;
	}
	const bool boundedSegment = firstFrame != 0 || lastFrame != 0xffffffff;
	const bool boundedLoop = boundedLoopStartFrame != 0xffffffff;
	if (lastFrame == 0xffffffff)
		lastFrame = decoder.getFrameCount() - 1;
	if (decoder.getFrameCount() == 0 || firstFrame > lastFrame ||
			lastFrame >= decoder.getFrameCount() ||
			(boundedLoop && (!boundedSegment || boundedLoopStartFrame < firstFrame ||
				boundedLoopStartFrame > lastFrame))) {
		warning("Ripper: invalid Smacker segment '%s' frames=%u..%u count=%u",
			name.c_str(), firstFrame, lastFrame, decoder.getFrameCount());
		return false;
	}
	// InitializeMediaPresentationDisplayModeCallback at 0x163a8 is invoked for
	// each packetized branch, not only for the IAVF container dimensions. In the
	// original 640x400 mode every branch smaller than 321x201 receives the 2:1
	// display descriptor, including PROLOG2.AVI's 320x200 branches inside a
	// 640x400 container.
	if (displayScale == kAutoPacketizedDisplayScale)
		displayScale = decoder.getHeight() < 0xc9 && decoder.getWidth() < 0x141 ? 2 : 1;
	const uint outputWidth = decoder.getWidth() * displayScale;
	const uint outputHeight = decoder.getHeight() * displayScale;
	const int displayWidth = g_system->getWidth();
	const int displayHeight = g_system->getHeight();
	if (!centerBounds.isEmpty()) {
		if (centerBounds.width() < (int)outputWidth ||
				centerBounds.height() < (int)outputHeight) {
			warning("Ripper: Smacker '%s' output=%ux%u exceeds centering bounds=%d,%d,%d,%d",
				name.c_str(), outputWidth, outputHeight, centerBounds.left,
				centerBounds.top, centerBounds.right, centerBounds.bottom);
			decoder.close();
			return false;
		}
		x = centerBounds.left + (centerBounds.width() - (int)outputWidth) / 2;
		y = centerBounds.top + (centerBounds.height() - (int)outputHeight) / 2;
	} else {
		if (x < 0)
			x = (displayWidth - outputWidth) / 2;
		else
			x *= displayScale;
		if (y < 0)
			y = (displayHeight - outputHeight) / 2;
		else {
			y *= displayScale;
			if (originY != 0 && y + originY + (int)outputHeight > displayHeight &&
					y + (int)outputHeight <= displayHeight) {
				// InitializeMediaPresentationDisplayModeCallback at 0x163a8
				// switches a full-size packetized branch from the scene viewport
				// to the full display context. Do not retain the viewport origin
				// when the scaled branch already fills that display.
				debugC(2, kDebugVideo,
					"Ripper: suppressed Smacker scene origin media='%s' originY=%d "
					"output=%ux%u display=%dx%d",
					name.c_str(), originY, outputWidth, outputHeight,
					displayWidth, displayHeight);
			} else {
				y += originY;
			}
		}
	}
	if (x < 0 || y < 0 || x + (int)outputWidth > displayWidth ||
			y + (int)outputHeight > displayHeight) {
		warning("Ripper: Smacker '%s' draw rectangle %d,%d,%ux%u exceeds display %dx%d",
			name.c_str(), x, y, outputWidth, outputHeight,
			displayWidth, displayHeight);
		decoder.close();
		return false;
	}
	debugC(1, kDebugVideo,
		"Ripper: playing Smacker '%s' frames=%u source=%ux%u output=%ux%u at %d,%d controls=%d sceneUi=%d frameLimit=%u",
		name.c_str(), decoder.getFrameCount(), decoder.getWidth(), decoder.getHeight(),
		outputWidth, outputHeight, x, y, allowEscSpace, serviceSceneUi, frameLimit);
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
	Common::Array<byte> transparentBacking;
	if (transparentFirstPixel) {
		Graphics::Surface *screen = g_system->lockScreen();
		if (!screen || screen->format.bytesPerPixel != 1 || x < 0 || y < 0 ||
				x + (int)outputWidth > screen->w || y + (int)outputHeight > screen->h) {
			if (screen)
				g_system->unlockScreen();
			warning("Ripper: transparent Smacker overlay '%s' is outside the indexed display",
				name.c_str());
			decoder.close();
			return false;
		}
		transparentBacking.resize(outputWidth * outputHeight);
		for (uint row = 0; row < outputHeight; ++row)
			memcpy(transparentBacking.data() + row * outputWidth,
				screen->getBasePtr(x, y + row), outputWidth);
		g_system->unlockScreen();
		debugC(2, kDebugVideo,
			"Ripper: captured transparent Smacker backing media='%s' rect=%d,%d,%ux%u",
			name.c_str(), x, y, outputWidth, outputHeight);
	}
	auto applyDecoderPalette = [&](bool forcePalette) {
		if (forcePalette || decoder.hasDirtyPalette()) {
			byte palette[256 * 3];
			if (sourcePalette) {
				sourcePalette->resize(sizeof(palette));
				memcpy(sourcePalette->data(), decoder.getPalette(), sizeof(palette));
			}
			if (preserveDisplayPalette)
				return;
			if (patchWacMediaPalette) {
				g_system->getPaletteManager()->grabPalette(palette, 0, 256);
				memcpy(palette + 10 * 3, decoder.getPalette() + 10 * 3, 140 * 3);
			} else {
				memcpy(palette, decoder.getPalette(), sizeof(palette));
			}
			if (patchInterfacePalette && !patchWacMediaPalette)
				_engine->getToolbar()->applySharedPalettePatch(palette, 256);
			if (!patchWacMediaPalette)
				_engine->getSettings()->applyVideoPalette(palette, 256,
					rememberVideoPalette);
			g_system->getPaletteManager()->setPalette(palette, 0, 256);
		}
	};
	auto presentFrame = [&](const Graphics::Surface *frame, bool forcePalette) {
		applyDecoderPalette(forcePalette);
		if (transparentFirstPixel) {
			Graphics::Surface *screen = g_system->lockScreen();
			if (!screen) {
				warning("Ripper: could not lock display for transparent Smacker overlay '%s'",
					name.c_str());
				completed = false;
				return;
			}
			for (uint row = 0; row < outputHeight; ++row)
				memcpy(screen->getBasePtr(x, y + row),
					transparentBacking.data() + row * outputWidth, outputWidth);
			const byte transparentColor = *(const byte *)frame->getPixels();
			for (uint sourceY = 0; sourceY < (uint)frame->h; ++sourceY) {
				const byte *source = (const byte *)frame->getBasePtr(0, sourceY);
				for (uint sourceX = 0; sourceX < (uint)frame->w; ++sourceX) {
					if (source[sourceX] == transparentColor)
						continue;
					for (uint scaleY = 0; scaleY < displayScale; ++scaleY) {
						byte *destination = (byte *)screen->getBasePtr(
							x + sourceX * displayScale,
							y + sourceY * displayScale + scaleY);
						memset(destination, source[sourceX], displayScale);
					}
				}
			}
			g_system->unlockScreen();
		} else if (displayScale == 1) {
			g_system->copyRectToScreen(frame->getPixels(), frame->pitch, x, y, frame->w, frame->h);
		} else {
			Graphics::Surface *screen = g_system->lockScreen();
			if (!screen) {
				warning("Ripper: could not lock display for scaled Smacker '%s'",
					name.c_str());
				completed = false;
				return;
			}
			Graphics::scaleBlit((byte *)screen->getBasePtr(x, y),
				(const byte *)frame->getPixels(), screen->pitch, frame->pitch,
				outputWidth, outputHeight, frame->w, frame->h, frame->format);
			g_system->unlockScreen();
		}
		if (serviceSceneUi) {
			_engine->getScripts()->drawDialogueOverlay(true);
			_engine->getScripts()->drawBriefingOverlay();
		}
		// Sequence callbacks may add puzzle or dialogue overlays. Defer their
		// screen submission until the callback completes so an undecorated movie
		// frame is never visible between the frame blit and overlay composition.
		if (!sequenceCallback)
			presentScreen();
	};
	// ExecutePresentationEntry at 0x1652a deactivates the shared selection
	// presentation before packetized AVI playback. Only RunMediaSequence's
	// scene-frame callback path keeps cursor, toolbar, and dialogue controls live.
	if (!serviceSceneUi)
		_engine->getCursor()->setVisible(false);
	ActivePlaybackGuard playbackGuard(this, &decoder, name, externalAudio);
	decoder.start();
	bool sequencePaletteRefresh = false;
	bool callbackOwnedInput = false;
	auto serviceSequenceCallback = [&](uint frame) {
		if (!sequenceCallback)
			return false;
		decoder.pauseVideo(true);
		const uint16 command = sequenceCallback->service(frame);
		decoder.pauseVideo(false);
		if (command == 0)
			return false;
		if (command == MediaSequenceCallback::kContinueRefreshPalette) {
			sequencePaletteRefresh = true;
			debugC(3, kDebugVideo,
				"Ripper: interactive Smacker '%s' requested palette refresh frame=%u",
				name.c_str(), frame);
			return false;
		}
		if (sequenceCommand)
			*sequenceCommand = command;
		debugC(2, kDebugVideo,
			"Ripper: interactive Smacker '%s' returned command=0x%04x frame=%u",
			name.c_str(), command, frame);
		return true;
	};
	auto finishSequenceFramePresentation = [&](uint frame) {
		const bool stopSequence = serviceSequenceCallback(frame);
		const bool refreshedPalette = sequencePaletteRefresh;
		if (sequencePaletteRefresh) {
			applyDecoderPalette(true);
			sequencePaletteRefresh = false;
		}
		if (sequenceCallback && sequenceCallback->managesPalette()) {
			byte palette[256 * 3];
			// RunCombatEncounterScene at 0x31436 derives its temporary hit and
			// shield palettes from the active Smacker palette on every frame.
			// Rebuild that base before asking the encounter callback to transform it.
			applyDecoderPalette(true);
			g_system->getPaletteManager()->grabPalette(palette, 0, 256);
			sequenceCallback->transformPalette(palette, 256);
			g_system->getPaletteManager()->setPalette(palette, 0, 256);
		}
		presentScreen();
		debugC(11, kDebugVideo,
			"Ripper: presented interactive Smacker '%s' after callback frame=%u paletteRefresh=%d stop=%d",
			name.c_str(), frame, refreshedPalette, stopSequence);
		return stopSequence;
	};
	if (firstFrame != 0) {
		// RunTubeSwitchScene at 0x25e18 advances a newly loaded GA_TUBE decoder
		// through frames 0..14 before presenting frame 15. Decode from the start
		// here as well so delta-coded pixels and palette entries are based on the
		// complete initial surface rather than a blank seek surface.
		const Graphics::Surface *frame = nullptr;
		while (decoder.getCurFrame() < (int)firstFrame && !decoder.endOfVideo())
			frame = decoder.decodeNextFrame();
		if (!frame || decoder.getCurFrame() != (int)firstFrame) {
			warning("Ripper: could not advance Smacker '%s' to bounded segment frame=%u",
				name.c_str(), firstFrame);
			decoder.close();
			return false;
		}
		presentFrame(frame, true);
		++presentedFrames;
		debugC(3, kDebugVideo,
			"Ripper: entered bounded Smacker segment '%s' frame=%u..%u",
			name.c_str(), firstFrame, lastFrame);
		if (sequenceCallback && finishSequenceFramePresentation(firstFrame + 1)) {
			decoder.close();
			return true;
		}
		if (boundedSegment && firstFrame == lastFrame) {
			decoder.close();
			return true;
		}
	}
	while (!_engine->shouldQuit()) {
		if (decoder.endOfVideo()) {
			if (loopFromStart) {
				// RunMediaSequence at 0x1e516 resets its one-based frame counters
				// to one when the loop flag is set with a zero loop-start. It does
				// not reload or seek the Smacker state, so skipped blocks in the
				// first packet continue from the decoder's wrapped framebuffer.
				if (!decoder.rewind()) {
					warning("Ripper: could not rewind looping scene Smacker '%s'",
						name.c_str());
					completed = false;
					break;
				}
				presentedFrames = 0;
				debugC(3, kDebugVideo,
					"Ripper: rewound looping scene Smacker '%s' with retained decode surface",
					name.c_str());
				continue;
			}
			if (sequenceCallback && sequenceCallback->continueAfterEnd()) {
				// Async scene choosers keep servicing input over the final decoded
				// frame. HandleSceneEntryAsyncTextRequest at 0x157a1 does not stop
				// RunMediaSequence while the chooser owns the callback continuation.
				if (finishSequenceFramePresentation(decoder.getFrameCount()))
					break;
				g_system->delayMillis(10);
				continue;
			}
			if (!sequenceCallback || loopStartFrame == 0)
				break;
			if (loopStartFrame > decoder.getFrameCount()) {
				warning("Ripper: Smacker '%s' loop start frame=%u exceeds frame count=%u",
					name.c_str(), loopStartFrame, decoder.getFrameCount());
				completed = false;
				break;
			}
			// SeekSmackerPlaybackFrame at 0x50a88 resumes at the requested packet
			// over the retail decoder's wrapped framebuffer. ScummVM's offscreen
			// decoder does not reproduce that surface state after an arbitrary seek,
			// so rebuild the exact loop-start frame from a cleared initial surface.
			const Graphics::Surface *frame =
				decoder.restartAtFrame(loopStartFrame - 1);
			if (!frame) {
				warning("Ripper: could not seek Smacker '%s' to loop start frame=%u",
					name.c_str(), loopStartFrame);
				completed = false;
				break;
			}
			presentFrame(frame, true);
			++presentedFrames;
			debugC(3, kDebugVideo,
				"Ripper: looped interactive Smacker '%s' to frame=%u after full reconstruction",
				name.c_str(), loopStartFrame);
			if (finishSequenceFramePresentation(loopStartFrame))
				break;
			continue;
		}
		// ExecuteSceneFrameAndInteractions at 0x13277 passes
		// PollInteractionAndResolveSelection at 0x13c8d as RunMediaSequence's
		// per-frame callback. RunFrontEndActionMenu blocks that callback while the
		// pointer remains in the toolbar band, so no Smacker frame advances.
		const bool callbackOwnsInput = sequenceCallback && sequenceCallback->ownsInput();
		if (callbackOwnsInput != callbackOwnedInput) {
			debugC(2, kDebugVideo,
				"Ripper: interactive Smacker '%s' sequence callback input=%d",
				name.c_str(), callbackOwnsInput);
			callbackOwnedInput = callbackOwnsInput;
		}
		bool playbackUiFailed = false;
		const bool toolbarOwnsInput = serviceSceneUi && !callbackOwnsInput &&
			_engine->getScripts()->updateInteractiveCursor(
				_input->peekMouseState().position, &playbackUiFailed);
		if (playbackUiFailed) {
			completed = false;
			break;
		}
		if (_engine->getScripts()->hasPendingRuntimeRestore() ||
				(serviceSceneUi && !callbackOwnsInput &&
				_engine->getScripts()->hasPendingSceneTransition())) {
			debugC(1, kDebugScene,
				"Ripper: interactive media '%s' returning queued runtime transition",
				name.c_str());
			break;
		}
		if (toolbarOwnsInput != toolbarPaused) {
			toolbarPaused = toolbarOwnsInput;
			decoder.pauseVideo(toolbarPaused);
			if (externalAudio)
				_mixer->pauseHandle(*externalAudio, toolbarPaused);
			debugC(2, kDebugVideo,
				"Ripper: interactive scene media '%s' toolbarPause=%d keyboardPause=%d",
				name.c_str(), toolbarPaused, paused);
			presentScreen();
		}
		bool skipToEnd = false;
		bool advanceToNextSegment = false;
		if (!servicePlaybackInput(decoder, allowEscSpace && !callbackOwnsInput,
				advanceSegment != nullptr,
				paused, skipToEnd, advanceToNextSegment,
				externalAudio, toolbarOwnsInput, serviceSceneUi, name)) {
			completed = false;
			if (advanceToNextSegment) {
				if (advanceSegment)
					*advanceSegment = true;
				completed = true;
			}
			if (skipToEnd && decoder.getFrameCount() != 0) {
				const uint finalFrame = decoder.getFrameCount() - 1;
				// Smacker pixels and palette packets are stateful. Rebuild the
				// terminal frame from a cleared surface so the engine-added
				// Escape fast-forward cannot retain intermediate frame state.
				const Graphics::Surface *frame = decoder.restartAtFrame(finalFrame);
				if (frame) {
					presentFrame(frame, true);
					if (sequenceCallback)
						presentScreen();
					if (serviceSceneUi)
						_engine->getSceneAudio()->service(finalFrame + 1);
					completed = true;
					debugC(2, kDebugVideo,
						"Ripper: Escape reconstructed and presented final Smacker frame '%s' frame=%u; completing presentation",
						name.c_str(), finalFrame);
				}
			}
			if (!advanceToNextSegment && stoppedByUser && !_engine->shouldQuit())
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
				if (serviceSceneUi)
					_engine->getSceneAudio()->service(presentedFrames);
				if (sequenceCallback &&
						finishSequenceFramePresentation(decoder.getCurFrame() + 1))
					break;
				if (boundedSegment && (uint)decoder.getCurFrame() >= lastFrame) {
					if (boundedLoop) {
						// SeekSmackerPlaybackFrame in RunTubeSwitchScene resumes at
						// frame 15. Rebuild from the cleared initial surface before
						// presenting that frame so skipped blocks cannot retain stale
						// pixels from frame 46.
						const Graphics::Surface *loopFrame =
							decoder.restartAtFrame(boundedLoopStartFrame);
						if (!loopFrame) {
							warning("Ripper: could not loop bounded Smacker '%s' to frame=%u",
								name.c_str(), boundedLoopStartFrame);
							completed = false;
							break;
						}
						presentFrame(loopFrame, true);
						++presentedFrames;
						debugC(3, kDebugVideo,
							"Ripper: looped bounded Smacker '%s' frame=%u after endFrame=%u",
							name.c_str(), boundedLoopStartFrame, lastFrame);
						if (sequenceCallback &&
								finishSequenceFramePresentation(boundedLoopStartFrame + 1))
							break;
						continue;
					}
					debugC(2, kDebugVideo,
						"Ripper: completed bounded Smacker segment '%s' frame=%u..%u",
						name.c_str(), firstFrame, lastFrame);
					break;
				}
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
			presentScreen();
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
	if (toolbarPaused) {
		if (externalAudio)
			_mixer->pauseHandle(*externalAudio, false);
		decoder.pauseVideo(false);
	}
	if (paused) {
		_mixer->pauseAll(false);
		decoder.pauseVideo(false);
		logPlaybackPause("Space cleanup", false, decoder, name, externalAudio);
	}
	if (synchronizeToTimeline && completed) {
		const uint32 elapsedMs = externalAudio && _mixer->isSoundHandleActive(*externalAudio) ?
			_mixer->getSoundElapsedTime(*externalAudio) : g_system->getMillis(true) - timelineStartMillis;
		debugC(2, kDebugVideo, "Ripper: completed IAVF Smacker '%s' targetMs=%u audioElapsedMs=%u",
			name.c_str(),
			(uint32)((uint64)frameAudioOffsets->back() * 1000 / audioByteRate),
			elapsedMs);
	}
	if (transparentFirstPixel && !transparentBacking.empty() && !retainFinalTransparentFrame) {
		for (uint row = 0; row < outputHeight; ++row)
			g_system->copyRectToScreen(transparentBacking.data() + row * outputWidth,
				outputWidth, x, y + row, outputWidth, 1);
		presentScreen();
		debugC(2, kDebugVideo,
			"Ripper: restored transparent Smacker backing media='%s' rect=%d,%d,%ux%u",
			name.c_str(), x, y, outputWidth, outputHeight);
	} else if (transparentFirstPixel && retainFinalTransparentFrame) {
		debugC(2, kDebugVideo,
			"Ripper: retained final transparent Smacker frame media='%s' rect=%d,%d,%ux%u",
			name.c_str(), x, y, outputWidth, outputHeight);
	}
	decoder.close();
	return completed;
}

bool MediaPlayer::playIavf(Common::SeekableReadStream &stream, const Common::String &name,
		bool allowEscSpace, int overrideX, int overrideY, int overrideOriginY,
		bool serviceSceneUi, bool rememberVideoPalette) {
	IavfMovie movie;
	if (!parseIavf(stream, name, movie)) {
		warning("Ripper: invalid IAVF presentation '%s'", name.c_str());
		return false;
	}

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
	for (uint i = 0; i < movie.segments.size() && !_engine->shouldQuit(); ++i) {
		if (movie.segments[i].clearDisplayBefore) {
			// RunPacketizedMediaPlaybackCore at 0x5b592 handles IAVF opcode 0x68 by
			// clearing the active logical page through display command 0x14.
			g_system->fillScreen(0);
			presentScreen();
			debugC(2, kDebugVideo,
				"Ripper: IAVF '%s' cleared display before segment=%u from opcode 0x68",
				name.c_str(), i);
		}
		Common::SeekableReadStream *smacker = rebuildSmackerStream(movie.segments[i]);
		const int segmentX = overrideX != -1 ? overrideX : movie.segments[i].x;
		const int segmentY = overrideY != -1 ? overrideY : movie.segments[i].y;
		const int segmentOriginY = overrideY != -1 ? overrideOriginY : 0;
		if (overrideX != -1 || overrideY != -1) {
			// PreparePacketizedMediaPlaybackBranchSetup at 0x5b237 replaces each
			// packetized branch coordinate independently when the caller supplies
			// an override. Scene-space Y receives the retained viewport origin once.
			debugC(2, kDebugVideo,
				"Ripper: IAVF '%s' segment=%u coordinate override raw=%d,%d caller=%d,%d originY=%d effective=%d,%d",
				name.c_str(), i, movie.segments[i].x, movie.segments[i].y,
				overrideX, overrideY, segmentOriginY, segmentX,
				segmentY + segmentOriginY);
		}
		bool stoppedByUser = false;
		bool advanceSegment = false;
		Common::Array<uint32> relativeAudioOffsets;
		const Common::Array<uint32> *frameAudioOffsets = &movie.segments[i].frameAudioOffsets;
		if (audioTimelineOffset != 0) {
			relativeAudioOffsets.reserve(frameAudioOffsets->size());
			for (uint frame = 0; frame < frameAudioOffsets->size(); ++frame) {
				relativeAudioOffsets.push_back((*frameAudioOffsets)[frame] >= audioTimelineOffset ?
					(*frameAudioOffsets)[frame] - audioTimelineOffset : 0);
			}
			frameAudioOffsets = &relativeAudioOffsets;
		}
		SmackerPlaybackPlan plan;
		plan.retailRoute = "RunPacketizedMediaPlaybackCore@0x5b592";
		plan.input.allowEscSpace = allowEscSpace;
		plan.placement.x = segmentX;
		plan.placement.y = segmentY;
		plan.timeline.externalAudio = audioActive ? &audioHandle : nullptr;
		plan.input.stoppedByUser = &stoppedByUser;
		plan.timeline.frameAudioOffsets = frameAudioOffsets;
		plan.timeline.audioByteRate = audioByteRate;
		plan.timeline.timelineStartMillis = timelineStartMillis;
		plan.placement.displayScale = kAutoPacketizedDisplayScale;
		plan.palette.patchInterfacePalette = false;
		plan.placement.originY = segmentOriginY;
		plan.input.serviceSceneUi = serviceSceneUi;
		plan.palette.rememberVideoPalette = rememberVideoPalette;
		plan.input.advanceSegment = &advanceSegment;
		if (!smacker || !playSmacker(smacker,
				Common::String::format("%s#%u", name.c_str(), i), plan)) {
			result = false;
			break;
		}
		if (stoppedByUser)
			break;
		if (_engine->getScripts()->hasPendingSceneTransition())
			break;
		if (advanceSegment) {
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
	// RunPacketizedMediaPlaybackCore at 0x5b592 does not tear down the
	// presentation when opcode 0x70 ends packet dispatch. It waits until
	// GetManagedAudioTriggerActiveDescriptor reports that the managed-audio
	// tail has completed. Some IAVF files, including KA_BOOK.AVI, have several
	// seconds of audio after their final rendered frame.
	if (result && completedFinalSegment && audioActive &&
			!_engine->shouldQuit() && !_engine->getScripts()->hasPendingSceneTransition() &&
			_mixer->isSoundHandleActive(audioHandle)) {
		const uint32 targetAudioMs = (uint32)((uint64)(movie.audio.size() - audioTimelineOffset) *
			1000 / audioByteRate);
		debugC(2, kDebugVideo,
			"Ripper: retaining final IAVF display '%s' for managed-audio tail targetMs=%u elapsedMs=%u",
			name.c_str(), targetAudioMs, _mixer->getSoundElapsedTime(audioHandle));
		bool paused = false;
		bool skipped = false;
		while (!_engine->shouldQuit() && _mixer->isSoundHandleActive(audioHandle)) {
			if (_input->pollEvents()) {
				_engine->quitGame();
				result = false;
				break;
			}
			if (allowEscSpace && _input->hasPendingKey()) {
				const uint16 command = _input->consumeKey();
				if (command == 0x1b || command == 0x4d00) {
					skipped = true;
					debugC(2, kDebugVideo,
						"Ripper: %s completed final IAVF managed-audio tail '%s'",
						command == 0x1b ? "Escape" : "Right Arrow", name.c_str());
					break;
				}
				if (command == 0x20) {
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

bool MediaPlayer::play(const Common::String &path, bool allowEscSpace, int x, int y,
		bool sceneViewport) {
	// ExecutePresentationEntry at 0x1652a routes WAV entries to
	// PlayBlockingAudioClip at 0x1f0ea before considering either video path.
	// The original blocking-audio loop always permits Escape, independently of
	// the presentation control argument used by AVI and Smacker playback.
	if (path.hasSuffixIgnoreCase(".wav")) {
		debugC(2, kDebugAudio,
			"Ripper: dispatching media presentation '%s' as blocking audio",
			path.c_str());
		const bool result = playBlockingAudio(path);
		_input->drainKeys();
		return result;
	}

	Common::String source;
	Common::SeekableReadStream *stream = openSource(path, kSourceDirectFile, source);
	if (!stream) {
		warning("Ripper: could not open media '%s'", path.c_str());
		return false;
	}

	// ExecutePresentationEntry at 0x1652a passes its keyboard-control flag to
	// RunMediaPresentation at 0x168af. Controlled IAVF media redraws the saved
	// logical page afterward; uncontrolled media leaves its final frame visible.
	const MediaFormat format = detectMediaFormat(*stream);
	const bool isSmacker = format == kMediaFormatSmacker;
	const bool isIavf = format == kMediaFormatIavf;
	const bool restoreIavfDisplay = isIavf && allowEscSpace;
	IndexedDisplaySnapshot displayContext;
	const bool displayContextCaptured = restoreIavfDisplay && displayContext.capture();
	const bool rememberIavfPalette = !displayContextCaptured;
	if (isIavf) {
		debugC(2, kDebugVideo,
			"Ripper: IAVF display policy media='%s' keyboardControls=%d restore=%d captured=%d rememberPalette=%d size=%dx%d",
			path.c_str(), allowEscSpace, restoreIavfDisplay, displayContextCaptured,
			rememberIavfPalette, displayContext.bounds().width(),
			displayContext.bounds().height());
	}

	bool result = false;
	if (isSmacker) {
		// RunMediaSequence at 0x1e516 draws direct scene-script Smackers against
		// the active scene display descriptor. Its logical y=0 is the top of the
		// 640x300 scene page, which begins at physical y=50 in ScummVM's retained
		// 640x400 framebuffer.
		const int originY = sceneViewport ? kScenePresentationTop : 0;
		SmackerPlaybackPlan plan;
		plan.input.allowEscSpace = allowEscSpace;
		plan.placement.x = x;
		plan.placement.y = y;
		plan.placement.originY = originY;
		result = playValidatedSmacker(stream, path, "presentation", plan);
	} else if (isIavf) {
		const int originY = sceneViewport ? kScenePresentationTop : 0;
		// RunWacVoiceLockPuzzleScene at 0x24ba4 fades ACCESED.AVI out before
		// returning to the restored WAC page. A presentation whose indexed page
		// and palette will be restored must not replace the source palette later
		// used to rebuild the surrounding scene and interface bands.
		result = playIavf(*stream, path, allowEscSpace, x, y, originY, false,
			rememberIavfPalette);
		delete stream;
	} else {
		warning("Ripper: unsupported media format for '%s' format=%s",
			path.c_str(), mediaFormatName(format));
		delete stream;
	}
	if (displayContextCaptured && !_engine->shouldQuit()) {
		const bool restored = displayContext.restore();
		debugC(restored ? 2 : 1, kDebugVideo,
			"Ripper: restored script media display context media='%s' success=%d size=%dx%d",
			path.c_str(), restored, displayContext.bounds().width(),
			displayContext.bounds().height());
		if (!restored)
			result = false;
	} else if (isIavf && !restoreIavfDisplay && result && !_engine->shouldQuit()) {
		debugC(2, kDebugVideo,
			"Ripper: retained final IAVF display media='%s' keyboardControls=%d",
			path.c_str(), allowEscSpace);
	}

	_input->drainKeys();
	return result;
}

bool MediaPlayer::playWacMedia(const Common::String &path, int x, int y) {
	debugC(1, kDebugVideo,
		"Ripper: entering WAC media presentation media='%s' position=%d,%d palette=10..149",
		path.c_str(), x, y);
	SmackerPlaybackPlan plan;
	plan.placement.x = x;
	plan.placement.y = y;
	plan.palette.patchInterfacePalette = false;
	plan.palette.patchWacMediaPalette = true;
	Common::String source;
	const bool result = playValidatedSmacker(
		openSource(path, kSourceDirectFile, source), path, "WAC", plan);
	_input->drainKeys();
	return result;
}

bool MediaPlayer::playWacInterfaceSequence(const Common::String &path,
		const Common::Rect &centerBounds,
		uint loopStartFrame, MediaSequenceCallback *callback, uint16 *command) {
	debugC(1, kDebugVideo,
		"Ripper: entering WAC interface sequence media='%s' centerBounds=%d,%d,%d,%d palette=10..149 loopStartFrame=%u callback=%d",
		path.c_str(), centerBounds.left, centerBounds.top, centerBounds.right,
		centerBounds.bottom, loopStartFrame, callback != nullptr);
	SmackerPlaybackPlan plan;
	plan.placement.centerBounds = centerBounds;
	plan.palette.patchInterfacePalette = false;
	plan.palette.patchWacMediaPalette = true;
	plan.loop.loopStartFrame = loopStartFrame;
	plan.callback.sequenceCallback = callback;
	plan.callback.sequenceCommand = command;
	Common::String source;
	return playValidatedSmacker(
		openSource(path, kSourceInterfaceLibrary, source), path,
		"WAC interface sequence", plan);
}

bool MediaPlayer::playWacInterfaceSequenceStream(Common::SeekableReadStream *stream,
		const Common::String &name, const Common::Rect &centerBounds,
		uint loopStartFrame, MediaSequenceCallback *callback, uint16 *command) {
	debugC(1, kDebugVideo,
		"Ripper: entering streamed WAC interface sequence media='%s' centerBounds=%d,%d,%d,%d palette=10..149 loopStartFrame=%u callback=%d",
		name.c_str(), centerBounds.left, centerBounds.top, centerBounds.right,
		centerBounds.bottom, loopStartFrame, callback != nullptr);
	SmackerPlaybackPlan plan;
	plan.placement.centerBounds = centerBounds;
	plan.palette.patchInterfacePalette = false;
	plan.palette.patchWacMediaPalette = true;
	plan.loop.loopStartFrame = loopStartFrame;
	plan.callback.sequenceCallback = callback;
	plan.callback.sequenceCommand = command;
	return playValidatedSmacker(stream, name,
		"streamed WAC interface sequence", plan);
}

bool MediaPlayer::playInterfaceSequence(const Common::String &path, int x, int y,
		Common::Array<byte> &sourcePalette) {
	debugC(1, kDebugVideo,
		"Ripper: entering interface media presentation media='%s' position=%d,%d transparent=first-pixel",
		path.c_str(), x, y);
	SmackerPlaybackPlan plan;
	plan.placement.x = x;
	plan.placement.y = y;
	// RunTake2IniSliderSetupMenu at 0x1989b enables RunMediaSequence's captured
	// backing path. RunMediaSequence snapshots decoded pixel zero at 0x1e996 and
	// its flagged blit at 0x1e9d3 uses that value as the transparent color.
	plan.rendering.transparentFirstPixel = true;
	// Cleanup at 0x1eaf9 releases the temporary page without redrawing the
	// backing. RunTake2IniSliderSetupMenu restores its outer capture at 0x1a108.
	plan.rendering.retainFinalTransparentFrame = true;
	// The active scene keeps the shared palette bands installed by
	// RunMediaSequence at 0x1e8e9. The movie's chroma-key palette is not display
	// state.
	plan.palette.patchInterfacePalette = false;
	plan.palette.preserveDisplayPalette = true;
	plan.palette.sourcePalette = &sourcePalette;
	plan.palette.rememberVideoPalette = false;
	Common::String source;
	return playValidatedSmacker(
		openSource(path, kSourceInterfaceLibrary, source), path,
		"interface sequence", plan);
}

bool MediaPlayer::playPuzzleSequence(const Common::String &path, uint loopStartFrame,
		MediaSequenceCallback *callback, uint16 *command) {
	debugC(1, kDebugVideo,
		"Ripper: entering puzzle Smacker sequence media='%s' loopStartFrame=%u callback=%d",
		path.c_str(), loopStartFrame, callback != nullptr);
	SmackerPlaybackPlan plan;
	plan.placement.x = 0;
	plan.placement.y = 0;
	plan.placement.originY = kScenePresentationTop;
	plan.loop.loopStartFrame = loopStartFrame;
	plan.callback.sequenceCallback = callback;
	plan.callback.sequenceCommand = command;
	Common::String source;
	return playValidatedSmacker(
		openSource(path, kSourceDirectFile, source), path,
		"puzzle sequence", plan);
}

bool MediaPlayer::playPuzzleSequenceStream(Common::SeekableReadStream *stream,
		const Common::String &name, int x, int y, uint loopStartFrame,
		MediaSequenceCallback *callback, uint16 *command) {
	debugC(1, kDebugVideo,
		"Ripper: entering archived puzzle Smacker sequence media='%s' "
		"position=%d,%d loopStartFrame=%u callback=%d",
		name.c_str(), x, y, loopStartFrame, callback != nullptr);
	SmackerPlaybackPlan plan;
	plan.placement.x = x;
	plan.placement.y = y;
	plan.placement.originY = kScenePresentationTop;
	plan.input.serviceSceneUi = true;
	plan.loop.loopStartFrame = loopStartFrame;
	plan.callback.sequenceCallback = callback;
	plan.callback.sequenceCommand = command;
	return playValidatedSmacker(stream, name,
		"archived puzzle sequence", plan);
}

bool MediaPlayer::playSceneStream(Common::SeekableReadStream *stream,
		const Common::String &name, int x, int y, bool allowEscSpace) {
	if (!stream)
		return false;
	const MediaFormat format = detectMediaFormat(*stream);
	if (format != kMediaFormatSmacker && format != kMediaFormatIavf) {
		warning("Ripper: unsupported archived scene media '%s' format=%s",
			name.c_str(), mediaFormatName(format));
		delete stream;
		return false;
	}

	debugC(1, kDebugVideo,
		"Ripper: entering archived scene presentation media='%s' "
		"mode=%s position=%d,%d controls=%d",
		name.c_str(), mediaFormatName(format), x, y, allowEscSpace);
	bool result = false;
	if (format == kMediaFormatSmacker) {
		SmackerPlaybackPlan plan;
		plan.input.allowEscSpace = allowEscSpace;
		plan.placement.x = x;
		plan.placement.y = y;
		plan.placement.originY = kScenePresentationTop;
		result = playValidatedSmacker(stream, name,
			"archived scene", plan);
	} else {
		// RunShockLeverPuzzleScene at 0x3affb passes each archived outcome
		// member through RunMediaPresentation at 0x168af. Its controlled IAVF
		// path restores the puzzle page after the presentation completes.
		IndexedDisplaySnapshot displayContext;
		const bool captured = displayContext.capture();
		result = playIavf(*stream, name, allowEscSpace, x, y,
			kScenePresentationTop, false, !captured);
		delete stream;
		if (captured && !_engine->shouldQuit()) {
			const bool restored = displayContext.restore();
			debugC(restored ? 2 : 1, kDebugVideo,
				"Ripper: restored archived scene display media='%s' "
				"success=%d size=%dx%d",
				name.c_str(), restored, displayContext.bounds().width(),
				displayContext.bounds().height());
			if (!restored)
				result = false;
		}
	}
	_input->drainKeys();
	return result;
}

bool MediaPlayer::playPuzzleSequenceSegment(const Common::String &path, uint firstFrame,
		uint lastFrame, int x, int y, MediaSequenceCallback *callback, uint16 *command,
		uint boundedLoopStartFrame) {
	debugC(1, kDebugVideo,
		"Ripper: entering puzzle Smacker segment media='%s' frames=%u..%u loopStart=%d position=%d,%d callback=%d",
		path.c_str(), firstFrame, lastFrame,
		boundedLoopStartFrame == 0xffffffff ? -1 : (int)boundedLoopStartFrame,
		x, y, callback != nullptr);
	SmackerPlaybackPlan plan;
	plan.placement.x = x;
	plan.placement.y = y;
	plan.placement.originY = kScenePresentationTop;
	plan.callback.sequenceCallback = callback;
	plan.callback.sequenceCommand = command;
	plan.frames.firstFrame = firstFrame;
	plan.frames.lastFrame = lastFrame;
	plan.loop.boundedLoopStartFrame = boundedLoopStartFrame;
	Common::String source;
	return playValidatedSmacker(
		openSource(path, kSourceDirectFile, source), path,
		"puzzle segment", plan);
}

bool MediaPlayer::playCombatSequence(const Common::String &path,
		MediaSequenceCallback *callback, uint16 *command) {
	return playScaledInteractiveSequence(path, "combat", callback, command);
}

bool MediaPlayer::playShootingGallerySequence(const Common::String &path,
		MediaSequenceCallback *callback, uint16 *command) {
	return playScaledInteractiveSequence(path, "shooting-gallery", callback, command);
}

bool MediaPlayer::playBlobShooterSequence(const Common::String &path,
		MediaSequenceCallback *callback, uint16 *command) {
	// RunBlobShooterScene at 0x338a4 advances KJ.SMK continuously while its
	// local actor loop remains active. Reconstruct frame one when the short
	// background presentation wraps.
	return playScaledInteractiveSequence(path, "blob-shooter", callback, command, 1);
}

bool MediaPlayer::playScaledInteractiveSequence(const Common::String &path,
		const char *description, MediaSequenceCallback *callback, uint16 *command,
		uint loopStartFrame) {
	debugC(1, kDebugVideo,
		"Ripper: entering %s Smacker sequence media='%s' callback=%d",
		description, path.c_str(), callback != nullptr);
	SmackerPlaybackPlan plan;
	plan.placement.x = 0;
	plan.placement.y = 0;
	plan.placement.displayScale = kAutoPacketizedDisplayScale;
	// RunCombatEncounterScene at 0x31436 and RunKdShootingGalleryScene at
	// 0x3288e retain interface bitmap colors while their active Smacker
	// palettes change.
	plan.palette.patchInterfacePalette = true;
	plan.loop.loopStartFrame = loopStartFrame;
	plan.callback.sequenceCallback = callback;
	plan.callback.sequenceCommand = command;
	plan.palette.rememberVideoPalette = false;
	Common::String source;
	return playValidatedSmacker(
		openSource(path, kSourceDirectFile, source), path,
		description, plan);
}

bool MediaPlayer::playTransparentSmackerOverlay(const Common::String &path, int x, int y) {
	debugC(1, kDebugVideo,
		"Ripper: entering transparent Smacker overlay media='%s' position=%d,%d",
		path.c_str(), x, y);
	SmackerPlaybackPlan plan;
	plan.placement.x = x;
	plan.placement.y = y;
	plan.placement.originY = kScenePresentationTop;
	plan.rendering.transparentFirstPixel = true;
	Common::String source;
	return playValidatedSmacker(
		openSource(path, kSourceDirectFile, source), path,
		"transparent overlay", plan);
}

bool MediaPlayer::playScene(const Common::String &path, int x, int y, bool firstFrameOnly,
		bool loopUntilInput, bool allowEscSpace, MediaSequenceCallback *callback,
		uint16 *command) {
	debugC(1, kDebugVideo,
		"Ripper: entering scene presentation media='%s' firstFrameOnly=%d loopUntilInput=%d controls=%d callback=%d",
		path.c_str(), firstFrameOnly, loopUntilInput, allowEscSpace, callback != nullptr);
	Common::String source;
	Common::SeekableReadStream *stream =
		openSource(path, kSourceConfiguredPath, source);
	if (!stream) {
		warning("Ripper: could not open scene media '%s'", path.c_str());
		return false;
	}

	// HandleSceneEntryMediaPreviewOrPrompt at 0x15b03 supplies a target of one to
	// MediaSequenceCounterEqualsTarget at 0x15ac8. RunMediaSequence at 0x1e516
	// calls it after presenting the frame, so this path retains frame one onscreen.
	const uint frameLimit = firstFrameOnly ? 1 : 0;
	const MediaFormat format = detectMediaFormat(*stream);
	const bool isSmacker = format == kMediaFormatSmacker;
	const bool isIavf = format == kMediaFormatIavf;
	debugC(2, kDebugVideo,
		"Ripper: scene media '%s' mode=%s scriptPosition=%d,%d originY=%d controls=%d",
		path.c_str(), firstFrameOnly ? "first-frame-preview" : "sequence", x, y,
		kScenePresentationTop, allowEscSpace);
	bool result = false;
	_stopSceneOnMouse = loopUntilInput && !firstFrameOnly;
	const bool loop = loopUntilInput && !firstFrameOnly;
	uint pass = 0;
	do {
		const bool repeatedLoopPass = pass++ != 0;
		if (repeatedLoopPass) {
			stream = openSource(path, kSourceConfiguredPath, source);
			if (!stream)
				return false;
		}
	if (isSmacker) {
		SmackerPlaybackPlan plan;
		plan.input.allowEscSpace = allowEscSpace;
		plan.placement.x = x;
		plan.placement.y = y;
		plan.frames.frameLimit = frameLimit;
		plan.placement.originY = kScenePresentationTop;
		plan.input.serviceSceneUi = true;
		plan.loop.loopFromStart = loop;
		plan.callback.sequenceCallback = callback;
		plan.callback.sequenceCommand = command;
		result = playValidatedSmacker(stream, path, "scene", plan);
		if (!result && _stopSceneOnMouse && _input->peekMouseState().pressed != 0) {
			result = true;
			debugC(1, kDebugVideo,
				"Ripper: interactive scene media stopped by mouse; returning to hotspot polling");
		}
	} else if (isIavf && !firstFrameOnly) {
		if (callback) {
			warning("Ripper: scene callback is unsupported for packetized media '%s'",
				path.c_str());
			delete stream;
			result = false;
			break;
		}
		result = playIavf(*stream, path, allowEscSpace, x, y,
			kScenePresentationTop, true, true);
		delete stream;
	} else {
		warning("Ripper: unsupported scene media mode for '%s' format=%s",
			path.c_str(), mediaFormatName(format));
		delete stream;
	}
	} while (loop && isIavf && !_input->peekMouseState().pressed &&
		!_engine->shouldQuit() && result &&
		!_engine->getScripts()->hasPendingSceneTransition());
	_input->drainKeys();
	_stopSceneOnMouse = false;
	return result;
}

} // End of namespace Ripper
