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

#ifndef HODJNPODJ_GFX_CURSOR_H
#define HODJNPODJ_GFX_CURSOR_H

#include "common/formats/winexe_ne.h"
#include "bagel/afxwin.h"
#include "bagel/boflib/cursor_data.h"

namespace Bagel {
namespace HodjNPodj {

enum {
	IDC_RULES_OKAY    = 900,
	IDC_RULES_ARROWDN = 901,
	IDC_RULES_ARROWUP = 902,
	IDC_RULES_INVALID = 903
};

class Cursor {
private:
	Common::NEResources _resources;
	int _cursorId = IDC_NONE;

	void setArrowCursor();
	void setCursorResource();

public:
	void loadCursors();

	void setCursor(int cursorId);
	void showCursor(bool visible) {
		setCursor(visible ? 1 : 0);
	}
};

} // namespace HodjNPodj
} // namespace Bagel

#endif
