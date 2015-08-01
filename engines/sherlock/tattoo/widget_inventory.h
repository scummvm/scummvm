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

#ifndef SHERLOCK_TATTOO_WIDGET_INVENTORY_H
#define SHERLOCK_TATTOO_WIDGET_INVENTORY_H

#include "common/scummsys.h"
#include "sherlock/tattoo/widget_base.h"
#include "sherlock/tattoo/widget_tooltip.h"

namespace Sherlock {

class SherlockEngine;

namespace Tattoo {

#define NUM_INVENTORY_SHOWN 8		// Number of Inventory Items Shown

class WidgetInventory;

class WidgetInventoryTooltip: public WidgetTooltipBase {
private:
	WidgetInventory *_owner;
protected:
	/**
	* Overriden from base class, since tooltips have a completely transparent background
	*/
	virtual void drawBackground() {}
public:
	WidgetInventoryTooltip(SherlockEngine *vm, WidgetInventory *owner);
	virtual ~WidgetInventoryTooltip() {}

	/**
	 * Set the text for the tooltip
	 */
	void setText(const Common::String &str);

	/**
	 * Handle updating the tooltip state
	 */
	virtual void handleEvents();
};

class WidgetInventoryVerbs : public WidgetBase {
private:
	WidgetInventory *_owner;
	Common::StringArray _inventCommands;

	void highlightControls();
public:
	int _invVerbSelect, _oldInvVerbSelect;
public:
	WidgetInventoryVerbs(SherlockEngine *vm, WidgetInventory *owner);
	virtual ~WidgetInventoryVerbs() {}

	void load();

	/**
	 * Handle updating the tooltip state
	 */
	virtual void handleEvents();
};

class WidgetInventory: public WidgetBase {
	friend class WidgetInventoryTooltip;
	friend class WidgetInventoryVerbs;
private:
	int _invVerbMode;
	int _selector, _oldSelector;
	int _invSelect, _oldInvSelect;
	WidgetInventoryTooltip _tooltipWidget;
	WidgetInventoryVerbs _verbList;
	bool _swapItems;
	Surface _menuSurface;
	Common::String _invTarget;

	/**
	 * Draw the bars within the dialog
	 */
	void drawBars();

	/**
	 * Check for keys to mouse the mouse within the inventory dialog
	 */
	void checkInvTabbingKeys();

	/**
	 * Highlights the controls
	 */
	void highlightControls();
public:
	int _invMode;
	Common::String _action;
	Common::String _verb;
public:
	WidgetInventory(SherlockEngine *vm);
	virtual ~WidgetInventory() {}

	/**
	 * Load the inventory window
	 */
	void load(int mode);

	/**
	 * Draw the inventory on the surface
	 */
	void drawInventory();

	/**
	 * Close the window
	 */
	void close();

	/**
	 * Handle events whilst the widget is on-screen
	 */
	virtual void handleEvents();

	/**
	 * Close a currently active menu
	 */
	virtual void banishWindow();

	/**
	 * Erase any previous display of the widget on the screen
	 */
	virtual void erase();

	/**
	 * Update the display of the widget on the screen
	 */
	virtual void draw();
};

} // End of namespace Tattoo

} // End of namespace Sherlock

#endif
