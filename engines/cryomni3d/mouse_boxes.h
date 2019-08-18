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

#ifndef CRYOMNI3D_MOUSE_BOXES_H
#define CRYOMNI3D_MOUSE_BOXES_H

#include "common/array.h"
#include "common/str.h"

namespace Common {
struct Point;
struct Rect;
}

namespace CryOmni3D {

class FontManager;

class MouseBoxes {
public:
	MouseBoxes(uint size);
	virtual ~MouseBoxes();

	void reset();
	void setupBox(int box_id, int left, int top, int right, int bottom,
	              const Common::String *text = nullptr);
	void setupBox(int box_id, int left, int top, int right, int bottom, const char *text);
	Common::Rect getBoxRect(int box_id) const;
	Common::Point getBoxOrigin(int box_id) const;
	bool hitTest(int box_id, const Common::Point &pt);
	void display(int box_id, const FontManager &font_manager);

private:
	struct MouseBox {
		MouseBox() : left(-1), top(-1), right(-1), bottom(-1), string(nullptr), isChar(false) {}

		int left;
		int top;
		int right;
		int bottom;
		// Can be nullptr
		bool isChar;
		union {
			const Common::String *string;
			const char *charp;
		};
	};

	Common::Array<MouseBox> _boxes;
};

} // End of namespace CryOmni3D

#endif
