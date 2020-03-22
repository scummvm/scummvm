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

#include "ultima/ultima4/core/debug.h"
#include "ultima/ultima4/core/error.h"
#include "ultima/ultima4/game/menu.h"
#include "ultima/ultima4/game/menuitem.h"
#include "ultima/ultima4/game/settings.h"

namespace Ultima {
namespace Ultima4 {

/**
 * MenuItem class
 */
MenuItem::MenuItem(Common::String t, short xpos, short ypos, int sc) :
	_id(-1), x(xpos), y(ypos), _text(t), _highlighted(false),
	_selected(false), _visible(true), _scOffset(sc), _closesMenu(false) {
	// if the sc/scOffset is outside the range of the text string, assert
	ASSERT(sc == -1 || (sc >= 0 && sc <= (int)_text.size()), "sc value of %d out of range!", sc);
	if (sc != -1) addShortcutKey(tolower(_text[sc]));
}

int MenuItem::getId() const                         {
	return _id;
}
short MenuItem::getX() const                        {
	return x;
}
short MenuItem::getY() const                        {
	return y;
}
int MenuItem::getScOffset() const                   {
	return _scOffset;
}
Common::String MenuItem::getText() const                    {
	return _text;
}
bool MenuItem::isHighlighted() const                {
	return _highlighted;
}
bool MenuItem::isSelected() const                   {
	return _selected;
}
bool MenuItem::isVisible() const                    {
	return _visible;
}
const Std::set<int> &MenuItem::getShortcutKeys() const   {
	return _shortcutKeys;
}
bool MenuItem::getClosesMenu() const                {
	return _closesMenu;
}

void MenuItem::setId(int i) {
	_id = i;
}

void MenuItem::setX(int xpos) {
	x = xpos;
}

void MenuItem::setY(int ypos) {
	y = ypos;
}

void MenuItem::setText(Common::String t) {
	_text = t;
}

void MenuItem::setHighlighted(bool h) {
	_highlighted = h;
}

void MenuItem::setSelected(bool s) {
	_selected = s;
}

void MenuItem::setVisible(bool v) {
	_visible = v;
}

void MenuItem::addShortcutKey(int sc) {
	_shortcutKeys.insert(sc);
}

void MenuItem::setClosesMenu(bool closesMenu) {
	this->_closesMenu = closesMenu;
}

BoolMenuItem::BoolMenuItem(Common::String text, short x, short y, int shortcutKey, bool *val) :
	MenuItem(text, x, y, shortcutKey),
	val(val),
	on("On"),
	off("Off") {
}

BoolMenuItem *BoolMenuItem::setValueStrings(const Common::String &onString, const Common::String &offString) {
	on = onString;
	off = offString;
	return this;
}

Common::String BoolMenuItem::getText() const {
	char buffer[64];
	snprintf(buffer, sizeof(buffer), _text.c_str(), *val ? on.c_str() : off.c_str());
	return buffer;
}

void BoolMenuItem::activate(MenuEvent &event) {
	if (event.getType() == MenuEvent::DECREMENT ||
	        event.getType() == MenuEvent::INCREMENT ||
	        event.getType() == MenuEvent::ACTIVATE)
		*val = !(*val);
}

StringMenuItem::StringMenuItem(Common::String text, short x, short y, int shortcutKey,
                               Common::String *val, const Std::vector<Common::String> &validSettings) :
	MenuItem(text, x, y, shortcutKey),
	val(val),
	validSettings(validSettings) {
}

Common::String StringMenuItem::getText() const {
	char buffer[64];
	snprintf(buffer, sizeof(buffer), _text.c_str(), val->c_str());
	return buffer;
}

void StringMenuItem::activate(MenuEvent &event) {
	Std::vector<Common::String>::const_iterator current =
	    find(validSettings.begin(), validSettings.end(), *val);

	if (current == validSettings.end())
		errorFatal("Error: menu Common::String '%s' not a valid choice", val->c_str());

	if (event.getType() == MenuEvent::INCREMENT || event.getType() == MenuEvent::ACTIVATE) {
		/* move to the next valid choice, wrapping if necessary */
		current++;
		if (current == validSettings.end())
			current = validSettings.begin();
		*val = *current;

	} else if (event.getType() == MenuEvent::DECREMENT) {
		/* move back one, wrapping if necessary */
		if (current == validSettings.begin())
			current = validSettings.end();
		current--;
		*val = *current;
	}
}

IntMenuItem::IntMenuItem(Common::String text, short x, short y, int shortcutKey, int *val,
                         int min, int max, int increment, menuOutputType output) :
	MenuItem(text, x, y, shortcutKey),
	val(val),
	min(min),
	max(max),
	increment(increment),
	output(output) {
}

Common::String IntMenuItem::getText() const {
	// do custom formatting for some menu entries,
	// and generate a Common::String of the results
	char outputBuffer[10];

	switch (output) {
	case MENU_OUTPUT_REAGENT:
		snprintf(outputBuffer, sizeof(outputBuffer), "%2d", static_cast<short>(*val));
		break;
	case MENU_OUTPUT_GAMMA:
		snprintf(outputBuffer, sizeof(outputBuffer), "%.1f", static_cast<float>(*val) / 100);
		break;
	case MENU_OUTPUT_SHRINE:
		/*
		 * is this code really necessary?  the increments/decrements can be handled by IntMenuItem(),
		 * as well as the looping once the max is reached.  more importantly, the minimum value is
		 * inconstant, and based upon another setting that can be changed independent of this one.
		 * This variable could be set to it's minimum value, but when the gameCyclesPerSecond setting
		 * is changed, the value of this setting could become out of bounds.
		 *
		 * settings.shrineTime is only used in one function within shrine.cpp, and that code appears
		 * to handle the min value, caping the minimum interval at 1.
		 *
		            // make sure that the setting we're trying for is even possible
		            if (event.getType() == MenuEvent::INCREMENT || event.getType() == MenuEvent::ACTIVATE) {
		                settingsChanged.shrineTime++;
		                if (settingsChanged.shrineTime > MAX_SHRINE_TIME)
		                    settingsChanged.shrineTime = MEDITATION_MANTRAS_PER_CYCLE / settingsChanged.gameCyclesPerSecond;
		            } else if (event.getType() == MenuEvent::DECREMENT) {
		                settingsChanged.shrineTime--;
		                if (settingsChanged.shrineTime < (MEDITATION_MANTRAS_PER_CYCLE / settingsChanged.gameCyclesPerSecond))
		                    settingsChanged.shrineTime = MAX_SHRINE_TIME;
		            }
		 *
		 */
		snprintf(outputBuffer, sizeof(outputBuffer), "%d sec", *val);
		break;
	case MENU_OUTPUT_SPELL:
		snprintf(outputBuffer, sizeof(outputBuffer), "%3g sec", static_cast<double>(*val) / 5);
		break;
	case MENU_OUTPUT_VOLUME:
		if (*val == 0) {
			snprintf(outputBuffer, sizeof(outputBuffer), "Disabled");
		} else if (*val == MAX_VOLUME) {
			snprintf(outputBuffer, sizeof(outputBuffer), "Full");
		} else {
			snprintf(outputBuffer, sizeof(outputBuffer), "%d%s%s", *val * 10, "%", "%");
		}
		break;
	default:
		break;
	}

	// the buffer must contain a field character %d or %s depending
	// on the menuOutputType selected. MENU_OUTPUT_INT always uses
	// %d, whereas all others use %s
	char buffer[64];
	if (output != MENU_OUTPUT_INT)
		snprintf(buffer, sizeof(buffer), _text.c_str(), outputBuffer);
	else
		snprintf(buffer, sizeof(buffer), _text.c_str(), *val);
	return buffer;
}

void IntMenuItem::activate(MenuEvent &event) {
	if (event.getType() == MenuEvent::INCREMENT || event.getType() == MenuEvent::ACTIVATE) {
		*val += increment;
		if (*val > max)
			*val = min;

	} else if (event.getType() == MenuEvent::DECREMENT) {
		*val -= increment;
		if (*val < min)
			*val = max;
	}
}

} // End of namespace Ultima4
} // End of namespace Ultima
