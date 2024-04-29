
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

#ifndef BAGEL_BAGLIB_EXAM_H
#define BAGEL_BAGLIB_EXAM_H

#include "bagel/boflib/gui/movie.h"

namespace Bagel {

class CBagExam : public CBofMovie {
protected:
	CBofRect _leftRect;
	CBofRect _rightRect;
	uint32 _start;
	uint32 _end;

public:
	CBagExam(CBofWindow *pParent = nullptr) : CBofMovie(pParent),
		_leftRect(0, 0, 0, 0), _rightRect(0, 0, 0, 0), _start(0), _end(0) {}
	CBagExam(CBofWindow *pParent, const char *sFilename) : CBofMovie(pParent, sFilename),
		_leftRect(0, 0, 0, 0), _rightRect(0, 0, 0, 0), _start(0), _end(0) {}
	CBagExam(CBofWindow *pParent, const char *sFilename, CBofRect *pBounds) : CBofMovie(pParent, sFilename, pBounds),
			_leftRect(0, 0, 0, 0), _rightRect(0, 0, 0, 0), _start(0), _end(0) {}
	~CBagExam() {}

	/**
	 * Initialize exam object after opened and ready to play
	 * @return      Success/failure
	 */
	virtual bool initExam();

	/**
	 * Called when the mouse is moved over window, check if the
	 * cursor is in one of the rectangle and rotate object accordingly
	 */
	void onMouseMove(uint32 /* flags */ , CBofPoint *point, void * /* extraInfo */ = nullptr) override;

	/**
	 * Resize the examination window
	 */
	void onReSize(CBofSize *size) override;
	void onButtonUp(uint32 /* flags */, CBofPoint * /* point */) override;

	/**
	 * Set the rectangles that determine "hot" mouse locations
	 * @return      Success/failure
	 */
	bool setRotationRects();

	/**
	 * Store off the start and end frames of the movies
	 * @return      Success/failure
	 */
	bool markBegEnd();

	/**
	 * Rotate the object to the right (play movie)
	 */
	bool rotateRight();

	/**
	 * Rotate the object to the left (reverse movie)
	 */
	bool rotateLeft();

	/**
	 * Stop the rotation of the object (stop movie)
	 */
	bool rotateStop();
};

} // namespace Bagel

#endif
