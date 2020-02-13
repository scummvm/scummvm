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

#ifndef TITANIC_PET_SAVE_H
#define TITANIC_PET_SAVE_H

#include "titanic/pet_control/pet_load_save.h"

namespace Titanic {

class CPetSave : public CPetLoadSave {
public:
	/**
	 * Reset the glyph
	 */
	bool reset() override;

	/**
	 * Handles mouse button up messages
	 */
	bool MouseButtonUpMsg(const Point &pt) override;

	/**
	 * Handles keypresses when the glyph is focused
	 */
	bool KeyCharMsg(int key) override;

	/**
	 * Unhighlight any currently highlighted element
	 */
	void unhighlightCurrent() override { unhighlightSave(_savegameSlotNum); }

	/**
	 * Highlight any currently highlighted element
	 */
	void highlightCurrent(const Point &pt) override;

	/**
	 * Returns the tooltip text for when the glyph is selected
	 */
	void getTooltip(CTextControl *text) override;

	/**
	 * Called on a highlighted item when PET area is entered
	 */
	bool enterHighlighted() override {
		highlightSave(_savegameSlotNum);
		return true;
	}

	/**
	 * Called on a highlighted item when PET area is left
	 */
	void leaveHighlighted() override { unhighlightSave(_savegameSlotNum); }

	/**
	 * Highlights a save slot
	 */
	void highlightSave(int index) override;

	/**
	 * Unhighlight a save slot
	 */
	void unhighlightSave(int index) override;

	/**
	 * Executes the loading or saving
	 */
	void execute() override;
};

} // End of namespace Titanic

#endif /* TITANIC_PET_SAVE_H */
