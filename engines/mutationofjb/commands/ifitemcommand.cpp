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

#include "mutationofjb/commands/ifitemcommand.h"
#include "mutationofjb/gamedata.h"
#include "mutationofjb/script.h"
#include "mutationofjb/util.h"
#include "common/str.h"

/** @file
 * "IFITEM " <item> [ "!" ]
 *
 * IFITEM command tests whether an item is in the inventory.
 * If it is, execution continues to the next line.
 * Otherwise execution continues after first "#ELSE" or "=ELSE".
 * The logic can be reversed with exclamation mark at the end.
 *
 * Please note that this does not work like you are used to from saner languages.
 * IFITEM does not have any blocks. It only searches for first #ELSE, so you can have stuff like:
 *   IFITEM item1
 *   IFITEM item2
 *   #ELSE
 *   ...
 * This is effectively logical AND.
 */

namespace MutationOfJB {

bool IfItemCommandParser::parse(const Common::String &line, ScriptParseContext &parseContext, Command *&command) {
	if (line.size() < 8) {
		return false;
	}

	if (!line.hasPrefix("IFITEM")) {
		return false;
	}

	const bool negative = (line.lastChar() == '!');
	Common::String item(line.c_str() + 7);
	if (negative) {
		item.deleteLastChar(); // Remove '!'.
	}

	_tags.push(0);
	command = new IfItemCommand(item, negative);

	return true;
}


IfItemCommand::IfItemCommand(const Common::String &item, bool negative) :
	_item(item),
	_negative(negative) {}

Command::ExecuteResult IfItemCommand::execute(ScriptExecutionContext &scriptExecCtx) {
	_cachedResult = scriptExecCtx.getGameData()._inventory.hasItem(_item);
	if (_negative) {
		_cachedResult = !_cachedResult;
	}

	return Finished;
}

Common::String IfItemCommand::debugString() const {
	return Common::String::format("IFITEM %s%s", _negative ? "NOT " : "", _item.c_str());
}

}
