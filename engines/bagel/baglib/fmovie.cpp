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

#if BOF_MAC
#include "smack.h"
#include <mac.h>
#elif BOF_WINDOWS
#include <commdlg.h>
#endif

namespace Bagel {

#define MOVTIMEID (1)

CBagFMovie::CBagFMovie(CBofWindow *pParent, const char *sFilename, CBofRect *pBounds, BOOL bUseNewPalette, BOOL bBlackOutWindow) {
	// Allow movie to not shift to new palette.
	m_bUseNewPalette = bUseNewPalette;

	// Black out first and last frame of flythroughs and examine movies
	m_bBlackOutWindow = bBlackOutWindow;

#if BOF_MAC
	// Eliminate the rectangle around the movie
	SetCustomWindow(TRUE);
	m_bPositioned = FALSE;
#endif

	Initialize(pParent);
	Open(sFilename, pBounds);
}

CBagFMovie::~CBagFMovie() {
	CloseMovie();
}

ERROR_CODE CBagFMovie::Initialize(CBofWindow *pParent) {
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
	BOOL Success;

	// No filename, so put up an open file box
	if (sFilename == nullptr) {
		Success = FileOpenWin();
		return Success;
	}

	// Do the resize or center before we open the movie and smack our first frame
#if BOF_MAC
	if (pBounds) {
		ReSize(pBounds, false);
		m_bPositioned = TRUE;
	} else {
		m_bPositioned = FALSE;
	}
#endif

#if !BOF_MAC
	if (pBounds != nullptr) {
		m_cRect = *pBounds;
	}
#endif

	if (OpenMovie(sFilename)) {
		// We were given specific rect for movie
#if !BOF_MAC
		if (pBounds)
			ReSize(pBounds, TRUE);
		else
			// Center the movie to the parent window
			CenterRect();
#endif

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

	if (m_eMovType == SMACKER) {
		if (m_pSmk) {
			CloseMovie();
		}

		m_pSmk = SmackOpen(filename.GetBuffer(), SMACKTRACKS, SMACKAUTOEXTRA);

		// Opened failed ?
		//
		if (!m_pSmk) {
#ifdef _DEBUG
#if BOF_MAC
			BofMessageBox("SmackOpen failed", filename.GetBuffer());
#else
			error("SmackOpen failed");
#endif
			FileOpenWin(); // Put up to open file message box
#endif
			return FALSE;
		}

		// Create a Windows palette based on the smacker movie palette.
		// This palette that will be used when the offscreen bitmap is
		// created.
		//
		HPALETTE hPalette;
		hPalette = WinPalFromSmkPal();

		// Allocate the bitmaps.
		//
		m_pSmackerPal = new CBofPalette(hPalette);

		m_pBmpBuf = new CBofBitmap(m_pSmk->Width, m_pSmk->Height, m_pSmackerPal, FALSE);

		m_pFilterBmp = new CBofBitmap(m_pSmk->Width, m_pSmk->Height, m_pSmackerPal, FALSE);
		m_pFilterBmp->Lock();

		SelectPalette(m_pSmackerPal);

		if (m_pBmpBuf) {
			m_pBmpBuf->Lock();
			m_pBmpBuf->FillRect(nullptr, m_pSmackerPal->GetNearestIndex(RGB(255, 255, 255)));

			m_nReversed = !(m_pBmpBuf->IsTopDown());
			m_pBufferStart = (char *)m_pBmpBuf->GetPixelAddress(0, m_nReversed * (m_pBmpBuf->Height() - 1));
			m_nBufferLength = ABS(m_pBmpBuf->Height() * m_pBmpBuf->Width());
			SmackToBuffer(m_pSmk, 0, 0, m_pBmpBuf->Width(), m_pBmpBuf->Height(), m_pBufferStart, m_nReversed);

#if BOF_MAC
			// if we were opened without being positioned, do that here.
			if (m_bPositioned == false) {
				CenterRect();
			}
#endif
			SmackDoFrame(m_pSmk);
		}
		bRepaint = TRUE;

		m_xBounds = CBofRect(0, 0, (WORD)m_pBmpBuf->Width() - 1, (WORD)m_pBmpBuf->Height() - 1);

#if BOF_MAC
		if (m_bPositioned == FALSE)
#endif
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
#if BOF_MAC && __POWERPC__
					CallUniversalProc(pFilterFunction,
					                  uppFilterProcInfo,
					                  nFilterId, m_pFilterBmp, &m_xBounds);
#else
					(*pFilterFunction)(nFilterId, m_pFilterBmp, &m_xBounds);
#endif
				}
			}
		}

