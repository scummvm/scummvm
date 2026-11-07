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
#include "ripper/media/playback_pause.h"
#include "ripper/media/smacker_decoder.h"
#include "ripper/ripper.h"
#include "ripper/scene_audio.h"
#include "ripper/settings.h"
#include "ripper/script.h"
#include "ripper/toolbar.h"

namespace Ripper {

namespace {

static const uint16 kHelpCommand = 0x3b00;

class SmackerFrameRenderer : private Common::NonCopyable {
public:
	SmackerFrameRenderer(RipperEngine *engine, Video::SmackerDecoder &decoder,
			const Common::String &name, const SmackerPlaybackPlan &plan,
			int x, int y, uint scale) :
			_engine(engine), _decoder(decoder), _name(name), _plan(plan),
			_x(x), _y(y), _scale(scale), _width(decoder.getWidth() * scale),
			_height(decoder.getHeight() * scale), _paletteBaseValid(false) {}

	bool captureBacking() {
		if (_plan.rendering.transparentFirstPixel) {
			Graphics::Surface *screen = g_system->lockScreen();
			if (!screen || screen->format.bytesPerPixel != 1 || _x < 0 || _y < 0 ||
					_x + (int)_width > screen->w || _y + (int)_height > screen->h) {
				if (screen)
					g_system->unlockScreen();
				warning("Ripper: transparent Smacker overlay '%s' is outside the indexed display",
					_name.c_str());
				return false;
			}
			_backing.resize(_width * _height);
			for (uint row = 0; row < _height; ++row)
				memcpy(_backing.data() + row * _width,
					screen->getBasePtr(_x, _y + row), _width);
			g_system->unlockScreen();
			debugC(2, kDebugVideo,
				"Ripper: captured transparent Smacker backing media='%s' rect=%d,%d,%ux%u",
				_name.c_str(), _x, _y, _width, _height);
		}
		return true;
	}

	bool applyPalette(bool forcePalette) {
		const bool dirtyPalette = _decoder.hasDirtyPalette();
		if (forcePalette || dirtyPalette) {
			if (dirtyPalette) {
				const byte *decoderPalette = _decoder.getPalette();
				if (!decoderPalette) {
					warning("Ripper: Smacker '%s' reported a dirty palette without data",
						_name.c_str());
					return false;
				}
				memcpy(_paletteBase, decoderPalette,
					sizeof(_paletteBase));
				_paletteBaseValid = true;
			} else if (!_paletteBaseValid) {
				// Packetized branches may omit an initial palette record and retain
				// the palette installed by the preceding branch. Cache that inherited
				// base before a callback applies temporary interface or combat colors.
				g_system->getPaletteManager()->grabPalette(_paletteBase, 0, 256);
				_paletteBaseValid = true;
				debugC(3, kDebugVideo,
					"Ripper: Smacker '%s' inherited display palette for callback base",
					_name.c_str());
			}
			byte palette[256 * 3];
			if (_plan.palette.sourcePalette) {
				_plan.palette.sourcePalette->resize(sizeof(palette));
				memcpy(_plan.palette.sourcePalette->data(), _paletteBase, sizeof(palette));
			}
			if (_plan.palette.preserveDisplayPalette)
				return true;
			if (_plan.palette.patchWacMediaPalette) {
				g_system->getPaletteManager()->grabPalette(palette, 0, 256);
				memcpy(palette + 10 * 3, _paletteBase + 10 * 3, 140 * 3);
			} else {
				memcpy(palette, _paletteBase, sizeof(palette));
			}
			if (_plan.palette.patchInterfacePalette && !_plan.palette.patchWacMediaPalette)
				_engine->applySharedPalettePatch(palette, 256);
			if (!_plan.palette.patchWacMediaPalette)
				_engine->getSettings()->applyVideoPalette(palette, 256,
					_plan.palette.rememberVideoPalette);
			g_system->getPaletteManager()->setPalette(palette, 0, 256);
		}
		return true;
	}

