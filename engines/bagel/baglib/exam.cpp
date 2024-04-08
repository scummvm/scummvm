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

#include "bagel/baglib/exam.h"
#include "bagel/dialogs/opt_window.h"
#include "bagel/baglib/bagel.h"

namespace Bagel {

BOOL CBagExam::Exam() {
	CBofRect r(155, 55, 155 + 330 - 1, 55 + 250 - 1);
	PaintBitmap(CBagel::GetBagApp()->GetMasterWnd()->GetCurrentGameWindow(), BuildSysDir("SSBORDER.BMP"), &r);

	Common::Event e;
	while (g_system->getEventManager()->pollEvent(e)) {
		// Flush events, though not sure why we need it
	}

	GetParent()->Disable();
	GetParent()->FlushAllMessages();

	MarkBegEnd();
	SetRotationRects();
	m_bLoop = TRUE;
	m_bEscCanStop = TRUE;

	Pause();

	CBofCursor::Show();
	DoModal();
	CBofCursor::Hide();

	return TRUE;
}

void CBagExam::OnReSize(CBofSize *pSize) {

	CBofMovie::OnReSize(pSize);
	SetRotationRects();

}

BOOL CBagExam::SetRotationRects() {
	CBofRect rcClient = GetClientRect();    // Get the  windows rect

	// Left quarter of the video window
	m_LeftRect.left = rcClient.left;
	m_LeftRect.top = rcClient.top;
	m_LeftRect.right = rcClient.left + ((rcClient.right - rcClient.left) / 4);
	m_LeftRect.bottom = rcClient.bottom;

	// Right quarter of the video window
	m_RightRect.left = rcClient.right - ((rcClient.bottom - rcClient.left) / 4);
	m_RightRect.top = rcClient.top;
	m_RightRect.right = rcClient.right;
	m_RightRect.bottom = rcClient.bottom;

	return TRUE;
}

BOOL CBagExam::MarkBegEnd() {
	m_dwEnd = m_pSmk->getFrameCount() - 1;
	m_dwStart = 0;

	return TRUE;
}

void  CBagExam::OnButtonUp(uint32 /*nFlags*/, CBofPoint * /*pPoint*/) {
	// Clean up and exit
	m_bLoop = FALSE;

	Stop();
	OnMovieDone();
}


BOOL CBagExam::RotateLeft() {
	if (m_eMovStatus != FOREWARD)
		return Play();
	else
		return TRUE;
}

BOOL CBagExam::RotateRight() {
	if (m_eMovStatus != REVERSE)
		return Reverse();
	else
		return TRUE;
}

BOOL CBagExam::RotateStop() {
	if (m_eMovStatus == FOREWARD || m_eMovStatus == REVERSE)
		// The movie is currently playing
		return Pause();
	else
		return TRUE;

}

void CBagExam::OnMouseMove(uint32 /*nFlags*/, CBofPoint *pPoint, void *) {
	Assert(IsValidObject(this));
	Assert(pPoint != nullptr);

	// No more cursor in Examine movies
	if (pPoint->x <= m_LeftRect.right) {
		// left rect, play reverse
		RotateLeft();
	} else if (pPoint->x >= m_RightRect.left) {
		//right rect, play forward
		RotateRight();
	} else if (m_eMovStatus == FOREWARD || m_eMovStatus == REVERSE) {
		RotateStop();
	}
}

} // namespace Bagel
