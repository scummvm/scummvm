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
#include "bagel/boflib/gfx/cursor.h"
#include "bagel/boflib/gfx/palette.h"
#include "bagel/boflib/app.h"
#include "bagel/boflib/app.h"
#include "bagel/baglib/pan_window.h"
#include "bagel/boflib/misc.h"

#include "video/smk_decoder.h"
#include "video/qt_decoder.h"

namespace Bagel {


CBagFMovie::CBagFMovie(CBofWindow *pParent, const char *sFilename, CBofRect *pBounds, BOOL bUseNewPalette, BOOL bBlackOutWindow) {
	// Allow movie to not shift to new palette.
	m_bUseNewPalette = bUseNewPalette;

	// Black out first and last frame of flythroughs and examine movies
	m_bBlackOutWindow = bBlackOutWindow;

	initialize(pParent);
	Open(sFilename, pBounds);
}

CBagFMovie::~CBagFMovie() {
	CloseMovie();
}

ERROR_CODE CBagFMovie::initialize(CBofWindow *pParent) {
	// Movie Stuff
	m_eMovStatus = STOPPED;
	m_eMovType = SMACKER;
	m_bEscCanStop = TRUE;
	m_pSmackerPal = nullptr;
	m_pBmpBuf = nullptr;
	m_pFilterBmp = nullptr;
	m_pBufferStart = nullptr;
	m_nBufferLength = 0;

	// Smacker Stuff
	m_pSbuf = nullptr;
	m_pSmk = nullptr;
	m_bLoop = FALSE;

	// Call dialog box creates
	if (Create("MovieWin", 0, 0, 1, 1, pParent, 1) == ERR_NONE) {
		SetCapture();
	}

	return ERR_NONE;
}

BOOL CBagFMovie::Open(const char *sFilename, CBofRect *pBounds) {
	// No filename, so put up an open file box
	if (sFilename == nullptr) {
		Assert(sFilename);
		return FALSE;
	}

	if (pBounds != nullptr) {
		m_cRect = *pBounds;
	}

	if (OpenMovie(sFilename)) {
		// We were given specific rect for movie
		if (pBounds)
			ReSize(pBounds, TRUE);
		else
			// Center the movie to the parent window
			CenterRect();

		return TRUE;
	}

	return FALSE;
}

BOOL CBagFMovie::OpenMovie(const char *sFilename) {
	BOOL bRepaint;
	Assert(sFilename[0] != '\0');

	// Get a non const version of the filename
	CBofString filename(sFilename);
	CBofString extension(filename.Right(3));

	extension.MakeUpper();

	if (extension == "SMK")
		m_eMovType = SMACKER;
	else
		m_eMovType = QT;

	if (m_pSmk) {
		CloseMovie();
	}

	if (m_eMovType == SMACKER) {
		m_pSmk = new Video::SmackerDecoder();
	} else {
		m_pSmk = new Video::QuickTimeDecoder();
	}

	// Opened failed ?
	//
	if (!m_pSmk->loadFile(filename.GetBuffer())) {
		error("Movie not found=%s", filename.GetBuffer());
		return FALSE;
	}

	// Create a Windows palette based on the smacker movie palette.
	// This palette that will be used when the offscreen bitmap is
	// created.
	//
	HPALETTE hPalette;
	//hPalette = WinPalFromSmkPal();

	// Allocate the bitmaps.
	//
	m_pSmackerPal = new CBofPalette(hPalette);

	m_pBmpBuf = new CBofBitmap(m_pSmk->getWidth(), m_pSmk->getHeight(), m_pSmackerPal, FALSE);

	m_pFilterBmp = new CBofBitmap(m_pSmk->getWidth(), m_pSmk->getHeight(), m_pSmackerPal, FALSE);
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
	bRepaint = TRUE;

	m_xBounds = CBofRect(0, 0, (WORD)m_pBmpBuf->Width() - 1, (WORD)m_pBmpBuf->Height() - 1);
	ReSize(&m_xBounds, bRepaint);

	// Filter the bitmap.
	//
	CBagMasterWin *pWnd;
	CBagStorageDevWnd *pSDevWnd;
	FilterFunction pFilterFunction;
	if ((pWnd = CBagel::GetBagApp()->GetMasterWnd()) != nullptr) {

		if ((pSDevWnd = pWnd->GetCurrentStorageDev()) != nullptr) {

			if (pSDevWnd->IsFiltered()) {

				USHORT nFilterId = pSDevWnd->GetFilterId();
				pFilterFunction = pSDevWnd->GetFilter();
				m_pBmpBuf->Paint(m_pFilterBmp);
				(*pFilterFunction)(nFilterId, m_pFilterBmp, &m_xBounds);
			}
		}
	}

	// Paint the image to the screen.
	m_pFilterBmp->Paint(this, 0, 0);


	return TRUE;
}

VOID CBagFMovie::OnKeyHit(ULONG lKey, ULONG /*lRepCount*/) {
	if (m_bEscCanStop && lKey == BKEY_ESC) {
		// Clean up and exit
		m_bLoop = FALSE;
		Stop();
		OnMovieDone();
	}
}

VOID CBagFMovie::OnMainLoop() {
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

			// Filter the bitmap.
			CBagMasterWin *pWnd;
			CBagStorageDevWnd *pSDevWnd;
			FilterFunction pFilterFunction;

			m_pBmpBuf->Paint1To1(m_pFilterBmp);

			if ((pWnd = CBagel::GetBagApp()->GetMasterWnd()) != nullptr) {
				if ((pSDevWnd = pWnd->GetCurrentStorageDev()) != nullptr) {
					if (pSDevWnd->IsFiltered()) {
						USHORT nFilterId = pSDevWnd->GetFilterId();
						pFilterFunction = pSDevWnd->GetFilter();
						(*pFilterFunction)(nFilterId, m_pFilterBmp, &m_xBounds);
					}
				}
			}

			// Paint the buffer to the screen.
			m_pFilterBmp->Paint(this, 0, 0);

			if (m_eMovStatus == FOREWARD) {
				if ((m_pSmk->getCurFrame() == (m_pSmk->getFrameCount() - 1)) && m_bLoop == FALSE)
					OnMovieDone();
				else
					SeekToStart(); // Will loop to beginning

			} else if (m_eMovStatus == REVERSE) {

				if ((m_pSmk->getCurFrame() == 0) || (m_pSmk->getCurFrame() == 1)) {
					if (m_bLoop == FALSE)
						OnMovieDone();
					else
						SeekToEnd();

				} else {
					SetFrame(m_pSmk->getCurFrame() - 1); // Go back 1 frame
				}
			}// REVERSE

		}// !STOPPED
	} // !SMACKWAIT
}

