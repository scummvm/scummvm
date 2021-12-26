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

#include "ultima/ultima8/gumps/cru_energy_gump.h"

#include "ultima/ultima8/world/actors/main_actor.h"
#include "ultima/ultima8/graphics/palette_manager.h"
#include "ultima/ultima8/graphics/render_surface.h"
#include "ultima/ultima8/world/get_object.h"

namespace Ultima {
namespace Ultima8 {

static const uint32 ENERGY_BAR_R = 154;
static const uint32 ENERGY_BAR_G = 4;
static const uint32 ENERGY_BAR_B = 4;

DEFINE_RUNTIME_CLASSTYPE_CODE(CruEnergyGump)

CruEnergyGump::CruEnergyGump() : CruStatGump() {

}

CruEnergyGump::CruEnergyGump(Shape *shape, int x)
	: CruStatGump(shape, x) {
	_frameNum = 3;
}

CruEnergyGump::~CruEnergyGump() {
}

void CruEnergyGump::InitGump(Gump *newparent, bool take_focus) {
	CruStatGump::InitGump(newparent, take_focus);
}

void CruEnergyGump::PaintThis(RenderSurface *surf, int32 lerp_factor, bool scaled) {
	const Actor *a = getControlledActor();
	if (!a) {
		// avatar gone??
		return;
	}

	int16 energy = a->getMana();
	int16 max_energy = a->getMaxMana();

	// Don't display for NPCs without energy
	if (!max_energy)
		return;

	CruStatGump::PaintThis(surf, lerp_factor, scaled);

	int width = (energy * 67) / max_energy;
	const Palette *gamepal = PaletteManager::get_instance()->getPalette(PaletteManager::Pal_Game);
	if (!gamepal)
		return;

	int r = ENERGY_BAR_R;
	int g = ENERGY_BAR_G;
	int b = ENERGY_BAR_B;
	gamepal->transformRGB(r, g, b);
	uint32 fillcolor = (r << 16) | (g << 8) | b;
	surf->Fill32(fillcolor, 34, 7, width, 14);
}

void CruEnergyGump::saveData(Common::WriteStream *ws) {
	CruStatGump::saveData(ws);
}

bool CruEnergyGump::loadData(Common::ReadStream *rs, uint32 version) {
	return CruStatGump::loadData(rs, version);
}

} // End of namespace Ultima8
} // End of namespace Ultima
