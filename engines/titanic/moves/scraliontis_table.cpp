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

#include "titanic/moves/scraliontis_table.h"

namespace Titanic {

BEGIN_MESSAGE_MAP(CScraliontisTable, CRestaurantPanHandler)
	ON_MESSAGE(MouseMoveMsg)
	ON_MESSAGE(MouseButtonDownMsg)
	ON_MESSAGE(MaitreDDefeatedMsg)
END_MESSAGE_MAP()

CScraliontisTable::CScraliontisTable() : CRestaurantPanHandler(),
		_fieldE0(false), _counter(0), _ticks(0), _fieldEC(false) {
}

void CScraliontisTable::save(SimpleFile *file, int indent) {
	file->writeNumberLine(1, indent);
	file->writeNumberLine(_fieldE0, indent);
	file->writeNumberLine(_counter, indent);
	file->writeNumberLine(_ticks, indent);
	file->writeNumberLine(_fieldEC, indent);

	CRestaurantPanHandler::save(file, indent);
}

void CScraliontisTable::load(SimpleFile *file) {
	file->readNumber();
	_fieldE0 = file->readNumber();
	_counter = file->readNumber();
	_ticks = file->readNumber();
	_fieldEC = file->readNumber();

	CRestaurantPanHandler::load(file);
}

bool CScraliontisTable::MouseMoveMsg(CMouseMoveMsg *msg) {
	if (!_fieldEC && !_fieldE0) {
		if (++_counter > 20) {
			CTriggerNPCEvent triggerMsg;
			triggerMsg.execute("MaitreD");
			_fieldE0 = true;
		}
	}

	return true;
}

bool CScraliontisTable::MouseButtonDownMsg(CMouseButtonDownMsg *msg) {
	if (_fieldEC) {
		changeView(_destination, _armPickedUp ? _armDestination : _armlessDestination);
	}
	else if (!_ticks || (getTicksCount() - _ticks) >= 5000) {
		CTriggerNPCEvent triggerMsg(119);
		triggerMsg.execute("MaitreD");
		_ticks = getTicksCount();
	}

	return true;
}

bool CScraliontisTable::MaitreDDefeatedMsg(CMaitreDDefeatedMsg *msg) {
	_cursorId = CURSOR_MOVE_FORWARD;
	_fieldEC = true;
	return true;
}

} // End of namespace Titanic