		// Paint the image to the screen.
#if BOF_MAC
		if (m_pFilterBmp) {
			m_pFilterBmp->Lock();
			MacintizeBitmap(m_pFilterBmp->GetPixelAddress(0, 0), m_pSmk->Width * m_pSmk->Height);
		}
#endif
		m_pFilterBmp->Paint(this, 0, 0);

#if BOF_MAC
		if (m_pFilterBmp) {
			m_pFilterBmp->UnLock();
		}
#endif

		// Get next frame, will loop to beginning
		SmackNextFrame(m_pSmk);

		return TRUE;
	} else if (m_eMovType == QT) {
		// Add filtered QuickTime movie support here.
	}

	return FALSE;
}

VOID CBagFMovie::OnKeyHit(ULONG lKey, ULONG /*lRepCount*/) {
	if (m_bEscCanStop && lKey == BKEY_ESC) {
		// Clean up and exit
		m_bLoop = FALSE;
		Stop();
		if (m_eMovType == SMACKER)
			OnMovieDone();

		// SMACKER NEEDS THIS CALLED,
		// MCI WILL CALL IT AUTOMATICALLY
	}
}

VOID CBagFMovie::OnMainLoop() {
	if (!SmackWait(m_pSmk)) {
		// Not needed for filtered movies
		if (m_eMovStatus != STOPPED) {
			// Smack the current frame into the buffer
			SmackDoFrame(m_pSmk);

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

#if BOF_MAC && __POWERPC__
						CallUniversalProc(pFilterFunction,
						                  uppFilterProcInfo,
						                  nFilterId, m_pFilterBmp, &m_xBounds);
#else
						(*pFilterFunction)(nFilterId, m_pFilterBmp, &m_xBounds);
#endif
					}
				}
			}

			// Paint the buffer to the screen.
#if BOF_MAC
			if (m_pFilterBmp) {
				m_pFilterBmp->Lock();
				MacintizeBitmap(m_pFilterBmp->GetPixelAddress(0, 0), m_pSmk->Width * m_pSmk->Height);
			}
#endif
			m_pFilterBmp->Paint(this, 0, 0);

#if BOF_MAC
			if (m_pFilterBmp) {
				m_pFilterBmp->UnLock();
			}
#endif

			if (m_eMovStatus == FOREWARD) {
				if ((m_pSmk->FrameNum == (m_pSmk->Frames - 1)) && m_bLoop == FALSE)
					OnMovieDone();
				else
					SmackNextFrame(m_pSmk); // Get next frame, will loop to beginning

			} else if (m_eMovStatus == REVERSE) {

				if ((m_pSmk->FrameNum == 0) || (m_pSmk->FrameNum == 1)) {
					if (m_bLoop == FALSE)
						OnMovieDone();
					else
						SeekToEnd();

				} else {
					SetFrame(m_pSmk->FrameNum - 1); // Go back 1 frame
				}
			}// REVERSE

		}// !STOPPED
	} // !SMACKWAIT
}

VOID CBagFMovie::OnPaint(CBofRect *) {
}

VOID CBagFMovie::CloseMovie() {
	if (m_eMovType == SMACKER) {

		if (m_pSbuf != nullptr) {
			SmackBufferClose(m_pSbuf);
			m_pSbuf = nullptr;
		}

		if (m_pSmk != nullptr) {
			SmackClose(m_pSmk);
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

	} else if (m_eMovType == QT) {

		// Add support for QuickTime movies here.
		//
	}
}


VOID CBagFMovie::OnClose() {
	CloseMovie();
	CBofDialog::OnClose();
}


VOID CBagFMovie::OnMovieDone() {
	if (!m_bLoop) {

		if (m_bCaptured)
			ReleaseCapture();

		// m_pMovTimer->Stop();

		GetParent()->Enable();
		_bEndDialog = TRUE;
	}
}


BOOL CBagFMovie::ShowMovie() {
	if (m_eMovType == QT) {

		// Add QuickTime support here.
		//
	}
	return FALSE;
}


BOOL CBagFMovie::HideMovie() {
	if (m_eMovType == QT) {

		// Add QuickTime support here.
		//
	}
	return FALSE;
}


#if BOF_MAC
#pragma profile off     // movies are skewing our profiling
// data, since we don't have control over it, don't
// include it.
#endif
BOOL CBagFMovie::Play(BOOL bLoop, BOOL bEscCanStop) {
	BOOL bSuccess;

	m_bEscCanStop = bEscCanStop;
	m_bLoop = bLoop;

	bSuccess = Play();

	GetParent()->Disable();
	GetParent()->FlushAllMessages();

#if BOF_WINDOWS
	CBofCursor::Hide();
#endif

	DoModal();

#if BOF_WINDOWS
	CBofCursor::Show();
#endif

	return bSuccess;
}


BOOL CBagFMovie::Play() {

	if (m_eMovType == SMACKER) {
		if (m_pSmk) {
			m_eMovStatus = FOREWARD;
			return TRUE;
		}
	} else if (m_eMovType == QT) {
		// Add QuickTime support here.
		//
	}

	return FALSE;

}
#if BOF_MAC
#pragma profile reset
#endif
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

	if (m_eMovType == SMACKER) {
		if (m_pSmk) {
			m_eMovStatus = REVERSE;
			return TRUE;
		}
	} else if (m_eMovType == QT) {
		// Add QuickTime support here.
		//
	}

	return FALSE;

}

