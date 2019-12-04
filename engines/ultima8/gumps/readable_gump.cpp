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
#include "ultima8/gumps/readable_gump.h"

#include "ultima8/gumps/widgets/text_widget.h"
#include "ultima8/games/game_data.h"
#include "ultima8/graphics/shape.h"
#include "ultima8/graphics/gump_shape_archive.h"
#include "ultima8/graphics/shape_frame.h"
#include "ultima8/usecode/uc_machine.h"
#include "ultima8/gumps/gump_notify_process.h"
#include "ultima8/world/item.h"
#include "ultima8/world/get_object.h"
#include "ultima8/kernel/core_app.h"
#include "ultima8/games/game_info.h"
#include "ultima8/misc/util.h"

#include "ultima8/filesys/idata_source.h"
#include "ultima8/filesys/odata_source.h"

namespace Ultima8 {

DEFINE_RUNTIME_CLASSTYPE_CODE(ReadableGump, ModalGump)

const int jpsub_font = 6;

ReadableGump::ReadableGump()
	: ModalGump() {

}

ReadableGump::ReadableGump(ObjId owner_, uint16 shape_, int font, std::string msg) :
	ModalGump(0, 0, 100, 100, owner_), shapenum(shape_), fontnum(font), text(msg) {
}

ReadableGump::~ReadableGump(void) {
}

void ReadableGump::InitGump(Gump *newparent, bool take_focus) {
	ModalGump::InitGump(newparent, take_focus);

	Shape *shape_ = GameData::get_instance()->getGumps()->getShape(shapenum);

	SetShape(shape_, 0);

	ShapeFrame *sf = shape_->getFrame(0);
	assert(sf);

	dims.w = sf->width;
	dims.h = sf->height;

	if (CoreApp::get_instance()->getGameInfo()->language ==
	        GameInfo::GAMELANG_JAPANESE) {
		// Japanese subtitles
		std::string::size_type pos;
		pos = text.find('%');
		if (pos != std::string::npos) {
			std::string jpsub = text.substr(pos + 1);
			text = text.substr(0, pos);

			Gump *subwidget = new TextWidget(0, 0, jpsub, true, jpsub_font, 0, 0, Pentagram::Font::TEXT_CENTER);
			subwidget->InitGump(this);
			subwidget->setRelativePosition(BOTTOM_CENTER, 0, -8);
		}
	}

	Gump *widget = new TextWidget(0, 0, text, true, fontnum, dims.w - 16, 0, Pentagram::Font::TEXT_CENTER);
	widget->InitGump(this);
	widget->setRelativePosition(CENTER);
}

Gump *ReadableGump::OnMouseDown(int button, int mx, int my) {
	Close();
	return this;
}

bool ReadableGump::OnKeyDown(int key, int mod) {
	Close();
	return true;
}


uint32 ReadableGump::I_readGrave(const uint8 *args, unsigned int /*argsize*/) {
	ARG_ITEM_FROM_PTR(item);
	ARG_UINT16(shape);
	ARG_STRING(str);
	assert(item);

	Gump *gump = new ReadableGump(item->getObjId(), shape, 11, str);
	gump->InitGump(0);
	gump->setRelativePosition(CENTER);

	return gump->GetNotifyProcess()->getPid();
}

uint32 ReadableGump::I_readPlaque(const uint8 *args, unsigned int /*argsize*/) {
	ARG_ITEM_FROM_PTR(item);
	ARG_UINT16(shape);
	ARG_STRING(str);
	assert(item);

	Gump *gump = new ReadableGump(item->getObjId(), shape, 10, str);
	gump->InitGump(0);
	gump->setRelativePosition(CENTER);

	return gump->GetNotifyProcess()->getPid();
}

void ReadableGump::saveData(ODataSource *ods) {
	CANT_HAPPEN_MSG("Trying to load ModalGump");
}

bool ReadableGump::loadData(IDataSource *ids, uint32 version) {
	CANT_HAPPEN_MSG("Trying to load ModalGump");

	return false;
}

} // End of namespace Ultima8
