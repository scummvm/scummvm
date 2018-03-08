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

#include "mutationofjb/debug.h"
#include "mutationofjb/mutationofjb.h"
#include "mutationofjb/script.h"
#include "mutationofjb/commands/command.h"
#include "mutationofjb/commands/seqcommand.h"
#include "mutationofjb/commands/conditionalcommand.h"
#include "common/debug-channels.h"
#include "common/translation.h"
#include "common/scummsys.h"

namespace MutationOfJB {

/*
TODO
static Common::String convertTo7bitASCII() {
	return Common::String();
}
*/

Console::Console(MutationOfJBEngine *vm) : _vm(vm) {
	registerCmd("listsections", WRAP_METHOD(Console, cmd_listsections));
	registerCmd("showsection", WRAP_METHOD(Console, cmd_showsection));
}

bool Console::cmd_listsections(int argc, const char **argv) {
	if (argc == 3) {
		Script *script = nullptr;
		if (strcmp(argv[1], "G") == 0) {
			script = _vm->getGlobalScript();
		} else if (strcmp(argv[1], "L") == 0) {
			script = _vm->getLocalScript();
		}
		if (!script) {
			debugPrintf(_("Choose 'G' (global) or 'L' (local) script.\n"));
		} else {
			if (strcmp(argv[2], "L") == 0) {
				const ActionInfos &actionInfos = script->getLookActionInfos();
				for (ActionInfos::const_iterator it = actionInfos.begin(); it != actionInfos.end(); ++it) {
					const ActionInfo &actionInfo = *it;
					debugPrintf(_("Look %s\n"), actionInfo._object1Name.c_str());
				}
			} else if (strcmp(argv[2], "W") == 0) {
				const ActionInfos &actionInfos = script->getWalkActionInfos();
				for (ActionInfos::const_iterator it = actionInfos.begin(); it != actionInfos.end(); ++it) {
					const ActionInfo &actionInfo = *it;
					debugPrintf(_("Walk %s\n"), actionInfo._object1Name.c_str());
				}
			} else if (strcmp(argv[2], "T") == 0) {
				const ActionInfos &actionInfos = script->getTalkActionInfos();
				for (ActionInfos::const_iterator it = actionInfos.begin(); it != actionInfos.end(); ++it) {
					const ActionInfo &actionInfo = *it;
					debugPrintf(_("Talk %s\n"), actionInfo._object1Name.c_str());
				}
			} else if (strcmp(argv[2], "U") == 0) {
				const ActionInfos &actionInfos = script->getUseActionInfos();
				for (ActionInfos::const_iterator it = actionInfos.begin(); it != actionInfos.end(); ++it) {
					const ActionInfo &actionInfo = *it;
					debugPrintf(_("Use %s\n"), actionInfo._object1Name.c_str());
				}
			} else {
				debugPrintf(_("Choose 'L' (look), 'W' (walk), 'T' (talk) or 'U' (use).\n"));
			}
		}
	} else {
		debugPrintf(_("listsections <G|L> <L|W|T|U>\n"));
	}
	return true;
}

void Console::showIndent(int indentLevel) {
	for (int i = 0; i < indentLevel; ++i) {
		debugPrintf("  ");
	}
}

void Console::showCommands(Command *command, int indentLevel) {
	while (command) {
		showIndent(indentLevel);
		debugPrintf("%s\n", command->debugString().c_str());

		if (SeqCommand *const seqCmd = dynamic_cast<SeqCommand *>(command)) {
			command = seqCmd->next();
		} else if (ConditionalCommand *const condCmd = dynamic_cast<ConditionalCommand *>(command)) {
			showCommands(condCmd->getTrueCommand(), indentLevel + 1);
			showIndent(indentLevel);
			debugPrintf("ELSE\n");
			showCommands(condCmd->getFalseCommand(), indentLevel + 1);
			command = nullptr;
		} else {
			command = nullptr;
		}
	}
}

bool Console::cmd_showsection(int argc, const char **argv) {
	if (argc == 4) {
		Script *script = nullptr;
		if (strcmp(argv[1], "G") == 0) {
			script = _vm->getGlobalScript();
		} else if (strcmp(argv[1], "L") == 0) {
			script = _vm->getLocalScript();
		}
		if (!script) {
			debugPrintf(_("Choose 'G' (global) or 'L' (local) script.\n"));
		} else {
			Command *command = nullptr;
			bool found = false;
			if (strcmp(argv[2], "L") == 0) {
				const ActionInfos &actionInfos = script->getLookActionInfos();
				for (ActionInfos::const_iterator it = actionInfos.begin(); it != actionInfos.end(); ++it) {
					const ActionInfo &actionInfo = *it;
					if (actionInfo._object1Name == argv[3]) {
						found = true;
						command = actionInfo._command;
						break;
					}
				}
			} else if (strcmp(argv[2], "W") == 0) {
				const ActionInfos &actionInfos = script->getWalkActionInfos();
				for (ActionInfos::const_iterator it = actionInfos.begin(); it != actionInfos.end(); ++it) {
					const ActionInfo &actionInfo = *it;
					if (actionInfo._object1Name == argv[3]) {
						found = true;
						command = actionInfo._command;
						break;
					}
				}
			} else if (strcmp(argv[2], "T") == 0) {
				const ActionInfos &actionInfos = script->getTalkActionInfos();
				for (ActionInfos::const_iterator it = actionInfos.begin(); it != actionInfos.end(); ++it) {
					const ActionInfo &actionInfo = *it;
					if (actionInfo._object1Name == argv[3]) {
						found = true;
						command = actionInfo._command;
						break;
					}
				}
			} else if (strcmp(argv[2], "U") == 0) {
				const ActionInfos &actionInfos = script->getUseActionInfos();
				for (ActionInfos::const_iterator it = actionInfos.begin(); it != actionInfos.end(); ++it) {
					const ActionInfo &actionInfo = *it;
					if (actionInfo._object1Name == argv[3]) {
						found = true;
						command = actionInfo._command;
						break;
					}
				}
			} else {
				debugPrintf(_("Choose 'L' (look), 'W' (walk), 'T' (talk) or 'U' (use).\n"));
			}

			if (found) {
				if (command) {
					showCommands(command);
				}
			} else {
				debugPrintf("Section not found.\n");
			}
		}
	} else {
		debugPrintf(_("showsection <G|L> <L|W|T|U> <sectionname>\n"));
	}

	return true;
}

}
