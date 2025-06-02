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

#include "ultima/ultima8/gumps/cru_inventory_gump.h"

#include "ultima/ultima8/games/game_data.h"
#include "ultima/ultima8/gfx/gump_shape_archive.h"
#include "ultima/ultima8/gfx/shape.h"
#include "ultima/ultima8/world/actors/main_actor.h"
#include "ultima/ultima8/world/get_object.h"
#include "ultima/ultima8/gumps/widgets/text_widget.h"

namespace Ultima {
namespace Ultima8 {

static const int INVENTORY_TEXT_FONT = 12;

DEFINE_RUNTIME_CLASSTYPE_CODE(CruInventoryGump)
CruInventoryGump::CruInventoryGump() : CruStatGump(), _inventoryItemGump(nullptr), _inventoryText(nullptr) {
}

CruInventoryGump::CruInventoryGump(Shape *shape, int x)
	: CruStatGump(shape, x), _inventoryItemGump(nullptr), _inventoryText(nullptr) {
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

	_inventoryItemGump = new Gump();
	_inventoryItemGump->InitGump(this, false);
	// we'll set the shape for this gump later.

	resetText();
}

void CruInventoryGump::resetText() {
	if (_inventoryText) {
		RemoveChild(_inventoryText);
		_inventoryText->Close();
	}
	_inventoryText = new TextWidget();
	_inventoryText->InitGump(this, false);
}


void CruInventoryGump::PaintThis(RenderSurface *surf, int32 lerp_factor, bool scaled) {
	const MainActor *a = getMainActor();
	if (!a) {
		// avatar gone??
		return;
	}

	uint16 activeitem = a->getActiveInvItem();
	if (!activeitem || a != getControlledActor()) {
		resetText();
		_inventoryItemGump->SetShape(0, 0);
	} else {
		Item *item = getItem(activeitem);
		if (!item) {
			resetText();
			_inventoryItemGump->SetShape(0, 0);
		} else {
			GumpShapeArchive *gumpshapes = GameData::get_instance()->getGumps();
			if (!gumpshapes) {
				warning("failed to paint stat gump: no gump shape archive");
				return;
			}

			const ShapeInfo *shapeinfo = item->getShapeInfo();
			if (!shapeinfo->_weaponInfo) {
				warning("no weapon info for active inventory item %d", item->getShape());
				return;
			}
			Shape *invshape = gumpshapes->getShape(shapeinfo->_weaponInfo->_displayGumpShape);
			_inventoryItemGump->SetShape(invshape, shapeinfo->_weaponInfo->_displayGumpFrame);
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
					_inventoryText->Close();
					_inventoryText = new TextWidget(_dims.width() / 2 + 22, _dims.height() / 2 + 3, qtext, true, INVENTORY_TEXT_FONT);
					_inventoryText->InitGump(this, false);
				}
			} else {
				if (!_inventoryText->getText().empty()) {
					resetText();
				}
			}
		}
		// Now that the shape is configured, we can paint.
		CruStatGump::PaintThis(surf, lerp_factor, scaled);
	}
}

void CruInventoryGump::saveData(Common::WriteStream *ws) {
	CruStatGump::saveData(ws);
}

bool CruInventoryGump::loadData(Common::ReadStream *rs, uint32 version) {
	return CruStatGump::loadData(rs, version);
}

} // End of namespace Ultima8
} // End of namespace Ultima
