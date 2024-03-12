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

#define MOVTIMEID (1)

CBofMovie::CBofMovie(CBofWindow *pParent, const CHAR *pszFilename, CBofRect *pBounds, BOOL bStretch, BOOL bUseNewPalette, BOOL bBlackOutWindow) {
	m_bStretch = bStretch;

	// allow movie to not shift to new palette.
	m_bUseNewPalette = bUseNewPalette;

	// black out first and last frame of flythroughs and examine movies
	m_bBlackOutWindow = bBlackOutWindow;

#if BOF_MAC
	// eliminate the rectangle around the movie
	SetCustomWindow(TRUE);
	m_bPositioned = FALSE;
#endif
	Initialize(pParent);
	Open(pszFilename, pBounds);
}

CBofMovie::~CBofMovie() {
	CloseMovie();
	// removed the need for the timer mdm 8/4
#if 0
	if (m_pMovTimer) {
		delete m_pMovTimer;
		m_pMovTimer = nullptr;
	}
#endif

}

ERROR_CODE CBofMovie::Initialize(CBofWindow *pParent) {
	// Movie Stuff
	m_eMovStatus = STOPPED;
	m_eMovType = SMACKER;
	m_bEscCanStop = TRUE;

#if BOF_WINNT
	// MCI Stuff
	m_wMCIDeviceID = 0;
#endif
	// Smacker Stuff
	m_pSbuf = nullptr;
	m_pSmk = nullptr;
	m_bLoop = FALSE;

	// removed the need for the timer mdm 8/4
	//  m_pMovTimer = nullptr;
	//  m_bReverseWait=FALSE;

	// Set the movie timer for movies in reverse to be 10 fps
	///m_pMovTimer = new CBofTimer( MOVTIMEID, (UINT)100, (ULONG)this, fOnMovieTimer);
	// Assert(m_pMovTimer!=nullptr);

	// Call dialog box creates
	if (Create("MovieWin", 0, 0, 640, 480, pParent, 1) == ERR_NONE) {
		SetCapture();
	}

	return ERR_NONE;

}

BOOL CBofMovie::Open(const CHAR *sFilename, CBofRect *pBounds) {
	BOOL Success;

	// no filename, so put up an open file box
	//
	if (sFilename == nullptr) {

		Success = FileOpenWin();
		return Success;
	}

	// do the resize or center before we open the movie
	// and smack our first frame
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
		else // Center the movie to the parent window
			CenterRect();
#endif

		return TRUE;
	}

	return FALSE;
}

