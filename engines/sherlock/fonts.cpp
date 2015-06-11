/* ScummVM - Graphic Adventure Engine
 *
 * ScummVM is the legal property of its developers, whose names
 * are too numerous to list here. Please refer to the COPYRIGHT
 * file distributed with this source distribution.
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License
 * as published by the Free Software Foundation; either version 2
 * of the License, or (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.
 *
 */

#include "common/system.h"
#include "common/platform.h"
#include "sherlock/fonts.h"
#include "sherlock/image_file.h"
#include "sherlock/surface.h"

namespace Sherlock {

Common::Platform Fonts::_platform;
ImageFile *Fonts::_font;
int Fonts::_fontNumber;
int Fonts::_fontHeight;

void Fonts::init(Common::Platform platform) {
	_font = nullptr;
	_platform = platform;
}

void Fonts::free() {
	delete _font;
}

void Fonts::setFont(int fontNumber) {
	_fontNumber = fontNumber;

	if (_platform == Common::kPlatform3DO)
		return;

	Common::String fname = Common::String::format("FONT%d.VGS", fontNumber + 1);

	// Discard any previous font and read in new one
	delete _font;
	_font = new ImageFile(fname);

	// Iterate through the frames to find the tallest font character
	_fontHeight = 0;
	for (uint idx = 0; idx < _font->size(); ++idx)
		_fontHeight = MAX((uint16)_fontHeight, (*_font)[idx]._frame.h);
}

void Fonts::writeString(Surface *surface, const Common::String &str,
		const Common::Point &pt, int overrideColor) {
	Common::Point charPos = pt;

	for (const char *c = str.c_str(); *c; ++c) {
		if (*c == ' ')
			charPos.x += 5;
		else {
			assert(Common::isPrint(*c));
			ImageFrame &frame = (*_font)[*c - 33];
			surface->transBlitFrom(frame, charPos, false, overrideColor);
			charPos.x += frame._frame.w + 1;
		}
	}
}

int Fonts::stringWidth(const Common::String &str) {
	int width = 0;

	for (const char *c = str.c_str(); *c; ++c)
		width += charWidth(*c);

	return width;
}

int Fonts::stringHeight(const Common::String &str) {
	int height = 0;

	for (const char *c = str.c_str(); *c; ++c)
		height = MAX(height, charHeight(*c));

	return height;
}

int Fonts::charWidth(unsigned char c) {
	if (c == ' ')
		return 5;
	else if (Common::isPrint(c))
		return (*_font)[c - 33]._frame.w + 1;
	else
		return 0;
}

int Fonts::charHeight(unsigned char c) {
	int idx = c - 33;
	if (c == ' ')
		idx = 0;
	else if (c == 225)
		idx = 136;

	const ImageFrame &img = (*_font)[idx];
	return img._height + img._offset.y + 1;
}

} // End of namespace Sherlock
