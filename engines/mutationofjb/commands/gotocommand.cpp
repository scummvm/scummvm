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

#include "mutationofjb/commands/gotocommand.h"
#include "mutationofjb/commands/labelcommand.h"
#include "mutationofjb/gamedata.h"
#include "mutationofjb/script.h"

/** @file
 * "GOTO " <label>
 *
 * Jumps to a label.
 */

namespace MutationOfJB {

bool GotoCommandParser::parse(const Common::String &line, ScriptParseContext &parseCtx, Command *&command) {
	if (line.size() < 6 || !line.hasPrefix("GOTO")) {
		return false;
	}

	Common::String label = line.c_str() + 5;
	GotoCommand *gotoCmd = new GotoCommand();

	if (parseCtx._labels.contains(label)) {
		// We already have the label, set it.
		gotoCmd->setLabelCommand(parseCtx._labels[label]);
	} else {
		// Label is after goto, add to pending list.
		parseCtx._pendingGotos[label].push_back(gotoCmd);
	}

	command = gotoCmd;

	return true;
}


void GotoCommand::setLabelCommand(LabelCommand *labelCmd) {
	_labelCommand = labelCmd;
}

Command::ExecuteResult GotoCommand::execute(ScriptExecutionContext &) {
	// Intentionally empty.

	return Finished;
}

Command *GotoCommand::next() const {
	return _labelCommand;
}

Common::String GotoCommand::debugString() const {
	if (!_labelCommand) {
		return "GOTO (null)";
	}

	return Common::String::format("GOTO %s", _labelCommand->getName().c_str());
}

}
