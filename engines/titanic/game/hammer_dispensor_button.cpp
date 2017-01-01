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

#include "titanic/game/hammer_dispensor_button.h"

namespace Titanic {

BEGIN_MESSAGE_MAP(CHammerDispensorButton, CStartAction)
	ON_MESSAGE(PuzzleSolvedMsg)
	ON_MESSAGE(MouseButtonUpMsg)
	ON_MESSAGE(ActMsg)
	ON_MESSAGE(FrameMsg)
	ON_MESSAGE(LeaveViewMsg)
	ON_MESSAGE(EnterViewMsg)
END_MESSAGE_MAP()

CHammerDispensorButton::CHammerDispensorButton() : CStartAction(),
	_fieldF8(0), _fieldFC(0), _field100(0), _btnPos(Point(56, 6)),
	_field10C(nullptr), _field110(0) {
}

void CHammerDispensorButton::save(SimpleFile *file, int indent) {
	file->writeNumberLine(1, indent);
	file->writeNumberLine(_fieldF8, indent);
	file->writeNumberLine(_fieldFC, indent);
	file->writeNumberLine(_field100, indent);
	file->writeNumberLine(_btnPos.x, indent);
	file->writeNumberLine(_btnPos.y, indent);
	file->writeNumberLine(_field110, indent);

	CStartAction::save(file, indent);
}

void CHammerDispensorButton::load(SimpleFile *file) {
	file->readNumber();
	_fieldF8 = file->readNumber();
	_fieldFC = file->readNumber();
	_field100 = file->readNumber();
	_btnPos.x = file->readNumber();
	_btnPos.y = file->readNumber();
	_field110 = file->readNumber();

	CStartAction::load(file);
}

bool CHammerDispensorButton::PuzzleSolvedMsg(CPuzzleSolvedMsg *msg) {
	_fieldF8 = 1;
	return true;
}

bool CHammerDispensorButton::MouseButtonUpMsg(CMouseButtonUpMsg *msg) {
	playSound("z#93.wav");
	petDisplayMessage(1, POKE_WITH_LONG_STICK);
	return true;
}

bool CHammerDispensorButton::ActMsg(CActMsg *msg) {
	if (msg->_action == "HammerTaken")
		_field110 = true;
	return true;
}

bool CHammerDispensorButton::FrameMsg(CFrameMsg *msg) {
	if (!_fieldF8)
		return true;

	if (!_field10C) {
		CGameObject *obj = getDraggingObject();
		if (obj) {
			if (obj->isEquals("Perch") && getView() == findView())
				_field10C = obj;
		}
	}

	if (_field10C) {
		Point pt(_btnPos.x + _bounds.left, _btnPos.y + _bounds.top);
		bool flag = checkPoint(pt, true);

		switch (_fieldFC) {
		case 0:
			if (flag) {
				playSound("z#93.wav");
				if (++_field100 == 5) {
					if (!_field110) {
						CActMsg actMsg(_msgAction);
						actMsg.execute(_msgTarget);
					}

					setVisible(false);
					_fieldF8 = 0;
					_field100 = 0;
				}

				_fieldFC = 1;
			}
			break;

		case 1:
			if (!flag) {
				_fieldFC = 0;
				++_field100;
			}
			break;

		default:
			break;
		}
	}

	return true;
}

bool CHammerDispensorButton::LeaveViewMsg(CLeaveViewMsg *msg) {
	_field10C = nullptr;
	_field100 = 0;
	_fieldFC = 0;
	return true;
}

bool CHammerDispensorButton::EnterViewMsg(CEnterViewMsg *msg) {
	setVisible(true);
	_fieldF8 = 1;
	return true;
}

} // End of namespace Titanic
