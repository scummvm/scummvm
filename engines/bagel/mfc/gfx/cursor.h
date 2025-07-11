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

#ifndef BAGEL_MFC_GFX_CURSOR_H
#define BAGEL_MFC_GFX_CURSOR_H

#include "common/hash-str.h"
#include "common/path.h"
#include "common/formats/winexe_ne.h"
#include "graphics/managed_surface.h"
#include "graphics/wincursor.h"
#include "bagel/mfc/minwindef.h"
#include "bagel/mfc/libs/resources.h"

namespace Bagel {
namespace MFC {
namespace Gfx {

constexpr int CURSOR_W = 12;
constexpr int CURSOR_H = 20;

struct ResourceString_Hash {
	uint operator()(const LPCSTR s) const {
		return (intptr)s <= 0xffff ? (intptr)s : Common::hashit(s);
	}
};

struct ResourceString_EqualTo {
	bool operator()(const LPCSTR x, const LPCSTR y) const {
		bool xNum = (intptr)x <= 0xffff;
		bool yNum = (intptr)y <= 0xffff;

		return (xNum == yNum) && (
		           (xNum && (intptr)x == (intptr)y) ||
		           (!xNum && !strcmp(x, y))
		       );
	}
};

class Cursor {
private:
	Graphics::ManagedSurface _surface;
	Graphics::WinCursorGroup *_cursorGroup = nullptr;
	bool _isBuiltIn = false;

public:
	static void hide();

public:
	/**
	 * Constructor for predefined cursors
	 */
	Cursor(const byte *pixels);

	/**
	 * Constructor for cursor resources
	 */
	Cursor(LPCSTR cursorId);

	~Cursor();

	/**
	 * Set the cursor to be active
	 */
	void showCursor();
};

class Cursors {
private:
	Libs::Resources &_resources;
	typedef Common::HashMap<LPCSTR, Cursor *,
	        ResourceString_Hash, ResourceString_EqualTo> CursorHash;
	CursorHash _cursors;

public:
	HCURSOR _arrowCursor;
	HCURSOR _waitCursor;

public:
	Cursors(Libs::Resources &res);
	~Cursors();

	HCURSOR loadCursor(LPCSTR cursorId);
};

} // namespace Gfx
} // namespace MFC
} // namespace Bagel

#endif
