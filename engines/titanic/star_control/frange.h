/* ScummVM - Graphic Adventure Engine
 *
 * ScummVM is the legal property of its developers, whose names
 * are too numerous to list here. Please refer to the COPYRIGHT
 * file distributed with this source distribution.
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

#ifndef TITANIC_FRANGE_H
#define TITANIC_FRANGE_H

#include "titanic/star_control/fvector.h"

namespace Titanic {

class FRange {
private:
	FVector _min;
	FVector _max;
public:
	FRange();

	/**
	 * Resets the minimum & maximum vector values
	 */
	void reset();

	/**
	 * Expands the minimum & maximum as necessary to encompass the passed vector/
	 */
	void expand(const FVector &v);
};

} // End of namespace Titanic

#endif /* TITANIC_FRANGE_H */
