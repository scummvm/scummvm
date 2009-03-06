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
	static const uint8 stashSetupData[] = { 4, 4, 4, 4, 2, 2, 2, 3, 3, 0, 1, 1 };

	if (redraw)
		snd_playSoundEffect(0x65, 0xff);

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
					int d = stashSetupData[_credits % 12] - _credits / 12;
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
				ii = _itemsInPlay[ii - 1].next;
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
			if (_itemsInPlay[r].next)
				_itemsInPlay[_itemsInPlay[r].next].level = _itemsInPlay[r].level;
			deleteItem(r);
			slot = r;
		} else {
			int ii = _itemsInPlay[slot].next;
			while (ii) {
				if (testUnkItemFlags(ii)) {
					_itemsInPlay[slot].next = _itemsInPlay[ii].next;
					deleteItem(ii);
					slot = ii;
					break;
				} else {
					slot = ii;
				}
				ii = _itemsInPlay[slot].next;
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

ItemInPlay *LoLEngine::findItem(uint16 index) {
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

	if (itemIndex && !(_screen->_drawGuiFlag & 0x200)) {
		mouseOffs = 10;
		if (!_hideControls || textEnabled())
			_txt->printMessage(0, getLangString(0x403E), getLangString(_itemProperties[_itemsInPlay[itemIndex].itemPropertyIndex].nameStringId));
	}

	_itemInHand = itemIndex;
	_screen->setMouseCursor(mouseOffs, mouseOffs, getItemIconShapePtr(itemIndex));
}

void LoLEngine::clickSceneSub1() {
	assignBlockCaps(_currentBlock, _currentDirection);
	_screen->fillRect(112, 0, 287, 119, 0);

	static const uint8 sceneItemWidth[] = { 0, 254, 1, 255, 2, 0, 1, 255 } ;
	static const uint8 sceneClickTileIndex[] = { 13, 16};

	int16 x1 = 0;
	int16 x2 = 0;

	for (int i = 0; i < 2; i++) {
		uint8 tile = sceneClickTileIndex[i];
		setLevelShapesDim(sceneClickTileIndex[i], x1, x2, 13);
		uint16 s = _curBlockCaps[tile]->field_6;

		int t = (i << 7) + 1;		
		while (s) {
			if (s & 0x8000) {
				s &= 0x7fff;
				s = _monsters[i].unk2;
			} else {
				ItemInPlay *item = &_itemsInPlay[s];

				if (item->shpCurFrame_flg & 0x4000) {
					if (checkMonsterSpace(item->x, item->y, _partyPosX, _partyPosY) > 319)
						break;

					int w =	sceneItemWidth[s & 7] << 1;
					int h = sceneItemWidth[(s >> 1) & 7] + 5;
					if (item->unk4 > 1)
						h -= ((item->unk4 - 1) * 6);

					uint8 shpIx = _itemProperties[item->itemPropertyIndex].shpIndex;
					uint8 *shp = (_itemProperties[item->itemPropertyIndex].flags & 0x40) ? _gameShapes[shpIx] : _itemShapes[_gameShapeMap[shpIx]];					

					drawItemOrMonster(shp, 0, item->x, item->y, w, h, 0, t, 0);
				}

				s = item->unk2;
				t++;
			}
		}
	}
}

int LoLEngine::checkMonsterSpace(int itemX, int itemY, int partyX, int partyY) {
	int a = itemX - partyX;
	if (a < 0)
		a = -a;

	int b = itemY - partyY;
	if (b < 0)
		b = -b;

	return a + b;
}

int LoLEngine::checkSceneForItems(LevelBlockProperty *block, int pos) {
	return -1;
}

void LoLEngine::foundItemSub(int item, int block) {

}

} // end of namespace Kyra

#endif // ENABLE_LOL

