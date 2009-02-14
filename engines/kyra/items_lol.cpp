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


#include "kyra/lol.h"

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
				update();
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
				ii = _itemsInPlay[ii - 1].itemIndexUnk;
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
			if (_itemsInPlay[r].itemIndexUnk)
				_itemsInPlay[_itemsInPlay[r].itemIndexUnk].level = _itemsInPlay[r].level;
			clearItemTableEntry(r);
			slot = r;
		} else {
			int ii = _itemsInPlay[slot].itemIndexUnk;
			while (ii) {
				if (testUnkItemFlags(ii)) {
					_itemsInPlay[slot].itemIndexUnk = _itemsInPlay[ii].itemIndexUnk;
					clearItemTableEntry(ii);
					slot = ii;
					break;
				} else {
					slot = ii;
				}
				ii = _itemsInPlay[slot].itemIndexUnk;
			}
		}
	}

	memset(&_itemsInPlay[slot], 0, sizeof(ItemInPlay));

	_itemsInPlay[slot].itemPropertyIndex = itemIndex;
	_itemsInPlay[slot].shpCurFrame_flg = (curFrame & 0x1fff) | flags;
	_itemsInPlay[slot].level = -1;

	return slot;
}

bool LoLEngine::testUnkItemFlags(int itemIndex) {
	if (!(_itemsInPlay[itemIndex].shpCurFrame_flg & 0x4000))
		return false;

	if (_itemProperties[_itemsInPlay[itemIndex].itemPropertyIndex].flags & 4)
		return false;

	return true;

}

void LoLEngine::clearItemTableEntry(int itemIndex) {
	memset(&_itemsInPlay[itemIndex], 0, sizeof(ItemInPlay));
	_itemsInPlay[itemIndex].shpCurFrame_flg |= 0x8000;
}

CLevelItem *LoLEngine::findItem(uint16 index) {
	if (index & 0x8000)
		return &_cLevelItems[index & 0x7fff];
	else
		return (CLevelItem *)&_itemsInPlay[index];
}

void LoLEngine::runItemScript(int reg1, int item, int reg0, int reg3, int reg4) {
	EMCState scriptState;
	memset(&scriptState, 0, sizeof(EMCState));

	uint8 func = item ? _itemProperties[_itemsInPlay[item].itemPropertyIndex].itemScriptFunc : 3;
	if (func == 0xff)
		return;

	_emc->init(&scriptState, &_itemScript);
	_emc->start(&scriptState, func);

	scriptState.regs[0] = reg0;
	scriptState.regs[1] = reg1;
	scriptState.regs[2] = item;
	scriptState.regs[3] = reg3;
	scriptState.regs[4] = reg4;

	while (_emc->isValid(&scriptState))
		_emc->run(&scriptState);
}

} // end of namespace Kyra


