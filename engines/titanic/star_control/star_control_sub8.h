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

#ifndef TITANIC_STAR_CONTROL_SUB8_H
#define TITANIC_STAR_CONTROL_SUB8_H

#include "common/array.h"
#include "common/rect.h"
#include "titanic/star_control/base_star.h"
#include "titanic/star_control/surface_area.h"
#include "titanic/star_control/fpoint.h"
#include "titanic/support/simple_file.h"
#include "titanic/support/video_surface.h"

namespace Titanic {

class CStarField;
class CStarControlSub7;
class CStarControlSub12;

class CStarControlSub8 {
private:
	Common::Array<CStarPosition> _positions;
	int _entryIndex;
	Common::Rect _entries[3];
private:
	/**
	 * Allocates space in the _rects array
	 */
	void allocate(int count);

	/**
	 * Clears any current data
	 */
	void clear();
public:
	int _field8;
	int _fieldC;
public:
	CStarControlSub8();
	~CStarControlSub8() { clear(); }

	/**
	 * Load the data for the class from file
	 */
	void load(SimpleFile *file) {}

	/**
	 * Save the data for the class to file
	 */
	void save(SimpleFile *file, int indent) {}

	int findStar(const Common::Point &pt);

	void selectStar(int starNum, CVideoSurface *surface, CStarField *starField,
		CStarControlSub7 *sub7);

	void draw(CSurfaceArea *surfaceArea);

	bool fn1(CStarField *starField, CSurfaceArea *surfaceArea, CStarControlSub12 *sub12);
	void fn2(CVideoSurface *surface, CStarField *starField, CStarControlSub7 *sub7);
	void fn3();
	void fn4(int index, CSurfaceArea *surfaceArea);
	void fn5(int index, CVideoSurface *surface, CStarField *starField, CStarControlSub7 *sub7);
	void fn6(CSurfaceArea *surfaceArea);
	void fn7(const FPoint &pt, CSurfaceArea *surfaceArea);
	FPoint getPosition() const;

	/**
	 * Returns the index of an entry in the rects list a given point falls within
	 */
	int indexOf(const Common::Point &pt) const;
};

} // End of namespace Titanic

#endif /* TITANIC_STAR_CONTROL_SUB8_H */
