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

#ifndef TITANIC_CONST_BOUNDARIES_H
#define TITANIC_CONST_BOUNDARIES_H

#include "titanic/star_control/fvector.h"
#include "common/array.h"

namespace Titanic {

class CCamera;
class CSurfaceArea;

class CConstBoundaries {
	struct CBoundaryVector : public FVector {
		bool _isDrawn;		// Line is drawn to previous point
		CBoundaryVector() : FVector(), _isDrawn(false) {}
	};
private:
	Common::Array<CBoundaryVector> _data;
public:
	CConstBoundaries();

	/**
	 * Initialize the array
	 */
	bool initialize();

	/**
	 * Draw the boundary structure
	 */
	void draw(CSurfaceArea *surface, CCamera *camera);
};

} // End of namespace Titanic

#endif /* TITANIC_CONST_BOUNDARIES_H */