BOOL CBagFMovie::Stop() {

	if (m_eMovType == SMACKER) {
		if (m_pSmk) {
			m_eMovStatus = STOPPED;
			return TRUE;
		}
	} else if (m_eMovType == QT) {
		// Add QuickTime support here.
		//
	}
	return FALSE;

}

BOOL CBagFMovie::Pause() {

	if (m_eMovType == SMACKER) {
		if (m_pSmk) {
			m_eMovStatus = PAUSED;
			return TRUE;
		} else
			return FALSE;
	} else if (m_eMovType == QT) {
		// Add QuickTime support here.
		//
	}

	return FALSE;

}

BOOL CBagFMovie::SeekToStart() {
	if (m_eMovType == SMACKER) {
		SmackGoto(m_pSmk, 0);
		return FALSE;
	} else if (m_eMovType == QT) {
		// Add Quicktime support here.
		//
	}

	return FALSE;

}

BOOL CBagFMovie::SeekToEnd() {
	if (m_eMovType == SMACKER) {
		SmackGoto(m_pSmk, m_pSmk->Frames - 1); // Goto last frame
		return TRUE;
	} else if (m_eMovType == QT) {
		// Add QuickTime support here.
		//
	}

	return FALSE;

}

DWORD CBagFMovie::GetFrame() {
	if (m_eMovType == SMACKER) {
		if (m_pSmk)
			return m_pSmk->FrameNum;
	} else if (m_eMovType == QT) {
		// Add QuickTime suport here.
		//
	}

	return (DWORD) -1;
}

BOOL CBagFMovie::SetFrame(DWORD dwFrameNum) {
	if (m_eMovType == SMACKER) {
		if (m_pSmk) {
			SmackGoto(m_pSmk, dwFrameNum);
			SmackNextFrame(m_pSmk); // This seems to be neccessary to increment frameNum etc.
			return TRUE;
		} else
			return FALSE;
	} else if (m_eMovType == QT) {
		// Add QuickTime support here.
		//
	}

	return FALSE;
}

VOID CBagFMovie::OnReSize(CBofSize *) {
	if (m_eMovType == QT) {
		// Add QuickTime support here.
		//
	}
}

BOOL CBagFMovie::CenterRect() {
	CBofRect            cBofRect;
	RECT                rcParentRect, rcMovieBounds;
	int                 ClientWidth, ClientHeight;
	int                 MovieWidth = 0;
	int                 MovieHeight = 0;

#if BOF_MAC     // Use the windows port rect
	rcParentRect = m_pParentWnd->GetRect();
	m_bPositioned = TRUE;
#else
	cBofRect = GetParent()->GetClientRect();
	rcParentRect = cBofRect.GetWinRect();
#endif
	ClientWidth = rcParentRect.right - rcParentRect.left;
	ClientHeight = rcParentRect.bottom - rcParentRect.top;

	// Get Movies width and height
	if (m_eMovType == SMACKER) {
		MovieWidth = m_pSmk->Width;
		MovieHeight = m_pSmk->Height;
	} else if (m_eMovType == QT) {
		// Add QuickTime support here.
		//
	}

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

BOOL CBagFMovie::FileOpenWin() {
	// Currently disabled because got wacky with the compiler
	return FALSE;
}

// Create a windows palette from a smacker palette.
//
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

} // namespace Bagel
