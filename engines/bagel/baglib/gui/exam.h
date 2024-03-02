
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

#ifndef BAGEL_BAGLIB_GUI_EXAM_H
#define BAGEL_BAGLIB_GUI_EXAM_H

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
		m_LeftRect(0, 0, 0, 0), m_RightRect(0, 0, 0, 0),																																		m_dwStart(0), m_dwEnd(0) {}
	~CBagExam() {}

	virtual BOOL Exam();

#if BOF_WINDOWS
	virtual VOID OnMCINotify(ULONG wParam, ULONG lParam);
#endif
	virtual VOID OnMouseMove(UINT nFlags, CBofPoint *pPoint);
	virtual VOID OnReSize(CBofSize *pSize);
	virtual VOID OnButtonUp(UINT nFlags, CBofPoint *pPoint);

	BOOL SetRotationRects();
	BOOL MarkBegEnd();

	BOOL RotateRight();
	BOOL RotateLeft();
	BOOL RotateStop();
};

} // namespace Bagel

#endif
