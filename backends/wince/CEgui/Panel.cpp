/* ScummVM - Scumm Interpreter
 * Copyright (C) 2001-2004 The ScummVM project
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License
 * as published by the Free Software Foundation; either version 2
 * of the License, or (at your option) any later version.

 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.

 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.
 *
 * $Header$
 *
 */

#include "stdafx.h"
#include "Panel.h"

namespace CEGUI {

	Panel::Panel(int interleave_first, int interleave) : Toolbar()
	{
		_interleave = interleave;
		_currentItem = _x + interleave_first;
	}


	bool Panel::add(const String &name, const PanelItem *item) {
		_itemsMap.addKey(name);
		_itemsMap[name] = (PanelItem*)item;
		_itemsMap[name]->move(_currentItem, _y + 10);
		_itemsMap[name]->setPanel(this);
		_currentItem += _interleave;

		return true;
	}

	bool Panel::draw(SDL_Surface *surface) {
		ItemMap::const_iterator iterator;
		if (!_drawn && _visible) {
			GUIElement::draw(surface);
			for (iterator = _itemsMap.begin(); iterator != _itemsMap.end(); ++iterator) {
				((GUIElement*)(iterator->_value))->draw(surface);
			}
			return true;
		}
		else
			return false;
	}

	void Panel::forceRedraw() {
		ItemMap::const_iterator iterator;
		GUIElement::forceRedraw();
		for (iterator = _itemsMap.begin(); iterator != _itemsMap.end(); ++iterator)
			((GUIElement*)(iterator->_value))->forceRedraw();
	}

	bool Panel::action(int x, int y, bool pushed) {
		ItemMap::const_iterator iterator;
		bool result = false;
		if (!_visible || !checkInside(x, y))
			return false;

		for (iterator = _itemsMap.begin(); !result && iterator != _itemsMap.end(); ++iterator) 
			result = ((GUIElement*)(iterator->_value))->action(x, y, pushed);
		return result;
	}

	void Panel::clear() {
		_itemsMap.clear();
	}

	Panel::~Panel() {
		_itemsMap.clear();
	}
}
