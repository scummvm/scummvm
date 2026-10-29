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

#include "ultima/ultima1/views/interactions/operate_craft.h"
#include "ultima/ultima1/ultima1.h"

namespace Ultima {
namespace Ultima1 {
namespace Views {
namespace Interactions {

// Same generic dialog box geometry the Lose and Win views use
constexpr int DIALOG_X1 = 31, DIALOG_Y1 = 23, DIALOG_X2 = 287, DIALOG_Y2 = 127;
constexpr int BORDER_X1 = 34, BORDER_Y1 = 26, BORDER_X2 = 284, BORDER_Y2 = 124;

bool OperateCraft::msgFocus(const FocusMessage &msg) {
	setBounds(Graphics::Views::TextRect(0, 0, 39, 19));
	showCursor();
	return true;
}

void OperateCraft::draw() {
	auto s = getSurface();
	s.fillRect(Common::Rect(DIALOG_X1, DIALOG_Y1, DIALOG_X2, DIALOG_Y2), 0);
	s.frameRect(Common::Rect(BORDER_X1, BORDER_Y1, BORDER_X2, BORDER_Y2), _G(colors)._border);

	s.setColor(_G(colors)._text);
	s.writeString(Common::Point(6, 7), "Thou canst not determine how");
	s.writeString(Common::Point(6, 8), "to operate the craft at this");
	s.writeString(Common::Point(6, 9), "           time.");
}

bool OperateCraft::msgAction(const ActionMessage &msg) {
	finish();
	return true;
}

bool OperateCraft::msgKeypress(const KeypressMessage &msg) {
	finish();
	return true;
}

void OperateCraft::finish() {
	prompt();
	close();
}

} // namespace Interactions
} // namespace Views
} // namespace Ultima1
} // namespace Ultima
