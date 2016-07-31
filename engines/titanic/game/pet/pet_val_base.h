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

#ifndef TITANIC_pet_element_H
#define TITANIC_pet_element_H

namespace Titanic {

class CPetElement {
protected:
	int _field4;
	int _field8;
	int _fieldC;
	int _field10;
	int _field14;
public:
	CPetElement();

	virtual void proc1() {}
	virtual void proc2() {}
	virtual void proc3() {}
	virtual void proc4() {}

	virtual void proc5() {}

	virtual void proc6() {}
	virtual void proc7() {}
	virtual void proc8() {}
	virtual void proc9() {}
	virtual void proc10() {}
	virtual void proc11() {}
	virtual void proc12() {}
	virtual void proc13() {}
	virtual void proc14() {}
	virtual void proc15() {}
	virtual void proc16() {}
	virtual void proc17() {}
};

} // End of namespace Titanic

#endif /* TITANIC_pet_element_H */
