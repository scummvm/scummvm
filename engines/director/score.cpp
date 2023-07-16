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
#include "director/score.h"
#include "director/movie.h"
#include "director/sound.h"
#include "director/cursor.h"
#include "director/channel.h"
#include "director/sprite.h"
#include "director/window.h"
#include "director/util.h"
#include "director/castmember/castmember.h"
#include "director/castmember/palette.h"
#include "director/castmember/transition.h"
#include "director/lingo/lingo.h"

namespace Director {

#include "director/palette-fade.h"

Score::Score(Movie *movie) {
	_movie = movie;
	_window = movie->getWindow();
	_vm = _movie->getVM();
	_lingo = _vm->getLingo();

	_soundManager = _window->getSoundManager();

	_puppetPalette = false;
	_paletteTransitionIndex = 0;
	memset(_paletteSnapshotBuffer, 0, 768);

	_labels = nullptr;

	_currentFrameRate = 20;
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
	_skipTransition = false;

	_curFrameNumber = 0;
	_framesStream = nullptr;
	_currentFrame = nullptr;
}

Score::~Score() {
	for (uint i = 0; i < _channels.size(); i++)
		delete _channels[i];

	if (_labels)
		for (auto &it : *_labels)
			delete it;

	delete _labels;

	if (_framesStream)
		delete _framesStream;

	if (_currentFrame) {
		delete _currentFrame;
	}
}

void Score::setPuppetTempo(int16 puppetTempo) {
	_currentFrame->_mainChannels.tempo = puppetTempo;
}

CastMemberID Score::getCurrentPalette() {
	return g_director->_lastPalette;
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

	for (auto &i : *_labels) {
		if (i->name.equalsIgnoreCase(label)) {
			return i->number;
		}
	}

	return 0;
}

Common::String *Score::getLabelList() {
	Common::String *res = new Common::String;

	for (auto &i : *_labels) {
		*res += i->name;
		*res += '\n';
	}

	return res;
}

Common::String *Score::getFrameLabel(uint id) {
	for (auto &i : *_labels) {
		if (i->number == id) {
			return new Common::String(i->name);
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
	if (!_labels)
		return 0;

	int frame = 0;

	for (auto &i : *_labels) {
		if (i->number <= _curFrameNumber)
			frame = i->number;
	}

	return frame;
}

void Score::gotoNext() {
	// we can just try to use the current frame and get the next label
	_nextFrame = getNextLabelNumber(_curFrameNumber);
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
	_curFrameNumber = 1;
	_playState = kPlayStarted;
	_nextFrameTime = 0;

	if (!_currentFrame) {
		warning("Score::startLoop(): Movie has no frames");
		_playState = kPlayStopped;

		return;
	}

	// All frames in the same movie have the same number of channels
	if (_playState != kPlayStopped)
		for (uint i = 0; i < _currentFrame->_sprites.size(); i++)
			_channels.push_back(new Channel(this, _currentFrame->_sprites[i], i));

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
		} else if (g_system->getMillis() < _nextFrameTime) {
			keepWaiting = true;
		}

		if (keepWaiting) {
			if (_movie->_videoPlayback) {
				updateWidgets(true);
				_window->render();
			}

			// Don't process frozen script if we use jump instructions
			// like "go to frame", or open a new movie.
			if (!_nextFrame || _nextFrame == _curFrameNumber) {
				processFrozenScripts();
			}

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
		if (_currentFrame->_mainChannels.transType == 0 && _currentFrame->_mainChannels.trans.isNull()) {
			_lingo->executePerFrameHook(_curFrameNumber, 0);
		}
	}

	_vm->_skipFrameAdvance = false;

	// the exitFrame event handler may have stopped this movie
	if (_playState == kPlayStopped) {
		processFrozenScripts();
		return;
	}

	for (uint ch = 0; ch < _channels.size(); ch++)
		*_currentFrame->_sprites[ch] = *_channels[ch]->_sprite;

	if (!_vm->_playbackPaused) {
		if (_nextFrame) {
			// With the advent of demand loading frames and due to partial updates, we rebuild our channel data
			// when jumping.
			if (_nextFrame != _curFrameNumber)
				rebuildChannelData(_nextFrame);
			_curFrameNumber = _nextFrame;
		}
		else if (!_window->_newMovieStarted)
			_curFrameNumber++;
	}

	_nextFrame = 0;

	if (_curFrameNumber >= getFramesNum()) {
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

			rebuildChannelData(ref.frameI);
			_curFrameNumber = ref.frameI;
		} else {
			if (debugChannelSet(-1, kDebugNoLoop)) {
				_playState = kPlayStopped;
				processFrozenScripts();
				return;
			}

			_curFrameNumber = 1;
		}
	}

	if (_labels != nullptr) {
		for (auto &i : *_labels) {
			if (i->number == _curFrameNumber) {
				_currentLabel = _curFrameNumber;
			}
		}
	}

	loadFrame(_curFrameNumber);

	byte tempo = _currentFrame->_mainChannels.tempo;

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
			debugC(5, kDebugLoading, "Score::update(): setting _nextFrameTime to %d based on a framerate of %d", _nextFrameTime, _currentFrameRate);
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

	debugC(1, kDebugLoading, "******************************  Current frame: %d, time: %d", _curFrameNumber, g_system->getMillis(false));
	g_debugger->frameHook();

	_lingo->executeImmediateScripts(_currentFrame);

	if (_vm->getVersion() >= 600) {
		// _movie->processEvent(kEventBeginSprite);
		// TODO Director 6 step: send beginSprite event to any sprites whose span begin in the upcoming frame
		// _movie->processEvent(kEventPrepareFrame);
		// TODO: Director 6 step: send prepareFrame event to all sprites and the script channel in upcoming frame
	}

	// Window is drawn between the prepareFrame and enterFrame events (Lingo in a Nutshell, p.100)
	renderFrame(_curFrameNumber);
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
	uint32 start = g_system->getMillis(false);
	// Force cursor update if a new movie's started.
	if (_window->_newMovieStarted)
		renderCursor(_movie->getWindow()->getMousePos(), true);

	if (_skipTransition) {
		_window->render();
		_skipTransition = false;
	} else if (!renderTransition(frameId)) {
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
	uint32 end = g_system->getMillis(false);
	debugC(5, kDebugLoading, "Score::renderFrame() finished in %d millis", end - start);
}

bool Score::renderTransition(uint16 frameId) {
	Frame *currentFrame = _currentFrame;
	TransParams *tp = _window->_puppetTransition;

	if (tp) {
		setLastPalette(frameId);
		_window->playTransition(frameId, tp->duration, tp->area, tp->chunkSize, tp->type, currentFrame->_mainChannels.scoreCachedPaletteId);
		delete _window->_puppetTransition;
		_window->_puppetTransition = nullptr;
		return true;
	} else if (currentFrame->_mainChannels.transType) {
		setLastPalette(frameId);
		_window->playTransition(frameId, currentFrame->_mainChannels.transDuration, currentFrame->_mainChannels.transArea, currentFrame->_mainChannels.transChunkSize, currentFrame->_mainChannels.transType, currentFrame->_mainChannels.scoreCachedPaletteId);
		return true;
	} else if (!currentFrame->_mainChannels.trans.isNull()) {
		CastMember *member = _movie->getCastMember(currentFrame->_mainChannels.trans);
		if (member && member->_type == kCastTransition) {
			TransitionCastMember *trans = static_cast<TransitionCastMember *>(member);
			setLastPalette(frameId);
			_window->playTransition(frameId, trans->_durationMillis, trans->_area, trans->_chunkSize, trans->_transType, currentFrame->_mainChannels.scoreCachedPaletteId);
			return true;
		}
	}
	return false;
}

void Score::renderSprites(uint16 frameId, RenderMode mode) {
	if (_window->_newMovieStarted)
		mode = kRenderForceUpdate;

	_movie->_videoPlayback = false;

	for (uint16 i = 0; i < _channels.size(); i++) {
		Channel *channel = _channels[i];
		Sprite *currentSprite = channel->_sprite;
		Sprite *nextSprite = _currentFrame->_sprites[i];

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
	CastMemberID currentPalette = _currentFrame->_mainChannels.palette.paletteId;
	if (currentPalette.isNull())
		return false;

	if (!_currentFrame->_mainChannels.palette.colorCycling &&
		!_currentFrame->_mainChannels.palette.overTime) {

		int frameRate = CLIP<int>(_currentFrame->_mainChannels.palette.speed, 1, 30);

		if (debugChannelSet(-1, kDebugFast))
			frameRate = 30;

		int frameDelay = 1000 / 60;
		int fadeFrames = kFadeColorFrames[frameRate - 1];
		if (_vm->getVersion() >= 500)
			fadeFrames = kFadeColorFramesD5[frameRate - 1];
		byte calcPal[768];

		// Copy the current palette into the snapshot buffer
		memset(_paletteSnapshotBuffer, 0, 768);
		memcpy(_paletteSnapshotBuffer, g_director->getPalette(), g_director->getPaletteColorCount() * 3);
		PaletteV4 *destPal = g_director->getPalette(currentPalette);
		if (!destPal) {
			warning("Unable to fetch palette %s", currentPalette.asString().c_str());
			return false;
		}

		if (_currentFrame->_mainChannels.palette.normal) {
			// If the target palette ID is the same as the previous palette ID,
			// a normal fade is a no-op.
			if (_currentFrame->_mainChannels.palette.paletteId == g_director->_lastPalette) {
				return false;
			}

			// For fade palette transitions, the whole fade happens with
			// the previous frame's layout.
			debugC(2, kDebugImages, "Score::renderPrePaletteCycle(): fading palette to %s over %d frames", currentPalette.asString().c_str(), fadeFrames);
			for (int i = 0; i < fadeFrames; i++) {
				uint32 startTime = g_system->getMillis();
				lerpPalette(
					calcPal,
					_paletteSnapshotBuffer, 256,
					destPal->palette, destPal->length,
					i + 1,
					fadeFrames
				);
				g_director->setPalette(calcPal, 256);
				g_director->draw();
				if (_activeFade) {
					if (!_soundManager->fadeChannel(_activeFade))
					_activeFade = 0;
				}
				// On click, stop loop and reset palette
				if (_vm->processEvents(true)) {
					debugC(2, kDebugImages, "Score::renderPrePaletteCycle(): interrupted, setting palette to %s", currentPalette.asString().c_str());
					g_director->setPalette(currentPalette);
					return true;
				}
				uint32 endTime = g_system->getMillis();
				int diff = (int)frameDelay - (int)(endTime - startTime);
				g_director->delayMillis(MAX(0, diff));
			}

		} else {
			// For fade to black and fade to white palette transitions,
			// the first half happens with the previous frame's layout.

			byte *fadePal = nullptr;
			if (_currentFrame->_mainChannels.palette.fadeToBlack) {
				// Fade everything except color index 0 to black
				debugC(2, kDebugImages, "Score::renderPrePaletteCycle(): fading palette to black over %d frames", fadeFrames);
				fadePal = kBlackPalette;
			} else if (_currentFrame->_mainChannels.palette.fadeToWhite) {
				// Fade everything except color index 255 to white
				debugC(2, kDebugImages, "Score::renderPrePaletteCycle(): fading palette to white over %d frames", fadeFrames);
				fadePal = kWhitePalette;
			} else {
				// Shouldn't reach here
				return false;
			}

			for (int i = 0; i < fadeFrames; i++) {
				uint32 startTime = g_system->getMillis();
				lerpPalette(
					calcPal,
					_paletteSnapshotBuffer, 256,
					fadePal, 256,
					i + 1,
					fadeFrames
				);
				g_director->setPalette(calcPal, 256);
				g_director->draw();
				if (_activeFade) {
					if (!_soundManager->fadeChannel(_activeFade))
					_activeFade = 0;
				}
				// On click, stop loop and reset palette
				if (_vm->processEvents(true)) {
					debugC(2, kDebugImages, "Score::renderPrePaletteCycle(): interrupted, setting palette to %s", currentPalette.asString().c_str());
					g_director->setPalette(currentPalette);
					return true;
				}
				uint32 endTime = g_system->getMillis();
				int diff = (int)frameDelay - (int)(endTime - startTime);
				g_director->delayMillis(MAX(0, diff));
			}
		}
	}
	return false;
}

void Score::setLastPalette(uint16 frameId) {
	if (_puppetPalette)
		return;

	bool isCachedPalette = false;
	CastMemberID currentPalette = _currentFrame->_mainChannels.palette.paletteId;
	// Palette not specified in the frame
	if (currentPalette.isNull()) {
		// Use the score cached palette ID
		isCachedPalette = true;
		currentPalette = _currentFrame->_mainChannels.scoreCachedPaletteId;
		// The cached ID is created before the cast gets loaded; if it's zero,
		// this corresponds to the movie default palette.
		if (currentPalette.isNull())
			currentPalette = g_director->getCurrentMovie()->getCast()->_defaultPalette;
		// If for whatever reason this doesn't resolve, abort.
		if (currentPalette.isNull())
			return;
	}

	// If the palette is defined in the frame and doesn't match
	// the current one, set it
	bool paletteChanged = (currentPalette != g_director->_lastPalette) && (!currentPalette.isNull());
	if (paletteChanged) {
		debugC(2, kDebugImages, "Score::setLastPalette(): palette changed to %s, from %s", currentPalette.asString().c_str(), isCachedPalette ? "cache" :"frame");
		g_director->_lastPalette = currentPalette;
		_paletteTransitionIndex = 0;

		// Switch to a new palette immediately if:
		// - this is color cycling mode, or
		// - the cached palette ID is different (i.e. we jumped in the score)
		if (_currentFrame->_mainChannels.palette.colorCycling || isCachedPalette)
			g_director->setPalette(g_director->_lastPalette);
	}

}

bool Score::isPaletteColorCycling() {
	return _currentFrame->_mainChannels.palette.colorCycling;
}

void Score::renderPaletteCycle(uint16 frameId, RenderMode mode) {
	if (_puppetPalette)
		return;

	// If the palette is defined in the frame and doesn't match
	// the current one, set it
	CastMemberID currentPalette = _currentFrame->_mainChannels.palette.paletteId;
	if (currentPalette.isNull())
		return;

	// For palette cycling, the only thing that is checked is if
	// the palette ID is the same. Different cycling configs with
	// the same palette ID will persist any mutated state.
	// e.g. if you use overTime to cycle the palette partially
	// through a cycle, then switch to doing a full color cycle
	// on the same palette, it will not reset and the weird
	// offset will remain.

	// Cycle speed in FPS
	int speed = _currentFrame->_mainChannels.palette.speed;
	if (speed == 0)
		return;

	if (debugChannelSet(-1, kDebugFast))
		speed = 30;

	// 30 (the maximum) is actually unbounded
	int delay = speed == 30 ? 10 : 1000 / speed;
	if (_currentFrame->_mainChannels.palette.colorCycling) {
		// Cycle the colors of a chosen palette
		int firstColor = _currentFrame->_mainChannels.palette.firstColor;
		int lastColor = _currentFrame->_mainChannels.palette.lastColor;

		if (_currentFrame->_mainChannels.palette.overTime) {
			// Do a single color step in one frame transition
			debugC(2, kDebugImages, "Score::renderPaletteCycle(): color cycle palette %s, from colors %d to %d, by 1 frame", currentPalette.asString().c_str(), firstColor, lastColor);
			g_director->shiftPalette(firstColor, lastColor, false);
			g_director->draw();
		} else {
			// Short circuit for few frames renderer
			if (debugChannelSet(-1, kDebugFast)) {
				g_director->setPalette(currentPalette);
				return;
			}

			// Do a full color cycle in one frame transition
			int steps = lastColor - firstColor + 1;
			debugC(2, kDebugImages, "Score::renderPaletteCycle(): color cycle palette %s, from colors %d to %d, over %d steps %d times", currentPalette.asString().c_str(), firstColor, lastColor, steps, _currentFrame->_mainChannels.palette.cycleCount);
			for (int i = 0; i < _currentFrame->_mainChannels.palette.cycleCount; i++) {
				for (int j = 0; j < steps; j++) {
					uint32 startTime = g_system->getMillis();
					g_director->shiftPalette(firstColor, lastColor, false);
					g_director->draw();
					if (_activeFade) {
						if (!_soundManager->fadeChannel(_activeFade))
						_activeFade = 0;
					}
					// On click, stop loop and reset palette
					if (_vm->processEvents(true)) {
						g_director->setPalette(currentPalette);
						return;
					}
					uint32 endTime = g_system->getMillis();
					int diff = (int)delay - (int)(endTime - startTime);
					g_director->delayMillis(MAX(0, diff));
				}
				if (_currentFrame->_mainChannels.palette.autoReverse) {
					for (int j = 0; j < steps; j++) {
						uint32 startTime = g_system->getMillis();
						g_director->shiftPalette(firstColor, lastColor, true);
						g_director->draw();
						if (_activeFade) {
							if (!_soundManager->fadeChannel(_activeFade))
							_activeFade = 0;
						}
						// On click, stop loop and reset palette
						if (_vm->processEvents(true)) {
							g_director->setPalette(currentPalette);
							return;
						}
						uint32 endTime = g_system->getMillis();
						int diff = (int)delay - (int)(endTime - startTime);
						g_director->delayMillis(MAX(0, diff));
					}
				}
			}
		}
	} else {
		// Transition from the current palette to a new palette
		PaletteV4 *destPal = g_director->getPalette(currentPalette);
		if (!destPal) {
			warning("Score::renderPaletteCycle(): no match for palette id %s", currentPalette.asString().c_str());
			return;
		}
		int frameCount = _currentFrame->_mainChannels.palette.frameCount;
		byte calcPal[768];

		if (_currentFrame->_mainChannels.palette.overTime) {
			// Transition over a series of frames
			if (_paletteTransitionIndex == 0) {
				// Copy the current palette into the snapshot buffer
				memset(_paletteSnapshotBuffer, 0, 768);
				memcpy(_paletteSnapshotBuffer, g_director->getPalette(), g_director->getPaletteColorCount() * 3);
				debugC(2, kDebugImages, "Score::renderPaletteCycle(): fading palette to %s over %d frames", currentPalette.asString().c_str(), frameCount);
			}

			if (_currentFrame->_mainChannels.palette.normal) {
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
				if (_currentFrame->_mainChannels.palette.fadeToBlack) {
					// Fade everything except color index 0 to black
					fadePal = kBlackPalette;
				} else if (_currentFrame->_mainChannels.palette.fadeToWhite) {
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
				debugC(2, kDebugImages, "Score::renderPaletteCycle(): setting palette to %s", currentPalette.asString().c_str());
				g_director->setPalette(currentPalette);
				return;
			}

			// Do a full cycle in one frame transition
			// For normal mode, we've already faded the palette in renderPrePaletteCycle
			if (!_currentFrame->_mainChannels.palette.normal) {
				byte *fadePal = nullptr;
				if (_currentFrame->_mainChannels.palette.fadeToBlack) {
					// Fade everything except color index 0 to black
					fadePal = kBlackPalette;
				} else if (_currentFrame->_mainChannels.palette.fadeToWhite) {
					// Fade everything except color index 255 to white
					fadePal = kWhitePalette;
				} else {
					// Shouldn't reach here
					return;
				}
				int frameRate = CLIP<int>(_currentFrame->_mainChannels.palette.speed, 1, 30);

				if (debugChannelSet(-1, kDebugFast))
					frameRate = 30;

				int frameDelay = 1000 / 60;
				int fadeFrames = kFadeColorFrames[frameRate - 1];
				if (_vm->getVersion() >= 500)
					fadeFrames = kFadeColorFramesD5[frameRate - 1];

				// Wait for a fixed time
				g_director->setPalette(fadePal, 256);
				g_director->draw();
				for (int i = 0; i < kFadeColorWait; i++) {
					uint32 startTime = g_system->getMillis();
					if (_activeFade) {
						if (!_soundManager->fadeChannel(_activeFade))
						_activeFade = 0;
					}
					// On click, stop loop and reset palette
					if (_vm->processEvents(true)) {
						debugC(2, kDebugImages, "Score::renderPaletteCycle(): interrupted, setting palette to %s", currentPalette.asString().c_str());
						g_director->setPalette(currentPalette);
						return;
					}
					uint32 endTime = g_system->getMillis();
					int diff = (int)frameDelay - (int)(endTime - startTime);
					g_director->delayMillis(MAX(0, diff));
				}

				debugC(2, kDebugImages, "Score::renderPaletteCycle(): fading palette to %s over %d frames", currentPalette.asString().c_str(), fadeFrames);

				for (int i = 0; i < fadeFrames; i++) {
					uint32 startTime = g_system->getMillis();
					lerpPalette(
						calcPal,
						fadePal, 256,
						destPal->palette, destPal->length,
						i + 1,
						fadeFrames
					);
					g_director->setPalette(calcPal, 256);
					g_director->draw();
					if (_activeFade) {
						if (!_soundManager->fadeChannel(_activeFade))
						_activeFade = 0;
					}
					// On click, stop loop and reset palette
					if (_vm->processEvents(true)) {
						debugC(2, kDebugImages, "Score::renderPaletteCycle(): interrupted, setting palette to %s", currentPalette.asString().c_str());
						g_director->setPalette(currentPalette);
						return;
					}
					uint32 endTime = g_system->getMillis();
					int diff = (int)frameDelay - (int)(endTime - startTime);
					g_director->delayMillis(MAX(0, diff));
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

		Common::String buildDir = Common::String::format("%s/%s", ConfMan.get("screenshotpath").c_str(),
			g_director->getTargetName().c_str());

		// We run for the first time, let's check if we had the directory previously
		if (_previousBuildBotBuild == -1) {
			Common::FSNode dir(buildDir);

			if (!dir.exists())
				_previousBuildBotBuild = 0; // We will skip attempts to search screenshots
			else
				_previousBuildBotBuild = atoi(buildNumber) - 1;
		}

		int prevbuild = _previousBuildBotBuild;

		// Now we try to find any previous dump
		while (prevbuild > 0) {
			filename = Common::String::format("%s/%d/%s-%d.png", buildDir.c_str(), prevbuild, prefix.c_str(), g_director->_framesRan);

			Common::FSNode fs(filename);

			if (fs.exists())
				break;

			prevbuild--;
		}

		// We found a previous screenshot. Let's compare it
		if (prevbuild > 0) {
			Common::FSNode fs(filename);
			Image::PNGDecoder decoder;
			Common::SeekableReadStream *stream = fs.createReadStream();

			if (stream && decoder.loadStream(*stream)) {
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

		// We are here because we either have nothing to compare with or
		// the screenshot was different from the previous one.
		//
		// Regenerate file name with the correct build number
		filename = Common::String::format("%s/%s/%s-%d.png", buildDir.c_str(), buildNumber, prefix.c_str(), g_director->_framesRan);
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
		warning("Score::getSpriteById(): sprite on frame %d with id %d not found", _curFrameNumber, id);
		return nullptr;
	}
}

Sprite *Score::getOriginalSpriteById(uint16 id) {
	if (id < _currentFrame->_sprites.size())
		return _currentFrame->_sprites[id];
	warning("Score::getOriginalSpriteById(%d): out of bounds, >= %d", id, _currentFrame->_sprites.size());
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
	debugC(5, kDebugSound, "playSoundChannel(): Sound1 %s Sound2 %s", _currentFrame->_mainChannels.sound1.asString().c_str(), _currentFrame->_mainChannels.sound2.asString().c_str());
	DirectorSound *sound = _window->getSoundManager();

	if (sound->isChannelPuppet(1)) {
		sound->playPuppetSound(1);
	} else if (!puppetOnly) {
		if (_currentFrame->_mainChannels.soundType1 >= kMinSampledMenu && _currentFrame->_mainChannels.soundType1 <= kMaxSampledMenu) {
			sound->playExternalSound(_currentFrame->_mainChannels.soundType1, _currentFrame->_mainChannels.sound1.member, 1);
		} else {
			sound->playCastMember(_currentFrame->_mainChannels.sound1, 1);
		}
	}

	if (sound->isChannelPuppet(2)) {
		sound->playPuppetSound(2);
	} else if (!puppetOnly) {
		if (_currentFrame->_mainChannels.soundType2 >= kMinSampledMenu && _currentFrame->_mainChannels.soundType2 <= kMaxSampledMenu) {
			sound->playExternalSound(_currentFrame->_mainChannels.soundType2, _currentFrame->_mainChannels.sound2.member, 2);
		} else {
			sound->playCastMember(_currentFrame->_mainChannels.sound2, 2);
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

	// Setup our streams for frames processing
	uint dataSize = stream.size();
	byte *data = (byte *)malloc(dataSize);
	stream.read(data, dataSize);

	_framesStream = new Common::MemoryReadStreamEndian(data, dataSize, stream.isBE(), DisposeAfterUse::YES);

	if (debugChannelSet(8, kDebugLoading)) {
		_framesStream->hexdump(_framesStream->size());
	}

	_framesStreamSize = _framesStream->readUint32();

	if (version < kFileVer400) {
		_numChannelsDisplayed = 30;
	} else if (version >= kFileVer400 && version < kFileVer600) {
		uint32 frame1Offset = _framesStream->readUint32();
		/* uint32 numOfFrames = */ _framesStream->readUint32();
		_framesVersion = _framesStream->readUint16();
		uint16 spriteRecordSize = _framesStream->readUint16();
		_numChannels = _framesStream->readUint16();

		if (_framesVersion > 13) {
			_numChannelsDisplayed = _framesStream->readUint16();
		} else {
			if (_framesVersion <= 7)    // Director5
				_numChannelsDisplayed = 48;
			else
				_numChannelsDisplayed = 120;    // D6

			_framesStream->readUint16(); // Skip
		}

		warning("STUB: Score::loadFrames(): frame1Offset: %x, version: %x, spriteRecordSize: %x, numChannels: %x, numChannelsDisplayed: %x",
			frame1Offset, _framesVersion, spriteRecordSize, _numChannels, _numChannelsDisplayed);
		// Unknown, some bytes - constant (refer to contuinity).
	}

	// partically by channels, hence we keep it and read the score from left to right
	// TODO Merge it with shared cast
	_currentFrame = new Frame(this, _numChannelsDisplayed);

	// This makes all indexing simpler
	_frameOffsets.push_back(0);

	_currentTempo = 0;
	_currentPaletteId = CastMemberID(0, 0);

	// Prepare frameOffsets
	_version = version;
	_frameOffsets.push_back(_framesStream->pos());

	// Pre-computing number of frames, as sometimes the frameNumber in stream mismatches
	debugC(1, kDebugLoading, "Score::loadFrames(): Precomputing total number of frames!");

	// Calculate number of frames and their positions
	// numOfFrames in the header is often incorrect
	for (_numFrames = 1; loadFrame(_numFrames); _numFrames++) {
		if (_framesStream->pos() < _framesStreamSize) {
			// Record the starting offset for next frame
			_frameOffsets.push_back(_framesStream->pos());
		}
	}

	debugC(1, kDebugLoading, "Score::loadFrames(): Calculated, total number of frames %d!", _numFrames);

	_currentFrame->reset();

	loadFrame(1);

	// Read over frame offset array and print each item
	for (uint i = 0; i < _frameOffsets.size(); i++) {
		debugC(8, kDebugLoading, "Score::loadFrames(): Frame %d, offset %ld", i, _frameOffsets[i]);
	}

	debugC(1, kDebugLoading, "Score::loadFrames(): Number of frames: %d, framesStreamSize: %d", _numFrames, _framesStreamSize);
}

bool Score::loadFrame(int frameNum) {
	debugC(7, kDebugLoading, "****** Frame request %d, current pos: %ld", frameNum, _framesStream->pos());

	// Read existing frame (ie already visited)
	if (frameNum < (int)_frameOffsets.size()) {
		// Seek to existing frame in offsets
		_framesStream->seek(_frameOffsets[frameNum]);
	} else {
		// Seek to latest frame
		_framesStream->seek(_frameOffsets[_frameOffsets.size() - 1]);
		_curFrameNumber = _frameOffsets.size() - 1;
	}

	debugC(7, kDebugLoading, "****** Frame request %d, current offset %ld", frameNum, _framesStream->pos());

	while (_frameOffsets.size() <= (uint)frameNum && readOneFrame())
		_curFrameNumber++;

	// Finally read the target frame!
	bool isFrameRead = readOneFrame();
	if (!isFrameRead)
		return false;

	// Load frame cast
	setSpriteCasts();

	return true;
}

// Rebuild channel data from frame 1 to frameNum, to prepare for next frame loading
// Why? Because director works on concept of "delta" changes between frame, to save
// space each frame only contains changes from previous frame, so we need to rebuild
// whole channel data if we were to make a very big jump. This is essential because
// without it we might have leftover casts/properties which will interfere with channel
// data after the jump
void Score::rebuildChannelData(int frameNum) {
	// Builds channel data from frame 1 to frame
	if (frameNum > (int)_numFrames) {
		warning("Score::rebuildChannelData(): frameNum %d is greater than total frames %d", frameNum, _numFrames);
		return;
	}

	// Lock variables
	int curFrameNumber = _curFrameNumber;

	_currentFrame->reset();

	_framesStream->seek(_frameOffsets[1]); // Seek to frame 1
	for (int i = 1; i < frameNum; i++) {
		readOneFrame();
	}

	// Unlock variables
	_curFrameNumber = curFrameNumber;
}

bool Score::readOneFrame() {
	uint16 channelSize;
	uint16 channelOffset;

	if (_framesStream->pos() >= _framesStreamSize || _framesStream->eos())
		return false;

	uint16 frameSize = _framesStream->readUint16();
	assert(frameSize < _framesStreamSize);

	debugC(3, kDebugLoading, "++++++++++ score load frame %d (frameSize %d) saveOffset", _curFrameNumber, frameSize);
	if (debugChannelSet(8, kDebugLoading)) {
		_framesStream->hexdump(frameSize);
	}
	if (frameSize > 0) {
		frameSize -= 2;

		while (frameSize != 0) {

			if (_vm->getVersion() < 400) {
				channelSize = _framesStream->readByte() * 2;
				channelOffset = _framesStream->readByte() * 2;
				frameSize -= channelSize + 2;
			} else {
				channelSize = _framesStream->readUint16();
				channelOffset = _framesStream->readUint16();
				frameSize -= channelSize + 4;
			}

			_currentFrame->readChannel(*_framesStream, channelOffset, channelSize, _version);
		}

		if (debugChannelSet(4, kDebugLoading)) {
			debugC(4, kDebugLoading, "%s", _currentFrame->formatChannelInfo().c_str());
		}

		debugC(8, kDebugLoading, "Score::readOneFrame(): Frame %d actionId: %s", _curFrameNumber, _currentFrame->_mainChannels.actionId.asString().c_str());
		return true;
	} else {
		warning("Score::readOneFrame(): Zero sized frame!? exiting loop until we know what to do with the tags that follow.");
	}

	return false; // Error in loading frame
}

Frame *Score::getFrameData(int frameNum){
	// This function is for previewing selected frame,
	// It doesn't make any changes to current render state
	// In case of any problem, it returns nullptr
	// Be sure to delete this frame after use

	// Backup variables
	int curFrameNumber = _curFrameNumber;
	Frame *frame = _currentFrame;
	_currentFrame = nullptr; // To avoid later deletion of frame inside renderOneFrame()

	bool isFrameRead = loadFrame(frameNum);

	// Start restoring all states
	_curFrameNumber = curFrameNumber;
	_currentFrame = frame;

	if (isFrameRead) {
		return _currentFrame;
	}
	return nullptr;
}

void Score::setSpriteCasts() {
	// Update sprite cache of cast pointers/info
	for (uint16 j = 0; j < _currentFrame->_sprites.size(); j++) {
		_currentFrame->_sprites[j]->setCast(_currentFrame->_sprites[j]->_castId);

		debugC(5, kDebugImages, "Score::setSpriteCasts(): Frame: 0 Channel: %d castId: %s type: %d (%s)",
			 j, _currentFrame->_sprites[j]->_castId.asString().c_str(), _currentFrame->_sprites[j]->_spriteType,
			spriteType2str(_currentFrame->_sprites[j]->_spriteType));
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
		while (j < nextStringPos) {
			j++;
			ch = stream.readByte();
			if (ch == '\r')
				break;
			label += ch;
		}
		// handle label comments
		while (j < nextStringPos) {
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

	debugC(2, kDebugLoading, "****** Loading labels");
	for (auto &j : *_labels) {
		debugC(2, kDebugLoading, "Frame %d, Label '%s', Comment '%s'", j->number, utf8ToPrintable(j->name).c_str(), j->comment.c_str());
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

	for (auto &j : _actions) {
		if (!j._value.empty()) {
			if (ConfMan.getBool("dump_scripts"))
				_movie->getCast()->dumpScript(j._value.c_str(), kScoreScript, j._key);

			_movie->getMainLingoArch()->addCode(j._value, kScoreScript, j._key);

			processImmediateFrameScript(j._value, j._key);
		}
	}
}

Common::String Score::formatChannelInfo() {
	Frame &frame = *_currentFrame;
	Common::String result;
	CastMemberID defaultPalette = g_director->getCurrentMovie()->getCast()->_defaultPalette;
	result += Common::String::format("TMPO:   tempo: %d, skipFrameFlag: %d, blend: %d, currentFPS: %d\n",
		frame._mainChannels.tempo, frame._mainChannels.skipFrameFlag, frame._mainChannels.blend, _currentFrameRate);
	if (!frame._mainChannels.palette.paletteId.isNull()) {
		result += Common::String::format("PAL:    paletteId: %s, firstColor: %d, lastColor: %d, flags: %d, cycleCount: %d, speed: %d, frameCount: %d, fade: %d, delay: %d, style: %d, currentId: %s, defaultId: %s\n",
			frame._mainChannels.palette.paletteId.asString().c_str(), frame._mainChannels.palette.firstColor, frame._mainChannels.palette.lastColor, frame._mainChannels.palette.flags,
			frame._mainChannels.palette.cycleCount, frame._mainChannels.palette.speed, frame._mainChannels.palette.frameCount,
			frame._mainChannels.palette.fade, frame._mainChannels.palette.delay, frame._mainChannels.palette.style, g_director->_lastPalette.asString().c_str(), defaultPalette.asString().c_str());
	} else {
		result += Common::String::format("PAL:    paletteId: 000, currentId: %s, defaultId: %s\n", g_director->_lastPalette.asString().c_str(), defaultPalette.asString().c_str());
	}
	result += Common::String::format("TRAN:   transType: %d, transDuration: %d, transChunkSize: %d\n",
		frame._mainChannels.transType, frame._mainChannels.transDuration, frame._mainChannels.transChunkSize);
	result += Common::String::format("SND: 1  sound1: %d, soundType1: %d\n", frame._mainChannels.sound1.member, frame._mainChannels.soundType1);
	result += Common::String::format("SND: 2  sound2: %d, soundType2: %d\n", frame._mainChannels.sound2.member, frame._mainChannels.soundType2);
	result += Common::String::format("LSCR:   actionId: %d\n", frame._mainChannels.actionId.member);

	for (int i = 0; i < frame._numChannels; i++) {
		Channel &channel = *_channels[i + 1];
		Sprite &sprite = *channel._sprite;
		if (sprite._castId.member) {
			result += Common::String::format("CH: %-3d castId: %s, visible: %d, [inkData: 0x%02x [ink: %d, trails: %d, line: %d], %dx%d@%d,%d type: %d (%s) fg: %d bg: %d], script: %s, colorcode: 0x%x, blendAmount: 0x%x, unk3: 0x%x, constraint: %d, puppet: %d, stretch: %d, moveable: %d\n",
				i + 1, sprite._castId.asString().c_str(), channel._visible, sprite._inkData,
				sprite._ink, sprite._trails, sprite._thickness, channel._width, channel._height,
				channel._currentPoint.x, channel._currentPoint.y,
				sprite._spriteType, spriteType2str(sprite._spriteType), sprite._foreColor, sprite._backColor,
				sprite._scriptId.asString().c_str(), sprite._colorcode, sprite._blendAmount, sprite._unk3,
				channel._constraint, sprite._puppet, sprite._stretch, sprite._moveable);
		} else {
			result += Common::String::format("CH: %-3d castId: 000\n", i + 1);
		}
	}

	return result;

}

} // End of namespace Director
