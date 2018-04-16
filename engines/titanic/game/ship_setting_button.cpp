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

#include "titanic/game/ship_setting_button.h"

namespace Titanic {

BEGIN_MESSAGE_MAP(CShipSettingButton, CGameObject)
	ON_MESSAGE(TurnOn)
	ON_MESSAGE(TurnOff)
	ON_MESSAGE(MouseButtonDownMsg)
	ON_MESSAGE(EnterViewMsg)
END_MESSAGE_MAP()

CShipSettingButton::CShipSettingButton() : CGameObject(), _pressed(false), _enabled(false) {
}

void CShipSettingButton::save(SimpleFile *file, int indent) {
	file->writeNumberLine(1, indent);
	file->writeQuotedLine(_target, indent);
	file->writeNumberLine(_pressed, indent);
	file->writeNumberLine(_enabled, indent);

	CGameObject::save(file, indent);
}

void CShipSettingButton::load(SimpleFile *file) {
	file->readNumber();
	_target = file->readString();
	_pressed = file->readNumber();
	_enabled = file->readNumber();

	CGameObject::load(file);
}

bool CShipSettingButton::TurnOn(CTurnOn *msg) {
	_pressed = true;
	return true;
}

bool CShipSettingButton::TurnOff(CTurnOff *msg) {
	_pressed = false;
	return true;
}

bool CShipSettingButton::MouseButtonDownMsg(CMouseButtonDownMsg *msg) {
	if (_pressed) {
		if (_enabled)
			playMovie(8, 16, 0);
		else
			playMovie(0, 8, 0);

		_enabled = !_enabled;
		CActMsg actMsg(_enabled ? "EnableObject" : "DisableObject");
		actMsg.execute(_target);
	} else {
		if (_enabled) {
			playMovie(8, 16, 0);
			playMovie(0, 8, 0);
		} else {
			playMovie(0, 16, 0);
		}
	}

	return true;
}

bool CShipSettingButton::EnterViewMsg(CEnterViewMsg *msg) {
	loadFrame(_enabled ? 8 : 16);
	return true;
}

} // End of namespace Titanic
