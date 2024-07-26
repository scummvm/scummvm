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

#include "graphics/cursorman.h"

#include "bagel/baglib/fmovie.h"
#include "bagel/boflib/string.h"
#include "bagel/boflib/gfx/palette.h"
#include "bagel/boflib/app.h"
#include "bagel/baglib/master_win.h"
#include "bagel/baglib/pan_window.h"
#include "bagel/boflib/std_keys.h"

namespace Bagel {


CBagFMovie::CBagFMovie(CBofWindow *parent, const char *filename, CBofRect *bounds, bool useNewPalette, bool blackOutWindow) {
	// Allow movie to not shift to new palette.
	_useNewPaletteFl = useNewPalette;

	// Black out first and last frame of flythroughs and examine movies
	_blackOutWindowFl = blackOutWindow;

	CBagFMovie::initialize(parent);
	CBagFMovie::open(filename, bounds);
}

CBagFMovie::~CBagFMovie() {
	CBagFMovie::closeMovie();
}

ErrorCode CBagFMovie::initialize(CBofWindow *pParent) {
	// Movie Stuff
	_movieStatus = MOVIE_STOPPED;
	_escCanStopFl = true;
	_smackerPal = nullptr;
	_bmpBuf = nullptr;
	_filterBmp = nullptr;
	_bufferStart = nullptr;
	_bufferLength = 0;

	// Smacker Stuff
	_smk = nullptr;
	_loopFl = false;

	// Call dialog box creates
	if (create("MovieWin", 0, 0, 1, 1, pParent, 1) == ERR_NONE) {
		setCapture();
	}

	return ERR_NONE;
}

bool CBagFMovie::open(const char *filename, CBofRect *bounds) {
	// No filename, so put up an open file box
	if (filename == nullptr) {
		assert(filename);
		return false;
	}

	if (bounds != nullptr) {
		_cRect = *bounds;
	}

	if (openMovie(filename)) {
		// We were given specific rect for movie
		if (bounds)
			reSize(bounds, true);
		else
			// Center the movie to the parent window
			centerRect();

		// Paint the image to the screen.
		_filterBmp->paint(this, 0, 0);

		return true;
	}

	return false;
}

bool CBagFMovie::openMovie(const char *sFilename) {
	assert(sFilename[0] != '\0');

	if (_smk) {
		closeMovie();
	}
	_smk = new Video::SmackerDecoder();
	_smk->setSoundType(Audio::Mixer::kSFXSoundType);

	// Opened failed ?
	if (!_smk->loadFile(sFilename)) {
		error("Movie not found=%s", sFilename);
	}


	// Allocate the bitmaps.
	HPALETTE pal;
	_smackerPal = new CBofPalette(pal);

	_bmpBuf = new CBofBitmap(_smk->getWidth(), _smk->getHeight(), _smackerPal, false);

	_filterBmp = new CBofBitmap(_smk->getWidth(), _smk->getHeight(), _smackerPal, false);
	_filterBmp->lock();

	selectPalette(_smackerPal);

	if (_bmpBuf) {
		_bmpBuf->lock();
		_bmpBuf->fillRect(nullptr, _smackerPal->getNearestIndex(CTEXT_WHITE));

		_bufferStart = (char *)_bmpBuf->getPixelAddress(0, _bmpBuf->isTopDown() ? 0 : (_bmpBuf->height() - 1));
		_bufferLength = ABS(_bmpBuf->height() * _bmpBuf->width());

		const Graphics::Surface *frame = _smk->decodeNextFrame();
		_smackerPal->setData(_smk->getPalette());
		if (frame) {
			_bmpBuf->getSurface().blitFrom(*frame);
		}
	}
	const bool repaintFl = true;

	_bounds = CBofRect(0, 0, (uint16)_bmpBuf->width() - 1, (uint16)_bmpBuf->height() - 1);
	reSize(&_bounds, repaintFl);

	CBagMasterWin *curWin = CBagel::getBagApp()->getMasterWnd();
	if (curWin != nullptr) {
		CBagStorageDevWnd *curSDev = curWin->getCurrentStorageDev();
		if ((curSDev != nullptr) && curSDev->isFiltered()) {
			const uint16 filterId = curSDev->getFilterId();
			const FilterFunction filterFunction = curSDev->getFilter();
			_bmpBuf->paint(_filterBmp);
			(*filterFunction)(filterId, _filterBmp, &_bounds);
		}
	}

	return true;
}

void CBagFMovie::onKeyHit(uint32 keyCode, uint32 /* repCount */) {
	if (_escCanStopFl && keyCode == BKEY_ESC) {
		// Clean up and exit
		_loopFl = false;
		stop();
		onMovieDone();
	}
}

void CBagFMovie::onMainLoop() {
	if (!_smk->needsUpdate() || (_movieStatus == MOVIE_STOPPED))
		return;

	
	// Smack the current frame into the buffer
	const Graphics::Surface *frame = _smk->decodeNextFrame();
	if (_smk->hasDirtyPalette()) {
		_smackerPal->setData(_smk->getPalette());
	}
	if (frame) {
		_bmpBuf->getSurface().blitFrom(*frame);
	}

	_bmpBuf->paint1To1(_filterBmp);

	// Filter the bitmap.
	CBagMasterWin *curWin = CBagel::getBagApp()->getMasterWnd();
	if (curWin != nullptr) {
		CBagStorageDevWnd *curSDev = curWin->getCurrentStorageDev();
		if ((curSDev != nullptr) && curSDev->isFiltered()) {
			const uint16 filterId = curSDev->getFilterId();
			const FilterFunction filterFunction = curSDev->getFilter();
			(*filterFunction)(filterId, _filterBmp, &_bounds);
		}
	}

	// Paint the buffer to the screen.
	_filterBmp->paint(this, 0, 0);

	if (_movieStatus == MOVIE_FORWARD) {
		if (_smk->getCurFrame() == (int)_smk->getFrameCount() - 1) {
			if (_loopFl == false) {
				onMovieDone();
			} else {
				seekToStart();
				_smk->start();
			}
		}
	} else if (_movieStatus == MOVIE_REVERSE) {
		if (_smk->getCurFrame() == 0 || _smk->getCurFrame() == 1) {
			if (_loopFl == false) {
				onMovieDone();
			} else {
				seekToEnd();
			}
		} else {
			setFrame(_smk->getCurFrame() - 2); // HACK: Reverse playback
		}
	}
}

void CBagFMovie::onPaint(CBofRect *) {
}

void CBagFMovie::closeMovie() {
	delete _smk;
	_smk = nullptr;

	if (_filterBmp != nullptr) {
		_filterBmp->unlock();
		delete _filterBmp;
		_filterBmp = nullptr;
	}

	if (_bmpBuf != nullptr) {
		_bmpBuf->unlock();
		delete _bmpBuf;
		_bmpBuf = nullptr;
	}

	delete _smackerPal;
	_smackerPal = nullptr;

	_bufferStart = nullptr;
	_bufferLength = 0;
}


void CBagFMovie::onClose() {
	closeMovie();
	CBofDialog::onClose();
}


void CBagFMovie::onMovieDone() {
	if (!_loopFl) {
		if (_bCaptured)
			releaseCapture();

		getParent()->enable();
		_bEndDialog = true;
	}
}


bool CBagFMovie::play(bool loop, bool escCanStop) {
	_escCanStopFl = escCanStop;
	_loopFl = loop;

	const bool retVal = play();

	getParent()->disable();
	getParent()->flushAllMessages();

	CursorMan.showMouse(false);

	doModal();

	CursorMan.showMouse(true);

	return retVal;
}


bool CBagFMovie::play() {
	if (!_smk)
		return false;

	_smk->pauseVideo(false);
	// _smk->setReverse(false); // TODO: Not supported by SMK
	_smk->start();
	_movieStatus = MOVIE_FORWARD;
	return true;
}

bool CBagFMovie::reverse(bool loop, bool escCanStop) {
	_escCanStopFl = escCanStop;
	_loopFl = loop;

	const bool retVal = reverse();

	getParent()->disable();
	getParent()->flushAllMessages();
	doModal();

	return retVal;
}

bool CBagFMovie::reverse() {
	if (!_smk)
		return false;

	_smk->pauseVideo(false);
	// _smk->setReverse(true); // TODO: Not supported by SMK
	_smk->start();
	_movieStatus = MOVIE_REVERSE;
	return true;
}

bool CBagFMovie::stop() {
	if (!_smk)
		return false;

	_smk->stop();
	_movieStatus = MOVIE_STOPPED;
	return true;
}

bool CBagFMovie::pause() {
	if (!_smk)
		return false;

	_smk->pauseVideo(true);
	_movieStatus = MOVIE_PAUSED;
	return true;

}

bool CBagFMovie::seekToStart() {
	if (!_smk)
		return false;

	_smk->rewind();
	return true;
}

bool CBagFMovie::seekToEnd() {
	if (!_smk)
		return false;

	setFrame(_smk->getFrameCount() - 2); // HACK: Reverse rewind
	return true;
}

uint32 CBagFMovie::getFrame() {
	if (_smk) {
		return _smk->getCurFrame();
	}

	return (uint32) -1;
}

bool CBagFMovie::setFrame(uint32 frameNum) {
	if (!_smk)
		return false;

	frameNum = CLIP<uint32>(frameNum, 0, _smk->getFrameCount() - 1);
	_smk->forceSeekToFrame(frameNum);
	return true;
}

bool CBagFMovie::centerRect() {
	CBofRect clientRect = getParent()->getClientRect();
	const RECT parentRect = clientRect.getWinRect();
	const int clientWidth = parentRect.right - parentRect.left;
	const int clientHeight = parentRect.bottom - parentRect.top;

	// Get Movies width and height
	const int movieWidth = _smk->getWidth();
	const int movieHeight = _smk->getHeight();

	RECT movieBounds;
	movieBounds.left = (clientWidth - movieWidth) / 2;
	movieBounds.top = (clientHeight - movieHeight) / 2;
	movieBounds.right = movieBounds.left + movieWidth;
	movieBounds.bottom = movieBounds.top + movieHeight;

	// Reposition the playback window
	clientRect = movieBounds;
	reSize(&clientRect, true);

	return true;
}

} // namespace Bagel
