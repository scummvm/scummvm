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
#include "director/stage.h"
#include "director/score.h"
#include "director/castmember.h"
#include "director/channel.h"
#include "director/sprite.h"
#include "director/util.h"

namespace Director {

Stage::Stage(int id, bool scrollable, bool resizable, bool editable, Graphics::MacWindowManager *wm, DirectorEngine *vm)
	: MacWindow(id, scrollable, resizable, editable, wm), Object("Stage") {
	_vm = vm;
	_stageColor = 0;
	_puppetTransition = nullptr;

	_currentMovie = nullptr;
	_mainArchive = nullptr;
	_macBinary = nullptr;
	_nextMovie.frameI = -1;
	_newMovieStarted = true;

	_objType = kWindowObj;
}

Stage::~Stage() {
	delete _currentMovie;
	if (_macBinary) {
		delete _macBinary;
		_macBinary = nullptr;
	}
}

void Stage::invertChannel(Channel *channel) {
	Common::Rect destRect = channel->getBbox();
	for (int i = 0; i < destRect.height(); i++) {
		byte *src = (byte *)_surface.getBasePtr(destRect.left, destRect.top + i);
		for (int j = 0; j < destRect.width(); j++, src++)
			*src = ~(*src);
	}
}

bool Stage::render(bool forceRedraw, Graphics::ManagedSurface *blitTo) {
	if (!_currentMovie)
		return false;

	if (!blitTo)
		blitTo = &_surface;

	if (forceRedraw) {
		blitTo->clear(_stageColor);
		_dirtyRects.clear();
		_dirtyRects.push_back(Common::Rect(_surface.w, _surface.h));
	} else {
		if (_dirtyRects.size() == 0)
			return true;

		mergeDirtyRects();
	}

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

void Stage::setStageColor(uint stageColor) {
	if (stageColor != _stageColor) {
		_stageColor = stageColor;
		reset();
	}
}

void Stage::reset() {
	_surface.clear(_stageColor);
	_contentIsDirty = true;
}

void Stage::addDirtyRect(const Common::Rect &r) {
	Common::Rect bounds = r;
	bounds.clip(Common::Rect(_innerDims.width(), _innerDims.height()));

	if (bounds.width() > 0 && bounds.height() > 0)
		_dirtyRects.push_back(bounds);
}

void Stage::mergeDirtyRects() {
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

void Stage::inkBlitFrom(Channel *channel, Common::Rect destRect, Graphics::ManagedSurface *blitTo) {
	Common::Rect srcRect = channel->getBbox();
	destRect.clip(srcRect);

	DirectorPlotData pd = channel->getPlotData();
	pd.destRect = destRect;
	pd.dst = blitTo;

	if (pd.isShape) {
		inkBlitShape(&pd, srcRect);
	} else if (pd.src) {
		if (channel->_sprite->_spriteType == kTextSprite) {
			// Copy colourization is already applied to text by default
			if (pd.ink != kInkTypeCopy)
				pd.manualInk = true;
			else
				pd.applyColor = false;
		}

		inkBlitSurface(&pd, srcRect, channel->getMask());
	} else {
		warning("Stage::inkBlitFrom: No source surface");
	}
}

void Stage::inkBlitShape(DirectorPlotData *pd, Common::Rect &srcRect) {
	MacShape *ms = ((MacShape *)pd->src);

	if (!ms)
		return;

	switch (pd->ink) {
	case kInkTypeNotTrans:
	case kInkTypeNotReverse:
	case kInkTypeNotGhost:
		return;
	case kInkTypeReverse:
		ms->foreColor = 0;
		ms->backColor = 0;
		break;
	default:
		break;
	}

	Common::Rect fillRect((int)srcRect.width(), (int)srcRect.height());
	fillRect.moveTo(srcRect.left, srcRect.top);
	Graphics::MacPlotData plotFill(pd->dst, nullptr, &g_director->getPatterns(), ms->pattern, srcRect.left, srcRect.top, 1, ms->backColor);

	Common::Rect strokeRect(MAX((int)srcRect.width() - ms->lineSize, 0), MAX((int)srcRect.height() - ms->lineSize, 0));
	strokeRect.moveTo(srcRect.left, srcRect.top);
	Graphics::MacPlotData plotStroke(pd->dst, nullptr, &g_director->getPatterns(), 1, strokeRect.left, strokeRect.top, ms->lineSize, ms->backColor);

	switch (ms->spriteType) {
	case kRectangleSprite:
		ms->pd = &plotFill;
		Graphics::drawFilledRect(fillRect, ms->foreColor, inkDrawPixel, pd);
		// fall through
	case kOutlinedRectangleSprite:
		ms->pd = &plotStroke;
		Graphics::drawRect(strokeRect, ms->foreColor, inkDrawPixel, pd);
		break;
	case kRoundedRectangleSprite:
		ms->pd = &plotFill;
		Graphics::drawRoundRect(fillRect, 12, ms->foreColor, true, inkDrawPixel, pd);
		// fall through
	case kOutlinedRoundedRectangleSprite:
		ms->pd = &plotStroke;
		Graphics::drawRoundRect(strokeRect, 12, ms->foreColor, false, inkDrawPixel, pd);
		break;
	case kOvalSprite:
		ms->pd = &plotFill;
		Graphics::drawEllipse(fillRect.left, fillRect.top, fillRect.right, fillRect.bottom, ms->foreColor, true, inkDrawPixel, pd);
		// fall through
	case kOutlinedOvalSprite:
		ms->pd = &plotStroke;
		Graphics::drawEllipse(strokeRect.left, strokeRect.top, strokeRect.right, strokeRect.bottom, ms->foreColor, false, inkDrawPixel, pd);
		break;
	case kLineTopBottomSprite:
		ms->pd = &plotStroke;
		Graphics::drawLine(strokeRect.left, strokeRect.top, strokeRect.right, strokeRect.bottom, ms->foreColor, inkDrawPixel, pd);
		break;
	case kLineBottomTopSprite:
		ms->pd = &plotStroke;
		Graphics::drawLine(strokeRect.left, strokeRect.top, strokeRect.right, strokeRect.bottom, ms->foreColor, inkDrawPixel, pd);
		break;
	default:
		warning("Stage::inkBlitFrom: Expected shape type but got type %d", ms->spriteType);
	}
}

void Stage::inkBlitSurface(DirectorPlotData *pd, Common::Rect &srcRect, const Graphics::Surface *mask) {
	pd->srcPoint.y = MAX(abs(srcRect.top - pd->destRect.top), 0);
	for (int i = 0; i < pd->destRect.height(); i++, pd->srcPoint.y++) {
		pd->srcPoint.x = MAX(abs(srcRect.left - pd->destRect.left), 0);
		const byte *msk = mask ? (const byte *)mask->getBasePtr(pd->srcPoint.x, pd->srcPoint.y) : nullptr;

		for (int j = 0; j < pd->destRect.width(); j++, pd->srcPoint.x++)
			if (!mask || (msk && (pd->ink == kInkTypeMask ? *msk++ : !(*msk++))))
				inkDrawPixel(pd->destRect.left + j, pd->destRect.top + i, 0, pd);
	}
}

Common::Point Stage::getMousePos() {
	return g_system->getEventManager()->getMousePos() - Common::Point(_innerDims.left, _innerDims.top);
}

void Stage::setVisible(bool visible) {
	// setting visible triggers movie load
	if (!_currentMovie) {
		Common::String movieName = getName();
		setNextMovie(movieName);
	}

	BaseMacWindow::setVisible(visible);
}

bool Stage::setNextMovie(Common::String &movieFilenameRaw) {
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

	debug(1, "Stage::setNextMovie: '%s' -> '%s' -> '%s' -> '%s'", movieFilenameRaw.c_str(), convertPath(movieFilenameRaw).c_str(),
			movieFilename.c_str(), cleanedFilename.c_str());

	if (!fileExists) {
		warning("Movie %s does not exist", movieFilename.c_str());
		return false;
	}

	_nextMovie.movie = cleanedFilename;
	return true;
}

bool Stage::step() {
	if (_currentMovie) {
		debug(0, "\n@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@");
		debug(0, "@@@@   Movie name '%s' in '%s'", _currentMovie->getMacName().c_str(), _currentPath.c_str());
		debug(0, "@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@\n");

		switch (_currentMovie->getScore()->_playState) {
		case kPlayNotStarted:
			{
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
				} else {
					return false;
				}
			}
			return true;
		case kPlayStarted:
			_currentMovie->getScore()->step();
			return true;
		case kPlayStopped:
			_currentMovie->getScore()->stopPlay();
			debugC(1, kDebugEvents, "Finished playback of movie '%s'", _currentMovie->getMacName().c_str());
		}
	}

	if (_vm->getGameGID() == GID_TESTALL) {
		_nextMovie = getNextMovieFromQueue();
	}

	// If a loop was requested, do it
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
		debug(0, "Switching to movie '%s'", _currentMovie->getMacName().c_str());

		g_lingo->resetLingo();
		if (sharedCast && sharedCast->_castArchive
				&& sharedCast->_castArchive->getFileName().equalsIgnoreCase(_currentPath + _vm->_sharedCastFile)) {
			_currentMovie->_sharedCast = sharedCast;
		} else {
			delete sharedCast;
			_currentMovie->loadSharedCastsFrom(_currentPath + _vm->_sharedCastFile);
		}

		_nextMovie.movie.clear();
		return true;
	}

	return false;
}

} // end of namespace Director
