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
#include "darkseed/console.h"
#include "darkseed/inventory.h"
#include "darkseed/objects.h"
#include "darkseed/player.h"

namespace Darkseed {

class UseCode {
	Console *_console = nullptr;
	Player *_player = nullptr;
	Objects &_objectVar;
	Inventory &_inventory;
	uint8 _genericResponseCounter = 0;

public:
	UseCode(Console *console, Player *player, Objects &objectVar, Inventory &inventory);

	void useCode(int objNum);
	void useCodeGloves(int16 targetObjNum);
	void useCodeMoney(int16 targetObjNum);
	void useCodeJournal(int16 actionObjNum, int16 targetObjNum);
	void useCodeShopItems(int16 actionObjNum, int16 targetObjNum);
	void useCrowBar(int16 targetObjNum);
	void useCodeNewspaper(int16 targetObjNum);
	void useCodeLibraryCard(int16 targetObjNum);
	void useCodeBobbyPin(int16 targetObjNum);
	void useCodeKeys(int16 actionObjNum, int16 targetObjNum);
	void useCodeMirrorShard(int16 targetObjNum);
	void useCodeBinoculars(int16 targetObjNum);
	void useCodeShovel(int16 targetObjNum);
	void useCodeDelbertsCard(int16 targetObjNum);
	void useCodeStick(int16 targetObjNum);
	void useCodeAxeHandle(int16 targetObjNum);
	void useCodeRope(int16 targetObjNum);
	void useCodeMicroFilm(int16 targetObjNum);
	void useCodeSpecialHammer(int16 actionObjNum, int16 targetObjNum);
	void useCodeGun(int16 targetObjNum);
	void useCodeMoversNote(int16 targetObjNum);
	void useCodeBluePrints(int16 targetObjNum);
	void useCodeWatch(int16 targetObjNum);
	void useCodeTinCup(int16 targetObjNum);
	void useCodeEmptyUrn(int16 targetObjNum);
	void genericResponse(int16 useObjNum, int16 targetObjNum, int16 tosIdx);

private:
	int getHandDescriptionTosIdx(uint16 objNum);
	int16 getUseGlovesTosIdx(uint16 objNum);
	int16 getUseMoneyTosIdx(uint16 objNum);
	int16 getUseCrowbarTosIdx(uint16 objNum);
	int16 getUseJournalTosIdx(uint16 objNum);
	int16 getUseLibraryCardTosIdx(uint16 objNum);
	int16 getUseBobbyPinTosIdx(uint16 objNum);
	int16 getUseKeysTosIdx(uint16 objNum);
	int16 getUseBinocularsTosIdx(uint16 objNum);
	int16 getUseShovelTosIdx(uint16 objNum);
	int16 getUseDelbertsCardTosIdx(uint16 objNum);
	int16 getUseStickTosIdx(uint16 objNum);
	int16 getUseAxeHandleTosIdx(uint16 objNum);
	int16 getUseRopeTosIdx(uint16 objNum);
	int16 getUseMicroFilmTosIdx(uint16 objNum);
	int16 getUseSpecialHammerTosIdx(uint16 objNum);
	int16 getUseGunTosIdx(uint16 objNum);
	int16 getUseWatchTosIdx(uint16 objNum);

	void putObjUnderPillow(int objNum);

	void startDigging(int16 targetObjNum);
};
} // namespace Darkseed

#endif // DARKSEED_USECODE_H
