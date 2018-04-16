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

#include "titanic/npcs/summon_bots.h"

namespace Titanic {

BEGIN_MESSAGE_MAP(CSummonBots, CRobotController)
	ON_MESSAGE(SummonBotQueryMsg)
	ON_MESSAGE(SummonBotMsg)
END_MESSAGE_MAP()

CSummonBots::CSummonBots() : CRobotController(), _string2("NULL"),
		_fieldC8(0), _fieldCC(0) {
}

void CSummonBots::save(SimpleFile *file, int indent) {
	file->writeNumberLine(1, indent);
	file->writeNumberLine(_fieldC8, indent);
	file->writeNumberLine(_fieldCC, indent);
	file->writeQuotedLine(_string2, indent);

	CRobotController::save(file, indent);
}

void CSummonBots::load(SimpleFile *file) {
	file->readNumber();
	_fieldC8 = file->readNumber();
	_fieldCC = file->readNumber();
	_string2 = file->readString();

	CRobotController::load(file);
}

bool CSummonBots::SummonBotQueryMsg(CSummonBotQueryMsg *msg) {
	if (msg->_npcName == "BellBot") {
		if (_fieldC8 && !petCheckNode(_string2))
			return true;
	} else if (msg->_npcName == "DoorBot") {
		if (_fieldCC && !petCheckNode(_string2))
			return true;
	}

	return false;
}

bool CSummonBots::SummonBotMsg(CSummonBotMsg *msg) {
	if (msg->_npcName == "BellBot") {
		if (!_fieldC8)
			return false;

		if (!petDismissBot("BellBot"))
			petOnSummonBot("Bellbot", msg->_value);
	} else if (msg->_npcName == "DoorBot") {
		if (!_fieldCC)
			return false;

		if (!petDismissBot("Doorbot"))
			petOnSummonBot("Doorbot", msg->_value);
	} else {
		return false;
	}

	return true;
}

} // End of namespace Titanic
