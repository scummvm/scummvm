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

#ifndef DARKSEED_INVENTORY_H
#define DARKSEED_INVENTORY_H

#include "common/list.h"
namespace Darkseed {

class Inventory {
private:
	Common::Array<uint8> _inventory;
	int16 _inventoryLength = 0;
	Common::Array<uint8> _iconList;
	int _viewOffset = 0;
	int _numIcons = 0;
	bool _isVisible = false;
	bool _redraw = false;

public:
	Inventory();
	void reset();
	void addItem(uint8 item);
	void removeItem(uint8 item);
	void draw();
	void handleClick();
	void endOfDayOutsideLogic();
	void gotoJailLogic();
	bool hasObject(uint8 objNum);

	Common::Error sync(Common::Serializer &s);

private:
	void update();
	void leftArrowClicked();
	void rightArrowClicked();
	void restoreFrame();

};

} // namespace Darkseed

#endif // DARKSEED_INVENTORY_H
