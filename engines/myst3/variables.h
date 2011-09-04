/* Residual - A 3D game interpreter
 *
 * Residual is the legal property of its developers, whose names
 * are too numerous to list here. Please refer to the AUTHORS
 * file distributed with this source distribution.
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Lesser General Public
 * License as published by the Free Software Foundation; either
 * version 2.1 of the License, or (at your option) any later version.

 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Lesser General Public License for more details.

 * You should have received a copy of the GNU Lesser General Public
 * License along with this library; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301, USA
 *
 */

#ifndef VARIABLES_H_
#define VARIABLES_H_

#include "engines/myst3/myst3.h"

namespace Myst3 {

class Variables {
public:
	Variables(Myst3Engine *vm);
	virtual ~Variables();

	uint32 get(uint16 var);
	void set(uint16 var, uint32 value);
	bool evaluate(int16 condition);
	uint32 valueOrVarValue(int16 value);

private:
	Myst3Engine *_vm;

	uint32 _vars[2048];

	void checkRange(uint16 var);
};

} /* namespace Myst3 */
#endif /* VARIABLES_H_ */
