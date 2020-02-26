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
#include "ultima/ultima8/gumps/quit_gump.h"
#include "ultima/ultima8/games/game_data.h"
#include "ultima/ultima8/graphics/gump_shape_archive.h"
#include "ultima/ultima8/graphics/shape.h"
#include "ultima/ultima8/graphics/shape_frame.h"
#include "ultima/ultima8/ultima8.h"
#include "ultima/ultima8/gumps/desktop_gump.h"
#include "ultima/ultima8/gumps/widgets/button_widget.h"
#include "ultima/ultima8/gumps/widgets/text_widget.h"
#include "ultima/ultima8/filesys/idata_source.h"
#include "ultima/ultima8/filesys/odata_source.h"

namespace Ultima {
namespace Ultima8 {

DEFINE_RUNTIME_CLASSTYPE_CODE(QuitGump, ModalGump)

QuitGump::QuitGump(): ModalGump(0, 0, 5, 5) {
	Mouse *mouse = Mouse::get_instance();
	mouse->pushMouseCursor();
	mouse->setMouseCursor(Mouse::MOUSE_HAND);
}

QuitGump::~QuitGump() {
	Mouse::get_instance()->popMouseCursor();
}

static const int gumpShape = 17;
static const int askShapeId = 18;
static const int yesShapeId = 47;
static const int noShapeId = 50;

void QuitGump::InitGump(Gump *newparent, bool take_focus) {
	ModalGump::InitGump(newparent, take_focus);

	_shape = GameData::get_instance()->getGumps()->getShape(gumpShape);
	ShapeFrame *sf = _shape->getFrame(0);
	assert(sf);

	_dims.w = sf->_width;
	_dims.h = sf->_height;

	FrameID askshape(GameData::GUMPS, askShapeId, 0);
	askshape = _TL_SHP_(askshape);

	Shape *askShape = GameData::get_instance()->getShape(askshape);
	sf = askShape->getFrame(askshape._frameNum);
	assert(sf);

	Gump *ask = new Gump(0, 0, sf->_width, sf->_height);
	ask->SetShape(askShape, askshape._frameNum);
	ask->InitGump(this);
	ask->setRelativePosition(TOP_CENTER, 0, 5);

	FrameID yesbutton_up(GameData::GUMPS, yesShapeId, 0);
	FrameID yesbutton_down(GameData::GUMPS, yesShapeId, 1);
	yesbutton_up = _TL_SHP_(yesbutton_up);
	yesbutton_down = _TL_SHP_(yesbutton_down);

	Gump *widget;
	widget = new ButtonWidget(0, 0, yesbutton_up, yesbutton_down);
	widget->InitGump(this);
	widget->setRelativePosition(TOP_LEFT, 16, 38);
	_yesWidget = widget->getObjId();

	FrameID nobutton_up(GameData::GUMPS, noShapeId, 0);
	FrameID nobutton_down(GameData::GUMPS, noShapeId, 1);
	nobutton_up = _TL_SHP_(nobutton_up);
	nobutton_down = _TL_SHP_(nobutton_down);

	widget = new ButtonWidget(0, 0, nobutton_up, nobutton_down);
	widget->InitGump(this);
	widget->setRelativePosition(TOP_RIGHT, -16, 38);
	_noWidget = widget->getObjId();
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
			Ultima8Engine::get_instance()->ForceQuit();
		} else if (cid == _noWidget) {
			Close();
		}
	}
}

bool QuitGump::OnTextInput(int unicode) {
	if (!(unicode & 0xFF80)) {
		char c = unicode & 0x7F;
		if (_TL_("Yy").find(c) != Std::string::npos) {
			Ultima8Engine::get_instance()->ForceQuit();
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

bool QuitGump::loadData(IDataSource *ids) {
	CANT_HAPPEN_MSG("Trying to load ModalGump");
	return true;
}

void QuitGump::saveData(ODataSource *ods) {
	CANT_HAPPEN_MSG("Trying to save ModalGump");
}

} // End of namespace Ultima8
} // End of namespace Ultima
