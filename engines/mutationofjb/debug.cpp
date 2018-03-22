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
#include "mutationofjb/game.h"
#include "mutationofjb/mutationofjb.h"
#include "mutationofjb/script.h"
#include "mutationofjb/commands/command.h"
#include "mutationofjb/commands/seqcommand.h"
#include "mutationofjb/commands/conditionalcommand.h"
#include "mutationofjb/commands/callmacrocommand.h"
#include "common/debug-channels.h"
#include "common/translation.h"
#include "common/scummsys.h"

namespace MutationOfJB {

/* Converts CP895 string to 7bit ASCII, so we can show it in the console. */
static Common::String convertToASCII(const Common::String &str) {
	static const char conversionTable[] = {
		'C', 'u', 'e', 'd', 'a', 'D', 'T', 'c', 'e', 'E', 'L', 'I', 'l', 'l', 'A', 'A', /* 0x80-0x8F */
		'E', 'z', 'Z', 'o', 'o', 'O', 'u', 'U', 'y', 'O', 'U', 'S', 'L', 'Y', 'R', 't', /* 0x90-0x9F */
		'a', 'i', 'o', 'u', 'n', 'N', 'U', 'O', 's', 'r', 'r', 'R'						/* 0xA0-0xAB */
	};

	Common::String ret = str;
	for (Common::String::iterator it = ret.begin(); it != ret.end(); ++it) {
		const byte cp895Byte = reinterpret_cast<const byte &>(*it);
		if (cp895Byte >= 0x80 && cp895Byte <= 0xAB) {
			*it = conversionTable[cp895Byte - 0x80];
		} else if (cp895Byte == 0xE1) { // ß
			*it = 's';
		}
	}
	return ret;
}

Console::Console(MutationOfJBEngine *vm) : _vm(vm) {
	registerCmd("listsections", WRAP_METHOD(Console, cmd_listsections));
	registerCmd("showsection", WRAP_METHOD(Console, cmd_showsection));
	registerCmd("listmacros", WRAP_METHOD(Console, cmd_listmacros));
	registerCmd("showmacro", WRAP_METHOD(Console, cmd_showmacro));
	registerCmd("changescene", WRAP_METHOD(Console, cmd_changescene));
}

bool Console::cmd_listsections(int argc, const char **argv) {
	if (argc == 3) {
		Script *script = nullptr;
		if (strcmp(argv[1], "G") == 0) {
			script = _vm->getGame().getGlobalScript();
		} else if (strcmp(argv[1], "L") == 0) {
			script = _vm->getGame().getLocalScript();
		}
		if (!script) {
			debugPrintf(_("Choose 'G' (global) or 'L' (local) script.\n"));
		} else {
			if (strcmp(argv[2], "L") == 0) {
				const ActionInfos &actionInfos = script->getLookActionInfos();
				for (ActionInfos::const_iterator it = actionInfos.begin(); it != actionInfos.end(); ++it) {
					const ActionInfo &actionInfo = *it;
					debugPrintf(_("Look %s\n"), convertToASCII(actionInfo._object1Name).c_str());
				}
			} else if (strcmp(argv[2], "W") == 0) {
				const ActionInfos &actionInfos = script->getWalkActionInfos();
				for (ActionInfos::const_iterator it = actionInfos.begin(); it != actionInfos.end(); ++it) {
					const ActionInfo &actionInfo = *it;
					debugPrintf(_("Walk %s\n"), convertToASCII(actionInfo._object1Name).c_str());
				}
			} else if (strcmp(argv[2], "T") == 0) {
				const ActionInfos &actionInfos = script->getTalkActionInfos();
				for (ActionInfos::const_iterator it = actionInfos.begin(); it != actionInfos.end(); ++it) {
					const ActionInfo &actionInfo = *it;
					debugPrintf(_("Talk %s\n"), convertToASCII(actionInfo._object1Name).c_str());
				}
			} else if (strcmp(argv[2], "U") == 0) {
				const ActionInfos &actionInfos = script->getUseActionInfos();
				for (ActionInfos::const_iterator it = actionInfos.begin(); it != actionInfos.end(); ++it) {
					const ActionInfo &actionInfo = *it;
					if (actionInfo._object2Name.empty()) {
						debugPrintf(_("Use %s\n"), convertToASCII(actionInfo._object1Name).c_str());
					} else {
						debugPrintf(_("Use %s %s\n"), convertToASCII(actionInfo._object1Name).c_str(), convertToASCII(actionInfo._object2Name).c_str());
					}
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
		debugPrintf("%s\n", convertToASCII(command->debugString()).c_str());

		if (SeqCommand *const seqCmd = dynamic_cast<SeqCommand *>(command)) {
			command = seqCmd->next();
		} else if (ConditionalCommand *const condCmd = dynamic_cast<ConditionalCommand *>(command)) {
			showCommands(condCmd->getTrueCommand(), indentLevel + 1);
			showIndent(indentLevel);
			debugPrintf("ELSE\n");
			showCommands(condCmd->getFalseCommand(), indentLevel + 1);
			command = nullptr;
		} else if (CallMacroCommand* const callMacroCmd = dynamic_cast<CallMacroCommand *>(command)) {
			command = callMacroCmd->getReturnCommand();
		} else {
			command = nullptr;
		}
	}
}

bool Console::cmd_showsection(int argc, const char **argv) {
	if (argc >= 4) {
		Script *script = nullptr;
		if (strcmp(argv[1], "G") == 0) {
			script = _vm->getGame().getGlobalScript();
		} else if (strcmp(argv[1], "L") == 0) {
			script = _vm->getGame().getLocalScript();
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
					if (convertToASCII(actionInfo._object1Name) == argv[3]) {
						found = true;
						command = actionInfo._command;
						break;
					}
				}
			} else if (strcmp(argv[2], "W") == 0) {
				const ActionInfos &actionInfos = script->getWalkActionInfos();
				for (ActionInfos::const_iterator it = actionInfos.begin(); it != actionInfos.end(); ++it) {
					const ActionInfo &actionInfo = *it;
					if (convertToASCII(actionInfo._object1Name) == argv[3]) {
						found = true;
						command = actionInfo._command;
						break;
					}
				}
			} else if (strcmp(argv[2], "T") == 0) {
				const ActionInfos &actionInfos = script->getTalkActionInfos();
				for (ActionInfos::const_iterator it = actionInfos.begin(); it != actionInfos.end(); ++it) {
					const ActionInfo &actionInfo = *it;
					if (convertToASCII(actionInfo._object1Name) == argv[3]) {
						found = true;
						command = actionInfo._command;
						break;
					}
				}
			} else if (strcmp(argv[2], "U") == 0) {
				const ActionInfos &actionInfos = script->getUseActionInfos();
				for (ActionInfos::const_iterator it = actionInfos.begin(); it != actionInfos.end(); ++it) {
					const ActionInfo &actionInfo = *it;
					if (convertToASCII(actionInfo._object1Name) == argv[3] && ((argc == 4 && actionInfo._object2Name.empty()) || (argc > 4 && convertToASCII(actionInfo._object2Name) == argv[4]))) {
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

bool Console::cmd_listmacros(int argc, const char **argv) {
	if (argc == 2) {
		Script *script = nullptr;
		if (strcmp(argv[1], "G") == 0) {
			script = _vm->getGame().getGlobalScript();
		} else if (strcmp(argv[1], "L") == 0) {
			script = _vm->getGame().getLocalScript();
		}
		if (!script) {
			debugPrintf(_("Choose 'G' (global) or 'L' (local) script.\n"));
		} else {
			const Macros &macros = script->getMacros();
			for (Macros::const_iterator it = macros.begin(); it != macros.end(); ++it) {
				debugPrintf("%s\n", it->_key.c_str());
			}
		}
	} else {
		debugPrintf(_("listmacros <G|L>\n"));
	}

	return true;
}

bool Console::cmd_showmacro(int argc, const char **argv) {
	if (argc == 3) {
		Script *script = nullptr;
		if (strcmp(argv[1], "G") == 0) {
			script = _vm->getGame().getGlobalScript();
		} else if (strcmp(argv[1], "L") == 0) {
			script = _vm->getGame().getLocalScript();
		}
		if (!script) {
			debugPrintf(_("Choose 'G' (global) or 'L' (local) script.\n"));
		} else {
			const Macros &macros = script->getMacros();
			Macros::const_iterator itMacro = macros.find(argv[2]);
			if (itMacro != macros.end()) {
				if (itMacro->_value) {
					showCommands(itMacro->_value);
				}
			} else {
				debugPrintf("Macro not found.\n");
			}
		}
	} else {
		debugPrintf(_("showmacro <G|L> <macroname>\n"));
	}

	return true;
}

bool Console::cmd_changescene(int argc, const char **argv) {
	if (argc == 2) {
		const uint8 sceneId = atoi(argv[1]);
		const bool partB = argv[1][strlen(argv[1]) - 1] == 'B';

		_vm->getGame().changeScene(sceneId, partB);
	} else {
		debugPrintf(_("changescene <scenename>\n"));
	}

	return true;
}

}
