/* ScummVM - Graphic Adventure Engine
 *
 * ScummVM is the legal property of its developers, whose names
 * are too numerous to list here. Please refer to the COPYRIGHT
 * file distributed with this source distribution.
 *
 * Additional copyright for this file:
 * Copyright (C) 1999-2000 Revolution Software Ltd.
 * This code is based on source code created by Revolution Software,
 * used with permission.
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 *
 */

#ifndef ICB_INCLUDED_P4_GENERIC_H
#define ICB_INCLUDED_P4_GENERIC_H

#include "engines/icb/p4.h"
#include "engines/icb/p4_generic_pc.h"

namespace ICB {

inline PXfloat remainder(const PXfloat value, const PXfloat divisor, const PXfloat half_divisor) {
	PXfloat mod = (PXfloat)PXfmod(value, divisor);

	if (PXfabs(mod) > half_divisor) {
		if (mod < FLOAT_ZERO)
			mod += divisor;
		else
			mod -= divisor;
	}

	return (mod);
}

} // End of namespace ICB

#endif // INCLUDED_P4_GENERIC_H
