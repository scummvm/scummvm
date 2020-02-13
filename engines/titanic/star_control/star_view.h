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

#ifndef TITANIC_STAR_VIEW_H
#define TITANIC_STAR_VIEW_H

#include "titanic/star_control/camera.h"
#include "titanic/star_control/surface_fader.h"
#include "titanic/star_control/viewport.h"
#include "titanic/support/rect.h"

namespace Titanic {

class CErrorCode;
class CGameObject;
class CStarControl;
class CStarField;
class CVideoSurface;
class FVector;

class CStarView {
private:
	CStarControl *_owner;
	CStarField *_starField;
	CVideoSurface *_videoSurface;
	CCamera _camera;
	bool _lensValid;
	CViewport _photoViewport;
	CSurfaceFader _fader;
	CVideoSurface *_photoSurface;
	CGameObject *_homePhotoMask;
	bool _stereoPair;
	bool _showingPhoto;
private:
	/**
	 * Take a photograph of a view specified by the camera
	 */
	void takeHomePhotoHelper(CCamera *camera);

	/**
	 * View a specified star
	 */
	void viewRequiredStar(int index);

	/**
	 * Gets a random position and orientation
	 */
	void getRandomViewpoint(FVector &pos, FVector &orientation);

	/**
	 * Gets a random position and orientation
	 */
	void getRandomPhotoViewpoint(FVector &pos, FVector &orientation);

	/**
	 * Handles resizing the surface
	 */
	void resizeSurface(CScreenManager *scrManager, int width, int height,
		CVideoSurface **surface);
public:
	CStarView();
	~CStarView();

	/**
	 * Load the data for the class from file
	 */
	void load(SimpleFile *file, int param);

	/**
	 * Save the data for the class to file
	 */
	void save(SimpleFile *file, int indent);

	/**
	 * Sets references used by the view
	 */
	void setup(CScreenManager *screenManager, CStarField *starField, CStarControl *starControl);

	/**
	 * Take a photograph of a view specified by the current home photo lens
	 */
	void takeCurrentHomePhoto();

	/**
	 * Allows the item to draw itself
	 */
	void draw(CScreenManager *screenManager);

	/**
	 * Updates the camera, allowing for movement
	 */
	bool updateCamera();

	/**
	 * Handles mouse down messages
	 */
	bool MouseButtonDownMsg(int unused, const Point &pt);

	/**
	 * Handles mouse move messages
	 */
	bool MouseMoveMsg(int unused, const Point &pt);

	/**
	 * Handles keyboard messages
	 */
	bool KeyCharMsg(int key, CErrorCode *errorCode);

	/**
	 * Returns true if a star destination can be set
	 */
	bool canSetStarDestination() const;

	/**
	 * Called when a star destination is set
	 */
	void starDestinationSet();

	/**
	 * Reset the starfield view
	 */
	void resetView();

	void triggerFade(bool fadeIn);

	/**
	 * View the solar system
	 */
	void viewEarth();

	/**
	 * Set the view to be from earth
	 */
	void viewFromEarth();

	/**
	 * Turn on constellation boundaries
	 */
	void viewBoundaries();

	/**
	 * Turn on the constellation lines
	 */
	void viewConstellations();

	/**
	 * Look at a random star
	 */
	void viewRandomStar();

	/**
	 * Increase starfield movement to full speed
	 */
	void fullSpeed();

	/**
	 * Enable stereo pair vision
	 */
	void toggleSteroPair();

	/**
	 * Toggles between starfield and photo modes
	 */
	void toggleHomePhoto();

	/**
	 * Toggles the solar object rendering
	 */
	void toggleSolarRendering();

	/**
	 * Toggles whether the viewpoint box is visible in the starfield
	 */
	void TogglePosFrame();

	/**
	 * Turn on Stereo Pair imaging
	 */
	void stereoPairOn();

	/**
	 * Turn off Stereo Pair imaging
	 */
	void stereoPairOff();

	/**
	 * Called when the photograph is used on the navigation computer,
	 * takes a photograph of the current view, writing it to the home photo surface
	 */
	void takeHomePhoto();

	/**
	 * Handles locking in a star
	 */
	void lockStar();

	/**
	 * Handles unlocking a star
	 */
	void unlockStar();
};

} // End of namespace Titanic

#endif /* TITANIC_STAR_VIEW_H */
