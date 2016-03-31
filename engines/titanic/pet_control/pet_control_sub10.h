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

#ifndef TITANIC_PET_CONTROL_SUB10_H
#define TITANIC_PET_CONTROL_SUB10_H

#include "titanic/core/list.h"
#include "titanic/pet_control/pet_control_sub10.h"
#include "titanic/pet_control/pet_control_list_item.h"

namespace Titanic {

class CPetControlSub10 : public List<CPetControlListItem> {
protected:
	int _field10;
	int _field14;
	int _field18;
	int _field1C;
	int _field20;
	int _field24;
	CPetGraphic _val1;
	CPetGraphic _val2;
	CPetGraphic _val3;
public:
	CPetControlSub10();

	virtual void proc8();
	virtual void proc9();
	virtual void proc10();
	virtual void proc11();
};

} // End of namespace Titanic

#endif /* TITANIC_PET_CONTROL_SUB10_H */
