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

#ifndef GLOBAL_UI_H
#define GLOBAL_UI_H

#include "objects.h"

namespace Alcachofa {

class GlobalUI {
public:
	GlobalUI();

	inline Font &generalFont() const { assert(_generalFont != nullptr); return *_generalFont; }
	inline Font &dialogFont() const { assert(_dialogFont != nullptr); return *_dialogFont; }

private:
	Common::ScopedPtr<Font>
		_generalFont,
		_dialogFont;
	Common::ScopedPtr<Animation>
		_iconMortadelo,
		_iconFilemon,
		_iconInventory,
		_iconMortadeloDisabled,
		_iconFilemonDisabled,
		_iconInventoryDisabled;
};

}


#endif // GLOBAL_UI_H
