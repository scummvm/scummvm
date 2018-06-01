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

#include "common/debug.h"

#include "pink/archive.h"
#include "pink/objects/actions/action_text.h"

namespace Pink {

void ActionText::deserialize(Archive &archive) {
	Action::deserialize(archive);
	_fileName = archive.readString();

	_xLeft = archive.readDWORD();
	_yTop = archive.readDWORD();
	_xRight = archive.readDWORD();
	_yBottom = archive.readDWORD();

	_centered = archive.readDWORD();
	_scrollBar = archive.readDWORD();
	_textColor = archive.readDWORD();
	_backgroundColor = archive.readDWORD();
}

void ActionText::toConsole() {
	debug("\tActionText: _name = %s, _fileName = %s, "
				  "_xLeft = %u, _yTop = %u, _xRight = %u, _yBottom = %u _centered = %u, _scrollBar = %u, _textColor = %u _backgroundColor = %u",
		  _name.c_str(), _fileName.c_str(), _xLeft, _yTop, _xRight, _yBottom, _centered, _scrollBar, _textColor, _backgroundColor);
}

} // End of namespace Pink
