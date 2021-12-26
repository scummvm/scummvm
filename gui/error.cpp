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

#include "common/error.h"
#include "gui/message.h"
#include "gui/error.h"

#include "common/translation.h"

namespace GUI {

void displayErrorDialog(const Common::U32String &text) {
	GUI::MessageDialog alert(text);
	alert.runModal();
}

void displayErrorDialog(const Common::Error &error, const Common::U32String &extraText) {
	Common::U32String errorText(extraText);
	errorText += Common::U32String(" ");
	errorText += _(error.getDesc());
	GUI::MessageDialog alert(errorText);
	alert.runModal();
}

} // End of namespace GUI
