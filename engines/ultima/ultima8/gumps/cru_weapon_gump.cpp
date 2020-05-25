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
#include "ultima/ultima8/gumps/cru_weapon_gump.h"

#include "ultima/ultima8/games/game_data.h"
#include "ultima/ultima8/graphics/gump_shape_archive.h"
#include "ultima/ultima8/graphics/shape.h"
#include "ultima/ultima8/graphics/shape_frame.h"
#include "ultima/ultima8/world/actors/main_actor.h"
#include "ultima/ultima8/graphics/render_surface.h"
#include "ultima/ultima8/kernel/mouse.h"
#include "ultima/ultima8/gumps/paperdoll_gump.h"
#include "ultima/ultima8/world/get_object.h"

namespace Ultima {
namespace Ultima8 {

static const int WEAPON_GUMP_SHAPE = 3;

DEFINE_RUNTIME_CLASSTYPE_CODE(CruWeaponGump)

CruWeaponGump::CruWeaponGump() : CruStatGump(), _weaponShape(nullptr) {

}

CruWeaponGump::CruWeaponGump(Shape *shape, int x)
	: CruStatGump(shape, x), _weaponShape(nullptr) {
	_frameNum = 0;
}

CruWeaponGump::~CruWeaponGump() {
}

void CruWeaponGump::InitGump(Gump *newparent, bool take_focus) {
	CruStatGump::InitGump(newparent, take_focus);

	GumpShapeArchive *gumpshapes = GameData::get_instance()->getGumps();
	if (!gumpshapes) {
		warning("failed to init stat gump: no gump shape archive");
		return;
	}

	_weaponShape = gumpshapes->getShape(WEAPON_GUMP_SHAPE);
	if (!_weaponShape || !_weaponShape->getFrame(0)) {
		warning("failed to init stat gump: no weapon shape");
		return;
	}
}

void CruWeaponGump::PaintThis(RenderSurface *surf, int32 lerp_factor, bool scaled) {
	CruStatGump::PaintThis(surf, lerp_factor, scaled);

	const MainActor *a = getMainActor();
	if (!a) {
		// avatar gone??
		return;
	}

	uint16 weapon = a->getDamageType(); // ?? TODO: Where do we store current weapon?
	/*const ShapeFrame *frame = */_weaponShape->getFrame(weapon);
	// TODO: Paint the current weapon
}

void CruWeaponGump::saveData(Common::WriteStream *ws) {
	Gump::saveData(ws);
}

bool CruWeaponGump::loadData(Common::ReadStream *rs, uint32 version) {
	return Gump::loadData(rs, version);
}

} // End of namespace Ultima8
} // End of namespace Ultima