	bool draw(const Graphics::Surface *frame, bool forcePalette) {
		const bool paletteApplied = applyPalette(forcePalette);
		if (_plan.rendering.transparentFirstPixel) {
			Graphics::Surface *screen = g_system->lockScreen();
			if (!screen) {
				warning("Ripper: could not lock display for transparent Smacker overlay '%s'",
					_name.c_str());
				return false;
			}
			for (uint row = 0; row < _height; ++row)
				memcpy(screen->getBasePtr(_x, _y + row),
					_backing.data() + row * _width, _width);
			const byte transparentColor = *(const byte *)frame->getPixels();
			for (uint sourceY = 0; sourceY < (uint)frame->h; ++sourceY) {
				const byte *source = (const byte *)frame->getBasePtr(0, sourceY);
				for (uint sourceX = 0; sourceX < (uint)frame->w; ++sourceX) {
					if (source[sourceX] == transparentColor)
						continue;
					for (uint scaleY = 0; scaleY < _scale; ++scaleY) {
						byte *destination = (byte *)screen->getBasePtr(
							_x + sourceX * _scale,
							_y + sourceY * _scale + scaleY);
						memset(destination, source[sourceX], _scale);
					}
				}
			}
			g_system->unlockScreen();
		} else if (_scale == 1) {
			g_system->copyRectToScreen(frame->getPixels(), frame->pitch, _x, _y, frame->w, frame->h);
		} else {
			Graphics::Surface *screen = g_system->lockScreen();
			if (!screen) {
				warning("Ripper: could not lock display for scaled Smacker '%s'",
					_name.c_str());
				return false;
			}
			Graphics::scaleBlit((byte *)screen->getBasePtr(_x, _y),
				(const byte *)frame->getPixels(), screen->pitch, frame->pitch,
				_width, _height, frame->w, frame->h, frame->format);
			g_system->unlockScreen();
		}
		if (_plan.input.serviceSceneUi) {
			_engine->getScripts()->drawDialogueOverlay(true);
			_engine->getScripts()->drawBriefingOverlay();
		}
		// Callbacks may add overlays; present only after they finish composing.
		if (!_plan.callback.sequenceCallback)
			presentScreen();
		return paletteApplied;
	}

