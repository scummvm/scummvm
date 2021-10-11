/* ScummVM - Graphic Adventure Engine
 *
 * ScummVM is the legal property of its developers, whose names
 * are too numerous to list here. Please refer to the COPYRIGHT
 * file distributed with this source distribution.
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License
 * as published by the Free Software Foundation; either version 2
 * of the License, or (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.
 *
 */

#include "common/file.h"
#include "common/system.h"
#include "common/macresman.h"

#include "graphics/primitives.h"
#include "graphics/macgui/macwindowmanager.h"

#include "director/director.h"
#include "director/cast.h"
#include "director/lingo/lingo.h"
#include "director/movie.h"
#include "director/window.h"
#include "director/score.h"
#include "director/castmember.h"
#include "director/cursor.h"
#include "director/channel.h"
#include "director/sound.h"
#include "director/sprite.h"
#include "director/util.h"

namespace Director {

Window::Window(int id, bool scrollable, bool resizable, bool editable, Graphics::MacWindowManager *wm, DirectorEngine *vm, bool isStage)
	: MacWindow(id, scrollable, resizable, editable, wm), Object<Window>("Window") {
	_vm = vm;
	_isStage = isStage;
	_stageColor = _wm->_colorBlack;
	_puppetTransition = nullptr;
	_soundManager = new DirectorSound(this);

	_currentMovie = nullptr;
	_mainArchive = nullptr;
	_macBinary = nullptr;
	_nextMovie.frameI = -1;
	_newMovieStarted = true;

	_objType = kWindowObj;
	_startFrame = _vm->getStartMovie().startFrame;

	_windowType = -1;
	_titleVisible = true;
	updateBorderType();

	_retPC = 0;
	_retScript = nullptr;
	_retContext = nullptr;
	_retFreezeContext = false;
	_retLocalVars = nullptr;
}

Window::~Window() {
	delete _soundManager;
	delete _currentMovie;
	if (_macBinary) {
		delete _macBinary;
		_macBinary = nullptr;
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
					inkBlitFrom(*j, r, blitTo);
					if ((*j) == hiliteChannel)
						invertChannel(hiliteChannel, r);
				}
			}
		}
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

void Window::reset() {
	resize(_composeSurface->w, _composeSurface->h, true);
	_composeSurface->clear(_stageColor);
	_contentIsDirty = true;
}

void Window::inkBlitFrom(Channel *channel, Common::Rect destRect, Graphics::ManagedSurface *blitTo) {
	Common::Rect srcRect = channel->getBbox();
	destRect.clip(srcRect);

	DirectorPlotData pd = channel->getPlotData();
	pd.destRect = destRect;
	pd.dst = blitTo;

	if (pd.ms) {
		pd.inkBlitShape();
	} else if (pd.srf) {
		if (channel->isStretched()) {
			srcRect = channel->getBbox(true);
			pd.inkBlitStretchSurface(srcRect, channel->getMask());
		} else {
			pd.inkBlitSurface(srcRect, channel->getMask());
		}
	} else {
		if (debugChannelSet(kDebugImages, 2))
			warning("Window::inkBlitFrom: No source surface: spriteType: %d, castType: %d, castId: %s", channel->_sprite->_spriteType, channel->_sprite->_cast ? channel->_sprite->_cast->_type : 0, channel->_sprite->_castId.asString().c_str());
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

	// prepare next movie
	if (!_nextMovie.movie.empty()) {
		_soundManager->changingMovie();

		_newMovieStarted = true;

		_currentPath = getPath(_nextMovie.movie, _currentPath);

		Cast *sharedCast = nullptr;
		if (_currentMovie) {
			sharedCast = _currentMovie->getSharedCast();
			_currentMovie->_sharedCast = nullptr;
		}

		delete _currentMovie;
		_currentMovie = nullptr;

		Archive *mov = openMainArchive(_currentPath + Common::lastPathComponent(_nextMovie.movie, g_director->_dirSeparator));

		if (!mov) {
			warning("nextMovie: No movie is loaded");

			if (_vm->getGameGID() == GID_TESTALL) {
				return true;
			}

			return false;
		}

		_currentMovie = new Movie(this);
		_currentMovie->setArchive(mov);

		debug(0, "\n@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@");
		debug(0, "@@@@   Switching to movie '%s' in '%s'", utf8ToPrintable(_currentMovie->getMacName()).c_str(), _currentPath.c_str());
		debug(0, "@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@\n");

		g_lingo->resetLingo();
		Common::String sharedCastPath = getSharedCastPath();
		if (!sharedCastPath.empty()) {
			if (sharedCast && sharedCast->_castArchive
					&& sharedCast->_castArchive->getPathName().equalsIgnoreCase(sharedCastPath)) {
				// if we are not deleting shared cast, then we need to clear those previous widget pointer
				sharedCast->releaseCastMemberWidget();
				_currentMovie->_sharedCast = sharedCast;
			} else {
				delete sharedCast;
				_currentMovie->loadSharedCastsFrom(sharedCastPath);
			}
		} else {
			delete sharedCast;
		}

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

} // End of namespace Director