BOOL CBofMovie::OpenMovie(const char *sFilename) {

	Assert(sFilename[0] != '\0');

	// Get a non const version of the filename
	CBofString              filename(sFilename);
	CBofString              extension(filename.Right(3));

	extension.MakeUpper();

	if (extension == "SMK")
		m_eMovType = SMACKER;
	else
		m_eMovType = QT;

	if (m_eMovType == SMACKER) {
		if (m_pSmk) {
			CloseMovie();
		}

		m_pSmk = new Video::SmackerDecoder();
		if (!m_pSmk->loadFile(filename.GetBuffer())) {
			// Opened failed
#ifdef _DEBUG
#if BOF_MAC || BOF_WINMAC
			ReportError(ERR_FOPEN, "Movie not found=%s", filename.GetBuffer());
#else
			error("SmackOpen failed");
#endif
			FileOpenWin(); // Put up to open file message box
#endif
			return FALSE;
		}

		// If supposed to stretch into specified window
		//
		if (m_bStretch) {
			m_pSbuf = new Graphics::ManagedSurface(Width(), Height(), m_pSmk->getPixelFormat());

		} else {
			m_pSbuf = new Graphics::ManagedSurface(m_pSmk->getWidth(), m_pSmk->getHeight(), m_pSmk->getPixelFormat());
		}

		CBofRect MovieBounds(0, 0, (WORD)m_pSbuf->w - 1, (WORD)m_pSbuf->h - 1);
		ReSize(&MovieBounds, TRUE);


#if PALETTESHIFTFIX
		CBofWindow::CheckPaletteShiftList();
#endif

#if 0
		// mdm black frame fix
		// Paint first frame
		if (m_pSmk->NewPalette && m_bUseNewPalette) {

			SmackBufferNewPalette(m_pSbuf, m_pSmk->Palette, 0);
			SmackColorRemap(m_pSmk, m_pSbuf->Palette, m_pSbuf->MaxPalColors, m_pSbuf->PalType);

			// if we have a window that is going to cause a single frame
			// palette shift, then black it out here.
			if (m_bBlackOutWindow) {
				FillWindow(COLOR_BLACK);
			}
#if BOF_MAC
			// gets rid of first frame palette problems
			Show();

			// get a black and white movie without this call.
			SmackBufferSetPalette(m_pSbuf);
#endif
		}
#endif

		// Smack the current frame into the buffer
		const Graphics::Surface *frame = m_pSmk->decodeNextFrame();
		if (frame) {
			m_pSbuf->copyFrom(*frame);
			m_pSbuf->setPalette(m_pSmk->getPalette(), 0, 256);
		}

		return TRUE;
	} else if (m_eMovType == QT) {
#if BOF_WINNT

		MCI_DGV_OPEN_PARMS     mciOpen;
		MCIERROR               mciError;
		MCI_GENERIC_PARMS      mciGeneric;

		if (m_wMCIDeviceID) // Device already open
			CloseMovie();    // Close device

		mciOpen.dwCallback = nullptr;
		mciOpen.lpstrDeviceType = nullptr;
		mciOpen.lpstrElementName = filename.GetBuffer();
		mciOpen.lpstrAlias = nullptr;
		mciOpen.hWndParent = GetHandle();

		// This was changed to work with dialog boxes
		// VERY SENSITIVE CODE
		// mciOpen.dwStyle = WS_POPUP;

		// try to open the file
		// mciError = mciSendCommand(0, MCI_OPEN,
		//      (DWORD)(MCI_OPEN_ELEMENT|MCI_DGV_OPEN_PARENT|MCI_DGV_OPEN_WS),
		//          (DWORD)(LPMCI_DGV_OPEN_PARMS)&mciOpen);
		mciError = mciSendCommand(0, MCI_OPEN,
		                          (DWORD)(MCI_OPEN_ELEMENT | MCI_DGV_OPEN_PARENT),
		                          (DWORD)(LPMCI_DGV_OPEN_PARMS)&mciOpen);


		if (mciError) {
			EvalMciError(mciError);

			::MessageBox(GetParent()->GetHandle(),
			             filename.GetBuffer(),
			             __FILE__, MB_ICONEXCLAMATION | MB_OK);

			FileOpenWin();
			return FALSE;
		} else { // we opened the file o.k.

			m_wMCIDeviceID = mciOpen.wDeviceID; // save ID

			// Realize the movie palette in the background
			// so only unused colors will be altered - no flash

			mciError = mciSendCommand(m_wMCIDeviceID,
			                          MCI_REALIZE, MCI_ANIM_REALIZE_BKGD | MCI_WAIT,
			                          (DWORD)(LPMCI_GENERIC_PARMS)&mciGeneric);

			if (mciError) {
				EvalMciError(mciError);
				return FALSE;
			}

			// Initialize Movie
			SeekToStart();
			Pause();
			ShowMovie();

			// cause an update to occur
			GetParent()->InvalidateRect(nullptr);
			GetParent()->UpdateWindow();

			return TRUE;

		}
#endif
	}
	return FALSE;
}

VOID  CBofMovie::OnKeyHit(ULONG lKey, ULONG /*lRepCount*/) {
	if (m_bEscCanStop && lKey == BKEY_ESC) {
		// Clean up and exit
		m_bLoop = FALSE;
		Stop();
		if (m_eMovType == SMACKER)
			OnMovieDone();  // SMACKER NEEDS THIS CALLED,
		// MCI WILL CALL IT AUTOMATICALLY
	}
}

