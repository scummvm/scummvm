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

#include "sky/logic.h"
#include "sky/compact.h"
#include "sky/text.h"
#include "sky/screen.h"
#include "sky/mouse.h"
#include "sky/sky.h"


namespace Sky {

const int invIconId[NUM_INV_ANIMS + 1] = {
	2304,  3201,  3203,  3205,  3207,
	3209,  3211,  4672,  4674,  5568,
	5570, 12225, 12227, 12229, 12231,
	12233, 12235, 12237, 12239, 12241,
	12243, 12245, 12247, 12249, 12251,
	12253, 12255, 12257, 16768, 16832,
	16896, 16960, 17024, 17088, 17152,
	17154, 17156, 17158, 17160, 17162,
	17164, 17216, 17344, 17408,  -1,
};

void Screen::addInvIcon(int frame, int x, int y, bool highlighted) {
	if (_invIconsInUse >= NUM_INV_ICONS)
		return;

	_invIcon[_invIconsInUse]._anim = _invAnim[getInventoryAnimIdx(frame)];
	_invIcon[_invIconsInUse].set(x, y);
	_invIcon[_invIconsInUse]._curFrame = highlighted ? 1 : 0;

	_invIconsInUse++;
}

void Screen::showInventory(int x1, int y1, int x2, int y2) {
	_invX1 = x1;
	_invY1 = y1;
	_invX2 = x2;
	_invY2 = y2;
	_invVisible = true;
}

void Screen::hideInventory() {
	_invVisible = false;
}

void Screen::clearAllInvIcons() {
	for (int i = 0; i < NUM_INV_ICONS; i++)
		_invIcon[i]._visible = false;

	_invIconsInUse = 0;
}

int Screen::getInventoryAnimIdx(int frame) {
	for (int i = 0; i < NUM_INV_ANIMS; i++) {
		if (frame == invIconId[i])
			return i;
	}

	debug(1, "Couldn't find inventory item%d", frame);
	return 0; // fallback
}

void Logic::startInventory(uint32 highlightedId) {
	// normal gameplay inv
	fnStartMenu(Logic::_scriptVariables[FIRST_ICON], highlightedId, 0);
}

void Logic::killInventory() {
	// shut down displayed inv objects
	int j;
	Compact *itemData;

	if (_liveInv) {
		for (j = 0; j < _scriptVariables[MENU_LENGTH]; j++) {
			// fetch the compact
			itemData = _skyCompact->fetchCpt(_objectList[j]);

			itemData->status = 0;
		}
	}
	_liveInv = false;

	// kill inv bg
	_skyScreen->hideInventory();
	_skyScreen->clearAllInvIcons();
}

} // End of namespace Sky
