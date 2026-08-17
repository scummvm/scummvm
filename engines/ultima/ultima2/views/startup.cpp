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
#include "ultima/ultima2/views/startup.h"
#include "ultima/ultima2/gfx/pic_decoder.h"

namespace Ultima {
namespace Ultima2 {
namespace Views {

#define DELAY_SECONDS 3

bool Startup::msgFocus(const FocusMessage &msg) {
	_pageCtr = 0;
	delaySeconds(DELAY_SECONDS);
	return View::msgFocus(msg);
}

void Startup::timeout() {
	if (++_pageCtr == 3) {
		showTitle();
	} else {
		redraw();
		delaySeconds(DELAY_SECONDS);
	}
}

void Startup::draw() {
	Gfx::PicDecoder decoder;
	Common::File f;
	auto s = getSurface();
	s.clear();

	switch (_pageCtr) {
	case 0:
		s.writeString(Common::Point(16, 10), "ORIGIN");
		s.writeString(Common::Point(12, 11), "PROUDLY PRESENTS");
		break;
	case 1:
		s.writeString(Common::Point(16, 8), "PART ][");
		s.writeString(Common::Point(9, 10), "OF THE #1 BEST SELLING");
		s.writeString(Common::Point(7, 12), "FANTASY ROLE-PLAYING GAME");
		s.writeString(Common::Point(12, 14), "BY LORD BRITISH");
		break;
	case 2:
		if (!f.open("PICDRA") || !decoder.loadStream(f))
			error("Could not load bitmap");
		s.blitFrom(*decoder.getSurface());
		break;
	case 3:
		showTitle();
	}
}

} // namespace Views
} // namespace Ultima2
} // namespace Ultima
