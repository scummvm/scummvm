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
#include "ultima/ultima8/gumps/cru_inventory_gump.h"

#include "ultima/ultima8/games/game_data.h"
#include "ultima/ultima8/graphics/gump_shape_archive.h"
#include "ultima/ultima8/graphics/shape.h"
#include "ultima/ultima8/graphics/shape_frame.h"
#include "ultima/ultima8/world/actors/main_actor.h"
#include "ultima/ultima8/graphics/render_surface.h"
#include "ultima/ultima8/kernel/mouse.h"
#include "ultima/ultima8/world/get_object.h"
#include "ultima/ultima8/gumps/widgets/text_widget.h"

namespace Ultima {
namespace Ultima8 {

static const int INVENTORY_GUMP_SHAPE = 5;
static const int INVENTORY_TEXT_FONT = 12;

DEFINE_RUNTIME_CLASSTYPE_CODE(CruInventoryGump)
CruInventoryGump::CruInventoryGump() : CruStatGump(), _inventoryShape(nullptr),
	_inventoryItemGump(nullptr), _inventoryText(nullptr) {

}

CruInventoryGump::CruInventoryGump(Shape *shape, int x)
	: CruStatGump(shape, x), _inventoryShape(nullptr), _inventoryItemGump(nullptr),
		_inventoryText(nullptr) {
	_frameNum = 0;
}

CruInventoryGump::~CruInventoryGump() {
}

void CruInventoryGump::InitGump(Gump *newparent, bool take_focus) {
	CruStatGump::InitGump(newparent, take_focus);

	GumpShapeArchive *gumpshapes = GameData::get_instance()->getGumps();
	if (!gumpshapes) {
		warning("failed to init stat gump: no gump shape archive");
		return;
	}

	_inventoryShape = gumpshapes->getShape(INVENTORY_GUMP_SHAPE);
	if (!_inventoryShape || !_inventoryShape->getFrame(0)) {
		warning("failed to init stat gump: no inventory shape");
		return;
	}
	_inventoryItemGump = new Gump();
	_inventoryItemGump->InitGump(this, false);
	// we'll set the shape for this gump later.

	_inventoryText = new TextWidget();
	_inventoryText->InitGump(this, false);
}

// TODO: This is a bit of a hack.. should be configured
// in the weapon ini file.
static uint16 getDisplayFrameForShape(uint16 shapeno) {
	switch (shapeno) {
	case 0x351:
		return 0x0;
	case 0x4D4:
		return 0x1;
	case 0x52D:
		return 0x2;
	case 0x52E:
		return 0x3;
	case 0x582:
		return 0x19;
	case 0x52F:
		return 0x5;
	case 0x55F:
		return 0x18;
	case 0x530:
		return 0x7;
	case 0x3A2:
		return 0x16;
	case 0x3A3:
		return 0x15;
	case 0x3A4:
		return 0x17;
	default:
		warning("No inventory gump frame for shape %d", shapeno);
		return 0;
	}
}

void CruInventoryGump::PaintThis(RenderSurface *surf, int32 lerp_factor, bool scaled) {
	const MainActor *a = getMainActor();
	if (!a) {
		// avatar gone??
		return;
	}

	uint16 activeitem = a->getActiveInvItem();
	if (!activeitem) {
		_inventoryItemGump->SetShape(0, 0);
	} else {
		Item *item = getItem(activeitem);
		if (!item) {
			_inventoryItemGump->SetShape(0, 0);
		} else {
			uint16 frame = getDisplayFrameForShape(item->getShape());
			_inventoryItemGump->SetShape(_inventoryShape, frame);
			_inventoryItemGump->UpdateDimsFromShape();
			_inventoryItemGump->setRelativePosition(CENTER);

			uint16 q = item->getQuality();
			if (q > 1) {
				// This isn't the most efficient way to work out if we need to make new
				// text, but it works..
				const Std::string qtext = Std::string::format("%d", q);
				const Std::string &currenttext = _inventoryText->getText();
				if (!qtext.equals(currenttext)) {
					RemoveChild(_inventoryText);
					_inventoryText = new TextWidget(_dims.width() / 2 + 22, _dims.height() / 2 + 3, qtext, true, INVENTORY_TEXT_FONT);
					_inventoryText->InitGump(this, false);
				}
			} else {
				if (_inventoryText->getText().length() > 0) {
					RemoveChild(_inventoryText);
					_inventoryText = new TextWidget();
					_inventoryText->InitGump(this, false);
				}
			}
		}
	}

	// Now that the shape is configured, we can paint.
	CruStatGump::PaintThis(surf, lerp_factor, scaled);
}

void CruInventoryGump::saveData(Common::WriteStream *ws) {
	CruStatGump::saveData(ws);
}

bool CruInventoryGump::loadData(Common::ReadStream *rs, uint32 version) {
	return CruStatGump::loadData(rs, version);
}

} // End of namespace Ultima8
} // End of namespace Ultima
