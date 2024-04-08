
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

#include "bagel/boflib/gui/window.h"
#include "bagel/boflib/gfx/palette.h"

namespace Bagel {

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
	RGBCOLOR m_cFace;
	RGBCOLOR m_cHighlight;
	RGBCOLOR m_cShadow;
	RGBCOLOR m_cText;
	RGBCOLOR m_cTextDisabled;
	RGBCOLOR m_cOutline;
};

class CBofButton : public CBofWindow {
public:
	CBofButton();
	CBofButton(ST_COLORSCHEME *pColorScheme);
	virtual ~CBofButton();

	void LoadColorScheme(ST_COLORSCHEME *pColorScheme);

	virtual ERROR_CODE Paint(CBofRect *pRect = nullptr);

	virtual void Enable() override;
	virtual void Disable() override;

	ERROR_CODE SetState(int nNewState, bool bRepaintNow = TRUE);
	int GetState() {
		return (m_nState);
	}

protected:
	void OnPaint(CBofRect *pRect) override;
	void OnLButtonDown(uint32 nFlags, CBofPoint *pPoint, void * = nullptr) override;
	void OnLButtonUp(uint32 nFlags, CBofPoint *pPoint, void * = nullptr) override;

	RGBCOLOR m_cFaceColor;
	RGBCOLOR m_cHighlightColor;
	RGBCOLOR m_cShadowColor;
	RGBCOLOR m_cTextColor;
	RGBCOLOR m_cTextDisabledColor;
	RGBCOLOR m_cOutlineColor;

	int m_nState;
};

class CBofRadioButton : public CBofButton {
public:
	virtual ERROR_CODE Paint(CBofRect *pRect = nullptr);

protected:
	virtual void OnLButtonDown(uint32 nFlags, CBofPoint *pPoint, void * = nullptr);
	virtual void OnLButtonUp(uint32 nFlags, CBofPoint *pPoint, void * = nullptr);
};

class CBofCheckButton : public CBofButton {
public:
	ERROR_CODE SetCheck(bool bChecked);
	bool GetCheck() {
		return (m_nState == BUTTON_CHECKED);
	}

	virtual ERROR_CODE Paint(CBofRect *pRect = nullptr);

protected:
	virtual void OnLButtonDown(uint32 nFlags, CBofPoint *pPoint, void * = nullptr);
	virtual void OnLButtonUp(uint32 nFlags, CBofPoint *pPoint, void * = nullptr);
};

class CBofBmpButton : public CBofWindow {
public:
	// Constructors
	CBofBmpButton();

	virtual ~CBofBmpButton();

	// NOTE: CBofBmpButton takes control of these bitmaps, so don't use
	// them after you give them to LoadBitmaps.
	//
	ERROR_CODE LoadBitmaps(CBofBitmap *pUp, CBofBitmap *pDown, CBofBitmap *pFocus, CBofBitmap *pDisabled, int nMaskColor = NOT_TRANSPARENT);
	ERROR_CODE LoadBitmaps(CBofPalette *pPalette, const char *pszUp, const char *pszDown = nullptr, const char *pszFocus = nullptr, const char *pszDisabled = nullptr, int nMaskColor = NOT_TRANSPARENT);

	ERROR_CODE Paint(CBofRect *pRect = nullptr);

	ERROR_CODE SetState(int nNewState, bool bRepaintNow = TRUE);
	int GetState() {
		return (m_nState);
	}

	CBofBitmap *GetButtonBmp() {
		return m_pButtonUp;
	}

protected:
	virtual void OnPaint(CBofRect *pRect);
	virtual void OnLButtonDown(uint32 nFlags, CBofPoint *pPoint, void * = nullptr);
	virtual void OnLButtonUp(uint32 nFlags, CBofPoint *pPoint, void * = nullptr);

	CBofBitmap *m_pButtonUp;
	CBofBitmap *m_pButtonDown;
	CBofBitmap *m_pButtonFocus;
	CBofBitmap *m_pButtonDisabled;

	CBofBitmap *m_pBackground;
	int m_nState;
	int m_nMaskColor;
};

} // namespace Bagel

#endif
