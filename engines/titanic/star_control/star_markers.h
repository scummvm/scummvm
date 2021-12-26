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

#ifndef TITANIC_STAR_MARKERS_H
#define TITANIC_STAR_MARKERS_H

#include "titanic/star_control/base_stars.h"

namespace Titanic {

class CStarMarkers : public CBaseStars {
public:
	~CStarMarkers() override { clear(); }

	/**
	 * Draw the item
	 */
	void draw(CSurfaceArea *surfaceArea, CCamera *camera, CStarCloseup *closeup) override;

	/**
	 * Adds a new star, or removes one if already present at the given co-ordinates
	 */
	bool addStar(const CBaseStarEntry *entry) override;
};

} // End of namespace Titanic

#endif /* TITANIC_STAR_MARKERS_H */
