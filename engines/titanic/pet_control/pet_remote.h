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

#ifndef TITANIC_PET_REMOTE_H
#define TITANIC_PET_REMOTE_H

#include "common/array.h"
#include "titanic/pet_control/pet_section.h"
#include "titanic/pet_control/pet_glyphs.h"
#include "titanic/pet_control/pet_remote_glyphs.h"
#include "titanic/gfx/text_control.h"

namespace Titanic {

class CPetRemote : public CPetSection {
private:
	CPetRemoteGlyphs _items;
	CPetGfxElement _onOff;
	CPetGfxElement _up;
	CPetGfxElement _down;
	CPetGfxElement _left;
	CPetGfxElement _right;
	CPetGfxElement _top;
	CPetGfxElement _bottom;
	CPetGfxElement _action;
	CPetGfxElement _send;
	CPetGfxElement _receive;
	CPetGfxElement _call;
	CTextControl _text;
private:
	/**
	 * Setup the control
	 */
	bool setupControl(CPetControl *petControl);

	/**
	 * Get the current room
	 */
	CRoomItem *getRoom() const;

	/**
	 * Return a highlight index
	 */
	int getHighlightIndex(RemoteGlyph val);

	/**
	 * Return the index of a room name in the master room names list
	 */
	int roomIndexOf(const CString &name);

	/**
	 * Return a list of remote action glyph indexes for a given room
	 */
	bool getRemoteData(int roomIndex, Common::Array<uint> &indexes);

	/**
	 * Clear the list of rooms glyphs
	 */
	void clearGlyphs() { _items.clear(); }

	/**
	 * Load the room glyphs
	 */
	bool loadGlyphs(const Common::Array<uint> &indexes);

	/**
	 * Load a single room glyph
	 */
	bool loadGlyph(int glyphIndex);
public:
	CPetRemote();

	/**
	 * Sets up the section
	 */
	virtual bool setup(CPetControl *petControl);

	/**
	 * Reset the section
	 */
	virtual bool reset();

	/**
	 * Draw the section
	 */
	virtual void draw(CScreenManager *screenManager);

	/**
	* Following are handlers for the various messages that the PET can
	* pass onto the currently active section/area
	*/
	virtual bool MouseButtonDownMsg(CMouseButtonDownMsg *msg);
	virtual bool MouseButtonUpMsg(CMouseButtonUpMsg *msg);
	virtual bool MouseDoubleClickMsg(CMouseDoubleClickMsg *msg);
	virtual bool VirtualKeyCharMsg(CVirtualKeyCharMsg *msg);
	virtual bool MouseWheelMsg(CMouseWheelMsg *msg);

	/**
	 * Returns true if the object is in a valid state
	 */
	virtual bool isValid(CPetControl *petControl);

	/**
	 * Called after a game has been loaded
	 */
	virtual void postLoad();

	/**
	 * Called when a section is switched to
	 */
	virtual void enter(PetArea oldArea);

	/**
	 * Called when a new room is entered
	 */
	virtual void enterRoom(CRoomItem *room);

	/**
	 * Get a reference to the tooltip text associated with the section
	 */
	virtual CTextControl *getText();

	/**
	 * Get an element from the section by a designated Id
	 */
	virtual CPetGfxElement *getElement(uint id);

	/**
	 * Highlights a glyph item in the section
	 */
	virtual void highlight(int id);

	/**
	 * Generates a PET message
	 */
	void generateMessage(RemoteMessage msgNum, const CString &name, int num);
};

} // End of namespace Titanic

#endif /* TITANIC_PET_REMOTE_H */
