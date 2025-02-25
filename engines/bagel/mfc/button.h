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

#ifndef BAGEL_MFC_BUTTON_H
#define BAGEL_MFC_BUTTON_H

#include "bagel/mfc/mfc_types.h"
#include "bagel/mfc/wnd.h"
#include "bagel/mfc/bitmap.h"

namespace Bagel {
namespace MFC {

enum {
	BS_OWNERDRAW,
	WS_CHILD,
	WS_VISIBLE
};

class CButton : public CWnd {
public:
	void SetButtonStyle(UINT nStyle, BOOL bRedraw = TRUE);
};

class CBitmapButton : public CButton {
protected:
	CBitmap m_bitmapDisabled;
	CBitmap m_bitmapSel;
	CBitmap m_bitmapFocus;
	CBitmap m_bitmap;

public:
	void SizeToContent();
};

} // namespace MFC
} // namespace Bagel

#endif
