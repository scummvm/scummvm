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

#include "bagel/baglib/fmovie.h"
#include "bagel/boflib/string.h"
#include "bagel/boflib/gfx/palette.h"
#include "bagel/boflib/app.h"
#include "bagel/baglib/master_win.h"
#include "bagel/baglib/pan_window.h"
#include "bagel/boflib/misc.h"
#include "bagel/boflib/std_keys.h"

namespace Bagel {


CBagFMovie::CBagFMovie(CBofWindow *pParent, const char *sFilename, CBofRect *pBounds, bool bUseNewPalette, bool bBlackOutWindow) {
	// Allow movie to not shift to new palette.
	m_bUseNewPalette = bUseNewPalette;

	// Black out first and last frame of flythroughs and examine movies
	m_bBlackOutWindow = bBlackOutWindow;

	initialize(pParent);
	open(sFilename, pBounds);
}

CBagFMovie::~CBagFMovie() {
	closeMovie();
}

ErrorCode CBagFMovie::initialize(CBofWindow *pParent) {
	// Movie Stuff
	m_eMovStatus = STOPPED;
	m_bEscCanStop = true;
	m_pSmackerPal = nullptr;
	m_pBmpBuf = nullptr;
	m_pFilterBmp = nullptr;
	m_pBufferStart = nullptr;
	m_nBufferLength = 0;

	// Smacker Stuff
	m_pSbuf = nullptr;
	m_pSmk = nullptr;
	m_bLoop = false;

	// Call dialog box creates
	if (create("MovieWin", 0, 0, 1, 1, pParent, 1) == ERR_NONE) {
		SetCapture();
	}

	return ERR_NONE;
}

bool CBagFMovie::open(const char *sFilename, CBofRect *pBounds) {
	// No filename, so put up an open file box
	if (sFilename == nullptr) {
		Assert(sFilename);
		return false;
	}

	if (pBounds != nullptr) {
		_cRect = *pBounds;
	}

	if (openMovie(sFilename)) {
		// We were given specific rect for movie
		if (pBounds)
			ReSize(pBounds, true);
		else
			// Center the movie to the parent window
			centerRect();

		// Paint the image to the screen.
		m_pFilterBmp->paint(this, 0, 0);

		return true;
	}

	return false;
}

bool CBagFMovie::openMovie(const char *sFilename) {
	Assert(sFilename[0] != '\0');

	if (m_pSmk) {
		closeMovie();
	}
	m_pSmk = new Video::SmackerDecoder();
	m_pSmk->setSoundType(Audio::Mixer::kSFXSoundType);

	// Opened failed ?
	if (!m_pSmk->loadFile(sFilename)) {
		error("Movie not found=%s", sFilename);
		return false;
	}


	// Allocate the bitmaps.
	HPALETTE hPalette;
	m_pSmackerPal = new CBofPalette(hPalette);

	m_pBmpBuf = new CBofBitmap(m_pSmk->getWidth(), m_pSmk->getHeight(), m_pSmackerPal, false);

	m_pFilterBmp = new CBofBitmap(m_pSmk->getWidth(), m_pSmk->getHeight(), m_pSmackerPal, false);
	m_pFilterBmp->Lock();

	SelectPalette(m_pSmackerPal);

	if (m_pBmpBuf) {
		m_pBmpBuf->Lock();
		m_pBmpBuf->FillRect(nullptr, m_pSmackerPal->GetNearestIndex(RGB(255, 255, 255)));

		m_nReversed = !(m_pBmpBuf->IsTopDown());
		m_pBufferStart = (char *)m_pBmpBuf->GetPixelAddress(0, m_nReversed * (m_pBmpBuf->Height() - 1));
		m_nBufferLength = ABS(m_pBmpBuf->Height() * m_pBmpBuf->Width());

		const Graphics::Surface *frame = m_pSmk->decodeNextFrame();
		m_pSmackerPal->SetData(m_pSmk->getPalette());
		if (frame) {
			m_pBmpBuf->getSurface().blitFrom(*frame);
		}
	}
	bool bRepaint = true;

	m_xBounds = CBofRect(0, 0, (uint16)m_pBmpBuf->Width() - 1, (uint16)m_pBmpBuf->Height() - 1);
	ReSize(&m_xBounds, bRepaint);

	CBagMasterWin *pWnd = CBagel::getBagApp()->getMasterWnd();
	if (pWnd != nullptr) {
		CBagStorageDevWnd *pSDevWnd = pWnd->GetCurrentStorageDev();
		if (pSDevWnd != nullptr) {
			if (pSDevWnd->IsFiltered()) {
				uint16 nFilterId = pSDevWnd->GetFilterId();
				FilterFunction pFilterFunction = pSDevWnd->GetFilter();
				m_pBmpBuf->paint(m_pFilterBmp);
				(*pFilterFunction)(nFilterId, m_pFilterBmp, &m_xBounds);
			}
		}
	}

	return true;
}

void CBagFMovie::onKeyHit(uint32 lKey, uint32 /*lRepCount*/) {
	if (m_bEscCanStop && lKey == BKEY_ESC) {
		// Clean up and exit
		m_bLoop = false;
		stop();
		onMovieDone();
	}
}

void CBagFMovie::onMainLoop() {
	if (m_pSmk->needsUpdate()) {
		// Not needed for filtered movies
		if (m_eMovStatus != STOPPED) {
			// Smack the current frame into the buffer
			const Graphics::Surface *frame = m_pSmk->decodeNextFrame();
			if (m_pSmk->hasDirtyPalette()) {
				m_pSmackerPal->SetData(m_pSmk->getPalette());
			}
			if (frame) {
				m_pBmpBuf->getSurface().blitFrom(*frame);
			}

			m_pBmpBuf->Paint1To1(m_pFilterBmp);

			// Filter the bitmap.
			CBagMasterWin *pWnd = CBagel::getBagApp()->getMasterWnd();
			if (pWnd != nullptr) {
				CBagStorageDevWnd *pSDevWnd = pWnd->GetCurrentStorageDev();
				if ((pSDevWnd != nullptr) && pSDevWnd->IsFiltered()) {
					uint16 nFilterId = pSDevWnd->GetFilterId();
					FilterFunction pFilterFunction = pSDevWnd->GetFilter();
					(*pFilterFunction)(nFilterId, m_pFilterBmp, &m_xBounds);
				}
			}

			// Paint the buffer to the screen.
			m_pFilterBmp->paint(this, 0, 0);

			if (m_eMovStatus == FOREWARD) {
				if (m_pSmk->getCurFrame() == (int)m_pSmk->getFrameCount() - 1) {
					if (m_bLoop == false) {
						onMovieDone();
					} else {
						seekToStart();
						m_pSmk->start();
					}
				}
			} else if ((m_eMovStatus == REVERSE) && ((m_pSmk->getCurFrame() == 0) || (m_pSmk->getCurFrame() == 1))) {
				if (m_bLoop == false) {
					onMovieDone();
				} else {
					seekToEnd();
				}
			} else {
				setFrame(m_pSmk->getCurFrame() - 2); // HACK: Reverse playback
			}

		}// !STOPPED
	} // !SMACKWAIT
}

void CBagFMovie::onPaint(CBofRect *) {
}

void CBagFMovie::closeMovie() {
	if (m_pSbuf != nullptr) {
		delete m_pSbuf;
		m_pSbuf = nullptr;
	}

	if (m_pSmk != nullptr) {
		delete m_pSmk;
		m_pSmk = nullptr;
	}

	if (m_pFilterBmp != nullptr) {
		m_pFilterBmp->UnLock();
		delete m_pFilterBmp;
		m_pFilterBmp = nullptr;
	}

	if (m_pBmpBuf != nullptr) {
		m_pBmpBuf->UnLock();
		delete m_pBmpBuf;
		m_pBmpBuf = nullptr;
	}

	if (m_pSmackerPal != nullptr) {
		delete m_pSmackerPal;
		m_pSmackerPal = nullptr;
	}

	m_pBufferStart = nullptr;
	m_nBufferLength = 0;
}


void CBagFMovie::onClose() {
	closeMovie();
	CBofDialog::onClose();
}


void CBagFMovie::onMovieDone() {
	if (!m_bLoop) {
		if (_bCaptured)
			ReleaseCapture();

		GetParent()->enable();
		_bEndDialog = true;
	}
}


bool CBagFMovie::play(bool bLoop, bool bEscCanStop) {
	m_bEscCanStop = bEscCanStop;
	m_bLoop = bLoop;

	bool bSuccess = play();

	GetParent()->disable();
	GetParent()->FlushAllMessages();

	CursorMan.showMouse(false);

	doModal();

	CursorMan.showMouse(true);

	return bSuccess;
}


bool CBagFMovie::play() {
	if (m_pSmk) {
		m_pSmk->pauseVideo(false);
		// m_pSmk->setReverse(false); // TODO: Not supported by SMK
		m_pSmk->start();
		m_eMovStatus = FOREWARD;
		return true;
	}

	return false;
}

bool CBagFMovie::reverse(bool bLoop, bool bEscCanStop) {
	m_bEscCanStop = bEscCanStop;
	m_bLoop = bLoop;

	bool bSuccess = reverse();

	GetParent()->disable();
	GetParent()->FlushAllMessages();
	doModal();

	return bSuccess;
}

bool CBagFMovie::reverse() {
	if (m_pSmk) {
		m_pSmk->pauseVideo(false);
		// m_pSmk->setReverse(true); // TODO: Not supported by SMK
		m_pSmk->start();
		m_eMovStatus = REVERSE;
		return true;
	}

	return false;

}

bool CBagFMovie::stop() {
	if (m_pSmk) {
		m_pSmk->stop();
		m_eMovStatus = STOPPED;
		return true;
	}
	return false;

}

bool CBagFMovie::pause() {
	if (m_pSmk) {
		m_pSmk->pauseVideo(true);
		m_eMovStatus = PAUSED;
		return true;
	}

	return false;

}

bool CBagFMovie::seekToStart() {
	if (m_pSmk) {
		m_pSmk->rewind();
		return true;
	}

	return false;

}

bool CBagFMovie::seekToEnd() {
	if (m_pSmk) {
		setFrame(m_pSmk->getFrameCount() - 2); // HACK: Reverse rewind
		return true;
	}

	return false;

}

uint32 CBagFMovie::getFrame() {
	if (m_pSmk) {
		return m_pSmk->getCurFrame();
	}

	return (uint32) -1;
}

bool CBagFMovie::setFrame(uint32 dwFrameNum) {
	if (m_pSmk) {
		dwFrameNum = CLIP<uint32>(dwFrameNum, 0, m_pSmk->getFrameCount() - 1);
		m_pSmk->forceSeekToFrame(dwFrameNum);
		return true;
	}

	return false;
}

bool CBagFMovie::centerRect() {
	CBofRect cBofRect = GetParent()->GetClientRect();
	RECT rcParentRect = cBofRect.GetWinRect();
	int ClientWidth = rcParentRect.right - rcParentRect.left;
	int ClientHeight = rcParentRect.bottom - rcParentRect.top;

	// Get Movies width and height
	int MovieWidth = m_pSmk->getWidth();
	int MovieHeight = m_pSmk->getHeight();

	RECT rcMovieBounds;
	rcMovieBounds.left = (ClientWidth - MovieWidth) / 2;
	rcMovieBounds.top = (ClientHeight - MovieHeight) / 2;
	rcMovieBounds.right = rcMovieBounds.left + MovieWidth;
	rcMovieBounds.bottom = rcMovieBounds.top + MovieHeight;

	// Reposition the playback window
	cBofRect = rcMovieBounds;
	ReSize(&cBofRect, true);

	return true;
}

void CBagFMovie::OnButtonUp(uint32 /*nFlags*/, CBofPoint * /*pPoint*/) {
}

} // namespace Bagel
