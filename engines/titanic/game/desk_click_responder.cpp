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

#include "titanic/game/desk_click_responder.h"
#include "titanic/translation.h"

namespace Titanic {

BEGIN_MESSAGE_MAP(CDeskClickResponder, CClickResponder)
	ON_MESSAGE(MouseButtonDownMsg)
	ON_MESSAGE(LoadSuccessMsg)
END_MESSAGE_MAP()

void CDeskClickResponder::save(SimpleFile *file, int indent) {
	file->writeNumberLine(1, indent);
	file->writeNumberLine(_fieldD4, indent);
	file->writeNumberLine(_ticks, indent);

	CClickResponder::save(file, indent);
}

void CDeskClickResponder::load(SimpleFile *file) {
	file->readNumber();
	_fieldD4 = file->readNumber();
	_ticks = file->readNumber();

	CClickResponder::load(file);
}

bool CDeskClickResponder::MouseButtonDownMsg(CMouseButtonDownMsg *msg) {
	_fieldD4 = (_fieldD4 + 1) % 3;
	if (_fieldD4)
		return CClickResponder::MouseButtonDownMsg(msg);

	uint ticks = getTicksCount();
	if (!_ticks || ticks > (_ticks + 4000)) {
		playSound(TRANSLATE("a#22.wav", "a#17.wav"));
		_ticks = ticks;
	}

	return true;
}

bool CDeskClickResponder::LoadSuccessMsg(CLoadSuccessMsg *msg) {
	_ticks = 0;
	return true;
}

} // End of namespace Titanic
