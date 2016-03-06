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

#ifndef TITANIC_STAR_CONTROL_SUB3_H
#define TITANIC_STAR_CONTROL_SUB3_H

#include "titanic/simple_file.h"
#include "titanic/star_control/star_control_sub4.h"

namespace Titanic {

class CStarControlSub3 {
protected:
	int _field4;
	int _field8;
	int _fieldC;
	CStarControlSub4 _sub4;
	int _field28;
	int _field2C;
public:
	CStarControlSub3();
	virtual ~CStarControlSub3() {}

	virtual void proc2();
	virtual int proc3() { return 1; }
	virtual int proc4() { return 0; }
	virtual int proc5() { return 0; }
	virtual int proc6() { return 0; }
	virtual int proc7() { return 1; }

	virtual void load(SimpleFile *file) {}

	virtual void proc9() {}

};

} // End of namespace Titanic

#endif /* TITANIC_STAR_CONTROL_SUB3_H */
