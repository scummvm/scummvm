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

#include "common/array.h"
#include "graphics/surface.h"

namespace Common {
class String;
}

namespace MutationOfJB {

class InventoryWidget;

class InventoryWidgetCallback {
public:
	virtual ~InventoryWidgetCallback() {}

	/**
	 * Called when the user hovers an inventory item with the mouse or when stops hovering an item.
	 *
	 * @param widget Inventory widget.
	 * @param posInWidget Item position in the widget or -1 if none.
	 */
	virtual void onInventoryItemHovered(InventoryWidget *widget, int posInWidget) = 0;

	/**
	 * Called when the user clicks on an inventory item.
	 *
	 * @param widget Inventory widget.
	 * @param posInWidget Item position in the widget.
	 */
	virtual void onInventoryItemClicked(InventoryWidget *widget, int posInWidget) = 0;
};

class InventoryWidget : public Widget {
public:
	InventoryWidget(GuiScreen &gui, const Common::Array<Graphics::Surface> &inventorySurfaces);
	void setCallback(InventoryWidgetCallback *callback) {
		_callback = callback;
	}

	void handleEvent(const Common::Event &event) override;

protected:
	void draw(Graphics::ManagedSurface &surface) override;

private:
	void drawInventoryItem(Graphics::ManagedSurface &surface, const Common::String &item, int pos);
	const Common::Array<Graphics::Surface> &_surfaces;
	InventoryWidgetCallback *_callback;
	int _hoveredItemPos;
};

}

#endif
