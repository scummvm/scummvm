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

#include "common/file.h"
#include "common/system.h"
#include "common/macresman.h"

#include "graphics/primitives.h"
#include "graphics/macgui/macwindowmanager.h"

#include "director/director.h"
#include "director/archive.h"
#include "director/cast.h"
#include "director/lingo/lingo.h"
#include "director/movie.h"
#include "director/window.h"
#include "director/score.h"
#include "director/cursor.h"
#include "director/channel.h"
#include "director/sound.h"
#include "director/sprite.h"
#include "director/util.h"
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

	_currentMovie = nullptr;
	_mainArchive = nullptr;
	_nextMovie.frameI = -1;
	_newMovieStarted = true;

	_objType = kWindowObj;
	_startFrame = _vm->getStartMovie().startFrame;

	_windowType = -1;
	_titleVisible = true;
	updateBorderType();
}

Window::~Window() {
	delete _lingoState;
	delete _soundManager;
	delete _currentMovie;
	for (uint i = 0; i < _frozenLingoStates.size(); i++)
		delete _frozenLingoStates[i];
	if (_puppetTransition)
		delete _puppetTransition;
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
				if (!mask || (msk && !(*msk++)))
					*src = _wm->inverter(*src);
		}
	} else {

		for (int i = 0; i < srcRect.height(); i++) {
			uint32 *src = (uint32 *)_composeSurface->getBasePtr(srcRect.left, srcRect.top + i);
			const uint32 *msk = mask ? (const uint32 *)mask->getBasePtr(xoff, yoff + i) : nullptr;

			for (int j = 0; j < srcRect.width(); j++, src++)
				if (!mask || (msk && !(*msk++)))
					*src = _wm->inverter(*src);
		}
	}
}

bool Window::render(bool forceRedraw, Graphics::ManagedSurface *blitTo) {
	if (!_currentMovie)
		return false;

	if (forceRedraw) {
		blitTo->clear(_stageColor);
		markAllDirty();
	} else {
		if (_dirtyRects.size() == 0 && _currentMovie->_videoPlayback == false)
			return false;

		mergeDirtyRects();
	}

	if (!blitTo)
		blitTo = _composeSurface;
	Channel *hiliteChannel = _currentMovie->getScore()->getChannelById(_currentMovie->_currentHiliteChannelId);

	for (Common::List<Common::Rect>::iterator i = _dirtyRects.begin(); i != _dirtyRects.end(); i++) {
		const Common::Rect &r = *i;
		_dirtyChannels = _currentMovie->getScore()->getSpriteIntersections(r);

		bool shouldClear = true;
		for (Common::List<Channel *>::iterator j = _dirtyChannels.begin(); j != _dirtyChannels.end(); j++) {
			if ((*j)->_visible && r == (*j)->getBbox() && (*j)->isTrail()) {
				shouldClear = false;
				break;
			}
		}

		if (shouldClear)
			blitTo->fillRect(r, _stageColor);

		for (int pass = 0; pass < 2; pass++) {
			for (Common::List<Channel *>::iterator j = _dirtyChannels.begin(); j != _dirtyChannels.end(); j++) {
				if ((*j)->isActiveVideo() && (*j)->isVideoDirectToStage()) {
					if (pass == 0)
						continue;
				} else {
					if (pass == 1)
						continue;
				}

				if ((*j)->_visible) {
					if ((*j)->hasSubChannels()) {
						Common::Array<Channel> *list = (*j)->getSubChannels();
						for (Common::Array<Channel>::iterator k = list->begin(); k != list->end(); k++) {
							inkBlitFrom(&(*k), r, blitTo);
						}
					} else {
						inkBlitFrom(*j, r, blitTo);
						if ((*j) == hiliteChannel)
							invertChannel(hiliteChannel, r);
					}
				}
			}
		}
	}

	if (g_director->_debugDraw & kDebugDrawFrame) {
		const Graphics::Font *font = FontMan.getFontByUsage(Graphics::FontManager::kConsoleFont);
		Common::String msg = Common::String::format("Frame: %d", g_director->getCurrentMovie()->getScore()->getCurrentFrame());
		uint32 width = font->getStringWidth(msg);

		blitTo->fillRect(Common::Rect(blitTo->w - 3 - width, 1, blitTo->w - 1, font->getFontHeight() + 1), _wm->_colorBlack);
		font->drawString(blitTo, msg, blitTo->w - 2 - width, 2, width , _wm->_colorWhite);
	}

	_dirtyRects.clear();
	_contentIsDirty = true;

	return true;
}

