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

#ifndef QDENGINE_SYSTEM_GRAPHICS_GR_FONT_H
#define QDENGINE_SYSTEM_GRAPHICS_GR_FONT_H

#include "common/path.h"
#include "qdengine/system/graphics/gr_screen_region.h"

namespace Common {
class SeekableReadStream;
}

namespace QDEngine {

class grFont {
public:
	grFont();
	~grFont();

	bool load(const Common::Path fname);

	bool load_index(Common::SeekableReadStream *fh);
	bool load_alpha(Common::SeekableReadStream *fh);

	int size_x() const {
		return _size_x;
	}
	int size_y() const {
		return _size_y;
	}

	int alpha_buffer_size_x() const {
		return _alpha_buffer_sx;
	}
	int alpha_buffer_size_y() const {
		return _alpha_buffer_sy;
	}

	const byte *alpha_buffer() const {
		return _alpha_buffer;
	}

	const grScreenRegion find_char(int code) const {
		grFontCharVector::const_iterator it = Common::find(_chars.begin(), _chars.end(), code);
		if (it != _chars.end()) return it->_region;

		return grScreenRegion_EMPTY;
	}

	int char_width(int code) const {
		return code == ' ' ? size_x() / 2 : find_char(code).size_x();
	}

private:

	int _size_x;
	int _size_y;

	int _alpha_buffer_sx;
	int _alpha_buffer_sy;
	byte *_alpha_buffer;

	struct grFontChar {
		grFontChar() : _code(-1) { }

		int _code;
		grScreenRegion _region;

		bool operator == (int code) const {
			return (_code == code);
		}
	};

	typedef Std::vector<grFontChar> grFontCharVector;
	grFontCharVector _chars;
};

} // namespace QDEngine

#endif // QDENGINE_SYSTEM_GRAPHICS_GR_FONT_H