VOID  CBofMovie::OnMainLoop() {
	if (m_eMovType == SMACKER) {
		if (m_pSmk->needsUpdate()) {
			if (m_eMovStatus != STOPPED) {
				// Smack the current frame into the buffer
				const Graphics::Surface *frame = m_pSmk->decodeNextFrame();
				if (frame) {
					m_pSbuf->copyFrom(*frame);
					m_pSbuf->setPalette(m_pSmk->getPalette(), 0, 256);
				}

				if (m_eMovStatus == FOREWARD) {
					// removed the need for the timer mdm 8/4
					// if (m_pMovTimer->IsActive())
					//  m_pMovTimer->Stop();

					if ((m_pSmk->getCurFrame() == (m_pSmk->getFrameCount() - 1)) && m_bLoop == FALSE)
						OnMovieDone();
				} else if (m_eMovStatus == REVERSE) {
					// removed the need for the timer mdm 8/4
					// if (!m_pMovTimer->IsActive())
					//  m_pMovTimer->Start();

					if ((m_pSmk->getCurFrame() == 0) || (m_pSmk->getCurFrame() == 1)) {
						if (m_bLoop == FALSE)
							OnMovieDone();
						else
							SeekToEnd();
					} else {
						// removed the need for the timer mdm 8/4
						//if (m_bReverseWait==FALSE)
						//{
						SetFrame(m_pSmk->getCurFrame() - 1); // Go back 1 frame
						// m_bReverseWait=TRUE;     // removed the need for the timer mdm 8/4
						//}
					}
				}// REVERSE
			}// !STOPPED
		}// !SMACKWAIT
	}// SMACKER
}

VOID  CBofMovie::OnPaint(CBofRect *) {
	if (m_eMovType == SMACKER) {
		if (m_pSbuf) {
			getSurface()->blitFrom(*m_pSbuf);
		}
	}
}

VOID  CBofMovie::CloseMovie() {
	if (m_eMovType == SMACKER) {
		if (m_pSbuf) {
			delete m_pSbuf;
			m_pSbuf = nullptr;
		}

		if (m_pSmk) {
			delete m_pSmk;
			m_pSmk = nullptr;
		}

	} else if (m_eMovType == QT) {

#if BOF_WINNT

		MCIERROR                mciError;
		MCI_GENERIC_PARMS       mciGeneric;

		if (m_wMCIDeviceID) {
			mciError =  mciSendCommand(m_wMCIDeviceID, MCI_CLOSE, MCI_WAIT,
			                           (DWORD)(LPMCI_GENERIC_PARMS)&mciGeneric);

			if (mciError) {
				EvalMciError(mciError);
			}

			m_wMCIDeviceID = nullptr;
		}
#endif
	}
}

VOID  CBofMovie::OnClose() {

	CloseMovie();
	CBofDialog::OnClose();

}

VOID CBofMovie::OnMovieDone() {
	if (!m_bLoop) {
		if (m_bCaptured)
			ReleaseCapture();

		// m_pMovTimer->Stop();

		GetParent()->Enable();
		_bEndDialog = TRUE;
	}
}

#if BOF_WINNT
VOID CBofMovie::OnMCINotify(ULONG /*lParam1*/, ULONG /*lParam2*/) {
	OnMovieDone();
}
#endif

BOOL CBofMovie::ShowMovie() {
	if (m_eMovType == QT) {
#if BOF_WINNT
		MCI_DGV_WINDOW_PARMS   mciWindow;
		MCIERROR               mciError;

		if (m_wMCIDeviceID) {
			// show the playback window
			mciWindow.dwCallback = nullptr;
			mciWindow.hWnd = nullptr;
			mciWindow.nCmdShow = SW_SHOW;
			mciWindow.lpstrText = (LPSTR)nullptr;

			mciError = mciSendCommand(m_wMCIDeviceID, MCI_WINDOW,
			                          MCI_DGV_WINDOW_STATE | MCI_WAIT,
			                          (DWORD)(LPMCI_DGV_WINDOW_PARMS)&mciWindow);

			if (mciError) {
				EvalMciError(mciError);
				return FALSE;
			}

			return TRUE;
		}
#endif
	}
	return FALSE;
}

