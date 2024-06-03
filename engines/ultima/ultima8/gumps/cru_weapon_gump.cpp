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

#include "ultima/ultima8/gumps/cru_weapon_gump.h"

#include "ultima/ultima8/games/game_data.h"
#include "ultima/ultima8/gfx/gump_shape_archive.h"
#include "ultima/ultima8/gfx/shape.h"
#include "ultima/ultima8/world/actors/main_actor.h"
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

CruWeaponGump::~CruWeaponGump() {
}

void CruWeaponGump::InitGump(Gump *newparent, bool take_focus) {
	CruStatGump::InitGump(newparent, take_focus);

	// We will fill out the shape to paint for this later.
	Gump *weaponGump = new Gump();
	weaponGump->InitGump(this, false);
	weaponGump->SetIndex(1);
}

void CruWeaponGump::PaintThis(RenderSurface *surf, int32 lerp_factor, bool scaled) {
	const MainActor *a = getMainActor();
	if (!a) {
		// avatar gone??
		return;
	}

	Gump *weaponGump = _children.front();

	if (a != getControlledActor()) {
		// Only paint when controlling avatar
		weaponGump->HideGump();
		return;
	} else {
		weaponGump->UnhideGump();
	}

	assert(weaponGump);
	uint16 active = a->getActiveWeapon();
	if (!active) {
		weaponGump->SetShape(0, 0);
	} else {
		Item *item = getItem(active);
		if (!item) {
			weaponGump->SetShape(0, 0);
		} else {
			const WeaponInfo *weaponinfo = item->getShapeInfo()->_weaponInfo;
			uint16 frameno = 0;
			if (weaponinfo) {
				// this should be a weapon, otherwise why are we here?
				assert(WEAPON_GUMP_SHAPE == weaponinfo->_displayGumpShape);
				frameno = weaponinfo->_displayGumpFrame;
			}
			weaponGump->SetShape(_weaponShape, frameno);
			weaponGump->UpdateDimsFromShape();
			weaponGump->setRelativePosition(CENTER);
		}
	}

	CruStatGump::PaintThis(surf, lerp_factor, scaled);
}

void CruWeaponGump::saveData(Common::WriteStream *ws) {
	CruStatGump::saveData(ws);
}

bool CruWeaponGump::loadData(Common::ReadStream *rs, uint32 version) {
	return CruStatGump::loadData(rs, version);
}

} // End of namespace Ultima8
} // End of namespace Ultima
