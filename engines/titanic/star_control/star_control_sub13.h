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

#ifndef TITANIC_STAR_CONTROL_SUB13_H
#define TITANIC_STAR_CONTROL_SUB13_H

#include "titanic/simple_file.h"
#include "titanic/star_control/star_control_sub6.h"
#include "titanic/star_control/star_control_sub14.h"

namespace Titanic {

class CStarControlSub13 {
private:
	double _field0;
	double _field4;
	double _field8;
	double _fieldC;
	double _field10;
	double _field14;
	double _field18;
	double _field1C;
	int _field20;
	int _field22;
	int _field24;
	double _valArray[5];
	CStarControlSub14 _sub14;
	CStarControlSub6 _sub1;
	CStarControlSub6 _sub2;
	int _fieldC0;
	int _fieldC4;
	int _fieldC8;
	int _fieldCC;
	int _fieldD0;
	int _fieldD4;
private:
	void setup(void *ptr);
public:
	CStarControlSub13(void *ptr);

	/**
	 * Load the data for the class from file
	 */
	void load(SimpleFile *file, int param);

	/**
	 * Save the data for the class to file
	 */
	void save(SimpleFile *file, int indent) const;
};

} // End of namespace Titanic

#endif /* TITANIC_STAR_CONTROL_SUB13_H */
