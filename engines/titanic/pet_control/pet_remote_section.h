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

#ifndef TITANIC_PET_REMOTE_SECTION_H
#define TITANIC_PET_REMOTE_SECTION_H

#include "titanic/pet_control/pet_section.h"
#include "titanic/pet_control/pet_control_sub10.h"
#include "titanic/pet_control/pet_control_sub12.h"
#include "titanic/pet_control/pet_val.h"

namespace Titanic {

class CPetRemoteSection : public CPetSection {
private:
	CPetControlSub10 _sub10;
	CPetVal _val1;
	CPetVal _val2;
	CPetVal _val3;
	CPetVal _val4;
	CPetVal _val5;
	CPetVal _val6;
	CPetVal _val7;
	CPetVal _val8;
	CPetVal _val9;
	CPetVal _val10;
	CPetVal _val11;
	CPetControlSub12 _sub12;
public:


};

} // End of namespace Titanic

#endif /* TITANIC_PET_REMOTE_SECTION_H */
