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

#ifndef TITANIC_PET_VAL_H
#define TITANIC_PET_VAL_H

#include "titanic/pet_control/pet_val_base.h"

namespace Titanic {

class CPetVal: public CPetValBase {
protected:
	int _field18;
	int _field1C;
	int _field20;
public:
	CPetVal() : CPetValBase(), _field18(0), _field1C(0), _field20(0) {}

	virtual void proc1();
	virtual void proc2();
	virtual void proc3();
	virtual void proc4();

	virtual void proc5(Common::Rect *linkItem);

	virtual int proc16();
};

} // End of namespace Titanic

#endif /* TITANIC_PET_VAL_H */
