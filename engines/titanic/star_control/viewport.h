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

#ifndef TITANIC_VIEWPORT_H
#define TITANIC_VIEWPORT_H

#include "titanic/star_control/base_stars.h"
#include "titanic/star_control/fpose.h"

class SimpleFile;

namespace Titanic {

/**
 * The color of the stars when drawn (CBaseStars::draw)
 * For starview it should be white
 * For skyview it should be pink
 */
enum StarColor { WHITE = 0, PINK = 2 };

/**
 * Implements the viewport functionality for viewing the star field in
 * a given position and orientation.
 * CCamera is a big user of this class
 */
class CViewport {
private:
	double _spin;
	double _centerYAngleDegrees;
	double _centerZAngleDegrees;
	int _width;
	int _height;
	FMatrix _orientation;
	FPose _currentPose;
	FPose _rawPose;
	FPoint _center;
	bool _poseUpToDate;
private:
	void reset();
public:
	FVector _position;
	double _frontClip;
	double _backClip;
	StarColor _starColor;	// Used in CBaseStars::draw
	double _valArray[2];	// has value 0.0 or 30.0
	double _isZero;
	double _pixel1OffSetX;	// Used in CBaseStars::draw3 and CBaseStars::draw4 has value 0.0 or 28000.0
	double _pixel2OffSetX;	// Used in CBaseStars::draw3 and CBaseStars::draw4 has value 0.0 or -28000.0
	FVector _centerVector;
public:
	CViewport();
	CViewport(CViewport *src);

	/**
	 * Copys the data from another instance
	 */
	void copyFrom(const CViewport *src);

	/**
	 * Load the data for the class from file
	 */
	void load(SimpleFile *file, int param);

	/**
	 * Save the data for the class to file
	 */
	void save(SimpleFile *file, int indent);

	/**
	 * Sets the position
	 */
	void setPosition(const FVector &v);

	/**
	 * Sets the position
	 */
	void setPosition(const FPose &pose);

	/**
	 * Sets the orientation from a passed matrix
	 */
	void setOrientation(const FMatrix &m);

	/**
	 * Sets the orientation from a passed vector
	 */
	void setOrientation(const FVector &v);

	void randomizeOrientation();

	/**
	 * The view has changed between starview and skyview
	 * Change the enum that tracks the color of the stars
	 * Also change the X coordinate pixel offset used for star drawing
	 */
	void changeStarColorPixel(StarMode mode, double pixelOffSet);
	void reposition(double factor);

	/**
	 * Applys a rotation matrix to the current
	 * orientation
	 */
	void changeOrientation(const FMatrix &matrix);

	FPose getPose();
	FPose getRawPose();
	FVector getRelativePosNoCentering(int index, const FVector &src);
	FVector getRelativePosCentering(int index, const FVector &src);
	FVector getRelativePosCenteringRaw(int index, const FVector &src);

	/**
	 * All arguments are return values
	 * First is the x center coordinate relative to y
	 * Second is the x center coordinate relative to z
	 * Third is the first x center pixel offset
	 * Fourth is the second x center pixel offset
	 */
	void getRelativeXCenterPixels(double *v1, double *v2, double *v3, double *v4);

	/**
	 * Returns the viewport's orientation
	 */
	const FMatrix &getOrientation() const;

	/**
	 * Assigns a roll angle about the view direction
	 */
	void SetRoleAngle(double angle);

	/**
	 * Assign a near clip plane distance
	 */
	void setFrontClip(double dist);

	/**
	 * Assign a far clipping plane distance
	 */
	void setBackClip(double dist);

	/**
	 * Sets the center vector y angle
	 * The actual center y value doesn't
	 * change untill reset is called
	 */
	void setCenterYAngle(double angleDegrees);

	/**
	 * Sets the center vector z angle
	 * The actual center z value doesn't
	 * change untill reset is called
	 */
	void setCenterZAngle(double angleDegrees);
};

} // End of namespace Titanic

#endif /* TITANIC_VIEWPORT_H */
