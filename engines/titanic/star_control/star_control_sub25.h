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

#ifndef TITANIC_STAR_CONTROL_SUB25_H
#define TITANIC_STAR_CONTROL_SUB25_H

#include "titanic/star_control/fmatrix.h"
#include "titanic/star_control/star_control_sub26.h"

namespace Titanic {

class CStarControlSub25 {
public:
	FMatrix _matrix1;
	FMatrix _matrix2;
	CStarControlSub26 _sub1;
	CStarControlSub26 _sub2;
public:

};

} // End of namespace Titanic

#endif /* TITANIC_STAR_CONTROL_SUB25_H */
