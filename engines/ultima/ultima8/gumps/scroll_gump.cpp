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

#include "common/keyboard.h"
#include "ultima/ultima8/gumps/scroll_gump.h"
#include "ultima/ultima8/gumps/widgets/text_widget.h"
#include "ultima/ultima8/games/game_data.h"
#include "ultima/ultima8/gfx/gump_shape_archive.h"
#include "ultima/ultima8/usecode/uc_machine.h"
#include "ultima/ultima8/gumps/gump_notify_process.h"
#include "ultima/ultima8/world/item.h"
#include "ultima/ultima8/world/get_object.h"

namespace Ultima {
namespace Ultima8 {

DEFINE_RUNTIME_CLASSTYPE_CODE(ScrollGump)

// TODO: Remove all the hacks

ScrollGump::ScrollGump()
	: ModalGump(), _textWidget(0) {

}

ScrollGump::ScrollGump(ObjId owner, const Std::string &msg) :
	ModalGump(0, 0, 100, 100, owner), _text(msg), _textWidget(0) {
}

ScrollGump::~ScrollGump(void) {
}

void ScrollGump::InitGump(Gump *newparent, bool take_focus) {
	ModalGump::InitGump(newparent, take_focus);

	// Create the TextWidget
	Gump *widget = new TextWidget(22, 29, _text, true, 9, 204, 115, Font::TEXT_LEFT, true); //!! constants
	widget->InitGump(this);
	_textWidget = widget->getObjId();

	_text.clear(); // no longer need this

	const Shape *shape = GameData::get_instance()->getGumps()->getShape(19);

	SetShape(shape, 0);
	UpdateDimsFromShape();
}

void ScrollGump::NextText() {
	TextWidget *widget = dynamic_cast<TextWidget *>(getGump(_textWidget));
	assert(widget);
	if (!widget->setupNextText()) {
		Close();
	}
}

void ScrollGump::onMouseClick(int button, int32 mx, int32 my) {
	// Scroll to next _text, if possible
	NextText();
}

void ScrollGump::onMouseDouble(int button, int32 mx, int32 my) {
	Close();
}

bool ScrollGump::OnKeyDown(int key, int mod) {
	switch (key) {
	case Common::KEYCODE_ESCAPE:
		Close();
		break;
	case Common::KEYCODE_SPACE:
		NextText();
		break;
	default:
		break;
	}

	return true;
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

void ScrollGump::saveData(Common::WriteStream *ws) {
	warning("Trying to save ModalGump");
}

bool ScrollGump::loadData(Common::ReadStream *rs, uint32 version) {
	warning("Trying to load ModalGump");

	return false;
}

} // End of namespace Ultima8
} // End of namespace Ultima
