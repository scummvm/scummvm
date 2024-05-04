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

#include "bagel/boflib/app.h"
#include "bagel/boflib/gui/scroll_bar.h"

namespace Bagel {

#define BMP_SCROLL_TIMER    9999
#define DEF_TIMER_INTERVAL  100


CBofScrollBar::CBofScrollBar() {
	_pLeftBtnUp = nullptr;
	_pRightBtnUp = nullptr;
	_pLeftBtnDn = nullptr;
	_pRightBtnDn = nullptr;
	_pThumb = nullptr;
	_nMin = 0;
	_nMax = 10;
	_nPos = 0;
	_nLineDelta = 1;
	_nPageDelta = 1;

	_pScrollText = nullptr;
	_szScrollText[0] = '\0';

	_cThumbSize.cx = 0;
	_cThumbSize.cy = 0;
	_cBkSize.cx = 0;
	_cBkSize.cy = 0;
	_cCurPoint.x = 0;
	_cCurPoint.y = 0;
	_cThumbPos.x = 0;
	_cThumbPos.y = 0;

	_nOffset = 0;
	_nScrollWidth = 0;
	_nRange = 0;
	_bMouseCaptured = false;
	_nScrollState = 0;

	_nTimerCount = DEF_TIMER_INTERVAL;
}


CBofScrollBar::~CBofScrollBar() {
	Assert(IsValidObject(this));

	_szScrollText[0] = '\0';

	if (_pScrollText != nullptr) {
		delete _pScrollText;
		_pScrollText = nullptr;
	}
	if (_pThumb != nullptr) {
		delete _pThumb;
		_pThumb = nullptr;
	}
	if (_pLeftBtnUp != nullptr) {
		delete _pLeftBtnUp;
		_pLeftBtnUp = nullptr;
	}
	if (_pRightBtnUp != nullptr) {
		delete _pRightBtnUp;
		_pRightBtnUp = nullptr;
	}
	if (_pLeftBtnDn != nullptr) {
		delete _pLeftBtnDn;
		_pLeftBtnDn = nullptr;
	}
	if (_pRightBtnDn != nullptr) {
		delete _pRightBtnDn;
		_pRightBtnDn = nullptr;
	}
}


void CBofScrollBar::onPaint(CBofRect *pDirtyRect) {
	Assert(IsValidObject(this));

	paint(pDirtyRect);
}


ErrorCode CBofScrollBar::setText(const char *pszText, int nJustify) {
	Assert(IsValidObject(this));

	_szScrollText[0] = '\0';

	if ((pszText != nullptr) && (_parent != nullptr)) {
		Common::strlcpy(_szScrollText, pszText, MAX_TEXT);

		if (_pScrollText == nullptr) {

			CBofPoint cPoint = _parent->getWindowRect().TopLeft();
			CBofRect cTempRect = _cWindowRect - cPoint;

			cTempRect -= CPoint(0, 20);
			cTempRect.right += 20;

			_pScrollText = new CBofText(&cTempRect, nJustify);
			if (_pScrollText != nullptr) {

			} else {
				ReportError(ERR_MEMORY, "Could not allocate a new CBofText");
			}
		}

		if (_pScrollText != nullptr) {
			_pScrollText->display(_parent, _szScrollText, FONT_DEFAULT_SIZE, TEXT_DEFAULT_FACE);
		}
	}

	return _errCode;
}


ErrorCode CBofScrollBar::setPos(const int nPos, bool bRepaint, bool isInitial) {
	Assert(IsValidObject(this));

	// Save old position
	int nOriginalPos = _nPos;

	_nPos = nPos;
	if (nPos < _nMin)
		_nPos = _nMin;
	if (nPos > _nMax)
		_nPos = _nMax;

	Assert(_nRange != 0);

	_cThumbPos.x = (int)(((int32)(_nScrollWidth - _cThumbSize.cx) * _nPos) / (_nRange - 1)) + _nOffset;
	_cThumbPos.y = (int)(_cBkSize.cy / 2) - (int)(_cThumbSize.cy / 2);

	if (_cThumbPos.x < 0)
		_cThumbPos.x = 0;
	if (_cThumbPos.x > (_nScrollWidth - _cThumbSize.cx + _nOffset))
		_cThumbPos.x = _nScrollWidth - _cThumbSize.cx + _nOffset;

	// If forced to repaint
	if (bRepaint) {

		paint();

	} else if (_nPos != nOriginalPos) {
		// Otherwise, only paint the thumb if it's position changed
		if (_pThumb != nullptr) {
			if (_pThumb->paintSprite(this, _cThumbPos) == false) {
				ReportError(ERR_UNKNOWN, "_pThumb->paintSprite() failed");
			}
		}
	}

	// If the thumb actually moved, then tell our parent about it
	if (_nPos != nOriginalPos && !isInitial) {
		_parent->onBofScrollBar(this, _nPos);
	}

	return _errCode;
}


void CBofScrollBar::getScrollRange(int &nMin, int &nMax) {
	Assert(IsValidObject(this));

	nMin = _nMin;
	nMax = _nMax;
}


void CBofScrollBar::setScrollRange(int nMin, int nMax, bool bRepaint) {
	Assert(IsValidObject(this));

	_nMin = nMin;
	_nMax = nMax;

	_nRange = _nMax - _nMin + 1;

	// Should we repaint the scroll bar now?
	if (bRepaint) {
		paint(nullptr);
	}
}


ErrorCode CBofScrollBar::loadBitmaps(const char *pszBack, const char *pszThumb, const char *pszLeftBtnUp, const char *pszRightBtnUp, const char *pszLeftBtnDn, const char *pszRightBtnDn) {
	Assert(IsValidObject(this));

	if ((pszBack != nullptr) && (pszThumb != nullptr)) {
		_cLeftBtnRect.SetRect(0, 0, 0, 0);
		_cRightBtnRect.SetRect(0, 0, 0, 0);

		if (_pThumb != nullptr) {
			_pThumb->eraseSprite(this);
			delete _pThumb;
			_pThumb = nullptr;
		}

		killBackdrop();
		setBackdrop(pszBack);

		CBofPalette *pPalette = CBofApp::getApp()->getPalette();

		_cBkSize = _pBackdrop->getSize();
		_nScrollWidth = _cBkSize.cx;

		if ((_pThumb = new CBofSprite) != nullptr) {
			if (_pThumb->loadSprite(pszThumb) != false) {
				_pThumb->setMaskColor(COLOR_WHITE);
				_cThumbSize = _pThumb->getSize();
			}
		}

		if (_pLeftBtnUp != nullptr) {
			delete _pLeftBtnUp;
			_pLeftBtnUp = nullptr;
		}

		CBofPoint cPoint;
		if (pszLeftBtnUp != nullptr) {
			if ((_pLeftBtnUp = new CBofBitmap(pszLeftBtnUp, pPalette)) != nullptr) {
				cPoint.x = 0;
				cPoint.y = (_pBackdrop->height() / 2) - (_pLeftBtnUp->height() / 2);

				_cLeftBtnRect = _pLeftBtnUp->getRect() + cPoint;

				_nOffset = _pLeftBtnUp->width();
				_nScrollWidth -= _nOffset;

			} else {
				ReportError(ERR_MEMORY, "Could not allocate a new CBofBitmap(%s)", pszLeftBtnUp);
			}
		}

		if (_pRightBtnUp != nullptr) {
			delete _pRightBtnUp;
			_pRightBtnUp = nullptr;
		}

		if (pszRightBtnUp != nullptr) {
			if ((_pRightBtnUp = new CBofBitmap(pszRightBtnUp, pPalette)) != nullptr) {
				cPoint.x = _pBackdrop->width() - _pRightBtnUp->width();
				cPoint.y = (_pBackdrop->height() / 2) - (_pRightBtnUp->height() / 2);
				_cRightBtnRect = _pLeftBtnUp->getRect() + cPoint;

				_nScrollWidth -= _cRightBtnRect.width();

			} else {
				ReportError(ERR_MEMORY, "Could not allocate a new CBofBitmap(%s)", pszRightBtnUp);
			}
		}

		if (_pLeftBtnDn != nullptr) {
			delete _pLeftBtnDn;
			_pLeftBtnDn = nullptr;
		}
		if (pszLeftBtnDn != nullptr) {
			if ((_pLeftBtnDn = new CBofBitmap(pszLeftBtnDn, pPalette)) != nullptr) {

			} else {
				ReportError(ERR_MEMORY, "Could not allocate a new CBofBitmap(%s)", pszLeftBtnDn);
			}
		}

		if (_pRightBtnDn != nullptr) {
			delete _pRightBtnDn;
			_pRightBtnDn = nullptr;
		}
		if (pszRightBtnDn != nullptr) {
			if ((_pRightBtnDn = new CBofBitmap(pszRightBtnDn, pPalette)) != nullptr) {

			} else {
				ReportError(ERR_MEMORY, "Could not allocate a new CBofBitmap(%s)", pszRightBtnDn);
			}
		}
	}

	return _errCode;
}


ErrorCode CBofScrollBar::paint(CBofRect *pDirtyRect) {
	Assert(IsValidObject(this));

	if (!ErrorOccurred()) {
		CBofRect cRect(0, 0, _cRect.width() - 1, _cRect.height() - 1);
		CBofPoint cPoint(0, 0);

		if (pDirtyRect == nullptr) {
			pDirtyRect = &cRect;
		}

		CBofPalette *pPalette = CBofApp::getApp()->getPalette();

		//
		// This function needs to be optimized to paint only the section that is
		// invalidated.  Right now it just repaints the entire scroll bar each time.
		//

		if ((_pBackdrop != nullptr) && (_pThumb != nullptr)) {
			// Do all painting offscreen
			CBofBitmap *pBmp = new CBofBitmap(_cBkSize.cx, _cBkSize.cy, pPalette);
			if (pBmp != nullptr) {
				_pBackdrop->paint(pBmp, 0, 0, nullptr, COLOR_WHITE);

				if ((_nScrollState == 1) && (_pLeftBtnDn != nullptr)) {
					cPoint = _cLeftBtnRect.TopLeft();
					_pLeftBtnDn->paint(pBmp, cPoint.x, cPoint.y, nullptr, COLOR_WHITE);

				} else if (_pLeftBtnUp != nullptr) {
					cPoint = _cLeftBtnRect.TopLeft();
					_pLeftBtnUp->paint(pBmp, cPoint.x, cPoint.y, nullptr, COLOR_WHITE);
				}

				if ((_nScrollState == 4) && (_pRightBtnDn != nullptr)) {
					cPoint = _cRightBtnRect.TopLeft();
					_pRightBtnDn->paint(pBmp, cPoint.x, cPoint.y, nullptr, COLOR_WHITE);

				} else if (_pRightBtnUp != nullptr) {
					cPoint = _cRightBtnRect.TopLeft();
					_pRightBtnUp->paint(pBmp, cPoint.x, cPoint.y, nullptr, COLOR_WHITE);
				}

				_cThumbPos.x = (int)(((int32)(_nScrollWidth - _cThumbSize.cx) * _nPos) / (_nRange - 1)) + _nOffset;
				_cThumbPos.y = (int)(_cBkSize.cy / 2) - (int)(_cThumbSize.cy / 2);

				if (_cThumbPos.x < 0)
					_cThumbPos.x = 0;
				if (_cThumbPos.x > (_nScrollWidth - _cThumbSize.cx + _nOffset))
					_cThumbPos.x = _nScrollWidth - _cThumbSize.cx + _nOffset;

				_pThumb->paintSprite(pBmp, _cThumbPos);

				// now we can paint the offscreen buffer to the screen
				pBmp->paint(this, 0, 0);

				delete pBmp;
			} else {
				ReportError(ERR_MEMORY, "Could not allocate a new CBofBitmap(%d x %d))", _cBkSize.cx, _cBkSize.cy);
			}
		}

		if ((_pScrollText != nullptr) && (_parent != nullptr)) {
			_pScrollText->display(_parent, _szScrollText, FONT_DEFAULT_SIZE, TEXT_DEFAULT_FACE);
		}
	}

	return _errCode;
}


void CBofScrollBar::onLButtonDown(uint32 nFlags, CBofPoint *pPoint, void *) {
	Assert(IsValidObject(this));

	CBofRect cLeftPageRect, cRightPageRect;

	bool bDoNothing = false;

	cLeftPageRect.SetRect(_nOffset, 0, (_nScrollWidth / _nRange) * _nPos + _nOffset - 1, _cBkSize.cy - 1);
	cRightPageRect.SetRect(((_nScrollWidth / _nRange) * _nPos) + _nOffset + _cThumbSize.cx, 0, _nOffset + _nScrollWidth - 1, _cBkSize.cy - 1);

	_cCurPoint = *pPoint;

	if (_pLeftBtnUp != nullptr && _cLeftBtnRect.PtInRect(*pPoint)) {
		// Let timer know what happened
		_nScrollState = 1;

		// Set new thumb position
		setPos(_nPos - _nLineDelta, true);

	} else if (_pThumb->getRect().PtInRect(*pPoint)) {
		_nScrollState = 5;

	} else if (cLeftPageRect.PtInRect(*pPoint)) {
		_nScrollState = 2;

		// Set new thumb position
		setPos(_nPos - _nPageDelta, true);

	} else if (cRightPageRect.PtInRect(*pPoint)) {
		_nScrollState = 3;

		// Set new thumb position
		setPos(_nPos + _nPageDelta, true);

	} else if (_pRightBtnUp != nullptr && _cRightBtnRect.PtInRect(*pPoint)) {
		// Let timer know what happened
		_nScrollState = 4;

		// Set new thumb position
		setPos(_nPos + _nLineDelta, true);

	} else {
		bDoNothing = true;
	}

	if (!bDoNothing) {
		_bMouseCaptured = true;
		setCapture();
		if (_nScrollState != 5)
			setTimer(BMP_SCROLL_TIMER, _nTimerCount);
	}

	CBofWindow::onLButtonDown(nFlags, pPoint);
}


int CBofScrollBar::pointToPos(CBofPoint *pPoint) {
	Assert(IsValidObject(this));
	Assert(pPoint != nullptr);

	int nPos = _nPos;

	if (_cRect.PtInRect(*pPoint)) {
		nPos = (pPoint->x - _nOffset) / (int)(_nScrollWidth / _nRange);
	}

	return nPos;
}


void CBofScrollBar::onLButtonUp(uint32 nFlags, CBofPoint *pPoint, void *) {
	Assert(IsValidObject(this));

	if (_bMouseCaptured) {
		killTimer(BMP_SCROLL_TIMER);
		_bMouseCaptured = false;
		releaseCapture();

		int x, y;

		switch (_nScrollState) {
		case 5:
			setPos(pointToPos(pPoint));
			break;

		case 1:
			if (_pLeftBtnUp != nullptr) {
				x = 0;
				y = (int)(_cBkSize.cy / 2) - (int)(_cLeftBtnRect.height() / 2);
				_pLeftBtnUp->paint(this, x, y, nullptr, COLOR_WHITE);
			}
			break;

		case 4:
			if (_pRightBtnUp != nullptr) {
				x = _cBkSize.cx - _cRightBtnRect.width();
				y = (int)(_cBkSize.cy / 2) - (int)(_cRightBtnRect.height() / 2);
				_pRightBtnUp->paint(this, x, y, nullptr, COLOR_WHITE);
			}
			break;

		default:
			break;
		}

		_nScrollState = 0;
	}

	CBofWindow::onLButtonUp(nFlags, pPoint);
}


void CBofScrollBar::onMouseMove(uint32 nFlags, CBofPoint *pPoint, void *) {
	Assert(IsValidObject(this));

	if (_bMouseCaptured) {
		_cCurPoint = *pPoint;

		if (_nScrollState == 5) {
			setPos(pointToPos(pPoint));
		}
	}

	CBofWindow::onMouseMove(nFlags, pPoint);
}


void CBofScrollBar::setRepeatTimer(uint32 nTimerInt) {
	Assert(IsValidObject(this));

	_nTimerCount = nTimerInt;
}


void CBofScrollBar::onTimer(uint32 nWhichTimer) {
	Assert(IsValidObject(this));

	CBofRect cLeftPageRect, cRightPageRect;

	cLeftPageRect.SetRect(_nOffset, 0, (_nScrollWidth / _nRange) * _nPos + _nOffset - 1, _cBkSize.cy - 1);
	cRightPageRect.SetRect(((_nScrollWidth / _nRange) * _nPos) + _nOffset + _cThumbSize.cx, 0, _nOffset + _nScrollWidth - 1, _cBkSize.cy - 1);

	if (nWhichTimer == BMP_SCROLL_TIMER) {
		if ((_nScrollState == 1) && _cLeftBtnRect.PtInRect(_cCurPoint)) {
			lineLeft();

		} else if ((_nScrollState == 2) && cLeftPageRect.PtInRect(_cCurPoint)) {
			pageLeft();

		} else if ((_nScrollState == 3) && cRightPageRect.PtInRect(_cCurPoint)) {
			pageRight();

		} else if ((_nScrollState == 4) && _cRightBtnRect.PtInRect(_cCurPoint)) {
			lineRight();
		}
	}
}

} // namespace Bagel
