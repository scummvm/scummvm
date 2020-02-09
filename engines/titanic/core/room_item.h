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

#ifndef TITANIC_ROOM_ITEM_H
#define TITANIC_ROOM_ITEM_H

#include "titanic/support/rect.h"
#include "titanic/core/list.h"
#include "titanic/support/movie_clip.h"
#include "titanic/core/named_item.h"
#include "titanic/core/resource_key.h"

namespace Titanic {

class CRoomItem : public CNamedItem {
	DECLARE_MESSAGE_MAP;
private:
	/**
	 * Handles post-load processing
	 */
	void postLoad();
public:
	Rect _roomRect;
	CMovieClipList _clipList;
	int _roomNumber;
	CResourceKey _transitionMovieKey;
	CResourceKey _exitMovieKey;
	double _roomDimensionX, _roomDimensionY;
public:
	CLASSDEF;
	CRoomItem();

	/**
	 * Save the data for the class to file
	 */
	void save(SimpleFile *file, int indent) override;

	/**
	 * Load the data for the class from file
	 */
	void load(SimpleFile *file) override;

	/**
	 * Return a movie clip for the room by name
	 */
	CMovieClip *findClip(const CString &name) { return _clipList.findByName(name); }

	/**
	 * Calculates the positioning of a node within the overall room
	 */
	void calcNodePosition(const Point &nodePos, double &xVal, double &yVal) const;

	/**
	 * Get the TrueTalk script Id associated with the room
	 */
	int getScriptId() const;

	CResourceKey getTransitionMovieKey();

	CResourceKey getExitMovieKey();
};

} // End of namespace Titanic

#endif /* TITANIC_ROOM_ITEM_H */
