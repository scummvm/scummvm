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

#include "ultima/ultima8/gumps/readable_gump.h"
#include "ultima/ultima8/gumps/widgets/text_widget.h"
#include "ultima/ultima8/games/game_data.h"
#include "ultima/ultima8/gfx/gump_shape_archive.h"
#include "ultima/ultima8/usecode/uc_machine.h"
#include "ultima/ultima8/gumps/gump_notify_process.h"
#include "ultima/ultima8/world/item.h"
#include "ultima/ultima8/world/get_object.h"
#include "ultima/ultima8/ultima8.h"

namespace Ultima {
namespace Ultima8 {

DEFINE_RUNTIME_CLASSTYPE_CODE(ReadableGump)

const int jpsub_font = 6;

ReadableGump::ReadableGump()
	: ModalGump(), _shapeNum(0), _fontNum(0) {

}

ReadableGump::ReadableGump(ObjId owner, uint16 shape, int font, const Std::string &msg) :
	ModalGump(0, 0, 100, 100, owner), _shapeNum(shape), _fontNum(font), _text(msg) {
}

ReadableGump::~ReadableGump(void) {
}

void ReadableGump::InitGump(Gump *newparent, bool take_focus) {
	ModalGump::InitGump(newparent, take_focus);

	const Shape *shape = GameData::get_instance()->getGumps()->getShape(_shapeNum);

	SetShape(shape, 0);

	UpdateDimsFromShape();

	if (Ultima8Engine::get_instance()->getGameInfo()->_language ==
	        GameInfo::GAMELANG_JAPANESE) {
		// Japanese subtitles
		Std::string::size_type pos;
		pos = _text.find('%');
		if (pos != Std::string::npos) {
			Std::string jpsub = _text.substr(pos + 1);
			_text = _text.substr(0, pos);

			Gump *subwidget = new TextWidget(0, 0, jpsub, true, jpsub_font, 0, 0, Font::TEXT_CENTER);
			subwidget->InitGump(this);
			subwidget->setRelativePosition(BOTTOM_CENTER, 0, -8);
		}
	}

	Gump *widget = new TextWidget(0, 0, _text, true, _fontNum, _dims.width() - 16, 0, Font::TEXT_CENTER);
	widget->InitGump(this);
	widget->setRelativePosition(CENTER);
}

Gump *ReadableGump::onMouseDown(int button, int32 mx, int32 my) {
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

void ReadableGump::saveData(Common::WriteStream *ws) {
	warning("Trying to load ModalGump");
}

bool ReadableGump::loadData(Common::ReadStream *rs, uint32 version) {
	warning("Trying to load ModalGump");

	return false;
}

} // End of namespace Ultima8
} // End of namespace Ultima
