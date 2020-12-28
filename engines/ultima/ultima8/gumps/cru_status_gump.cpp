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
#include "ultima/ultima8/gumps/cru_status_gump.h"
#include "ultima/ultima8/gumps/cru_weapon_gump.h"
#include "ultima/ultima8/gumps/cru_ammo_gump.h"
#include "ultima/ultima8/gumps/cru_inventory_gump.h"
#include "ultima/ultima8/gumps/cru_health_gump.h"
#include "ultima/ultima8/gumps/cru_energy_gump.h"

#include "ultima/ultima8/games/game_data.h"
#include "ultima/ultima8/graphics/gump_shape_archive.h"
#include "ultima/ultima8/graphics/shape.h"
#include "ultima/ultima8/graphics/shape_frame.h"
#include "ultima/ultima8/graphics/render_surface.h"
#include "ultima/ultima8/kernel/mouse.h"
#include "ultima/ultima8/world/get_object.h"

namespace Ultima {
namespace Ultima8 {

DEFINE_RUNTIME_CLASSTYPE_CODE(CruStatusGump)

static const int PX_FROM_BOTTOM = 2;	//! gap (y) between bottom of screen and bottom of a single item
static const int PX_FROM_LEFT = 15;  	//! gap (x) from left of screen to weapon box
static const int PX_GAP = 17;			//! gap (x) between boxes in status bar

static const int FRAME_GUMP_SHAPE = 1;

CruStatusGump *CruStatusGump::_instance = nullptr;

CruStatusGump::CruStatusGump() : Gump() { }

// Start with an approximate width/height which we will adjust later..
CruStatusGump::CruStatusGump(bool unused) : Gump(PX_FROM_LEFT, PX_FROM_BOTTOM, 500, 100, 0, 0, LAYER_ABOVE_NORMAL) {
	assert(!_instance);
	_instance = this;
}

CruStatusGump::~CruStatusGump() {
	assert(_instance == this);
	_instance = nullptr;
}

void CruStatusGump::InitGump(Gump *newparent, bool take_focus) {
	Gump::InitGump(newparent, take_focus);

	createStatusItems();
}

void CruStatusGump::createStatusItems() {
	assert(_children.size() == 0);
	GumpShapeArchive *gumpshapes = GameData::get_instance()->getGumps();
	if (!gumpshapes) {
		warning("failed to init stats gump: no gump shape archive");
		return;
	}

	Shape *frameShape = gumpshapes->getShape(FRAME_GUMP_SHAPE);
	if (!frameShape || !frameShape->getFrame(0)) {
		warning("failed to init stats gump: no gump frame");
		return;
	}

	int w = frameShape->getFrame(0)->_width;
	int h = frameShape->getFrame(0)->_height;

	int xoff = 0;
	Gump *weaponGump = new CruWeaponGump(frameShape, xoff);
	weaponGump->InitGump(this);
	Gump *ammoGump = new CruAmmoGump(frameShape, xoff + w + PX_GAP);
	ammoGump->InitGump(this);
	Gump *inventoryGump = new CruInventoryGump(frameShape, xoff + (w + PX_GAP) * 2);
	inventoryGump->InitGump(this);
	Gump *health = new CruHealthGump(frameShape, xoff + (w + PX_GAP) * 3);
	health->InitGump(this);
	Gump *energyGump = new CruEnergyGump(frameShape, xoff + (w + PX_GAP) * 4);
	energyGump->InitGump(this);

	_dims.setWidth(w * 5 + PX_GAP * 4);
	_dims.setHeight(h);
	setRelativePosition(BOTTOM_LEFT, PX_FROM_LEFT, -PX_FROM_BOTTOM);
}

void CruStatusGump::PaintThis(RenderSurface *surf, int32 lerp_factor, bool scaled) {
	Gump::PaintThis(surf, lerp_factor, scaled);

	// All the painting logic is in the children.
}

void CruStatusGump::saveData(Common::WriteStream *ws) {
	Gump::saveData(ws);
}

bool CruStatusGump::loadData(Common::ReadStream *rs, uint32 version) {
	if (Gump::loadData(rs, version)) {
		createStatusItems();
		return true;
	} else {
		return false;
	}
}

uint32 CruStatusGump::I_hideStatusGump(const uint8 * /*args*/,
unsigned int /*argsize*/) {
	CruStatusGump *instance = get_instance();
	if (instance) {
		instance->Close();
		_instance = nullptr;
	}
	return 0;
}

uint32 CruStatusGump::I_showStatusGump(const uint8 * /*args*/,
	unsigned int /*argsize*/) {
	CruStatusGump *instance = get_instance();
	if (!instance) {
		instance = new CruStatusGump();
		instance->InitGump(nullptr, false);
	}
	return 0;
}

} // End of namespace Ultima8
} // End of namespace Ultima
