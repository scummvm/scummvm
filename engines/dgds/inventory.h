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

#ifndef DGDS_INVENTORY_H
#define DGDS_INVENTORY_H

#include "common/serializer.h"
#include "common/error.h"
#include "dgds/request.h"

namespace Graphics {
class ManagedSurface;
}

namespace Dgds {

class GameItem;

class Inventory {
public:
	Inventory();

	bool isOpen() const { return _isOpen; }
	void open();
	void close();
	void draw(Graphics::ManagedSurface &surf, int itemCount);
	void drawItems(Graphics::ManagedSurface &surf);
	void drawTime(Graphics::ManagedSurface &surf);
	void drawHeader(Graphics::ManagedSurface &surf);

	void mouseMoved(const Common::Point &pt);
	void mouseLDown(const Common::Point &pt);
	void mouseLUp(const Common::Point &pt);
	void mouseRUp(const Common::Point &pt);

	void setRequestData(const REQFileData &data);
	void setShowZoomBox(bool val) { _showZoomBox = val; }
	bool isZoomVisible() const { return _showZoomBox; }

	Common::Error syncState(Common::Serializer &s);

	static const byte HOC_CHARACTER_QUALS[];
private:
	GameItem *itemUnderMouse(const Common::Point &pt);
	bool isItemInInventory(const GameItem &item);

	uint16 _openedFromSceneNum;

	bool _showZoomBox;
	bool _isOpen;
	ButtonGadget *_prevPageBtn;
	ButtonGadget *_nextPageBtn;
	TextAreaGadget *_invClock;
	Gadget *_itemZoomBox;
	Gadget *_itemBox;
	ButtonGadget *_exitButton;
	ButtonGadget *_clockSkipMinBtn;
	ButtonGadget *_clockSkipHrBtn;
	ButtonGadget *_dropBtn;
	ImageGadget *_itemArea;
	ButtonGadget *_giveToBtn;
	ButtonGadget *_changeCharBtn;

	REQFileData _reqData;
	int _highlightItemNo;	// -1 means no item highlighted.
	int _itemOffset;  		// for scrolling through the item list
	int _fullWidth;  		// width of the box including zoom
};

} // end namespace Dgds

#endif // DGDS_INVENTORY_H
