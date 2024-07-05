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

#include "qdengine/system/graphics/gr_screen_region.h"

namespace QDEngine {

class XZipStream;

class grFont {
public:
	grFont();
	~grFont();

	bool load(const char *fname);

	bool load_index(Common::SeekableReadStream *fh);
	bool load_alpha(Common::SeekableReadStream *fh);

	int size_x() const {
		return size_x_;
	}
	int size_y() const {
		return size_y_;
	}

	int alpha_buffer_size_x() const {
		return alpha_buffer_sx_;
	}
	int alpha_buffer_size_y() const {
		return alpha_buffer_sy_;
	}

	const unsigned char *alpha_buffer() const {
		return alpha_buffer_;
	}

	const grScreenRegion &find_char(int code) const {
		grFontCharVector::const_iterator it = std::find(chars_.begin(), chars_.end(), code);
		if (it != chars_.end()) return it->region_;

		return grScreenRegion::EMPTY;
	}

	int char_width(int code) const {
		return code == ' ' ? size_x() / 2 : find_char(code).size_x();
	}

private:

	int size_x_;
	int size_y_;

	int alpha_buffer_sx_;
	int alpha_buffer_sy_;
	unsigned char *alpha_buffer_;

	struct grFontChar {
		grFontChar() : code_(-1) { }

		int code_;
		grScreenRegion region_;

		bool operator == (int code) const {
			return (code_ == code);
		}
	};

	typedef std::vector<grFontChar> grFontCharVector;
	grFontCharVector chars_;
};

} // namespace QDEngine

#endif // QDENGINE_SYSTEM_GRAPHICS_GR_FONT_H

