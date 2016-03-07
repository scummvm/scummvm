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

#ifndef TITANIC_STAR_CONTROL_SUB12_H
#define TITANIC_STAR_CONTROL_SUB12_H

#include "titanic/simple_file.h"
#include "titanic/star_control/star_control_sub13.h"

namespace Titanic {

class CStarControlSub12 {
	struct ArrayEntry {
		int _field0;
		int _field4;
		int _field8;
		ArrayEntry() : _field0(0), _field4(0), _field8(0) {}
	};
private:
	int _field4;
	ArrayEntry _array[3];
	int _field2C;
	CStarControlSub13 _sub13;
	int _field108;
	int _field21C;
public:
	CStarControlSub12(void *val1, void *val2);
	virtual ~CStarControlSub12() {}

	virtual void proc3() {}

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

#endif /* TITANIC_STAR_CONTROL_SUB12_H */
