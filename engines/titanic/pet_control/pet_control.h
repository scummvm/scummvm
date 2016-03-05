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

#ifndef TITANIC_PET_CONTROL_H
#define TITANIC_PET_CONTROL_H

#include "titanic/core/game_object.h"
#include "titanic/pet_control/pet_control_sub1.h"
#include "titanic/pet_control/pet_control_sub2.h"
#include "titanic/pet_control/pet_control_sub3.h"
#include "titanic/pet_control/pet_control_sub4.h"
#include "titanic/pet_control/pet_control_sub5.h"
#include "titanic/pet_control/pet_control_sub6.h"
#include "titanic/pet_control/pet_control_sub7.h"
#include "titanic/pet_control/pet_control_sub8.h"

namespace Titanic {

class CPetControl : public CGameObject {
private:
	int _fieldBC;
	int _fieldC0;
	int _fieldC4;
	int _fieldC8;
	CPetControlSub1 _sub1;
	CPetControlSub2 _sub2;
	CPetControlSub3 _sub3;
	CPetControlSub4 _sub4;
	CPetControlSub5 _sub5;
	CPetControlSub6 _sub6;
	CPetControlSub7 _sub7;
	CPetControlSub8 _sub8;
	int _field1384;
	CString _string1;
	int _field1394;
	CString _string2;
	int _field13A4;
private:
	void loadSubObjects(SimpleFile *file);

	void saveSubObjects(SimpleFile *file, int indent) const;
public:
	/**
	 * Return the class name
	 */
	virtual const char *getClassName() const { return "CPetControl"; }

	/**
	 * Save the data for the class to file
	 */
	virtual void save(SimpleFile *file, int indent) const;

	/**
	 * Load the data for the class from file
	 */
	virtual void load(SimpleFile *file);

	/**
	 * Called after loading a game has finished
	 */
	void gameLoaded();
};

} // End of namespace Titanic

#endif /* TITANIC_PET_CONTROL_H */
