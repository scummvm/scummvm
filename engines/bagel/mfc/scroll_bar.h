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

#ifndef BAGEL_HODJNPODJ_MFC_SCROLL_BAR_H
#define BAGEL_HODJNPODJ_MFC_SCROLL_BAR_H

#include "bagel/mfc/mfc_types.h"
#include "bagel/mfc/wnd.h"

namespace Bagel {
namespace MFC {

/*
 * Scroll Bar Commands
 */
enum {
	SB_LINEUP        = 0,
	SB_LINELEFT      = 0,
	SB_LINEDOWN      = 1,
	SB_LINERIGHT     = 1,
	SB_PAGEUP        = 2,
	SB_PAGELEFT      = 2,
	SB_PAGEDOWN      = 3,
	SB_PAGERIGHT     = 3,
	SB_THUMBPOSITION = 4,
	SB_THUMBTRACK    = 5,
	SB_TOP           = 6,
	SB_LEFT          = 6,
	SB_BOTTOM        = 7,
	SB_RIGHT         = 7,
	SB_ENDSCROLL     = 8
};

class CScrollBar : public CWnd {
public:
	void SetScrollRange(int nBar, int nMinPos, int nMaxPos, BOOL bRedraw = TRUE);
	void GetScrollRange(int *nMinPos, int *nMaxPos);
	int SetScrollPos(int nBar, int nPos, BOOL bRedraw = TRUE);
	int GetScrollPos() const;
};

} // namespace MFC
} // namespace Bagel

#endif
