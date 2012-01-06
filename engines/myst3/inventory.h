/* Residual - A 3D game interpreter
 *
 * Residual is the legal property of its developers, whose names
 * are too numerous to list here. Please refer to the AUTHORS
 * file distributed with this source distribution.
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License
 * as published by the Free Software Foundation; either version 2
 * of the License, or (at your option) any later version.

 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.

 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.
 *
 */

#ifndef INVENTORY_H_
#define INVENTORY_H_

#include "common/list.h"

namespace Myst3 {

class Myst3Engine;

class Inventory {
public:
	Inventory(Myst3Engine *vm);
	virtual ~Inventory();

	void reset();
	void addItem(uint16 var, bool atEnd);
	void removeItem(uint16 var);

private:
	Myst3Engine *_vm;

	Common::List<uint16> _inventory;
};

} /* namespace Myst3 */
#endif /* INVENTORY_H_ */
