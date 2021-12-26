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

#include "ultima/ultima8/gumps/cru_ammo_gump.h"

#include "ultima/ultima8/world/actors/main_actor.h"
#include "ultima/ultima8/gumps/widgets/text_widget.h"
#include "ultima/ultima8/world/get_object.h"
#include "ultima/ultima8/ultima8.h"

namespace Ultima {
namespace Ultima8 {

DEFINE_RUNTIME_CLASSTYPE_CODE(CruAmmoGump)

static const int REM_FONT_NUM = 15;
static const int REG_FONT_NUM = 8;

static const int REM_XOFF = 22;
static const int REG_XOFF = 38;
static const int REM_YOFF = 3;
static const int REG_YOFF = 6;

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
}

void CruAmmoGump::PaintThis(RenderSurface *surf, int32 lerp_factor, bool scaled) {
	MainActor *a = getMainActor();
	if (!a) {
		// avatar gone??
		return;
	}

	uint16 active = a->getActiveWeapon();
	uint16 ammoitem = 0;
	int bullets = -1;
	uint16 clips = 0;
	if (active) {
		Item *item = getItem(active);
		if (item) {
			const WeaponInfo *weaponinfo = item->getShapeInfo()->_weaponInfo;
			//uint16 frameno = 0;
			if (weaponinfo && weaponinfo->_ammoType) {
				//frameno = weaponinfo->_ammoType;
				ammoitem = weaponinfo->_ammoShape;
				bullets = item->getQuality();
			}
		}
	}

	// Only paint if this weapon has bullets that get used up.
	if (bullets >= 0 && a == getControlledActor()) {
		const int xoff = GAME_IS_REMORSE ? REM_XOFF : REG_XOFF;
		const int yoff = GAME_IS_REMORSE ? REM_YOFF : REG_YOFF;
		const int fontno = GAME_IS_REMORSE ? REM_FONT_NUM : REG_FONT_NUM;

		const Std::string bulletstr = Std::string::format("%d", bullets);
		if (!_bulletsText || !bulletstr.equals(_bulletsText->getText())) {
			if (_bulletsText) {
				RemoveChild(_bulletsText);
				_bulletsText->Close();
			}
			_bulletsText = new TextWidget(xoff, _dims.height() / 2 - yoff, bulletstr, true, fontno);
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
		if (!_clipsText || !clipstr.equals(_clipsText->getText())) {
			if (_clipsText) {
				RemoveChild(_clipsText);
				_clipsText->Close();
			}
			_clipsText = new TextWidget(_dims.width() / 2 + xoff, _dims.height() / 2 - yoff, clipstr, true, fontno);
			_clipsText->InitGump(this, false);
		}
		CruStatGump::PaintThis(surf, lerp_factor, scaled);
	} else {
		if (_bulletsText) {
			RemoveChild(_bulletsText);
			_bulletsText->Close();
			_bulletsText = nullptr;
		}
		if (_clipsText) {
			RemoveChild(_clipsText);
			_clipsText->Close();
			_clipsText = nullptr;
		}
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
