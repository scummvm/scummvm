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

#include "titanic/support/simple_file.h"
#include "titanic/support/video_surface.h"
#include "titanic/star_control/star_control_sub12.h"
#include "titanic/star_control/star_control_sub13.h"
#include "titanic/star_control/surface_fader.h"
#include "titanic/star_control/error_code.h"
#include "titanic/star_control/fvector.h"

namespace Titanic {

class CStarControl;
class CStarField;

class CStarView {
private:
	CStarControl *_owner;
	CStarField *_starField;
	CVideoSurface *_videoSurface;
	CStarControlSub12 _sub12;
	int _field118;
	CStarControlSub13 _sub13;
	CSurfaceFader _fader;
	CVideoSurface *_videoSurface2;
	CGameObject *_homePhotoMask;
	int _field218;
	bool _showingPhoto;
#if 0
	int _field210;
#endif
private:
	void fn1();
	void fn18(CStarControlSub12 *sub12);

	void randomizeVectors(FVector *v1, FVector *v2);

	/**
	 * Handles resizing the surface
	 */
	void resizeSurface(CScreenManager *scrManager, int width, int height,
		CVideoSurface **surface);
public:
	CStarView();

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

	void reset();

	/**
	 * Allows the item to draw itself
	 */
	void draw(CScreenManager *screenManager);

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
	 * Resets back to the origin position
	 */
	void resetPosition();
	void petDestinationSet();

	void fn2();
	void fn3(bool fadeIn);
	void fn4();
	void fn5();
	void fn6();
	void fn7();
	void fn8();
	void fn9();

	/**
	 * Toggles between starfield and photo modes
	 */
	void toggleMode();
	
	void fn11();
	void fn12();
	void fn13();
	void fn14();

	/**
	 * Called when the photograph is used on the navigation computer
	 */
	void setHasReference();
	
	void fn16();
	void fn17();

};

} // End of namespace Titanic

#endif /* TITANIC_STAR_RENDERER_H */
