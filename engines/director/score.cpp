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

#include "common/config-manager.h"
#include "common/file.h"
#include "common/memstream.h"
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

Score::Score(Movie *movie) {
	_movie = movie;
	_window = movie->getWindow();
	_vm = _movie->getVM();
	_lingo = _vm->getLingo();

	_soundManager = _window->getSoundManager();

	_puppetTempo = 0x00;
	_puppetPalette = false;
	_lastPalette = 0;

	_labels = nullptr;

	_currentFrameRate = 20;
	_currentFrame = 0;
	_nextFrame = 0;
	_currentLabel = 0;
	_nextFrameTime = 0;
	_waitForChannel = 0;
	_cursorDirty = false;
	_waitForClick = false;
	_waitForClickCursor = false;
	_activeFade = 0;
	_playState = kPlayNotStarted;

	_numChannelsDisplayed = 0;

	_framesRan = 0; // used by kDebugFewFramesOnly and kDebugScreenshot
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
	return _frames[_currentFrame]->_palette.paletteId;
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
	// This command has the playback head contonuously return to the first marker to to the left and then loop back.
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
		warning("Score::startLoop(): ran frame %0d", _framesRan);
		_framesRan++;
	}

	if (debugChannelSet(-1, kDebugFewFramesOnly) && _framesRan > 9) {
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
		} else if (g_system->getMillis() < _nextFrameTime && !_nextFrame) {
			keepWaiting = true;
		}

		if (keepWaiting) {
			if (_movie->_videoPlayback) {
				updateWidgets(true);
				_window->render();
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
		if (_frames[_currentFrame]->_transType == 0) {
			_lingo->executePerFrameHook(_currentFrame, 0);
		}
	}

	_vm->_skipFrameAdvance = false;

	// the exitFrame event handler may have stopped this movie
	if (_playState == kPlayStopped)
		return;

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
				return;
			}

			_currentFrame = ref.frameI;
		} else {
			if (debugChannelSet(-1, kDebugNoLoop)) {
				_playState = kPlayStopped;
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

	debugC(1, kDebugImages, "******************************  Current frame: %d", _currentFrame);

	uint initialCallStackSize = _window->_callstack.size();

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
		_movie->processEvent(kEventEnterFrame); // Triggers the frame script in D2-3, explicit enterFrame handlers in D4+
		if ((_vm->getVersion() >= 300 && _vm->getVersion() < 400) || _movie->_allowOutdatedLingo) {
			// Movie version of enterFrame, for D3 only. The D3 Interactivity Manual claims
			// "This handler executes before anything else when the playback head moves."
			// but this is incorrect. The frame script is executed first.
			_movie->processEvent(kEventStepMovie);
		}
		if (_movie->_timeOutPlay)
			_movie->_lastTimeOut = _vm->getMacTicks();
	}
	// TODO Director 6 - another order

	// TODO: Figure out when exactly timeout events are processed
	if (_vm->getMacTicks() - _movie->_lastTimeOut >= _movie->_timeOutLength) {
		_movie->processEvent(kEventTimeout);
		_movie->_lastTimeOut = _vm->getMacTicks();
	}

	// If we have more call stack frames than we started with, then we have a newly
	// added frozen context. We'll deal with that later.
	if (_window->_callstack.size() == initialCallStackSize) {
		// We may have a frozen Lingo context from func_goto.
		// Now that we've entered a new frame, let's unfreeze that context.
		if (g_lingo->_freezeContext) {
			debugC(1, kDebugLingoExec, "Score::update(): Unfreezing Lingo context");
			g_lingo->_freezeContext = false;
			g_lingo->execute();
		}
	}

	byte tempo = _frames[_currentFrame]->_tempo;
	if (tempo) {
		_puppetTempo = 0;
	} else if (_puppetTempo) {
		tempo = _puppetTempo;
	}

	if (tempo) {
		if (tempo > 161) {
			// Delay
			_nextFrameTime = g_system->getMillis() + (256 - tempo) * 1000;
		} else if (tempo <= 120) {
			// FPS
			_currentFrameRate = tempo;
			_nextFrameTime = g_system->getMillis() + 1000.0 / (float)_currentFrameRate;
		} else {
			if (tempo == 128) {
				_waitForClick = true;
				_waitForClickCursor = false;
				renderCursor(_movie->getWindow()->getMousePos());
			} else if (tempo == 135) {
				// Wait for sound channel 1
				_waitForChannel = 1;
			} else if (tempo == 134) {
				// Wait for sound channel 2
				_waitForChannel = 2;
			} else {
				warning("STUB: tempo %d", tempo);
			}
			_nextFrameTime = g_system->getMillis();
		}
	} else {
		_nextFrameTime = g_system->getMillis() + 1000.0 / (float)_currentFrameRate;
	}

	if (debugChannelSet(-1, kDebugSlow))
		_nextFrameTime += 1000;
}

