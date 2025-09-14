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
#include "ultima/ultima8/gfx/palette.h"
#include "ultima/ultima8/gfx/palette_manager.h"
#include "ultima/ultima8/gfx/render_surface.h"
#include "ultima/ultima8/gfx/texture.h"
#include "ultima/ultima8/world/get_object.h"

namespace Ultima {
namespace Ultima8 {

static const uint HEALTH_BAR_COLOR = 65;

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

	Common::Rect32 rect(34, 7, 34 + width, 21);
	surf->fillRect(rect, gamepal->_native[HEALTH_BAR_COLOR]);
}

void CruHealthGump::saveData(Common::WriteStream *ws) {
	CruStatGump::saveData(ws);
}

bool CruHealthGump::loadData(Common::ReadStream *rs, uint32 version) {
	return CruStatGump::loadData(rs, version);
}

} // End of namespace Ultima8
} // End of namespace Ultima
