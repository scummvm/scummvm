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

#include "ultima/ultima0/views/intro.h"

namespace Ultima {
namespace Ultima0 {
namespace Views {

void Intro::draw() {
	auto s = getSurface();
	s.clear();
	s.writeString(Common::Point(5, 1), "Many, many, many years ago the");
	s.writeString(Common::Point(0, 3), "Dark Lord Mondain, Archfoe of British,");
	s.writeString(Common::Point(0, 5), "traversed the lands of Akalabeth,");
	s.writeString(Common::Point(0, 7), "spreading evil and death as he passed.");
	s.writeString(Common::Point(0, 9), "By the time Mondain was driven from the");
	s.writeString(Common::Point(0, 11), "land by British, bearer of the White");
	s.writeString(Common::Point(0, 13), "Light, he had done much damage unto");
	s.writeString(Common::Point(0, 15), "the lands.");
	s.writeString(Common::Point(0, 17), "`Tis thy duty to help rid Akalabeth of");
	s.writeString(Common::Point(0, 19), "the foul beasts which infest it,");
	s.writeString(Common::Point(0, 21), "while trying to stay alive!!!");

	s.writeString(Common::Point(20, 24), "Press any Key to Continue",
		Graphics::kTextAlignCenter);
}

} // namespace Views
} // namespace Ultima0
} // namespace Ultima
