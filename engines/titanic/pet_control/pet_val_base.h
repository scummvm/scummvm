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

#ifndef TITANIC_PET_VAL_BASE_H
#define TITANIC_PET_VAL_BASE_H

#include "titanic/simple_file.h"
#include "titanic/core/link_item.h"

namespace Titanic {

class CPetValBase {
protected:
	int _field4;
	int _field8;
	int _fieldC;
	int _field10;
	int _field14;
public:
	CPetValBase();
	virtual ~CPetValBase() {}

	virtual void proc1() {}
	virtual void proc2() {}
	virtual void proc3() {}
	virtual void proc4() {}

	virtual void proc5(Rect *rect);

	virtual int proc6();
	virtual int proc7();
	virtual void proc8();
	virtual int proc9();
	virtual void proc10();
	virtual void proc11();
	virtual void proc12();
	virtual void proc13();
	virtual void proc14();
	virtual void proc15();

	virtual int proc16() { return 0; }

	virtual void proc17(int v) { _field14 = v; }
};

} // End of namespace Titanic

#endif /* TITANIC_PET_VAL_BASE_H */
