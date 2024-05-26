
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

#ifndef BAGEL_BOFLIB_GUI_SCROLL_BAR_H
#define BAGEL_BOFLIB_GUI_SCROLL_BAR_H

#include "bagel/boflib/gui/window.h"
#include "bagel/boflib/gfx/sprite.h"
#include "bagel/boflib/gfx/text.h"

namespace Bagel {

#define BSB_LEFT 800
#define BSB_RIGHT 801
#define BSB_LINE_LEFT 802
#define BSB_LINE_RIGHT 803
#define BSB_PAGE_LEFT 804
#define BSB_PAGE_RIGHT 805
#define BSB_THUMB_POS 806
#define BSB_THUMB_TRACK 807

#define MAX_TEXT 128

class CBofScrollBar : public CBofWindow {
public:
	CBofScrollBar();

	virtual ~CBofScrollBar();

	// Implementation
	//

	ErrorCode loadBitmaps(const char *pszBack, const char *pszThumb, const char *pszLeftUp = nullptr, const char *pszRightUp = nullptr, const char *pszLeftDown = nullptr, const char *pszRightDown = nullptr);

	ErrorCode setPos(int nPos, bool bRepaint = true, bool isInitial = false);
	int getPos() const {
		return _nPos;
	}

	ErrorCode lineLeft() {
		return setPos(_nPos - _nLineDelta);
	}
	ErrorCode lineRight() {
		return setPos(_nPos + _nLineDelta);
	}
	ErrorCode pageLeft() {
		return setPos(_nPos - _nPageDelta);
	}
	ErrorCode pageRight() {
		return setPos(_nPos + _nPageDelta);
	}

	ErrorCode home() {
		return setPos(_nMin);
	}
	ErrorCode end() {
		return setPos(_nMax);
	}

	int getScrollMin() const {
		return _nMin;
	}
	int getScrollMax() const {
		return _nMax;
	}

	void setLineDelta(const int nDelta) {
		_nLineDelta = nDelta;
	}
	int getLineDelta() const {
		return _nLineDelta;
	}

	void setPageDelta(const int nDelta) {
		_nPageDelta = nDelta;
	}
	int getPageDelta() const {
		return _nPageDelta;
	}

	void getScrollRange(int &nMin, int &nMax);
	void setScrollRange(int nMin, int nMax, bool bRepaint = true);

	ErrorCode setText(const char *pszText, int nFlags = JUSTIFY_CENTER);

	void setRepeatTimer(uint32 nMilliSeconds);
	ErrorCode paint(CBofRect *pRect = nullptr);

protected:
	int pointToPos(CBofPoint *pPoint);

	void onPaint(CBofRect *pDirtyRect) override;
	void onLButtonDown(uint32 nFlags, CBofPoint *pPoint, void * = nullptr) override;
	void onLButtonUp(uint32 nFlags, CBofPoint *pPoint, void * = nullptr) override;
	void onMouseMove(uint32 nFlags, CBofPoint *pPoint, void * = nullptr) override;
	void onTimer(uint32) override;

	//
	// Data members
	//
	CBofBitmap *_pLeftBtnUp;
	CBofBitmap *_pRightBtnUp;
	CBofBitmap *_pLeftBtnDn;
	CBofBitmap *_pRightBtnDn;
	CBofSprite *_pThumb;

	CBofRect _cLeftBtnRect;
	CBofRect _cRightBtnRect;

	int _nMin;
	int _nMax;
	int _nPos;
	int _nLineDelta;
	int _nPageDelta;

	CBofText *_pScrollText;
	char _szScrollText[MAX_TEXT];

	CBofSize _cThumbSize;
	CBofSize _cBkSize;

	int _nOffset;
	int _nScrollWidth;
	int _nRange;
	bool _bMouseCaptured;
	CBofPoint _cCurPoint;
	CBofPoint _cThumbPos;
	int _nScrollState;
	uint32 _nTimerCount;
};

} // namespace Bagel

#endif
