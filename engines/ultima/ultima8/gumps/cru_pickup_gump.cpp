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
#include "ultima/ultima8/gumps/cru_pickup_gump.h"

#include "ultima/ultima8/gumps/translucent_gump.h"
#include "ultima/ultima8/kernel/kernel.h"
#include "ultima/ultima8/games/game_data.h"
#include "ultima/ultima8/graphics/gump_shape_archive.h"
#include "ultima/ultima8/graphics/shape.h"
#include "ultima/ultima8/graphics/shape_frame.h"
#include "ultima/ultima8/world/actors/main_actor.h"
#include "ultima/ultima8/graphics/render_surface.h"
#include "ultima/ultima8/world/get_object.h"
#include "ultima/ultima8/gumps/widgets/text_widget.h"

namespace Ultima {
namespace Ultima8 {

static const int PICKUP_GUMP_SHAPE = 2;
static const int COUNT_TEXT_FONT = 12;
static const int ITEM_TEXT_FONT = 13;
static const int ITEM_AREA_WIDTH = 60;

DEFINE_RUNTIME_CLASSTYPE_CODE(CruPickupGump)
CruPickupGump::CruPickupGump() : Gump(), _startFrame(0), _shapeno(0), _q(0) {

}

CruPickupGump::CruPickupGump(Item *item, int y, uint16 currentq) : Gump(0, y, 5, 5, 0), _startFrame(0) {
	_shapeno = item->getShape();
	_q = item->getQuality();
	// TODO: should we add current q? + currentq;
	// It seems like the items are hacked to give the right "q" for
	// this gump from the last item, which is why the add process
	// uses q + 1 instead of adding the new q.
}

CruPickupGump::~CruPickupGump() {
}

void CruPickupGump::InitGump(Gump *newparent, bool take_focus) {
	Gump::InitGump(newparent, take_focus);

	if (!_shapeno)
		return;

	_startFrame = Kernel::get_instance()->getFrameNum();

	// TODO: Get the appropriate name, shape, and frame for the item here.
	Std::string itemname = "MEDICAL KIT";
	// The gump shape no for the item (not the no. of the item itself)
	// Eg, weapons are gumpshape 3, other items are gumpshape 5, etc.
	uint32 itemshapeno = 5;
	// The frame within that gump shape
	uint32 itemframeno = 0;

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

	Shape* itemshape = gumpshapes->getShape(itemshapeno);
	if (!itemshape || !itemshape->getFrame(itemframeno)) {
		warning("failed to init stat gump: no item shape");
		return;
	}

	// Paint a semi-transparent background
	const FrameID bfgrameid(GameData::GUMPS, PICKUP_GUMP_SHAPE, 0);
	// TODO: The game uses a variable number of these depending on the text length
	for (int i = 0; i < 5; i++) {
		Gump *gump = new TranslucentGump(i * bgframe->_width, 0, bgframe->_width, bgframe->_height);
		gump->SetShape(bfgrameid, false);
		gump->InitGump(this, false);
	}
	_dims.w = bgframe->_width * 5;
	_dims.h = bgframe->_height;

	// Paint the item name text
	TextWidget *text = new TextWidget(ITEM_AREA_WIDTH, bgframe->_height / 2, itemname, true, ITEM_TEXT_FONT);
	text->InitGump(this, false);

	// Paint the count if needed
	if (_q > 1) {
		Std::string qstr = Std::string::format("%d", _q);
		TextWidget *count = new TextWidget(ITEM_AREA_WIDTH / 2 + 22, _dims.h / 2 + 3, qstr, true, COUNT_TEXT_FONT);
		count->InitGump(this, false);
	}

	// Paint the item in the mid-left item area.
	const ShapeFrame *itemframe = itemshape->getFrame(itemframeno);
	Gump *itemgump = new Gump(0, _dims.h / 2 - itemframe->_height / 2, itemframe->_width, itemframe->_height, 0, 0, LAYER_ABOVE_NORMAL);
	itemgump->SetShape(itemshape, itemframeno);
	itemgump->InitGump(this, false);
	itemgump->UpdateDimsFromShape();
	itemgump->Move(ITEM_AREA_WIDTH / 2 - itemframe->_width / 2, _dims.h / 2 - itemframe->_height / 2);
}

void CruPickupGump::PaintThis(RenderSurface *surf, int32 lerp_factor, bool scaled) {
	uint32 frameno = Kernel::get_instance()->getFrameNum();
	if (!_shapeno || frameno - _startFrame > 90) {
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
