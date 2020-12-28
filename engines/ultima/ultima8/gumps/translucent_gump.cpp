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

#include "ultima/ultima8/misc/pent_include.h"
#include "ultima/ultima8/gumps/translucent_gump.h"
#include "ultima/ultima8/graphics/render_surface.h"

namespace Ultima {
namespace Ultima8 {

DEFINE_RUNTIME_CLASSTYPE_CODE(TranslucentGump)
TranslucentGump::TranslucentGump() : Gump() {
}

TranslucentGump::TranslucentGump(int x, int y, int width, int height,
								 uint16 owner, uint32 flags, int32 layer) :
	Gump(x, y, width, height, owner, flags, layer) {
}

TranslucentGump::~TranslucentGump() {
}

void TranslucentGump::PaintThis(RenderSurface *surf, int32 lerp_factor, bool scaled) {
	if (_shape) {
		surf->PaintTranslucent(_shape, _frameNum, 0, 0);
	}
}

void TranslucentGump::saveData(Common::WriteStream *ws) {
	Gump::saveData(ws);
}

bool TranslucentGump::loadData(Common::ReadStream *rs, uint32 version) {
	return Gump::loadData(rs, version);
}

} // End of namespace Ultima8
} // End of namespace Ultima
