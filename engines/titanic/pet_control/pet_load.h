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

#ifndef TITANIC_PET_LOAD_H
#define TITANIC_PET_LOAD_H

#include "titanic/pet_control/pet_load_save.h"

namespace Titanic {

class CPetLoad : public CPetLoadSave {
public:
	/**
	 * Reset the glyph
	 */
	virtual bool reset();

	/**
	 * Handles mouse button up messages
	 */
	virtual bool MouseButtonUpMsg(const Point &pt);

	/**
	 * Highlight any currently highlighted element
	 */
	virtual void highlightCurrent(const Point &pt) { resetSlots(); }

	/**
	 * Returns the tooltip text for when the glyph is selected
	 */
	virtual void getTooltip(CTextControl *text);

	/**
	 * Highlights a save slot
	 */
	virtual void highlightSave(int index) {}

	/**
	 * Unhighlight a save slot
	 */
	virtual void unhighlightSave(int index) {}

	/**
	 * Executes the loading or saving
	 */
	virtual void execute();
};

} // End of namespace Titanic

#endif /* TITANIC_PET_LOAD_H */
