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

namespace Ultima {
namespace Ultima1 {
namespace Views {
namespace Interactions {

bool OperateCraft::msgFocus(const FocusMessage &msg) {
	setBounds(Common::Rect(0, 0, 0, 0));

	writeString("Thou canst not determine how\n");
	writeString("to operate the craft at this\n");
	writeString("time.\n");

	showCursor();
	return true;
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
	close();
}

} // namespace Interactions
} // namespace Views
} // namespace Ultima1
} // namespace Ultima
