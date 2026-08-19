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

#include "common/file.h"
#include "ultima/ultima2/views/demo.h"
#include "ultima/ultima2/gfx/pic_decoder.h"

namespace Ultima {
namespace Ultima2 {
namespace Views {

#define DELAY_SECONDS 3

bool Demo::msgFocus(const FocusMessage &msg) {
	_pageCtr = 0;
	delaySeconds(DELAY_SECONDS);
	return View::msgFocus(msg);
}

void Demo::timeout() {
	_pageCtr = (_pageCtr + 1) % 8;
	redraw();
	delaySeconds(DELAY_SECONDS);
}

void Demo::draw() {
	Gfx::PicDecoder decoder;
	Common::File f;
	auto s = getSurface();
	s.clear();

	if (_pageCtr < 5) {
		static const char *FILENAMES[] = { "PICOUT", "PICTWN", "PICDNG", "PICSPA", "PICMIN" };

		if (!f.open(FILENAMES[_pageCtr]) || !decoder.loadStream(f))
			error("Could not load bitmap");
		s.blitFrom(*decoder.getSurface());
	}

	switch (_pageCtr) {
	case 0:
		s.writeString(Common::Point(0, 23), "        BATTLE STRANGE CREATURES");
		s.writeString(Common::Point(0, 24), "      ACROSS THE FACE OF THE EARTH");
		break;

	case 1:
		s.writeString(Common::Point(0, 23), "   SEARCH FOR CLUES IN CARELESS WORDS");
		s.writeString(Common::Point(0, 24), "      SPOKEN AT THE LOCAL PUB");
		break;

	case 2:
		s.writeString(Common::Point(0, 23), "   TRAVERSE DEEP DARK DEADLY DUNGEONS");
		s.writeString(Common::Point(0, 24), "      AND TALL TERRIFYING TOWERS");
		break;

	case 3:
		s.writeString(Common::Point(0, 23), "      TRAVEL THROUGHOUT THE GALAXY");
		s.writeString(Common::Point(0, 24), "  TO THE PLANETS OF OUR SOLAR SYSTEM");
		break;

	case 4:
		s.writeString(Common::Point(0, 23), "   AND CONQUER TIME ITSELF TO BATTLE");
		s.writeString(Common::Point(8, 24), "MINAX THE ENCHANTRESS");
		break;

	case 5:
		s.writeString(Common::Point(16, 10), "ORIGIN");
		s.writeString(Common::Point(12, 11), "PROUDLY PRESENTS");
		break;

	case 6:
		s.writeString(Common::Point(16, 8), "PART ][");
		s.writeString(Common::Point(9, 10), "OF THE #1 BEST SELLING");
		s.writeString(Common::Point(7, 12), "FANTASY ROLE-PLAYING GAME");
		s.writeString(Common::Point(12, 14), "BY LORD BRITISH");
		break;

	case 7:
		if (!f.open("PICDRA") || !decoder.loadStream(f))
			error("Could not load bitmap");
		s.blitFrom(*decoder.getSurface());
		break;
	}
}

} // namespace Views
} // namespace Ultima2
} // namespace Ultima
