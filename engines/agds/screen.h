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

#ifndef AGDS_SCREEN_H
#define AGDS_SCREEN_H

#include "common/scummsys.h"
#include "common/list.h"
#include "common/str.h"

namespace Graphics {
	class Surface;
}

namespace AGDS {

class Object;

class Screen {
	typedef Common::List<Object *> ChildrenType;
	ChildrenType _children;

public:
	Screen(Object *object);
	void add(Object *object);
	void remove(const Common::String &name);
	void paint(Graphics::Surface &backbuffer);
};


} // End of namespace AGDS

#endif /* AGDS_SCREEN_H */
