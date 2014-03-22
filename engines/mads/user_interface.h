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
 */

#ifndef MADS_USER_INTERFACE_H
#define MADS_USER_INTERFACE_H

#include "common/scummsys.h"
#include "common/rect.h"
#include "common/str.h"
#include "mads/msurface.h"
#include "mads/screen.h"

namespace MADS {

class UISlot {
public:
	int _slotType;
	int _field2;
	int _spritesIndex;
	int _frameNumber;
	Common::Point _position;

	UISlot();
};

/**
 * Sprite list for the user interface
 */
class UISlots : public Common::Array<UISlot> {
private:
	MADSEngine *_vm;
public:
	/**
	 * Constructor
	 */
	UISlots(MADSEngine *vm) : _vm(vm) {}

	/**
	 * Add a sprite to the list
	 */
	void add(const Common::Point &pt, int frameNumber, int spritesIndex);

	/**
	 * Adds a special entry for full refresh of the user interface
	 */
	void fullRefresh();

	/**
	 * Draw all the sprites in the list on the user interface.
	 * @param updateFlag	Flag drawn areas to be updated on physical screen
	 * @param delFlag		Controls how used slots are deleted after drawing
	 */
	void draw(bool updateFlag, bool delFlag);
};


class UserInterface : public MSurface {
	friend class UISlots;
private:
	MADSEngine *_vm;
	int _invSpritesIndex;
	int _invFrameNumber;

	/**
	 * Loads the elements of the user interface
	 */
	void loadElements();

	/**
	 * Returns the area within the user interface a given element falls
	 */
	bool getBounds(ScrCategory category, int invIndex, Common::Rect &bounds);

	/**
	 * Reposition a bounding rectangle to physical co-ordinates
	 */
	void moveRect(Common::Rect &bounds);

	/**
	 * Draw options during a conversation.
	 */
	void drawTalkList();

	/**
	 * Draw the action list
	 */
	void drawActions();

	/**
	 * Draw the inventory list
	 */
	void drawInventoryList();

	/**
	 * Draw the inventory item vocab list
	 */
	void drawItemVocabList();

	/**
	 * Draw the inventory scroller
	 */
	void drawScrolller();

	/**
	 * Draw a UI textual element
	 */
	void writeVocab(ScrCategory category, int id);

	void refresh();
public:
	MSurface _surface;
	UISlots _uiSlots;
	DirtyAreas _dirtyAreas;
	ScrCategory _category;
	Common::Rect _drawBounds;
	Common::Rect *_rectP;
	int _inventoryTopIndex;
	int _objectY;
	int _selectedInvIndex;
	int _selectedActionIndex;
	int _selectedItemVocabIdx;
	int _scrollerY;
	int _v1A;
	int _v1C;
	int _v1E;
	bool _inventoryChanged;
	Common::StringArray _talkStrings;
	Common::Array<int> _talkIds;
public:
	/**
	* Constructor
	*/
	UserInterface(MADSEngine *vm);

	/**
	* Loads an interface from a specified resource
	*/
	void load(const Common::String &resName);

	/**
	* Set up the interface
	*/
	void setup(int id);

	void elementHighlighted();

	void drawTextElements();

	void setBounds(const Common::Rect &r);

	/**
	 * Loads the animation sprite data for a given inventory object
	 */
	void loadInventoryAnim(int objectId);

	/**
	 * Resets the inventory animation when no inventory item is selected
	 */
	void noInventoryAnim();

	/**
	* Handles queuing a new frame of an inventory animation for drawing
	*/
	void inventoryAnim();

	void categoryChanged();

	/**
	 * Select an item from the inventory list
	 * @param invIndex	Index in the inventory list of the item to select
	 */
	void selectObject(int invIndex);

	void drawInventory(int v1, int v2, int *v3);

	void scrollerChanged();
};

} // End of namespace MADS

#endif /* MADS_USER_INTERFACE_H */
