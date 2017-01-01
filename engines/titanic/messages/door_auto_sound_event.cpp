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

#include "titanic/messages/door_auto_sound_event.h"

namespace Titanic {

BEGIN_MESSAGE_MAP(CDoorAutoSoundEvent, CAutoSoundEvent)
	ON_MESSAGE(PreEnterNodeMsg)
	ON_MESSAGE(LeaveNodeMsg)
	ON_MESSAGE(TimerMsg)
END_MESSAGE_MAP()

void CDoorAutoSoundEvent::save(SimpleFile *file, int indent) {
	file->writeNumberLine(1, indent);
	file->writeQuotedLine(_string1, indent);
	file->writeQuotedLine(_string2, indent);
	file->writeNumberLine(_fieldDC, indent);
	file->writeNumberLine(_fieldE0, indent);

	CAutoSoundEvent::save(file, indent);
}

void CDoorAutoSoundEvent::load(SimpleFile *file) {
	file->readNumber();
	_string1 = file->readString();
	_string2 = file->readString();
	_fieldDC = file->readNumber();
	_fieldE0 = file->readNumber();

	CAutoSoundEvent::load(file);
}

bool CDoorAutoSoundEvent::PreEnterNodeMsg(CPreEnterNodeMsg *msg) {
	return true;
}

bool CDoorAutoSoundEvent::LeaveNodeMsg(CLeaveNodeMsg *msg) {
	return true;
}

bool CDoorAutoSoundEvent::TimerMsg(CTimerMsg *msg) {
	return true;
}

} // End of namespace Titanic
