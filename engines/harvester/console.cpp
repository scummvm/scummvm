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
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 *
 */

#include "harvester/console.h"

#include "common/algorithm.h"
#include "harvester/harvester.h"
#include "harvester/resources.h"

namespace Harvester {

namespace {

static const uint kMaxDebugActionChainSteps = 128;
static const uint kMaxDebugActionChainLines = 256;
static const uint kMaxDebugActionBranchDepth = 16;

static Script *getActiveStartupScript() {
	return g_engine ? g_engine->getScript() : nullptr;
}

static const CommandRecord *findCommandRecord(const Script &script, const Common::String &tag) {
	if (tag.empty())
		return nullptr;

	for (const CommandRecord &command : script.getCommands()) {
		if (command.triggerTag.equalsIgnoreCase(tag))
			return &command;
	}

	return nullptr;
}

static const ExecListRecord *findExecListRecord(const Script &script, const Common::String &listName) {
	if (listName.empty())
		return nullptr;

	for (const ExecListRecord &execList : script.getExecLists()) {
		if (execList.listName.equalsIgnoreCase(listName))
			return &execList;
	}

	return nullptr;
}

static bool appendStartupAudioCommandLabel(const CommandRecord &command, Common::String &label) {
	if (command.opcodeName.equalsIgnoreCase("START_WAV")) {
		label = Common::String::format("path='%s'", command.arg1.c_str());
		return true;
	}

	if (command.opcodeName.equalsIgnoreCase("START_SINGLE_WAV")) {
		label = Common::String::format("path='%s'", command.arg1.c_str());
		return true;
	}

	if (command.opcodeName.equalsIgnoreCase("LOAD_WAV")) {
		label = Common::String::format("path='%s' slot='%s'", command.arg1.c_str(), command.arg2.c_str());
		return true;
	}

	if (command.opcodeName.equalsIgnoreCase("PLAY_WAV")) {
		label = Common::String::format("slot='%s'", command.arg1.c_str());
		return true;
	}

	if (command.opcodeName.equalsIgnoreCase("DELETE_WAV")) {
		label = Common::String::format("slot='%s'", command.arg1.c_str());
		return true;
	}

	return false;
}

static Common::String makeIndent(uint depth) {
	Common::String indent;
	for (uint i = 0; i < depth; ++i)
		indent += "  ";

	return indent;
}

static void appendDebugActionLine(Common::Array<Common::String> &lines, bool &truncated,
		uint depth, const Common::String &line) {
	if (truncated)
		return;

	if (lines.size() >= kMaxDebugActionChainLines) {
		lines.push_back(makeIndent(depth) + "... command chain truncated ...");
		truncated = true;
		return;
	}

	lines.push_back(makeIndent(depth) + line);
}

static bool containsTagIgnoreCase(const Common::Array<Common::String> &tags, const Common::String &tag) {
	for (const Common::String &candidate : tags) {
		if (candidate.equalsIgnoreCase(tag))
			return true;
	}

	return false;
}

static Common::String buildCommandDetail(const CommandRecord &command) {
	if (command.opcodeName.equalsIgnoreCase("CHECK_FLAG")) {
		return Common::String::format("flag='%s' true='%s' false='%s'",
			command.arg1.c_str(), command.arg2.c_str(), command.arg3.c_str());
	}

	if (command.opcodeName.equalsIgnoreCase("CHECK_PERC")) {
		return Common::String::format("threshold='%s' success='%s' failure='%s'",
			command.arg1.c_str(), command.arg2.c_str(), command.arg3.c_str());
	}

	if (command.opcodeName.equalsIgnoreCase("EXEC_LIST"))
		return Common::String::format("list='%s' next='%s'", command.arg1.c_str(), command.arg4.c_str());

	Common::String audioDetail;
	if (appendStartupAudioCommandLabel(command, audioDetail))
		return audioDetail + Common::String::format(" next='%s'", command.arg4.c_str());

	if (!command.arg1.empty() || !command.arg2.empty() || !command.arg3.empty() || !command.arg4.empty()) {
		return Common::String::format("args=['%s','%s','%s','%s']",
			command.arg1.c_str(), command.arg2.c_str(), command.arg3.c_str(), command.arg4.c_str());
	}

	return Common::String();
}

static bool isTerminalDebugCommand(const CommandRecord &command) {
	return command.opcodeName.equalsIgnoreCase("START_DIALOG") ||
		command.opcodeName.equalsIgnoreCase("GOFLIC") ||
		command.opcodeName.equalsIgnoreCase("GODEATHFLIC") ||
		command.opcodeName.equalsIgnoreCase("SHOW_TEXT") ||
		command.opcodeName.equalsIgnoreCase("PC_GOTO_XZ") ||
		command.opcodeName.equalsIgnoreCase("CHANGE_LIGHTING");
}

static void appendCommandChain(Common::Array<Common::String> &lines, bool &truncated,
		const Script &script, const Common::String &tag, uint depth,
		Common::Array<Common::String> &activeTags, uint &remainingSteps) {
	if (truncated || tag.empty())
		return;

	if (remainingSteps == 0) {
		appendDebugActionLine(lines, truncated, depth,
			Common::String::format("tag '%s': step limit reached", tag.c_str()));
		return;
	}

	if (depth > kMaxDebugActionBranchDepth) {
		appendDebugActionLine(lines, truncated, depth,
			Common::String::format("tag '%s': branch depth limit reached", tag.c_str()));
		return;
	}

	if (containsTagIgnoreCase(activeTags, tag)) {
		appendDebugActionLine(lines, truncated, depth,
			Common::String::format("tag '%s': loop detected", tag.c_str()));
		return;
	}

	activeTags.push_back(tag);
	--remainingSteps;

	const CommandRecord *command = findCommandRecord(script, tag);
	if (!command) {
		appendDebugActionLine(lines, truncated, depth,
			Common::String::format("tag '%s': unresolved", tag.c_str()));
		activeTags.pop_back();
		return;
	}

	Common::String line = Common::String::format("tag '%s': %s", tag.c_str(), command->opcodeName.c_str());
	const Common::String detail = buildCommandDetail(*command);
	if (!detail.empty())
		line += " " + detail;
	if (isTerminalDebugCommand(*command))
		line += " [terminal]";
	appendDebugActionLine(lines, truncated, depth, line);

	if (command->opcodeName.equalsIgnoreCase("CHECK_FLAG")) {
		const bool flagValue = script.getFlagValue(command->arg1);
		const Common::String &nextTag = flagValue ? command->arg2 : command->arg3;
		appendDebugActionLine(lines, truncated, depth + 1,
			Common::String::format("resolved flag '%s'=%d -> '%s'",
				command->arg1.c_str(), flagValue, nextTag.c_str()));
		appendCommandChain(lines, truncated, script, nextTag, depth + 1, activeTags, remainingSteps);
		activeTags.pop_back();
		return;
	}

	if (command->opcodeName.equalsIgnoreCase("CHECK_PERC")) {
		if (!command->arg2.empty()) {
			appendDebugActionLine(lines, truncated, depth + 1,
				Common::String::format("success branch -> '%s'", command->arg2.c_str()));
			appendCommandChain(lines, truncated, script, command->arg2, depth + 2, activeTags, remainingSteps);
		}
		if (!command->arg3.empty()) {
			appendDebugActionLine(lines, truncated, depth + 1,
				Common::String::format("failure branch -> '%s'", command->arg3.c_str()));
			appendCommandChain(lines, truncated, script, command->arg3, depth + 2, activeTags, remainingSteps);
		}
		activeTags.pop_back();
		return;
	}

	if (command->opcodeName.equalsIgnoreCase("EXEC_LIST")) {
		const ExecListRecord *execList = findExecListRecord(script, command->arg1);
		if (!execList) {
			appendDebugActionLine(lines, truncated, depth + 1,
				Common::String::format("exec list '%s': unresolved", command->arg1.c_str()));
		} else if (execList->entries.empty()) {
			appendDebugActionLine(lines, truncated, depth + 1,
				Common::String::format("exec list '%s': empty", execList->listName.c_str()));
		} else {
			for (uint i = 0; i < execList->entries.size(); ++i) {
				const Common::String &entry = execList->entries[i];
				appendDebugActionLine(lines, truncated, depth + 1,
					Common::String::format("entry[%u] -> '%s'", i, entry.c_str()));
				appendCommandChain(lines, truncated, script, entry, depth + 2, activeTags, remainingSteps);
			}
		}

		if (!command->arg4.empty())
			appendCommandChain(lines, truncated, script, command->arg4, depth, activeTags, remainingSteps);
		activeTags.pop_back();
		return;
	}

	if (!isTerminalDebugCommand(*command) && !command->arg4.empty())
		appendCommandChain(lines, truncated, script, command->arg4, depth, activeTags, remainingSteps);

	activeTags.pop_back();
}

static void buildActionChainLines(const Script &script, const Common::String &tag,
		Common::Array<Common::String> &lines) {
	lines.clear();

	if (tag.empty()) {
		lines.push_back("  <no action tag>");
		return;
	}

	bool truncated = false;
	uint remainingSteps = kMaxDebugActionChainSteps;
	Common::Array<Common::String> activeTags;
	appendCommandChain(lines, truncated, script, tag, 1, activeTags, remainingSteps);
}

static void collectSortedRoomNames(const Script &script, Common::Array<Common::String> &roomNames) {
	roomNames.clear();

	for (const RoomRecord &room : script.getRooms()) {
		if (room.roomName.empty())
			continue;

		bool alreadyPresent = false;
		for (const Common::String &existingName : roomNames) {
			if (existingName.equalsIgnoreCase(room.roomName)) {
				alreadyPresent = true;
				break;
			}
		}
		if (!alreadyPresent)
			roomNames.push_back(room.roomName);
	}

	Common::sort(roomNames.begin(), roomNames.end(), [](const Common::String &lhs, const Common::String &rhs) {
		Common::String upperLhs = lhs;
		Common::String upperRhs = rhs;
		upperLhs.toUppercase();
		upperRhs.toUppercase();
		return upperLhs < upperRhs;
	});
}

static bool findRoomName(const Script &script, const Common::String &candidate, Common::String &roomName) {
	for (const RoomRecord &room : script.getRooms()) {
		if (!room.roomName.equalsIgnoreCase(candidate))
			continue;

		roomName = room.roomName;
		return true;
	}

	return false;
}

static void printDebugCommandUsage(Console &console) {
	console.debugPrintf("Usage: DEBUG_COMMAND <opcode> [arg1] [arg2] [arg3] [arg4]\n");
	console.debugPrintf("   or: DEBUG_COMMAND <opcode> [arg1] [arg2] [arg3] CHAIN <action_tag>\n");
}

static bool parseDebugCommandArgs(int argc, const char **argv, CommandRecord &command,
		Common::String &errorMessage) {
	command = CommandRecord();
	if (argc < 2) {
		errorMessage = "Missing opcode";
		return false;
	}

	command.opcodeName = argv[1];
	uint positionalArgIndex = 0;
	bool sawChain = false;
	for (int i = 2; i < argc; ++i) {
		const Common::String token = argv[i];
		if (token.equalsIgnoreCase("CHAIN")) {
			if (sawChain) {
				errorMessage = "CHAIN may only be specified once";
				return false;
			}
			if (i + 1 >= argc) {
				errorMessage = "CHAIN requires an action tag";
				return false;
			}
			if (i + 2 != argc) {
				errorMessage = "CHAIN must be the final option";
				return false;
			}
			command.arg4 = argv[i + 1];
			sawChain = true;
			break;
		}

		// Positional args map directly onto the startup command fields, with arg4 also
		// serving as the optional continuation tag when CHAIN is not used explicitly.
		switch (positionalArgIndex++) {
		case 0:
			command.arg1 = token;
			break;
		case 1:
			command.arg2 = token;
			break;
		case 2:
			command.arg3 = token;
			break;
		case 3:
			command.arg4 = token;
			break;
		default:
			errorMessage = "Too many command arguments";
			return false;
		}
	}

	return true;
}

} // End of anonymous namespace

Console::Console() : GUI::Debugger() {
	registerCmd("about", WRAP_METHOD(Console, Cmd_about));
	registerCmd("DEBUG_COMBAT", WRAP_METHOD(Console, Cmd_debugCombat));
	registerCmd("DEBUG_PATHFINDING", WRAP_METHOD(Console, Cmd_debugPathfinding));
	registerCmd("DEBUG_TIMERS", WRAP_METHOD(Console, Cmd_debugTimers));
	registerCmd("DEBUG_ACTIONS", WRAP_METHOD(Console, Cmd_debugActions));
	registerCmd("DEBUG_COMMAND", WRAP_METHOD(Console, Cmd_debugCommand));
	registerCmd("DEBUG_ROOM", WRAP_METHOD(Console, Cmd_debugRoom));
	registerCmd("GOTO_ROOM", WRAP_METHOD(Console, Cmd_gotoRoom));
}

Console::~Console() {
}

bool Console::Cmd_about(int argc, const char **argv) {
	debugPrintf("Harvester engine scaffold\n");
	return true;
}

bool Console::Cmd_debugCombat(int argc, const char **argv) {
	if (argc != 1) {
		debugPrintf("Usage: DEBUG_COMBAT\n");
		return true;
	}

	if (!g_engine) {
		debugPrintf("Harvester engine is not active\n");
		return true;
	}

	const bool enabled = g_engine->toggleCombatDebugEnabled();
	debugPrintf("Combat debug overlay %s\n", enabled ? "enabled" : "disabled");
	return true;
}

bool Console::Cmd_debugRoom(int argc, const char **argv) {
	if (argc != 1) {
		debugPrintf("Usage: DEBUG_ROOM\n");
		return true;
	}

	if (!g_engine) {
		debugPrintf("Harvester engine is not active\n");
		return true;
	}

	const bool enabled = g_engine->toggleRoomDebugEnabled();
	debugPrintf("Room debug overlay %s\n", enabled ? "enabled" : "disabled");
	return true;
}

bool Console::Cmd_debugPathfinding(int argc, const char **argv) {
	if (argc != 1) {
		debugPrintf("Usage: DEBUG_PATHFINDING\n");
		return true;
	}

	if (!g_engine) {
		debugPrintf("Harvester engine is not active\n");
		return true;
	}

	const bool enabled = g_engine->togglePathfindingDebugEnabled();
	debugPrintf("Pathfinding debug overlay %s\n", enabled ? "enabled" : "disabled");
	return true;
}

bool Console::Cmd_debugTimers(int argc, const char **argv) {
	if (argc != 1) {
		debugPrintf("Usage: DEBUG_TIMERS\n");
		return true;
	}

	if (!g_engine) {
		debugPrintf("Harvester engine is not active\n");
		return true;
	}

	const bool enabled = g_engine->toggleTimerDebugEnabled();
	debugPrintf("Timer debug overlay %s\n", enabled ? "enabled" : "disabled");
	return true;
}

bool Console::Cmd_debugActions(int argc, const char **argv) {
	if (argc != 1) {
		debugPrintf("Usage: DEBUG_ACTIONS\n");
		return true;
	}

	if (!g_engine) {
		debugPrintf("Harvester engine is not active\n");
		return true;
	}

	Script *script = g_engine->getScript();
	if (!script || !g_engine->hasCurrentSaveRoomState()) {
		debugPrintf("DEBUG_ACTIONS is only available while a room is active\n");
		return true;
	}

	ResourceManager *resources = g_engine->getResources();
	if (!resources) {
		debugPrintf("Harvester resources are not available\n");
		return true;
	}

	const SaveRoomState &roomState = g_engine->getCurrentSaveRoomState();
	RoomSetupState materializedState;
	if (!script->materializeRoomState(roomState.entranceName, roomState.roomName,
			materializedState, *resources)) {
		debugPrintf("Unable to materialize room state for '%s'\n", roomState.roomName.c_str());
		return true;
	}

	uint actionObjectCount = 0;
	for (const ObjectRecord &object : materializedState.roomObjects) {
		if (!object.actionTag.empty())
			++actionObjectCount;
	}

	debugPrintf("Room '%s' action tags on %u/%u objects\n",
		materializedState.roomName.c_str(), actionObjectCount, (uint)materializedState.roomObjects.size());
	if (actionObjectCount == 0) {
		debugPrintf("No room objects in '%s' have action tags\n", materializedState.roomName.c_str());
		return true;
	}

	for (const ObjectRecord &object : materializedState.roomObjects) {
		if (object.actionTag.empty())
			continue;

		const Common::String label = script->resolveObjectLabel(object);
		Common::String objectLine = Common::String::format(
			"Object '%s' owner='%s' visible=%d runtimeVisible=%d operatable=%d action='%s'",
			object.objectName.c_str(), object.currentOwnerOrRoom.c_str(),
			object.visible, object.runtimeVisible, object.operatable, object.actionTag.c_str());
		if (!label.empty())
			objectLine += Common::String::format(" label='%s'", label.c_str());
		debugPrintf("%s\n", objectLine.c_str());

		Common::Array<Common::String> chainLines;
		buildActionChainLines(*script, object.actionTag, chainLines);
		for (const Common::String &line : chainLines)
			debugPrintf("%s\n", line.c_str());
	}

	return true;
}

bool Console::Cmd_debugCommand(int argc, const char **argv) {
	if (!g_engine) {
		debugPrintf("Harvester engine is not active\n");
		return true;
	}

	if (!g_engine->getScript() || !g_engine->hasCurrentSaveRoomState()) {
		debugPrintf("DEBUG_COMMAND is only available while a room is active\n");
		return true;
	}

	CommandRecord command;
	Common::String errorMessage;
	if (!parseDebugCommandArgs(argc, argv, command, errorMessage)) {
		if (!errorMessage.empty())
			debugPrintf("%s\n", errorMessage.c_str());
		printDebugCommandUsage(*this);
		return true;
	}

	if (!g_engine->requestDebugCommand(command)) {
		debugPrintf("Unable to queue debug command '%s'\n", command.opcodeName.c_str());
		return true;
	}

	// The command only queues work here; the active room loop applies the interaction on its next tick.
	Common::String commandLine = Common::String::format("Queued debug command %s", command.opcodeName.c_str());
	const Common::String detail = buildCommandDetail(command);
	if (!detail.empty())
		commandLine += Common::String::format(" %s", detail.c_str());
	debugPrintf("%s\n", commandLine.c_str());
	return true;
}

bool Console::Cmd_gotoRoom(int argc, const char **argv) {
	if (argc > 2) {
		debugPrintf("Usage: GOTO_ROOM [room_name]\n");
		return true;
	}

	Script *script = getActiveStartupScript();
	if (!script) {
		debugPrintf("Harvester engine is not active\n");
		return true;
	}

	Common::Array<Common::String> roomNames;
	collectSortedRoomNames(*script, roomNames);
	if (roomNames.empty()) {
		debugPrintf("No rooms were found in HARVEST.SCR\n");
		return true;
	}

	if (argc == 1) {
		debugPrintf("Rooms from HARVEST.SCR (%u):\n", (uint)roomNames.size());
		for (const Common::String &roomName : roomNames)
			debugPrintf("  %s\n", roomName.c_str());
		return true;
	}

	if (!g_engine || !g_engine->hasCurrentSaveRoomState()) {
		debugPrintf("GOTO_ROOM is only available while a room is active\n");
		return true;
	}

	Common::String roomName;
	if (!findRoomName(*script, argv[1], roomName)) {
		debugPrintf("Unknown room target '%s'\n", argv[1]);
		return true;
	}

	if (!g_engine->requestDebugRoomChange(roomName)) {
		debugPrintf("Unable to queue room change to '%s'\n", roomName.c_str());
		return true;
	}

	debugPrintf("Queued room change to %s\n", roomName.c_str());
	return true;
}

} // End of namespace Harvester
