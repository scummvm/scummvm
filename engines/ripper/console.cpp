/* ScummVM - Graphic Adventure Engine
 *
 * ScummVM is the legal property of its developers, whose names
 * are too numerous to list here. Please refer to the COPYRIGHT
 * file distributed with this source distribution.
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program. If not, see <http://www.gnu.org/licenses/>.
 */

#include "ripper/console.h"

#include "common/debug.h"

#include "ripper/detection.h"
#include "ripper/milestones.h"
#include "ripper/ripper.h"

namespace Ripper {

namespace {

bool parseMilestoneId(const Common::String &text, uint &flag) {
	if (text.empty())
		return false;

	flag = 0;
	for (uint index = 0; index < text.size(); ++index) {
		if (text[index] < '0' || text[index] > '9')
			return false;
		flag = flag * 10 + text[index] - '0';
		if (flag >= Milestones::kFlagCount)
			return false;
	}
	return true;
}

} // End of anonymous namespace

Console::Console(RipperEngine *engine) : GUI::Debugger(), _engine(engine) {
	assert(_engine);
	registerCmd("MILESTONE", WRAP_METHOD(Console, cmdMilestones));
	registerCmd("MILESTONES", WRAP_METHOD(Console, cmdMilestones));
	registerCmd("PUZZLE_HELP", WRAP_METHOD(Console, cmdPuzzleHelp));
	debugC(1, kDebugGeneral,
		"Ripper: initialized debug console commands=3 commands=MILESTONE,MILESTONES,PUZZLE_HELP");
}

void Console::printMilestone(uint flag) {
	const Milestones *milestones = _engine->getMilestones();
	const Common::String &label = milestones->label(flag);
	debugPrintf("%3u  value=%u  domain=%-12s  label='%s'\n",
		flag, milestones->isSet(flag) ? 1 : 0, Milestones::domain(flag),
		label.empty() ? "<unnamed>" : label.c_str());
}

bool Console::cmdMilestones(int argc, const char **argv) {
	Milestones *milestones = _engine->getMilestones();
	if (argc == 1 || (argc == 2 && Common::String(argv[1]).equalsIgnoreCase("ACTIVE"))) {
		const bool activeOnly = argc == 2;
		uint count = 0;
		for (uint flag = 0; flag < Milestones::kFlagCount; ++flag) {
			if (activeOnly) {
				if (!milestones->isSet(flag))
					continue;
			} else if (milestones->label(flag).empty()) {
				continue;
			}
			printMilestone(flag);
			++count;
		}
		debugPrintf("%u %s milestone(s)\n", count,
			activeOnly ? "active" : "defined");
		debugC(2, kDebugMilestones,
			"Ripper: debugger listed milestones filter=%s count=%u",
			activeOnly ? "active" : "defined", count);
		return true;
	}

	if (argc == 2) {
		uint flag = 0;
		if (parseMilestoneId(argv[1], flag)) {
			printMilestone(flag);
			debugC(2, kDebugMilestones,
				"Ripper: debugger queried milestone flag=%u value=%d",
				flag, milestones->isSet(flag));
			return true;
		}
	}

	if (argc == 3 && Common::String(argv[1]).equalsIgnoreCase("TOGGLE")) {
		uint flag = 0;
		if (parseMilestoneId(argv[2], flag)) {
			const bool oldValue = milestones->isSet(flag);
			if (milestones->toggle(flag, "debugger-toggle")) {
				printMilestone(flag);
				debugC(1, kDebugMilestones,
					"Ripper: debugger toggled milestone flag=%u label='%s' oldValue=%d newValue=%d",
					flag, milestones->label(flag).c_str(), oldValue,
					milestones->isSet(flag));
				return true;
			}
		}
	}

	debugPrintf("Usage: MILESTONES [ACTIVE|<ID>|TOGGLE <ID>]\n");
	debugPrintf("ID must be between 0 and %u\n", Milestones::kFlagCount - 1);
	return true;
}

bool Console::cmdPuzzleHelp(int argc, const char **argv) {
	bool enabled = !_engine->isPuzzleHelpEnabled();
	if (argc == 2) {
		const Common::String argument(argv[1]);
		if (argument.equalsIgnoreCase("ON")) {
			enabled = true;
		} else if (argument.equalsIgnoreCase("OFF")) {
			enabled = false;
		} else {
			debugPrintf("Usage: PUZZLE_HELP [ON|OFF]\n");
			return true;
		}
	} else if (argc != 1) {
		debugPrintf("Usage: PUZZLE_HELP [ON|OFF]\n");
		return true;
	}

	_engine->setPuzzleHelpEnabled(enabled);
	debugPrintf("Puzzle help overlays %s\n", enabled ? "enabled" : "disabled");
	debugC(1, kDebugPuzzles,
		"Ripper: debugger puzzle-help overlays enabled=%d command=PUZZLE_HELP",
		enabled);
	return true;
}

} // End of namespace Ripper
