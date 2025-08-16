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

#ifndef ALCACHOFA_GLOBAL_UI_H
#define ALCACHOFA_GLOBAL_UI_H

#include "alcachofa/objects.h"

namespace Alcachofa {

Common::Rect openInventoryTriggerBounds();
Common::Rect closeInventoryTriggerBounds();

class GlobalUI {
public:
	GlobalUI();

	inline Font &generalFont() const { assert(_generalFont != nullptr); return *_generalFont; }
	inline Font &dialogFont() const { assert(_dialogFont != nullptr); return *_dialogFont; }
	inline bool &isPermanentFaded() { return _isPermanentFaded; }

	bool updateChangingCharacter();
	void drawChangingButton();
	bool updateOpeningInventory();
	void updateClosingInventory();
	void startClosingInventory();
	void drawScreenStates(); // black borders and/or permanent fade
	void syncGame(Common::Serializer &s);

private:
	Animation *activeAnimation() const;
	bool isHoveringChangeButton() const;

	Graphic _changeButton;
	Common::ScopedPtr<Font>
		_generalFont,
		_dialogFont;
	Common::ScopedPtr<Animation>
		_iconMortadelo,
		_iconFilemon,
		_iconInventory;

	bool
		_isOpeningInventory = false,
		_isClosingInventory = false,
		_isPermanentFaded = false;
	uint32 _timeForInventory = 0;
};

Task *showCenterBottomText(Process &process, int32 dialogId, uint32 durationMs);

}


#endif // ALCACHOFA_GLOBAL_UI_H
