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

#include "cryomni3d/mouse_boxes.h"

#include "common/rect.h"

#include "cryomni3d/font_manager.h"

namespace CryOmni3D {

MouseBoxes::MouseBoxes(uint size) {
	_boxes.resize(size);
}

MouseBoxes::~MouseBoxes() {
}

void MouseBoxes::reset() {
	uint sz = _boxes.size();
	_boxes.clear();
	_boxes.resize(sz);
}

void MouseBoxes::setupBox(int box_id, int left, int top, int right, int bottom,
                          const Common::String *text) {
	MouseBox &box = _boxes[box_id];
	box.left = left;
	box.top = top;
	box.right = right;
	box.bottom = bottom;
	box.isChar = false;
	box.string = text;
}

void MouseBoxes::setupBox(int box_id, int left, int top, int right, int bottom, const char *text) {
	MouseBox &box = _boxes[box_id];
	box.left = left;
	box.top = top;
	box.right = right;
	box.bottom = bottom;
	box.isChar = true;
	box.charp = text;
}

Common::Rect MouseBoxes::getBoxRect(int box_id) const {
	const MouseBox &box = _boxes[box_id];
	return Common::Rect(box.left, box.top, box.right, box.bottom);
}

Common::Point MouseBoxes::getBoxOrigin(int box_id) const {
	const MouseBox &box = _boxes[box_id];
	return Common::Point(box.left, box.top);
}

bool MouseBoxes::hitTest(int box_id, const Common::Point &pt) {
	const MouseBox &box = _boxes[box_id];

	return (box.left != -1) &&
	       (pt.x > box.left && pt.x < box.right &&
	        pt.y > box.top && pt.y < box.bottom);
}

void MouseBoxes::display(int box_id, const FontManager &font_manager) {
	const MouseBox &box = _boxes[box_id];

	if (box.string) {
		if (box.isChar) {
			font_manager.displayStr(box.left, box.top, box.charp);
		} else {
			font_manager.displayStr(box.left, box.top, *box.string);
		}
	}
}

}
