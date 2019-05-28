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

#include "mutationofjb/commands/labelcommand.h"
#include "mutationofjb/commands/gotocommand.h"
#include "mutationofjb/script.h"

/** @file
 * <label> ":"
 *
 * Creates a label.
 */

namespace MutationOfJB {

bool LabelCommandParser::parse(const Common::String &line, ScriptParseContext &parseCtx, Command *&command) {
	if (line.lastChar() != ':') {
		return false;
	}

	Common::String label = line;
	label.deleteLastChar();

	LabelCommand *labelCmd = new LabelCommand(label);
	if (!parseCtx._labels.contains(label)) {
		parseCtx._labels[label] = labelCmd;
	} else {
		warning("Label '%s' already exists", label.c_str());
	}

	if (parseCtx._pendingGotos.contains(label)) {
		GotoCommands &gotos = parseCtx._pendingGotos[label];
		for (GotoCommands::const_iterator it = gotos.begin(); it != gotos.end(); ++it) {
			(*it)->setLabelCommand(labelCmd);
		}
		gotos.clear();
	}

	command = labelCmd;
	return true;
}


const Common::String &LabelCommand::getName() const {
	return _name;
}

Command::ExecuteResult LabelCommand::execute(ScriptExecutionContext &) {
	// Intentionally empty.

	return Finished;
}

Common::String LabelCommand::debugString() const {
	return Common::String::format("LABEL %s", _name.c_str());
}

}