	void restoreBacking() {
		if (_plan.rendering.transparentFirstPixel && !_backing.empty() &&
				!_plan.rendering.retainFinalTransparentFrame) {
			for (uint row = 0; row < _height; ++row)
				g_system->copyRectToScreen(_backing.data() + row * _width,
					_width, _x, _y + row, _width, 1);
			presentScreen();
			debugC(2, kDebugVideo,
				"Ripper: restored transparent Smacker backing media='%s' rect=%d,%d,%ux%u",
				_name.c_str(), _x, _y, _width, _height);
		} else if (_plan.rendering.transparentFirstPixel &&
				_plan.rendering.retainFinalTransparentFrame) {
			debugC(2, kDebugVideo,
				"Ripper: retained final transparent Smacker frame media='%s' rect=%d,%d,%ux%u",
				_name.c_str(), _x, _y, _width, _height);
		}
	}

private:
	RipperEngine *_engine;
	Video::SmackerDecoder &_decoder;
	const Common::String &_name;
	const SmackerPlaybackPlan &_plan;
	int _x;
	int _y;
	uint _scale;
	uint _width;
	uint _height;
	byte _paletteBase[256 * 3];
	bool _paletteBaseValid;
	Common::Array<byte> _backing;
};

} // End of anonymous namespace

MediaPlayer::MediaPlayer(RipperEngine *engine, InputManager *input, Audio::Mixer *mixer) :
		_engine(engine), _input(input), _mixer(mixer) {
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
}

MediaPlayer::PlaybackInputResult MediaPlayer::servicePlaybackInput(
		Video::SmackerDecoder &decoder, const SmackerInputPolicy &policy,
		bool &paused, Audio::SoundHandle *externalAudio, bool toolbarOwnsInput,
		const Common::String &name) {
	RipperEngine::SaveLoadScope saveLoad(*_engine,
		policy.serviceSceneUi && _engine->canLoadGameStateCurrently());
	if (_input->pollEvents()) {
		_engine->quitGame();
		return kQuitPlayback;
	}
	if (!_engine->isDemo() &&
			_engine->getSettings()->handlePresentationTextCommand(
				_input->peekKey(), "media-playback")) {
		_input->consumeKey();
		return kContinuePlayback;
	}
	if (policy.serviceSceneUi && !toolbarOwnsInput && !_engine->isDemo() &&
			_engine->getScripts()->isAwaitingInteraction() &&
			_input->peekKey() == kReplayPreviousSceneCommand) {
		_input->consumeKey();
		bool replayed;
		{
			ScopedPlaybackPause pause(decoder, *_mixer, externalAudio);
			debugC(1, kDebugVideo,
				"Ripper: Ctrl+R suspended interactive media='%s' frame=%d "
				"paused=%d externalAudio=%d",
				name.c_str(), decoder.getCurFrame(), paused,
				pause.hasExternalAudio());
			replayed = _engine->getScripts()->replayPreviousInteractionScene(
				"interactive-media");
		}
		debugC(1, kDebugVideo,
			"Ripper: Ctrl+R restored interactive media='%s' frame=%d "
			"paused=%d replayed=%d",
			name.c_str(), decoder.getCurFrame(), decoder.isPaused(), replayed);
		return kContinuePlayback;
	}
	if (policy.serviceSceneUi && _input->peekKey() == kHelpCommand) {
		_input->consumeKey();
		decoder.pauseVideo(true);
		_mixer->pauseAll(true);
		logPlaybackPause("Help", true, decoder, name, externalAudio);
		const bool helpDisplayed =
			_engine->getScripts()->triggerHelpHotkey("interactive-media");
		_mixer->pauseAll(false);
		decoder.pauseVideo(false);
		logPlaybackPause("Help", false, decoder, name, externalAudio);
		return helpDisplayed ? kContinuePlayback : kStopPlayback;
	}
	if (policy.serviceSceneUi) {
		const uint16 command = _input->peekKey();
		const int actionIndex =
			_engine->getScripts()->resolveToolbarActionKey(command);
		if (actionIndex >= 0) {
			_input->consumeKey();
			bool dispatched;
			{
				ScopedPlaybackPause pause(decoder, *_mixer, externalAudio);
				debugC(2, kDebugVideo,
					"Ripper: toolbar hotkey suspended interactive media='%s' frame=%d "
					"command=0x%04x action=%u paused=%d toolbarPaused=%d externalAudio=%d",
					name.c_str(), decoder.getCurFrame(), command, actionIndex + 1,
					paused, toolbarOwnsInput, pause.hasExternalAudio());
				dispatched = _engine->getScripts()->dispatchToolbarActionKey(
					actionIndex, command, "interactive-media");
			}
			debugC(2, kDebugVideo,
				"Ripper: toolbar hotkey restored interactive media='%s' frame=%d "
				"command=0x%04x action=%u paused=%d dispatched=%d",
				name.c_str(), decoder.getCurFrame(), command, actionIndex + 1,
				decoder.isPaused(), dispatched);
			return dispatched ? kContinuePlayback : kStopPlayback;
		}
	}
	if (!policy.allowEscSpace || !_input->hasPendingKey())
		return policy.stopOnMouse && !toolbarOwnsInput &&
			_input->peekMouseState().pressed != 0 ? kStopForMouse : kContinuePlayback;

	const uint16 command = _input->consumeKey();
	if (command == 0x1b) {
		debugC(2, kDebugVideo, "Ripper: Escape advanced skippable presentation to end");
		return kSkipToEnd;
	}
	if (command == 0x20) {
		paused = !paused;
		decoder.pauseVideo(paused);
		_mixer->pauseAll(paused);
		logPlaybackPause("Space", paused, decoder, name, externalAudio);
	}
	if (command == 0x4d00 && policy.allowSegmentAdvance) {
		debugC(2, kDebugVideo, "Ripper: Right Arrow requested next IAVF segment");
		return kAdvanceSegment;
	}
	return kContinuePlayback;
}

SmackerPlaybackResult MediaPlayer::playSmacker(Common::SeekableReadStream *stream, const Common::String &name,
		const SmackerPlaybackPlan &plan) {
	debugC(2, kDebugVideo, "Ripper: Smacker playback plan media='%s' %s",
		name.c_str(), describeSmackerPlaybackPlan(plan).c_str());
	const bool allowEscSpace = plan.input.allowEscSpace;
	int x = plan.placement.x;
	int y = plan.placement.y;
	const Common::Rect &centerBounds = plan.placement.centerBounds;
	Audio::SoundHandle *externalAudio = plan.timeline.externalAudio;
	const Common::Array<uint32> *frameAudioOffsets = plan.timeline.frameAudioOffsets;
	const uint32 audioByteRate = plan.timeline.audioByteRate;
	const uint32 timelineStartMillis = plan.timeline.timelineStartMillis;
	uint displayScale = plan.placement.displayScale;
	const uint frameLimit = plan.frames.frameLimit;
	const int originY = plan.placement.originY;
	const bool serviceSceneUi = plan.input.serviceSceneUi;
	const bool loopFromStart = plan.loop.loopFromStart;
	const uint loopStartFrame = plan.loop.loopStartFrame;
	MediaSequenceCallback *sequenceCallback = plan.callback.sequenceCallback;
	const uint firstFrame = plan.frames.firstFrame;
	uint lastFrame = plan.frames.lastFrame;
	const uint boundedLoopStartFrame = plan.loop.boundedLoopStartFrame;
	SmackerPlaybackResult result;
	RipperSmackerDecoder decoder;
	if (!decoder.loadStreamWithDataFixes(stream, name)) {
		warning("Ripper: invalid Smacker stream '%s'", name.c_str());
		return result;
	}
	const uint frameCount = decoder.getFrameCount();
	if (frameCount == 0) {
		warning("Ripper: invalid Smacker segment '%s' frames=%u..%u count=0",
			name.c_str(), firstFrame, lastFrame);
		return result;
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
		return result;
	}
	// Each packetized branch chooses its own scale, regardless of the IAVF canvas.
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
			return result;
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
		return result;
	}
	debugC(1, kDebugVideo,
		"Ripper: playing Smacker '%s' frames=%u source=%ux%u output=%ux%u at %d,%d controls=%d sceneUi=%d frameLimit=%u",
		name.c_str(), decoder.getFrameCount(), decoder.getWidth(), decoder.getHeight(),
		outputWidth, outputHeight, x, y, allowEscSpace, serviceSceneUi, frameLimit);
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
	result.completed = true;
	uint presentedFrames = 0;
	SmackerFrameRenderer renderer(_engine, decoder, name, plan, x, y, displayScale);
	if (!renderer.captureBacking()) {
		decoder.close();
		return SmackerPlaybackResult();
	}
	// A branch callback may show its own cursor during playback.
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
		result.command = command;
		debugC(2, kDebugVideo,
			"Ripper: interactive Smacker '%s' returned command=0x%04x frame=%u",
			name.c_str(), command, frame);
		return true;
	};
	auto finishSequenceFramePresentation = [&](uint frame) {
		const bool stopSequence = serviceSequenceCallback(frame);
		const bool refreshedPalette = sequencePaletteRefresh;
		if (sequencePaletteRefresh) {
			if (!renderer.applyPalette(true))
				result.completed = false;
			sequencePaletteRefresh = false;
		}
		if (sequenceCallback && sequenceCallback->managesPalette()) {
			byte palette[256 * 3];
			// Combat transforms the active Smacker palette each frame; restore that
			// base before applying the next callback transform.
			if (!renderer.applyPalette(true))
				result.completed = false;
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
			return SmackerPlaybackResult();
		}
		if (!renderer.draw(frame, true))
			result.completed = false;
		++presentedFrames;
		debugC(3, kDebugVideo,
			"Ripper: entered bounded Smacker segment '%s' frame=%u..%u",
			name.c_str(), firstFrame, lastFrame);
		if (sequenceCallback && finishSequenceFramePresentation(firstFrame + 1)) {
			decoder.close();
			result.completed = true;
			return result;
		}
		if (boundedSegment && firstFrame == lastFrame) {
			decoder.close();
			result.completed = true;
			return result;
		}
	}
	while (!_engine->shouldQuit()) {
		if (decoder.endOfVideo()) {
			if (loopFromStart) {
				if (!decoder.rewind()) {
					warning("Ripper: could not rewind looping scene Smacker '%s'",
						name.c_str());
					result.completed = false;
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
				result.completed = false;
				break;
			}
			const Graphics::Surface *frame =
				decoder.restartAtFrame(loopStartFrame - 1);
			if (!frame) {
				warning("Ripper: could not seek Smacker '%s' to loop start frame=%u",
					name.c_str(), loopStartFrame);
				result.completed = false;
				break;
			}
			if (!renderer.draw(frame, true))
				result.completed = false;
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
			result.completed = false;
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
		SmackerInputPolicy inputPolicy = plan.input;
		inputPolicy.allowEscSpace = allowEscSpace && !callbackOwnsInput;
		inputPolicy.serviceSceneUi = serviceSceneUi && !callbackOwnsInput;
		const PlaybackInputResult inputResult = servicePlaybackInput(decoder,
			inputPolicy, paused, externalAudio, toolbarOwnsInput, name);
		if (inputResult != kContinuePlayback) {
			result.completed = false;
			result.stoppedByMouse = inputResult == kStopForMouse;
			result.advanceSegment = inputResult == kAdvanceSegment;
			if (result.advanceSegment)
				result.completed = true;
			if (inputResult == kSkipToEnd && decoder.getFrameCount() != 0) {
				const uint finalFrame = decoder.getFrameCount() - 1;
				// Smacker pixels and palette packets are stateful. Rebuild the
				// terminal frame from a cleared surface so the engine-added
				// Escape fast-forward cannot retain intermediate frame state.
				const Graphics::Surface *frame = decoder.restartAtFrame(finalFrame);
				if (frame) {
					if (!renderer.draw(frame, true))
						result.completed = false;
					if (sequenceCallback)
						presentScreen();
					if (serviceSceneUi)
						_engine->getSceneAudio()->service(finalFrame + 1);
					result.completed = true;
					debugC(2, kDebugVideo,
						"Ripper: Escape reconstructed and presented final Smacker frame '%s' frame=%u; completing presentation",
						name.c_str(), finalFrame);
				}
			}
			if (!result.advanceSegment && !_engine->shouldQuit())
				result.stoppedByUser = true;
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
				if (!renderer.draw(frame, false))
					result.completed = false;
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
							result.completed = false;
							break;
						}
						if (!renderer.draw(loopFrame, true))
							result.completed = false;
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
	if (synchronizeToTimeline && result.completed) {
		const uint32 elapsedMs = externalAudio && _mixer->isSoundHandleActive(*externalAudio) ?
			_mixer->getSoundElapsedTime(*externalAudio) : g_system->getMillis(true) - timelineStartMillis;
		debugC(2, kDebugVideo, "Ripper: completed IAVF Smacker '%s' targetMs=%u audioElapsedMs=%u",
			name.c_str(),
			(uint32)((uint64)frameAudioOffsets->back() * 1000 / audioByteRate),
			elapsedMs);
	}
	renderer.restoreBacking();
	decoder.close();
	return result;
}


} // End of namespace Ripper
