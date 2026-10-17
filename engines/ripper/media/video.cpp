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

#include "common/array.h"
#include "common/debug.h"
#include "common/system.h"
#include "graphics/blit.h"
#include "graphics/paletteman.h"
#include "graphics/surface.h"

#include "ripper/cursor.h"
#include "ripper/detection.h"
#include "ripper/display.h"
#include "ripper/input.h"
#include "ripper/media/smacker_decoder.h"
#include "ripper/ripper.h"
#include "ripper/scene_audio.h"
#include "ripper/settings.h"
#include "ripper/script.h"
#include "ripper/toolbar.h"

namespace Ripper {

namespace {

static const uint16 kHelpCommand = 0x3b00;

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
	if (!_engine->isDemo() &&
			_engine->getSettings()->handlePresentationTextCommand(
				_input->peekKey(), "media-playback")) {
		_input->consumeKey();
		return true;
	}
	if (allowSceneHelp && !suppressSceneMouseStop && !_engine->isDemo() &&
			_engine->getScripts()->isAwaitingInteraction() &&
			_input->peekKey() == kReplayPreviousSceneCommand) {
		_input->consumeKey();
		const bool externalAudioActive = externalAudio &&
			_mixer->isSoundHandleActive(*externalAudio);
		decoder.pauseVideo(true);
		if (externalAudioActive)
			_mixer->pauseHandle(*externalAudio, true);
		debugC(1, kDebugVideo,
			"Ripper: Ctrl+R suspended interactive media='%s' frame=%d "
			"paused=%d externalAudio=%d",
			name.c_str(), decoder.getCurFrame(), paused,
			externalAudioActive);
		const bool replayed =
			_engine->getScripts()->replayPreviousInteractionScene(
				"interactive-media");
		if (externalAudioActive)
			_mixer->pauseHandle(*externalAudio, paused);
		decoder.pauseVideo(paused);
		debugC(1, kDebugVideo,
			"Ripper: Ctrl+R restored interactive media='%s' frame=%d "
			"paused=%d replayed=%d",
			name.c_str(), decoder.getCurFrame(), paused, replayed);
		return true;
	}
	if (allowSceneHelp && _input->peekKey() == kHelpCommand) {
		_input->consumeKey();
		decoder.pauseVideo(true);
		_mixer->pauseAll(true);
		logPlaybackPause("Help", true, decoder, name, externalAudio);
		const bool helpDisplayed =
			_engine->getScripts()->triggerHelpHotkey("interactive-media");
		_mixer->pauseAll(false);
		decoder.pauseVideo(false);
		logPlaybackPause("Help", false, decoder, name, externalAudio);
		return helpDisplayed;
	}
	if (allowSceneHelp) {
		const uint16 command = _input->peekKey();
		const int actionIndex =
			_engine->getScripts()->resolveToolbarActionKey(command);
		if (actionIndex >= 0) {
			_input->consumeKey();
			const bool externalAudioActive = externalAudio &&
				_mixer->isSoundHandleActive(*externalAudio);
			decoder.pauseVideo(true);
			if (externalAudioActive)
				_mixer->pauseHandle(*externalAudio, true);
			debugC(2, kDebugVideo,
				"Ripper: toolbar hotkey suspended interactive media='%s' frame=%d "
				"command=0x%04x action=%u paused=%d toolbarPaused=%d externalAudio=%d",
				name.c_str(), decoder.getCurFrame(), command, actionIndex + 1,
				paused, suppressSceneMouseStop, externalAudioActive);
			const bool dispatched = _engine->getScripts()->dispatchToolbarActionKey(
				actionIndex, command, "interactive-media");
			const bool restorePaused = paused || suppressSceneMouseStop;
			if (externalAudioActive)
				_mixer->pauseHandle(*externalAudio, restorePaused);
			decoder.pauseVideo(restorePaused);
			debugC(2, kDebugVideo,
				"Ripper: toolbar hotkey restored interactive media='%s' frame=%d "
				"command=0x%04x action=%u paused=%d dispatched=%d",
				name.c_str(), decoder.getCurFrame(), command, actionIndex + 1,
				restorePaused, dispatched);
			return dispatched;
		}
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
	const uint frameCount = decoder.getFrameCount();
	if (frameCount == 0) {
		warning("Ripper: invalid Smacker segment '%s' frames=%u..%u count=0",
			name.c_str(), firstFrame, lastFrame);
		return false;
	}
	const bool boundedSegment = firstFrame != 0 || lastFrame != 0xffffffff;
	const bool boundedLoop = boundedLoopStartFrame != 0xffffffff;
	if (lastFrame == 0xffffffff)
		lastFrame = frameCount - 1;
	if (firstFrame > lastFrame || lastFrame >= frameCount ||
			(boundedLoop && (!boundedSegment || boundedLoopStartFrame < firstFrame ||
				boundedLoopStartFrame > lastFrame))) {
		warning("Ripper: invalid Smacker segment '%s' frames=%u..%u count=%u",
			name.c_str(), firstFrame, lastFrame, frameCount);
		return false;
	}
	// RIPPER.LE chooses scale per packetized branch, not from the IAVF container.
	// In 640x400 mode, branches smaller than 321x201 use 2:1 scaling.
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
	byte decoderPaletteBase[256 * 3];
	bool decoderPaletteBaseValid = false;
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
		const bool dirtyPalette = decoder.hasDirtyPalette();
		if (forcePalette || dirtyPalette) {
			if (dirtyPalette) {
				const byte *decoderPalette = decoder.getPalette();
				if (!decoderPalette) {
					warning("Ripper: Smacker '%s' reported a dirty palette without data",
						name.c_str());
					completed = false;
					return;
				}
				memcpy(decoderPaletteBase, decoderPalette,
					sizeof(decoderPaletteBase));
				decoderPaletteBaseValid = true;
			} else if (!decoderPaletteBaseValid) {
				// Packetized branches may omit an initial palette record and retain
				// the palette installed by the preceding branch. Cache that inherited
				// base before a callback applies temporary interface or combat colors.
				g_system->getPaletteManager()->grabPalette(decoderPaletteBase, 0, 256);
				decoderPaletteBaseValid = true;
				debugC(3, kDebugVideo,
					"Ripper: Smacker '%s' inherited display palette for callback base",
					name.c_str());
			}
			byte palette[256 * 3];
			if (sourcePalette) {
				sourcePalette->resize(sizeof(palette));
				memcpy(sourcePalette->data(), decoderPaletteBase, sizeof(palette));
			}
			if (preserveDisplayPalette)
				return;
			if (patchWacMediaPalette) {
				g_system->getPaletteManager()->grabPalette(palette, 0, 256);
				memcpy(palette + 10 * 3, decoderPaletteBase + 10 * 3, 140 * 3);
			} else {
				memcpy(palette, decoderPaletteBase, sizeof(palette));
			}
			if (patchInterfacePalette && !patchWacMediaPalette)
				_engine->applySharedPalettePatch(palette, 256);
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
		// Callbacks may add overlays; present only after they finish composing.
		if (!sequenceCallback)
			presentScreen();
	};
	// Hide the shared cursor before AVI playback.
	// A branch callback may own a cursor during playback.
	if (!serviceSceneUi &&
			(!sequenceCallback || !sequenceCallback->keepsCursorVisible()))
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
			// Combat transforms the active Smacker palette each frame; restore that
			// base before applying the next callback transform.
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
		// Decode GA_TUBE frames 0..14 before showing frame 15. Starting
		// from a seek surface would lose delta-coded pixels and palette state.
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
				// frame.
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
		// The toolbar blocks per-frame callback while it owns the
		// pointer, so Smacker playback must pause too.
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
				externalAudio, toolbarOwnsInput,
				serviceSceneUi && !callbackOwnsInput, name)) {
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
		if (_engine->getScripts()->hasPendingRuntimeRestore() ||
				(serviceSceneUi && !callbackOwnsInput &&
					_engine->getScripts()->hasPendingSceneTransition())) {
			debugC(1, kDebugScene,
				"Ripper: interactive media '%s' returning hotkey runtime transition",
				name.c_str());
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


} // End of namespace Ripper
