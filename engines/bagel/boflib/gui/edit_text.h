
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

#ifndef BAGEL_BOFLIB_GUI_EDIT_TEXT_H
#define BAGEL_BOFLIB_GUI_EDIT_TEXT_H

#include "bagel/boflib/gui/window.h"
#include "bagel/boflib/string.h"

namespace Bagel {

class CBofEditText : public CBofWindow {
private:
	CBofString _text;
	size_t _cursorPos = 0;

protected:
	void onLButtonDown(uint32 nFlags, CBofPoint *pPoint, void * = nullptr) override;
	void onKeyHit(uint32 lKey, uint32 lRepCount) override;

public:
	CBofEditText() {}

	CBofEditText(const char *pszName, int x, int y, int nWidth, int nHeight, CBofWindow *pParent);

	ErrorCode create(const char *pszName, int x, int y, int nWidth, int nHeight, CBofWindow *pParent, uint32 nControlID = 0) override;
	ErrorCode create(const char *pszName, CBofRect *pRect, CBofWindow *pParent, uint32 nControlID = 0) override;

	CBofString getText() const {
		return _text;
	}

	void setText(const char *pszString);

	void onPaint(CBofRect *pRect) override;
};

} // namespace Bagel

#endif
