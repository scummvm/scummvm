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

DEFINE_RUNTIME_CLASSTYPE_CODE(PagedGump, ModalGump)

PagedGump::PagedGump(int left, int right, int top, int shape_):
	ModalGump(0, 0, 5, 5), leftOff(left), rightOff(right), topOff(top),
	gumpShape(shape_), nextButton(0), prevButton(0), buttonsEnabled(true) {
	current = gumps.end();
}

PagedGump::~PagedGump(void) {
	gumps.clear();
}

void PagedGump::Close(bool no_del) {
	Mouse::get_instance()->popMouseCursor();
	std::vector<Gump *>::iterator iter;
	for (iter = gumps.begin(); iter != gumps.end(); ++iter) {
		(*iter)->Close(no_del); // CHECKME: no_del?
	}

	ModalGump::Close(no_del);
}

static const int pageOverShape = 34;

void PagedGump::InitGump(Gump *newparent, bool take_focus) {
	ModalGump::InitGump(newparent, take_focus);

	shape = GameData::get_instance()->getGumps()->getShape(gumpShape);
	ShapeFrame *sf = shape->getFrame(0);
	assert(sf);

	dims.w = sf->width;
	dims.h = sf->height;

	FrameID buttonleft(GameData::GUMPS, pageOverShape, 0);
	FrameID buttonright(GameData::GUMPS, pageOverShape, 1);

	//!! Hardcoded gump
	nextButton = new ButtonWidget(0, 0, buttonright, buttonright, false,
	                              LAYER_ABOVE_NORMAL);
	nextButton->InitGump(this);
	nextButton->setRelativePosition(TOP_RIGHT, rightOff, topOff);

	prevButton = new ButtonWidget(0, 0, buttonleft, buttonleft, false,
	                              LAYER_ABOVE_NORMAL);
	prevButton->InitGump(this);
	prevButton->setRelativePosition(TOP_LEFT, leftOff, topOff);
	prevButton->HideGump();

	Mouse *mouse = Mouse::get_instance();
	mouse->pushMouseCursor();
	mouse->setMouseCursor(Mouse::MOUSE_HAND); // default cursor
}

void PagedGump::PaintThis(RenderSurface *surf, int32 lerp_factor, bool scaled) {
	Gump::PaintThis(surf, lerp_factor, scaled);
}



bool PagedGump::OnKeyDown(int key, int mod) {
	if (current != gumps.end())
		if ((*current)->OnKeyDown(key, mod)) return true;

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
	if (!buttonsEnabled) return;
	if (gumps.empty()) return;

	ObjId cid = child->getObjId();

	if (message == ButtonWidget::BUTTON_CLICK) {
		if (cid == nextButton->getObjId()) {
			if (current + 1 != gumps.end()) {
				(*current)->HideGump();
				++current;
				(*current)->UnhideGump();
				(*current)->MakeFocus();

				if (current + 1 == gumps.end())
					nextButton->HideGump();

				prevButton->UnhideGump();
			}
		} else if (cid == prevButton->getObjId()) {
			if (current != gumps.begin()) {
				(*current)->HideGump();
				--current;
				(*current)->UnhideGump();
				(*current)->MakeFocus();

				if (current == gumps.begin())
					prevButton->HideGump();

				nextButton->UnhideGump();
			}
		}
	}
}

void PagedGump::addPage(Gump *g) {
	assert(g->GetParent() == this);
	g->setRelativePosition(TOP_CENTER, 0, 3 + topOff);
	g->HideGump();
	gumps.push_back(g);

	current = gumps.begin();
	(*current)->UnhideGump();
	if (focus_child != *current)
		(*current)->MakeFocus();

	if (current + 1 == gumps.end())
		nextButton->HideGump();
	else
		nextButton->UnhideGump();
}

bool PagedGump::loadData(IDataSource *ids) {
	CANT_HAPPEN_MSG("Trying to load ModalGump");
	return false;
}

void PagedGump::saveData(ODataSource *ods) {
	CANT_HAPPEN_MSG("Trying to save ModalGump");
}

} // End of namespace Ultima8
} // End of namespace Ultima
