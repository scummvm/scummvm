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

#include "mm/mm1/views_enh/title.h"
#include "mm/mm1/globals.h"
#include "mm/shared/utils/xeen_font.h"

namespace MM {
namespace MM1 {
namespace ViewsEnh {

#define ENHANCED_Y 150
static const char *ENHANCED = "Enhanced";

bool Title::msgFocus(const FocusMessage &msg) {
	Views::Title::msgFocus(msg);

	// Draw the Enhanced word on the title screen
	XeenFont &font = g_globals->_fontNormal;
	size_t strWidth = font.getStringWidth(ENHANCED);
	Graphics::ManagedSurface s(strWidth, 9);
	s.clear(255);
	s.setTransparentColor(255);
	font.drawString(&s, ENHANCED, 0, 0, strWidth, 0);

	Graphics::ManagedSurface &dest = _screens[1];
	dest.blitFrom(s, Common::Rect(0, 0, s.w, s.h),
		Common::Rect(320 - strWidth * 2 - 10, ENHANCED_Y,
			320 - 10, ENHANCED_Y + 9 * 2));

	return true;
}

} // namespace ViewsEnh
} // namespace MM1
} // namespace MM
