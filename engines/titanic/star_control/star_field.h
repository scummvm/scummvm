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
	bool _points1On;
	bool _points2On;
	StarMode _mode;
	bool _showCrosshairs;
	int _val5;
	bool _isSolved;
private:
	/**
	 * Draws the square box crosshairs in the middle of the screen
	 */
	void drawCrosshairs(CSurfaceArea *surfaceArea);

	void fn4(CSurfaceArea *surfaceArea, CStarControlSub12 *sub12);
public:
	CStarField();

	/**
	 * Load the data for the class from file
	 */
	void load(SimpleFile *file);

	/**
	 * Save the data for the class to file
	 */
	void save(SimpleFile *file, int indent);

	bool initDocument();

	/**
	 * Renders the contents of the starfield
	 */
	void render(CVideoSurface *surface, CStarControlSub12 *sub12);

	int get1() const;
	void set1(int val);
	int get2() const;
	void set2(int val);
	int get54() const;
	void set54(int val);
	
	/**
	 * Gets the current display mode
	 */
	StarMode getMode() const;
	
	/**
	 * Sets the display mode
	 */
	void setMode(StarMode mode);
	
	/**
	 * Toggles whether the crosshairs box is visible
	 */
	void toggleCrosshairs();

	/**
	 * Sets whether the crosshairs box is visible
	 */
	bool setCrosshairs(bool isVisible);

	int get88() const;
	int get5() const;

	/**
	 * Sets the flag that the starfield has been solved
	 */
	void setSolved();

	/**
	 * Returns true if the starfield puzzle has been solved
	 */
	bool isSolved() const;

	int get7Count() const {
		return _sub7.size();
	}

	void fn1(CErrorCode *errorCode);
	double fn5(CSurfaceArea *surfaceArea, CStarControlSub12 *sub12,
		FVector &v1, FVector &v2, FVector &v3);
	void fn6(CVideoSurface *surface, CStarControlSub12 *sub12);
	void fn7();
	void fn8(CVideoSurface *surface);
	void fn9() { _sub5.fn1(); }

	/**
	 * Called when the starfield is clicked
	 */
	bool mouseButtonDown(CVideoSurface *surface, CStarControlSub12 *sub12,
		int flags, const Common::Point &pt);

	/**
	 * Gets a random star
	 */
	const CBaseStarEntry *getRandomStar() const;

	/**
	 * Gets a specified star
	 */
	const CBaseStarEntry *getStar(int index) const;
};

} // End of namespace Titanic

#endif /* TITANIC_STAR_FIELD_H */
