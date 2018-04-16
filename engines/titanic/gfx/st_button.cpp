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

#include "titanic/gfx/st_button.h"

namespace Titanic {

BEGIN_MESSAGE_MAP(CSTButton, CBackground)
	ON_MESSAGE(MouseButtonDownMsg)
	ON_MESSAGE(MouseButtonUpMsg)
	ON_MESSAGE(EnterViewMsg)
END_MESSAGE_MAP()

CSTButton::CSTButton() : CBackground() {
	_statusInc = 0;
	_actionTarget = "NULL";
	_fieldF0 = 0;
	_currentStatus = 0;
	_actionName = "NULL";
	_soundName = "NULL";
	_buttonFrame = 0;
}

void CSTButton::save(SimpleFile *file, int indent) {
	file->writeNumberLine(1, indent);
	file->writeNumberLine(_statusInc, indent);
	file->writeQuotedLine(_actionTarget, indent);
	file->writeNumberLine(_fieldF0, indent);
	file->writeNumberLine(_currentStatus, indent);
	file->writeQuotedLine(_actionName, indent);
	file->writeQuotedLine(_soundName, indent);
	file->writeNumberLine(_buttonFrame, indent);

	CBackground::save(file, indent);
}

void CSTButton::load(SimpleFile *file) {
	file->readNumber();
	_statusInc = file->readNumber();
	_actionTarget = file->readString();
	_fieldF0 = file->readNumber();
	_currentStatus = file->readNumber();
	_actionName = file->readString();
	_soundName = file->readString();
	_buttonFrame = file->readNumber() != 0;

	CBackground::load(file);
}

bool CSTButton::MouseButtonDownMsg(CMouseButtonDownMsg *msg) {
	playMovie(0);
	playSound(_soundName);

	return true;
}

bool CSTButton::MouseButtonUpMsg(CMouseButtonUpMsg *msg) {
	int oldStatus = _currentStatus;
	int newStatus = _currentStatus + _statusInc;

	CStatusChangeMsg statusMsg(oldStatus, newStatus, false);
	_currentStatus = newStatus;
	statusMsg.execute(_actionTarget);

	if (!statusMsg._success) {
		_currentStatus -= _statusInc;
	}

	return true;
}

bool CSTButton::EnterViewMsg(CEnterViewMsg *msg) {
	loadFrame(_buttonFrame);
	return true;
}

} // End of namespace Titanic
