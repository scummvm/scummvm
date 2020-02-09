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

#ifndef TITANIC_RESTAURANT_CYLINDER_HOLDER_H
#define TITANIC_RESTAURANT_CYLINDER_HOLDER_H

#include "titanic/core/drop_target.h"

namespace Titanic {

class CRestaurantCylinderHolder : public CDropTarget {
	DECLARE_MESSAGE_MAP;
	bool EjectCylinderMsg(CEjectCylinderMsg *msg);
	bool EnterViewMsg(CEnterViewMsg *msg);
	bool MovieEndMsg(CMovieEndMsg *msg);
	bool QueryCylinderHolderMsg(CQueryCylinderHolderMsg *msg);
	bool QueryCylinderNameMsg(CQueryCylinderNameMsg *msg);
	bool MouseDragStartMsg(CMouseDragStartMsg *msg);
	bool DropObjectMsg(CDropObjectMsg *msg);
private:
	bool _isOpen;
	int _field11C;
	CString _target;
	int _field12C;
	int _field130;
	CString _ejectSoundName;
	CursorId _defaultCursorId;
public:
	CLASSDEF;
	CRestaurantCylinderHolder();

	/**
	 * Save the data for the class to file
	 */
	void save(SimpleFile *file, int indent) override;

	/**
	 * Load the data for the class from file
	 */
	void load(SimpleFile *file) override;
};

} // End of namespace Titanic

#endif /* TITANIC_RESTAURANT_CYLINDER_HOLDER_H */
