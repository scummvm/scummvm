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

#ifndef TITANIC_PET_LOAD_SAVE_H
#define TITANIC_PET_LOAD_SAVE_H

#include "titanic/pet_control/pet_glyphs.h"
#include "titanic/pet_control/pet_text.h"

namespace Titanic {

#define SAVEGAME_SLOTS_COUNT 5

class CPetLoadSave : public CPetGlyph {
private:
	/**
	 * Get the rect area for a given savegame name will be displayed in
	 */
	Rect getSlotBounds(int index);

	/**
	 * Called when savegame slot highlight changes or the view is reset
	 */
	void highlightChange();

	/**
	 * Check for whether a slot is under the passed point
	 */
	bool checkSlotsHighlight(const Point &pt);

	/**
	 * Checks if a point is within a given saveame slot
	 */
	bool isSlotHighlighted(int index, const Point &pt);
protected:
	CPetText _slotNames[SAVEGAME_SLOTS_COUNT];
	bool _slotInUse[SAVEGAME_SLOTS_COUNT];
	CPetGfxElement _btnLoadSave;
	CPetGfxElement _gutter;
	static int _savegameSlotNum;
protected:
	/**
	 * Reset the slot names list
	 */
	void resetSlots();

	/**
	 * Highlight one of the slots
	 */
	void highlightSlot(int index);
public:
	/**
	 * Setup the glyph
	 */
	virtual bool setup(CPetControl *petControl, CPetGlyphs *owner);

	/**
	 * Reset the glyph
	 */
	virtual bool reset();

	/**
	 * Handles any secondary drawing of the glyph
	 */
	virtual void draw2(CScreenManager *screenManager);

	/**
	 * Called for mouse button down messages
	 */
	virtual bool MouseButtonDownMsg(const Point &pt);

	/**
	 * Handles keypresses when the glyph is focused
	 */
	virtual bool KeyCharMsg(int key);

	/**
	 * Resets highlighting on the save slots
	 */
	virtual void resetSaves() { resetSlots(); }

	/**
	 * Highlights a save slot
	 */
	virtual void highlightSave(int index) = 0;

	/**
	 * Unhighlight a save slot
	 */
	virtual void unhighlightSave(int index) = 0;

	/**
	 * Executes the loading or saving
	 */
	virtual void execute() = 0;
};

} // End of namespace Titanic

#endif
