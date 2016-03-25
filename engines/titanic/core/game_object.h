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

#include "titanic/mouse_cursor.h"
#include "titanic/rect.h"
#include "titanic/core/movie_clip.h"
#include "titanic/core/named_item.h"

namespace Titanic {

class CVideoSurface;

class CGameObject : public CNamedItem {
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

	/**
	 * Loads a frame
	 */
	void loadFrame(int frameNumber);

	void processClipList2();

	/**
	 * Marks the area in the passed rect as dirty, and requiring re-rendering
	 */
	void makeDirty(const Rect &r);

	/**
	 * Marks the area occupied by the object as dirty, requiring re-rendering
	 */
	void makeDirty();
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
	int _field5C;
	CMovieClipList _clipList1;
	int _field78;
	CMovieClipList _clipList2;
	int _frameNumber;
	int _field90;
	int _field94;
	int _field98;
	int _field9C;
	int _fieldA0;
	int _fieldA4;
	CVideoSurface *_surface;
	CString _resource;
	int _fieldB8;
protected:
	bool soundFn1(int val);
	void soundFn2(int val, int val2);
	void set5C(int val);
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
	 * Stops any movie currently playing for the object
	 */
	void stopMovie();

	bool checkPoint(const Point &pt, int v0, int v1);
};

} // End of namespace Titanic

#endif /* TITANIC_GAME_OBJECT_H */
