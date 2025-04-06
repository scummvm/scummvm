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

#include "common/events.h"

#include "ultima/ultima8/gumps/paged_gump.h"
#include "ultima/ultima8/games/game_data.h"
#include "ultima/ultima8/gfx/gump_shape_archive.h"
#include "ultima/ultima8/kernel/mouse.h"
#include "ultima/ultima8/gumps/widgets/button_widget.h"

namespace Ultima {
namespace Ultima8 {

DEFINE_RUNTIME_CLASSTYPE_CODE(PagedGump)

PagedGump::PagedGump(int left, int right, int top, int shape):
	ModalGump(0, 0, 5, 5), _leftOff(left), _rightOff(right), _topOff(top),
	_gumpShape(shape), _nextButton(nullptr), _prevButton(nullptr),
	_current(0), _buttonsEnabled(true) {
}

PagedGump::~PagedGump(void) {
}

void PagedGump::Close(bool no_del) {
	Mouse::get_instance()->popMouseCursor();
	for (auto *g : _gumps) {
		g->Close(no_del); // CHECKME: no_del?
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
	mouse->pushMouseCursor(Mouse::MOUSE_HAND);
}

void PagedGump::PaintThis(RenderSurface *surf, int32 lerp_factor, bool scaled) {
	Gump::PaintThis(surf, lerp_factor, scaled);
}

void PagedGump::onMouseDouble(int button, int32 mx, int32 my) {
	Close();
}

bool PagedGump::OnKeyDown(int key, int mod) {
	if (_current < _gumps.size())
		if (_gumps[_current]->OnKeyDown(key, mod))
			return true;

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

	if (message == ButtonWidget::BUTTON_UP) {
		if (cid == _nextButton->getObjId()) {
			if (_current + 1 < _gumps.size()) {
				_gumps[_current]->HideGump();
				++_current;
				_gumps[_current]->UnhideGump();
				_gumps[_current]->MakeFocus();

				if (_current + 1 == _gumps.size())
					_nextButton->HideGump();

				_prevButton->UnhideGump();
			}
		} else if (cid == _prevButton->getObjId()) {
			if (_current > 0) {
				_gumps[_current]->HideGump();
				--_current;
				_gumps[_current]->UnhideGump();
				_gumps[_current]->MakeFocus();

				if (_current == 0)
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

	_current = 0;
	_gumps[_current]->UnhideGump();
	if (_focusChild != _gumps[_current])
		_gumps[_current]->MakeFocus();

	if (_current + 1 == _gumps.size())
		_nextButton->HideGump();
	else
		_nextButton->UnhideGump();
}

void PagedGump::showPage(uint index) {
	if (index >= _gumps.size())
		return;

	_gumps[_current]->HideGump();
	_current = index;
	_gumps[_current]->UnhideGump();
	_gumps[_current]->MakeFocus();

	if (_current + 1 == _gumps.size())
		_nextButton->HideGump();
	else
		_nextButton->UnhideGump();

	if (_current == 0)
		_prevButton->HideGump();
	else
		_prevButton->UnhideGump();
}

bool PagedGump::loadData(Common::ReadStream *rs) {
	warning("Trying to load ModalGump");
	return false;
}

void PagedGump::saveData(Common::WriteStream *ws) {
	warning("Trying to save ModalGump");
}

} // End of namespace Ultima8
} // End of namespace Ultima
