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

#ifndef MUTATIONOFJB_INVENTORYWIDGET_H
#define MUTATIONOFJB_INVENTORYWIDGET_H

#include "mutationofjb/widgets/widget.h"
#include "mutationofjb/gui.h"

namespace Common {
class String;
}

namespace MutationOfJB {

class InventoryWidget : public Widget {
public:
	InventoryWidget(Gui &gui, Gui::InventoryMap &inventoryMap, const Common::Array<Graphics::Surface>& inventorySurfaces);
	virtual void _draw(Graphics::ManagedSurface &) override;

private:
	void drawInventoryItem(Graphics::ManagedSurface &surface, const Common::String &item, int pos);
	Gui::InventoryMap &_inventoryMap;
	const Common::Array<Graphics::Surface>& _surfaces;
};

}

#endif
