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

#include "ultima/ultima8/gumps/cru_pickup_gump.h"

#include "ultima/ultima8/gumps/translucent_gump.h"
#include "ultima/ultima8/kernel/kernel.h"
#include "ultima/ultima8/games/game_data.h"
#include "ultima/ultima8/gfx/gump_shape_archive.h"
#include "ultima/ultima8/gfx/shape.h"
#include "ultima/ultima8/gfx/shape_frame.h"
#include "ultima/ultima8/world/actors/main_actor.h"
#include "ultima/ultima8/gumps/widgets/text_widget.h"

namespace Ultima {
namespace Ultima8 {

static const int PICKUP_GUMP_SHAPE = 2;
static const int COUNT_TEXT_FONT = 12;
static const int ITEM_TEXT_FONT = 13;
static const int ITEM_AREA_WIDTH = 60;

// A high "random" index so we can always find this..
static const int COUNT_TEXT_INDEX = 0x100;

DEFINE_RUNTIME_CLASSTYPE_CODE(CruPickupGump)
CruPickupGump::CruPickupGump() : Gump(), _startFrame(0), _itemShapeNo(0), _q(0),
	_gumpShapeNo(0), _gumpFrameNo(0), _showCount(false) {
}

CruPickupGump::CruPickupGump(const Item *item, int y, bool showCount) : Gump(0, y, 5, 5, 0),
	_startFrame(0), _showCount(showCount) {
	const WeaponInfo *weaponInfo = item->getShapeInfo()->_weaponInfo;
	if (weaponInfo) {
		_itemShapeNo = item->getShape();
		if (item->getShapeInfo()->_family == ShapeInfo::SF_CRUAMMO)
			_q = 1;
		else
			_q = item->getQuality();
		_itemName = weaponInfo->_name;
		_gumpShapeNo = weaponInfo->_displayGumpShape;
		_gumpFrameNo = weaponInfo->_displayGumpFrame;
		// Special case for keycard - display depends on the card type
		if (_itemShapeNo == 0x111)
			_gumpFrameNo += item->getFrame();
	} else {
		_itemShapeNo = 0;
		_q = 0;
		_gumpShapeNo = 0;
		_gumpFrameNo = 0;
	}
}

void CruPickupGump::InitGump(Gump *newparent, bool take_focus) {
	Gump::InitGump(newparent, take_focus);

	if (!_itemShapeNo)
		return;

	_startFrame = Kernel::get_instance()->getFrameNum();

	// Get the shapes we will need..
	GumpShapeArchive *gumpshapes = GameData::get_instance()->getGumps();
	if (!gumpshapes) {
		warning("failed to init stat gump: no gump shape archive");
		return;
	}

	const Shape *background = gumpshapes->getShape(PICKUP_GUMP_SHAPE);
	if (!background || !background->getFrame(0)) {
		warning("failed to init stat gump: no pickup background shape");
		return;
	}
	const ShapeFrame *bgframe = background->getFrame(0);

	Shape* itemshape = gumpshapes->getShape(_gumpShapeNo);
	if (!itemshape || !itemshape->getFrame(_gumpFrameNo)) {
		warning("failed to init stat gump: no item shape");
		return;
	}

	// Paint a semi-transparent background
	const FrameID bfgrameid(GameData::GUMPS, PICKUP_GUMP_SHAPE, 0);
	// TODO: The game uses a variable number of these depending on the text length
	// For now 5 is ok.
	for (int i = 0; i < 5; i++) {
		Gump *gump = new TranslucentGump(i * bgframe->_width, 0, bgframe->_width, bgframe->_height);
		gump->SetShape(bfgrameid, false);
		gump->InitGump(this, false);
	}
	_dims.setWidth(bgframe->_width * 5);
	_dims.setHeight(bgframe->_height);

	// Paint the item name text
	TextWidget *text = new TextWidget(ITEM_AREA_WIDTH, bgframe->_height / 2 - 5, _itemName, true, ITEM_TEXT_FONT);
	text->InitGump(this, false);

	// Paint the count if needed
	addCountText();

	// Paint the item in the mid-left item area.
	const ShapeFrame *itemframe = itemshape->getFrame(_gumpFrameNo);
	Gump *itemgump = new Gump(0, _dims.height() / 2 - itemframe->_height / 2, itemframe->_width, itemframe->_height, 0, 0, LAYER_ABOVE_NORMAL);
	itemgump->SetShape(itemshape, _gumpFrameNo);
	itemgump->InitGump(this, false);
	itemgump->UpdateDimsFromShape();
	itemgump->Move(ITEM_AREA_WIDTH / 2 - itemframe->_width / 2, _dims.height() / 2 - itemframe->_height / 2);
}

void CruPickupGump::updateForNewItem(const Item *item) {
	assert(item);
	assert(item->getShape() == _itemShapeNo);
	TextWidget *oldtext = dynamic_cast<TextWidget *>(FindGump(&FindByIndex<COUNT_TEXT_INDEX>));
	if (oldtext) {
		RemoveChild(oldtext);
		oldtext->Close();
	}

	// Always show count for repeat objects.
	_showCount = true;

	// If we're updating the existing count, add 1 or special-case credits
	if (_itemShapeNo == 0x4ed)
		_q += item->getQuality();
	else
		_q++;

	addCountText();
}

void CruPickupGump::addCountText() {
	if (_q <= 1 || !_showCount)
		return;
	Std::string qstr = Std::string::format("%d", _q);
	TextWidget *count = new TextWidget(ITEM_AREA_WIDTH / 2 + 22, _dims.height() / 2 + 3, qstr, true, COUNT_TEXT_FONT);
	count->InitGump(this, false);
	count->SetIndex(COUNT_TEXT_INDEX);
}


void CruPickupGump::PaintThis(RenderSurface *surf, int32 lerp_factor, bool scaled) {
	uint32 frameno = Kernel::get_instance()->getFrameNum();
	if (!_itemShapeNo || frameno - _startFrame > 90) {
		Close();
		return;
	}
}

void CruPickupGump::saveData(Common::WriteStream *ws) {
	Gump::saveData(ws);
}

bool CruPickupGump::loadData(Common::ReadStream *rs, uint32 version) {
	return Gump::loadData(rs, version);
}

} // End of namespace Ultima8
} // End of namespace Ultima
