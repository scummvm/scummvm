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
	virtual bool setup(CPetControl *petControl);

	/**
	 * Sets up the section
	 */
	virtual bool reset();

	/**
	 * Draw the section
	 */
	virtual void draw(CScreenManager *screenManager);

	/**
	 * Get the bounds for the section
	 */
	virtual Rect getBounds() const;

	/**
	 * Called when a general change occurs
	 */
	virtual void changed(int changeType);

	/**
	 * Called when a new room is entered
	 */
	virtual void enterRoom(CRoomItem *room);

	/**
	 * Following are handlers for the various messages that the PET can
	 * pass onto the currently active section/area
	 */
	virtual bool MouseButtonDownMsg(CMouseButtonDownMsg *msg);
	virtual bool MouseDragStartMsg(CMouseDragStartMsg *msg);
	virtual bool MouseButtonUpMsg(CMouseButtonUpMsg *msg);
	virtual bool MouseDoubleClickMsg(CMouseDoubleClickMsg *msg);
	virtual bool VirtualKeyCharMsg(CVirtualKeyCharMsg *msg);
	virtual bool MouseWheelMsg(CMouseWheelMsg *msg);

	/**
	 * Returns item a drag-drop operation has dropped on, if any
	 */
	virtual CGameObject *dragEnd(const Point &pt) const;

	/**
	 * Returns true if the object is in a valid state
	 */
	virtual bool isValid(CPetControl *petControl);

	/**
	 * Load the data for the class from file
	 */
	virtual void load(SimpleFile *file, int param);

	/**
	 * Called after a game has been loaded
	 */
	virtual void postLoad();

	/**
	 * Save the data for the class to file
	 */
	virtual void save(SimpleFile *file, int indent);

	/**
	* Called when a section is switched to
	*/
	virtual void enter(PetArea oldArea);

	/**
	* Called when a section is being left, to switch to another area
	*/
	virtual void leave();

	/**
	 * Get a reference to the tooltip text associated with the section
	 */
	virtual CTextControl *getText() { return &_text; }

	/**
	 * Special retrieval of glyph background image
	 */
	virtual CGameObject *getBackground(int index) const;

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
