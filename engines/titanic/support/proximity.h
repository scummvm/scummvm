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

#ifndef TITANIC_PROXIMITY_H
#define TITANIC_PROXIMITY_H

#include "common/scummsys.h"

namespace Titanic {

typedef bool (*CProximityFn)(int val);

class TTtalker;

class CProximity {
public:
	int _field4;
	int _field8;
	int _fieldC;
	int _speechHandle;
	int _field14;
	int _field18;
	int _field1C;
	int _field20;
	int _field24;
	int _field28;
	uint32 _field2C;
	int _field30;
	int _field34;
	double _double1;
	double _double2;
	double _double3;
	int _field44;
	int _field48;
	int _field4C;
	int _field50;
	int _field54;
	int _field58;
	int _field5C;
	int _field60;
	CProximityFn _method1;
	TTtalker *_talker;
	int _field6C;
public:
	CProximity();
};

} // End of namespace Titanic

#endif /* TITANIC_PROXIMITY_H */
