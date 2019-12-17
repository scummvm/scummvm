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
#include "ultima/ultima8/filesys/idata_source.h"
#include "ultima/ultima8/filesys/odata_source.h"
#include "ultima/ultima8/gumps/console_gump.h"
#include "ultima/ultima8/gumps/modal_gump.h"
#include "ultima/ultima8/gumps/target_gump.h"

namespace Ultima {
namespace Ultima8 {

DEFINE_RUNTIME_CLASSTYPE_CODE(DesktopGump, Gump)

bool DesktopGump::faded_modal = true;

DesktopGump::DesktopGump()
	: Gump() {
}

DesktopGump::DesktopGump(int32 _x, int32 _y, int32 _width, int32 _height) :
	Gump(_x, _y, _width, _height, 0, FLAG_DONT_SAVE | FLAG_CORE_GUMP,
	     LAYER_DESKTOP) {
}

DesktopGump::~DesktopGump(void) {
}

void DesktopGump::PaintThis(RenderSurface *surf, int32 lerp_factor, bool scaled) {
	// Just fill it (only if console showing, or in debug mode)

#ifndef DEBUG
	ConsoleGump *console = Ultima8Engine::get_instance()->getConsoleGump();
	if (console->ConsoleIsVisible() || (children.size() && children.front()->IsOfType<ConsoleGump>()))
#endif
		surf->Fill32(0x000000, 0, 0, dims.w, dims.h);
	//surf->Fill32(0x3f3f3f, 0, 0, dims.w, dims.h);
}

void DesktopGump::PaintChildren(RenderSurface *surf, int32 lerp_factor, bool scaled) {
	// Iterate all children
	std::list<Gump *>::iterator it = children.begin();
	std::list<Gump *>::iterator end = children.end();

	while (it != end) {
		Gump *g = *it;

		// Paint if not closing
		if (!g->IsClosing()) {
			// If background blanking on modal is enabled...
			// Background is partially transparent
			if (faded_modal && g->IsOfType<ModalGump>() &&
			        !g->IsOfType<TargetGump>() && !g->IsHidden())
				surf->FillBlended(0x7F000000, 0, 0, dims.w, dims.h);

			g->Paint(surf, lerp_factor, scaled);
		}

		++it;
	}
}

bool DesktopGump::StartDraggingChild(Gump *gump, int mx, int my) {
	gump->ParentToGump(mx, my);
	Mouse::get_instance()->setDraggingOffset(mx, my);
	MoveChildToFront(gump);
	return true;
}

void DesktopGump::DraggingChild(Gump *gump, int mx, int my) {
	int dx, dy;
	Mouse::get_instance()->getDraggingOffset(dx, dy);
	gump->Move(mx - dx, my - dy);
}

void DesktopGump::StopDraggingChild(Gump *gump) {

}

void DesktopGump::RenderSurfaceChanged(RenderSurface *surf) {
	// Resize the desktop gump to match the RenderSurface
	Pentagram::Rect new_dims;
	surf->GetSurfaceDims(new_dims);
	dims.w = new_dims.w;
	dims.h = new_dims.h;

	Gump::RenderSurfaceChanged();
}

void DesktopGump::RenderSurfaceChanged() {
	// Resize the desktop gump to match the parent
	Pentagram::Rect new_dims;
	parent->GetDims(new_dims);
	dims.w = new_dims.w;
	dims.h = new_dims.h;

	Gump::RenderSurfaceChanged();
}

void DesktopGump::saveData(ODataSource *ods) {
	CANT_HAPPEN_MSG("Trying to save DesktopGump");
}

bool DesktopGump::loadData(IDataSource *ids, uint32 version) {
	CANT_HAPPEN_MSG("Trying to save DesktopGump");

	return false;
}

} // End of namespace Ultima8
} // End of namespace Ultima
