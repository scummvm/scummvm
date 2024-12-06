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

#include "got/views/story.h"
#include "got/gfx/palette.h"
#include "got/utils/file.h"
#include "got/vars.h"

namespace Got {
namespace Views {

bool Story::msgFocus(const FocusMessage &msg) {
	char back[4][262];

	res_read("OPENSONG", _G(song));
	res_read("STORY1", _G(tmp_buff));
	res_read("OPENBACK", back);
	res_read("STORYPIC", back);
#if 0
	// Load the images
	_image1.create(320, 240);
	_image2.create(320, 240);

	for (int i = 0; i < 12; i++) {
		Graphics::ManagedSurface *s = (i < 6) ? &_image1 : &_image2;
		byte *destP = (byte *)s->getBasePtr(0, (i % 6) * 40);
		res_read(Common::String::format("OPENP%d", i + 1), destP);
	}
#endif
	res_read("STORYPAL", _G(pbuff));
	_G(pbuff)[2] = 0;
	_G(pbuff)[1] = 0;
	_G(pbuff)[0] = 0;
	set_palette();

	return true;
}

bool Story::msgUnfocus(const UnfocusMessage &msg) {
	_image1.clear();
	_image2.clear();
	return true;
}

bool Story::msgKeypress(const KeypressMessage &msg) {
	// Any keypress to close the view
	close();
	return true;
}

void Story::draw() {
	Graphics::ManagedSurface s = getSurface();

	//s.blitFrom(_image1);
	for (int i = 0; i < 10 * 10; ++i) {
		Graphics::ManagedSurface & img = _G(bgPics)[i];
		s.blitFrom(img, Common::Point((i % 10) * 32, (i / 10) * 16));
	}
}

bool Story::tick() {
	return true;
}

} // namespace Views
} // namespace Got
