
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

#ifndef BAGEL_BOFLIB_GUI_MOVIE_H
#define BAGEL_BOFLIB_GUI_MOVIE_H

#include "bagel/boflib/boffo.h"

#include "bagel/boflib/object.h"
#include "bagel/boflib/error.h"
#include "bagel/boflib/llist.h"
#include "bagel/boflib/gui/dialog.h"
#include "bagel/boflib/rect.h"
#include "bagel/boflib/timer.h"
#include "bagel/api/smacker.h"

namespace Bagel {

#define BUFFER_LENGTH       254

class CBofMovie : public CBofDialog {
public:
	enum MVSTATUS { STOPPED, PAUSED, FOREWARD, REVERSE};
	enum MVTYPE { QT, SMACKER };
	// BOOL             m_bReverseWait;     // removed the need for the timer mdm 8/4
protected:
	// CBofTimer    *   m_pMovTimer;        // removed the need for the timer mdm 8/4
#if BOF_WINDOWS
	UINT            m_wMCIDeviceID;         // MCI Device ID for the QT file
#endif

	SmackBuf *m_pSbuf;
	Smack *m_pSmk;
	MVTYPE m_eMovType;
	BOOL m_bEscCanStop;
	BOOL m_bLoop;
	BOOL m_bStretch;
	BOOL m_bUseNewPalette;
	BOOL m_bBlackOutWindow;
	MVSTATUS m_eMovStatus;
#if BOF_MAC
	BOOL            m_bPositioned;
#endif

	virtual ERROR_CODE  Initialize(CBofWindow *pParent);   // Initialize

	virtual BOOL    OpenMovie(const char *sFilename);  // MCI_OPEN
	virtual VOID    CloseMovie(VOID);

	virtual BOOL    FileOpenWin();    // Display the File Open Dialog box

	virtual VOID    OnReSize(CBofSize *pSize);

	virtual BOOL    Play();
	virtual BOOL    Reverse();

	//virtual VOID  OnMouseMove(UINT nFlags, CBofPoint *pPoint){}
	virtual VOID    OnLButtonUp(UINT nFlags, CBofPoint *pPoint) {
		OnButtonUp(nFlags, pPoint);
	}
	virtual VOID    OnRButtonUp(UINT nFlags, CBofPoint *pPoint) {
		OnButtonUp(nFlags, pPoint);
	}
	virtual VOID    OnButtonUp(UINT nFlags, CBofPoint *pPoint);
	virtual VOID    OnPaint(CBofRect *pRect);
	virtual VOID    OnMovieDone(VOID);
	virtual VOID    OnClose(VOID);
	virtual VOID    OnMainLoop(VOID);
	virtual VOID    OnKeyHit(ULONG lKey, ULONG lRepCount);

#if BOF_WINNT
	virtual VOID    OnMCINotify(ULONG lParam1, ULONG lParam2);
	virtual void    EvalMciError(MCIERROR theError);   // MessageBox the MCI error
#endif

public:

	CBofMovie(CBofWindow *pParent = NULL, const CHAR *pszFilename = NULL, CBofRect *pBounds = NULL, BOOL bStretch = FALSE, BOOL bUseNewPalette = TRUE, BOOL bBlackOutWindow = FALSE);
	~CBofMovie();

	virtual BOOL        Open(const char *sFilename = NULL, CBofRect *pBounds = NULL);

	virtual BOOL        Play(BOOL bLoop,  BOOL bEscCanStop = TRUE);
	virtual BOOL        Reverse(BOOL bLoop,  BOOL bEscCanStop = TRUE);
	virtual BOOL        Pause(void);
	virtual BOOL        Stop(void);

	virtual MVSTATUS    Status(void) {
		return m_eMovStatus;
	}

	virtual BOOL        SeekToStart(void);           // Seek to the start of the movie
	virtual BOOL        SeekToEnd(void);                 // Seek to the start of the movie

	virtual DWORD       GetFrame(void);
	virtual BOOL        SetFrame(DWORD dwFrameNum);

	virtual BOOL        CenterRect();

	virtual BOOL        ShowMovie(void);
	virtual BOOL        HideMovie(void);

	SmackBuf            *GetSmackBuffer(void)  {
		return m_pSbuf;
	}
	Smack               *GetSmackMovie(void)   {
		return m_pSmk;
	}
};

ERROR_CODE BofPlayMovie(CBofWindow *pParent, const CHAR *pszMovieFile, CBofRect *pRect = NULL);

} // namespace Bagel

#endif
