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

#include "titanic/core/background.h"

namespace Titanic {

BEGIN_MESSAGE_MAP(CBackground, CGameObject)
	ON_MESSAGE(StatusChangeMsg)
	ON_MESSAGE(SetFrameMsg)
	ON_MESSAGE(VisibleMsg)
END_MESSAGE_MAP()

CBackground::CBackground() : CGameObject(), _startFrame(0), _endFrame(0), _isBlocking(false) {
}

void CBackground::save(SimpleFile *file, int indent) {
	file->writeNumberLine(1, indent);
	file->writeNumberLine(_startFrame, indent);
	file->writeNumberLine(_endFrame, indent);
	file->writeQuotedLine(_string1, indent);
	file->writeQuotedLine(_string2, indent);
	file->writeNumberLine(_isBlocking, indent);

	CGameObject::save(file, indent);
}

void CBackground::load(SimpleFile *file) {
	file->readNumber();
	_startFrame = file->readNumber();
	_endFrame = file->readNumber();
	_string1 = file->readString();
	_string2 = file->readString();
	_isBlocking = file->readNumber();

	CGameObject::load(file);
}

bool CBackground::StatusChangeMsg(CStatusChangeMsg *msg) {
	setVisible(true);
	if (_isBlocking) {
		playMovie(_startFrame, _endFrame, MOVIE_WAIT_FOR_FINISH);
	} else {
		playMovie(_startFrame, _endFrame, 0);
	}
	return true;
}

bool CBackground::SetFrameMsg(CSetFrameMsg *msg) {
	loadFrame(msg->_frameNumber);
	return true;
}

bool CBackground::VisibleMsg(CVisibleMsg *msg) {
	setVisible(!_visible);
	return true;
}

} // End of namespace Titanic
