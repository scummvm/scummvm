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
#include "ultima/ultima8/gumps/paged_gump.h"
#include "ultima/ultima8/games/game_data.h"
#include "ultima/ultima8/graphics/gump_shape_archive.h"
#include "ultima/ultima8/graphics/shape.h"
#include "ultima/ultima8/graphics/shape_frame.h"
#include "ultima/ultima8/ultima8.h"
#include "ultima/ultima8/gumps/widgets/button_widget.h"

namespace Ultima {
namespace Ultima8 {

DEFINE_RUNTIME_CLASSTYPE_CODE(PagedGump)

PagedGump::PagedGump(int left, int right, int top, int shape):
	ModalGump(0, 0, 5, 5), _leftOff(left), _rightOff(right), _topOff(top),
	_gumpShape(shape), _nextButton(nullptr), _prevButton(nullptr),
	_buttonsEnabled(true) {
	_current = _gumps.end();
}

PagedGump::~PagedGump(void) {
}

void PagedGump::Close(bool no_del) {
	Mouse::get_instance()->popMouseCursor();
	Std::vector<Gump *>::iterator iter;
	for (iter = _gumps.begin(); iter != _gumps.end(); ++iter) {
		(*iter)->Close(no_del); // CHECKME: no_del?
	}

	ModalGump::Close(no_del);
}

static const int pageOverShape = 34;

void PagedGump::InitGump(Gump *newparent, bool take_focus) {
	ModalGump::InitGump(newparent, take_focus);

	_shape = GameData::get_instance()->getGumps()->getShape(_gumpShape);
	UpdateDimsFromShape();

	FrameID buttonleft(GameData::GUMPS, pageOverShape, 0);
	FrameID buttonright(GameData::GUMPS, pageOverShape, 1);

	//!! Hardcoded gump
	_nextButton = new ButtonWidget(0, 0, buttonright, buttonright, false,
	                              LAYER_ABOVE_NORMAL);
	_nextButton->InitGump(this);
	_nextButton->setRelativePosition(TOP_RIGHT, _rightOff, _topOff);

	_prevButton = new ButtonWidget(0, 0, buttonleft, buttonleft, false,
	                              LAYER_ABOVE_NORMAL);
	_prevButton->InitGump(this);
	_prevButton->setRelativePosition(TOP_LEFT, _leftOff, _topOff);
	_prevButton->HideGump();

	Mouse *mouse = Mouse::get_instance();
	mouse->pushMouseCursor();
	mouse->setMouseCursor(Mouse::MOUSE_HAND); // default cursor
}

void PagedGump::PaintThis(RenderSurface *surf, int32 lerp_factor, bool scaled) {
	Gump::PaintThis(surf, lerp_factor, scaled);
}



bool PagedGump::OnKeyDown(int key, int mod) {
	if (_current != _gumps.end())
		if ((*_current)->OnKeyDown(key, mod)) return true;

	switch (key) {
	case Common::KEYCODE_ESCAPE:
		Close();
		return true;
	default:
		break;
	}

	return true;
}

void PagedGump::ChildNotify(Gump *child, uint32 message) {
	if (!_buttonsEnabled) return;
	if (_gumps.empty()) return;

	ObjId cid = child->getObjId();

	if (message == ButtonWidget::BUTTON_CLICK) {
		if (cid == _nextButton->getObjId()) {
			if (_current + 1 != _gumps.end()) {
				(*_current)->HideGump();
				++_current;
				(*_current)->UnhideGump();
				(*_current)->MakeFocus();

				if (_current + 1 == _gumps.end())
					_nextButton->HideGump();

				_prevButton->UnhideGump();
			}
		} else if (cid == _prevButton->getObjId()) {
			if (_current != _gumps.begin()) {
				(*_current)->HideGump();
				--_current;
				(*_current)->UnhideGump();
				(*_current)->MakeFocus();

				if (_current == _gumps.begin())
					_prevButton->HideGump();

				_nextButton->UnhideGump();
			}
		}
	}
}

void PagedGump::addPage(Gump *g) {
	assert(g->GetParent() == this);
	g->setRelativePosition(TOP_CENTER, 0, 3 + _topOff);
	g->HideGump();
	_gumps.push_back(g);

	_current = _gumps.begin();
	(*_current)->UnhideGump();
	if (_focusChild != *_current)
		(*_current)->MakeFocus();

	if (_current + 1 == _gumps.end())
		_nextButton->HideGump();
	else
		_nextButton->UnhideGump();
}

bool PagedGump::loadData(Common::ReadStream *rs) {
	CANT_HAPPEN_MSG("Trying to load ModalGump");
	return false;
}

void PagedGump::saveData(Common::WriteStream *ws) {
	CANT_HAPPEN_MSG("Trying to save ModalGump");
}

} // End of namespace Ultima8
} // End of namespace Ultima
