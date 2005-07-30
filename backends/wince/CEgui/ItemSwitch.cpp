/* ScummVM - Scumm Interpreter
 * Copyright (C) 2001-2005 The ScummVM project
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

#include "common/stdafx.h"
#include "ItemSwitch.h"

namespace CEGUI {

	ItemSwitch::ItemSwitch(WORD referenceTrue, WORD referenceFalse, bool *item) :
	PanelItem(referenceTrue) {
		_item = item;
		_backgroundTrue = _background;
		_backgroundFalse = new SDL_ImageResource();
		if (!_backgroundFalse->load(referenceFalse)) {
			delete _backgroundFalse;
			delete _background;
			_background = NULL;
			_backgroundFalse = NULL;
		}
		if (!*_item)
			_background = _backgroundFalse;
	}


	ItemSwitch::~ItemSwitch() {
		if (_backgroundFalse)
			delete _backgroundFalse;
	}

	bool ItemSwitch::action(int x, int y, bool pushed) {

		if (checkInside(x, y) && _visible && pushed) {
			*_item = !*_item;
			if (*_item)
				_background = _backgroundTrue;
			else
				_background = _backgroundFalse;

			if (_panel)
				_panel->forceRedraw();

			return true;
		}
		else
			return false;
	}
}
