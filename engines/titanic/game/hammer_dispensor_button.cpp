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
#include "titanic/translation.h"

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
	_active(false), _open(false), _hitCounter(0), _btnPos(Point(56, 6)),
	_perch(nullptr), _hammerTaken(0) {
}

void CHammerDispensorButton::save(SimpleFile *file, int indent) {
	file->writeNumberLine(1, indent);
	file->writeNumberLine(_active, indent);
	file->writeNumberLine(_open, indent);
	file->writeNumberLine(_hitCounter, indent);
	file->writeNumberLine(_btnPos.x, indent);
	file->writeNumberLine(_btnPos.y, indent);
	file->writeNumberLine(_hammerTaken, indent);

	CStartAction::save(file, indent);
}

void CHammerDispensorButton::load(SimpleFile *file) {
	file->readNumber();
	_active = file->readNumber();
	_open = file->readNumber();
	_hitCounter = file->readNumber();
	_btnPos.x = file->readNumber();
	_btnPos.y = file->readNumber();
	_hammerTaken = file->readNumber();

	CStartAction::load(file);
}

bool CHammerDispensorButton::PuzzleSolvedMsg(CPuzzleSolvedMsg *msg) {
	_active = true;
	return true;
}

bool CHammerDispensorButton::MouseButtonUpMsg(CMouseButtonUpMsg *msg) {
	playSound(TRANSLATE("z#93.wav", "z#624.wav"));
	petDisplayMessage(1, POKE_WITH_LONG_STICK);
	return true;
}

bool CHammerDispensorButton::ActMsg(CActMsg *msg) {
	if (msg->_action == "HammerTaken")
		_hammerTaken = true;
	return true;
}

bool CHammerDispensorButton::FrameMsg(CFrameMsg *msg) {
	if (!_active)
		return true;

	if (!_perch) {
		CGameObject *obj = getDraggingObject();
		if (obj) {
			if (obj->isEquals("Perch") && getView() == findView())
				_perch = static_cast<CPerch *>(obj);
		}
	}

	if (_perch) {
		Point pt(_btnPos.x + _perch->_bounds.left, _btnPos.y + _perch->_bounds.top);
		bool flag = checkPoint(pt, true);

		if (!_open) {
			if (flag) {
				playSound(TRANSLATE("z#93.wav", "z#624.wav"));
				if (++_hitCounter == 5) {
					if (!_hammerTaken) {
						CActMsg actMsg(_msgAction);
						actMsg.execute(_msgTarget);
					}

					setVisible(false);
					_active = false;
					_hitCounter = 0;
				}

				_open = true;
			}
		} else {
			if (!flag) {
				_open = false;
				++_hitCounter;
			}
		}
	}

	return true;
}

bool CHammerDispensorButton::LeaveViewMsg(CLeaveViewMsg *msg) {
	_perch = nullptr;
	_hitCounter = 0;
	_open = false;
	return true;
}

bool CHammerDispensorButton::EnterViewMsg(CEnterViewMsg *msg) {
	setVisible(true);
	_active = true;
	return true;
}

} // End of namespace Titanic
