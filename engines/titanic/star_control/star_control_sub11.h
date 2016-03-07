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

#ifndef TITANIC_STAR_CONTROL_SUB11_H
#define TITANIC_STAR_CONTROL_SUB11_H

#include "titanic/simple_file.h"
#include "titanic/star_control/star_control_sub12.h"
#include "titanic/star_control/star_control_sub13.h"
#include "titanic/star_control/star_control_sub15.h"

namespace Titanic {

class CStarControlSub11 {
private:
	int _field0;
	int _field4;
	int _field8;
	CStarControlSub12 _sub12;
	int _field118;
	CStarControlSub13 _sub13;
	CStarControlSub15 _sub15;
	int _field20C;
	int _field210;
	int _field214;
	int _field218;
	int _field21C;
public:
	CStarControlSub11();

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

#endif /* TITANIC_STAR_CONTROL_SUB11_H */
