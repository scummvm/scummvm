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
	static const int kMaxTooltipWidth = 150;

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

	/**
	 * Re-queues any cached hires TrueType text at the widget's current
	 * position. No-op by default; overridden by WidgetTooltip. Must be
	 * called from draw() (i.e. after doBgAnimRestoreUI()'s erase() pass
	 * has already cleared the previous frame's hires text rect this
	 * frame - see TattooScene::doBgAnim()'s call ordering), not from
	 * handleEvents() (which runs before that erase() pass and would have
	 * its queued text wiped out before ever reaching the screen).
	 */
	virtual void refreshHiresText() {}
};

class WidgetTooltip: public WidgetTooltipBase {
private:
	// Cached line-layout info from the last setText() call, replayed every
	// frame from draw() (see refreshHiresText()) so hires text stays
	// queued even when the tooltip repositions to follow the mouse
	// without setText() being called again.
	Common::String _line1, _line2;
	int _line1X, _line2X, _line2Y;

	/**
	 * Re-queues the hires TrueType rendering of the currently cached
	 * tooltip text at the tooltip's current _bounds position. Needed
	 * because hires text (unlike the bitmap _surface, which is drawn once
	 * and just re-blitted at a new position every frame) is only queued
	 * when actually written via Fonts::writeString(), and repositioning
	 * the tooltip to track the mouse doesn't call setText() again.
	 */
	void refreshHiresText() override;
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