void Score::renderFrame(uint16 frameId, RenderMode mode) {
	// Force cursor update if a new movie's started.
	if (_window->_newMovieStarted)
		renderCursor(_movie->getWindow()->getMousePos(), true);

	if (!renderTransition(frameId))
		renderSprites(frameId, mode);

	int currentPalette = _frames[frameId]->_palette.paletteId;
	if (!_puppetPalette && currentPalette != _lastPalette && currentPalette) {
		_lastPalette = currentPalette;
		g_director->setPalette(resolvePaletteId(currentPalette));
	}

	_window->render();

	playSoundChannel(frameId);
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
		_window->playTransition(tp->duration, tp->area, tp->chunkSize, tp->type, frameId);

		delete _window->_puppetTransition;
		_window->_puppetTransition = nullptr;
		return true;
	} else if (currentFrame->_transType) {
		_window->playTransition(currentFrame->_transDuration, currentFrame->_transArea, currentFrame->_transChunkSize, currentFrame->_transType, frameId);
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

			if (currentSprite->_cast && currentSprite->_cast->_erase) {
				auto cast = currentSprite->_cast->getCast()->_loadedCast;
				for (auto it = cast->begin(); it != cast->end(); ++it) {
					if (it->_value == currentSprite->_cast) {
						cast->erase(it);
						currentSprite->_cast->_erase = false;
						break;
					}
				}

				currentSprite->setCast(currentSprite->_castId);
				nextSprite->setCast(nextSprite->_castId);
			}

			channel->setClean(nextSprite, i);
			// Check again to see if a video has just been started by setClean.
			if (channel->isActiveVideo())
				_movie->_videoPlayback = true;

			_window->addDirtyRect(channel->getBbox());
			debugC(2, kDebugImages, "Score::renderSprites(): CH: %-3d castId: %s [ink: %d, puppet: %d, moveable: %d, visible: %d] [bbox: %d,%d,%d,%d] [type: %d fg: %d bg: %d] [script: %s]", i, currentSprite->_castId.asString().c_str(), currentSprite->_ink, currentSprite->_puppet, currentSprite->_moveable, channel->_visible, PRINT_RECT(channel->getBbox()), currentSprite->_spriteType, currentSprite->_foreColor, currentSprite->_backColor, currentSprite->_scriptId.asString().c_str());
		} else {
			channel->setClean(nextSprite, i, true);
		}

		// update editable text channel after we render the sprites. because for the current frame, we may get those sprites only when we finished rendering
		// (because we are creating widgets and setting active state when we rendering sprites)
		if (channel->isActiveText())
			_movie->_currentEditableTextChannel = i;
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
		if (cast && (cast->_type != kCastDigitalVideo || hasVideoPlayback) && cast->isModified()) {
			channel->replaceWidget();
			_window->addDirtyRect(channel->getBbox());
		}
	}
}

void Score::screenShot() {
	Graphics::Surface rawSurface = _window->getSurface()->rawSurface();
	const Graphics::PixelFormat requiredFormat_4byte(4, 8, 8, 8, 8, 0, 8, 16, 24);
	Graphics::Surface *newSurface = rawSurface.convertTo(requiredFormat_4byte, _vm->getPalette());
	Common::String currentPath = _vm->getCurrentPath().c_str();
	Common::replace(currentPath, Common::String(g_director->_dirSeparator), "-"); // exclude dir separator from screenshot filename prefix
	Common::String prefix = Common::String::format("%s%s", currentPath.c_str(), _movie->getMacName().c_str());
	Common::String filename = dumpScriptName(prefix.c_str(), kMovieScript, _framesRan, "png");

	Common::DumpFile screenshotFile;
	if (screenshotFile.open(filename)) {
#ifdef USE_PNG
		Image::writePNG(screenshotFile, *newSurface);
#else
		warning("Screenshot requested, but PNG support is not compiled in");
#endif
	}

	newSurface->free();
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

void Score::playSoundChannel(uint16 frameId) {
	Frame *frame = _frames[frameId];

	debugC(5, kDebugLoading, "playSoundChannel(): Sound1 %s Sound2 %s", frame->_sound1.asString().c_str(), frame->_sound2.asString().c_str());
	DirectorSound *sound = _window->getSoundManager();

	if (sound->isChannelPuppet(1)) {
		sound->playPuppetSound(1);
	} else if (frame->_soundType1 >= kMinSampledMenu && frame->_soundType1 <= kMaxSampledMenu) {
		sound->playExternalSound(frame->_soundType1, frame->_sound1.member, 1);
	} else {
		sound->playCastMember(frame->_sound1, 1);
	}

	if (sound->isChannelPuppet(2)) {
		sound->playPuppetSound(2);
	} else if (frame->_soundType2 >= kMinSampledMenu && frame->_soundType2 <= kMaxSampledMenu) {
		sound->playExternalSound(frame->_soundType2, frame->_sound2.member, 2);
	} else {
		sound->playCastMember(frame->_sound2, 2);
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

			debugC(1, kDebugImages, "Score::setSpriteCasts(): Frame: %d Channel: %d castId: %s type: %d", i, j, _frames[i]->_sprites[j]->_castId.asString().c_str(), _frames[i]->_sprites[j]->_spriteType);
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

} // End of namespace Director
