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

#include "titanic/messages/auto_sound_event.h"

namespace Titanic {

BEGIN_MESSAGE_MAP(CAutoSoundEvent, CGameObject)
	ON_MESSAGE(FrameMsg)
END_MESSAGE_MAP()

CAutoSoundEvent::CAutoSoundEvent() : CGameObject(), _value1(0), _value2(0xFFFFFF) {
}

void CAutoSoundEvent::save(SimpleFile *file, int indent) {
	file->writeNumberLine(1, indent);
	file->writeNumberLine(_value1, indent);
	file->writeNumberLine(_value2, indent);

	CGameObject::save(file, indent);
}

void CAutoSoundEvent::load(SimpleFile *file) {
	file->readNumber();
	_value1 = file->readNumber();
	_value2  = file->readNumber();

	CGameObject::load(file);
}

bool CAutoSoundEvent::FrameMsg(CFrameMsg *msg) {
	if (_value1 >= 0)
		_value1 = (_value1 + 1) & _value2;

	return true;
}

} // End of namespace Titanic
