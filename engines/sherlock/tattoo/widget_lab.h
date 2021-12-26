/* ScummVM - Graphic Adventure Engine
 *
 * ScummVM is the legal property of its developers, whose names
 * are too numerous to list here. Please refer to the COPYRIGHT
 * file distributed with this source distribution.
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 *
 */

#ifndef SHERLOCK_TATTOO_WIDGET_LAB_H
#define SHERLOCK_TATTOO_WIDGET_LAB_H

#include "common/scummsys.h"
#include "sherlock/tattoo/widget_base.h"
#include "sherlock/objects.h"

namespace Sherlock {

class SherlockEngine;

namespace Tattoo {

class WidgetLab: public WidgetBase {
private:
	Object *_labObject;

	/**
	 * Display tooltips of an object being dragged along with any object the dragged
	 * object is currently over
	 */
	void displayLabNames();
public:
	Common::String _remainingText;
public:
	WidgetLab(SherlockEngine *vm);
	~WidgetLab() override {}

	/**
	 * Summon the window
	 */
	void summonWindow() override;

	/**
	 * Handle event processing
	 */
	void handleEvents() override;
};

} // End of namespace Tattoo

} // End of namespace Sherlock

#endif
