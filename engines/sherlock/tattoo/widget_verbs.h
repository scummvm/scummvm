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

#ifndef SHERLOCK_TATTOO_WIDGET_VERBS_H
#define SHERLOCK_TATTOO_WIDGET_VERBS_H

#include "common/scummsys.h"
#include "common/rect.h"
#include "common/str-array.h"
#include "sherlock/tattoo/widget_base.h"

namespace Sherlock {

class SherlockEngine;

namespace Tattoo {

class WidgetVerbs: public WidgetBase {
private:
	int _selector, _oldSelector;
	bool _outsideMenu;

	/**
	 * Highlights the controls for the verb list
	 */
	void highlightVerbControls();

	/**
	 * Renders the window on an internal surface for later drawing on-screen
	 */
	void render();
public:
	Common::StringArray _verbCommands;
public:
	WidgetVerbs(SherlockEngine *vm);
	~WidgetVerbs() override {}

	/**
	 * Turns on the menu with all the verbs that are available for the given object
	 */
	void load(bool objectsOn);

	/**
	 * Process input for the dialog
	 */
	void handleEvents() override;
};

} // End of namespace Tattoo

} // End of namespace Sherlock

#endif
