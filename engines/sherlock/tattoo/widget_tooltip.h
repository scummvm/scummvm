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

class WidgetTooltip: public WidgetBase {
public:
	WidgetTooltip(SherlockEngine *vm);
	virtual ~WidgetTooltip() {}

	/**
	 * Set the text for the tooltip
	 */
	void setText(const Common::String &str);

	/**
	 * Draw the tooltip if necessary
	 */
	void draw();

	/**
	 * Erase the area covered by the tooltip if it's active
	 */
	void erase();

	/**
	 * Erase any area of the screen drawn by the tooltip in the previous frame
	 */
	void erasePrevious();
};

class WidgetSceneTooltip : public WidgetTooltip {
public:
	WidgetSceneTooltip(SherlockEngine *vm) : WidgetTooltip(vm) {}

	/**
	 * Handle updating the tooltip state
	 */
	virtual void handleEvents();
};

} // End of namespace Tattoo

} // End of namespace Sherlock

#endif
