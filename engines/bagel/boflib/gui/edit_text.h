
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

#if BOF_MAC
#include <TextEdit.h>
#endif

namespace Bagel {

class CBofEditText : public CBofWindow {
private:
	CBofString _text;
	size_t _cursorPos = 0;
	uint32 _lastCursorTime = 0;
	bool _cursorVisible = false;

protected:
	void OnLButtonDown(uint32 nFlags, CBofPoint *pPoint, void * = nullptr) override;
	void OnKeyHit(uint32 lKey, uint32 lRepCount) override;

public:
	CBofEditText() {}

	CBofEditText(const char *pszName, int x = 0, int y = 0, int nWidth = USE_DEFAULT, int nHeight = USE_DEFAULT, CBofWindow *pParent = nullptr);

	ERROR_CODE Create(const char *pszName, int x = 0, int y = 0, int nWidth = USE_DEFAULT, int nHeight = USE_DEFAULT, CBofWindow *pParent = nullptr, uint32 nControlID = 0);
	ERROR_CODE Create(const char *pszName, CBofRect *pRect = nullptr, CBofWindow *pParent = nullptr, uint32 nControlID = 0);

	CBofString GetText() const {
		return _text;
	}

	void SetText(const char *pszString);

	void OnPaint(CBofRect *pRect) override;
};

} // namespace Bagel

#endif
