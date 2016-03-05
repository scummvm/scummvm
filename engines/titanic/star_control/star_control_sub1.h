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

#ifndef TITANIC_STAR_CONTROL_SUB1_H
#define TITANIC_STAR_CONTROL_SUB1_H

#include "titanic/star_control/star_control_sub2.h"
#include "titanic/star_control/star_control_sub5.h"
#include "titanic/star_control/star_control_sub7.h"
#include "titanic/star_control/star_control_sub8.h"
#include "titanic/star_control/star_control_sub9.h"
#include "titanic/star_control/star_control_sub10.h"

namespace Titanic {

class CStarControlSub1 : public CStarControlSub2 {
private:
	CStarControlSub7 _sub7;
	CStarControlSub8 _sub8;
	CStarControlSub9 _sub9;
	CStarControlSub10 _sub10;
	CStarControlSub5 _sub5;
	int _field7DA8;
	int _field7DAC;
	int _field7DB0;
	int _field7DB4;
	int _field7DB8;
	int _field7DBC;
public:
	CStarControlSub1();
};

} // End of namespace Titanic

#endif /* TITANIC_STAR_CONTROL_SUB1_H */
