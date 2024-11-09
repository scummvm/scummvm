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

#include "common/debug.h"
#include "common/file.h"
#include "common/system.h"
#include "common/macresman.h"

#include "graphics/macgui/macwindowmanager.h"

#include "director/director.h"
#include "director/archive.h"
#include "director/cast.h"
#include "director/debugger.h"
#include "director/lingo/lingo.h"
#include "director/movie.h"
#include "director/window.h"
#include "director/score.h"
#include "director/channel.h"
#include "director/sound.h"
#include "director/sprite.h"
#include "director/castmember/castmember.h"

namespace Director {

Window::Window(int id, bool scrollable, bool resizable, bool editable, Graphics::MacWindowManager *wm, DirectorEngine *vm, bool isStage)
	: MacWindow(id, scrollable, resizable, editable, wm), Object<Window>("Window") {
	_vm = vm;
	_isStage = isStage;
	_stageColor = _wm->_colorBlack;
	_puppetTransition = nullptr;
	_soundManager = new DirectorSound(this);
	_lingoState = new LingoState;
	_lingoPlayState = nullptr;

	_currentMovie = nullptr;
	_mainArchive = nullptr;
	_nextMovie.frameI = -1;
	_newMovieStarted = true;

	_objType = kWindowObj;
	_startFrame = _vm->getStartMovie().startFrame;

	_windowType = -1;
	_isModal = false;

	updateBorderType();

	_draggable = !_isStage;
}

Window::~Window() {
	delete _lingoState;
	if (_lingoPlayState)
		delete _lingoPlayState;
	delete _soundManager;
	delete _currentMovie;
	for (uint i = 0; i < _frozenLingoStates.size(); i++)
		delete _frozenLingoStates[i];
	if (_puppetTransition)
		delete _puppetTransition;
}

void Window::decRefCount() {
	*_refCount -= 1;
	if (*_refCount <= 0) {
		g_director->_wm->removeWindow(this);
		g_director->_wm->removeMarked();
	}
}

void Window::invertChannel(Channel *channel, const Common::Rect &destRect) {
	const Graphics::Surface *mask;

	// in D3, we have inverted QDshape
	if (channel->_sprite->isQDShape() && channel->_sprite->_ink == kInkTypeMatte)
		mask = channel->_sprite->getQDMatte();
	else
		mask = channel->getMask(true);

	Common::Rect srcRect = channel->getBbox();
	srcRect.clip(destRect);

	// let compiler to optimize it
	int xoff = srcRect.left - channel->getBbox().left;
	int yoff = srcRect.top - channel->getBbox().top;

	if (_wm->_pixelformat.bytesPerPixel == 1) {
		for (int i = 0; i < srcRect.height(); i++) {
			byte *src = (byte *)_composeSurface->getBasePtr(srcRect.left, srcRect.top + i);
			const byte *msk = mask ? (const byte *)mask->getBasePtr(xoff, yoff + i) : nullptr;

			for (int j = 0; j < srcRect.width(); j++, src++)
				if (!mask || (msk && (*msk++)))
					*src = _wm->inverter(*src);
		}
	} else {

		for (int i = 0; i < srcRect.height(); i++) {
			uint32 *src = (uint32 *)_composeSurface->getBasePtr(srcRect.left, srcRect.top + i);
			const byte *msk = mask ? (const byte *)mask->getBasePtr(xoff, yoff + i) : nullptr;

			for (int j = 0; j < srcRect.width(); j++, src++)
				if (!mask || (msk && (*msk++)))
					*src = _wm->inverter(*src);
		}
	}
}

void Window::drawFrameCounter(Graphics::ManagedSurface *blitTo) {
	const Graphics::Font *font = FontMan.getFontByUsage(Graphics::FontManager::kConsoleFont);
	Common::String msg = Common::String::format("Frame: %d", g_director->getCurrentMovie()->getScore()->getCurrentFrameNum());
	uint32 width = font->getStringWidth(msg);

	blitTo->fillRect(Common::Rect(blitTo->w - 3 - width, 1, blitTo->w - 1, font->getFontHeight() + 1), _wm->_colorBlack);
	font->drawString(blitTo, msg, blitTo->w - 1 - width, 3, width, _wm->_colorBlack);
	font->drawString(blitTo, msg, blitTo->w - 2 - width, 2, width, _wm->_colorWhite);
}

bool Window::render(bool forceRedraw, Graphics::ManagedSurface *blitTo) {
	if (!_currentMovie)
		return false;

	if (!blitTo)
		blitTo = _composeSurface;

	if (forceRedraw) {
		blitTo->clear(_stageColor);
		markAllDirty();
	} else {
		if (_dirtyRects.size() == 0 && _currentMovie->_videoPlayback == false) {
			if (g_director->_debugDraw & kDebugDrawFrame) {
				drawFrameCounter(blitTo);

				_contentIsDirty = true;
			}

			return false;
		}

		mergeDirtyRects();
	}

	Channel *hiliteChannel = _currentMovie->getScore()->getChannelById(_currentMovie->_currentHiliteChannelId);

	uint32 renderStartTime = g_system->getMillis();
	debugC(7, kDebugImages, "Window::render(): Updating %d rects", _dirtyRects.size());

	for (auto &i : _dirtyRects) {
		const Common::Rect &r = i;
		_dirtyChannels = _currentMovie->getScore()->getSpriteIntersections(r);

		bool shouldClear = true;
		Channel *trailChannel = nullptr;
		for (auto &j : _dirtyChannels) {
			if (j->_visible && r == j->getBbox() && j->isTrail()) {
				shouldClear = false;
				trailChannel = j;
				break;
			}
		}

		if (shouldClear) {
			blitTo->fillRect(r, _stageColor);
		} else if (trailChannel) {
			// Trail rendering mode; do not re-render the background and sprites underneath.
			_dirtyChannels.clear();
			_dirtyChannels.push_back(trailChannel);
		}

		for (int pass = 0; pass < 2; pass++) {
			for (auto &j : _dirtyChannels) {
				if (j->isActiveVideo() && j->isVideoDirectToStage()) {
					if (pass == 0)
						continue;
				} else {
					if (pass == 1)
						continue;
				}

				if (j->_visible) {
					if (j->hasSubChannels()) {
						Common::Array<Channel> *list = j->getSubChannels();
						for (auto &k : *list) {
							inkBlitFrom(&k, r, blitTo);
						}
					} else {
						inkBlitFrom(j, r, blitTo);
						if (j == hiliteChannel)
							invertChannel(hiliteChannel, r);
					}
				}
			}
		}
	}

	if (g_director->_debugDraw & kDebugDrawCast) {
		const Graphics::Font *font = FontMan.getFontByUsage(Graphics::FontManager::kConsoleFont);

		for (uint i = 0; i < _currentMovie->getScore()->_channels.size(); i++) {
			Channel *channel = _currentMovie->getScore()->_channels[i];
			if (!channel->isEmpty()) {
				Common::Rect bbox = channel->getBbox();
				blitTo->frameRect(bbox, g_director->_wm->_colorWhite);

				font->drawString(blitTo, Common::String::format("m: %d, ch: %d, fr: %d", channel->_sprite->_castId.member, i, channel->_filmLoopFrame ? channel->_filmLoopFrame : channel->_movieTime), bbox.left + 3, bbox.top + 3, 128, g_director->_wm->_colorBlack);
				font->drawString(blitTo, Common::String::format("m: %d, ch: %d, fr: %d", channel->_sprite->_castId.member, i, channel->_filmLoopFrame ? channel->_filmLoopFrame : channel->_movieTime), bbox.left + 2, bbox.top + 2, 128, g_director->_wm->_colorWhite);
			}
		}
	}

	if (g_director->_debugDraw & kDebugDrawFrame)
		drawFrameCounter(blitTo);

	_dirtyRects.clear();
	_contentIsDirty = true;
	debugC(7, kDebugImages, "Window::render(): Draw finished in %d ms",  g_system->getMillis() - renderStartTime);

	return true;
}

void Window::setStageColor(uint32 stageColor, bool forceReset) {
	if (stageColor != _stageColor || forceReset) {
		_stageColor = stageColor;
		reset();
		markAllDirty();
	}
}

void Window::setTitleVisible(bool titleVisible) {
	MacWindow::setTitleVisible(titleVisible);
	updateBorderType();
}

Datum Window::getStageRect() {
	ensureMovieIsLoaded();

	Common::Rect rect = getInnerDimensions();
	Datum d;
	d.type = RECT;
	d.u.farr = new FArray;
	d.u.farr->arr.push_back(rect.left);
	d.u.farr->arr.push_back(rect.top);
	d.u.farr->arr.push_back(rect.right);
	d.u.farr->arr.push_back(rect.bottom);

	return d;
}

bool Window::setStageRect(Datum datum) {
	if (datum.type != RECT) {
		warning("Window::setStageRect(): bad argument passed to rect field");
		return false;
	}

	// Unpack rect from datum
	Common::Rect rect = Common::Rect(datum.u.farr->arr[0].asInt(), datum.u.farr->arr[1].asInt(), datum.u.farr->arr[2].asInt(), datum.u.farr->arr[3].asInt());

	setInnerDimensions(rect);

	return true;
}

void Window::setModal(bool modal) {
	if (_isModal && !modal) {
		_wm->setLockedWidget(nullptr);
		_isModal = false;
	} else if (!_isModal && modal) {
		_wm->setLockedWidget(this);
		_isModal = true;
	}
}

void Window::setFileName(Common::String filename) {
	setNextMovie(filename);
	ensureMovieIsLoaded();
}

void Window::reset() {
	resizeInner(_composeSurface->w, _composeSurface->h);
	_contentIsDirty = true;
}

void Window::inkBlitFrom(Channel *channel, Common::Rect destRect, Graphics::ManagedSurface *blitTo) {
	Common::Rect srcRect = channel->getBbox();
	destRect.clip(srcRect);

	DirectorPlotData pd = channel->getPlotData();
	pd.destRect = destRect;
	pd.dst = blitTo;

	uint32 renderStartTime = 0;
	if (debugChannelSet(8, kDebugImages)) {
		CastType castType = channel->_sprite->_cast ? channel->_sprite->_cast->_type : kCastTypeNull;
		debugC(8, kDebugImages, "Window::inkBlitFrom(): updating %dx%d @ %d,%d -> %dx%d @ %d,%d, type: %s, cast: %s, ink: %d",
				srcRect.width(), srcRect.height(), srcRect.left, srcRect.top,
				destRect.width(), destRect.height(), destRect.left, destRect.top,
				castType2str(castType), channel->_sprite->_castId.asString().c_str(),
				channel->_sprite->_ink);
		renderStartTime = g_system->getMillis();
	}

	if (pd.ms) {
		pd.inkBlitShape(srcRect);
	} else if (pd.srf) {
		pd.inkBlitSurface(srcRect, channel->getMask());
	} else {
		if (debugChannelSet(4, kDebugImages)) {
			CastType castType = channel->_sprite->_cast ? channel->_sprite->_cast->_type : kCastTypeNull;
			warning("Window::inkBlitFrom(): No source surface: spriteType: %d (%s), castType: %d (%s), castId: %s",
				channel->_sprite->_spriteType, spriteType2str(channel->_sprite->_spriteType), castType, castType2str(castType),
				channel->_sprite->_castId.asString().c_str());
		}
	}

	if (debugChannelSet(8, kDebugImages)) {
		debugC(8, kDebugImages, "Window::inkBlitFrom(): Draw finished in %d ms",  g_system->getMillis() - renderStartTime);
	}
}

Common::Point Window::getMousePos() {
	return g_system->getEventManager()->getMousePos() - Common::Point(_innerDims.left, _innerDims.top);
}

void Window::setVisible(bool visible, bool silent) {
	// setting visible triggers movie load
	if (!_currentMovie && !silent)
		ensureMovieIsLoaded();

	BaseMacWindow::setVisible(visible);

	if (visible)
		_wm->setActiveWindow(_id);
}

void Window::ensureMovieIsLoaded() {
	if (!_currentMovie) {
		if (_fileName.empty()) {
			Common::String movieName = getName();
			setNextMovie(movieName);
		}
	} else if (_nextMovie.movie.empty()) { // The movie is loaded and no next movie to load
		return;
	}

	if (_nextMovie.movie.empty()) {
		warning("Window::ensureMovieIsLoaded(): No movie to load");
		return;
	}

	loadNextMovie();

	if (_currentMovie->getScore()->_playState == kPlayNotStarted)
		step(); // we will load it here and move to kPlayLoaded state
}

bool Window::setNextMovie(Common::String &movieFilenameRaw) {
	_fileName = findMoviePath(movieFilenameRaw);

	bool fileExists = false;
	Common::File file;
	if (!_fileName.empty() && file.open(_fileName)) {
		fileExists = true;
		file.close();
	}

	debug(1, "Window::setNextMovie: '%s' -> '%s' -> '%s'", movieFilenameRaw.c_str(), convertPath(movieFilenameRaw).c_str(), _fileName.toString(Common::Path::kNativeSeparator).c_str());

	if (!fileExists) {
		warning("Movie %s does not exist", _fileName.toString(Common::Path::kNativeSeparator).c_str());
		_fileName.clear();
		return false;
	}

	_nextMovie.movie = _fileName.toString(g_director->_dirSeparator);

	return true;
}

void Window::updateBorderType() {
	if (_isStage) {
		setBorderType(3);
	} else if (!isTitleVisible()) {
		setBorderType(2);
	} else {
		setBorderType(MAX(0, MIN(_windowType, 16)));
	}
}

void Window::loadNewSharedCast(Cast *previousSharedCast) {
	Common::Path previousSharedCastPath;
	Common::Path newSharedCastPath = getSharedCastPath();
	if (previousSharedCast && previousSharedCast->getArchive()) {
		previousSharedCastPath = previousSharedCast->getArchive()->getPathName();
	}

	// Check if previous and new sharedCasts are the same
	if (!previousSharedCastPath.empty() && previousSharedCastPath == newSharedCastPath) {
		// Clear those previous widget pointers
		previousSharedCast->releaseCastMemberWidget();
		_currentMovie->_sharedCast = previousSharedCast;

		debugC(1, kDebugLoading, "Skipping loading already loaded shared cast, path: %s", previousSharedCastPath.toString(Common::Path::kNativeSeparator).c_str());
		return;
	}

	// Clean up the previous sharedCast
	if (previousSharedCast) {
		debug(0, "@@   Clearing shared cast '%s'", previousSharedCastPath.toString().c_str());

		g_director->_allSeenResFiles.erase(previousSharedCastPath);
		g_director->_allOpenResFiles.remove(previousSharedCastPath);
		delete previousSharedCast->_castArchive;
		delete previousSharedCast;
	} else {
		debug(0, "@@   No previous shared cast");
	}

	// Load the new sharedCast
	if (!newSharedCastPath.empty()) {
		_currentMovie->loadSharedCastsFrom(newSharedCastPath);
	}
}

bool Window::loadNextMovie() {
	_soundManager->changingMovie();
	_newMovieStarted = true;
	_currentPath = Common::firstPathComponents(_nextMovie.movie, g_director->_dirSeparator);

	Cast *previousSharedCast = nullptr;
	if (_currentMovie) {
		previousSharedCast = _currentMovie->getSharedCast();
		_currentMovie->_sharedCast = nullptr;
	}

	delete _currentMovie;
	_currentMovie = nullptr;

	Common::Path archivePath = Common::Path(_currentPath, g_director->_dirSeparator);
	archivePath.appendInPlace(Common::lastPathComponent(_nextMovie.movie, g_director->_dirSeparator));
	Archive *mov = g_director->openArchive(archivePath);

	_nextMovie.movie.clear(); // Clearing it, so we will not attempt to load again

	if (!mov)
		return false;

	probeResources(mov);

	// Artificial delay for games that expect slow media, e.g. Spaceship Warlock
	if (g_director->_loadSlowdownFactor && !debugChannelSet(-1, kDebugFast)) {
		// Check that we're not cooling down from skipping a delay.
		if (g_system->getMillis() > g_director->_loadSlowdownCooldownTime) {
			uint32 delay = mov->getFileSize() * 1000 / g_director->_loadSlowdownFactor;
			debugC(5, kDebugLoading, "Slowing load of next movie by %d ms", delay);
			while (delay != 0) {
				uint32 dec = MIN((uint32)10, delay);
				// Skip delay if mouse is clicked
				if (g_director->processEvents(true, true)) {
					g_director->loadSlowdownCooloff();
					break;
				}
				g_director->_wm->replaceCursor(Graphics::kMacCursorWatch);
				g_director->draw();
				g_system->delayMillis(dec);
				delay -= dec;
			}
		}
		// If this movie switch is within the cooldown time,
		// don't add a delay. This is to allow for rapid navigation.
		// User input events will call loadSlowdownCooloff() and
		// extend the cooldown time.
	}

	_currentMovie = new Movie(this);
	_currentMovie->setArchive(mov);

	debug(0, "\n@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@");
	debug(0, "@@@@   Switching to movie '%s' in '%s'", utf8ToPrintable(_currentMovie->getMacName()).c_str(), _currentPath.c_str());
	debug(0, "@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@\n");

	g_director->setCurrentWindow(this);
	loadNewSharedCast(previousSharedCast);

	return true;
}

bool Window::step() {
	// finish last movie
	if (_currentMovie && _currentMovie->getScore()->_playState == kPlayStopped) {
		debugC(5, kDebugEvents, "\n@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@");
		debugC(5, kDebugEvents, "@@@@   Finishing movie '%s' in '%s'", utf8ToPrintable(_currentMovie->getMacName()).c_str(), _currentPath.c_str());
		debugC(5, kDebugEvents, "@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@\n");

		_currentMovie->getScore()->stopPlay();
		debugC(1, kDebugEvents, "Finished playback of movie '%s'", utf8ToPrintable(_currentMovie->getMacName()).c_str());

		if (_vm->getGameGID() == GID_TESTALL) {
			_nextMovie = getNextMovieFromQueue();
		}
	}

	if (debugChannelSet(-1, kDebugFewFramesOnly) && g_director->_framesRan > kFewFamesMaxCounter)
		return false;

	// prepare next movie
	if (!_nextMovie.movie.empty()) {
		if (!loadNextMovie())
			return (_vm->getGameGID() == GID_TESTALL);

		g_lingo->resetLingo();
		g_director->_lastPalette = CastMemberID();
	}

	// play current movie
	if (_currentMovie) {
		switch (_currentMovie->getScore()->_playState) {
		case kPlayNotStarted:
			{
				debug(0, "\n@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@");
				debug(0, "@@@@   Loading movie '%s' in '%s'", utf8ToPrintable(_currentMovie->getMacName()).c_str(), _currentPath.c_str());
				debug(0, "@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@\n");

				bool goodMovie = _currentMovie->loadArchive();

				// If we came in a loop, then skip as requested
				if (!_nextMovie.frameS.empty()) {
					_currentMovie->getScore()->setStartToLabel(_nextMovie.frameS);
					_nextMovie.frameS.clear();
				}

				if (_nextMovie.frameI != -1) {
					_currentMovie->getScore()->setCurrentFrame(_nextMovie.frameI);
					_nextMovie.frameI = -1;
				}

				if (debugChannelSet(-1, kDebugPauseOnLoad) ||
						(g_director->_firstMovie && debugChannelSet(-1, kDebugPaused))) {
					_currentMovie->getScore()->_playState = kPlayPausedAfterLoading;
					debug(0, "Window::step(): Putting score in paused state as requested");
					g_system->displayMessageOnOSD(Common::U32String("Paused"));

					g_director->_firstMovie = false;
					return true;
				}

				if (!goodMovie)
					return false;

				_currentMovie->getScore()->_playState = kPlayLoaded;

				return true;
			}

		case kPlayLoaded:
			if (!debugChannelSet(-1, kDebugCompileOnly)) {
				debugC(1, kDebugEvents, "Starting playback of movie '%s'", _currentMovie->getMacName().c_str());
				_currentMovie->getScore()->startPlay();
				if (_startFrame != -1) {
					_currentMovie->getScore()->setCurrentFrame(_startFrame);
					_startFrame = -1;
				}
				g_debugger->movieHook();
			} else {
				return false;
			}
			// fall through
		case kPlayStarted:
			debugC(5, kDebugEvents, "\n@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@");
			debugC(5, kDebugEvents, "@@@@   Stepping movie '%s' in '%s'", utf8ToPrintable(_currentMovie->getMacName()).c_str(), _currentPath.c_str());
			debugC(5, kDebugEvents, "@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@\n");
			_currentMovie->getScore()->step();
			return true;
		case kPlayPausedAfterLoading:
		case kPlayPaused:
			return true;
		default:
			return false;
		}
	}

	return false;
}

Common::Path Window::getSharedCastPath() {
	Common::Array<Common::String> namesToTry;
	if (_vm->getVersion() < 400) {
		if (g_director->getPlatform() == Common::kPlatformWindows) {
			namesToTry.push_back("SHARDCST.MMM");
		} else {
			namesToTry.push_back("Shared Cast");
		}
	} else if (_vm->getVersion() < 500) {
		namesToTry.push_back("Shared.dir");
	} else {
		// TODO: Does D5 actually support D4-style shared cast?
		namesToTry.push_back("Shared.cst");
	}

	Common::Path result;
	for (uint i = 0; i < namesToTry.size(); i++) {
		result = findMoviePath(namesToTry[i]);
		if (!result.empty())
			return result;
	}

	return result;
}

void Window::freezeLingoState() {
	_frozenLingoStates.push_back(_lingoState);
	_lingoState = new LingoState;
	debugC(3, kDebugLingoExec, "Freezing Lingo state, depth %d", _frozenLingoStates.size());
}

void Window::thawLingoState() {
	if (_frozenLingoStates.empty()) {
		warning("Tried to thaw when there's no frozen state, ignoring");
		return;
	}
	if (!_lingoState->callstack.empty()) {
		warning("Can't thaw a Lingo state in mid-execution, ignoring");
		return;
	}
	delete _lingoState;
	debugC(3, kDebugLingoExec, "Thawing Lingo state, depth %d", _frozenLingoStates.size());
	_lingoState = _frozenLingoStates.back();
	_frozenLingoStates.pop_back();
}

void Window::freezeLingoPlayState() {
	if (_lingoPlayState) {
		warning("FIXME: Just clobbered the play state");
		delete _lingoPlayState;
	}
	_lingoPlayState = _lingoState;
	_lingoState = new LingoState;
	debugC(3, kDebugLingoExec, "Freezing Lingo play state");
}

bool Window::thawLingoPlayState() {
	if (!_lingoPlayState) {
		warning("Tried to thaw when there's no frozen play state, ignoring");
		return false;
	}
	if (!_lingoState->callstack.empty()) {
		warning("Can't thaw a Lingo state in mid-execution, ignoring");
		return false;
	}
	delete _lingoState;
	debugC(3, kDebugLingoExec, "Thawing Lingo play state");
	_lingoState = _lingoPlayState;
	_lingoPlayState = nullptr;
	return true;
}


// Check how many times enterFrame/stepMovie have been called recursively.
// When Lingo encounters a go() call, it freezes the execution state and starts
// processing the next frame. In the case of enterFrame/stepMovie, it is possible
// to keep recursing without reaching a point where the frozen contexts are finished.
// D4 and higher will only process recursive handlers to a depth of 2.
// e.g. in a movie:
// - frame 1 has an enterFrame handler that calls go(2)
// - frame 2 has an enterFrame handler that calls go(3)
// - frame 3 has an enterFrame handler that calls go(4)
// The third enterFrame handler will be eaten and not called.
// We can count the number of frozen states which started from enterFrame/stepMovie.
uint32 Window::frozenLingoRecursionCount() {
	uint32 count = 0;

	for (int i = (int)_frozenLingoStates.size() - 1; i >= 0; i--) {
		LingoState *state = _frozenLingoStates[i];
		CFrame *frame = state->callstack.front();
		if (frame->sp.name->equalsIgnoreCase("enterFrame") ||
				frame->sp.name->equalsIgnoreCase("stepMovie")) {
			count++;
		} else {
			break;
		}
	}

	return count;
}

} // End of namespace Director