BOOL CBofMovie::HideMovie() {
	if (m_eMovType == QT) {
#if BOF_WINNT

		MCI_DGV_WINDOW_PARMS   mciWindow;
		MCIERROR               mciError;

		if (m_wMCIDeviceID) {
			// hide the playback window
			mciWindow.dwCallback = nullptr;
			mciWindow.hWnd = nullptr;
			mciWindow.nCmdShow = SW_HIDE;
			mciWindow.lpstrText = (LPSTR)nullptr;

			mciError = mciSendCommand(m_wMCIDeviceID, MCI_WINDOW,
			                          MCI_DGV_WINDOW_STATE | MCI_WAIT,
			                          (DWORD)(LPMCI_DGV_WINDOW_PARMS)&mciWindow);

			if (mciError) {
				EvalMciError(mciError);
				return FALSE;
			}

			return TRUE;
		}

#endif
	}
	return FALSE;
}

#if BOF_MAC
#pragma profile off     // movies are skewing our profiling
// data, since we don't have control over it, don't
// include it.
#endif
BOOL CBofMovie::Play(BOOL bLoop, BOOL bEscCanStop) {
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


BOOL CBofMovie::Play() {

	if (m_eMovType == SMACKER) {
		if (m_pSmk) {
			m_pSmk->start();
			m_eMovStatus = FOREWARD;
			return TRUE;
		}
	} else if (m_eMovType == QT) {
#if BOF_WINNT
		DWORD                   dwFlags;
		MCI_DGV_PLAY_PARMS      mciPlay;
		MCIERROR                mciError;

		if (m_wMCIDeviceID) {
			mciPlay.dwFrom = mciPlay.dwTo = 0;
			dwFlags = MCI_NOTIFY;
			mciPlay.dwCallback = (DWORD)(GetHandle());

			mciError = mciSendCommand(m_wMCIDeviceID, MCI_PLAY, dwFlags,
			                          (DWORD)(LPMCI_DGV_PLAY_PARMS)&mciPlay);

			if (mciError) {
				EvalMciError(mciError);
				return FALSE;
			} else {
				m_eMovStatus = FOREWARD;
				return TRUE;
			}
		}
#endif
	}

	return FALSE;

}
#if BOF_MAC
#pragma profile reset
#endif

BOOL CBofMovie::Reverse(BOOL bLoop,  BOOL bEscCanStop) {
	BOOL bSuccess = TRUE;

	m_bEscCanStop   = bEscCanStop;
	m_bLoop = bLoop;

	bSuccess = Reverse();

	GetParent()->Disable();
	GetParent()->FlushAllMessages();
	DoModal();

	return bSuccess;

}

BOOL CBofMovie::Reverse() {

	if (m_eMovType == SMACKER) {
		if (m_pSmk) {
			m_pSmk->setReverse(true);
			m_eMovStatus = REVERSE;
			return TRUE;
		}
	} else if (m_eMovType == QT) {
#if BOF_WINNT
		DWORD                   dwFlags;
		MCI_DGV_PLAY_PARMS      mciPlay;
		MCIERROR                mciError;

		if (m_wMCIDeviceID) {
			mciPlay.dwFrom = mciPlay.dwTo = 0;

			dwFlags = MCI_NOTIFY | MCI_DGV_PLAY_REVERSE;
			mciPlay.dwCallback = (DWORD)(GetHandle());

			mciError = mciSendCommand(m_wMCIDeviceID, MCI_PLAY, dwFlags,
			                          (DWORD)(LPMCI_DGV_PLAY_PARMS)&mciPlay);

			if (mciError) {
				EvalMciError(mciError);
				return FALSE;
			} else {
				m_eMovStatus = REVERSE;
				return TRUE;
			}
		}
#endif
	}

	return FALSE;

}

BOOL CBofMovie::Stop() {

	if (m_eMovType == SMACKER) {
		if (m_pSmk) {
			m_pSmk->stop();
			m_eMovStatus = STOPPED;
			return TRUE;
		}
	} else if (m_eMovType == QT) {
#if BOF_WINNT
		MCI_DGV_STOP_PARMS       mciStop;
		MCIERROR                 mciError;

		if (m_wMCIDeviceID) {
			// tell it to pause
			mciError = mciSendCommand(m_wMCIDeviceID, MCI_STOP, MCI_WAIT,
			                          (DWORD)(LPMCI_DGV_STOP_PARMS)&mciStop);

			if (mciError) {
				EvalMciError(mciError);
				return FALSE;
			}
			m_eMovStatus = STOPPED;
			return TRUE;
		}
#endif
	}
	return FALSE;

}

BOOL CBofMovie::Pause() {

	if (m_eMovType == SMACKER) {
		if (m_pSmk) {
			m_pSmk->pauseVideo(true);
			m_eMovStatus = PAUSED;
			return TRUE;
		} else
			return FALSE;
	} else if (m_eMovType == QT) {
#if BOF_WINNT
		MCI_DGV_PAUSE_PARMS      mciPause;
		MCIERROR                 mciError;

		if (m_wMCIDeviceID) {
			// tell it to pause
			mciError = mciSendCommand(m_wMCIDeviceID, MCI_PAUSE, MCI_WAIT,
			                          (DWORD)(LPMCI_DGV_PAUSE_PARMS)&mciPause);
			if (mciError) {
				EvalMciError(mciError);
				return FALSE;
			}
			m_eMovStatus = PAUSED;
			return TRUE;
		}
#endif
	}

	return FALSE;

}

BOOL CBofMovie::SeekToStart() {
	if (m_eMovType == SMACKER) {
		m_pSmk->seekToFrame(0);
		return FALSE;
	} else if (m_eMovType == QT) {
#if BOF_WINNT
		MCI_SEEK_PARMS          mciSeek;
		MCIERROR                mciError;

		if (m_wMCIDeviceID) {
			// rewind the movie
			mciError = mciSendCommand(m_wMCIDeviceID, MCI_SEEK, MCI_SEEK_TO_START | MCI_WAIT,
			                          (DWORD)(LPMCI_SEEK_PARMS)&mciSeek);

			if (mciError) {
				EvalMciError(mciError);
				return FALSE;
			}

			return TRUE;

		}
#endif
	}
	return FALSE;

}

BOOL CBofMovie::SeekToEnd() {
	if (m_eMovType == SMACKER) {
		m_pSmk->seekToFrame(m_pSmk->getCurFrame() - 1); // Goto last frame
		return TRUE;
	} else if (m_eMovType == QT) {
#if BOF_WINNT
		MCIERROR                mciError;
		MCI_SEEK_PARMS          mciSeek;

		if (m_wMCIDeviceID) {
			// rewind the movie
			mciError = mciSendCommand(m_wMCIDeviceID, MCI_SEEK, MCI_SEEK_TO_END | MCI_WAIT,
			                          (DWORD)(LPMCI_SEEK_PARMS)&mciSeek);

			if (mciError) {
				EvalMciError(mciError);
				return FALSE;
			}

			return TRUE;

		}
#endif
	}
	return FALSE;

}

DWORD CBofMovie::GetFrame() {
	if (m_eMovType == SMACKER) {
		if (m_pSmk)
			return m_pSmk->getCurFrame();
	} else if (m_eMovType == QT) {
#if BOF_WINNT
		MCI_DGV_STATUS_PARMS    mciStatus;
		MCIERROR                mciError;

		if (m_wMCIDeviceID) {
			mciStatus.dwItem = MCI_STATUS_POSITION;

			mciError = mciSendCommand(m_wMCIDeviceID, MCI_STATUS,
			                          MCI_STATUS_ITEM,
			                          (DWORD)(LPMCI_STATUS_PARMS)&mciStatus);

			if (mciError) {
				EvalMciError(mciError);
				return (DWORD) -1;
			}

			return mciStatus.dwReturn;
		}
#endif
	}
	return (DWORD) -1;
}


BOOL CBofMovie::SetFrame(DWORD dwFrameNum) {
	if (m_eMovType == SMACKER) {
		if (m_pSmk) {
			m_pSmk->seekToFrame(dwFrameNum);
			return TRUE;
		} else
			return FALSE;
	} else if (m_eMovType == QT) {
#if BOF_WINNT
		MCIERROR                mciError;
		MCI_SEEK_PARMS          mciSeek;


		mciSeek.dwTo = dwFrameNum;

		if (m_wMCIDeviceID) {
			// rewind the movie
			mciError = mciSendCommand(m_wMCIDeviceID, MCI_SEEK, MCI_TO | MCI_WAIT,
			                          (DWORD)(LPMCI_SEEK_PARMS)&mciSeek);

			if (mciError) {
				EvalMciError(mciError);
				return FALSE;
			}

			else return TRUE;

		} else
			return FALSE;
#endif
	}

	return FALSE;
}

VOID CBofMovie::OnReSize(CBofSize *pSize) {
	if (m_eMovType == QT) {
#if BOF_WINNT

		DWORD                   mciErr;
		MCI_DGV_PUT_PARMS       mciPut;
		DWORD                   putFlags = 0;

		if (m_wMCIDeviceID) {

			// putFlags = putFlags | MCI_DGV_RECT | MCI_DGV_PUT_DESTINATION | MCI_WAIT;
			putFlags = putFlags | MCI_DGV_RECT | MCI_DGV_PUT_WINDOW;

			mciPut.rc.top = (int) 0;
			mciPut.rc.left = (int) 0;
			mciPut.rc.bottom = (int) pSize->cy;
			mciPut.rc.right = (int) pSize->cx;


			mciErr = mciSendCommand(m_wMCIDeviceID, MCI_PUT, putFlags, (DWORD)(LPSTR)&mciPut);
			if (mciErr) {
				EvalMciError(mciErr);
			}

		}
#endif
	}
}

BOOL CBofMovie::CenterRect() {
	CBofRect            cBofRect;
	RECT                rcParentRect, rcMovieBounds;
	int                 ClientWidth, ClientHeight;
	int                 MovieWidth = 0;
	int                 MovieHeight = 0;

#if BOF_MAC     // Use the windows port rect
	rcParentRect = m_pParentWnd->GetRect();
	m_bPositioned = TRUE;
#else
	cBofRect =    GetParent()->GetClientRect();
	rcParentRect =  cBofRect.GetWinRect();
#endif
	ClientWidth =   rcParentRect.right - rcParentRect.left;
	ClientHeight =  rcParentRect.bottom - rcParentRect.top;

	// Get Movies width and height
	if (m_eMovType == SMACKER) {
		MovieWidth =    m_pSmk->getWidth();
		MovieHeight =   m_pSmk->getHeight();
	} else if (m_eMovType == QT) {
#if BOF_WINNT
		MCI_DGV_RECT_PARMS  mciRect;
		MCIERROR                mciError;

		if (m_wMCIDeviceID) {


			// get the original size of the movie
			mciError = mciSendCommand(m_wMCIDeviceID, MCI_WHERE,
			                          MCI_DGV_WHERE_SOURCE,
			                          (DWORD)(LPMCI_DGV_RECT_PARMS)&mciRect);

			if (mciError) {
				EvalMciError(mciError);
				return FALSE;
			} else {
				rcMovieBounds = mciRect.rc; // get it in the movie bounds rect
				MovieWidth = rcMovieBounds.right - rcMovieBounds.left;
				MovieHeight = rcMovieBounds.bottom - rcMovieBounds.top;
			}
		}
#endif
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

VOID CBofMovie::OnButtonUp(UINT /*nFlags*/, CBofPoint * /*pPoint*/) {

}


BOOL CBofMovie::FileOpenWin() {
	// Currently disabled because got wacky with the compiler
#if 0
	OPENFILENAME ofn;

	static char szFile [BUFFER_LENGTH];
	static char szFileTitle [BUFFER_LENGTH];

	// use the OpenFile dialog to get the filename
	memset(&ofn, 0, sizeof(ofn));
	ofn.lStructSize = sizeof(ofn);
	ofn.hwndOwner = GetHandle();
	ofn.hwndOwner = m_hWnd;
	ofn.lpstrFile = szFile;
	ofn.nMaxFile = sizeof(szFile);
	ofn.lpstrFileTitle = szFileTitle;
	ofn.nMaxFileTitle = sizeof(szFileTitle);
	ofn.Flags = OFN_PATHMUSTEXIST | OFN_FILEMUSTEXIST;

	// use CommDlg to get our filename
	if (GetOpenFileName(&ofn)) {

		if (OpenMovie(ofn.lpstrFile))
			return TRUE;
		else
			return FALSE;
	}

#endif

	return FALSE;
}

#if 0  // removed the need for the timer
void *fOnMovieTimer(int nTimerId, void *pvInfo) {
	CBofMovie *pMovieObj = (CBofMovie *)pvInfo;

	if (nTimerId == MOVTIMEID)
		pMovieObj->m_bReverseWait = FALSE;

	return pvInfo;
}
#endif

#if BOF_WINNT
void CBofMovie::EvalMciError(MCIERROR   mciError) {
	char   mciErrorString[BUFFER_LENGTH];

	mciGetErrorString(mciError, mciErrorString,
	                  BUFFER_LENGTH);

	::MessageBox(GetParent()->GetHandle(), mciErrorString, __FILE__,
	             MB_ICONEXCLAMATION | MB_OK);

}
#endif


ERROR_CODE BofPlayMovie(CBofWindow *pParent, const CHAR *pszMovieFile, CBofRect *pRect) {
	Assert(pParent != nullptr);
	Assert(pszMovieFile != nullptr);

	CBofMovie cMovie(pParent, pszMovieFile, pRect);
	if (!cMovie.ErrorOccurred()) {
#if BOF_MAC
		BOOL            bDone = FALSE;
		EventRecord     eRec;
		WindowPtr       pMacWindow = cMovie.GetMacWindow();
		SmackBuf       *pSmkBuffer = cMovie.GetSmackBuffer();
		Smack          *pSmkMovie = cMovie.GetSmackMovie();
		CBofBitmap     *pSmkBitmap = cMovie.GetSmackBitmap();
		STBofPort       stSavePort(pMacWindow);

		// since we're seeing such terrible performance with the smacker
		// movies, don't play it as a dialog, just assume the backdrop is painted
		// the way it should be and all we have to worry about is playing the movie.
		if (pMacWindow && pSmkBuffer && pSmkMovie) {
			while (!bDone) {
				if (::OSEventAvail(everyEvent, &eRec)) {
					::WaitNextEvent(everyEvent, &eRec, 0, nullptr);
					switch (eRec.what) {
					case keyDown:
						switch (eRec.message & charCodeMask) {
						case 'q':
						case 'Q':
						case 0x1b:
							bDone = TRUE;
							break;
						}
						break;

					case updateEvt:
						if (pMacWindow == (WindowPtr)eRec.message) {
							GrafPtr     savePort;
							::GetPort(&savePort);
							::SetPort(pMacWindow);
							::BeginUpdate(pMacWindow);
							if (pSmkBuffer) {
								SmackBufferBlit(pSmkBuffer, 0, 0, 0, 0, pSmkMovie->Width, pSmkMovie->Height);
							}
							::EndUpdate(pMacWindow);
							::SetPort(savePort);
						}
						break;
					}
				} else {
					if (!SmackWait(pSmkMovie)) {
						if (pSmkMovie->NewPalette) {
							SmackBufferNewPalette(pSmkBuffer, pSmkMovie->Palette, pSmkMovie->PalType);
							if (pSmkBuffer->PalColorsInUse < 256) {
								SmackColorRemap(pSmkMovie,
								                pSmkBuffer->Palette,
								                pSmkBuffer->MaxPalColors,
								                pSmkBuffer->PalType);
							}
							SmackBufferSetPalette(pSmkBuffer);
						}

						SmackDoFrame(pSmkMovie);

						while (SmackToBufferRect(pSmkMovie, pSmkBuffer->SurfaceType)) {
							if (SmackBufferBlit(pSmkBuffer,
							                    pSmkMovie->LastRectx,
							                    pSmkMovie->LastRecty,
							                    pSmkMovie->LastRectx,
							                    pSmkMovie->LastRecty,
							                    pSmkMovie->LastRectw,
							                    pSmkMovie->LastRecth)) {
								break;
							}
						}
						// number of frames is one based, framenum is zero based, check if we got to the
						// frame at the end, if so, all done.
						if (pSmkMovie->FrameNum == (pSmkMovie->Frames - 1)) {
							bDone = TRUE;
						} else {
							SmackNextFrame(pSmkMovie);
						}
					}
				}
			}
		}
#else
		cMovie.Play(FALSE, TRUE);
#endif
	}

	return cMovie.GetErrorCode();
}

} // namespace Bagel
