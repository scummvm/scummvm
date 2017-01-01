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

#ifndef TITANIC_LINK_ITEM_H
#define TITANIC_LINK_ITEM_H

#include "titanic/support/mouse_cursor.h"
#include "titanic/core/named_item.h"
#include "titanic/support/movie_clip.h"

namespace Titanic {

class CViewItem;
class CNodeItem;
class CRoomItem;

class CLinkItem : public CNamedItem {
	DECLARE_MESSAGE_MAP;
private:
	/**
	 * Returns a new name for the link item, based on the
	 * current values for it's destination
	 */
	CString formName();
protected:
	int _roomNumber;
	int _nodeNumber;
	int _viewNumber;
	int _linkMode;
public:
	Rect _bounds;
	CursorId _cursorId;
public:
	CLASSDEF;
	CLinkItem();

	/**
	 * Save the data for the class to file
	 */
	virtual void save(SimpleFile *file, int indent);

	/**
	 * Load the data for the class from file
	 */
	virtual void load(SimpleFile *file);

	/**
	 * Returns true if the given item connects to another specified view
	 */
	virtual bool connectsTo(CViewItem *destView) const;

	/**
	 * Set the destination for the link item
	 */
	virtual void setDestination(int roomNumber, int nodeNumber,
		int viewNumber, int linkMode);

	/**
	 * Get the destination view for the link item
	 */
	virtual CViewItem *getDestView() const;

	/**
	 * Get the destination node for the link item
	 */
	virtual CNodeItem *getDestNode() const;

	/**
	 * Get the destination view for the link item
	 */
	virtual CRoomItem *getDestRoom() const;

	/**
	 * Get the movie clip, if any, that's used when the link is used
	 */
	CMovieClip *getClip() const;
};

} // End of namespace Titanic

#endif /* TITANIC_LINK_ITEM_H */
