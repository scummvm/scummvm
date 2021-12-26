/* ScummVM - Graphic Adventure Engine
 *
 * ScummVM is the legal property of its developers, whose names
 * are too numerous to list here. Please refer to the COPYRIGHT
 * file distributed with this source distribution.
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 *
 */

#include "ultima/ultima8/gumps/cru_health_gump.h"

#include "ultima/ultima8/world/actors/main_actor.h"
#include "ultima/ultima8/graphics/palette_manager.h"
#include "ultima/ultima8/graphics/render_surface.h"
#include "ultima/ultima8/world/get_object.h"

namespace Ultima {
namespace Ultima8 {

static const uint32 HEALTH_BAR_R = 0;
static const uint32 HEALTH_BAR_G = 48;
static const uint32 HEALTH_BAR_B = 113;

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

	const Actor *a = getControlledActor();
	int current_hp = a ? a->getHP() : 0;
	int max_hp = a ? a->getMaxHP() : 1;
	// max width = 67
	int width = max_hp ? ((current_hp * 67) / max_hp) : 67;

	const Palette *gamepal = PaletteManager::get_instance()->getPalette(PaletteManager::Pal_Game);
	if (!gamepal)
		return;

	int r = HEALTH_BAR_R;
	int g = HEALTH_BAR_G;
	int b = HEALTH_BAR_B;
	gamepal->transformRGB(r, g, b);
	uint32 fillcolor = (r << 16) | (g << 8) | b;
	surf->Fill32(fillcolor, 34, 7, width, 14);
}

void CruHealthGump::saveData(Common::WriteStream *ws) {
	CruStatGump::saveData(ws);
}

bool CruHealthGump::loadData(Common::ReadStream *rs, uint32 version) {
	return CruStatGump::loadData(rs, version);
}

} // End of namespace Ultima8
} // End of namespace Ultima
