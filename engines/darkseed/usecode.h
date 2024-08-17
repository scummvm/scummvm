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

#ifndef DARKSEED_USECODE_H
#define DARKSEED_USECODE_H

#include "common/scummsys.h"
#include "console.h"
#include "inventory.h"
#include "objects.h"
#include "player.h"

namespace Darkseed {
class UseCode {
private:
	Console *_console = nullptr;
	Player *_player = nullptr;
	Objects &_objectVar;
	Inventory &_inventory;

public:
	UseCode(Console *console, Player *player, Objects &objectVar, Inventory &inventory);

public:
	void useCode(int objNum);
	void useCodeGloves(int16 targetObjNum);
	void useCodeMoney(int16 targetObjNum);
	void useCodeJournal(int16 actionObjNum, int16 targetObjNum);
	void useCodeShopItems(int16 actionObjNum, int16 targetObjNum);
	void useCrowBar(int16 targetObjNum);

private:
	void gancanim();
	void putobjunderpillow(int objNum);
	void genericresponse(int16 useObjNum, int16 targetObjNum, int16 tosIdx);
};
} // namespace Darkseed

#endif // DARKSEED_USECODE_H
