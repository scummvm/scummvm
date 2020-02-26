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
#include "ultima/ultima8/gumps/scroll_gump.h"
#include "ultima/ultima8/gumps/widgets/text_widget.h"
#include "ultima/ultima8/games/game_data.h"
#include "ultima/ultima8/graphics/shape.h"
#include "ultima/ultima8/graphics/gump_shape_archive.h"
#include "ultima/ultima8/graphics/shape_frame.h"
#include "ultima/ultima8/usecode/uc_machine.h"
#include "ultima/ultima8/gumps/gump_notify_process.h"
#include "ultima/ultima8/world/item.h"
#include "ultima/ultima8/world/get_object.h"
#include "ultima/ultima8/filesys/idata_source.h"
#include "ultima/ultima8/filesys/odata_source.h"

namespace Ultima {
namespace Ultima8 {

DEFINE_RUNTIME_CLASSTYPE_CODE(ScrollGump, ModalGump)

// TODO: Remove all the hacks

ScrollGump::ScrollGump()
	: ModalGump() {

}

ScrollGump::ScrollGump(ObjId owner_, Std::string msg) :
	ModalGump(0, 0, 100, 100, owner_), _text(msg) {
}

ScrollGump::~ScrollGump(void) {
}

void ScrollGump::InitGump(Gump *newparent, bool take_focus) {
	ModalGump::InitGump(newparent, take_focus);

	// Create the TextWidget
	Gump *widget = new TextWidget(22, 29, _text, true, 9, 204, 115); //!! constants
	widget->InitGump(this);
	_textWidget = widget->getObjId();

	_text.clear(); // no longer need this

	Shape *shape_ = GameData::get_instance()->getGumps()->getShape(19);

	SetShape(shape_, 0);

	ShapeFrame *sf = shape_->getFrame(0);
	assert(sf);

	_dims.w = sf->_width;
	_dims.h = sf->_height;
}

void ScrollGump::NextText() {
	TextWidget *widget = p_dynamic_cast<TextWidget *>(getGump(_textWidget));
	assert(widget);
	if (!widget->setupNextText()) {
		Close();
	}
}

void ScrollGump::OnMouseClick(int button, int32 mx, int32 my) {
	// Scroll to next _text, if possible
	NextText();
}

void ScrollGump::OnMouseDouble(int button, int32 mx, int32 my) {
	Close();
}

uint32 ScrollGump::I_readScroll(const uint8 *args, unsigned int /*argsize*/) {
	ARG_ITEM_FROM_PTR(item);
	ARG_STRING(str);
	assert(item);

	Gump *gump = new ScrollGump(item->getObjId(), str);
	gump->InitGump(0);
	gump->setRelativePosition(CENTER);

	return gump->GetNotifyProcess()->getPid();
}

void ScrollGump::saveData(ODataSource *ods) {
	CANT_HAPPEN_MSG("Trying to save ModalGump");
}

bool ScrollGump::loadData(IDataSource *ids, uint32 version) {
	CANT_HAPPEN_MSG("Trying to load ModalGump");

	return false;
}

} // End of namespace Ultima8
} // End of namespace Ultima
