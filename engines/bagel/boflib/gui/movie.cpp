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

#include "bagel/boflib/string.h"
#include "bagel/boflib/gui/movie.h"
#include "bagel/boflib/gfx/cursor.h"

namespace Bagel {

CBofMovie::CBofMovie(CBofWindow *pParent, const char *pszFilename, CBofRect *pBounds, bool bStretch, bool bUseNewPalette, bool bBlackOutWindow) {
	m_bStretch = bStretch;

	// allow movie to not shift to new palette.
	m_bUseNewPalette = bUseNewPalette;

	// black out first and last frame of flythroughs and examine movies
	m_bBlackOutWindow = bBlackOutWindow;

	initialize(pParent);
	Open(pszFilename, pBounds);
}

CBofMovie::~CBofMovie() {
	CloseMovie();
}

ERROR_CODE CBofMovie::initialize(CBofWindow *pParent) {
	// Movie Stuff
	m_eMovStatus = STOPPED;
	m_bEscCanStop = true;

	// Smacker Stuff
	m_pSbuf = nullptr;
	m_pSmk = nullptr;
	m_bLoop = false;

	// Call dialog box creates
	if (Create("MovieWin", 0, 0, 1, 1, pParent, 1) == ERR_NONE) {
		SetCapture();
	}

	return ERR_NONE;

}

bool CBofMovie::Open(const char *sFilename, CBofRect *pBounds) {
	if (sFilename == nullptr) {
		Assert(sFilename);
		return false;
	}

	if (pBounds != nullptr) {
		m_cRect = *pBounds;
	}

	if (OpenMovie(sFilename)) {

		// We were given specific rect for movie
		if (pBounds)
			ReSize(pBounds, true);
		else // Center the movie to the parent window
			CenterRect();

		return true;
	}

	return false;
}

bool CBofMovie::OpenMovie(const char *sFilename) {

	Assert(sFilename[0] != '\0');

	if (m_pSmk) {
		CloseMovie();
	}
	m_pSmk = new Video::SmackerDecoder();

	if (!m_pSmk->loadFile(sFilename)) {
		// Opened failed
		error("Movie not found=%s", sFilename);
		return false;
	}

	// If supposed to stretch into specified window
	//
	if (m_bStretch) {
		m_pSbuf = new Graphics::ManagedSurface(Width(), Height(), m_pSmk->getPixelFormat());
	} else {
		m_pSbuf = new Graphics::ManagedSurface(m_pSmk->getWidth(), m_pSmk->getHeight(), m_pSmk->getPixelFormat());
	}
	_srcRect = Common::Rect(m_pSmk->getWidth(), m_pSmk->getHeight());
	_dstRect = Common::Rect(m_pSbuf->w, m_pSbuf->h);
	if (!m_bStretch) {
		_dstRect.moveTo((m_pSbuf->w - m_pSmk->getWidth()) / 2, (m_pSbuf->h - m_pSmk->getHeight()) / 2);
	}

	CBofRect MovieBounds(0, 0, (uint16)m_pSbuf->w - 1, (uint16)m_pSbuf->h - 1);
	ReSize(&MovieBounds, true);

	// if we have a window that is going to cause a single frame
	// palette shift, then black it out here.
	if (m_bBlackOutWindow) {
		FillWindow(COLOR_BLACK);
	}

	// Smack the current frame into the buffer
	const Graphics::Surface *frame = m_pSmk->decodeNextFrame();
	if (frame) {
		m_pSbuf->setPalette(m_pSmk->getPalette(), 0, 256);
		m_pSbuf->blitFrom(*frame, _srcRect, _dstRect);
	}

	return true;
}

void  CBofMovie::OnKeyHit(uint32 lKey, uint32 /*lRepCount*/) {
	if (m_bEscCanStop && lKey == BKEY_ESC) {
		// Clean up and exit
		m_bLoop = false;
		Stop();
		OnMovieDone();
	}
}

void  CBofMovie::OnMainLoop() {
	if (m_pSmk->needsUpdate()) {
		if (m_eMovStatus != STOPPED) {
			// Smack the current frame into the buffer
			const Graphics::Surface *frame = m_pSmk->decodeNextFrame();
			if (m_pSmk->hasDirtyPalette()) {
				m_pSbuf->setPalette(m_pSmk->getPalette(), 0, 256);
			}
			if (frame) {
				m_pSbuf->blitFrom(*frame, _srcRect, _dstRect);
				UpdateWindow();
			}

			if (m_eMovStatus == FOREWARD) {
				if (m_pSmk->getCurFrame() == (int)m_pSmk->getFrameCount() - 1) {
					if (m_bLoop == false) {
						OnMovieDone();
					} else {
						SeekToStart();
						m_pSmk->start();
					}
				}
			} else if (m_eMovStatus == REVERSE) {
				if ((m_pSmk->getCurFrame() == 0) || (m_pSmk->getCurFrame() == 1)) {
					if (m_bLoop == false) {
						OnMovieDone();
					} else {
						SeekToEnd();
						//m_pSmk->start();
					}
				} else {
					SetFrame(m_pSmk->getCurFrame() - 2); // HACK: Reverse playback
				}
			}// REVERSE
		}// !STOPPED
	}// !SMACKWAIT
}

void  CBofMovie::OnPaint(CBofRect *) {
	if (m_pSbuf) {
		getSurface()->blitFrom(*m_pSbuf);
	}
}

void  CBofMovie::CloseMovie() {
	if (m_pSbuf) {
		delete m_pSbuf;
		m_pSbuf = nullptr;
	}

	if (m_pSmk) {
		delete m_pSmk;
		m_pSmk = nullptr;
	}
}

void  CBofMovie::OnClose() {

	CloseMovie();
	CBofDialog::OnClose();

}

void CBofMovie::OnMovieDone() {
	if (!m_bLoop) {
		if (m_bCaptured)
			ReleaseCapture();

		GetParent()->Enable();
		_bEndDialog = true;
	}
}

bool CBofMovie::Play(bool bLoop, bool bEscCanStop) {
	bool bSuccess;

	m_bEscCanStop = bEscCanStop;
	m_bLoop = bLoop;

	bSuccess = Play();

	GetParent()->Disable();
	GetParent()->FlushAllMessages();

	CBofCursor::Hide();

	DoModal();

	CBofCursor::Show();

	return bSuccess;
}


bool CBofMovie::Play() {

	if (m_pSmk) {
		m_pSmk->pauseVideo(false);
		//m_pSmk->setReverse(false); // TODO: Not supported by SMK
		m_pSmk->start();
		m_eMovStatus = FOREWARD;
		return true;
	}

	return false;

}

bool CBofMovie::Reverse(bool bLoop,  bool bEscCanStop) {
	bool bSuccess = true;

	m_bEscCanStop   = bEscCanStop;
	m_bLoop = bLoop;

	bSuccess = Reverse();

	GetParent()->Disable();
	GetParent()->FlushAllMessages();
	DoModal();

	return bSuccess;

}

bool CBofMovie::Reverse() {

	if (m_pSmk) {
		m_pSmk->pauseVideo(false);
		//m_pSmk->setReverse(true); // TODO: Not supported by SMK
		m_pSmk->start();
		m_eMovStatus = REVERSE;
		return true;
	}

	return false;

}

bool CBofMovie::Stop() {

	if (m_pSmk) {
		m_pSmk->stop();
		m_eMovStatus = STOPPED;
		return true;
	}

	return false;

}

bool CBofMovie::Pause() {

	if (m_pSmk) {
		m_pSmk->pauseVideo(true);
		m_eMovStatus = PAUSED;
		return true;
	}

	return false;

}

bool CBofMovie::SeekToStart() {
	if (m_pSmk) {
		m_pSmk->rewind();
		return true;
	}

	return false;

}

bool CBofMovie::SeekToEnd() {
	if (m_pSmk) {
		SetFrame(m_pSmk->getFrameCount() - 2); // HACK: Reverse rewind
		return true;
	}

	return false;

}

uint32 CBofMovie::GetFrame() {
	if (m_pSmk) {
		return m_pSmk->getCurFrame();
	}
	return (uint32) -1;
}


bool CBofMovie::SetFrame(uint32 dwFrameNum) {
	if (m_pSmk) {
		dwFrameNum = CLIP<uint32>(dwFrameNum, 0, m_pSmk->getFrameCount() - 1);
		m_pSmk->forceSeekToFrame(dwFrameNum);
		return true;
	}

	return false;
}

void CBofMovie::OnReSize(CBofSize *pSize) {
}

bool CBofMovie::CenterRect() {
	CBofRect            cBofRect;
	RECT                rcParentRect, rcMovieBounds;
	int                 ClientWidth, ClientHeight;
	int                 MovieWidth = 0;
	int                 MovieHeight = 0;

	cBofRect =      GetParent()->GetClientRect();
	rcParentRect =  cBofRect.GetWinRect();
	ClientWidth =   rcParentRect.right - rcParentRect.left;
	ClientHeight =  rcParentRect.bottom - rcParentRect.top;

	// Get Movies width and height
	MovieWidth =    m_pSmk->getWidth();
	MovieHeight =   m_pSmk->getHeight();

	rcMovieBounds.left = (ClientWidth - MovieWidth) / 2;
	rcMovieBounds.top = (ClientHeight - MovieHeight) / 2;
	rcMovieBounds.right = rcMovieBounds.left + MovieWidth;
	rcMovieBounds.bottom = rcMovieBounds.top + MovieHeight;

	// reposition the playback window
	cBofRect = rcMovieBounds;
	ReSize(&cBofRect, true);

	return true;

}

void CBofMovie::OnButtonUp(uint32 /*nFlags*/, CBofPoint * /*pPoint*/) {

}


ERROR_CODE BofPlayMovie(CBofWindow *pParent, const char *pszMovieFile, CBofRect *pRect) {
	Assert(pParent != nullptr);
	Assert(pszMovieFile != nullptr);

	CBofMovie cMovie(pParent, pszMovieFile, pRect);
	if (!cMovie.ErrorOccurred()) {
		cMovie.Play(false, true);
	}

	return cMovie.GetErrorCode();
}

} // namespace Bagel
