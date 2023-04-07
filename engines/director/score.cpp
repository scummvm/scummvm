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

#define FORBIDDEN_SYMBOL_EXCEPTION_getenv

#include "common/config-manager.h"
#include "common/file.h"
#include "common/md5.h"
#include "common/memstream.h"
#include "common/punycode.h"
#include "common/substream.h"

#include "audio/audiostream.h"

#include "graphics/macgui/mactext.h"

#ifdef USE_PNG
#include "image/png.h"
#endif

#include "director/director.h"
#include "director/cast.h"
#include "director/castmember.h"
#include "director/score.h"
#include "director/frame.h"
#include "director/movie.h"
#include "director/sound.h"
#include "director/cursor.h"
#include "director/channel.h"
#include "director/sprite.h"
#include "director/window.h"
#include "director/util.h"
#include "director/lingo/lingo.h"

namespace Director {

#include "director/palette-fade.h"

Score::Score(Movie *movie) {
	_movie = movie;
	_window = movie->getWindow();
	_vm = _movie->getVM();
	_lingo = _vm->getLingo();

	_soundManager = _window->getSoundManager();

	_puppetTempo = 0x00;
	_puppetPalette = false;
	_lastPalette = 0;
	_paletteTransitionIndex = 0;
	memset(_paletteSnapshotBuffer, 0, 768);

	_labels = nullptr;

	_currentFrameRate = 20;
	_currentFrame = 0;
	_nextFrame = 0;
	_currentLabel = 0;
	_nextFrameTime = 0;
	_lastTempo = 0;
	_waitForChannel = 0;
	_waitForVideoChannel = 0;
	_cursorDirty = false;
	_waitForClick = false;
	_waitForClickCursor = false;
	_activeFade = 0;
	_playState = kPlayNotStarted;

	_numChannelsDisplayed = 0;
}

Score::~Score() {
	for (uint i = 0; i < _frames.size(); i++)
		delete _frames[i];

	for (uint i = 0; i < _channels.size(); i++)
		delete _channels[i];

	if (_labels)
		for (Common::SortedArray<Label *>::iterator it = _labels->begin(); it != _labels->end(); ++it)
			delete *it;

	delete _labels;
}

int Score::getCurrentPalette() {
	return _lastPalette;
}

int Score::resolvePaletteId(int id) {
	// TODO: Palette ID should be a CastMemberID to allow for palettes in different casts
	// 255 represent system palette in D2
	if (id == 255) {
		id = g_director->getCurrentMovie()->getCast()->_defaultPalette;
	} else if (id > 0) {
		CastMember *member = _movie->getCastMember(CastMemberID(id, 0));
		id = (member && member->_type == kCastPalette) ? ((PaletteCastMember *)member)->getPaletteId() : 0;
	}

	return id;
}

bool Score::processImmediateFrameScript(Common::String s, int id) {
	s.trim();

	// In D2/D3 this specifies immediately the sprite/field properties
	if (!s.compareToIgnoreCase("moveableSprite") || !s.compareToIgnoreCase("editableText")) {
		_immediateActions[id] = true;
	}

	return false;
}

bool Score::processFrozenScripts() {
	// Unfreeze any in-progress scripts and attempt to run them
	// to completion.
	while (uint32 count = _window->frozenLingoStateCount()) {
		_window->thawLingoState();
		g_lingo->switchStateFromWindow();
		g_lingo->execute();
		if (_window->frozenLingoStateCount() >= count) {
			debugC(3, kDebugLingoExec, "Score::processFrozenScripts(): State froze again mid-thaw, interrupting");
			return false;
		}
	}
	return true;
}

uint16 Score::getLabel(Common::String &label) {
	if (!_labels) {
		warning("Score::getLabel: No labels set");
		return 0;
	}

	for (Common::SortedArray<Label *>::iterator i = _labels->begin(); i != _labels->end(); ++i) {
		if ((*i)->name.equalsIgnoreCase(label)) {
			return (*i)->number;
		}
	}

	return 0;
}

Common::String *Score::getLabelList() {
	Common::String *res = new Common::String;

	for (Common::SortedArray<Label *>::iterator i = _labels->begin(); i != _labels->end(); ++i) {
		*res += (*i)->name;
		*res += '\n';
	}

	return res;
}

Common::String *Score::getFrameLabel(uint id) {
	for (Common::SortedArray<Label *>::iterator i = _labels->begin(); i != _labels->end(); ++i) {
		if ((*i)->number == id) {
			return new Common::String((*i)->name);
			break;
		}
	}

	return new Common::String;
}

void Score::setStartToLabel(Common::String &label) {
	uint16 num = getLabel(label);

	if (num == 0)
		warning("Label %s not found", label.c_str());
	else
		_nextFrame = num;
}

void Score::gotoLoop() {
	// This command has the playback head continuously return to the first marker to the left and then loop back.
	// If no marker are to the left of the playback head, the playback head continues to the right.
	if (_labels == nullptr) {
		_nextFrame = 1;
		return;
	} else {
		_nextFrame = _currentLabel;
	}

	_vm->_skipFrameAdvance = true;
}

int Score::getCurrentLabelNumber() {
	Common::SortedArray<Label *>::iterator i;

	if (!_labels)
		return 0;

	int frame = 0;

	for (i = _labels->begin(); i != _labels->end(); ++i) {
		if ((*i)->number <= _currentFrame)
			frame = (*i)->number;
	}

	return frame;
}

void Score::gotoNext() {
	// we can just try to use the current frame and get the next label
	_nextFrame = getNextLabelNumber(_currentFrame);
}

void Score::gotoPrevious() {
	// we actually need the frame of the label prior to the most recent label.
	_nextFrame = getPreviousLabelNumber(getCurrentLabelNumber());
}

int Score::getNextLabelNumber(int referenceFrame) {
	if (_labels == nullptr || _labels->size() == 0)
		return 0;

	Common::SortedArray<Label *>::iterator i;

	for (i = _labels->begin(); i != _labels->end(); ++i) {
		if ((*i)->number >= referenceFrame) {
			int n = (*i)->number;
			++i;
			if (i != _labels->end()) {
				// return to the first marker to to the right
				return (*i)->number;
			} else {
				// if no markers are to the right of the playback head,
				// the playback head goes to the first marker to the left
				return n;
			}
		}
	}

	// If there are not markers to the left,
	// the playback head goes to frame 1, (Director frame array start from 1, engine from 0)
	return 0;
}

int Score::getPreviousLabelNumber(int referenceFrame) {
	if (_labels == nullptr || _labels->size() == 0)
		return 0;

	// One label
	if (_labels->begin() == _labels->end())
		return (*_labels->begin())->number;

	Common::SortedArray<Label *>::iterator previous = _labels->begin();
	Common::SortedArray<Label *>::iterator i;

	for (i = (previous + 1); i != _labels->end(); ++i, ++previous) {
		if ((*i)->number >= referenceFrame)
			return (*previous)->number;
	}

	return 0;
}

void Score::startPlay() {
	_currentFrame = 1;
	_playState = kPlayStarted;
	_nextFrameTime = 0;

	if (_frames.size() <= 1) {	// We added one empty sprite
		warning("Score::startLoop(): Movie has no frames");
		_playState = kPlayStopped;

		return;
	}

	_lastPalette = _frames[_currentFrame]->_palette.paletteId;
	if (!_lastPalette)
		_lastPalette = _movie->getCast()->_defaultPalette;
	debugC(2, kDebugImages, "Score::startPlay(): palette changed to %d", _lastPalette);
	_vm->setPalette(resolvePaletteId(_lastPalette));

	// All frames in the same movie have the same number of channels
	if (_playState != kPlayStopped)
		for (uint i = 0; i < _frames[1]->_sprites.size(); i++)
			_channels.push_back(new Channel(_frames[1]->_sprites[i], i));

	if (_vm->getVersion() >= 300)
		_movie->processEvent(kEventStartMovie);
}

void Score::step() {
	if (_playState == kPlayStopped)
		return;

	if (!_movie->_userEventQueue.empty()) {
		_lingo->processEvents(_movie->_userEventQueue);
	} else if (_vm->getVersion() >= 300 && !_window->_newMovieStarted && _playState != kPlayStopped) {
		_movie->processEvent(kEventIdle);
	}

	update();

	if (debugChannelSet(-1, kDebugFewFramesOnly) || debugChannelSet(-1, kDebugScreenshot)) {
		warning("Score::startLoop(): ran frame %0d", g_director->_framesRan);
		g_director->_framesRan++;
	}

	if (debugChannelSet(-1, kDebugFewFramesOnly) && g_director->_framesRan > kFewFamesMaxCounter) {
		warning("Score::startLoop(): exiting due to debug few frames only");
		_playState = kPlayStopped;
		return;
	}

	if (debugChannelSet(-1, kDebugScreenshot))
		screenShot();
}

void Score::stopPlay() {
	if (_vm->getVersion() >= 300)
		_movie->processEvent(kEventStopMovie);
	_lingo->executePerFrameHook(-1, 0);
}

void Score::update() {
	if (_activeFade) {
		if (!_soundManager->fadeChannel(_activeFade))
			_activeFade = 0;
	}

	if (!debugChannelSet(-1, kDebugFast)) {
		bool keepWaiting = false;

		debugC(8, kDebugLoading, "Score::update(): nextFrameTime: %d, time: %d", _nextFrameTime, g_system->getMillis(false));
		if (_waitForChannel) {
			if (_soundManager->isChannelActive(_waitForChannel)) {
				keepWaiting = true;
			} else {
				_waitForChannel = 0;
			}
		} else if (_waitForClick) {
			if (g_system->getMillis() >= _nextFrameTime + 1000) {
				_waitForClickCursor = !_waitForClickCursor;
				renderCursor(_movie->getWindow()->getMousePos());
				_nextFrameTime = g_system->getMillis();
			}
			keepWaiting = true;
		} else if (_waitForVideoChannel) {
			Channel *movieChannel = _channels[_waitForVideoChannel];
			if (movieChannel->isActiveVideo() && movieChannel->_movieRate != 0.0) {
				keepWaiting = true;
			} else {
				_waitForVideoChannel = 0;
			}
		} else if (g_system->getMillis() < _nextFrameTime && !_nextFrame) {
			keepWaiting = true;
		}

		if (keepWaiting) {
			if (_movie->_videoPlayback) {
				updateWidgets(true);
				_window->render();
			}
			processFrozenScripts();
			return;
		}
	}

	// For previous frame
	if (!_window->_newMovieStarted && !_vm->_playbackPaused) {
		// When Lingo::func_goto* is called, _nextFrame is set
		// and _skipFrameAdvance is set to true.
		// exitFrame is not called in this case.
		if (!_vm->_skipFrameAdvance && _vm->getVersion() >= 400) {
			_movie->processEvent(kEventExitFrame);
		}

		// If there is a transition, the perFrameHook is called
		// after each transition subframe instead.
		if (_frames[_currentFrame]->_transType == 0) {
			_lingo->executePerFrameHook(_currentFrame, 0);
		}
	}

	_vm->_skipFrameAdvance = false;

	// the exitFrame event handler may have stopped this movie
	if (_playState == kPlayStopped) {
		processFrozenScripts();
		return;
	}

	if (!_vm->_playbackPaused) {
		if (_nextFrame)
			_currentFrame = _nextFrame;
		else if (!_window->_newMovieStarted)
			_currentFrame++;
	}

	_nextFrame = 0;

	if (_currentFrame >= _frames.size()) {
		Window *window = _vm->getCurrentWindow();
		if (!window->_movieStack.empty()) {
			MovieReference ref = window->_movieStack.back();
			window->_movieStack.pop_back();
			if (!ref.movie.empty()) {
				_playState = kPlayStopped;
				window->setNextMovie(ref.movie);
				window->_nextMovie.frameI = ref.frameI;
				processFrozenScripts();
				return;
			}

			_currentFrame = ref.frameI;
		} else {
			if (debugChannelSet(-1, kDebugNoLoop)) {
				_playState = kPlayStopped;
				processFrozenScripts();
				return;
			}

			_currentFrame = 1;
		}
	}

	Common::SortedArray<Label *>::iterator i;
	if (_labels != nullptr) {
		for (i = _labels->begin(); i != _labels->end(); ++i) {
			if ((*i)->number == _currentFrame) {
				_currentLabel = _currentFrame;
			}
		}
	}

	byte tempo = _frames[_currentFrame]->_scoreCachedTempo;
	// puppetTempo is overridden by changes in score tempo
	if (_frames[_currentFrame]->_tempo || tempo != _lastTempo) {
		_puppetTempo = 0;
	} else if (_puppetTempo) {
		tempo = _puppetTempo;
	}

	if (tempo) {
		const bool waitForClickOnly = _vm->getVersion() < 300;
		int maxDelay = 60;
		if (_vm->getVersion() < 300) {
			maxDelay = 120;
		} else if (_vm->getVersion() < 400) {
			// Director 3 has a slider that goes up to 120, but any value
			// beyond 95 gets converted into a video wait instruction.
			maxDelay = 95;
		}
		if (tempo >= 256 - maxDelay) {
			// Delay
			_nextFrameTime = g_system->getMillis() + (256 - tempo) * 1000;
			debugC(5, kDebugLoading, "Score::update(): setting _nextFrameTime to %d based on a delay of %d", _nextFrameTime, 256 - tempo);
		} else if (tempo <= 120) {
			// FPS
			_currentFrameRate = tempo;
			if (g_director->_fpsLimit)
				_currentFrameRate = MIN(g_director->_fpsLimit, _currentFrameRate);
			_nextFrameTime = g_system->getMillis() + 1000.0 / (float)_currentFrameRate;
			debugC(5, kDebugLoading, "Score::update(): setting _nextFrameTime to %d based on a framerate of %d", _nextFrameTime, tempo);
		} else {
			if (tempo == 128) {
				_waitForClick = true;
				_waitForClickCursor = false;
				renderCursor(_movie->getWindow()->getMousePos());
				debugC(5, kDebugLoading, "Score::update(): waiting for mouse click before next frame");
			} else if (!waitForClickOnly && tempo == 135) {
				// Wait for sound channel 1
				_waitForChannel = 1;
				debugC(5, kDebugLoading, "Score::update(): waiting for sound channel 1 before next frame");
			} else if (!waitForClickOnly && tempo == 134) {
				// Wait for sound channel 2
				_waitForChannel = 2;
				debugC(5, kDebugLoading, "Score::update(): waiting for sound channel 2 before next frame");
			} else if (!waitForClickOnly && tempo >= 136 && tempo <= 135 + _numChannelsDisplayed) {
				// Wait for a digital video in a channel to finish playing
				_waitForVideoChannel = tempo - 135;
				debugC(5, kDebugLoading, "Score::update(): waiting for video in channel %d before next frame", _waitForVideoChannel);
			} else {
				warning("Unhandled tempo instruction: %d", tempo);
			}
			_nextFrameTime = g_system->getMillis();
		}
	} else {
		_nextFrameTime = g_system->getMillis() + 1000.0 / (float)_currentFrameRate;
	}
	_lastTempo = tempo;

	if (debugChannelSet(-1, kDebugSlow))
		_nextFrameTime += 1000;

	debugC(1, kDebugLoading, "******************************  Current frame: %d, time: %d", _currentFrame, g_system->getMillis(false));
	g_debugger->frameHook();

	_lingo->executeImmediateScripts(_frames[_currentFrame]);

	if (_vm->getVersion() >= 600) {
		// _movie->processEvent(kEventBeginSprite);
		// TODO Director 6 step: send beginSprite event to any sprites whose span begin in the upcoming frame
		// _movie->processEvent(kEventPrepareFrame);
		// TODO: Director 6 step: send prepareFrame event to all sprites and the script channel in upcoming frame
	}

	// Window is drawn between the prepareFrame and enterFrame events (Lingo in a Nutshell, p.100)
	renderFrame(_currentFrame);
	_window->_newMovieStarted = false;

	// Enter and exit from previous frame
	if (!_vm->_playbackPaused) {
		uint32 count = _window->frozenLingoStateCount();
		// Triggers the frame script in D2-3, explicit enterFrame handlers in D4+
		// D4 will only process recursive enterFrame handlers to a depth of 2.
		// Any more will be ignored.
		if ((_vm->getVersion() >= 400) && (count < 2)) {
			_movie->processEvent(kEventEnterFrame);
		} else if ((_vm->getVersion() < 400) || _movie->_allowOutdatedLingo) {
			// Force a flush of any frozen scripts before raising enterFrame
			if (!processFrozenScripts())
				return;
			_movie->processEvent(kEventEnterFrame);
			if ((_vm->getVersion() >= 300) || _movie->_allowOutdatedLingo) {
				// Movie version of enterFrame, for D3 only. The D3 Interactivity Manual claims
				// "This handler executes before anything else when the playback head moves."
				// but this is incorrect. The frame script is executed first.
				_movie->processEvent(kEventStepMovie);
			}
		}
		// If another frozen state gets triggered, wait another update() before thawing
		if (_window->frozenLingoStateCount() > count)
			return;
	}

	// Attempt to thaw and continue any frozen execution after startMovie and enterFrame.
	// If they don't complete (i.e. another freezing event like a "go to frame"),
	// force another cycle of Score::update().
	if (!processFrozenScripts())
		return;

	if (!_vm->_playbackPaused) {
		if (_movie->_timeOutPlay)
			_movie->_lastTimeOut = _vm->getMacTicks();
	}

	// TODO Director 6 - another order

	// TODO: Figure out when exactly timeout events are processed
	if (_vm->getMacTicks() - _movie->_lastTimeOut >= _movie->_timeOutLength) {
		_movie->processEvent(kEventTimeout);
		_movie->_lastTimeOut = _vm->getMacTicks();
	}

}

void Score::renderFrame(uint16 frameId, RenderMode mode) {
	// Force cursor update if a new movie's started.
	if (_window->_newMovieStarted)
		renderCursor(_movie->getWindow()->getMousePos(), true);

	if (!renderTransition(frameId)) {
		bool skip = renderPrePaletteCycle(frameId, mode);
		setLastPalette(frameId);
		renderSprites(frameId, mode);
		_window->render();
		if (!skip)
			renderPaletteCycle(frameId, mode);
	}


	playSoundChannel(frameId, false);
	playQueuedSound(); // this is currently only used in FPlayXObj

	if (_cursorDirty) {
		renderCursor(_movie->getWindow()->getMousePos());
		_cursorDirty = false;
	}
}

bool Score::renderTransition(uint16 frameId) {
	Frame *currentFrame = _frames[frameId];
	TransParams *tp = _window->_puppetTransition;

	if (tp) {
		setLastPalette(frameId);
		_window->playTransition(frameId, tp->duration, tp->area, tp->chunkSize, tp->type, resolvePaletteId(currentFrame->_scoreCachedPaletteId));
		delete _window->_puppetTransition;
		_window->_puppetTransition = nullptr;
		return true;
	} else if (currentFrame->_transType) {
		setLastPalette(frameId);
		_window->playTransition(frameId, currentFrame->_transDuration, currentFrame->_transArea, currentFrame->_transChunkSize, currentFrame->_transType, resolvePaletteId(currentFrame->_scoreCachedPaletteId));
		return true;
	} else {
		return false;
 }
}

void Score::renderSprites(uint16 frameId, RenderMode mode) {
	if (_window->_newMovieStarted)
		mode = kRenderForceUpdate;

	_movie->_videoPlayback = false;

	for (uint16 i = 0; i < _channels.size(); i++) {
		Channel *channel = _channels[i];
		Sprite *currentSprite = channel->_sprite;
		Sprite *nextSprite = _frames[frameId]->_sprites[i];

		// widget content has changed and needs a redraw.
		// this doesn't include changes in dimension or position!
		bool widgetRedrawn = channel->updateWidget();

		if (channel->isActiveVideo()) {
			channel->updateVideoTime();
			_movie->_videoPlayback = true;
		}

		if (channel->isDirty(nextSprite) || widgetRedrawn || mode == kRenderForceUpdate) {
			if (currentSprite && !currentSprite->_trails)
				_window->addDirtyRect(channel->getBbox());

			if (currentSprite && currentSprite->_cast && currentSprite->_cast->_erase) {
				_movie->eraseCastMember(currentSprite->_castId);
				currentSprite->_cast->_erase = false;

				currentSprite->setCast(currentSprite->_castId);
				nextSprite->setCast(nextSprite->_castId);
			}

			channel->setClean(nextSprite, i);
			// Check again to see if a video has just been started by setClean.
			if (channel->isActiveVideo())
				_movie->_videoPlayback = true;

			_window->addDirtyRect(channel->getBbox());
			if (currentSprite) {
				debugC(5, kDebugImages,
					"Score::renderSprites(): CH: %-3d castId: %s [ink: %d, puppet: %d, moveable: %d, visible: %d] [bbox: %d,%d,%d,%d] [type: %d fg: %d bg: %d] [script: %s]",
					i, currentSprite->_castId.asString().c_str(), currentSprite->_ink, currentSprite->_puppet, currentSprite->_moveable, channel->_visible,
					PRINT_RECT(channel->getBbox()), currentSprite->_spriteType, currentSprite->_foreColor, currentSprite->_backColor,
					currentSprite->_scriptId.asString().c_str());
			} else {
				debugC(5, kDebugImages, "Score::renderSprites(): CH: %-3d: No sprite", i);
			}
		} else {
			channel->setClean(nextSprite, i, true);
		}

		// update editable text channel after we render the sprites. because for the current frame, we may get those sprites only when we finished rendering
		// (because we are creating widgets and setting active state when we rendering sprites)
		if (channel->isActiveText())
			_movie->_currentEditableTextChannel = i;
	}
}

bool Score::renderPrePaletteCycle(uint16 frameId, RenderMode mode) {
	if (_puppetPalette)
		return false;

	// Skip this if we don't have a palette instruction
	int currentPalette = _frames[frameId]->_palette.paletteId;
	if (!currentPalette || !resolvePaletteId(currentPalette))
		return false;

	if (!_frames[frameId]->_palette.colorCycling &&
		!_frames[frameId]->_palette.overTime) {

		int frameRate = CLIP<int>(_frames[frameId]->_palette.speed, 1, 30);

		if (debugChannelSet(-1, kDebugFast))
			frameRate = 30;

		int frameDelay = 1000/60;
		int fadeFrames = kFadeColorFrames[frameRate - 1];
		byte calcPal[768];

		// Copy the current palette into the snapshot buffer
		memset(_paletteSnapshotBuffer, 0, 768);
		memcpy(_paletteSnapshotBuffer, g_director->getPalette(), g_director->getPaletteColorCount() * 3);
		PaletteV4 *destPal = g_director->getPalette(resolvePaletteId(currentPalette));

		if (_frames[frameId]->_palette.normal) {
			// For fade palette transitions, the whole fade happens with
			// the previous frame's layout.
			debugC(2, kDebugImages, "Score::renderPrePaletteCycle(): fading palette to %d over %d frames", currentPalette, fadeFrames);
			for (int i = 0; i < fadeFrames; i++) {
				lerpPalette(
					calcPal,
					_paletteSnapshotBuffer, 256,
					destPal->palette, destPal->length,
					i + 1,
					fadeFrames
				);
				g_director->setPalette(calcPal, 256);
				g_director->draw();
				// On click, stop loop and reset palette
				if (_vm->processEvents(true)) {
					debugC(2, kDebugImages, "Score::renderPrePaletteCycle(): interrupted, setting palette to %d", currentPalette);
					g_director->setPalette(resolvePaletteId(currentPalette));
					return true;
				}
				g_director->delayMillis(frameDelay);
			}

		} else {
			// For fade to black and fade to white palette transitions,
			// the first half happens with the previous frame's layout.

			byte *fadePal = nullptr;
			if (_frames[frameId]->_palette.fadeToBlack) {
				// Fade everything except color index 0 to black
				debugC(2, kDebugImages, "Score::renderPrePaletteCycle(): fading palette to black over %d frames", fadeFrames);
				fadePal = kBlackPalette;
			} else if (_frames[frameId]->_palette.fadeToWhite) {
				// Fade everything except color index 255 to white
				debugC(2, kDebugImages, "Score::renderPrePaletteCycle(): fading palette to white over %d frames", fadeFrames);
				fadePal = kWhitePalette;
			} else {
				// Shouldn't reach here
				return false;
			}

			for (int i = 0; i < fadeFrames; i++) {
				lerpPalette(
					calcPal,
					_paletteSnapshotBuffer, 256,
					fadePal, 256,
					i + 1,
					fadeFrames
				);
				g_director->setPalette(calcPal, 256);
				g_director->draw();
				// On click, stop loop and reset palette
				if (_vm->processEvents(true)) {
					debugC(2, kDebugImages, "Score::renderPrePaletteCycle(): interrupted, setting palette to %d", currentPalette);
					g_director->setPalette(resolvePaletteId(currentPalette));
					return true;
				}
				g_director->delayMillis(frameDelay);
			}
		}
	}
	return false;
}

void Score::setLastPalette(uint16 frameId) {
	if (_puppetPalette)
		return;

	bool isCachedPalette = false;
	int currentPalette = _frames[frameId]->_palette.paletteId;
	// Palette specified in the frame
	if (currentPalette) {
		// If for whatever reason the palette index is invalid, skip
		if (!resolvePaletteId(currentPalette))
			return;
	} else {
		// Use the score cached palette ID
		isCachedPalette = true;
		currentPalette = _frames[frameId]->_scoreCachedPaletteId;
		// The cached ID is created before the cast gets loaded; if it's zero,
		// this corresponds to the movie default palette.
		if (!currentPalette)
			currentPalette = g_director->getCurrentMovie()->getCast()->_defaultPalette;
		// If for whatever reason this doesn't resolve, abort.
		if (!currentPalette || !resolvePaletteId(currentPalette))
			return;
	}

	// If the palette is defined in the frame and doesn't match
	// the current one, set it
	bool paletteChanged = currentPalette != _lastPalette && currentPalette;
	if (paletteChanged) {
		debugC(2, kDebugImages, "Score::setLastPalette(): palette changed to %d, from %s", currentPalette, isCachedPalette ? "cache" :"frame");
		_lastPalette = currentPalette;
		_paletteTransitionIndex = 0;

		// Switch to a new palette immediately if:
		// - this is color cycling mode, or
		// - the cached palette ID is different (i.e. we jumped in the score)
		if (_frames[frameId]->_palette.colorCycling || isCachedPalette)
			g_director->setPalette(resolvePaletteId(_lastPalette));
	}

}

bool Score::isPaletteColorCycling() {
	return _frames[_currentFrame]->_palette.colorCycling;
}

void Score::renderPaletteCycle(uint16 frameId, RenderMode mode) {
	if (_puppetPalette)
		return;

	// If the palette is defined in the frame and doesn't match
	// the current one, set it
	int currentPalette = _frames[frameId]->_palette.paletteId;
	if (!currentPalette || !resolvePaletteId(currentPalette))
		return;

	// For palette cycling, the only thing that is checked is if
	// the palette ID is the same. Different cycling configs with
	// the same palette ID will persist any mutated state.
	// e.g. if you use overTime to cycle the palette partially
	// through a cycle, then switch to doing a full color cycle
	// on the same palette, it will not reset and the weird
	// offset will remain.

	// Cycle speed in FPS
	int speed = _frames[frameId]->_palette.speed;
	if (speed == 0)
		return;

	if (debugChannelSet(-1, kDebugFast))
		speed = 30;

	// 30 (the maximum) is actually unbounded
	int delay = speed == 30 ? 10 : 1000 / speed;
	if (_frames[frameId]->_palette.colorCycling) {
		// Cycle the colors of a chosen palette
		int firstColor = _frames[frameId]->_palette.firstColor;
		int lastColor = _frames[frameId]->_palette.lastColor;

		if (_frames[frameId]->_palette.overTime) {
			// Do a single color step in one frame transition
			debugC(2, kDebugImages, "Score::renderPaletteCycle(): color cycle palette %d, from colors %d to %d, by 1 frame", currentPalette, firstColor, lastColor);
			g_director->shiftPalette(firstColor, lastColor, false);
			g_director->draw();
		} else {
			// Short circuit for few frames renderer
			if (debugChannelSet(-1, kDebugFast)) {
				g_director->setPalette(resolvePaletteId(currentPalette));
				return;
			}

			// Do a full color cycle in one frame transition
			int steps = lastColor - firstColor + 1;
			debugC(2, kDebugImages, "Score::renderPaletteCycle(): color cycle palette %d, from colors %d to %d, over %d steps %d times", currentPalette, firstColor, lastColor, steps, _frames[frameId]->_palette.cycleCount);
			for (int i = 0; i < _frames[frameId]->_palette.cycleCount; i++) {
				for (int j = 0; j < steps; j++) {
					g_director->shiftPalette(firstColor, lastColor, false);
					g_director->draw();
					// On click, stop loop and reset palette
					if (_vm->processEvents(true)) {
						g_director->setPalette(resolvePaletteId(currentPalette));
						return;
					}
					g_director->delayMillis(delay);
				}
				if (_frames[frameId]->_palette.autoReverse) {
					for (int j = 0; j < steps; j++) {
						g_director->shiftPalette(firstColor, lastColor, true);
						g_director->draw();
						// On click, stop loop and reset palette
						if (_vm->processEvents(true)) {
							g_director->setPalette(resolvePaletteId(currentPalette));
							return;
						}
						g_director->delayMillis(delay);
					}
				}
			}
		}
	} else {
		// Transition from the current palette to a new palette
		PaletteV4 *destPal = g_director->getPalette(resolvePaletteId(currentPalette));
		int frameCount = _frames[frameId]->_palette.frameCount;
		byte calcPal[768];

		if (_frames[frameId]->_palette.overTime) {
			// Transition over a series of frames
			if (_paletteTransitionIndex == 0) {
				// Copy the current palette into the snapshot buffer
				memset(_paletteSnapshotBuffer, 0, 768);
				memcpy(_paletteSnapshotBuffer, g_director->getPalette(), g_director->getPaletteColorCount() * 3);
				debugC(2, kDebugImages, "Score::renderPaletteCycle(): fading palette to %d over %d frames", currentPalette, frameCount);
			}

			if (_frames[frameId]->_palette.normal) {
				// Fade the palette directly to the new palette
				lerpPalette(
					calcPal,
					_paletteSnapshotBuffer, 256,
					destPal->palette, destPal->length,
					_paletteTransitionIndex + 1,
					frameCount
				);
			} else {
				// Fade the palette to an intermediary color (black or white),
				// then to the new palette
				int halfway = frameCount / 2;

				byte *fadePal = nullptr;
				if (_frames[frameId]->_palette.fadeToBlack) {
					// Fade everything except color index 0 to black
					fadePal = kBlackPalette;
				} else if (_frames[frameId]->_palette.fadeToWhite) {
					// Fade everything except color index 255 to white
					fadePal = kWhitePalette;
				} else {
					// Shouldn't reach here
					return;
				}

				if (_paletteTransitionIndex < halfway) {
					lerpPalette(
						calcPal,
						_paletteSnapshotBuffer, 256,
						fadePal, 256,
						_paletteTransitionIndex + 1,
						halfway
					);
				} else {
					lerpPalette(
						calcPal,
						fadePal, 256,
						destPal->palette, destPal->length,
						_paletteTransitionIndex - halfway + 1,
						frameCount - halfway
					);
				}
			}
			g_director->setPalette(calcPal, 256);
			_paletteTransitionIndex++;
			_paletteTransitionIndex %= frameCount;
		} else {
			// Short circuit for fast renderer
			if (debugChannelSet(-1, kDebugFast)) {
				debugC(2, kDebugImages, "Score::renderPaletteCycle(): setting palette to %d", currentPalette);
				g_director->setPalette(resolvePaletteId(currentPalette));
				return;
			}

			// Do a full cycle in one frame transition
			// For normal mode, we've already faded the palette in renderPrePaletteCycle
			if (!_frames[frameId]->_palette.normal) {
				byte *fadePal = nullptr;
				if (_frames[frameId]->_palette.fadeToBlack) {
					// Fade everything except color index 0 to black
					fadePal = kBlackPalette;
				} else if (_frames[frameId]->_palette.fadeToWhite) {
					// Fade everything except color index 255 to white
					fadePal = kWhitePalette;
				} else {
					// Shouldn't reach here
					return;
				}
				int frameRate = CLIP<int>(_frames[frameId]->_palette.speed, 1, 30);

				if (debugChannelSet(-1, kDebugFast))
					frameRate = 30;

				int frameDelay = 1000/60;
				int fadeFrames = kFadeColorFrames[frameRate - 1];

				// Wait for a fixed time
				g_director->setPalette(fadePal, 256);
				g_director->draw();
				for (int i = 0; i < fadeColorWait; i++) {
					// On click, stop loop and reset palette
					if (_vm->processEvents(true)) {
						debugC(2, kDebugImages, "Score::renderPaletteCycle(): interrupted, setting palette to %d", currentPalette);
						g_director->setPalette(resolvePaletteId(currentPalette));
						return;
					}
					g_director->delayMillis(frameDelay);
				}

				debugC(2, kDebugImages, "Score::renderPaletteCycle(): fading palette to %d over %d frames", currentPalette, fadeFrames);

				for (int i = 0; i < fadeFrames; i++) {
					lerpPalette(
						calcPal,
						fadePal, 256,
						destPal->palette, destPal->length,
						i + 1,
						fadeFrames
					);
					g_director->setPalette(calcPal, 256);
					g_director->draw();
					// On click, stop loop and reset palette
					if (_vm->processEvents(true)) {
						debugC(2, kDebugImages, "Score::renderPaletteCycle(): interrupted, setting palette to %d", currentPalette);
						g_director->setPalette(resolvePaletteId(currentPalette));
						return;
					}
					g_director->delayMillis(frameDelay);
				}

			}
		}
	}
}

void Score::renderCursor(Common::Point pos, bool forceUpdate) {
	if (_window != _vm->getCursorWindow()) {
		// The cursor is outside of this window.
		return;
	}

	if (_waitForClick) {
		_vm->setCursor(_waitForClickCursor ? kCursorMouseDown : kCursorMouseUp);
		return;
	}

	if (!_channels.empty() && _playState != kPlayStopped) {
		uint spriteId = 0;

		for (int i = _channels.size() - 1; i >= 0; i--)
			if (_channels[i]->isMouseIn(pos) && !_channels[i]->_cursor.isEmpty()) {
				spriteId = i;
				break;
			}

		if (!_channels[spriteId]->_cursor.isEmpty()) {
			if (!forceUpdate && _currentCursor == _channels[spriteId]->_cursor)
				return;

			// try to use the cursor read from exe file.
			// currently, we are using mac arrow to represent custom win cursor since we didn't find where it stores. So i comment it out here.
//			if (g_director->getPlatform() == Common::kPlatformWindows && _channels[spriteId]->_cursor._cursorType == Graphics::kMacCursorCustom)
//				_vm->_wm->replaceCursor(_channels[spriteId]->_cursor._cursorType, g_director->_winCursor[_channels[spriteId]->_cursor._cursorResId]);
			_vm->_wm->replaceCursor(_channels[spriteId]->_cursor._cursorType, &_channels[spriteId]->_cursor);
			_currentCursor = _channels[spriteId]->_cursor.getRef();
			return;
		}
	}

	if (!forceUpdate && _currentCursor == _defaultCursor)
		return;

	_vm->_wm->replaceCursor(_defaultCursor._cursorType, &_defaultCursor);
	_currentCursor = _defaultCursor.getRef();
}

void Score::updateWidgets(bool hasVideoPlayback) {
	for (uint16 i = 0; i < _channels.size(); i++) {
		Channel *channel = _channels[i];
		CastMember *cast = channel->_sprite->_cast;
		if (hasVideoPlayback)
			channel->updateVideoTime();
		if (cast && (cast->_type != kCastDigitalVideo || hasVideoPlayback) && cast->isModified()) {
			channel->replaceWidget();
			_window->addDirtyRect(channel->getBbox());
		}
	}
}

void Score::invalidateRectsForMember(CastMember *member) {
	for (uint16 i = 0; i < _channels.size(); i++) {
		Channel *channel = _channels[i];
		if (channel->_sprite->_cast == member) {
			_window->addDirtyRect(channel->getBbox());
		}
	}
}

static Common::String computeSurfaceMd5(const Graphics::Surface *surf) {
	Common::MemoryReadStream stream((const byte *)surf->getPixels(), surf->pitch * surf->h);

	return Common::computeStreamMD5AsString(stream);
}

void Score::screenShot() {
#ifndef USE_PNG
		warning("Screenshot requested, but PNG support is not compiled in");

		return;
#else


	Graphics::Surface rawSurface = _window->getSurface()->rawSurface();
	const Graphics::PixelFormat requiredFormat_4byte(4, 8, 8, 8, 8, 0, 8, 16, 24);
	Graphics::Surface *newSurface = rawSurface.convertTo(requiredFormat_4byte, _vm->getPalette());

	Common::String currentPath = _vm->getCurrentPath().c_str();
	Common::replace(currentPath, Common::String(g_director->_dirSeparator), "-"); // exclude dir separator from screenshot filename prefix
	Common::String prefix = Common::String::format("%s%s", currentPath.c_str(), Common::punycode_encodefilename(_movie->getMacName()).c_str());
	Common::String filename = dumpScriptName(prefix.c_str(), kMovieScript, g_director->_framesRan, "png");

	const char *buildNumber = getenv("BUILD_NUMBER");

	// If we are not inside of buildbot, we just dump it
	if (buildNumber && ConfMan.hasKey("screenshotpath")) {
		// The filename is in the form:
		// ./dumps/theapartment/25/xn--Main Menu-zd0e-19.png

		// Now we try to find any previous dump
		int prevbuild = atoi(buildNumber) - 1;

		while (prevbuild > 0) {
			filename = Common::String::format("%s/%s/%d/%s-%d.png", ConfMan.get("screenshotpath").c_str(),
				g_director->getTargetName().c_str(), prevbuild, prefix.c_str(), g_director->_framesRan);

			Common::FSNode fs(filename);

			if (fs.exists())
				break;

			prevbuild--;
		}

		// We found previous screenshot. Let's compare it
		if (prevbuild > 0) {
			Common::FSNode fs(filename);
			Image::PNGDecoder decoder;
			Common::SeekableReadStream *stream = fs.createReadStream();

			if (decoder.loadStream(*stream)) {
				Common::String oldMd5 = computeSurfaceMd5(decoder.getSurface());
				Common::String newMd5 = computeSurfaceMd5(newSurface);

				if (oldMd5 == newMd5) {
					warning("Screenshot is equal to previous one, skipping: %s", filename.c_str());
					newSurface->free();
					delete newSurface;
					delete stream;

					return;
				}
			} else {
				warning("Error loading previous screenshot %s", filename.c_str());
			}

			delete stream;
		}
	}

	Common::DumpFile screenshotFile;
	if (screenshotFile.open(filename, true)) {
		debug("Dumping screenshot to %s", filename.c_str());

		Image::writePNG(screenshotFile, *newSurface);
	} else {
		warning("Cannot write screenshot to %s", filename.c_str());
	}

	newSurface->free();
	delete newSurface;

#endif // USE_PNG
}

uint16 Score::getSpriteIDFromPos(Common::Point pos) {
	for (int i = _channels.size() - 1; i >= 0; i--)
		if (_channels[i]->isMouseIn(pos))
			return i;

	return 0;
}

uint16 Score::getMouseSpriteIDFromPos(Common::Point pos) {
	for (int i = _channels.size() - 1; i >= 0; i--)
		if (_channels[i]->isMouseIn(pos) && _channels[i]->_sprite->respondsToMouse())
			return i;

	return 0;
}

uint16 Score::getActiveSpriteIDFromPos(Common::Point pos) {
	for (int i = _channels.size() - 1; i >= 0; i--)
		if (_channels[i]->isMouseIn(pos) && _channels[i]->_sprite->isActive())
			return i;

	return 0;
}

bool Score::checkSpriteIntersection(uint16 spriteId, Common::Point pos) {
	if (_channels[spriteId]->getBbox().contains(pos))
		return true;

	return false;
}

Common::List<Channel *> Score::getSpriteIntersections(const Common::Rect &r) {
	Common::List<Channel *>intersections;

	for (uint i = 0; i < _channels.size(); i++) {
		if (!_channels[i]->isEmpty() && !r.findIntersectingRect(_channels[i]->getBbox()).isEmpty())
			intersections.push_back(_channels[i]);
	}

	return intersections;
}

uint16 Score::getSpriteIdByMemberId(CastMemberID id) {
	for (uint i = 0; i < _channels.size(); i++)
		if (_channels[i]->_sprite->_castId == id)
			return i;

	return 0;
}

Sprite *Score::getSpriteById(uint16 id) {
	Channel *channel = getChannelById(id);

	if (channel) {
		return channel->_sprite;
	} else {
		warning("Score::getSpriteById(): sprite on frame %d with id %d not found", _currentFrame, id);
		return nullptr;
	}
}

Sprite *Score::getOriginalSpriteById(uint16 id) {
	Frame *frame = _frames[_currentFrame];
	if (id < frame->_sprites.size())
		return frame->_sprites[id];
	warning("Score::getOriginalSpriteById(%d): out of bounds, >= %d", id, frame->_sprites.size());
	return nullptr;
}

Channel *Score::getChannelById(uint16 id) {
	if (id >= _channels.size()) {
		warning("Score::getChannelById(%d): out of bounds, >= %d", id, _channels.size());
		return nullptr;
	}

	return _channels[id];
}

void Score::playSoundChannel(uint16 frameId, bool puppetOnly) {
	Frame *frame = _frames[frameId];

	debugC(5, kDebugSound, "playSoundChannel(): Sound1 %s Sound2 %s", frame->_sound1.asString().c_str(), frame->_sound2.asString().c_str());
	DirectorSound *sound = _window->getSoundManager();

	if (sound->isChannelPuppet(1)) {
		sound->playPuppetSound(1);
	} else if (!puppetOnly) {
		if (frame->_soundType1 >= kMinSampledMenu && frame->_soundType1 <= kMaxSampledMenu) {
			sound->playExternalSound(frame->_soundType1, frame->_sound1.member, 1);
		} else {
			sound->playCastMember(frame->_sound1, 1);
		}
	}

	if (sound->isChannelPuppet(2)) {
		sound->playPuppetSound(2);
	} else if (!puppetOnly) {
		if (frame->_soundType2 >= kMinSampledMenu && frame->_soundType2 <= kMaxSampledMenu) {
			sound->playExternalSound(frame->_soundType2, frame->_sound2.member, 2);
		} else {
			sound->playCastMember(frame->_sound2, 2);
		}
	}

	// Channels above 2 are only usable by Lingo.
	if (g_director->getVersion() >= 300) {
		sound->playPuppetSound(3);
		sound->playPuppetSound(4);
	}
}

void Score::playQueuedSound() {
	DirectorSound *sound = _window->getSoundManager();
	sound->playFPlaySound();
}

void Score::loadFrames(Common::SeekableReadStreamEndian &stream, uint16 version) {
	debugC(1, kDebugLoading, "****** Loading frames VWSC");

	//stream.hexdump(stream.size());

	uint32 size = stream.readUint32();
	size -= 4;

	if (version < kFileVer400) {
		_numChannelsDisplayed = 30;
	} else if (version >= kFileVer400 && version < kFileVer500) {
		uint32 frame1Offset = stream.readUint32();
		uint32 numFrames = stream.readUint32();
		uint16 framesVersion = stream.readUint16();
		uint16 spriteRecordSize = stream.readUint16();
		uint16 numChannels = stream.readUint16();
		size -= 14;

		if (framesVersion > 13) {
			_numChannelsDisplayed = stream.readUint16();
		} else {
			if (framesVersion <= 7)	// Director5
				_numChannelsDisplayed = 48;
			else
				_numChannelsDisplayed = 120;	// D6

			stream.readUint16(); // Skip
		}

		size -= 2;

		warning("STUB: Score::loadFrames. frame1Offset: %x numFrames: %x version: %x spriteRecordSize: %x numChannels: %x numChannelsDisplayed: %x",
			frame1Offset, numFrames, framesVersion, spriteRecordSize, numChannels, _numChannelsDisplayed);
		// Unknown, some bytes - constant (refer to contuinity).
	} else if (version >= kFileVer500) {
		//what data is up the top of D5 VWSC?
		uint32 unk1 = stream.readUint32();
		uint32 unk2 = stream.readUint32();

		uint16 unk3, unk4, unk5, unk6;

		if (unk2 > 0) {
			uint32 blockSize = stream.readUint32() - 1;
			stream.readUint32();
			stream.readUint32();
			stream.readUint32();
			stream.readUint32();
			for (uint32 skip = 0; skip < blockSize * 4; skip++)
				stream.readByte();

			//header number two... this is our actual score entry point.
			unk1 = stream.readUint32();
			unk2 = stream.readUint32();
			stream.readUint32();
			unk3 = stream.readUint16();
			unk4 = stream.readUint16();
			unk5 = stream.readUint16();
			unk6 = stream.readUint16();
		} else {
			unk3 = stream.readUint16();
			unk4 = stream.readUint16();
			unk5 = stream.readUint16();
			unk6 = stream.readUint16();
			size -= 16;
		}
		warning("STUB: Score::loadFrames. unk1: %x unk2: %x unk3: %x unk4: %x unk5: %x unk6: %x", unk1, unk2, unk3, unk4, unk5, unk6);
	}

	uint16 channelSize;
	uint16 channelOffset;

	Frame *initial = new Frame(this, _numChannelsDisplayed);
	// Push a frame at frame#0 position.
	// This makes all indexing simpler
	_frames.push_back(initial);

	// This is a representation of the channelData. It gets overridden
	// partically by channels, hence we keep it and read the score from left to right
	//
	// TODO Merge it with shared cast
	byte channelData[kChannelDataSize];
	memset(channelData, 0, kChannelDataSize);

	uint8 currentTempo = 0;
	int currentPaletteId = 0;

	while (size != 0 && !stream.eos()) {
		uint16 frameSize = stream.readUint16();
		debugC(3, kDebugLoading, "++++++++++ score frame %d (frameSize %d) size %d", _frames.size(), frameSize, size);

		if (frameSize > 0) {
			Frame *frame = new Frame(this, _numChannelsDisplayed);
			size -= frameSize;
			frameSize -= 2;

			while (frameSize != 0) {

				if (_vm->getVersion() < 400) {
					channelSize = stream.readByte() * 2;
					channelOffset = stream.readByte() * 2;
					frameSize -= channelSize + 2;
				} else {
					channelSize = stream.readUint16();
					channelOffset = stream.readUint16();
					frameSize -= channelSize + 4;
				}

				assert(channelOffset + channelSize < kChannelDataSize);
				stream.read(&channelData[channelOffset], channelSize);
			}

			Common::MemoryReadStreamEndian *str = new Common::MemoryReadStreamEndian(channelData, ARRAYSIZE(channelData), stream.isBE());
			// str->hexdump(str->size(), 32);
			frame->readChannels(str, version);
			delete str;
			// Precache the current FPS tempo, as this carries forward to frames to the right
			// of the instruction.
			// Delay type tempos (e.g. wait commands, delays) apply to only a single frame, and are ignored here.
			if (frame->_tempo && frame->_tempo <= 120)
				currentTempo = frame->_tempo;
			frame->_scoreCachedTempo = frame->_tempo ? frame->_tempo : currentTempo;
			// Precache the current palette ID, as this carries forward to frames to the right
			// of the instruction.
			if (frame->_palette.paletteId)
				currentPaletteId = frame->_palette.paletteId;
			frame->_scoreCachedPaletteId = currentPaletteId;

			debugC(8, kDebugLoading, "Score::loadFrames(): Frame %d actionId: %s", _frames.size(), frame->_actionId.asString().c_str());

			_frames.push_back(frame);
		} else {
			warning("zero sized frame!? exiting loop until we know what to do with the tags that follow.");
			size = 0;
		}
	}
}

void Score::setSpriteCasts() {
	// Update sprite cache of cast pointers/info
	for (uint16 i = 0; i < _frames.size(); i++) {
		for (uint16 j = 0; j < _frames[i]->_sprites.size(); j++) {
			_frames[i]->_sprites[j]->setCast(_frames[i]->_sprites[j]->_castId);

			debugC(5, kDebugImages, "Score::setSpriteCasts(): Frame: %d Channel: %d castId: %s type: %d (%s)",
				i, j, _frames[i]->_sprites[j]->_castId.asString().c_str(), _frames[i]->_sprites[j]->_spriteType,
				spriteType2str(_frames[i]->_sprites[j]->_spriteType));
		}
	}
}

void Score::loadLabels(Common::SeekableReadStreamEndian &stream) {
	if (debugChannelSet(5, kDebugLoading)) {
		debug("Score::loadLabels()");
		stream.hexdump(stream.size());
	}

	_labels = new Common::SortedArray<Label *>(compareLabels);
	uint16 count = stream.readUint16() + 1;
	uint32 offset = count * 4 + 2;

	uint16 frame = stream.readUint16();
	uint32 stringPos = stream.readUint16() + offset;

	for (uint16 i = 1; i < count; i++) {
		uint16 nextFrame = stream.readUint16();
		uint32 nextStringPos = stream.readUint16() + offset;
		uint32 streamPos = stream.pos();

		stream.seek(stringPos);
		Common::String label;
		Common::String comment = "";
		char ch;

		uint32 j = stringPos;
		// handle label
		while(j < nextStringPos) {
			j++;
			ch = stream.readByte();
			if (ch == '\r')
				break;
			label += ch;
		}
		// handle label comments
		while(j < nextStringPos) {
			j++;
			ch = stream.readByte();
			if (ch == '\r')
				ch = '\n';
			comment += ch;
		}

		label = _movie->getCast()->decodeString(label).encode(Common::kUtf8);

		_labels->insert(new Label(label, frame, comment));
		stream.seek(streamPos);

		frame = nextFrame;
		stringPos = nextStringPos;
	}

	Common::SortedArray<Label *>::iterator j;

	debugC(2, kDebugLoading, "****** Loading labels");
	for (j = _labels->begin(); j != _labels->end(); ++j) {
		debugC(2, kDebugLoading, "Frame %d, Label '%s', Comment '%s'", (*j)->number, utf8ToPrintable((*j)->name).c_str(), (*j)->comment.c_str());
	}
}

int Score::compareLabels(const void *a, const void *b) {
	return ((const Label *)a)->number - ((const Label *)b)->number;
}

void Score::loadActions(Common::SeekableReadStreamEndian &stream) {
	debugC(2, kDebugLoading, "****** Loading Actions VWAC");

	uint16 count = stream.readUint16() + 1;
	uint32 offset = count * 4 + 2;

	byte id = stream.readByte();

	byte subId = stream.readByte(); // I couldn't find how it used in continuity (except print). Frame actionId = 1 byte.
	uint32 stringPos = stream.readUint16() + offset;

	for (uint16 i = 1; i <= count; i++) {
		uint16 nextId = stream.readByte();
		byte nextSubId = stream.readByte();
		uint32 nextStringPos = stream.readUint16() + offset;
		uint32 streamPos = stream.pos();

		stream.seek(stringPos);

		Common::String script = stream.readString(0, nextStringPos - stringPos);
		_actions[i] = _movie->getCast()->decodeString(script).encode(Common::kUtf8);

		debugC(3, kDebugLoading, "Action index: %d id: %d nextId: %d subId: %d, code: %s", i, id, nextId, subId, _actions[i].c_str());

		stream.seek(streamPos);

		id = nextId;
		subId = nextSubId;
		stringPos = nextStringPos;

		if ((int32)stringPos == stream.size())
			break;
	}

	bool *scriptRefs = (bool *)calloc(_actions.size() + 1, sizeof(bool));

	// Now let's scan which scripts are actually referenced
	for (uint i = 0; i < _frames.size(); i++) {
		if ((uint)_frames[i]->_actionId.member <= _actions.size())
			scriptRefs[_frames[i]->_actionId.member] = true;

		for (uint16 j = 0; j <= _frames[i]->_numChannels; j++) {
			if ((uint)_frames[i]->_sprites[j]->_scriptId.member <= _actions.size())
				scriptRefs[_frames[i]->_sprites[j]->_scriptId.member] = true;
		}
	}

	Common::HashMap<uint16, Common::String>::iterator j;

	if (ConfMan.getBool("dump_scripts"))
		for (j = _actions.begin(); j != _actions.end(); ++j) {
			if (!j->_value.empty())
				_movie->getCast()->dumpScript(j->_value.c_str(), kScoreScript, j->_key);
		}

	for (j = _actions.begin(); j != _actions.end(); ++j) {
		if (!scriptRefs[j->_key]) {
			// Check if it is empty
			bool empty = true;
			Common::U32String u32Script(j->_value);
			for (const Common::u32char_type_t *ptr = u32Script.c_str(); *ptr; ptr++)
				if (!(*ptr == ' ' || *ptr == '-' || *ptr == '\n' || *ptr == '\r' || *ptr == '\t' || *ptr == CONTINUATION)) {
					empty = false;
					break;
				}

			if (!empty)
				warning("Action id %d is not referenced, the code is:\n-----\n%s\n------", j->_key, j->_value.c_str());

			continue;
		}
		if (!j->_value.empty()) {
			_movie->getMainLingoArch()->addCode(j->_value, kScoreScript, j->_key);

			processImmediateFrameScript(j->_value, j->_key);
		}
	}

	free(scriptRefs);
}

Common::String Score::formatChannelInfo() {
	Frame &frame = *_frames[_currentFrame];
	Common::String result;
	int defaultPalette = g_director->getCurrentMovie()->getCast()->_defaultPalette;
	result += Common::String::format("TMPO:   tempo: %d, skipFrameFlag: %d, blend: %d, currentFPS: %d\n",
		frame._tempo, frame._skipFrameFlag, frame._blend, _currentFrameRate);
	if (frame._palette.paletteId) {
		result += Common::String::format("PAL:    paletteId: %d, firstColor: %d, lastColor: %d, flags: %d, cycleCount: %d, speed: %d, frameCount: %d, fade: %d, delay: %d, style: %d, currentId: %d, defaultId: %d\n",
			resolvePaletteId(frame._palette.paletteId), frame._palette.firstColor, frame._palette.lastColor, frame._palette.flags,
			frame._palette.cycleCount, frame._palette.speed, frame._palette.frameCount,
			frame._palette.fade, frame._palette.delay, frame._palette.style, resolvePaletteId(_lastPalette), defaultPalette);
	} else {
		result += Common::String::format("PAL:    paletteId: 000, currentId: %d, defaultId: %d\n", resolvePaletteId(_lastPalette), defaultPalette);
	}
	result += Common::String::format("TRAN:   transType: %d, transDuration: %d, transChunkSize: %d\n",
		frame._transType, frame._transDuration, frame._transChunkSize);
	result += Common::String::format("SND: 1  sound1: %d, soundType1: %d\n", frame._sound1.member, frame._soundType1);
	result += Common::String::format("SND: 2  sound2: %d, soundType2: %d\n", frame._sound2.member, frame._soundType2);
	result += Common::String::format("LSCR:   actionId: %d\n", frame._actionId.member);

	for (int i = 0; i < frame._numChannels; i++) {
		Channel &channel = *_channels[i + 1];
		Sprite &sprite = *channel._sprite;
		if (sprite._castId.member) {
			result += Common::String::format("CH: %-3d castId: %s, visible: %d, [inkData: 0x%02x [ink: %d, trails: %d, line: %d], %dx%d@%d,%d type: %d (%s) fg: %d bg: %d], script: %s, colorcode: 0x%x, blendAmount: 0x%x, unk3: 0x%x, constraint: %d, puppet: %d, stretch: %d\n",
				i + 1, sprite._castId.asString().c_str(), channel._visible, sprite._inkData,
				sprite._ink, sprite._trails, sprite._thickness, channel._width, channel._height,
				channel._currentPoint.x, channel._currentPoint.y,
				sprite._spriteType, spriteType2str(sprite._spriteType), sprite._foreColor, sprite._backColor,
				sprite._scriptId.asString().c_str(), sprite._colorcode, sprite._blendAmount, sprite._unk3,
				channel._constraint, sprite._puppet, sprite._stretch);
		} else {
			result += Common::String::format("CH: %-3d castId: 000\n", i + 1);
		}
	}

	return result;

}

} // End of namespace Director
