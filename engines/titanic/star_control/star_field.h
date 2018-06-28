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

#include "titanic/star_control/star_field_base.h"
#include "titanic/star_control/star_closeup.h"
#include "titanic/star_control/star_markers.h"
#include "titanic/star_control/star_crosshairs.h"
#include "titanic/star_control/star_points1.h"
#include "titanic/star_control/star_points2.h"

namespace Titanic {

class CStarField : public CStarFieldBase {
private:
	CStarMarkers _markers;
	CStarCrosshairs _crosshairs;
	CStarPoints1 _points1;
	CStarPoints2 _points2;
	CStarCloseup _starCloseup;
	bool _points1On;
	bool _points2On;
	StarMode _mode;
	bool _showBox;
	bool _closeToMarker;
	bool _isSolved;
private:
	/**
	 * Draws the big square box in the middle of the screen
	 */
	void drawBox(CSurfaceArea *surfaceArea);

	void fn4(CSurfaceArea *surfaceArea, CStarCamera *camera);
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
	void render(CVideoSurface *surface, CStarCamera *camera);

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
	 * Toggles whether the big box is visible
	 */
	void toggleBox();

	/**
	 * Sets whether the big box is visible
	 */
	bool setBoxVisible(bool isVisible);

	/**
	 * Returns the index for the number of star matches
	 */
	int getMatchedIndex() const;

	/**
	 * Returns true if the center of the starfield viewpoint is close to a marker
	 */
	bool isCloseToMarker() const;

	/**
	 * Sets the flag that the starfield has been solved
	 */
	void setSolved();

	/**
	 * Returns true if the starfield puzzle has been solved
	 */
	bool isSolved() const;

	/**
	 * Return true if the starfield puzzle was skipped
	 */
	bool isSkipped() const;

	/**
	 * Skips the starfield puzzle
	 */
	void skipPuzzle();

	/**
	 * Returns the number of markers placed in the starfield
	 */
	int getMarkerCount() const {
		return _markers.size();
	}

	void fn1(CErrorCode *errorCode);
	double fn5(CSurfaceArea *surfaceArea, CStarCamera *camera,
		FVector &v1, FVector &v2, FVector &v3);
	void fn6(CVideoSurface *surface, CStarCamera *camera);

	/**
	 * Increments the number of matched markers
	 */
	void incMatches();

	void fn8(CVideoSurface *surface);
	void fn9() { _starCloseup.fn1(); }

	/**
	 * Called when the starfield is clicked
	 */
	bool mouseButtonDown(CVideoSurface *surface, CStarCamera *camera,
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
