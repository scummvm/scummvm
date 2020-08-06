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
#include "director/sprite.h"
#include "director/util.h"

namespace Director {

Window::Window(int id, bool scrollable, bool resizable, bool editable, Graphics::MacWindowManager *wm, DirectorEngine *vm, bool isStage)
	: MacWindow(id, scrollable, resizable, editable, wm), Object<Window>("Window") {
	_vm = vm;
	_isStage = isStage;
	_stageColor = 0;
	_puppetTransition = nullptr;

	_currentMovie = nullptr;
	_mainArchive = nullptr;
	_macBinary = nullptr;
	_nextMovie.frameI = -1;
	_newMovieStarted = true;

	_objType = kWindowObj;
	_startFrame = _vm->getStartMovie().startFrame;

	setWindowType(0);
}

Window::~Window() {
	delete _currentMovie;
	if (_macBinary) {
		delete _macBinary;
		_macBinary = nullptr;
	}
}

void Window::invertChannel(Channel *channel) {
	const Graphics::Surface *mask = channel->getMask(true);
	Common::Rect destRect = channel->getBbox();

	for (int i = 0; i < destRect.height(); i++) {
		byte *src = (byte *)_composeSurface->getBasePtr(destRect.left, destRect.top + i);
		const byte *msk = mask ? (const byte *)mask->getBasePtr(0, i) : nullptr;

		for (int j = 0; j < destRect.width(); j++, src++)
			if (!mask || (msk && !(*msk++)))
				*src = ~(*src);
	}
}

bool Window::render(bool forceRedraw, Graphics::ManagedSurface *blitTo) {
	if (!_currentMovie)
		return false;

	if (forceRedraw) {
		blitTo->clear(_stageColor);
		markAllDirty();
	} else {
		if (_dirtyRects.size() == 0)
			return false;

		mergeDirtyRects();
	}

	if (!blitTo)
		blitTo = _composeSurface;

	for (Common::List<Common::Rect>::iterator i = _dirtyRects.begin(); i != _dirtyRects.end(); i++) {
		const Common::Rect &r = *i;
		blitTo->fillRect(r, _stageColor);

		_dirtyChannels = _currentMovie->getScore()->getSpriteIntersections(r);
		for (Common::List<Channel *>::iterator j = _dirtyChannels.begin(); j != _dirtyChannels.end(); j++) {
			if ((*j)->_visible)
				inkBlitFrom(*j, r, blitTo);
		}
	}

	_dirtyRects.clear();
	_contentIsDirty = true;

	return true;
}

void Window::setStageColor(uint stageColor, bool forceReset) {
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

void Window::addDirtyRect(const Common::Rect &r) {
	if (!r.isValidRect())
		return;

	Common::Rect bounds = r;
	bounds.clip(Common::Rect(_innerDims.width(), _innerDims.height()));

	if (bounds.width() > 0 && bounds.height() > 0)
		_dirtyRects.push_back(bounds);
}

void Window::markAllDirty() {
	_dirtyRects.clear();
	_dirtyRects.push_back(Common::Rect(_composeSurface->w, _composeSurface->h));
}

void Window::mergeDirtyRects() {
	Common::List<Common::Rect>::iterator rOuter, rInner;

	// Process the dirty rect list to find any rects to merge
	for (rOuter = _dirtyRects.begin(); rOuter != _dirtyRects.end(); ++rOuter) {
		rInner = rOuter;
		while (++rInner != _dirtyRects.end()) {

			if ((*rOuter).intersects(*rInner)) {
				// These two rectangles overlap, so merge them
				rOuter->extend(*rInner);

				// remove the inner rect from the list
				_dirtyRects.erase(rInner);

				// move back to beginning of list
				rInner = rOuter;
			}
		}
	}
}

void Window::inkBlitFrom(Channel *channel, Common::Rect destRect, Graphics::ManagedSurface *blitTo) {
	Common::Rect srcRect = channel->getBbox();
	destRect.clip(srcRect);

	DirectorPlotData pd = channel->getPlotData();
	pd.destRect = destRect;
	pd.dst = blitTo;

	if (pd.ms) {
		inkBlitShape(&pd, srcRect);
	} else if (pd.srf) {
		if (channel->isStretched()) {
			srcRect = channel->getBbox(true);
			inkBlitStretchSurface(&pd, srcRect, channel->getMask());
		} else {
			inkBlitSurface(&pd, srcRect, channel->getMask());
		}
	} else {
		warning("Window::inkBlitFrom: No source surface");
	}
}

void Window::inkBlitShape(DirectorPlotData *pd, Common::Rect &srcRect) {
	if (!pd->ms)
		return;

	// Preprocess shape colours
	switch (pd->ink) {
	case kInkTypeNotTrans:
	case kInkTypeNotReverse:
	case kInkTypeNotGhost:
		return;
	case kInkTypeReverse:
		pd->ms->foreColor = 0;
		pd->ms->backColor = 0;
		break;
	default:
		break;
	}

	Common::Rect fillRect((int)srcRect.width(), (int)srcRect.height());
	fillRect.moveTo(srcRect.left, srcRect.top);
	Graphics::MacPlotData plotFill(pd->dst, nullptr, &g_director->getPatterns(), pd->ms->pattern, srcRect.left, srcRect.top, 1, pd->ms->backColor);

	Common::Rect strokeRect(MAX((int)srcRect.width() - pd->ms->lineSize, 0), MAX((int)srcRect.height() - pd->ms->lineSize, 0));
	strokeRect.moveTo(srcRect.left, srcRect.top);
	Graphics::MacPlotData plotStroke(pd->dst, nullptr, &g_director->getPatterns(), 1, strokeRect.left, strokeRect.top, pd->ms->lineSize, pd->ms->backColor);

	switch (pd->ms->spriteType) {
	case kRectangleSprite:
		pd->ms->pd = &plotFill;
		Graphics::drawFilledRect(fillRect, pd->ms->foreColor, inkDrawPixel, pd);
		// fall through
	case kOutlinedRectangleSprite:
		pd->ms->pd = &plotStroke;
		Graphics::drawRect(strokeRect, pd->ms->foreColor, inkDrawPixel, pd);
		break;
	case kRoundedRectangleSprite:
		pd->ms->pd = &plotFill;
		Graphics::drawRoundRect(fillRect, 12, pd->ms->foreColor, true, inkDrawPixel, pd);
		// fall through
	case kOutlinedRoundedRectangleSprite:
		pd->ms->pd = &plotStroke;
		Graphics::drawRoundRect(strokeRect, 12, pd->ms->foreColor, false, inkDrawPixel, pd);
		break;
	case kOvalSprite:
		pd->ms->pd = &plotFill;
		Graphics::drawEllipse(fillRect.left, fillRect.top, fillRect.right, fillRect.bottom, pd->ms->foreColor, true, inkDrawPixel, pd);
		// fall through
	case kOutlinedOvalSprite:
		pd->ms->pd = &plotStroke;
		Graphics::drawEllipse(strokeRect.left, strokeRect.top, strokeRect.right, strokeRect.bottom, pd->ms->foreColor, false, inkDrawPixel, pd);
		break;
	case kLineTopBottomSprite:
		pd->ms->pd = &plotStroke;
		Graphics::drawLine(strokeRect.left, strokeRect.top, strokeRect.right, strokeRect.bottom, pd->ms->foreColor, inkDrawPixel, pd);
		break;
	case kLineBottomTopSprite:
		pd->ms->pd = &plotStroke;
		Graphics::drawLine(strokeRect.left, strokeRect.top, strokeRect.right, strokeRect.bottom, pd->ms->foreColor, inkDrawPixel, pd);
		break;
	default:
		warning("Window::inkBlitFrom: Expected shape type but got type %d", pd->ms->spriteType);
	}
}

void Window::inkBlitSurface(DirectorPlotData *pd, Common::Rect &srcRect, const Graphics::Surface *mask) {
	if (!pd->srf)
		return;

	// TODO: Determine why colourization causes problems in Warlock
	if (pd->sprite == kTextSprite)
		pd->applyColor = false;

	pd->srcPoint.y = abs(srcRect.top - pd->destRect.top);
	for (int i = 0; i < pd->destRect.height(); i++, pd->srcPoint.y++) {
		pd->srcPoint.x = abs(srcRect.left - pd->destRect.left);
		const byte *msk = mask ? (const byte *)mask->getBasePtr(pd->srcPoint.x, pd->srcPoint.y) : nullptr;

		for (int j = 0; j < pd->destRect.width(); j++, pd->srcPoint.x++) {
			if (!mask || (msk && (pd->ink == kInkTypeMask ? *msk++ : !(*msk++)))) {
				inkDrawPixel(pd->destRect.left + j, pd->destRect.top + i,
										 preprocessColor(pd, *((byte *)pd->srf->getBasePtr(pd->srcPoint.x, pd->srcPoint.y))), pd);
			}
		}
	}
}

void Window::inkBlitStretchSurface(DirectorPlotData *pd, Common::Rect &srcRect, const Graphics::Surface *mask) {
	if (!pd->srf)
		return;

	// TODO: Determine why colourization causes problems in Warlock
	if (pd->sprite == kTextSprite)
		pd->applyColor = false;

	int scaleX = SCALE_THRESHOLD * srcRect.width() / pd->destRect.width();
	int scaleY = SCALE_THRESHOLD * srcRect.height() / pd->destRect.height();

	pd->srcPoint.y = abs(srcRect.top - pd->destRect.top);

	for (int i = 0, scaleYCtr = 0; i < pd->destRect.height(); i++, scaleYCtr += scaleY, pd->srcPoint.y++) {
		pd->srcPoint.x = abs(srcRect.left - pd->destRect.left);
		const byte *msk = mask ? (const byte *)mask->getBasePtr(pd->srcPoint.x, pd->srcPoint.y) : nullptr;

		for (int xCtr = 0, scaleXCtr = 0; xCtr < pd->destRect.width(); xCtr++, scaleXCtr += scaleX, pd->srcPoint.x++) {
			if (!mask || (msk && (pd->ink == kInkTypeMask ? *msk++ : !(*msk++)))) {
			inkDrawPixel(pd->destRect.left + xCtr, pd->destRect.top + i,
									 preprocessColor(pd, *((byte *)pd->srf->getBasePtr(scaleXCtr / SCALE_THRESHOLD, scaleYCtr / SCALE_THRESHOLD))), pd);
			}
		}
	}
}

int Window::preprocessColor(DirectorPlotData *p, int src) {
	// HACK: Right now this method is just used for adjusting the colourization on text
	// sprites, as it would be costly to colourize the chunks on the fly each
	// time a section needs drawing. It's ugly but mostly works.
	if (p->sprite == kTextSprite) {
		switch(p->ink) {
		case kInkTypeMask:
			src = (src == p->backColor ? 0xff : p->foreColor);
			break;
		case kInkTypeReverse:
			src = (src == p->foreColor ? 0 : p->colorWhite);
			break;
		case kInkTypeNotReverse:
			src = (src == p->backColor ? p->colorWhite : 0);
			break;
		case kInkTypeGhost:
			src = (src == p->foreColor ? p->backColor : p->colorWhite);
			break;
		case kInkTypeNotGhost:
			src = (src == p->backColor ? p->colorWhite : p->backColor);
			break;
		case kInkTypeNotCopy:
			src = (src == p->foreColor ? p->backColor : p->foreColor);
			break;
		case kInkTypeNotTrans:
			src = (src == p->foreColor ? p->backColor : p->colorWhite);
			break;
		default:
			break;
		}
	}

	return src;
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
}

bool Window::setNextMovie(Common::String &movieFilenameRaw) {
	Common::String movieFilename = pathMakeRelative(movieFilenameRaw);
	Common::String cleanedFilename;

	bool fileExists = false;

	if (_vm->getPlatform() == Common::kPlatformMacintosh) {
		Common::MacResManager resMan;

		for (const byte *p = (const byte *)movieFilename.c_str(); *p; p++)
			if (*p >= 0x20 && *p <= 0x7f)
				cleanedFilename += (char) *p;

		if (resMan.open(movieFilename)) {
			fileExists = true;
			cleanedFilename = movieFilename;
		} else if (!movieFilename.equals(cleanedFilename) && resMan.open(cleanedFilename)) {
			fileExists = true;
		}
	} else {
		Common::File file;
		cleanedFilename = movieFilename + ".MMM";

		if (file.open(movieFilename)) {
			fileExists = true;
			cleanedFilename = movieFilename;
		} else if (!movieFilename.equals(cleanedFilename) && file.open(cleanedFilename)) {
			fileExists = true;
		}
	}

	debug(1, "Window::setNextMovie: '%s' -> '%s' -> '%s' -> '%s'", movieFilenameRaw.c_str(), convertPath(movieFilenameRaw).c_str(),
			movieFilename.c_str(), cleanedFilename.c_str());

	if (!fileExists) {
		warning("Movie %s does not exist", movieFilename.c_str());
		return false;
	}

	_nextMovie.movie = cleanedFilename;
	return true;
}

void Window::updateBorderType() {
	if (_isStage) {
		setBorderType(3);
	} else {
		setBorderType(MAX(0, MIN(_windowType, 16)));
	}
}

bool Window::step() {
	// finish last movie
	if (_currentMovie && _currentMovie->getScore()->_playState == kPlayStopped) {
		debugC(3, kDebugEvents, "\n@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@");
		debugC(3, kDebugEvents, "@@@@   Finishing movie '%s' in '%s'", _currentMovie->getMacName().c_str(), _currentPath.c_str());
		debugC(3, kDebugEvents, "@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@\n");

		_currentMovie->getScore()->stopPlay();
		debugC(1, kDebugEvents, "Finished playback of movie '%s'", _currentMovie->getMacName().c_str());

		if (_vm->getGameGID() == GID_TESTALL) {
			_nextMovie = getNextMovieFromQueue();
		}
	}

	// prepare next movie
	if (!_nextMovie.movie.empty()) {
		_newMovieStarted = true;

		_currentPath = getPath(_nextMovie.movie, _currentPath);

		Cast *sharedCast = nullptr;
		if (_currentMovie) {
			sharedCast = _currentMovie->getSharedCast();
			_currentMovie->_sharedCast = nullptr;
		}

		delete _currentMovie;
		_currentMovie = nullptr;

		Archive *mov = openMainArchive(_currentPath + Common::lastPathComponent(_nextMovie.movie, '/'));

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
		debug(0, "@@@@   Switching to movie '%s' in '%s'", _currentMovie->getMacName().c_str(), _currentPath.c_str());
		debug(0, "@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@\n");

		g_lingo->resetLingo();
		if (sharedCast && sharedCast->_castArchive
				&& sharedCast->_castArchive->getPathName().equalsIgnoreCase(_currentPath + _vm->_sharedCastFile)) {
			_currentMovie->_sharedCast = sharedCast;
		} else {
			delete sharedCast;
			_currentMovie->loadSharedCastsFrom(_currentPath + _vm->_sharedCastFile);
		}

		_nextMovie.movie.clear();
	}

	// play current movie
	if (_currentMovie) {
		switch (_currentMovie->getScore()->_playState) {
		case kPlayNotStarted:
			{
				debug(0, "\n@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@");
				debug(0, "@@@@   Loading movie '%s' in '%s'", _currentMovie->getMacName().c_str(), _currentPath.c_str());
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
			debugC(3, kDebugEvents, "@@@@   Stepping movie '%s' in '%s'", _currentMovie->getMacName().c_str(), _currentPath.c_str());
			debugC(3, kDebugEvents, "@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@\n");
			_currentMovie->getScore()->step();
			return true;
		default:
			return false;
		}
	}

	return false;
}

} // end of namespace Director
