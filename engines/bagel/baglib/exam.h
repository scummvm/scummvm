
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
	CBofRect m_LeftRect;
	CBofRect m_RightRect;
	DWORD m_dwStart;
	DWORD m_dwEnd;

public:
	CBagExam(CBofWindow *pParent = nullptr) : CBofMovie(pParent),
		m_LeftRect(0, 0, 0, 0), m_RightRect(0, 0, 0, 0), m_dwStart(0), m_dwEnd(0) {}
	CBagExam(CBofWindow *pParent, const char *sFilename) : CBofMovie(pParent, sFilename),
		m_LeftRect(0, 0, 0, 0), m_RightRect(0, 0, 0, 0), m_dwStart(0), m_dwEnd(0) {}
	CBagExam(CBofWindow *pParent, const char *sFilename, CBofRect *pBounds,
	         BOOL bStretch = FALSE, BOOL bUseNewPal = TRUE, BOOL bBlackOutWindow = FALSE) :
		CBofMovie(pParent, sFilename, pBounds, bStretch, bUseNewPal, bBlackOutWindow),
		m_LeftRect(0, 0, 0, 0), m_RightRect(0, 0, 0, 0),                                                                                                                                        m_dwStart(0), m_dwEnd(0) {}
	~CBagExam() {}

	/**
	 * Initailize exam object after opened and ready to play
	 * @return      Success/failure
	 */
	virtual BOOL Exam();

#if BOF_WINDOWS
	virtual VOID OnMCINotify(ULONG wParam, ULONG lParam);
#endif

	/**
	 * Called when the mouse is moved over window, check if the
	 * cursor is in one of the rectangle and rotate object accordingly
	 */
	virtual VOID OnMouseMove(UINT nFlags, CBofPoint *pPoint);

	/**
	 * Resize the examination window
	 */
	virtual VOID OnReSize(CBofSize *pSize);
	virtual VOID OnButtonUp(UINT nFlags, CBofPoint *pPoint);

	/**
	 * Set the rectangles that determine "hot" mouse locations
	 * @return      Success/failure
	 */
	BOOL SetRotationRects();

	/**
	 * Store off the start and end frames of the movies
	 * @return      Success/failure
	 */
	BOOL MarkBegEnd();

	/**
	 * Rotate the object to the right (play movie)
	 */
	BOOL RotateRight();

	/**
	 * Rotate the object to the left (reverse movie)
	 */
	BOOL RotateLeft();

	/**
	 * Stop the rotation of the object (stop movie)
	 */
	BOOL RotateStop();
};

} // namespace Bagel

#endif
