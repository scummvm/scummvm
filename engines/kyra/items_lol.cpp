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

 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.

 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.
 *
 * $URL$
 * $Id$
 *
 */

#ifdef ENABLE_LOL

#include "kyra/lol.h"
#include "kyra/screen_lol.h"

namespace Kyra {

void LoLEngine::giveCredits(int credits, int redraw) {
	if (redraw)
		snd_playSoundEffect(101, -1);

	int t = credits / 30;
	if (!t)
		t = 1;

	int cnt = 0;

	while (credits) {
		if (t > credits)
			t = credits;

		if (_credits < 60 && t >= 0) {
			cnt = 0;

			do {
				if (_credits < 60) {
					int d = _stashSetupData[_credits % 12] - _credits / 12;
					if (d < 0)
						d += 5;
					_moneyColumnHeight[d]++;
				}
				_credits++;
			} while (++cnt < t);
		} else if (_credits >= 60) {
			_credits += t;
		}

		if (redraw) {
			gui_drawMoneyBox(6);
			if (credits)
				delay(_tickLength, 1);
		}
		credits -= t;
	}
}

void LoLEngine::takeCredits(int credits, int redraw) {
	if (redraw)
		snd_playSoundEffect(101, -1);

	if (credits > _credits)
		credits = _credits;

	int t = credits / 30;
	if (!t)
		t = 1;

	int cnt = 0;

	while (credits && _credits > 0) {
		if (t > credits)
			t = credits;

		if (_credits - t < 60 && t >= 0) {
			cnt = 0;

			do {
				if (--_credits < 60) {
					int d = _stashSetupData[_credits % 12] - _credits / 12;
					if (d < 0)
						d += 5;
					_moneyColumnHeight[d]--;
				}
			} while (++cnt < t);
		} else if (_credits - t < 60) {
			_credits -= t;
		}

		if (redraw) {
			gui_drawMoneyBox(6);
			if (credits)
				delay(_tickLength, 1);
		}
		credits -= t;
	}
}

int LoLEngine::makeItem(int itemIndex, int curFrame, int flags) {
	int cnt = 0;
	int r = 0;
	int i = 1;

	for (; i < 400; i++) {
		if (_itemsInPlay[i].shpCurFrame_flg & 0x8000) {
			cnt = 0;
			break;
		}

		if (_itemsInPlay[i].level < 1 || _itemsInPlay[i].level > 29 || _itemsInPlay[i].level == _currentLevel)
			continue;

		int diff = ABS(_currentLevel - _itemsInPlay[i].level);

		if (diff <= cnt)
			continue;

		bool t = false;
		int ii = i;
		while (ii && !t) {
			t = testUnkItemFlags(ii);
			if (t)
				break;
			else
				ii = _itemsInPlay[ii - 1].nextAssignedObject;
		}

		if (t) {
			cnt = diff;
			r = i;
		}
	}

	int slot = i;
	if (cnt) {
		slot = r;
		if (testUnkItemFlags(r)) {
			if (_itemsInPlay[r].nextAssignedObject)
				_itemsInPlay[_itemsInPlay[r].nextAssignedObject].level = _itemsInPlay[r].level;
			deleteItem(r);
			slot = r;
		} else {
			int ii = _itemsInPlay[slot].nextAssignedObject;
			while (ii) {
				if (testUnkItemFlags(ii)) {
					_itemsInPlay[slot].nextAssignedObject = _itemsInPlay[ii].nextAssignedObject;
					deleteItem(ii);
					slot = ii;
					break;
				} else {
					slot = ii;
				}
				ii = _itemsInPlay[slot].nextAssignedObject;
			}
		}
	}

	memset(&_itemsInPlay[slot], 0, sizeof(ItemInPlay));

	_itemsInPlay[slot].itemPropertyIndex = itemIndex;
	_itemsInPlay[slot].shpCurFrame_flg = (curFrame & 0x1fff) | flags;
	_itemsInPlay[slot].level = -1;

	return slot;
}

bool LoLEngine::addItemToInventory(int itemIndex) {
	int pos = 0;
	int i = 0;

	for (; i < 48; i++) {
		pos = _inventoryCurItem + i;
		if (pos > 47)
			pos -= 48;

		if (!_inventory[pos])
			break;
	}

	if (i == 48)
		return false;

	while ((_inventoryCurItem > pos) || ((_inventoryCurItem + 9) <= pos)) {
		if (++_inventoryCurItem > 47)
			_inventoryCurItem -= 48;
		gui_drawInventory();
	}

	_inventory[pos] = itemIndex;
	gui_drawInventory();

	return true;
}

bool LoLEngine::testUnkItemFlags(int itemIndex) {
	if (!(_itemsInPlay[itemIndex].shpCurFrame_flg & 0x4000))
		return false;

	if (_itemProperties[_itemsInPlay[itemIndex].itemPropertyIndex].flags & 4)
		return false;

	return true;

}

void LoLEngine::deleteItem(int itemIndex) {
	memset(&_itemsInPlay[itemIndex], 0, sizeof(ItemInPlay));
	_itemsInPlay[itemIndex].shpCurFrame_flg |= 0x8000;
}

ItemInPlay *LoLEngine::findObject(uint16 index) {
	if (index & 0x8000)
		return (ItemInPlay *)&_monsters[index & 0x7fff];
	else
		return &_itemsInPlay[index];
}

void LoLEngine::runItemScript(int charNum, int item, int reg0, int reg3, int reg4) {
	EMCState scriptState;
	memset(&scriptState, 0, sizeof(EMCState));

	uint8 func = item ? _itemProperties[_itemsInPlay[item].itemPropertyIndex].itemScriptFunc : 3;
	if (func == 0xff)
		return;

	_emc->init(&scriptState, &_itemScript);
	_emc->start(&scriptState, func);

	scriptState.regs[0] = reg0;
	scriptState.regs[1] = charNum;
	scriptState.regs[2] = item;
	scriptState.regs[3] = reg3;
	scriptState.regs[4] = reg4;

	while (_emc->isValid(&scriptState))
		_emc->run(&scriptState);
}

void LoLEngine::setHandItem(uint16 itemIndex) {
	if (itemIndex && _itemProperties[_itemsInPlay[itemIndex].itemPropertyIndex].flags & 0x80) {
		runItemScript(-1, itemIndex, 0x400, 0, 0);
		if (_itemsInPlay[itemIndex].shpCurFrame_flg & 0x8000)
			itemIndex = 0;
	}

	int mouseOffs = 0;

	if (itemIndex && !(_gameFlags[15] & 0x200)) {
		mouseOffs = 10;
		if (!_hideControls || textEnabled())
			_txt->printMessage(0, getLangString(0x403E), getLangString(_itemProperties[_itemsInPlay[itemIndex].itemPropertyIndex].nameStringId));
	}

	_itemInHand = itemIndex;
	_screen->setMouseCursor(mouseOffs, mouseOffs, getItemIconShapePtr(itemIndex));
}

void LoLEngine::dropItem(int item, uint16 x, uint16 y, int a, int b) {
	if (!a) {
		x = (x & 0xffc0) | 0x40;
		y = (y & 0xffc0) | 0x40;
	}

	uint16 block = calcBlockIndex(x, y);
	_itemsInPlay[item].x = x;
	_itemsInPlay[item].y = y;
	_itemsInPlay[item].blockPropertyIndex = block;
	_itemsInPlay[item].unk4 = a;

	if (b)
		_itemsInPlay[item].shpCurFrame_flg |= 0x4000;
	else
		_itemsInPlay[item].shpCurFrame_flg &= 0xbfff;

	
	assignItemToBlock(&_levelBlockProperties[block].assignedObjects, item);
	reassignDrawObjects(_currentDirection, item, &_levelBlockProperties[block], false);
	
	if (b)
		runLevelScriptCustom(block, 0x80, -1, item, 0, 0);

	checkSceneUpdateNeed(block);
}

bool LoLEngine::throwItem(int a, int item, int x, int y, int b, int direction, int c, int charNum, int d) {
	return true;
}

void LoLEngine::pickupItem(int item, int block) {
	removeAssignedItemFromBlock(&_levelBlockProperties[block].assignedObjects, item);
	removeDrawObjectFromBlock(&_levelBlockProperties[block].drawObjects, item);
	runLevelScriptCustom(block, 0x100, -1, item, 0, 0);
	_itemsInPlay[item].blockPropertyIndex = 0;
	_itemsInPlay[item].level = 0;
}

void LoLEngine::assignItemToBlock(uint16 *assignedBlockObjects, int id) {
	while (*assignedBlockObjects & 0x8000) {
		ItemInPlay *tmp = findObject(*assignedBlockObjects);
		assignedBlockObjects = &tmp->nextAssignedObject;
	}

	ItemInPlay *newObject = findObject(id);
	newObject->nextAssignedObject = *assignedBlockObjects;
	newObject->level = -1;
	*assignedBlockObjects = id;
}

int LoLEngine::checkDrawObjectSpace(int itemX, int itemY, int partyX, int partyY) {
	int a = itemX - partyX;
	if (a < 0)
		a = -a;

	int b = itemY - partyY;
	if (b < 0)
		b = -b;

	return a + b;
}

int LoLEngine::checkSceneForItems(uint16 *blockDrawObjects, int colour) {
	while (*blockDrawObjects) {
		if (!(*blockDrawObjects & 0x8000)) {
			if (!--colour)
				return *blockDrawObjects;
		}

		ItemInPlay *i = findObject(*blockDrawObjects);
		blockDrawObjects = &i->nextDrawObject;
	}

	return -1;
}

} // end of namespace Kyra

#endif // ENABLE_LOL

