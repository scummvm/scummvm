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

#ifndef TITANIC_PET_INVENTORY_GLYPHS_H
#define TITANIC_PET_INVENTORY_GLYPHS_H

#include "titanic/carry/carry.h"
#include "titanic/pet_control/pet_glyphs.h"
#include "titanic/support/screen_manager.h"

namespace Titanic {

class CPetInventoryGlyph : public CPetGlyph {
private:
	/**
	 * Populate the details for an item
	 */
	int populateItem(CGameObject *item, bool isLoading);

	/**
	 * For items which can have multiple different states, such as the
	 * beer glass or Maitre D arms, returns the correct item index to use
	 * for getting the inventory item glyph and description
	 */
	int getItemIndex(CGameObject *item, bool isLoading);

	/**
	 * Start a repeated animation for the item
	 */
	void startRepeatedMovie();

	/**
	 * Start a singular (non-repeating) animation for the item,
	 * such as an item's transformation into a piece of Titania 
	 */
	void startSingularMovie();

	/**
	 * Stop any previously started foreground or background movie
	 */
	void stopMovie();

	/**
	 * Reposition the inventory item
	 */
	void reposition(const Point &pt);
public:
	CGameObject *_item;
	bool _active;
	CGameObject *_repeated;
	CGameObject *_singular;
public:
	CPetInventoryGlyph() : _item(nullptr), _active(true),
		_repeated(nullptr), _singular(nullptr) {}
	CPetInventoryGlyph(CCarry *item, bool active) : _item(item),
		_active(active), _repeated(nullptr), _singular(nullptr) {}

	/**
	 * Called when the PET area is entered
	 */
	void enter() override;

	/**
	 * Called when the PET area is left
	 */
	void leave() override;

	/**
	 * Draw the glyph at a specified position
	 */
	void drawAt(CScreenManager *screenManager, const Point &pt, bool isHighlighted) override;

	/**
	 * Unhighlight any currently highlighted element
	 */
	void unhighlightCurrent() override;

	/**
	 * Highlight any currently highlighted element
	 */
	void highlightCurrent(const Point &pt) override;

	/**
	 * Glyph has been shifted to be first visible one
	 */
	void glyphFocused(const Point &topLeft, bool flag) override;

	/**
	 * Called when a glyph drag starts
	 */
	bool dragGlyph(const Point &topLeft, CMouseDragStartMsg *msg) override;

	/**
	 * Returns the tooltip text for when the glyph is selected
	 */
	void getTooltip(CTextControl *text) override;

	/**
	 * Return whether the glyph is currently valid
	 */
	bool isValid() const override { return _item && _repeated; }

	/**
	 * Returns the object associated with the glyph
	 */
	CGameObject *getObjectAt() override { return _item; }

	/**
	 * Does a processing action on the glyph
	 */
	bool doAction(CGlyphAction *action) override;

	/**
	 * Set the inventory item
	 */
	void setItem(CGameObject *item, bool isLoading);
};

class CInventoryGlyphAction : public CGlyphAction {
public:
	CGameObject *_item;
public:
	CInventoryGlyphAction(CGameObject *item, GlyphActionMode mode) :
		CGlyphAction(mode), _item(item) {}
};

class CPetInventoryGlyphs : public CPetGlyphs {
	friend class CPetInventoryGlyph;
private:
	CGameObject *getBackground(int index);
public:
	/**
	 * Do an action on the glyphs
	 */
	bool doAction(CInventoryGlyphAction *item);
};

} // End of namespace Titanic

#endif /* TITANIC_PET_INVENTORY_GLYPHS_H */
