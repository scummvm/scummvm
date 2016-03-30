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

#ifndef TITANIC_PET_CONVERSATION_SECTION_H
#define TITANIC_PET_CONVERSATION_SECTION_H

#include "titanic/pet_control/pet_section.h"
#include "titanic/pet_control/pet_control_sub12.h"
#include "titanic/pet_control/pet_val.h"

namespace Titanic {

class CPetConversationSection : public CPetSection {
private:
	CPetVal _val1;
	CPetVal _val2;
	CPetVal _val3;
	CPetVal _valArray1[3];
	CPetVal _val4;
	CPetVal _val5;
	CPetVal _val6;
	int  _field14C;
	CPetVal _val7;
	CPetVal _val8;
	CPetVal _val9;
	CPetVal _valArray2[9];
	int _field30C;
	CPetControlSub12 _sub1;
	CPetControlSub12 _sub2;
	int _valArray3[3];
	int _field414;
	int _field418;
	CString _string1;
public:
	CPetConversationSection();

	/**
	 * Save the data for the class to file
	 */
	virtual void save(SimpleFile *file, int indent) const;

	/**
	 * Load the data for the class from file
	 */
	virtual void load(SimpleFile *file, int param);
};

} // End of namespace Titanic

#endif /* TITANIC_PET_CONVERSATION_SECTION_H */
