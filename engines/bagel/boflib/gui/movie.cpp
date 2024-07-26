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
 * along with this program.  If not, see <http://www.gnu.org/licenses/".
 *
 */

#include "graphics/cursorman.h"

#include "bagel/boflib/string.h"
#include "bagel/boflib/gui/movie.h"
#include "bagel/boflib/gfx/cursor.h"
#include "bagel/boflib/std_keys.h"

namespace Bagel {

CBofMovie::CBofMovie(CBofWindow *pParent, const char *pszFilename, CBofRect *pBounds, bool bStretch, bool bUseNewPalette, bool bBlackOutWindow) {
	_bStretch = bStretch;

	// Allow movie to not shift to new palette.
	_bUseNewPalette = bUseNewPalette;

	// Black out first and last frame of flythroughs and examine movies
	_bBlackOutWindow = bBlackOutWindow;

	initialize(pParent);
	open(pszFilename, pBounds);
}

CBofMovie::~CBofMovie() {
	closeMovie();
}

ErrorCode CBofMovie::initialize(CBofWindow *pParent) {
	// Movie Stuff
	_eMovStatus = STOPPED;
	_bEscCanStop = true;

	// Smacker Stuff
	_pSbuf = nullptr;
	_pSmk = nullptr;
	_bLoop = false;

	// Call dialog box creates
	if (create("MovieWin", 0, 0, 1, 1, pParent, 1) == ERR_NONE) {
		setCapture();
	}

	return ERR_NONE;

}

bool CBofMovie::open(const char *sFilename, CBofRect *pBounds) {
	if (sFilename == nullptr) {
		assert(sFilename);
		return false;
	}

	if (pBounds != nullptr) {
		_cRect = *pBounds;
	}

	if (openMovie(sFilename)) {
		// We were given specific rect for movie
		if (pBounds)
			reSize(pBounds, true);
		else
			// Center the movie to the parent window
			centerRect();

		return true;
	}

	return false;
}

bool CBofMovie::openMovie(const char *sFilename) {
	assert(sFilename[0] != '\0');

	if (_pSmk) {
		closeMovie();
	}
	_pSmk = new Video::SmackerDecoder();
	_pSmk->setSoundType(Audio::Mixer::kSFXSoundType);

	if (!_pSmk->loadFile(sFilename)) {
		// Opened failed
		error("Movie not found=%s", sFilename);
	}

	// If supposed to stretch into specified window
	if (_bStretch) {
		_pSbuf = new Graphics::ManagedSurface(width(), height(), _pSmk->getPixelFormat());
	} else {
		_pSbuf = new Graphics::ManagedSurface(_pSmk->getWidth(), _pSmk->getHeight(), _pSmk->getPixelFormat());
	}

	_srcRect = Common::Rect(_pSmk->getWidth(), _pSmk->getHeight());
	_dstRect = Common::Rect(_pSbuf->w, _pSbuf->h);

	if (!_bStretch) {
		_dstRect.moveTo((_pSbuf->w - _pSmk->getWidth()) / 2, (_pSbuf->h - _pSmk->getHeight()) / 2);
	}

	CBofRect MovieBounds(0, 0, (uint16)_pSbuf->w - 1, (uint16)_pSbuf->h - 1);
	reSize(&MovieBounds, true);

	// If we have a window that is going to cause a single frame
	// palette shift, then black it out here.
	if (_bBlackOutWindow) {
		fillWindow(COLOR_BLACK);
	}

	// Smack the current frame into the buffer
	const Graphics::Surface *frame = _pSmk->decodeNextFrame();
	if (frame) {
		_pSbuf->setPalette(_pSmk->getPalette(), 0, 256);
		_pSbuf->blitFrom(*frame, _srcRect, _dstRect);
	}

	return true;
}

void  CBofMovie::onKeyHit(uint32 lKey, uint32 /*lRepCount*/) {
	if (_bEscCanStop && lKey == BKEY_ESC) {
		// Clean up and exit
		_bLoop = false;
		stop();
		onMovieDone();
	}
}

void  CBofMovie::onMainLoop() {
	if (!_pSmk->needsUpdate() || _eMovStatus == STOPPED)
		return;

	// Smack the current frame into the buffer
	const Graphics::Surface *frame = _pSmk->decodeNextFrame();
	if (_pSmk->hasDirtyPalette()) {
		_pSbuf->setPalette(_pSmk->getPalette(), 0, 256);
	}
	if (frame) {
		_pSbuf->blitFrom(*frame, _srcRect, _dstRect);
		updateWindow();
	}

	if (_eMovStatus == FORWARD) {
		if (_pSmk->getCurFrame() == (int)_pSmk->getFrameCount() - 1) {
			if (_bLoop == false) {
				onMovieDone();
			} else {
				seekToStart();
				_pSmk->start();
			}
		}
	} else if (_eMovStatus == REVERSE) {
		if ((_pSmk->getCurFrame() == 0) || (_pSmk->getCurFrame() == 1)) {
			if (_bLoop == false) {
				onMovieDone();
			} else {
				seekToEnd();
			}
		} else {
			setFrame(_pSmk->getCurFrame() - 2); // HACK: Reverse playback
		}
	}// MOVIE_REVERSE
}

void  CBofMovie::onPaint(CBofRect *) {
	if (_pSbuf) {
		getSurface()->blitFrom(*_pSbuf);
	}
}

void  CBofMovie::closeMovie() {
	delete _pSbuf;
	_pSbuf = nullptr;

	delete _pSmk;
	_pSmk = nullptr;
}

void  CBofMovie::onClose() {
	closeMovie();
	CBofDialog::onClose();
}

void CBofMovie::onMovieDone() {
	if (!_bLoop) {
		if (_bCaptured)
			releaseCapture();

		getParent()->enable();
		_bEndDialog = true;
	}
}

bool CBofMovie::play(bool bLoop, bool bEscCanStop) {

	_bEscCanStop = bEscCanStop;
	_bLoop = bLoop;

	bool bSuccess = play();

	getParent()->disable();
	getParent()->flushAllMessages();

	CursorMan.showMouse(false);

	doModal();

	CursorMan.showMouse(true);

	return bSuccess;
}


bool CBofMovie::play() {
	if (_pSmk) {
		_pSmk->pauseVideo(false);
		//_pSmk->setReverse(false); // TODO: Not supported by SMK
		_pSmk->start();
		_eMovStatus = FORWARD;
		return true;
	}

	return false;

}

bool CBofMovie::reverse(bool bLoop,  bool bEscCanStop) {
	_bEscCanStop   = bEscCanStop;
	_bLoop = bLoop;

	bool bSuccess = reverse();

	getParent()->disable();
	getParent()->flushAllMessages();
	doModal();

	return bSuccess;

}

bool CBofMovie::reverse() {

	if (_pSmk) {
		_pSmk->pauseVideo(false);
		//_smk->setReverse(true); // TODO: Not supported by SMK
		_pSmk->start();
		_eMovStatus = REVERSE;
		return true;
	}

	return false;

}

bool CBofMovie::stop() {
	if (_pSmk) {
		_pSmk->stop();
		_eMovStatus = STOPPED;
		return true;
	}

	return false;

}

bool CBofMovie::pause() {
	if (_pSmk) {
		_pSmk->pauseVideo(true);
		_eMovStatus = PAUSED;
		return true;
	}

	return false;

}

bool CBofMovie::seekToStart() {
	if (_pSmk) {
		_pSmk->rewind();
		return true;
	}

	return false;
}

bool CBofMovie::seekToEnd() {
	if (_pSmk) {
		setFrame(_pSmk->getFrameCount() - 2); // HACK: Reverse rewind
		return true;
	}

	return false;
}

uint32 CBofMovie::getFrame() {
	if (_pSmk) {
		return _pSmk->getCurFrame();
	}

	return (uint32) -1;
}


bool CBofMovie::setFrame(uint32 dwFrameNum) {
	if (_pSmk) {
		dwFrameNum = CLIP<uint32>(dwFrameNum, 0, _pSmk->getFrameCount() - 1);
		_pSmk->forceSeekToFrame(dwFrameNum);
		return true;
	}

	return false;
}

void CBofMovie::onReSize(CBofSize *pSize) {
}

bool CBofMovie::centerRect() {
	CBofRect cBofRect = getParent()->getClientRect();
	RECT rcParentRect = cBofRect.getWinRect();
	int ClientWidth = rcParentRect.right - rcParentRect.left;
	int ClientHeight = rcParentRect.bottom - rcParentRect.top;

	// Get Movies width and height
	int MovieWidth = _pSmk->getWidth();
	int MovieHeight = _pSmk->getHeight();

	RECT rcMovieBounds;
	rcMovieBounds.left = (ClientWidth - MovieWidth) / 2;
	rcMovieBounds.top = (ClientHeight - MovieHeight) / 2;
	rcMovieBounds.right = rcMovieBounds.left + MovieWidth;
	rcMovieBounds.bottom = rcMovieBounds.top + MovieHeight;

	// Reposition the playback window
	cBofRect = rcMovieBounds;
	reSize(&cBofRect, true);

	return true;
}

void CBofMovie::onButtonUp(uint32 /*nFlags*/, CBofPoint * /*pPoint*/) {
}


ErrorCode bofPlayMovie(CBofWindow *pParent, const char *pszMovieFile, CBofRect *pRect) {
	assert(pParent != nullptr);
	assert(pszMovieFile != nullptr);

	CBofMovie cMovie(pParent, pszMovieFile, pRect);
	if (!cMovie.errorOccurred()) {
		cMovie.play(false, true);
	}

	return cMovie.getErrorCode();
}

} // namespace Bagel
