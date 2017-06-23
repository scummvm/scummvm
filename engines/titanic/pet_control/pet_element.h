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

#ifndef TITANIC_PET_ELEMENT_H
#define TITANIC_PET_ELEMENT_H

#include "titanic/support/simple_file.h"
#include "titanic/support/string.h"
#include "titanic/core/link_item.h"
#include "titanic/messages/mouse_messages.h"

namespace Titanic {

enum PetElementMode { MODE_UNSELECTED = 0, MODE_SELECTED = 1, MODE_FOCUSED = 2 };

class CGameObject;
class CPetControl;

class CPetElement {
protected:
	Rect _bounds;
	PetElementMode _mode;
public:
	CPetElement();
	virtual ~CPetElement() {}

	/**
	 * Sets up the element
	 */
	virtual void setup(PetElementMode mode, const CString &name,
		CPetControl *petControl) {}

	/**
	 * Reset the element
	 */
	virtual void reset(const CString &name, CPetControl *petControl, PetElementMode mode) {}

	/**
	 * Draw the item
	 */
	virtual void draw(CScreenManager *screenManager) {}

	/**
	 * Draw the item
	 */
	virtual void draw(CScreenManager *screenManager, const Point &destPos) {}

	/**
	 * Get the bounds for the element
	 */
	virtual Rect getBounds() const;

	/**
	 * Handles processing mouse button down messages
	 */
	virtual bool MouseButtonDownMsg(const Point &pt);

	/**
	 * Handles processing mouse button up messages
	 */
	virtual bool MouseButtonUpMsg(const Point &pt);

	/**
	 * Handles processing mouse button double click messages
	 */
	virtual bool MouseDoubleClickMsg(const Point &pt) const;

	/**
	 * Handles processing mouse move messages
	 */
	virtual bool MouseMoveMsg(const Point &pt);

	/**
	 * Returns whether the passed point falls inside the item
	 */
	virtual bool contains2(const Point &pt) const;

	/**
	 * Plays back a range of frames in the loaded video file for the element
	 */
	virtual void playMovie(uint startFrame, uint endFrame) const;

	/**
	 * Change the status of the associated object
	 */
	virtual void changeStatus(int newStatus) const;

	/**
	 * Returns true if the object associated with the item has an active movie
	 */
	virtual bool hasActiveMovie() const;

	/**
	 * Loads a frame
	 */
	virtual void loadFrame(int frameNumber);

	/**
	 * Get the current frame
	 */
	virtual int getMovieFrame() const;

	/**
	 * Get the game object associated with this item
	 */
	virtual CGameObject *getObject() const { return nullptr; }

	virtual void changeMode(PetElementMode newMode) { _mode = newMode; }

	void setMode(PetElementMode mode);

	/**
	 * Set whether the element is selected
	 */
	void setSelected(bool flag);

	/**
	 * Set the bounds for the element
	 */
	void setBounds(const Rect &r) { _bounds = r; }

	/**
	 * Translate the position of the element
	 */
	void translate(int deltaX, int deltaY) { _bounds.translate(deltaX, deltaY); }

	/**
	 * Translate the position of the element
	 */
	void translate(const Point &delta) { _bounds.translate(delta.x, delta.y); }
};

} // End of namespace Titanic

#endif /* TITANIC_PET_ELEMENT_H */
