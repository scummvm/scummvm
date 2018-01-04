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

CAutoSoundEvent::CAutoSoundEvent() : CGameObject(), _counter(0), _mask(0xFFFFFF) {
}

void CAutoSoundEvent::save(SimpleFile *file, int indent) {
	file->writeNumberLine(1, indent);
	file->writeNumberLine(_counter, indent);
	file->writeNumberLine(_mask, indent);

	CGameObject::save(file, indent);
}

void CAutoSoundEvent::load(SimpleFile *file) {
	file->readNumber();
	_counter = file->readNumber();
	_mask  = file->readNumber();

	CGameObject::load(file);
}

bool CAutoSoundEvent::FrameMsg(CFrameMsg *msg) {
	if (_counter >= 0)
		_counter = (_counter + 1) & _mask;

	return true;
}

} // End of namespace Titanic
