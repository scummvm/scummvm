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

#ifndef TITANIC_PET_ROOMS_GLYPHS_H
#define TITANIC_PET_ROOMS_GLYPHS_H

#include "titanic/pet_control/pet_glyphs.h"
#include "titanic/game_location.h"

namespace Titanic {

class SimpleFile;

enum RoomGlyphMode {
	RGM_UNASSIGNED = 0, RGM_ASSIGNED_ROOM = 1, RGM_PREV_ASSIGNED_ROOM = 2
};

class CPetRoomsGlyph : public CPetGlyph {
private:
	uint _roomFlags;
	uint _mailFlag;
	RoomGlyphMode _mode;
	CGameObject *_chevLeftOnDim;
	CGameObject *_chevLeftOffDim;
	CGameObject *_chevLeftOnLit;
	CGameObject *_chevLeftOffLit;
	CGameObject *_chevRightOnDim;
	CGameObject *_chevRightOffDim;
	CGameObject *_chevRightOnLit;
	CGameObject *_chevRightOffLit;
private:
	/**
	 * Find the selected button under the given point, based on the buttons
	 * starting at a designated top/left position
	 */
	int getSelection(const Point &topLeft, const Point &pt);

	/**
	 * Draws the objects
	 */
	void drawObjects(uint flags, const Point &pt, CScreenManager *screenManager);
public:
	CPetRoomsGlyph();
	CPetRoomsGlyph(uint flags);

	/**
	 * Setup the glyph
	 */
	bool setup(CPetControl *petControl, CPetGlyphs *owner) override;

	/**
	 * Draw the glyph at a specified position
	 */
	void drawAt(CScreenManager *screenManager, const Point &pt, bool isHighlighted) override;

	/**
	 * Handles any secondary drawing of the glyph
	 */
	void draw2(CScreenManager *screenManager) override {}

	/**
	 * Selects a glyph
	 */
	void selectGlyph(const Point &topLeft, const Point &pt) override;

	/**
	 * Called when a glyph drag starts
	 */
	bool dragGlyph(const Point &topLeft, CMouseDragStartMsg *msg) override;

	/**
	 * Returns the tooltip text for when the glyph is selected
	 */
	void getTooltip(CTextControl *text) override;

	/**
	 * Saves the data for the glyph
	 */
	void saveGlyph(SimpleFile *file, int indent) override;

	bool proc33(CPetGlyph *glyph) override;

	/**
	 * Loads flags for the glyph
	 */
	virtual void loadFlags(SimpleFile *file, int val);

	/**
	 * Set the room flags for the glyph
	 */
	void setRoomFlags(uint flags) { _roomFlags = flags; }

	/**
	 * Get the room flags for the glyph
	 */
	uint getRoomFlags() const { return _roomFlags; }

	/**
	 * Set mail status flag
	 */
	void setFlag(uint val) { _mailFlag = val; }

	/**
	 * Sets the mode of the glyph
	 */
	void setMode(RoomGlyphMode mode) { _mode = mode; }

	/**
	 * Change the current class
	 */
	void changeClass(PassengerClass newClassNum);

	/**
	 * Returns true if the room is either currently or previously assigned
	 */
	bool isAssigned() const { return _mode != RGM_UNASSIGNED; }

	/**
	 * Returns true if the room is the one currently assigned to the player
	 */
	bool isCurrentlyAssigned() const { return _mode == RGM_ASSIGNED_ROOM; }

	/**
	 * Returns true if the room was previously assigned to the player
	 */
	bool isPreviouslyAssigned() const { return _mode == RGM_PREV_ASSIGNED_ROOM; }
};

class CPetRoomsGlyphs : public CPetGlyphs {
private:
public:
	/**
	 * Save the list
	 */
	void saveGlyphs(SimpleFile *file, int indent);

	/**
	 * Returns the glyph for hte player's assigned room
	 */
	CPetRoomsGlyph *findAssignedRoom() const;

	/**
	 * Finds a glyph in the list by it's room flags
	 */
	CPetRoomsGlyph *findGlyphByFlags(uint flags) const;

	/**
	 * Returns true if there's a glyph in the list with a given room flags
	 */
	bool hasFlags(uint flags) const { return findGlyphByFlags(flags) != nullptr; }
};

} // End of namespace Titanic

#endif /* TITANIC_PET_ROOMS_GLYPHS_H */
