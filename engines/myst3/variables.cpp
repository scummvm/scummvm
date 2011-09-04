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

#include "engines/myst3/variables.h"

namespace Myst3 {

Variables::Variables(Myst3Engine *vm):
	_vm(vm) {
	memset(&_vars, 0, sizeof(_vars));
	_vars[1] = 1;
}

Variables::~Variables() {
}

void Variables::checkRange(uint16 var) {
	if (var < 1 || var > 2047)
		error("Variable out of range %d", var);
}

uint32 Variables::get(uint16 var) {
	checkRange(var);
	return _vars[var];
}

void Variables::set(uint16 var, uint32 value) {
	checkRange(var);
	_vars[var] = value;
}

bool Variables::evaluate(int16 condition) {
	uint16 unsignedCond = abs(condition);
	uint16 var = unsignedCond & 2047;
	int32 varValue = get(var);
	int32 targetValue = (unsignedCond >> 11) - 1;

	if (targetValue >= 0) {
		if (condition >= 0)
			return varValue == targetValue;
		else
			return varValue != targetValue;
	} else {
		if (condition >= 0)
			return varValue != 0;
		else
			return varValue == 0;
	}
}

} /* namespace Myst3 */
