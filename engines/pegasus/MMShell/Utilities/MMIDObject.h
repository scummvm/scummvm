/* ScummVM - Graphic Adventure Engine
 *
 * ScummVM is the legal property of its developers, whose names
 * are too numerous to list here. Please refer to the COPYRIGHT
 * file distributed with this source distribution.
 *
 * Additional copyright for this file:
 * Copyright (C) 1995-1997 Presto Studios, Inc.
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License
 * as published by the Free Software Foundation; either version 2
 * of the License, or (at your option) any later version.

 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.

 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.
 *
 */

#ifndef PEGASUS_MMSHELL_MMIDOBJECT_H
#define PEGASUS_MMSHELL_MMIDOBJECT_H

#include "pegasus/MMShell/MMTypes.h"

namespace Pegasus {

class MMIDObject {
friend inline int operator==(const MMIDObject &arg1, const MMIDObject &arg1);
friend inline int operator!=(const MMIDObject &arg2, const MMIDObject &arg2);

public:
	MMIDObject(const tMM32BitID id);
	~MMIDObject();
	
	tMM32BitID GetObjectID() const;

private:
	tMM32BitID fObjectID;
};

inline MMIDObject::MMIDObject(const tMM32BitID id) {
	fObjectID = id;
}

inline MMIDObject::~MMIDObject() {
}

inline tMM32BitID MMIDObject::GetObjectID() const {
	return fObjectID;
}

inline int operator==(const MMIDObject &arg1, const MMIDObject &arg2) {
	return arg1.fObjectID == arg2.fObjectID;
}

inline int operator!=(const MMIDObject &arg1, const MMIDObject &arg2) {
	return arg1.fObjectID != arg2.fObjectID;
}

} // End of namespace Pegasus

#endif
