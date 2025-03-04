
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

#ifndef BAGEL_BOFLIB_GUI_BUTTON_H
#define BAGEL_BOFLIB_GUI_BUTTON_H

#include "bagel/spacebar/boflib/gui/window.h"
#include "bagel/boflib/palette.h"

namespace Bagel {
namespace SpaceBar {

// Button states
//
#define BUTTON_UP 0
#define BUTTON_DOWN 1
#define BUTTON_FOCUS 2
#define BUTTON_DISABLED 3
#define BUTTON_CLICKED 4
#define BUTTON_ON BUTTON_DOWN
#define BUTTON_OFF BUTTON_UP

#define BUTTON_CHECKED BUTTON_ON
#define BUTTON_UNCHECKED BUTTON_OFF

struct ST_COLORSCHEME {
	COLORREF _cFace;
	COLORREF _cHighlight;
	COLORREF _cShadow;
	COLORREF _cText;
	COLORREF _cTextDisabled;
	COLORREF _cOutline;
};

class CBofButton : public CBofWindow {
public:
	CBofButton();
	CBofButton(ST_COLORSCHEME *pColorScheme);
	virtual ~CBofButton();

	void loadColorScheme(ST_COLORSCHEME *pColorScheme);

	virtual ErrorCode paint(CBofRect *pRect = nullptr);

	void enable() override;
	void disable() override;

	ErrorCode setState(int nNewState, bool bRepaintNow = true);
	int getState() {
		return _nState;
	}

protected:
	void onPaint(CBofRect *pRect) override;
	void onLButtonDown(uint32 nFlags, CBofPoint *pPoint, void * = nullptr) override;
	void onLButtonUp(uint32 nFlags, CBofPoint *pPoint, void * = nullptr) override;

	COLORREF _cFaceColor;
	COLORREF _cHighlightColor;
	COLORREF _cShadowColor;
	COLORREF _cTextColor;
	COLORREF _cTextDisabledColor;
	COLORREF _cOutlineColor;

	int _nState;
};

class CBofRadioButton : public CBofButton {
public:
	virtual ErrorCode paint(CBofRect *pRect = nullptr);

protected:
	virtual void onLButtonDown(uint32 nFlags, CBofPoint *pPoint, void * = nullptr);
	virtual void onLButtonUp(uint32 nFlags, CBofPoint *pPoint, void * = nullptr);
};

class CBofCheckButton : public CBofButton {
public:
	ErrorCode SetCheck(bool bChecked);
	bool GetCheck() {
		return (_nState == BUTTON_CHECKED);
	}

	ErrorCode paint(CBofRect *pRect = nullptr) override;

protected:
	void onLButtonDown(uint32 nFlags, CBofPoint *pPoint, void * = nullptr) override;
	void onLButtonUp(uint32 nFlags, CBofPoint *pPoint, void * = nullptr) override;
};

class CBofBmpButton : public CBofWindow {
public:
	// Constructors
	CBofBmpButton();

	virtual ~CBofBmpButton();

	// NOTE: CBofBmpButton takes control of these bitmaps, so there's
	// no need for the callers to free them afterwards
	ErrorCode loadBitmaps(CBofBitmap *pUp, CBofBitmap *pDown, CBofBitmap *pFocus, CBofBitmap *pDisabled, int nMaskColor = NOT_TRANSPARENT);
	ErrorCode loadBitmaps(CBofPalette *pPalette, const char *pszUp, const char *pszDown = nullptr, const char *pszFocus = nullptr, const char *pszDisabled = nullptr, int nMaskColor = NOT_TRANSPARENT);

	ErrorCode paint(CBofRect *pRect = nullptr);

	ErrorCode setState(int nNewState, bool bRepaintNow = true);
	int getState() {
		return _nState;
	}

	CBofBitmap *getButtonBmp() {
		return _pButtonUp;
	}

protected:
	void onPaint(CBofRect *pRect) override;
	void onLButtonDown(uint32 nFlags, CBofPoint *pPoint, void * = nullptr) override;
	void onLButtonUp(uint32 nFlags, CBofPoint *pPoint, void * = nullptr) override;

	CBofBitmap *_pButtonUp;
	CBofBitmap *_pButtonDown;
	CBofBitmap *_pButtonFocus;
	CBofBitmap *_pButtonDisabled;

	CBofBitmap *_pBackground;
	int _nState;
	int _nMaskColor;
};

} // namespace SpaceBar
} // namespace Bagel

#endif
