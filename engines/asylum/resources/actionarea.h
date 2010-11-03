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

 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.	 See the
 * GNU General Public License for more details.

 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.
 *
 * $URL$
 * $Id$
 *
 */

#ifndef ASYLUM_ACTIONAREA_H
#define ASYLUM_ACTIONAREA_H

#include "asylum/shared.h"

#include "common/array.h"

namespace Asylum {

class ActionArea {
public:
	ActionArea();
	virtual ~ActionArea();

	char  name[52];
	int32 id;
	int32 field01;
	int32 field02;
	int32 field_40;
	int32 field_44;
	int32 flags;
	int32 scriptIndex;
	int32 actionListIdx2;
	int32 actionType; // aka flags2: 0-none, 1-findwhat, 2-talk, 3-findwhat??, 4-grab
	int32 flagNums[10];
	int32 field_7C;
	int32 polyIdx;
	int32 field_84;
	int32 field_88;
	ResourceId soundResourceId;
	int32 field_90;
	int32 paletteValue;
	int32 array[5];
	int32 volume;

}; // end of class ActionArea

} // end of namespace Asylum

#endif /* ASYLUM_ACTIONAREA_H */
