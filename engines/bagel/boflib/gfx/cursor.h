
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

#ifndef BAGEL_BOFLIB_GFX_CURSOR_H
#define BAGEL_BOFLIB_GFX_CURSOR_H

#include "graphics/managed_surface.h"
#include "bagel/boflib/boffo.h"
#include "bagel/boflib/error.h"
#include "bagel/boflib/object.h"
#include "bagel/boflib/stack.h"

namespace Bagel {

typedef Graphics::ManagedSurface HCURSOR;

class CBofCursor : public CBofObject, public CBofError {
protected:
	HCURSOR _currentCursor;

public:
	CBofCursor() {}
	CBofCursor(SHORT nResID);

	~CBofCursor();

#if 0
	CBofCursor(HCURSOR hCurs);
	HCURSOR operator=(HCURSOR hCurs);

	static HCURSOR GetCurrent() { return (m_hCurrentCurs); }
	static VOID SetCurrent(HCURSOR h) { m_hCurrentCurs = h; }

	HCURSOR GetWinCursor() { return (m_hCurs); }
	VOID SetWinCursor(HCURSOR hCursor) { m_hCurs = hCursor; }
#endif

	ERROR_CODE Load(SHORT nResId);
	ERROR_CODE UnLoad();

	ERROR_CODE Set();

	static VOID Show();
	static VOID Hide();
};

} // namespace Bagel

#endif
