
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

#ifndef BAGEL_BOFLIB_GFX_DISPLAY_BUTTON_H
#define BAGEL_BOFLIB_GFX_DISPLAY_BUTTON_H

#include "bagel/boflib/app.h"
#include "bagel/boflib/gui/display_window.h"
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

typedef struct {
	RGBCOLOR m_cFace;
	RGBCOLOR m_cHighlight;
	RGBCOLOR m_cShadow;
	RGBCOLOR m_cText;
	RGBCOLOR m_cTextDisabled;
	RGBCOLOR m_cOutline;

} ST_COLORSCHEME;

class CBofDisplayButton : public CBofDisplayWindow {
protected:
	virtual ERROR_CODE OnPaint(CBofBitmap *pDestBmp, CBofRect *pRect);

	virtual VOID OnLButtonDown(UINT nFlags, CBofPoint *pPoint);
	virtual VOID OnLButtonUp(UINT nFlags, CBofPoint *pPoint);

	RGBCOLOR m_cFaceColor;
	RGBCOLOR m_cHighlightColor;
	RGBCOLOR m_cShadowColor;
	RGBCOLOR m_cTextColor;
	RGBCOLOR m_cTextDisabledColor;
	RGBCOLOR m_cOutlineColor;

	INT m_nState;

public:
	CBofDisplayButton();
	CBofDisplayButton(ST_COLORSCHEME *pColorScheme);
	virtual ~CBofDisplayButton();

	VOID LoadColorScheme(ST_COLORSCHEME *pColorScheme);

	virtual VOID Enable();
	virtual VOID Disable();

	ERROR_CODE SetState(INT nNewState, bool bRepaintNow = TRUE);
	INT GetState() {
		return (m_nState);
	}
};

} // namespace Bagel

#endif
