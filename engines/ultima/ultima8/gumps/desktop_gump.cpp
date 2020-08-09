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
#include "ultima/ultima8/gumps/desktop_gump.h"
#include "ultima/ultima8/graphics/render_surface.h"
#include "ultima/ultima8/ultima8.h"
#include "ultima/ultima8/gumps/modal_gump.h"
#include "ultima/ultima8/gumps/target_gump.h"

namespace Ultima {
namespace Ultima8 {

DEFINE_RUNTIME_CLASSTYPE_CODE(DesktopGump)

bool DesktopGump::_fadedModal = true;

DesktopGump::DesktopGump()
	: Gump() {
}

DesktopGump::DesktopGump(int32 x, int32 y, int32 width, int32 height) :
	Gump(x, y, width, height, 0, FLAG_DONT_SAVE | FLAG_CORE_GUMP,
	     LAYER_DESKTOP) {
}

DesktopGump::~DesktopGump(void) {
}

void DesktopGump::PaintThis(RenderSurface *surf, int32 lerp_factor, bool scaled) {
}

void DesktopGump::PaintChildren(RenderSurface *surf, int32 lerp_factor, bool scaled) {
	// Iterate all children
	Std::list<Gump *>::iterator it = _children.begin();
	Std::list<Gump *>::iterator end = _children.end();

	while (it != end) {
		Gump *g = *it;

		// Paint if not closing
		if (!g->IsClosing()) {
			// If background blanking on modal is enabled...
			// Background is partially transparent
			if (_fadedModal && dynamic_cast<ModalGump *>(g) &&
			        !dynamic_cast<TargetGump *>(g) && !g->IsHidden())
				surf->FillBlended(0x7F000000, 0, 0, _dims.width(), _dims.height());

			g->Paint(surf, lerp_factor, scaled);
		}

		++it;
	}
}

bool DesktopGump::StartDraggingChild(Gump *gump, int32 mx, int32 my) {
	gump->ParentToGump(mx, my);
	Mouse::get_instance()->setDraggingOffset(mx, my);
	MoveChildToFront(gump);
	return true;
}

void DesktopGump::DraggingChild(Gump *gump, int mx, int my) {
	int32 dx, dy;
	Mouse::get_instance()->getDraggingOffset(dx, dy);
	gump->Move(mx - dx, my - dy);
}

void DesktopGump::StopDraggingChild(Gump *gump) {

}

void DesktopGump::RenderSurfaceChanged(RenderSurface *surf) {
	// Resize the desktop gump to match the RenderSurface
	Rect new_dims;
	surf->GetSurfaceDims(new_dims);
	_dims.setWidth(new_dims.width());
	_dims.setHeight(new_dims.height());

	Gump::RenderSurfaceChanged();
}

void DesktopGump::RenderSurfaceChanged() {
	// Resize the desktop gump to match the parent
	Rect new_dims;
	_parent->GetDims(new_dims);
	_dims.setWidth(new_dims.width());
	_dims.setHeight(new_dims.height());

	Gump::RenderSurfaceChanged();
}

void DesktopGump::saveData(Common::WriteStream *ws) {
	CANT_HAPPEN_MSG("Trying to save DesktopGump");
}

bool DesktopGump::loadData(Common::ReadStream *rs, uint32 version) {
	CANT_HAPPEN_MSG("Trying to save DesktopGump");

	return false;
}

} // End of namespace Ultima8
} // End of namespace Ultima
