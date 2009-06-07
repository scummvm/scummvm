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

		if (_credits < 60 && t > 0) {
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

		if (_credits - t < 60 && t > 0) {
			cnt = 0;

			do {
				if (--_credits < 60) {
					int d = _stashSetupData[_credits % 12] - _credits / 12;
					if (d < 0)
						d += 5;
					_moneyColumnHeight[d]--;
				}
			} while (++cnt < t);
		} else if (_credits - t >= 60) {
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

int LoLEngine::makeItem(int itemType, int curFrame, int flags) {
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

	_itemsInPlay[slot].itemPropertyIndex = itemType;
	_itemsInPlay[slot].shpCurFrame_flg = (curFrame & 0x1fff) | flags;
	_itemsInPlay[slot].level = -1;

	return slot;
}

void LoLEngine::placeMoveLevelItem(int itemIndex, int level, int block, int xOffs, int yOffs, int flyingHeight) {
	calcCoordinates(_itemsInPlay[itemIndex].x, _itemsInPlay[itemIndex].y, block, xOffs, yOffs);

	if (_itemsInPlay[itemIndex].block)
		removeLevelItem(itemIndex, _itemsInPlay[itemIndex].block);

	if (_currentLevel == level) {
		setItemPosition(itemIndex, _itemsInPlay[itemIndex].x, _itemsInPlay[itemIndex].y, flyingHeight, 1);
	} else {
		_itemsInPlay[itemIndex].level = level;
		_itemsInPlay[itemIndex].block = block;
		_itemsInPlay[itemIndex].flyingHeight = flyingHeight;
		_itemsInPlay[itemIndex].shpCurFrame_flg |= 0x4000;
	}
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

void LoLEngine::runItemScript(int charNum, int item, int sub, int next, int reg4) {
	EMCState scriptState;
	memset(&scriptState, 0, sizeof(EMCState));

	uint8 func = item ? _itemProperties[_itemsInPlay[item].itemPropertyIndex].itemScriptFunc : 3;
	if (func == 0xff)
		return;

	_emc->init(&scriptState, &_itemScript);
	_emc->start(&scriptState, func);

	scriptState.regs[0] = sub;
	scriptState.regs[1] = charNum;
	scriptState.regs[2] = item;
	scriptState.regs[3] = next;
	scriptState.regs[4] = reg4;

	if (_emc->isValid(&scriptState)) {
		if (*(scriptState.ip - 1) & sub) {
			while (_emc->isValid(&scriptState))
				_emc->run(&scriptState);
		}
	}
}

void LoLEngine::setHandItem(uint16 itemIndex) {
	if (itemIndex && _itemProperties[_itemsInPlay[itemIndex].itemPropertyIndex].flags & 0x80) {
		runItemScript(-1, itemIndex, 0x400, 0, 0);
		if (_itemsInPlay[itemIndex].shpCurFrame_flg & 0x8000)
			itemIndex = 0;
	}

	int mouseOffs = 0;

	if (itemIndex && !(_flagsTable[31] & 0x02)) {
		mouseOffs = 10;
		if (!_currentControlMode || textEnabled())
			_txt->printMessage(0, getLangString(0x403E), getLangString(_itemProperties[_itemsInPlay[itemIndex].itemPropertyIndex].nameStringId));
	}

	_itemInHand = itemIndex;
	_screen->setMouseCursor(mouseOffs, mouseOffs, getItemIconShapePtr(itemIndex));
}

bool LoLEngine::itemEquipped(int charNum, uint16 itemType) {
	if (charNum < 0 || charNum > 3)
		return false;

	if (!(_characters[charNum].flags & 1))
		return false;

	for (int i = 0; i < 11; i++) {
		if (!_characters[charNum].items[i])
			continue;

		if (_itemsInPlay[_characters[charNum].items[i]].itemPropertyIndex == itemType)
			return true;
	}

	return false;
}

void LoLEngine::setItemPosition(int item, uint16 x, uint16 y, int flyingHeight, int b) {
	if (!flyingHeight) {
		x = (x & 0xffc0) | 0x40;
		y = (y & 0xffc0) | 0x40;
	}

	uint16 block = calcBlockIndex(x, y);
	_itemsInPlay[item].x = x;
	_itemsInPlay[item].y = y;
	_itemsInPlay[item].block = block;
	_itemsInPlay[item].flyingHeight = flyingHeight;

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

void LoLEngine::removeLevelItem(int item, int block) {
	removeAssignedObjectFromBlock(&_levelBlockProperties[block], item);
	removeDrawObjectFromBlock(&_levelBlockProperties[block], item);
	runLevelScriptCustom(block, 0x100, -1, item, 0, 0);
	_itemsInPlay[item].block = 0;
	_itemsInPlay[item].level = 0;
}

bool LoLEngine::launchObject(int objectType, int item, int startX, int startY, int flyingHeight, int direction, int, int attackerId, int c) {
	int sp = checkDrawObjectSpace(_partyPosX, _partyPosY, startX, startY);
	FlyingObject *t = _flyingObjects;
	int slot = -1;
	int i = 0;

	for (; i < 8; i++) {
		if (!t->enable) {
			sp = -1;
			break;
		}

		int csp = checkDrawObjectSpace(_partyPosX, _partyPosY, t->x, t->y);
		if (csp > sp){
			sp = csp;
			slot = i;
		}
		t++;
	}

	if (sp != -1 && slot != -1) {
		i = slot;

		t = &_flyingObjects[i];
		endObjectFlight(t, startX, startY, 8);
	}

	if (i == 8)
		return false;

	t->enable = 1;
	t->objectType = objectType;
	t->item = item;
	t->x = startX;
	t->y = startY;
	t->flyingHeight = flyingHeight;
	t->direction = direction;
	t->distance = 255;
	t->attackerId = attackerId;
	t->flags = 7;
	t->wallFlags = 2;
	t->c = c;

	if (attackerId != -1) {
		if (attackerId & 0x8000) {
			t->flags &= 0xfd;
		} else {
			t->flags &= 0xfb;
			increaseExperience(attackerId, 1, 2);
		}
	}

	updateObjectFlightPosition(t);

	return true;
}

void LoLEngine::endObjectFlight(FlyingObject *t, int x, int y, int objectOnNextBlock) {
	int cx = x;
	int cy = y;
	uint16 block = calcBlockIndex(t->x, t->y);
	removeAssignedObjectFromBlock(&_levelBlockProperties[block], t->item);
	removeDrawObjectFromBlock(&_levelBlockProperties[block], t->item);

	if (objectOnNextBlock == 1) {
		cx = t->x;
		cy = t->y;
	}

	if (t->objectType == 0 || t->objectType == 1) {
		objectFlightProcessHits(t, cx, cy, objectOnNextBlock);
		t->x = (cx & 0xffc0) | 0x40;
		t->y = (cy & 0xffc0) | 0x40;
		t->flyingHeight = 0;
		updateObjectFlightPosition(t);
	}

	t->enable = 0;
}

void LoLEngine::processObjectFlight(FlyingObject *t, int x, int y) {
	int bl = calcBlockIndex(t->x, t->y);
	LevelBlockProperty *l = &_levelBlockProperties[bl];
	removeAssignedObjectFromBlock(l, t->item);
	removeDrawObjectFromBlock(l, t->item);
	t->x = x;
	t->y = y;
	updateObjectFlightPosition(t);
	checkSceneUpdateNeed(bl);
}

void LoLEngine::updateObjectFlightPosition(FlyingObject *t) {
	if (t->objectType == 0) {
		setItemPosition(t->item, t->x, t->y, t->flyingHeight, (t->flyingHeight == 0) ? 1 : 0);
	} else if (t->objectType == 1) {
		if (t->flyingHeight == 0) {
			deleteItem(t->item);
			checkSceneUpdateNeed(calcBlockIndex(t->x, t->y));
		} else {
			setItemPosition(t->item, t->x, t->y, t->flyingHeight, 0);
		}
	}
}

void LoLEngine::objectFlightProcessHits(FlyingObject *t, int x, int y, int objectOnNextBlock) {
	uint16 r = 0;

	if (objectOnNextBlock == 1) {
		runLevelScriptCustom(calcNewBlockPosition(_itemsInPlay[t->item].block, t->direction >> 1), 0x8000, -1, t->item, 0, 0);

	} else if (objectOnNextBlock == 2) {
		if (_itemProperties[_itemsInPlay[t->item].itemPropertyIndex].flags & 0x4000) {
			int o = _levelBlockProperties[_itemsInPlay[t->item].block].assignedObjects;
			while (o & 0x8000) {
				ItemInPlay *i = findObject(o);
				o = i->nextAssignedObject;
				runItemScript(t->attackerId, t->item, 0x8000, o, 0);
			}

		} else {
			r = getNearestMonsterFromPos(x, y);
			runItemScript(t->attackerId, t->item, 0x8000, r, 0);
		}

	} else if (objectOnNextBlock == 4) {
		_partyAwake = true;
		if (_itemProperties[_itemsInPlay[t->item].itemPropertyIndex].flags & 0x4000) {
			for (int i = 0; i < 4; i++) {
				if (_characters[i].flags & 1)
					runItemScript(t->attackerId, t->item, 0x8000, i, 0);
			}
		} else {
			r = getNearestPartyMemberFromPos(x, y);
			runItemScript(t->attackerId, t->item, 0x8000, r, 0);
		}
	}	
}

void LoLEngine::updateFlyingObject(FlyingObject *t) {
	int x = 0;
	int y = 0;
	getNextStepCoords(t->x, t->y, x, y, t->direction);
	// WORKAROUND: The next line seems to be bugged in the original code. I have fixed it in a way that at least seems to work fine.
	int objectOnNextBlock = checkBlockBeforeObjectPlacement(x, y, _itemProperties[_itemsInPlay[t->item].itemPropertyIndex].flags & 0x4000 ? 127 : 63,  t->flags, t->wallFlags);
	if (objectOnNextBlock) {
		endObjectFlight(t, x, y, objectOnNextBlock);
	} else {
		if (--t->distance) {
			processObjectFlight(t, x, y);
		} else {
			endObjectFlight(t, x, y, 8);
		}
	}
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

