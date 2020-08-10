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
#include "ultima/ultima8/gumps/computer_gump.h"
#include "ultima/ultima8/gumps/widgets/text_widget.h"
#include "ultima/ultima8/games/game_data.h"
#include "ultima/ultima8/audio/audio_process.h"
#include "ultima/ultima8/graphics/shape.h"
#include "ultima/ultima8/graphics/gump_shape_archive.h"
#include "ultima/ultima8/graphics/shape_frame.h"
#include "ultima/ultima8/usecode/uc_machine.h"
#include "ultima/ultima8/gumps/gump_notify_process.h"
#include "ultima/ultima8/world/item.h"
#include "ultima/ultima8/world/get_object.h"
#include "ultima/ultima8/kernel/core_app.h"
#include "ultima/ultima8/games/game_info.h"
#include "ultima/ultima8/misc/util.h"

namespace Ultima {
namespace Ultima8 {

DEFINE_RUNTIME_CLASSTYPE_CODE(ComputerGump)

static const int COMPUTER_FONT = 6;
static const int COMPUTER_GUMP_SHAPE = 30;
static const int COMPUTER_GUMP_SOUND = 0x33;

ComputerGump::ComputerGump()
	: ModalGump(), _textWidget(nullptr) {

}

ComputerGump::ComputerGump(const Std::string &msg) :
	ModalGump(0, 0, 100, 100), _text(msg), _textWidget(nullptr) {
}

ComputerGump::~ComputerGump(void) {
}

void ComputerGump::InitGump(Gump *newparent, bool take_focus) {
	ModalGump::InitGump(newparent, take_focus);

	Shape *shape = GameData::get_instance()->getGumps()->
							getShape(COMPUTER_GUMP_SHAPE);
	if (!shape) {
		error("Couldn't load shape for computer");
		return;
	}

	const ShapeFrame *topFrame = shape->getFrame(0);
	const ShapeFrame *botFrame = shape->getFrame(1);
	if (!topFrame || !botFrame) {
		error("Couldn't load shape frames for computer");
		return;
	}

	_dims.left = 0;
	_dims.top = 0;
	_dims.setWidth(topFrame->_width);
	_dims.setHeight(topFrame->_height + botFrame->_height);

	Gump *topGump = new Gump(0, 0, topFrame->_width, topFrame->_height);
	topGump->SetShape(shape, 0);
	topGump->InitGump(this, false);
	Gump *botGump = new Gump(0, topFrame->_height, botFrame->_width, botFrame->_height);
	botGump->SetShape(shape, 1);
	botGump->InitGump(this, false);

	_textWidget = new TextWidget(41, 38, _text, true, COMPUTER_FONT, _dims.width() - 100, 0, Font::TEXT_LEFT);
	_textWidget->InitGump(this);

	AudioProcess *audio = AudioProcess::get_instance();
	if (audio) {
		audio->playSFX(COMPUTER_GUMP_SOUND, 0x80, 0, 1);
	}
}

void ComputerGump::run() {
	ModalGump::run();
	TextWidget *widget = dynamic_cast<TextWidget *>(_textWidget);
	assert(widget);
	//widget->setupNextText();

	// TODO:
	// * Implement gradual display of the text
	// * Pause on '^' char
	// * Play sound 0x33 for each letter
	// * Add <MORE> if there is too many lines of text
}

Gump *ComputerGump::onMouseDown(int button, int32 mx, int32 my) {
	Close();
	return this;
}

bool ComputerGump::OnKeyDown(int key, int mod) {
	Close();
	return true;
}



uint32 ComputerGump::I_readComputer(const uint8 *args, unsigned int /*argsize*/) {
	ARG_STRING(str);

	Gump *gump = new ComputerGump(str);
	gump->InitGump(0);
	gump->setRelativePosition(CENTER);

	return 0;
}

void ComputerGump::saveData(Common::WriteStream *ws) {
	CANT_HAPPEN_MSG("Trying to load ModalGump");
}

bool ComputerGump::loadData(Common::ReadStream *rs, uint32 version) {
	CANT_HAPPEN_MSG("Trying to load ModalGump");

	return false;
}

} // End of namespace Ultima8
} // End of namespace Ultima
