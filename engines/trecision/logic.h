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

#ifndef TRECISION_LOGIC_H
#define TRECISION_LOGIC_H

#include "common/scummsys.h"
#include "common/serializer.h"

namespace Trecision {
class TrecisionEngine;

class LogicManager {
	TrecisionEngine *_vm;

	// panel puzzle 35
	uint16 _comb35[7];
	uint16 _count35;

	// sundial puzzle 49
	uint16 _comb49[4];

	// sundial puzzle 4CT
	uint16 _comb4CT[6];

	// keyboard puzzle 58
	uint16 _comb58[6];
	uint16 _count58;

	// SlotMachine41
	uint16 _slotMachine41Counter;

	// special management
	uint16 _wheel;
	uint16 _wheelPos[3];

	void initInventory();

public:
	LogicManager(TrecisionEngine *vm);
	~LogicManager();

	void syncGameStream(Common::Serializer &ser);

	void setupAltRoom(uint16 room, bool altRoomFl);
	void endChangeRoom();

	void useInventoryWithInventory();
	void useInventoryWithScreen();
	bool useScreenWithScreen();
	void roomOut(uint16 curObj, uint16 *action, uint16 *pos);
	bool mouseExamine(uint16 curObj);
	bool mouseOperate(uint16 curObj);
	bool mouseTake(uint16 curObj);
	bool mouseTalk(uint16 curObj);
	bool mouseClick(uint16 curObj);
	bool operateInventory();

	void doMouseGame();
	bool doMouseInventory();
	void doMouseLeftRight();

	void doSystemChangeRoom(uint16 room);

	bool isCloseupOrControlRoom() const;

private:
	void startCharacterAnimations();
	bool startPlayDialog();
	void initControlPanel();

	void handleClickControlPanel(uint16 curObj);
	void handleClickSphinxPuzzle();
	void handleClickPositioner();
	void handleClickSnakeEscape();
	void handleClickCloseup();
	void handleClickGameArea();
	void handleClickInventoryArea();
	void handleChangeRoomObjects();
};
// end of class

} // End of namespace Trecision
#endif

