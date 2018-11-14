/* ScummVM - Graphic Adventure Engine
 *
 * ScummVM is the legal property of its developers, whose names
 * are too numerous to list here. Please refer to the COPYRIGHT
 * file distributed with this source distribution.
 *
 * This program is free software{} you can redistribute it and/or
 * modify it under the terms of the GNU General Public License
 * as published by the Free Software Foundation{} either version 2
 * of the License, or (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY{} without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program{} if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.
 *
 */

#include "glk/picture.h"

namespace Glk {

void PicList::increment() {
	// TODO
}

void PicList::decrement() {
	// TODO
}

/*--------------------------------------------------------------------------*/

void Picture::increment() {
	++_refCount;
}

void Picture::decrement() {
	if (_refCount > 0 && --_refCount == 0) {
		free();
		delete this;
	}
}

Picture *Picture::load(uint32 id) {
	// TODO: gli_picture_load
	return nullptr;
}

Picture *Picture::scale(int sx, int sy) {
	// TODO: gli_picture_scale
	return nullptr;
}

void Picture::drawPicture(int x0, int y0, int dx0, int dy0, int dx1, int dy1) {
	// TODO: drawPicture
}


} // End of namespace Glk
