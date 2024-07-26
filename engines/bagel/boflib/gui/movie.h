
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

#include "graphics/managed_surface.h"
#include "video/smk_decoder.h"
#include "bagel/boflib/error.h"
#include "bagel/boflib/gui/dialog.h"
#include "bagel/boflib/rect.h"

namespace Bagel {

class CBofMovie : public CBofDialog {
public:
	enum MVSTATUS { STOPPED, PAUSED, FORWARD, REVERSE};
protected:
	Graphics::ManagedSurface *_pSbuf;
	Video::SmackerDecoder *_pSmk;
	bool _bEscCanStop;
	bool _bLoop;
	bool _bStretch;
	bool _bUseNewPalette;
	bool _bBlackOutWindow;
	MVSTATUS _eMovStatus;
	Common::Rect _srcRect, _dstRect;

	virtual ErrorCode  initialize(CBofWindow *pParent);

	virtual bool    openMovie(const char *sFilename);
	virtual void    closeMovie();

	virtual void    onReSize(CBofSize *pSize);

	virtual bool    play();
	virtual bool    reverse();

	virtual void    onLButtonUp(uint32 nFlags, CBofPoint *pPoint, void * = nullptr) {
		onButtonUp(nFlags, pPoint);
	}
	virtual void    onRButtonUp(uint32 nFlags, CBofPoint *pPoint) {
		onButtonUp(nFlags, pPoint);
	}
	virtual void    onButtonUp(uint32 nFlags, CBofPoint *pPoint);
	virtual void    onPaint(CBofRect *pRect);
	virtual void    onMovieDone();
	virtual void    onClose();
	virtual void    onMainLoop();
	virtual void    onKeyHit(uint32 lKey, uint32 lRepCount);

public:

	CBofMovie(CBofWindow *pParent = nullptr, const char *pszFilename = nullptr, CBofRect *pBounds = nullptr, bool bStretch = false, bool bUseNewPalette = true, bool bBlackOutWindow = false);
	~CBofMovie();

	virtual bool        open(const char *sFilename = nullptr, CBofRect *pBounds = nullptr);

	virtual bool        play(bool bLoop,  bool bEscCanStop = true);
	virtual bool        reverse(bool bLoop,  bool bEscCanStop = true);
	virtual bool        pause();
	virtual bool        stop();

	virtual MVSTATUS    status() {
		return _eMovStatus;
	}

	virtual bool        seekToStart();
	virtual bool        seekToEnd();

	virtual uint32      getFrame();
	virtual bool        setFrame(uint32 dwFrameNum);

	virtual bool        centerRect();

	Graphics::ManagedSurface *getSmackBuffer()  {
		return _pSbuf;
	}
	Video::SmackerDecoder *getSmackMovie()   {
		return _pSmk;
	}
};

ErrorCode bofPlayMovie(CBofWindow *pParent, const char *pszMovieFile, CBofRect *pRect = nullptr);

} // namespace Bagel

#endif
