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
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.
 *
 */

#ifndef TITANIC_PET_INVENTORY_H
#define TITANIC_PET_INVENTORY_H

#include "titanic/support/simple_file.h"
#include "titanic/pet_control/pet_section.h"
#include "titanic/pet_control/pet_inventory_glyphs.h"
#include "titanic/gfx/text_control.h"

namespace Titanic {

/**
 * Handles displaying the player's inventory in the PET
 */
class CPetInventory : public CPetSection {
private:
	CTextControl _text;
	CPetInventoryGlyphs _items;
	CGameObject *_itemBackgrounds[46];
	CGameObject *_movie;
	bool _isLoading;
	int _titaniaBitFlags;
private:
	/**
	 * Handles initial setup
	 */
	bool setPetControl(CPetControl *petControl);

	/**
	 * Get the index of an item added to the PET
	 */
	int getItemIndex(CGameObject *item) const;

	/**
	 * Remove any invalid inventory glyphs
	 */
	void removeInvalid();
public:
	CPetInventory();

	/**
	 * Sets up the section
	 */
	bool setup(CPetControl *petControl) override;

	/**
	 * Sets up the section
	 */
	bool reset() override;

	/**
	 * Draw the section
	 */
	void draw(CScreenManager *screenManager) override;

	/**
	 * Get the bounds for the section
	 */
	Rect getBounds() const override;

	/**
	 * Called when a general change occurs
	 */
	void changed(int changeType) override;

	/**
	 * Called when a new room is entered
	 */
	void enterRoom(CRoomItem *room) override;

	/**
	 * Following are handlers for the various messages that the PET can
	 * pass onto the currently active section/area
	 */
	bool MouseButtonDownMsg(CMouseButtonDownMsg *msg) override;
	bool MouseDragStartMsg(CMouseDragStartMsg *msg) override;
	bool MouseButtonUpMsg(CMouseButtonUpMsg *msg) override;
	bool MouseDoubleClickMsg(CMouseDoubleClickMsg *msg) override;
	bool VirtualKeyCharMsg(CVirtualKeyCharMsg *msg) override;
	bool MouseWheelMsg(CMouseWheelMsg *msg) override;

	/**
	 * Returns item a drag-drop operation has dropped on, if any
	 */
	CGameObject *dragEnd(const Point &pt) const override;

	/**
	 * Returns true if the object is in a valid state
	 */
	bool isValid(CPetControl *petControl) override;

	/**
	 * Load the data for the class from file
	 */
	void load(SimpleFile *file, int param) override;

	/**
	 * Called after a game has been loaded
	 */
	void postLoad() override;

	/**
	 * Save the data for the class to file
	 */
	void save(SimpleFile *file, int indent) override;

	/**
	* Called when a section is switched to
	*/
	void enter(PetArea oldArea) override;

	/**
	* Called when a section is being left, to switch to another area
	*/
	void leave() override;

	/**
	 * Get a reference to the tooltip text associated with the section
	 */
	CTextControl *getText() override { return &_text; }

	/**
	 * Special retrieval of glyph background image
	 */
	CGameObject *getBackground(int index) const override;

	/**
	 *
	 */
	void change(CGameObject *item);

	/**
	 * Called when an item has been removed from the PET
	 */
	void itemRemoved(CGameObject *item);

	/**
	 * Called when the items under the PET have changed
	 */
	void itemsChanged();

	/**
	 * Called when the inventory can't be shown after adding an item
	 */
	void highlightItem(CGameObject *item);

	/**
	 * Gets the object, if any, containing the transformation animation played 
	 * when pieces of Titania are added to the inventory for the first time.
	 */
	CGameObject *getTransformAnimation(int index);

	/**
	 * Play the animated movie for an object
	 */
	void playMovie(CGameObject *movie, bool repeat = true);
};

} // End of namespace Titanic

#endif /* TITANIC_PET_INVENTORY_H */
