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
#include "ultima/ultima8/audio/audio_process.h"
#include "ultima/ultima8/gumps/quit_gump.h"
#include "ultima/ultima8/games/game_data.h"
#include "ultima/ultima8/graphics/gump_shape_archive.h"
#include "ultima/ultima8/graphics/shape.h"
#include "ultima/ultima8/graphics/shape_frame.h"
#include "ultima/ultima8/ultima8.h"
#include "ultima/ultima8/gumps/desktop_gump.h"
#include "ultima/ultima8/gumps/widgets/button_widget.h"
#include "ultima/ultima8/gumps/widgets/text_widget.h"

namespace Ultima {
namespace Ultima8 {

DEFINE_RUNTIME_CLASSTYPE_CODE(QuitGump)

static const int u8GumpShape = 17;
static const int u8AskShapeId = 18;
static const int u8YesShapeId = 47;
static const int u8NoShapeId = 50;

static const int remGumpShape = 21;
static const int remAskShapeId = 0;
static const int remYesShapeId = 19;
static const int remNoShapeId = 20;
static const int remQuitSound = 0x109;

static const int regGumpShape = 21;
static const int regAskShapeId = 0;
static const int regYesShapeId = 19;
static const int regNoShapeId = 20;
static const int regQuitSound = 0;  // TODO: Work out what sound id

QuitGump::QuitGump(): ModalGump(0, 0, 5, 5), _yesWidget(0), _noWidget(0) {
	Mouse *mouse = Mouse::get_instance();
	mouse->pushMouseCursor();
	mouse->setMouseCursor(Mouse::MOUSE_HAND);
	if (GAME_IS_U8) {
		_gumpShape = u8GumpShape;
		_askShape = u8AskShapeId;
		_yesShape = u8YesShapeId;
		_noShape = u8NoShapeId;
		_buttonXOff = 16;
		_buttonYOff = 38;
		_playSound = 0;
	} else if (GAME_IS_REMORSE) {
		_gumpShape = remGumpShape;
		_askShape = remAskShapeId;
		_yesShape = remYesShapeId;
		_noShape = remNoShapeId;
		_buttonXOff = 55;
		_buttonYOff = 47;
		_playSound = remQuitSound;
	} else if (GAME_IS_REGRET) {
		_gumpShape = regGumpShape;
		_askShape = regAskShapeId;
		_yesShape = regYesShapeId;
		_noShape = regNoShapeId;
		// TODO: These are pretty approximate, need adjusting.
		_buttonXOff = 50;
		_buttonYOff = 70;
		_playSound = regQuitSound;
	} else {
		error("unsupported game type");
	}
}

QuitGump::~QuitGump() {
	Mouse::get_instance()->popMouseCursor();
}

void QuitGump::InitGump(Gump *newparent, bool take_focus) {
	ModalGump::InitGump(newparent, take_focus);

	_shape = GameData::get_instance()->getGumps()->getShape(_gumpShape);
	UpdateDimsFromShape();

	if (_askShape != 0) {
		FrameID askshape(GameData::GUMPS, _askShape, 0);
		askshape = _TL_SHP_(askshape);

		if (askshape._shapeNum == 0) {
			// In JP U8, the ask gump is replaced with text
			// confirming quit
			Std::string askstr = _TL_("Quit the game?");
			Gump *widget = new TextWidget(0, 0, askstr, true, 6); // CONSTANT!
			widget->InitGump(this, false);
			widget->setRelativePosition(TOP_CENTER, 0, 13);
		} else {
			const Shape *askShape = GameData::get_instance()->getShape(askshape);
			const ShapeFrame *sf = askShape->getFrame(askshape._frameNum);
			assert(sf);

			Gump *ask = new Gump(0, 0, sf->_width, sf->_height);
			ask->SetShape(askShape, askshape._frameNum);
			ask->InitGump(this);
			ask->setRelativePosition(TOP_CENTER, 0, 5);
		}
	}

	FrameID yesbutton_up(GameData::GUMPS, _yesShape, 0);
	FrameID yesbutton_down(GameData::GUMPS, _yesShape, 1);
	yesbutton_up = _TL_SHP_(yesbutton_up);
	yesbutton_down = _TL_SHP_(yesbutton_down);

	Gump *widget;
	widget = new ButtonWidget(0, 0, yesbutton_up, yesbutton_down);
	widget->InitGump(this);
	widget->setRelativePosition(TOP_LEFT, _buttonXOff, _buttonYOff);
	_yesWidget = widget->getObjId();

	FrameID nobutton_up(GameData::GUMPS, _noShape, 0);
	FrameID nobutton_down(GameData::GUMPS, _noShape, 1);
	nobutton_up = _TL_SHP_(nobutton_up);
	nobutton_down = _TL_SHP_(nobutton_down);

	widget = new ButtonWidget(0, 0, nobutton_up, nobutton_down);
	widget->InitGump(this);
	widget->setRelativePosition(TOP_RIGHT, -(int)_buttonXOff, _buttonYOff);
	_noWidget = widget->getObjId();

	if (_playSound) {
		AudioProcess *audioproc = AudioProcess::get_instance();
		audioproc->playSFX(_playSound, 0x10, _objId, 1);
	}
}


void QuitGump::PaintThis(RenderSurface *surf, int32 lerp_factor, bool scaled) {
	Gump::PaintThis(surf, lerp_factor, scaled);
}

bool QuitGump::OnKeyDown(int key, int mod) {
	switch (key) {
	case Common::KEYCODE_ESCAPE: {
		Close();
	}
	break;
	default:
		break;
	}

	return true;
}

void QuitGump::ChildNotify(Gump *child, uint32 message) {
	ObjId cid = child->getObjId();
	if (message == ButtonWidget::BUTTON_CLICK) {
		if (cid == _yesWidget) {
			Ultima8Engine::get_instance()->quitGame();
		} else if (cid == _noWidget) {
			Close();
		}
	}
}

bool QuitGump::OnTextInput(int unicode) {
	if (!(unicode & 0xFF80)) {
		char c = unicode & 0x7F;
		if (_TL_("Yy").find(c) != Std::string::npos) {
			Ultima8Engine::get_instance()->quitGame();
		} else if (_TL_("Nn").find(c) != Std::string::npos) {
			Close();
		}
	}
	return true;
}

//static
void QuitGump::verifyQuit() {
	ModalGump *gump = new QuitGump();
	gump->InitGump(0);
	gump->setRelativePosition(CENTER);
}

bool QuitGump::loadData(Common::ReadStream *rs) {
	CANT_HAPPEN_MSG("Trying to load ModalGump");
	return true;
}

void QuitGump::saveData(Common::WriteStream *ws) {
	CANT_HAPPEN_MSG("Trying to save ModalGump");
}

} // End of namespace Ultima8
} // End of namespace Ultima
