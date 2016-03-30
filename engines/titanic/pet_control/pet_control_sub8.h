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

#ifndef TITANIC_PET_CONTROL_SUB8_H
#define TITANIC_PET_CONTROL_SUB8_H

#include "titanic/pet_control/pet_section.h"
#include "titanic/pet_control/pet_val.h"

namespace Titanic {

class CPetControlSub8 : public CPetSection {
private:
	static int _indexes[6];

	CPetVal _valArray1[6];
	CPetVal _valArray2[6];
	CPetVal _val1;
	CPetVal _val2;
	CPetVal _val3;
	CPetVal _val4;
	CPetVal _valArray3[7];
public:
	CPetControlSub8();

	void setArea(PetArea newArea);
};

} // End of namespace Titanic

#endif /* TITANIC_PET_CONTROL_SUB8_H */
