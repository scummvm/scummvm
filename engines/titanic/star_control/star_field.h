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

#ifndef TITANIC_STAR_FIELD_H
#define TITANIC_STAR_FIELD_H

#include "titanic/star_control/star_control_sub2.h"
#include "titanic/star_control/star_control_sub5.h"
#include "titanic/star_control/star_control_sub7.h"
#include "titanic/star_control/star_control_sub8.h"
#include "titanic/star_control/star_points1.h"
#include "titanic/star_control/star_points2.h"

namespace Titanic {

class CStarField : public CStarControlSub2 {
private:
	CStarControlSub7 _sub7;
	CStarControlSub8 _sub8;
	CStarPoints1 _points1;
	CStarPoints2 _points2;
	CStarControlSub5 _sub5;
	int _val1;
	int _val2;
	int _val3;
	int _val4;
	int _val5;
	int _val6;
	bool _val7;
public:
	CStarField();

	/**
	 * Load the data for the class from file
	 */
	void load(SimpleFile *file, int param);

	/**
	 * Save the data for the class to file
	 */
	void save(SimpleFile *file, int indent);

	bool initDocument();

	/**
	 * Renders the contents of the starfield
	 */
	void draw(CVideoSurface *surface, CStarControlSub12 *sub12);

	int get6() const { return _val6; }
	bool get7() const { return _val7; }
};

} // End of namespace Titanic

#endif /* TITANIC_STAR_FIELD_H */
