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

#include "action_text.h"
#include <pink/archive.h>
#include <common/debug.h>

namespace Pink {

void ActionText::deserialize(Archive &archive) {
    for (int i = 0; i < 4 ; ++i) {
        archive >> _bounds[i];
    }
    archive >> _centered >> _scrollBar
            >> _textColor >> _backgroundColor;
}

void ActionText::toConsole() {
    debug("\tActionText: _name = %s, _text = %s, "
                  "_bound0 = %u, _bound1 = %u, _bound2 = %u, _bound3 = %u _centered = %u, _scrollBar = %u, _textColor = %u _backgroundColor = %u",
          _name.c_str(), _text.c_str(), _bounds[0], _bounds[1], _bounds[2], _bounds[3], _centered, _scrollBar, _textColor, _backgroundColor);
}



}