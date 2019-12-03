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
#include "ultima8/gumps/options_gump.h"

#include "ultima8/graphics/render_surface.h"
#include "ultima8/gumps/desktop_gump.h"
#include "ultima8/gumps/widgets/button_widget.h"
#include "ultima8/gumps/widgets/text_widget.h"
#include "ultima8/gumps/controls_gump.h"
#include "ultima8/gumps/paged_gump.h"

#include "ultima8/filesys/idata_source.h"
#include "ultima8/filesys/odata_source.h"
#include "common/events.h"

namespace Ultima8 {

static const int font = 0;

DEFINE_RUNTIME_CLASSTYPE_CODE(OptionsGump, Gump)

OptionsGump::OptionsGump(): Gump(0, 0, 5, 5) {
}

OptionsGump::~OptionsGump() {
}

void OptionsGump::InitGump(Gump *newparent, bool take_focus) {
	Gump::InitGump(newparent, take_focus);

	dims.w = 220;
	dims.h = 120;

	int x_ = dims.w / 2 + 14;
	int y_ = 4;
	Gump *widget;

#if 0
	widget = new ButtonWidget(x_, y_, "1. Video", true, font, 0x804000B0);
	widget->InitGump(this);
	widget->SetIndex(1);
#endif
	y_ += 14;

#if 0
	widget = new ButtonWidget(x_, y_, "2. Audio", true, font, 0x804000B0);
	widget->InitGump(this);
	widget->SetIndex(2);
#endif
	y_ += 14;

	widget = new ButtonWidget(x_, y_, "3. Controls", true, font, 0x804000B0);
	widget->InitGump(this);
	widget->SetIndex(3);
	y_ += 14;

#if 0
	widget = new ButtonWidget(x_, y_, "4. Gameplay", true, font, 0x804000B0);
	widget->InitGump(this);
	widget->SetIndex(4);
#endif
	y_ += 14;
}

void OptionsGump::ChildNotify(Gump *child, uint32 message) {
	if (message == ButtonWidget::BUTTON_CLICK) {
		selectEntry(child->GetIndex());
	}
}

void OptionsGump::PaintThis(RenderSurface *surf, int32 lerp_factor, bool scaled) {
	Gump::PaintThis(surf, lerp_factor, scaled);
}

static const int gumpShape = 35;

bool OptionsGump::OnKeyDown(int key, int mod) {
	if (key == Common::KEYCODE_ESCAPE) {
		parent->Close();
	} else if (key >= Common::KEYCODE_1 && key <= Common::KEYCODE_9) {
		// Minor hack.
		selectEntry(key - Common::KEYCODE_1 + 1);
		return true;
	}
	return true;
}

void OptionsGump::selectEntry(int entry) {
	switch (entry) {
	case 1: {
		// Video
	} break;
	case 2: {
		// Audio
	} break;
	case 3: {
		// Controls
		PagedGump *gump = new PagedGump(34, -38, 3, gumpShape);
		gump->InitGump(this);

		ControlsGump::showEngineMenu(gump);
		ControlsGump::showU8Menu(gump);

		// The parent to this gump is actually  a PagedGump,
		// and this gump does not cover the entire parent.
		gump->setRelativePosition(CENTER);
	}
	break;
	case 4: {
		// Gameplay
	} break;
	case 5: {
	} break;
	case 6: {
	} break;
	default:
		break;
	}
}

bool OptionsGump::loadData(IDataSource *ids) {
	return true;
}

void OptionsGump::saveData(ODataSource *ods) {
}

} // End of namespace Ultima8
