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
 * $URL: https://www.switchlink.se/svn/teen/surface.h $
 * $Id: surface.h 154 2009-08-10 20:10:23Z megath $
 */


#ifndef TAGET_SURFACE_H__
#define TAGET_SURFACE_H__

#include "graphics/surface.h"
#include "common/stream.h"

namespace TeenAgent {

class Pack;
class Surface : public Graphics::Surface {
public:
	enum Type {TypeOn, TypeOns, TypeLan};

	uint16 flags[255];
	uint16 x, y;
	
	Surface();
	void load(Common::SeekableReadStream * stream, Type type);
	void render(Graphics::Surface * surface, int dx = 0, int dy = 0, bool mirror = false);

	bool empty() const { return pixels == NULL; }
};

}

#endif