VOID CBagFMovie::OnPaint(CBofRect *) {
}

VOID CBagFMovie::CloseMovie() {
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


VOID CBagFMovie::OnClose() {
	CloseMovie();
	CBofDialog::OnClose();
}


VOID CBagFMovie::OnMovieDone() {
	if (!m_bLoop) {

		if (m_bCaptured)
			ReleaseCapture();

		GetParent()->Enable();
		_bEndDialog = TRUE;
	}
}


BOOL CBagFMovie::Play(BOOL bLoop, BOOL bEscCanStop) {
	BOOL bSuccess;

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


BOOL CBagFMovie::Play() {

	if (m_pSmk) {
		m_pSmk->start();
		m_eMovStatus = FOREWARD;
		return TRUE;
	}

	return FALSE;

}

BOOL CBagFMovie::Reverse(BOOL bLoop, BOOL bEscCanStop) {
	BOOL bSuccess = TRUE;

	m_bEscCanStop = bEscCanStop;
	m_bLoop = bLoop;

	bSuccess = Reverse();

	GetParent()->Disable();
	GetParent()->FlushAllMessages();
	DoModal();

	return bSuccess;

}

BOOL CBagFMovie::Reverse() {

	if (m_pSmk) {
		m_pSmk->setReverse(true);
		m_eMovStatus = REVERSE;
		return TRUE;
	}

	return FALSE;

}

BOOL CBagFMovie::Stop() {

	if (m_pSmk) {
		m_pSmk->stop();
		m_eMovStatus = STOPPED;
		return TRUE;
	}
	return FALSE;

}

BOOL CBagFMovie::Pause() {

	if (m_pSmk) {
		m_pSmk->pauseVideo(true);
		m_eMovStatus = PAUSED;
		return TRUE;
	}

	return FALSE;

}

BOOL CBagFMovie::SeekToStart() {
	if (m_pSmk) {
		m_pSmk->seekToFrame(0);
		return TRUE;
	}

	return FALSE;

}

BOOL CBagFMovie::SeekToEnd() {
	if (m_pSmk) {
		m_pSmk->seekToFrame(m_pSmk->getCurFrame() - 1); // Goto last frame
		return TRUE;
	}

	return FALSE;

}

DWORD CBagFMovie::GetFrame() {
	if (m_pSmk) {
		return m_pSmk->getCurFrame();
	}

	return (DWORD) -1;
}

BOOL CBagFMovie::SetFrame(DWORD dwFrameNum) {
	if (m_pSmk) {
		m_pSmk->seekToFrame(dwFrameNum);
		return TRUE;
	}

	return FALSE;
}

VOID CBagFMovie::OnReSize(CBofSize *) {
}

BOOL CBagFMovie::CenterRect() {
	CBofRect            cBofRect;
	RECT                rcParentRect, rcMovieBounds;
	int                 ClientWidth, ClientHeight;
	int                 MovieWidth = 0;
	int                 MovieHeight = 0;

	cBofRect = GetParent()->GetClientRect();
	rcParentRect = cBofRect.GetWinRect();
	ClientWidth = rcParentRect.right - rcParentRect.left;
	ClientHeight = rcParentRect.bottom - rcParentRect.top;

	// Get Movies width and height
	MovieWidth = m_pSmk->getWidth();
	MovieHeight = m_pSmk->getHeight();

	rcMovieBounds.left = (ClientWidth - MovieWidth) / 2;
	rcMovieBounds.top = (ClientHeight - MovieHeight) / 2;
	rcMovieBounds.right = rcMovieBounds.left + MovieWidth;
	rcMovieBounds.bottom = rcMovieBounds.top + MovieHeight;

	// reposition the playback window
	cBofRect = rcMovieBounds;
	ReSize(&cBofRect, TRUE);

	return TRUE;

}

VOID CBagFMovie::OnButtonUp(UINT /*nFlags*/, CBofPoint * /*pPoint*/) {

}

// Create a windows palette from a smacker palette.
//
#if 0
HPALETTE CBagFMovie::WinPalFromSmkPal() {
	static const int kNumColors = 256;
	static const int kSmkBytesPerColor = 3;
	HPALETTE returnValue;


#if BOF_MAC
	//  get a new palette and seed it from the palette
	USHORT nIndex;

	returnValue = ::NewPalette(kNumColors, 0, pmExplicit | pmAnimated, 0);
	Assert(returnValue != nullptr);

	RGBColor rgb;

	for (INT i = 0; i < kNumColors; i++) {
		rgb.red = m_pSmk->Palette[kSmkBytesPerColor * i] << 8;
		rgb.green = m_pSmk->Palette[(kSmkBytesPerColor * i) + 1] << 8;
		rgb.blue = m_pSmk->Palette[(kSmkBytesPerColor * i) + 2] << 8;

		// This flips our definition of black and white for our filtered movies
		switch (i) {
		case 0:
			nIndex = 255;
			break;
		case 255:
			nIndex = 0;
			break;
		default:
			nIndex = i;
			break;
		}
		SetEntryColor(returnValue, nIndex, &rgb);
	}

#else
	LOGPALETTE *pLogPal = nullptr;

	pLogPal = (LOGPALETTE *)BofAlloc(sizeof(LOGPALETTE) + sizeof(PALETTEENTRY) * kNumColors);
	if (pLogPal) {
		pLogPal->palNumEntries = (WORD)kNumColors;
		pLogPal->palVersion = 0x300;
		for (int i = 0; i < kNumColors; i++) {
			pLogPal->palPalEntry[i].peRed = m_pSmk->Palette[kSmkBytesPerColor * i];
			pLogPal->palPalEntry[i].peGreen = m_pSmk->Palette[(kSmkBytesPerColor * i) + 1];
			pLogPal->palPalEntry[i].peBlue = m_pSmk->Palette[(kSmkBytesPerColor * i) + 2];
			pLogPal->palPalEntry[i].peFlags = 0;
		}

		// Create the windows palette.
		//
		returnValue = Bagel::CreatePalette(pLogPal);

		// Release the memory allocated for the logical palette.
		//
		BofFree(pLogPal);
		pLogPal = nullptr;
	}
#endif

	// Return the handle to the windows palette to the caller.
	return returnValue;
}
#endif

} // namespace Bagel
