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

#ifndef TITANIC_GAME_OBJECT_H
#define TITANIC_GAME_OBJECT_H

#include "titanic/support/mouse_cursor.h"
#include "titanic/support/proximity.h"
#include "titanic/support/rect.h"
#include "titanic/core/movie_clip.h"
#include "titanic/core/named_item.h"
#include "titanic/pet_control/pet_section.h"

namespace Titanic {

class CVideoSurface;
class CMouseDragStartMsg;
class OSMovie;

class CGameObject : public CNamedItem {
	friend class OSMovie;
	DECLARE_MESSAGE_MAP
public:
	static void *_v1;
private:
	/**
	 * Load a visual resource for the object
	 */
	void loadResource(const CString &name);

	/**
	 * Loads a movie
	 */
	void loadMovie(const CString &name, bool pendingFlag = true);

	/**
	 * Loads an image
	 */
	void loadImage(const CString &name, bool pendingFlag = true);

	void processClipList2();
protected:
	Rect _bounds;
	double _field34;
	double _field38;
	double _field3C;
	int _field40;
	int _field44;
	int _field48;
	int _field4C;
	int _field50;
	int _field54;
	int _field58;
	bool _visible;
	CMovieClipList _clipList1;
	int _initialFrame;
	CMovieClipList _clipList2;
	int _frameNumber;
	int _field90;
	int _field94;
	int _field98;
	int _field9C;
	Common::Point _savedPos;
	CVideoSurface *_surface;
	CString _resource;
	int _fieldB8;
protected:
	/**
	 * Saves the current position the object is located at
	 */
	void savePosition();

	/**
	 * Resets the object back to the previously saved starting position
	 */
	void resetPosition();

	/**
	 * Check for starting to drag the object
	 */
	bool checkStartDragging(CMouseDragStartMsg *msg);

	/**
	 * Marks the area in the passed rect as dirty, and requiring re-rendering
	 */
	void makeDirty(const Rect &r);

	/**
	 * Marks the area occupied by the object as dirty, requiring re-rendering
	 */
	void makeDirty();

	/**
	 * Sets a new area in the PET
	 */
	void setPetArea(PetArea newArea) const;

	/**
	 * Goto a new view
	 */
	void gotoView(const CString &viewName, const CString &clipName);

	/**
	 * Parses a view into it's components of room, node, and view,
	 * and locates the designated view
	 */
	CViewItem * parseView(const CString &viewString);

	bool soundFn1(int val);
	void soundFn2(int val, int val2);
	void setVisible(bool val);
	void petFn2(int val);
	void petFn3(CTreeItem *item);

	/**
	 * Plays a sound
	 */
	bool playSound(const CString &name, int val2, int val3, int val4);

	/**
	 * Plays a sound
	 */
	bool playSound(const CString &name, CProximity &prox);
public:
	int _field60;
	CursorId _cursorId;
public:
	CLASSDEF
	CGameObject();

	/**
	 * Save the data for the class to file
	 */
	virtual void save(SimpleFile *file, int indent) const;

	/**
	 * Load the data for the class from file
	 */
	virtual void load(SimpleFile *file);

	/**
	 * Allows the item to draw itself
	 */
	virtual void draw(CScreenManager *screenManager);

	/**
	 * Allows the item to draw itself
	 */
	virtual void draw(CScreenManager *screenManager, const Common::Point &destPos);

	/**
	 * Stops any movie currently playing for the object
	 */
	void stopMovie();

	/**
	 * Checks the passed point is validly in the object,
	 * with extra checking of object flags status
	 */
	bool checkPoint(const Point &pt, bool ignore40 = false, bool visibleOnly = false);

	void fn1(int val1, int val2, int val3);

	/**
	 * Change the object's status
	 */
	void changeStatus(int newStatus);

	/**
	 * Set the position of the object
	 */
	void setPosition(const Common::Point &newPos);

	/**
	 * Returns true if the object has a currently active movie
	 */
	bool hasActiveMovie() const;

	int getMovie19() const;
	int getSurface45() const;

	/**
	 * Loads a frame
	 */
	void loadFrame(int frameNumber);
};

} // End of namespace Titanic

#endif /* TITANIC_GAME_OBJECT_H */
