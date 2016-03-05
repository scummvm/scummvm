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

#ifndef TITANIC_PET_CONTROL_SUB_BASE_H
#define TITANIC_PET_CONTROL_SUB_BASE_H

#include "titanic/simple_file.h"

namespace Titanic {

struct CPetControlSubData {
	int _field0;
	int _field4;
	int _field8;
	int _fieldC;

	CPetControlSubData() : _field0(0), _field4(0),
		_field8(0), _fieldC(0) {}
};

class CPetControlSubBase {
protected:
	int _field4;
public:
	CPetControlSubBase() : _field4(0) {}
	virtual ~CPetControlSubBase() {}

	virtual int proc1() { return 0; }
	virtual int proc2() { return 0; }
	virtual void proc3() {}
	virtual void proc4();
	virtual void proc5() {}
	virtual int proc6() { return 0; }
	virtual int proc7() { return 0; }
	virtual int proc8() { return 0; }
	virtual int proc9() { return 0; }
	virtual int proc10() { return 0; }
	virtual int proc11() { return 0; }
	virtual int proc12() { return 0; }
	virtual int proc13() { return 0; }
	virtual int proc14() { return 0; }
	virtual int proc15() { return 0; }
	virtual void proc16();

	/**
	 * Returns true if the object is in a valid state
	 */
	virtual bool isValid() const { return false; }

	/**
	 * Load the data for the class from file
	 */
	virtual void load(SimpleFile *file) = 0;

	virtual void proc19() {}

	/**
	 * Save the data for the class to file
	 */
	virtual void save(SimpleFile *file, int indent) const = 0;

	virtual void proc21() {}
	virtual void proc22() {}
	virtual void proc23() {}
	virtual void proc24() {}
	virtual void proc25();
	virtual int proc26() { return 0; }
	virtual void proc27();
	virtual void proc28();
	virtual void proc29();
	virtual void proc30();
	virtual void proc31();
	virtual void proc32() {}
	virtual void proc33() {}
	virtual void proc34() {}
	virtual void proc35() {}
	virtual void proc36() {}
	virtual void proc37() {}
	virtual void proc38() {}
};

} // End of namespace Titanic

#endif /* TITANIC_PET_CONTROL_SUB_BASE_H */
