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

#include "ultima/ultima4/views/menu.h"
#include "ultima/ultima4/views/menuitem.h"
#include "ultima/ultima4/core/settings.h"
#include "ultima/ultima4/core/utils.h"

namespace Ultima {
namespace Ultima4 {

MenuItem::MenuItem(Common::String t, short x, short y, int sc) :
	_id(-1), _x(x), _y(y), _text(t), _highlighted(false),
	_selected(false), _visible(true), _scOffset(sc), _closesMenu(false) {
	// if the sc/scOffset is outside the range of the text string, assert
	assertMsg(sc == -1 || (sc >= 0 && sc <= (int)_text.size()), "sc value of %d out of range!", sc);
	if (sc != -1) addShortcutKey(tolower(_text[sc]));
}

int MenuItem::getId() const {
	return _id;
}
short MenuItem::getX() const {
	return _x;
}
short MenuItem::getY() const {
	return _y;
}
int MenuItem::getScOffset() const {
	return _scOffset;
}
Common::String MenuItem::getText() const {
	return _text;
}
bool MenuItem::isHighlighted() const {
	return _highlighted;
}
bool MenuItem::isSelected() const {
	return _selected;
}
bool MenuItem::isVisible() const {
	return _visible;
}
const Std::set<int> &MenuItem::getShortcutKeys() const {
	return _shortcutKeys;
}
bool MenuItem::getClosesMenu() const {
	return _closesMenu;
}

void MenuItem::setId(int i) {
	_id = i;
}

void MenuItem::setX(int x) {
	_x = x;
}

void MenuItem::setY(int y) {
	_y = y;
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

BoolMenuItem::BoolMenuItem(Common::String text, short xp, short yp, int shortcutKey, bool *val) :
	MenuItem(text, xp, yp, shortcutKey),
	_val(val),
	_on("On"),
	_off("Off") {
}

BoolMenuItem *BoolMenuItem::setValueStrings(const Common::String &onString, const Common::String &offString) {
	_on = onString;
	_off = offString;
	return this;
}

Common::String BoolMenuItem::getText() const {
	char buffer[64];
	snprintf(buffer, sizeof(buffer), _text.c_str(), *_val ? _on.c_str() : _off.c_str());
	return buffer;
}

void BoolMenuItem::activate(MenuEvent &event) {
	if (event.getType() == MenuEvent::DECREMENT ||
	        event.getType() == MenuEvent::INCREMENT ||
	        event.getType() == MenuEvent::ACTIVATE)
		*_val = !(*_val);
}

StringMenuItem::StringMenuItem(Common::String text, short xp, short yp, int shortcutKey,
                               Common::String *val, const Std::vector<Common::String> &validSettings) :
	MenuItem(text, xp, yp, shortcutKey),
	_val(val),
	_validSettings(validSettings) {
}

Common::String StringMenuItem::getText() const {
	char buffer[64];
	snprintf(buffer, sizeof(buffer), _text.c_str(), _val->c_str());
	return buffer;
}

void StringMenuItem::activate(MenuEvent &event) {
	Std::vector<Common::String>::const_iterator current =
	    find(_validSettings.begin(), _validSettings.end(), *_val);

	if (current == _validSettings.end())
		error("Error: menu Common::String '%s' not a valid choice", _val->c_str());

	if (event.getType() == MenuEvent::INCREMENT || event.getType() == MenuEvent::ACTIVATE) {
		/* move to the next valid choice, wrapping if necessary */
		current++;
		if (current == _validSettings.end())
			current = _validSettings.begin();
		*_val = *current;

	} else if (event.getType() == MenuEvent::DECREMENT) {
		/* move back one, wrapping if necessary */
		if (current == _validSettings.begin())
			current = _validSettings.end();
		current--;
		*_val = *current;
	}
}

IntMenuItem::IntMenuItem(Common::String text, short xp, short yp, int shortcutKey, int *val,
                         int min, int max, int increment, menuOutputType output) :
	MenuItem(text, xp, yp, shortcutKey),
	_val(val),
	_min(min),
	_max(max),
	_increment(increment),
	_output(output) {
}

Common::String IntMenuItem::getText() const {
	// do custom formatting for some menu entries,
	// and generate a Common::String of the results
	char outputBuffer[10];

	switch (_output) {
	case MENU_OUTPUT_REAGENT:
		snprintf(outputBuffer, sizeof(outputBuffer), "%2d", static_cast<short>(*_val));
		break;
	case MENU_OUTPUT_GAMMA:
		snprintf(outputBuffer, sizeof(outputBuffer), "%.1f", static_cast<float>(*_val) / 100);
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
		snprintf(outputBuffer, sizeof(outputBuffer), "%d sec", *_val);
		break;
	case MENU_OUTPUT_SPELL:
		snprintf(outputBuffer, sizeof(outputBuffer), "%3g sec", static_cast<double>(*_val) / 5);
		break;
	case MENU_OUTPUT_VOLUME:
		if (*_val == 0) {
			snprintf(outputBuffer, sizeof(outputBuffer), "Disabled");
		} else if (*_val == MAX_VOLUME) {
			snprintf(outputBuffer, sizeof(outputBuffer), "Full");
		} else {
			snprintf(outputBuffer, sizeof(outputBuffer), "%d%s%s", *_val * 10, "%", "%");
		}
		break;
	default:
		break;
	}

	// the buffer must contain a field character %d or %s depending
	// on the menuOutputType selected. MENU_OUTPUT_INT always uses
	// %d, whereas all others use %s
	char buffer[64];
	if (_output != MENU_OUTPUT_INT)
		snprintf(buffer, sizeof(buffer), _text.c_str(), outputBuffer);
	else
		snprintf(buffer, sizeof(buffer), _text.c_str(), *_val);
	return buffer;
}

void IntMenuItem::activate(MenuEvent &event) {
	if (event.getType() == MenuEvent::INCREMENT || event.getType() == MenuEvent::ACTIVATE) {
		*_val += _increment;
		if (*_val > _max)
			*_val = _min;

	} else if (event.getType() == MenuEvent::DECREMENT) {
		*_val -= _increment;
		if (*_val < _min)
			*_val = _max;
	}
}

} // End of namespace Ultima4
} // End of namespace Ultima
