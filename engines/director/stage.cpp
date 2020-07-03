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
#include "director/sprite.h"
#include "director/util.h"

namespace Director {

Stage::Stage(int id, bool scrollable, bool resizable, bool editable, Graphics::MacWindowManager *wm, DirectorEngine *vm)
	: MacWindow(id, scrollable, resizable, editable, wm) {
	_vm = vm;
	_stageColor = 0;
	_puppetTransition = nullptr;

	_currentMovie = nullptr;
	_mainArchive = nullptr;
	_macBinary = nullptr;
	_nextMovie.frameI = -1;
	_newMovieStarted = true;
}

Stage::~Stage() {
	delete _currentMovie;
	if (_macBinary) {
		delete _macBinary;
		_macBinary = nullptr;
	}
}

bool Stage::render(bool forceRedraw, Graphics::ManagedSurface *blitTo) {
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

		_dirtyChannels = g_director->getCurrentMovie()->getScore()->getSpriteIntersections(r);
		for (Common::List<Channel *>::iterator j = _dirtyChannels.begin(); j != _dirtyChannels.end(); j++)
			inkBlitFrom(*j, r, blitTo);
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
	Common::Rect clip = _dims;
	clip.moveTo(0, 0);
	bounds.clip(clip);

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

	Sprite *sprite = channel->_sprite;
	MacShape *ms = channel->getShape();
	DirectorPlotData pd(_wm, channel->getSurface(), blitTo, sprite->_ink, sprite->_backColor, g_director->getPaletteColorCount());
	pd.destRect = destRect;

	// Shapes do not have surfaces of their own, so draw the shape directly upon
	// stage surface.
	if (ms) {
		if (ms->foreColor == ms->backColor)
			return;

		Common::Rect fillRect((int)srcRect.width(), (int)srcRect.height());
		fillRect.moveTo(srcRect.left, srcRect.top);
		Graphics::MacPlotData plotFill(blitTo, nullptr, &g_director->getPatterns(), sprite->getPattern(), srcRect.left, srcRect.top, 1, ms->backColor);

		Common::Rect strokeRect(MAX((int)srcRect.width() - ms->lineSize, 0), MAX((int)srcRect.height() - ms->lineSize, 0));
		strokeRect.moveTo(srcRect.left, srcRect.top);
		Graphics::MacPlotData plotStroke(blitTo, nullptr, &g_director->getPatterns(), 1, strokeRect.left, strokeRect.top, ms->lineSize, ms->backColor);

		switch (ms->spriteType) {
		case kRectangleSprite:
			pd.macPlot = &plotFill;
			Graphics::drawFilledRect(fillRect, ms->foreColor, inkDrawPixel, &pd);
			// fall through
		case kOutlinedRectangleSprite:
			pd.macPlot = &plotStroke;
			Graphics::drawRect(strokeRect, ms->foreColor, inkDrawPixel, &pd);
			break;
		case kRoundedRectangleSprite:
			pd.macPlot = &plotFill;
			Graphics::drawRoundRect(fillRect, 12, ms->foreColor, true, inkDrawPixel, &pd);
			// fall through
		case kOutlinedRoundedRectangleSprite:
			pd.macPlot = &plotStroke;
			Graphics::drawRoundRect(strokeRect, 12, ms->foreColor, false, inkDrawPixel, &pd);
			break;
		case kOvalSprite:
			pd.macPlot = &plotFill;
			Graphics::drawEllipse(fillRect.left, fillRect.top, fillRect.right, fillRect.bottom, ms->foreColor, true, inkDrawPixel, &plotFill);
			// fall through
		case kOutlinedOvalSprite:
			pd.macPlot = &plotStroke;
			Graphics::drawEllipse(strokeRect.left, strokeRect.top, strokeRect.right, strokeRect.bottom, ms->foreColor, false, inkDrawPixel, &plotStroke);
			break;
		case kLineTopBottomSprite:
			pd.macPlot = &plotStroke;
			Graphics::drawLine(strokeRect.left, strokeRect.top, strokeRect.right, strokeRect.bottom, ms->foreColor, inkDrawPixel, &plotStroke);
			break;
		case kLineBottomTopSprite:
			pd.macPlot = &plotStroke;
			Graphics::drawLine(strokeRect.left, strokeRect.top, strokeRect.right, strokeRect.bottom, ms->foreColor, inkDrawPixel, &plotStroke);
			break;
		default:
			warning("Stage::inkBlitFrom: Expected shape type but got type %d", ms->spriteType);
		}

		delete ms;
		return;
	}

	// First, get any masks that might be needed.
	const Graphics::Surface *mask = channel->getMask();

	pd.srcPoint.y = MAX(abs(srcRect.top - destRect.top), 0);
	for (int i = 0; i < destRect.height(); i++, pd.srcPoint.y++) {
		pd.srcPoint.x = MAX(abs(srcRect.left - destRect.left), 0);
		const byte *msk = mask ? (const byte *)mask->getBasePtr(pd.srcPoint.x, pd.srcPoint.y) : nullptr;

		for (int j = 0; j < destRect.width(); j++, pd.srcPoint.x++)
			if (!mask || (msk && (sprite->_ink == kInkTypeMatte ? !(*msk++) : *msk++)))
				inkDrawPixel(destRect.left + j, destRect.top + i, 0, &pd);
	}
}

Common::Point Stage::getMousePos() {
	return g_system->getEventManager()->getMousePos() - Common::Point(_dims.left, _dims.top);
}

bool Stage::step() {
	bool loop = false;

	if (_currentMovie) {
		debug(0, "\n@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@");
		debug(0, "@@@@   Movie name '%s' in '%s'", _currentMovie->getMacName().c_str(), _currentPath.c_str());
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

			_currentMovie->getScore()->startLoop();

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
		loop = true;
	}

	return loop;
}

} // end of namespace Director
