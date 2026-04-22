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
#include "director/castmember/text.h"
#include "director/debugger/debugtools.h"
#include "graphics/managed_surface.h"

namespace Director {

bool commandsWindowCallback(Graphics::WindowClick click, Common::Event &event, void *window) {
	Window *w = (Window*)window;
	return w->processWMEvent(click, event);
}

Window::Window(int id, bool scrollable, bool resizable, bool editable, Graphics::MacWindowManager *wm, DirectorEngine *vm, bool isStage)
: Object<Window>("Window") {
	_vm = vm;
	_wm = wm;
	_isStage = isStage;
	_stageColor = _wm->_colorBlack;
	_puppetTransition = nullptr;
	_soundManager = new DirectorSound(this);
	_lingoState = new LingoState;

	_currentMovie = nullptr;
	_nextMovie.frameI = -1;
	_newMovieStarted = true;

	_objType = kWindowObj;
	_startFrame = _vm->getStartMovie().startFrame;

	_windowType = -1;
	_isModal = false;
	_skipFrameAdvance = false;

	// Owned by the window manager
	_window = new Graphics::MacWindow(id, scrollable, resizable, editable, wm);
	_window->setDraggable(!_isStage);

	_window->setCallback(commandsWindowCallback, this);

	updateBorderType();
}

Window::~Window() {
	delete _lingoState;
	for (uint i = 0; i < _lingoPlayStates.size(); i++)
		delete _lingoPlayStates[i];
	delete _soundManager;
	delete _currentMovie;
	for (uint i = 0; i < _frozenLingoStates.size(); i++)
		delete _frozenLingoStates[i];
	if (_puppetTransition)
		delete _puppetTransition;
	g_director->_wm->removeWindow(_window);
	g_director->_wm->removeMarked();
	_window = nullptr;
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

	Graphics::ManagedSurface *composeSurface = _window->getSurface();

	if (_wm->_pixelformat.bytesPerPixel == 1) {
		for (int i = 0; i < srcRect.height(); i++) {
			byte *src = (byte *)composeSurface->getBasePtr(srcRect.left, srcRect.top + i);
			const byte *msk = mask ? (const byte *)mask->getBasePtr(xoff, yoff + i) : nullptr;

			for (int j = 0; j < srcRect.width(); j++, src++)
				if (!mask || (msk && (*msk++)))
					*src = _wm->inverter(*src);
		}
	} else {

		for (int i = 0; i < srcRect.height(); i++) {
			uint32 *src = (uint32 *)composeSurface->getBasePtr(srcRect.left, srcRect.top + i);
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

void Window::drawChannelBox(Director::Movie *currentMovie, Graphics::ManagedSurface *blitTo, int selectedChannel) {
	const Graphics::Font *font = FontMan.getFontByUsage(Graphics::FontManager::kConsoleFont);
	Channel *channel = currentMovie->getScore()->_channels[selectedChannel];

	if (!channel->isEmpty()) {
		Common::Rect bbox = channel->getBbox();
		blitTo->frameRect(bbox, g_director->_wm->_colorWhite);

		font->drawString(blitTo, Common::String::format("m: %d, ch: %d, fr: %d", channel->_sprite->_castId.member, selectedChannel, channel->_filmLoopFrame ? channel->_filmLoopFrame : channel->_movieTime), bbox.left + 3, bbox.top + 3, 128, g_director->_wm->_colorBlack);
		font->drawString(blitTo, Common::String::format("m: %d, ch: %d, fr: %d", channel->_sprite->_castId.member, selectedChannel, channel->_filmLoopFrame ? channel->_filmLoopFrame : channel->_movieTime), bbox.left + 2, bbox.top + 2, 128, g_director->_wm->_colorWhite);
	}
}

bool Window::render(bool forceRedraw, Graphics::ManagedSurface *blitTo) {
	if (!_currentMovie)
		return false;

	if (!blitTo)
		blitTo = _window->getSurface();

	Common::List<Common::Rect> &dirtyRects = _window->getDirtyRectList();

	if (forceRedraw) {
		blitTo->clear(_stageColor);
		_window->markAllDirty();
	} else {
		if (dirtyRects.size() == 0 && _currentMovie->_videoPlayback == false) {
			if (g_director->_debugDraw & kDebugDrawFrame) {
				drawFrameCounter(blitTo);

				_window->setContentDirty(true);
			}

			return false;
		}

		_window->mergeDirtyRects();
	}

	Channel *hiliteChannel = _currentMovie->getScore()->getChannelById(_currentMovie->_currentHiliteChannelId);

	uint32 renderStartTime = g_system->getMillis();
	debugC(7, kDebugImages, "Window::render(): Updating %d rects", dirtyRects.size());

	for (auto &i : dirtyRects) {
		Common::Rect r = i;
		// The inner dimensions are relative to the virtual desktop while
		// r isn't, so we need to move the window to be relative to the
		// same sapce.
		Common::Rect windowRect = _window->getInnerDimensions();
		windowRect.moveTo(r.left, r.top);
		r.clip(windowRect);

		_dirtyChannels = _currentMovie->getScore()->getSpriteIntersections(r);

		bool shouldClear = true;
		Channel *trailChannel = nullptr;
		for (auto &j : _dirtyChannels) {
			bool isHidden = false;
			isHidden = j->_hideFromStage;
			if (j->_visible && !isHidden && r == j->getBbox() && j->isTrail()) {
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

				if (j->_hideFromStage)
					continue;

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

#ifdef USE_IMGUI
	int selectedChannel = DT::getSelectedChannel();
	if (selectedChannel > 0)
		Window::drawChannelBox(_currentMovie, blitTo, selectedChannel);
#endif

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

	dirtyRects.clear();
	_window->setContentDirty(true);
	debugC(7, kDebugImages, "Window::render(): Draw finished in %d ms",  g_system->getMillis() - renderStartTime);

	return true;
}

void Window::setStageColor(uint32 stageColor, bool forceReset) {
	if (stageColor != _stageColor || forceReset) {
		_stageColor = stageColor;
		reset();
		_window->markAllDirty();
	}
}

void Window::setTitleVisible(bool titleVisible) {
	_window->setTitleVisible(titleVisible);
	updateBorderType();
}

Graphics::ManagedSurface *Window::getSurface() {
	return _window->getSurface();
}

void Window::addDirtyRect(const Common::Rect &r) {
	_window->addDirtyRect(r);
}

void Window::resizeInner(int w, int h) {
	_window->resizeInner(w, h);
}

int Window::getId() {
	return _window->getId();
}

void Window::setDirty(bool dirty) {
	_window->setDirty(dirty);
}

void Window::disableBorder() {
	_window->disableBorder();
}

void Window::center(bool toCenter) {
	_window->center(toCenter);
}

Common::Point Window::getAbsolutePos() {
	return _window->getAbsolutePos();
}

void Window::setTitle(const Common::String &title) {
	_window->setTitle(title);
}

void Window::move(int x, int y) {
	_window->move(x, y);
}

Datum Window::getStageRect() {
	ensureMovieIsLoaded();

	Common::Rect rect = _window->getInnerDimensions();
	Datum d;
	d.type = RECT;
	d.u.farr = new FArray;
	d.u.farr->arr.push_back(rect.left);
	d.u.farr->arr.push_back(rect.top);
	d.u.farr->arr.push_back(rect.right);
	d.u.farr->arr.push_back(rect.bottom);

	return d;
}

void Window::setStageRect(Datum datum) {
	if (datum.type != RECT) {
		warning("Window::setStageRect(): bad argument passed to rect field");
		return;
	}

	// Unpack rect from datum
	Common::Rect rect = Common::Rect(datum.u.farr->arr[0].asInt(), datum.u.farr->arr[1].asInt(), datum.u.farr->arr[2].asInt(), datum.u.farr->arr[3].asInt());

	_window->setInnerDimensions(rect);
}

void Window::setModal(bool modal) {
	if (_isModal && !modal) {
		_wm->setLockedWidget(nullptr);
		_isModal = false;
	} else if (!_isModal && modal) {
		_wm->setLockedWidget(this->_window);
		_isModal = true;
	}
}

void Window::setFileName(Common::String filename) {
	setNextMovie(filename);
	ensureMovieIsLoaded();
}

void Window::reset() {
	Graphics::ManagedSurface *composeSurface = _window->getSurface();
	resizeInner(composeSurface->w, composeSurface->h);
	_window->setContentDirty(true);
}

void Window::inkBlitFrom(Channel *channel, Common::Rect destRect, Graphics::ManagedSurface *blitTo) {
	Common::Rect srcRect = channel->getBbox();
	destRect.clip(srcRect);

	DirectorPlotData pd = channel->getPlotData();
	pd.destRect = destRect;
	pd.dst = blitTo;

	CastType castType = channel->_sprite->_cast ? channel->_sprite->_cast->_type : kCastTypeNull;

	uint32 renderStartTime = 0;
	if (debugChannelSet(8, kDebugImages)) {
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
	if (Director::DT::isMouseInputIgnored() && _currentMovie) {
		return _currentMovie->_lastMousePos;
	}

	Common::Rect innerDims = _window->getInnerDimensions();
	return g_system->getEventManager()->getMousePos() - Common::Point(innerDims.left, innerDims.top);
}

void Window::setVisible(bool visible, bool silent) {
	// setting visible triggers movie load
	if (!_currentMovie && !silent)
		ensureMovieIsLoaded();

	_window->setVisible(visible);

	if (visible)
		_wm->setActiveWindow(getId());
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

	if (_currentMovie) {
		if (!_lingoState->callstack.empty())
			freezeLingoState();
		_currentMovie->getScore()->stopPlay();
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

	warning("Window::setNextMovie: '%s' -> '%s' -> '%s'", movieFilenameRaw.c_str(), convertPath(movieFilenameRaw).c_str(), _fileName.toString(Common::Path::kNativeSeparator).c_str());

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
		_window->setBorderType(3);
	} else if (!_window->isTitleVisible()) {
		_window->setBorderType(2);
	} else {
		_window->setBorderType(MAX(0, MIN(_windowType, 16)));
	}
}

void Window::loadNewSharedCast(Cast *previousSharedCast) {
	if (g_director->getVersion() >= 500)
		return;

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


		// Persist text cast member values from external castlibs before this movie
		// is destroyed.  In real Director these castlibs stay in memory across play/go
		// transitions so script-visible field values survive intact.  ScummVM re-loads
		// them from disk each time, so we snapshot the current text here and re-apply
		// it after the next movie's casts have loaded.
		Archive *mainArchive = _currentMovie->getArchive();
		warning("loadNextMovie: snapshot from movie '%s' -> '%s'", mainArchive->getFileName().c_str(), _nextMovie.movie.c_str());
		for (auto &castEntry : *(_currentMovie->getCasts())) {
			Cast *cast = castEntry._value;
			if (!cast)
				continue;
			Archive *castArchive = cast->getArchive();
			if (!castArchive || castArchive == mainArchive)
				continue;
			Common::String castFilename = castArchive->getFileName();
			if (castFilename.empty())
				continue;
			Common::String key = castFilename;
			key.toUppercase();
			for (auto &member : *(cast->_loadedCast)) {
				if (!member._value)
					continue;
				if (member._value->_type == kCastText) {
					TextCastMember *tm = static_cast<TextCastMember *>(member._value);
					if (!tm->isLoaded())
						tm->load();
					if (!tm->_ptext.empty()) {
						g_director->_savedCastText[key][member._key] = tm->_ptext;
						if (key == "MASTER.CST")
							warning("MASTER.CST snapshot: member %d ptext='%s'", member._key, tm->_ptext.encode().c_str());
					}
				}
			}
		}
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

	if (g_director->getVersion() < 500)
		loadNewSharedCast(previousSharedCast);

	return true;
}

bool Window::step() {
	// finish last movie
	if (_currentMovie && _currentMovie->getScore()->_playState == kPlayStopped) {
		// attempt to thaw the lingo play state, if required
		// For movie switches, we want to run it in the context of the new movie.
		if (_nextMovie.movie.empty())
			_currentMovie->getScore()->processFrozenPlayScript();
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

				// Re-apply text values snapshotted from external castlibs before the
				// previous movie was destroyed.  This preserves script-visible field
				// values (money, time, etc.) that real Director keeps live in memory.
				warning("loadNextMovie apply: in movie '%s'", _currentMovie->getArchive()->getFileName().c_str());
				warning("savedCastText restore: entering movie '%s' with %d saved cast(s)",
					_currentMovie->getArchive()->getFileName().c_str(), g_director->_savedCastText.size());
				if (!g_director->_savedCastText.empty()) {
					Archive *newMainArchive = _currentMovie->getArchive();
					Common::Array<Common::String> appliedKeys;
					for (auto &castEntry : *(_currentMovie->getCasts())) {
						Cast *cast = castEntry._value;
						if (!cast)
							continue;
						Archive *castArchive = cast->getArchive();
						if (!castArchive || castArchive == newMainArchive)
							continue;
						Common::String castFilename = castArchive->getFileName();
						if (castFilename.empty())
							continue;
						Common::String key = castFilename;
						key.toUppercase();
						auto savedIt = g_director->_savedCastText.find(key);
						if (savedIt == g_director->_savedCastText.end()) {
							warning("savedCastText restore: cast '%s' present in movie but NOT in savedCastText — skipping", castFilename.c_str());
							continue;
						}
						for (auto &savedMember : savedIt->_value) {
							auto memberIt = cast->_loadedCast->find(savedMember._key);
							if (memberIt == cast->_loadedCast->end())
								continue;
							CastMember *cm = memberIt->_value;
							if (!cm || cm->_type != kCastText)
								continue;
							TextCastMember *tm = static_cast<TextCastMember *>(cm);
							// load() sets _loaded=true but overwrites _ptext from archive.
							// Restore our saved value and rebuild _ftext so the widget
							// renders the correct text.
							tm->load();
							if (key == "MASTER.CST")
								warning("MASTER.CST apply: restoring member %d disk='%s' -> saved='%s' [tm=%p cast=%p]",
									savedMember._key, tm->_ptext.encode().c_str(), savedMember._value.encode().c_str(),
									(void *)tm, (void *)cast);
							tm->_ptext = savedMember._value;
							tm->_rtext = savedMember._value.encode(Common::kUtf8);
							Common::U32String fmt = Common::String::format("\001\016%04x%02x%04x%04x%04x%04x",
								tm->_fontId, tm->_textSlant, tm->_fontSize,
								tm->_fgpalinfo1, tm->_fgpalinfo2, tm->_fgpalinfo3);
							tm->_ftext = fmt + tm->_ptext;
							tm->setModified(true);
							// Remember this value so we can re-apply it after startMovie
							// handlers run.  startMovie scripts (e.g. "init clock") may
							// unconditionally reset fields we just restored.
							_postStartMovieRestore[key][savedMember._key] = savedMember._value;
						}
						appliedKeys.push_back(key);
					}
					// Only remove entries that were actually applied to this movie's casts.
					// Unmatched entries (e.g. MASTER.CST when a transitional movie lacks it)
					// are preserved so the next movie in the chain can receive them.
					for (const auto &key : appliedKeys)
						g_director->_savedCastText.erase(key);
					if (!g_director->_savedCastText.empty()) {
						for (auto &remaining : g_director->_savedCastText)
							warning("savedCastText restore: '%s' still has %d entry/entries (not in this movie — will survive to next)",
								remaining._key.c_str(), remaining._value.size());
					}
				}

				// If we've just started, switch to the default palette
				if (g_director->_firstMovie)
					g_director->setPalette(_currentMovie->getCast()->_defaultPalette);

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
				g_director->_firstMovie = false;

				if (!goodMovie)
					return false;

				_currentMovie->getScore()->_playState = kPlayLoaded;

				return true;
			}

		case kPlayLoaded:
			warning("MASTER.CST kPlayLoaded: movie='%s' postRestore=%d savedCastText=%d",
				_currentMovie->getMacName().c_str(),
				(int)_postStartMovieRestore.size(),
				(int)g_director->_savedCastText.size());
			if (!debugChannelSet(-1, kDebugCompileOnly)) {
				debugC(1, kDebugEvents, "Starting playback of movie '%s'", _currentMovie->getMacName().c_str());

				if (_vm->getVersion() >= 600) {
					// We need to call this before behavior scripts are instantiated
					// or cast loaded
					_currentMovie->getScore()->_disableGoPlayUpdateStage = true;
					_currentMovie->processEvent(kEventPrepareMovie);
					_currentMovie->getScore()->_disableGoPlayUpdateStage = false;
				}

				_currentMovie->getScore()->startPlay();
				if (_startFrame != -1) {
					_currentMovie->getScore()->setCurrentFrame(_startFrame);
					_startFrame = -1;
				}

				// Re-apply any external cast text values that were snapshotted
				// before this movie loaded.  startMovie handlers (e.g. "init clock")
				// may have reset fields we already restored in kPlayNotStarted.
				// We also push the values back into savedCastText so the next child
				// window (e.g. MAINMENU as a dialog) inherits the correct value.
				if (_isStage && !_postStartMovieRestore.empty()) {
					warning("MASTER.CST post-startMovie: _postStartMovieRestore has %d entries", (int)_postStartMovieRestore.size());
					Archive *newMainArchive = _currentMovie->getArchive();
					for (auto &castEntry : *(_currentMovie->getCasts())) {
						Cast *cast = castEntry._value;
						if (!cast) continue;
						Archive *castArchive = cast->getArchive();
						if (!castArchive || castArchive == newMainArchive) continue;
						Common::String castFilename = castArchive->getFileName();
						if (castFilename.empty()) continue;
						Common::String key = castFilename;
						key.toUppercase();
						auto restoreIt = _postStartMovieRestore.find(key);
						if (restoreIt == _postStartMovieRestore.end()) {
							warning("MASTER.CST post-startMovie: key '%s' not found in _postStartMovieRestore", key.c_str());
							continue;
						}
						for (auto &entry : restoreIt->_value) {
							auto memberIt = cast->_loadedCast->find(entry._key);
							if (memberIt == cast->_loadedCast->end()) {
								warning("MASTER.CST post-startMovie: member %d not found in cast [cast=%p]", entry._key, (void *)cast);
								continue;
							}
							CastMember *cm = memberIt->_value;
							if (!cm || cm->_type != kCastText) {
								warning("MASTER.CST post-startMovie: member %d is not text [cm=%p type=%d]", entry._key, (void *)cm, cm ? (int)cm->_type : -1);
								continue;
							}
							TextCastMember *tm = static_cast<TextCastMember *>(cm);
							warning("MASTER.CST post-startMovie: re-applying member %d '%s' -> '%s' [tm=%p]",
								entry._key, tm->_ptext.encode().c_str(), entry._value.encode().c_str(), (void *)tm);
							tm->_ptext = entry._value;
							tm->_rtext = entry._value.encode(Common::kUtf8);
							Common::U32String fmt = Common::String::format("\001\016%04x%02x%04x%04x%04x%04x",
								tm->_fontId, tm->_textSlant, tm->_fontSize,
								tm->_fgpalinfo1, tm->_fgpalinfo2, tm->_fgpalinfo3);
							tm->_ftext = fmt + tm->_ptext;
							tm->setModified(true);
							// Also refresh savedCastText so any child window that opens
							// next (e.g. MAINMENU) inherits the correct value.
							g_director->_savedCastText[key][entry._key] = entry._value;
						}
					}
					// Do NOT clear _postStartMovieRestore here: frame-1 scripts (e.g.
					// an init-clock prepareFrame/enterFrame handler) fire during
					// Score::step() below and can overwrite values we just restored.
					// We re-apply and clear in kPlayStarted after that first step.
				} else {
					// Not the stage, or nothing to restore: discard so kPlayStarted
					// doesn't pick it up and override child-window script updates.
					_postStartMovieRestore.clear();
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
			// Re-apply saved external cast values that may have been overwritten by
			// frame-1 scripts (e.g. an init-clock handler that fires on the first
			// prepareFrame/enterFrame rather than on startMovie).  This is a second
			// pass identical to the kPlayLoaded post-startMovie block above; together
			// the two passes bracket startMovie writes AND first-frame writes.
			if (_isStage && !_postStartMovieRestore.empty()) {
				warning("MASTER.CST post-firstFrame: re-applying %d entry/entries", (int)_postStartMovieRestore.size());
				Archive *newMainArchive = _currentMovie->getArchive();
				for (auto &castEntry : *(_currentMovie->getCasts())) {
					Cast *cast = castEntry._value;
					if (!cast) continue;
					Archive *castArchive = cast->getArchive();
					if (!castArchive || castArchive == newMainArchive) continue;
					Common::String castFilename = castArchive->getFileName();
					if (castFilename.empty()) continue;
					Common::String key = castFilename;
					key.toUppercase();
					auto restoreIt = _postStartMovieRestore.find(key);
					if (restoreIt == _postStartMovieRestore.end()) continue;
					for (auto &entry : restoreIt->_value) {
						auto memberIt = cast->_loadedCast->find(entry._key);
						if (memberIt == cast->_loadedCast->end()) continue;
						CastMember *cm = memberIt->_value;
						if (!cm || cm->_type != kCastText) continue;
						TextCastMember *tm = static_cast<TextCastMember *>(cm);
						if (key == "MASTER.CST")
							warning("MASTER.CST post-firstFrame: member %d '%s' -> '%s' [tm=%p]",
								entry._key, tm->_ptext.encode().c_str(), entry._value.encode().c_str(), (void *)tm);
						tm->_ptext = entry._value;
						tm->_rtext = entry._value.encode(Common::kUtf8);
						Common::U32String fmt = Common::String::format("\001\016%04x%02x%04x%04x%04x%04x",
							tm->_fontId, tm->_textSlant, tm->_fontSize,
							tm->_fgpalinfo1, tm->_fgpalinfo2, tm->_fgpalinfo3);
						tm->_ftext = fmt + tm->_ptext;
						tm->setModified(true);
						g_director->_savedCastText[key][entry._key] = entry._value;
					}
				}
				_postStartMovieRestore.clear();
			}
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
		if (!_sharedCastFilenameHint.empty()) {
			namesToTry.push_back(_sharedCastFilenameHint);
		} else if (g_director->getPlatform() == Common::kPlatformWindows) {
			namesToTry.push_back("SHARDCST.MMM");
		} else {
			namesToTry.push_back("Shared Cast");
		}
	} else if (_vm->getVersion() < 500) {
		namesToTry.push_back("Shared.dir");
		if (!_sharedCastFilenameHint.empty()) {
			namesToTry.push_back(_sharedCastFilenameHint);
		}
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
	_lingoPlayStates.push_back(_lingoState);
	_lingoState = new LingoState;
	debugC(3, kDebugLingoExec, "Freezing Lingo play state, depth %d", _lingoPlayStates.size());
}

bool Window::thawLingoPlayState() {
	if (_lingoPlayStates.empty()) {
		warning("Tried to thaw when there's no frozen play state, ignoring");
		return false;
	}
	if (!_lingoState->callstack.empty()) {
		warning("Can't thaw a Lingo state in mid-execution, ignoring");
		return false;
	}
	delete _lingoState;
	debugC(3, kDebugLingoExec, "Thawing Lingo play state, depth %d", _lingoPlayStates.size());
	_lingoState = _lingoPlayStates.back();
	_lingoPlayStates.pop_back();
	return true;
}


void Window::moveLingoState(Window *target) {
	if (target == this)
		return;
	if (!target->_lingoState->callstack.empty())
		target->freezeLingoState();
	delete target->_lingoState;
	target->_lingoState = _lingoState;
	_lingoState = new LingoState();
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

	bool stepFrameCanRecurse = _vm->getVersion() < 500;

	for (int i = (int)_frozenLingoStates.size() - 1; i >= 0; i--) {
		LingoState *state = _frozenLingoStates[i];
		if (state->callstack.empty())
			continue;
		CFrame *frame = state->callstack.front();
		if (frame->sp.name->equalsIgnoreCase("enterFrame") ||
				frame->sp.name->equalsIgnoreCase("stepMovie") ||
				(!stepFrameCanRecurse && frame->sp.name->equalsIgnoreCase("stepFrame"))) {
			count++;
		} else {
			break;
		}
	}

	return count;
}

Common::String Window::formatWindowInfo() {
	Common::Rect dims = _window->getDimensions();
	Common::Rect innerDims = _window->getInnerDimensions();
	return Common::String::format(
			"name: \"%s\", movie: \"%s\", currentPath: \"%s\", dims: (%d,%d) %dx%d, innerDims: (%d, %d) %dx%d, visible: %d",
			_name.c_str(), _currentMovie->getMacName().c_str(), _currentPath.c_str(),
			dims.left, dims.top, dims.width(), dims.height(),
			innerDims.left, innerDims.top, innerDims.width(), innerDims.height(),
			_window->isVisible()
	);
}

} // End of namespace Director
