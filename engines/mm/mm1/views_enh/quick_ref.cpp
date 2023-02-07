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

#include "mm/mm1/views_enh/quick_ref.h"
#include "mm/mm1/maps/maps.h"
#include "mm/mm1/globals.h"

namespace MM {
namespace MM1 {
namespace ViewsEnh {

#define COLUMN_NUM 15
#define COLUMN_NAME 35
#define COLUMN_CLASS 118
#define COLUMN_LEVEL 162
#define COLUMN_HP 180
#define COLUMN_SP 216
#define COLUMN_AC 250
#define COLUMN_CONDITION 278


QuickRef::QuickRef() : ScrollPopup("QuickRef") {
	setBounds(Common::Rect(0, 0, 320, 146));
}

void QuickRef::draw() {
	ScrollPopup::draw();

	// Title
	writeString(0, 0, STRING["enhdialogs.quickref.title"], ALIGN_MIDDLE);

	// Header line
	writeString(COLUMN_NUM, 20, "#");
	writeString(COLUMN_NAME, 20, STRING["enhdialogs.quickref.headers.name"]);
	writeString(COLUMN_CLASS, 20, STRING["enhdialogs.quickref.headers.class"]);
	writeString(COLUMN_LEVEL, 20, STRING["enhdialogs.quickref.headers.level"], ALIGN_RIGHT);
	writeString(COLUMN_HP, 20, " ");
	writeString(STRING["enhdialogs.quickref.headers.hp"]);
	writeString(COLUMN_SP, 20, " ");
	writeString(STRING["enhdialogs.quickref.headers.sp"]);
	writeString(COLUMN_AC, 20, STRING["enhdialogs.quickref.headers.ac"]);
	writeString(COLUMN_CONDITION, 20, STRING["enhdialogs.quickref.headers.cond"]);

	// Write out individual character lines
	for (uint i = 0; i < g_globals->_party.size(); ++i)
		writeCharacterLine(i);
}

void QuickRef::writeCharacterLine(int charNum) {
	const Character &c = g_globals->_party[charNum];
	const int yp = 30 + (charNum * 10);

	writeChar(COLUMN_NUM, yp, '1' + charNum);
	writeChar(')');

	writeString(COLUMN_NAME, yp, c._name);

	Common::String classStr = STRING[Common::String::format(
		"stats.classes.%d", c._class)];
	writeString(COLUMN_CLASS, yp, Common::String(
		classStr.c_str(), classStr.c_str() + 3));

	writeString(COLUMN_LEVEL, yp,
		Common::String::format("%d", c._level), ALIGN_RIGHT);
}

} // namespace Views
} // namespace MM1
} // namespace MM
