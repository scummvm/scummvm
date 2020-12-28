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
#include "ultima/ultima8/gumps/cru_ammo_gump.h"

#include "ultima/ultima8/games/game_data.h"
#include "ultima/ultima8/graphics/gump_shape_archive.h"
#include "ultima/ultima8/graphics/shape.h"
#include "ultima/ultima8/graphics/shape_frame.h"
#include "ultima/ultima8/world/actors/main_actor.h"
#include "ultima/ultima8/graphics/render_surface.h"
#include "ultima/ultima8/gumps/widgets/text_widget.h"
#include "ultima/ultima8/world/get_object.h"

namespace Ultima {
namespace Ultima8 {

DEFINE_RUNTIME_CLASSTYPE_CODE(CruAmmoGump)

CruAmmoGump::CruAmmoGump() : CruStatGump(), _clipsText(nullptr), _bulletsText(nullptr) {

}

CruAmmoGump::CruAmmoGump(Shape *shape, int x) : CruStatGump(shape, x),
		_clipsText(nullptr), _bulletsText(nullptr) {
	_frameNum = 1;
}

CruAmmoGump::~CruAmmoGump() {
}

void CruAmmoGump::InitGump(Gump *newparent, bool take_focus) {
	CruStatGump::InitGump(newparent, take_focus);

	_bulletsText = new TextWidget();
	_clipsText = new TextWidget();
}

void CruAmmoGump::PaintThis(RenderSurface *surf, int32 lerp_factor, bool scaled) {
	CruStatGump::PaintThis(surf, lerp_factor, scaled);

	MainActor *a = getMainActor();
	if (!a) {
		// avatar gone??
		return;
	}

	uint16 active = a->getActiveWeapon();
	uint16 ammoitem = 0;
	uint16 bullets = 0;
	uint16 clips = 0;
	if (!active) {
		bullets = 0;
	} else {
		Item *item = getItem(active);
		if (!item) {
			bullets = 0;
		} else {
			WeaponInfo *weaponinfo = item->getShapeInfo()->_weaponInfo;
			//uint16 frameno = 0;
			if (weaponinfo) {
				//frameno = weaponinfo->_ammoType;
				ammoitem = weaponinfo->_ammoShape;
			}

			bullets = item->getQuality();
		}
	}

	const Std::string bulletstr = Std::string::format("%d", bullets);
	if (!bulletstr.equals(_bulletsText->getText())) {
		RemoveChild(_bulletsText);
		_bulletsText = new TextWidget(22, _dims.height() / 2 - 3, bulletstr, true, 15);
		_bulletsText->InitGump(this, false);
	}

	if (ammoitem) {
		Item *item = a->getFirstItemWithShape(ammoitem, true);
		if (item) {
			clips = item->getQuality();
		} else {
			clips = 0;
		}
	}

	const Std::string clipstr = Std::string::format("%d", clips);
	if (!clipstr.equals(_clipsText->getText())) {
		RemoveChild(_clipsText);
		_clipsText = new TextWidget(_dims.width() / 2 + 22, _dims.height() / 2 - 3, clipstr, true, 15);
		_clipsText->InitGump(this, false);
	}

}

void CruAmmoGump::saveData(Common::WriteStream *ws) {
	CruStatGump::saveData(ws);
}

bool CruAmmoGump::loadData(Common::ReadStream *rs, uint32 version) {
	return CruStatGump::loadData(rs, version);
}

} // End of namespace Ultima8
} // End of namespace Ultima
