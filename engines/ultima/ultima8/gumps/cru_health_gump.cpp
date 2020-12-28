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
#include "ultima/ultima8/gumps/cru_health_gump.h"

#include "ultima/ultima8/games/game_data.h"
#include "ultima/ultima8/graphics/gump_shape_archive.h"
#include "ultima/ultima8/graphics/shape.h"
#include "ultima/ultima8/graphics/shape_frame.h"
#include "ultima/ultima8/world/actors/main_actor.h"
#include "ultima/ultima8/graphics/render_surface.h"
#include "ultima/ultima8/kernel/mouse.h"
#include "ultima/ultima8/world/get_object.h"

namespace Ultima {
namespace Ultima8 {

static const uint32 HEALTH_BAR_COLOR = 0xFF003071; // RGB = (154, 4, 4)

DEFINE_RUNTIME_CLASSTYPE_CODE(CruHealthGump)

CruHealthGump::CruHealthGump() : CruStatGump() {

}

CruHealthGump::CruHealthGump(Shape *shape, int x)
	: CruStatGump(shape, x) {
	_frameNum = 2;
}

CruHealthGump::~CruHealthGump() {
}

void CruHealthGump::InitGump(Gump *newparent, bool take_focus) {
	CruStatGump::InitGump(newparent, take_focus);
}

void CruHealthGump::PaintThis(RenderSurface *surf, int32 lerp_factor, bool scaled) {
	CruStatGump::PaintThis(surf, lerp_factor, scaled);

	const MainActor *a = getMainActor();
	if (!a) {
		// avatar gone??
		return;
	}

	int current_hp = a->getHP();
	int max_hp = a->getMaxHP();
	// max width = 67
	int width = max_hp ? ((current_hp * 67) / max_hp) : 67;
	surf->Fill32(HEALTH_BAR_COLOR, 34, 7, width, 14);
}

void CruHealthGump::saveData(Common::WriteStream *ws) {
	CruStatGump::saveData(ws);
}

bool CruHealthGump::loadData(Common::ReadStream *rs, uint32 version) {
	return CruStatGump::loadData(rs, version);
}

} // End of namespace Ultima8
} // End of namespace Ultima
