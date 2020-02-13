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

#ifndef NUVIE_VIEWS_INVENTORY_FONT_H
#define NUVIE_VIEWS_INVENTORY_FONT_H

#include "ultima/nuvie/views/view.h"
#include "graphics/managed_surface.h"

namespace Ultima {
namespace Nuvie {

class Configuration;
class GUI_Button;

class DraggableView: public View {
protected:
	Graphics::ManagedSurface *bg_image;
	uint32 bg_color_key;

private:
	bool drag, need_full_redraw_when_moved, always_need_full_redraw_when_moved;
	int button_x, button_y;

public:
	DraggableView(Configuration *config);
	~DraggableView() override;

	/* events, used for dragging the area. */
	GUI_status MouseDown(int x, int y, Shared::MouseButton button) override;
	GUI_status MouseUp(int x, int y, Shared::MouseButton button) override;
	GUI_status MouseMotion(int x, int y, uint8 state) override;
	void MoveRelative(int dx, int dy) override;

protected:
	void set_bg_color_key(uint8 r, uint8 g, uint8 b);
	void force_full_redraw_if_needed();
};

} // End of namespace Nuvie
} // End of namespace Ultima

#endif
