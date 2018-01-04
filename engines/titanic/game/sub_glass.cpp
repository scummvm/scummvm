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

#include "titanic/game/sub_glass.h"
#include "titanic/translation.h"

namespace Titanic {

BEGIN_MESSAGE_MAP(CSUBGlass, CGameObject)
	ON_MESSAGE(MouseButtonDownMsg)
	ON_MESSAGE(MouseButtonUpMsg)
	ON_MESSAGE(MovieEndMsg)
	ON_MESSAGE(SignalObject)
	ON_MESSAGE(LeaveViewMsg)
END_MESSAGE_MAP()

CSUBGlass::CSUBGlass() : _fieldBC(0), _startFrame(0), _endFrame(1), _signalStartFrame(0) {
}

void CSUBGlass::save(SimpleFile *file, int indent) {
	file->writeNumberLine(1, indent);
	file->writeNumberLine(_fieldBC, indent);
	file->writeNumberLine(_startFrame, indent);
	file->writeNumberLine(_endFrame, indent);
	file->writeNumberLine(_signalStartFrame, indent);
	file->writeNumberLine(_signalEndFrame, indent);
	file->writeQuotedLine(_target, indent);

	CGameObject::save(file, indent);
}

void CSUBGlass::load(SimpleFile *file) {
	file->readNumber();
	_fieldBC = file->readNumber();
	_startFrame = file->readNumber();
	_endFrame = file->readNumber();
	_signalStartFrame = file->readNumber();
	_signalEndFrame = file->readNumber();
	_target = file->readString();

	CGameObject::load(file);
}

bool CSUBGlass::MouseButtonDownMsg(CMouseButtonDownMsg *msg) {
	return true;
}

bool CSUBGlass::MouseButtonUpMsg(CMouseButtonUpMsg *msg) {
	if (!_fieldBC && _startFrame >= 0) {
		_fieldBC = true;
		playMovie(_startFrame, _endFrame, MOVIE_NOTIFY_OBJECT);
		playSound(TRANSLATE("z#30.wav", "z#561.wav"));
	}

	return true;
}

bool CSUBGlass::MovieEndMsg(CMovieEndMsg *msg) {
	if (msg->_endFrame == _endFrame) {
		_fieldBC = true;
		CSignalObject signalMsg(getName(), 1);
		signalMsg.execute(_target);
	}

	return true;
}

bool CSUBGlass::SignalObject(CSignalObject *msg) {
	if (msg->_numValue == 1) {
		setVisible(true);

		if (_signalStartFrame >= 0) {
			playMovie(_signalStartFrame, _signalEndFrame, MOVIE_WAIT_FOR_FINISH);
			playSound(TRANSLATE("z#30.wav", "z#561.wav"));
			_fieldBC = false;
		}
	}

	return true;
}

bool CSUBGlass::LeaveViewMsg(CLeaveViewMsg *msg) {
	_fieldBC = false;
	setVisible(true);
	loadFrame(0);
	return true;
}

} // End of namespace Titanic
