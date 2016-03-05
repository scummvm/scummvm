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

#ifndef TITANIC_PET_CONTROL_SUB4_H
#define TITANIC_PET_CONTROL_SUB4_H

#include "titanic/simple_file.h"
#include "titanic/pet_control/pet_control_sub_base.h"
#include "titanic/pet_control/pet_control_sub10.h"
#include "titanic/pet_control/pet_control_sub12.h"

namespace Titanic {

class CPetControlSub4 : public CPetControlSubBase {
private:
	CPetControlSub12 _sub12;
	CPetControlSub10 _sub10;
	int _valArray1[46];
	int _valArray2[46];
	int _field28C;
	int _field290;
	int _field294;
	int _field298;
public:
	CPetControlSub4();

	/**
	 * Save the data for the class to file
	 */
	virtual void save(SimpleFile *file, int indent) const;

	/**
	 * Load the data for the class from file
	 */
	virtual void load(SimpleFile *file);
};

} // End of namespace Titanic

#endif /* TITANIC_PET_CONTROL_SUB4_H */
