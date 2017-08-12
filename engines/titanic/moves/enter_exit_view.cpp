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

#include "titanic/moves/enter_exit_view.h"

namespace Titanic {

BEGIN_MESSAGE_MAP(CEnterExitView, CGameObject)
	ON_MESSAGE(EnterViewMsg)
	ON_MESSAGE(LeaveViewMsg)
	ON_MESSAGE(MovieEndMsg)
END_MESSAGE_MAP()

CEnterExitView::CEnterExitView() : CGameObject(), _leaveEndFrame(0),
	_leaveStartFrame(0), _enterEndFrame(0), _enterStartFrame(0),
	_visibleAfterMovie(true) {
}

void CEnterExitView::save(SimpleFile *file, int indent) {
	file->writeNumberLine(1, indent);
	file->writeNumberLine(_leaveEndFrame, indent);
	file->writeNumberLine(_leaveStartFrame, indent);
	file->writeNumberLine(_enterEndFrame, indent);
	file->writeNumberLine(_enterStartFrame, indent);
	file->writeNumberLine(_visibleAfterMovie, indent);

	CGameObject::save(file, indent);
}

void CEnterExitView::load(SimpleFile *file) {
	file->readNumber();
	_leaveEndFrame = file->readNumber();
	_leaveStartFrame = file->readNumber();
	_enterEndFrame = file->readNumber();
	_enterStartFrame = file->readNumber();
	_visibleAfterMovie = file->readNumber();

	CGameObject::load(file);
}

bool CEnterExitView::EnterViewMsg(CEnterViewMsg *msg) {
	setVisible(true);
	playMovie(_enterStartFrame, _enterEndFrame, MOVIE_NOTIFY_OBJECT);
	return true;
}

bool CEnterExitView::LeaveViewMsg(CLeaveViewMsg *msg) {
	setVisible(true);
	playMovie(_leaveStartFrame, _leaveEndFrame, MOVIE_WAIT_FOR_FINISH);
	return true;
}

bool CEnterExitView::MovieEndMsg(CMovieEndMsg *msg) {
	if (!_visibleAfterMovie)
		setVisible(false);
	return true;
}

} // End of namespace Titanic
