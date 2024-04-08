
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

#include "graphics/managed_surface.h"
#include "video/smk_decoder.h"
#include "bagel/boflib/object.h"
#include "bagel/boflib/error.h"
#include "bagel/boflib/llist.h"
#include "bagel/boflib/gui/dialog.h"
#include "bagel/boflib/rect.h"

namespace Bagel {

class CBofMovie : public CBofDialog {
public:
	enum MVSTATUS { STOPPED, PAUSED, FOREWARD, REVERSE};
protected:
	Graphics::ManagedSurface *m_pSbuf;
	Video::SmackerDecoder *m_pSmk;
	bool m_bEscCanStop;
	bool m_bLoop;
	bool m_bStretch;
	bool m_bUseNewPalette;
	bool m_bBlackOutWindow;
	MVSTATUS m_eMovStatus;
	Common::Rect _srcRect, _dstRect;

	virtual ERROR_CODE  initialize(CBofWindow *pParent);

	virtual bool    OpenMovie(const char *sFilename);
	virtual void    CloseMovie();

	virtual void    OnReSize(CBofSize *pSize);

	virtual bool    Play();
	virtual bool    Reverse();

	virtual void    OnLButtonUp(uint32 nFlags, CBofPoint *pPoint, void * = nullptr) {
		OnButtonUp(nFlags, pPoint);
	}
	virtual void    OnRButtonUp(uint32 nFlags, CBofPoint *pPoint) {
		OnButtonUp(nFlags, pPoint);
	}
	virtual void    OnButtonUp(uint32 nFlags, CBofPoint *pPoint);
	virtual void    OnPaint(CBofRect *pRect);
	virtual void    OnMovieDone();
	virtual void    OnClose();
	virtual void    OnMainLoop();
	virtual void    OnKeyHit(uint32 lKey, uint32 lRepCount);

public:

	CBofMovie(CBofWindow *pParent = nullptr, const char *pszFilename = nullptr, CBofRect *pBounds = nullptr, bool bStretch = false, bool bUseNewPalette = true, bool bBlackOutWindow = false);
	~CBofMovie();

	virtual bool        Open(const char *sFilename = nullptr, CBofRect *pBounds = nullptr);

	virtual bool        Play(bool bLoop,  bool bEscCanStop = true);
	virtual bool        Reverse(bool bLoop,  bool bEscCanStop = true);
	virtual bool        Pause();
	virtual bool        Stop();

	virtual MVSTATUS    Status() {
		return m_eMovStatus;
	}

	virtual bool        SeekToStart();
	virtual bool        SeekToEnd();

	virtual uint32       GetFrame();
	virtual bool        SetFrame(uint32 dwFrameNum);

	virtual bool        CenterRect();

	Graphics::ManagedSurface *GetSmackBuffer()  {
		return m_pSbuf;
	}
	Video::SmackerDecoder *GetSmackMovie()   {
		return m_pSmk;
	}
};

ERROR_CODE BofPlayMovie(CBofWindow *pParent, const char *pszMovieFile, CBofRect *pRect = nullptr);

} // namespace Bagel

#endif