void Window::setStageColor(uint32 stageColor, bool forceReset) {
	if (stageColor != _stageColor || forceReset) {
		_stageColor = stageColor;
		reset();
		markAllDirty();
	}
}

Datum Window::getStageRect() {
	Graphics::ManagedSurface *surface = getSurface();
	Datum d;
	d.type = RECT;
	d.u.farr = new FArray;
	d.u.farr->arr.push_back(0);
	d.u.farr->arr.push_back(0);
	d.u.farr->arr.push_back(surface->w);
	d.u.farr->arr.push_back(surface->h);
	return d;
}

void Window::reset() {
	resize(_composeSurface->w, _composeSurface->h, true);
	_contentIsDirty = true;
}

void Window::inkBlitFrom(Channel *channel, Common::Rect destRect, Graphics::ManagedSurface *blitTo) {
	Common::Rect srcRect = channel->getBbox();
	destRect.clip(srcRect);

	DirectorPlotData pd = channel->getPlotData();
	pd.destRect = destRect;
	pd.dst = blitTo;

	if (pd.ms) {
		pd.inkBlitShape(srcRect);
	} else if (pd.srf) {
		if (channel->isStretched()) {
			srcRect = channel->getBbox(true);
			pd.inkBlitStretchSurface(srcRect, channel->getMask());
		} else {
			pd.inkBlitSurface(srcRect, channel->getMask());
		}
	} else {
		if (debugChannelSet(kDebugImages, 4)) {
			CastType castType = channel->_sprite->_cast ? channel->_sprite->_cast->_type : kCastTypeNull;
			warning("Window::inkBlitFrom: No source surface: spriteType: %d (%s), castType: %d (%s), castId: %s",
				channel->_sprite->_spriteType, spriteType2str(channel->_sprite->_spriteType), castType, castType2str(castType),
				channel->_sprite->_castId.asString().c_str());
		}
	}
}

Common::Point Window::getMousePos() {
	return g_system->getEventManager()->getMousePos() - Common::Point(_innerDims.left, _innerDims.top);
}

void Window::setVisible(bool visible, bool silent) {
	// setting visible triggers movie load
	if (!_currentMovie && !silent) {
		Common::String movieName = getName();
		setNextMovie(movieName);
	}

	BaseMacWindow::setVisible(visible);

	if (visible)
		_wm->setActiveWindow(_id);
}

bool Window::setNextMovie(Common::String &movieFilenameRaw) {
	Common::String movieFilename = pathMakeRelative(movieFilenameRaw);

	bool fileExists = false;
	Common::File file;
	if (file.open(Common::Path(movieFilename, _vm->_dirSeparator))) {
		fileExists = true;
		file.close();
	}

	debug(1, "Window::setNextMovie: '%s' -> '%s' -> '%s'", movieFilenameRaw.c_str(), convertPath(movieFilenameRaw).c_str(), movieFilename.c_str());

	if (!fileExists) {
		warning("Movie %s does not exist", movieFilename.c_str());
		return false;
	}

	_nextMovie.movie = movieFilename;
	return true;
}

void Window::updateBorderType() {
	if (_isStage) {
		setBorderType(3);
	} else if (!_titleVisible) {
		setBorderType(2);
	} else {
		setBorderType(MAX(0, MIN(_windowType, 16)));
	}
}

