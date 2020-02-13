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

#ifndef SHERLOCK_TATTOO_WIDGET_TOOLTIP_H
#define SHERLOCK_TATTOO_WIDGET_TOOLTIP_H

#include "common/scummsys.h"
#include "common/rect.h"
#include "sherlock/tattoo/widget_base.h"

namespace Sherlock {

class SherlockEngine;

namespace Tattoo {

class WidgetTooltipBase : public WidgetBase {
public:
	WidgetTooltipBase(SherlockEngine *vm) : WidgetBase(vm) {}
	~WidgetTooltipBase() override {}

	/**
	 * Erase any previous display of the widget on the screen
	 */
	void erase() override;

	/**
	 * Update the display of the widget on the screen
	 */
	void draw() override;
};

class WidgetTooltip: public WidgetTooltipBase {
public:
	int _offsetY;
public:
	WidgetTooltip(SherlockEngine *vm);
	~WidgetTooltip() override {}

	/**
	 * Set the text for the tooltip
	 */
	void setText(const Common::String &str);

	/**
	 * Handle updating the tooltip state
	 */
	void handleEvents() override;
};

class WidgetSceneTooltip : public WidgetTooltip {
public:
	WidgetSceneTooltip(SherlockEngine *vm) : WidgetTooltip(vm) {}

	/**
	 * Handle updating the tooltip state
	 */
	void handleEvents() override;
};

class WidgetMapTooltip : public WidgetTooltip {
public:
	WidgetMapTooltip(SherlockEngine *vm) : WidgetTooltip(vm) {}
};

} // End of namespace Tattoo

} // End of namespace Sherlock

#endif
