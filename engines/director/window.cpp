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
	_isModal = false;

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

void Window::drawFrameCounter(Graphics::ManagedSurface *blitTo) {
	const Graphics::Font *font = FontMan.getFontByUsage(Graphics::FontManager::kConsoleFont);
	Common::String msg = Common::String::format("Frame: %d", g_director->getCurrentMovie()->getScore()->getCurrentFrame());
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

	for (auto &i : _dirtyRects) {
		const Common::Rect &r = i;
		_dirtyChannels = _currentMovie->getScore()->getSpriteIntersections(r);

		bool shouldClear = true;
		for (auto &j : _dirtyChannels) {
			if (j->_visible && r == j->getBbox() && j->isTrail()) {
				shouldClear = false;
				break;
			}
		}

		if (shouldClear)
			blitTo->fillRect(r, _stageColor);

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

				font->drawString(blitTo, Common::String::format("m: %d, ch: %d", channel->_sprite->_castId.member, i), bbox.left + 3, bbox.top + 3, 128, g_director->_wm->_colorBlack);
				font->drawString(blitTo, Common::String::format("m: %d, ch: %d", channel->_sprite->_castId.member, i), bbox.left + 2, bbox.top + 2, 128, g_director->_wm->_colorWhite);
			}
		}
	}

	if (g_director->_debugDraw & kDebugDrawFrame)
		drawFrameCounter(blitTo);

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

void Window::setTitleVisible(bool titleVisible) {
	MacWindow::setTitleVisible(titleVisible);
	updateBorderType();

	setVisible(true); // Activate this window on top
}

Datum Window::getStageRect() {
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

	setVisible(true); // Activate this window on top
}

void Window::setFileName(Common::String filename) {
	setNextMovie(filename);

	setVisible(true); // Activate this window on top
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

	if (pd.ms) {
		pd.inkBlitShape(srcRect);
	} else if (pd.srf) {
		pd.inkBlitSurface(srcRect, channel->getMask());
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
	Common::Path movieFilename = findMoviePath(movieFilenameRaw);

	bool fileExists = false;
	Common::File file;
	if (!movieFilename.empty() && file.open(movieFilename)) {
		fileExists = true;
		file.close();
	}

	debug(1, "Window::setNextMovie: '%s' -> '%s' -> '%s'", movieFilenameRaw.c_str(), convertPath(movieFilenameRaw).c_str(), movieFilename.toString().c_str());

	if (!fileExists) {
		warning("Movie %s does not exist", movieFilename.toString().c_str());
		return false;
	}

	_nextMovie.movie = movieFilename.toString(g_director->_dirSeparator);
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
		previousSharedCastPath = Common::Path(previousSharedCast->getArchive()->getPathName(), g_director->_dirSeparator);
	}

	// Check if previous and new sharedCasts are the same
	if (!previousSharedCastPath.empty() && previousSharedCastPath == newSharedCastPath) {
		// Clear those previous widget pointers
		previousSharedCast->releaseCastMemberWidget();
		_currentMovie->_sharedCast = previousSharedCast;

		debugC(1, kDebugLoading, "Skipping loading already loaded shared cast, path: %s", previousSharedCastPath.toString().c_str());
		return;
	}

	// Clean up the previous sharedCast
	if (previousSharedCast) {
		g_director->_allSeenResFiles.erase(previousSharedCastPath);
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

	Common::Path archivePath = Common::Path(_currentPath, g_director->_dirSeparator);
	archivePath.appendInPlace(Common::lastPathComponent(_nextMovie.movie, g_director->_dirSeparator));
	Archive *mov = g_director->openArchive(archivePath);

	if (!mov)
		return false;

	probeResources(mov);
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
