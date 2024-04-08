
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

#ifndef BAGEL_BAGLIB_FMOVIE_H
#define BAGEL_BAGLIB_FMOVIE_H

#include "bagel/boflib/boffo.h"

#include "graphics/managed_surface.h"
#include "video/smk_decoder.h"
#include "bagel/boflib/gui/dialog.h"
#include "bagel/boflib/rect.h"
#include "bagel/boflib/error.h"
#include "bagel/boflib/object.h"
#include "bagel/boflib/gfx/palette.h"
#include "bagel/boflib/llist.h"
#include "bagel/baglib/storage_dev_win.h"

namespace Bagel {

class CBagFMovie : public CBofDialog {
public:
	enum MVSTATUS { STOPPED, PAUSED, FOREWARD, REVERSE };

protected:
	Graphics::ManagedSurface *m_pSbuf;
	Video::SmackerDecoder *m_pSmk;

	BOOL m_bEscCanStop;
	BOOL m_bLoop;
	MVSTATUS m_eMovStatus;

	CBofBitmap *m_pBmpBuf;
	CBofBitmap *m_pFilterBmp;
	CBofPalette *m_pSmackerPal;
	char *m_pBufferStart;
	int m_nBufferLength;
	uint32 m_nReversed;
	CBofRect m_xBounds;
	BOOL m_bUseNewPalette;
	BOOL m_bBlackOutWindow;

	/**
	 * Open the movie file, center it in parent, rewind it, and realize it's
	 * palette in the background
	 */
	virtual ERROR_CODE initialize(CBofWindow *pParent);

	/**
	 * Open the movie file, center it in parent, rewind it, and realize it's
	 * palette in the background.
	 */
	virtual BOOL OpenMovie(const char *sFilename);

	/**
	 * Close the MCI Device file
	 */
	virtual VOID CloseMovie();

	virtual VOID OnReSize(CBofSize *pSize);

	virtual BOOL Play();
	virtual BOOL Reverse();

	virtual VOID OnLButtonUp(UINT nFlags, CBofPoint *pPoint, void * = nullptr) {
		OnButtonUp(nFlags, pPoint);
	}
	virtual VOID OnButtonUp(UINT nFlags, CBofPoint *pPoint);
	virtual VOID OnPaint(CBofRect *pRect);
	virtual VOID OnMovieDone();
	virtual VOID OnClose();
	virtual VOID OnMainLoop();

	/**
	 * Catch the ESC key when hit
	 */
	virtual VOID OnKeyHit(uint32 lKey, uint32 lRepCount);

public:
	/**
	 * Initializes Movie Object and opens movie file @ Rect
	 * @param pParent       A pointer to the parent window
	 * @param sFilename     String containing filename of movie to be opened
	 * @param pBounds       Location for video object relative to parent
	 */
	CBagFMovie(CBofWindow *pParent = nullptr, const char *sFilename = nullptr,
	           CBofRect *pBounds = nullptr, BOOL bUseNewPalette = TRUE, BOOL bBlackOutWindow = FALSE);

	/**
	 * Destructor
	 */
	~CBagFMovie();

	/**
	 * Open the movie file, place it @ pBounds, rewind it, and realize it's
	 * palette in the background.
	 */
	virtual BOOL Open(const char *sFilename = nullptr, CBofRect *pBounds = nullptr);

	virtual BOOL Play(BOOL bLoop, BOOL bEscCanStop = TRUE);
	virtual BOOL Reverse(BOOL bLoop, BOOL bEscCanStop = TRUE);
	virtual BOOL Pause();
	virtual BOOL Stop();

	virtual MVSTATUS Status() {
		return m_eMovStatus;
	}

	virtual BOOL SeekToStart();
	virtual BOOL SeekToEnd();

	virtual DWORD GetFrame();
	virtual BOOL SetFrame(DWORD dwFrameNum);

	virtual BOOL CenterRect();

	// need to access members from outside of class for
	// performance optimization.
	Graphics::ManagedSurface *GetSmackBuffer() {
		return m_pSbuf;
	}
	Video::VideoDecoder *GetSmackMovie() {
		return m_pSmk;
	}
};

} // namespace Bagel

#endif
