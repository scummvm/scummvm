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
#include "titanic/star_control/const_boundaries.h"
#include "titanic/star_control/constellations.h"

namespace Titanic {

#define STAR_SCALE 1024.0F
#define UNIVERSE_SCALE 3000000.0f

class CStarField : public CStarFieldBase {
private:
	CStarMarkers _markers;
	CStarCrosshairs _crosshairs;
	CConstBoundaries _constBounds;
	CConstellations _constMap;
	CStarCloseup _starCloseup;
	bool _renderBoundaries;
	bool _renderConstMap;
	StarMode _mode;
	bool _showBox;
	bool _closeToMarker;
	bool _isSolved;
private:
	/**
	 * Draws the big square box in the middle of the screen
	 */
	void drawBox(CSurfaceArea *surfaceArea);

	/**
	 * If the player's home photo has a selected star, and the starfield view
	 * is close enough to it, draw a lock line
	 */
	void renderLockLine(CSurfaceArea *surfaceArea, CCamera *camera);
public:
	CStarField();

	/**
	 * Load the data for the class from file
	 */
	void load(SimpleFile *file) override;

	/**
	 * Save the data for the class to file
	 */
	void save(SimpleFile *file, int indent) override;

	bool initDocument();

	/**
	 * Renders the contents of the starfield
	 */
	void render(CVideoSurface *surface, CCamera *camera);

	bool getBoundaryState() const;

	void setBoundaryState(bool state);

	bool getConstMapState() const;

	void setConstMapState(bool state);

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

	/**
	  * Gets the lock distance to a star
	  */
	double lockDistance(CSurfaceArea *surfaceArea, CCamera *camera,
		FVector &screenCoord, FVector &worldCoord, FVector &photoPos);

	void fn6(CVideoSurface *surface, CCamera *camera);

	/**
	 * Increments the number of matched markers
	 */
	void incLockLevel();

	/**
	 * Decrements the number of matched markers
	 */
	void decLockLevel(CVideoSurface *surface);

	void ToggleSolarRendering() { _starCloseup.fn1(); }

	/**
	 * Called when the starfield is clicked
	 */
	bool mouseButtonDown(CVideoSurface *surface, CCamera *camera,
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
