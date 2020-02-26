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
#include "ultima/ultima8/gumps/bind_gump.h"

#include "ultima/ultima8/graphics/fonts/font.h"
#include "ultima/ultima8/graphics/render_surface.h"
#include "ultima/ultima8/gumps/widgets/text_widget.h"
#include "ultima/ultima8/kernel/mouse.h"

#include "ultima/ultima8/kernel/hid_manager.h"

#include "ultima/ultima8/filesys/idata_source.h"
#include "ultima/ultima8/filesys/odata_source.h"

namespace Ultima {
namespace Ultima8 {

DEFINE_RUNTIME_CLASSTYPE_CODE(BindGump, ModalGump)

BindGump::BindGump(istring *b, Gump *g): ModalGump(0, 0, 160, 80), _binding(b), _invoker(g) {
}

BindGump::~BindGump() {
}

void BindGump::InitGump(Gump *newparent, bool take_focus) {
	ModalGump::InitGump(newparent, take_focus);

	//! English ! - TODO: Externalize string.
	Std::string bindtext = "Press any key or button.\n\nPress ESCAPE to cancel\nor BACKSPACE to clear";
	Gump *widget = new TextWidget(0, 0, bindtext, true, 6, 0, 0,
	                              Font::TEXT_CENTER);
	widget->InitGump(this);
	widget->setRelativePosition(TOP_CENTER, 0, 8);
}


void BindGump::PaintThis(RenderSurface *surf, int32 lerp_factor, bool scaled) {
	uint32 rgb = 0x000000;
	surf->Fill32(rgb, 0, 0, _dims.w, _dims.h);

	Gump::PaintThis(surf, lerp_factor, scaled);
}

bool BindGump::OnKeyDown(int key, int mod) {
	if (key != Common::KEYCODE_ESCAPE && _binding) {
		HIDManager *hidmanager = HIDManager::get_instance();
		if (key == Common::KEYCODE_BACKSPACE) {
			hidmanager->unbind(*_binding);
		}
		if (_invoker)
			_invoker->ChildNotify(this, UPDATE);
	}
	Close();
	return true;
}

Gump *BindGump::OnMouseDown(int button, int32 mx, int32 my) {
	//istring control = GetMouseButtonName(static_cast<MouseButton>(button));
//	HIDManager * hidmanager = HIDManager::get_instance();
//	if (binding)
//		hidmanager->bind(control, *binding);
	if (_invoker)
		_invoker->ChildNotify(this, UPDATE);
	Close();
	return this;
}

bool BindGump::loadData(IDataSource *ids) {
	CANT_HAPPEN_MSG("Trying to load ModalGump");
	return true;
}

void BindGump::saveData(ODataSource *ods) {
	CANT_HAPPEN_MSG("Trying to save ModalGump");
}

} // End of namespace Ultima8
} // End of namespace Ultima