void Window::loadNewSharedCast(Cast *previousSharedCast) {
	Common::String previousSharedCastPath;
	Common::String newSharedCastPath = getSharedCastPath();
	if (previousSharedCast && previousSharedCast->getArchive()) {
		previousSharedCastPath = previousSharedCast->getArchive()->getPathName();
	}

	// Check if previous and new sharedCasts are the same
	if (!previousSharedCastPath.empty() && previousSharedCastPath.equalsIgnoreCase(newSharedCastPath)) {
		// Clear those previous widget pointers
		previousSharedCast->releaseCastMemberWidget();
		_currentMovie->_sharedCast = previousSharedCast;

		debugC(1, kDebugLoading, "Skipping loading already loaded shared cast, path: %s", previousSharedCastPath.c_str());
		return;
	}

	// Clean up the previous sharedCast
	if (!previousSharedCastPath.empty()) {
		g_director->_allOpenResFiles.erase(previousSharedCastPath);
		delete previousSharedCast;
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

	Archive *mov = openArchive(_currentPath + Common::lastPathComponent(_nextMovie.movie, g_director->_dirSeparator));

	if (!mov)
		return false;

	_currentMovie = new Movie(this);
	_currentMovie->setArchive(mov);

	debug(0, "\n@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@");
	debug(0, "@@@@   Switching to movie '%s' in '%s'", utf8ToPrintable(_currentMovie->getMacName()).c_str(), _currentPath.c_str());
	debug(0, "@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@\n");

	g_lingo->resetLingo();
	loadNewSharedCast(previousSharedCast);
	return true;
}

bool Window::step() {
	// finish last movie
	if (_currentMovie && _currentMovie->getScore()->_playState == kPlayStopped) {
		debugC(3, kDebugEvents, "\n@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@");
		debugC(3, kDebugEvents, "@@@@   Finishing movie '%s' in '%s'", utf8ToPrintable(_currentMovie->getMacName()).c_str(), _currentPath.c_str());
		debugC(3, kDebugEvents, "@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@\n");

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
		_nextMovie.movie.clear();
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


				if (!debugChannelSet(-1, kDebugCompileOnly) && goodMovie) {
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
			}
			// fall through
		case kPlayStarted:
			debugC(3, kDebugEvents, "\n@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@");
			debugC(3, kDebugEvents, "@@@@   Stepping movie '%s' in '%s'", utf8ToPrintable(_currentMovie->getMacName()).c_str(), _currentPath.c_str());
			debugC(3, kDebugEvents, "@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@\n");
			_currentMovie->getScore()->step();
			return true;
		default:
			return false;
		}
	}

	return false;
}

Common::String Window::getSharedCastPath() {
	Common::Array<Common::String> namesToTry;
	if (_vm->getVersion() < 400) {
		if (g_director->getPlatform() == Common::kPlatformWindows) {
			namesToTry.push_back("SHARDCST.MMM");
		} else {
			namesToTry.push_back("Shared Cast");
		}
	} else if (_vm->getVersion() < 500) {
		namesToTry.push_back("Shared.dir");
		namesToTry.push_back("Shared.dxr");
	} else {
		// TODO: Does D5 actually support D4-style shared cast?
		namesToTry.push_back("Shared.cst");
		namesToTry.push_back("Shared.cxt");
	}

	for (uint i = 0; i < namesToTry.size(); i++) {
		Common::File f;
		if (f.open(Common::Path(_currentPath + namesToTry[i], _vm->_dirSeparator))) {
			f.close();
			return _currentPath + namesToTry[i];
		}
	}

	return Common::String();
}

void Window::freezeLingoState() {
	_frozenLingoStates.push_back(_lingoState);
	_lingoState = new LingoState;
	debugC(kDebugLingoExec, 3, "Freezing Lingo state, depth %d", _frozenLingoStates.size());
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
	debugC(kDebugLingoExec, 3, "Thawing Lingo state, depth %d", _frozenLingoStates.size());
	_lingoState = _frozenLingoStates.back();
	_frozenLingoStates.pop_back();
}

} // End of namespace Director
