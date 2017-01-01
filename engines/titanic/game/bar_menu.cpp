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

#include "titanic/game/bar_menu.h"

namespace Titanic {

BEGIN_MESSAGE_MAP(CBarMenu, CGameObject)
	ON_MESSAGE(PETActivateMsg)
	ON_MESSAGE(PETDownMsg)
	ON_MESSAGE(PETUpMsg)
	ON_MESSAGE(EnterViewMsg)
	ON_MESSAGE(LeaveViewMsg)
END_MESSAGE_MAP()

CBarMenu::CBarMenu() : CGameObject(), _barFrameNumber(0), _visibleFlag(false), _numFrames(6) {
}

void CBarMenu::save(SimpleFile *file, int indent) {
	file->writeNumberLine(1, indent);
	file->writeNumberLine(_barFrameNumber, indent);
	file->writeNumberLine(_visibleFlag, indent);
	file->writeNumberLine(_numFrames, indent);

	CGameObject::save(file, indent);
}

void CBarMenu::load(SimpleFile *file) {
	file->readNumber();
	_barFrameNumber = file->readNumber();
	_visibleFlag = file->readNumber();
	_numFrames = file->readNumber();

	CGameObject::load(file);
}

bool CBarMenu::PETActivateMsg(CPETActivateMsg *msg) {
	if (msg->_name == "Television") {
		_visibleFlag = !_visibleFlag;
		setVisible(_visibleFlag);
		loadFrame(_barFrameNumber);
	}

	return true;
}

bool CBarMenu::PETDownMsg(CPETDownMsg *msg) {
	if (_visibleFlag) {
		if (--_barFrameNumber < 0)
			_barFrameNumber = _numFrames - 1;

		loadFrame(_barFrameNumber);
	}

	return true;
}

bool CBarMenu::PETUpMsg(CPETUpMsg *msg) {
	if (_visibleFlag) {
		_barFrameNumber = (_barFrameNumber + 1) % _numFrames;
		loadFrame(_barFrameNumber);
	}

	return true;
}

bool CBarMenu::EnterViewMsg(CEnterViewMsg *msg) {
	petSetArea(PET_REMOTE);
	petHighlightGlyph(2);
	petSetRemoteTarget();
	setVisible(_visibleFlag);
	loadFrame(_barFrameNumber);

	return true;
}

bool CBarMenu::LeaveViewMsg(CLeaveViewMsg *msg) {
	petClear();
	_visibleFlag = false;
	setVisible(false);

	return true;
}

} // End of namespace Titanic
