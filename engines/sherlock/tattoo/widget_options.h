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

#ifndef SHERLOCK_TATTOO_WIDGET_OPTIONS_H
#define SHERLOCK_TATTOO_WIDGET_OPTIONS_H

#include "common/scummsys.h"
#include "sherlock/tattoo/widget_base.h"

namespace Sherlock {

class SherlockEngine;

namespace Tattoo {

enum OptionRenderMode { OP_ALL = 0, OP_CONTENTS = 1, OP_NAMES = 2};

/**
 * Handles displaying the options dialog
 */
class WidgetOptions : public WidgetBase {
private:
	int _midiSliderX, _digiSliderX;
	int _selector, _oldSelector;
	Common::Point _centerPos;

	/**
	 * Render the contents of the dialog onto the widget's surface
	 */
	void render(OptionRenderMode mode = OP_ALL);
public:
	WidgetOptions(SherlockEngine *vm);
	~WidgetOptions() override {}

	/**
	 * Load and then display the options dialog
	 */
	void load();

	/**
	 * Handle event processing
	 */
	void handleEvents() override;
};

} // End of namespace Tattoo

} // End of namespace Sherlock

#endif
