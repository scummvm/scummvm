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
#include "ultima/ultima8/gumps/book_gump.h"
#include "ultima/ultima8/gumps/widgets/text_widget.h"
#include "ultima/ultima8/games/game_data.h"
#include "ultima/ultima8/gfx/gump_shape_archive.h"
#include "ultima/ultima8/usecode/uc_machine.h"
#include "ultima/ultima8/gumps/gump_notify_process.h"
#include "ultima/ultima8/world/item.h"
#include "ultima/ultima8/world/get_object.h"

namespace Ultima {
namespace Ultima8 {

DEFINE_RUNTIME_CLASSTYPE_CODE(BookGump)

BookGump::BookGump()
	: ModalGump(), _textWidgetL(0), _textWidgetR(0) {

}

BookGump::BookGump(ObjId owner, const Std::string &msg) :
	ModalGump(0, 0, 100, 100, owner), _text(msg),
	_textWidgetL(0), _textWidgetR(0) {
}

BookGump::~BookGump(void) {
}

void BookGump::InitGump(Gump *newparent, bool take_focus) {
	ModalGump::InitGump(newparent, take_focus);

	//
	// WORKAROUND (HACK), ScummVM bug #12503
	// The original game usecode has a bug which does not display the correct text for the
	// The Spell of Resurrection book.  This bug only exists in some versions of the game.
	//
	// Original book text is in the config as "translations" for "spell of resurrection".
	//
	Item *item = getItem(_owner);
	if (item && item->getShape() == 0x120 && item->getQuality() == 0x66) {
		const Std::string placeholder = "spell of resurrection";
		const Std::string replacement = _TL_(placeholder);
		if (replacement != placeholder)
			_text = replacement;
	}

	// Create the TextWidgets (NOTE: they _must_ have exactly the same _dims)
	TextWidget *widget = new TextWidget(9, 5, _text, true, 9, 123, 129, Font::TEXT_LEFT, true); //!! constants
	widget->InitGump(this);
	_textWidgetL = widget->getObjId();

	widget = new TextWidget(150, 5, _text, true, 9, 123, 129, Font::TEXT_LEFT, true); //!! constants
	widget->InitGump(this);
	_textWidgetR = widget->getObjId();
	widget->setupNextText();

	_text.clear(); // no longer need this

	//!! constant
	Shape *shapeP = GameData::get_instance()->getGumps()->getShape(6);

	SetShape(shapeP, 0);
	UpdateDimsFromShape();
}

void BookGump::NextText() {
	TextWidget *widgetL = dynamic_cast<TextWidget *>(getGump(_textWidgetL));
	TextWidget *widgetR = dynamic_cast<TextWidget *>(getGump(_textWidgetR));
	assert(widgetL);
	assert(widgetR);
	if (!widgetR->setupNextText()) {
		Close();
	}
	widgetL->setupNextText();
	widgetL->setupNextText();
	widgetR->setupNextText();
}

void BookGump::onMouseClick(int button, int32 mx, int32 my) {
	// Scroll to next text, if possible
	NextText();
}

void BookGump::onMouseDouble(int button, int32 mx, int32 my) {
	Close();
}

bool BookGump::OnKeyDown(int key, int mod) {
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

uint32 BookGump::I_readBook(const uint8 *args, unsigned int /*argsize*/) {
	ARG_ITEM_FROM_PTR(item);
	ARG_STRING(str);
	assert(item);

	Gump *gump = new BookGump(item->getObjId(), str);
	gump->InitGump(0);
	gump->setRelativePosition(CENTER);

	return gump->GetNotifyProcess()->getPid();
}

void BookGump::saveData(Common::WriteStream *ws) {
	warning("Trying to save ModalGump");
}

bool BookGump::loadData(Common::ReadStream *rs, uint32 version) {
	warning("Trying to load ModalGump");

	return false;
}

} // End of namespace Ultima8
} // End of namespace Ultima
