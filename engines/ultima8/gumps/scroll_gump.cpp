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

#include "ultima8/misc/pent_include.h"
#include "scroll_gump.h"
#include "text_widget.h"
#include "ultima8/games/game_data.h"
#include "Shape.h"
#include "GumpShapeArchive.h"
#include "ShapeFrame.h"
#include "ultima8/usecode/uc_machine.h"
#include "ultima8/gumps/gump_notify_process.h"
#include "ultima8/world/Item.h"
#include "ultima8/world/get_object.h"


#include "ultima8/filesys/idata_source.h"
#include "ultima8/filesys/odata_source.h"

DEFINE_RUNTIME_CLASSTYPE_CODE(ScrollGump, ModalGump);

// TODO: Remove all the hacks

ScrollGump::ScrollGump()
	: ModalGump() {

}

ScrollGump::ScrollGump(ObjId owner, std::string msg) :
	ModalGump(0, 0, 100, 100, owner), text(msg) {
}

ScrollGump::~ScrollGump(void) {
}

void ScrollGump::InitGump(Gump *newparent, bool take_focus) {
	ModalGump::InitGump(newparent, take_focus);

	// Create the TextWidget
	Gump *widget = new TextWidget(22, 29, text, true, 9, 204, 115); //!! constants
	widget->InitGump(this);
	textwidget = widget->getObjId();

	text.clear(); // no longer need this

	Shape *shape = GameData::get_instance()->getGumps()->getShape(19);

	SetShape(shape, 0);

	ShapeFrame *sf = shape->getFrame(0);
	assert(sf);

	dims.w = sf->width;
	dims.h = sf->height;
}

void ScrollGump::NextText() {
	TextWidget *widget = p_dynamic_cast<TextWidget *>(getGump(textwidget));
	assert(widget);
	if (!widget->setupNextText()) {
		Close();
	}
}

void ScrollGump::OnMouseClick(int button, int mx, int my) {
	// Scroll to next text, if possible
	NextText();
}

void ScrollGump::OnMouseDouble(int button, int mx, int my) {
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

