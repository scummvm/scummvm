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

#ifndef ULTIMA8_GUMPS_FASTAREAVISGUMP_H
#define ULTIMA8_GUMPS_FASTAREAVISGUMP_H

#include "ultima/ultima8/gumps/gump.h"
#include "ultima/ultima8/misc/p_dynamic_cast.h"

namespace Ultima {
namespace Ultima8 {

/**
 * A gump for debugging which shows all the things in the "fast" area (the area around the avatar
 * which is being calculated for pathfinding etc)
 */
class FastAreaVisGump : public Gump {
public:
	ENABLE_RUNTIME_CLASSTYPE()

	FastAreaVisGump();
	~FastAreaVisGump() override;

	void        PaintThis(RenderSurface *surf, int32 lerp_factor, bool scaled) override;
	uint16      TraceObjId(int32 mx, int32 my) override;
};

} // End of namespace Ultima8
} // End of namespace Ultima

#endif
