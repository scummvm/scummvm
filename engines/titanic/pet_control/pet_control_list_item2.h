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

#ifndef TITANIC_PET_CONTROL_LIST_ITEM2_H
#define TITANIC_PET_CONTROL_LIST_ITEM2_H

#include "titanic/pet_control/pet_control_list_item.h"

namespace Titanic {

class CPetControlListItem2  : public CPetControlListItem {
protected:
	int _field34;
	int _field38;
	int _field3C;
	int _field40;
	int _field44;
	int _field48;
	int _field4C;
	int _field50;
	int _field54;
	int _field58;
	int _field5C;
public:
	CPetControlListItem2();

	void setField34(int val);
};

} // End of namespace Titanic

#endif /* TITANIC_PET_CONTROL_LIST_ITEM2_H */
