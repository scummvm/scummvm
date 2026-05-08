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

#include "mediastation/actor.h"
#include "mediastation/actors/hotspot.h"
#include "mediastation/actors/movie.h"
#include "mediastation/actors/screen.h"
#include "mediastation/actors/sound.h"
#include "mediastation/actors/sprite.h"
#include "mediastation/actors/timer.h"
#include "mediastation/context.h"
#include "mediastation/debugger.h"
#include "mediastation/mediascript/function.h"
#include "mediastation/mediascript/scriptvalue.h"
#include "mediastation/mediastation.h"

namespace MediaStation {

Debugger::Debugger(MediaStationEngine *vm) : GUI::Debugger(), _vm(vm) {
	registerCmd("actors", WRAP_METHOD(Debugger, Cmd_ListActors));
	registerCmd("actor", WRAP_METHOD(Debugger, Cmd_PrintActor));
	registerCmd("functions", WRAP_METHOD(Debugger, Cmd_ListFunctions));
	registerCmd("function", WRAP_METHOD(Debugger, Cmd_DecompileFunction));
	registerCmd("document", WRAP_METHOD(Debugger, Cmd_GetDocumentInfo));
	registerCmd("branch", WRAP_METHOD(Debugger, Cmd_BranchToScreen));
	registerCmd("variables", WRAP_METHOD(Debugger, Cmd_ListVariables));
}

bool Debugger::Cmd_ListActors(int argc, const char **argv) {
	debugPrintf("Currently loaded actors\n");
	debugPrintf("+----+------------------------------+----------+-----+---+------+-----+-----+-----+-----+-----+-----+\n");
	debugPrintf("| id | name                         | type     | ctx |vis|active|  x  |  y  |  w  |  h  |  z  | stg |\n");
	debugPrintf("+----+------------------------------+----------+-----+---+------+-----+-----+-----+-----+-----+-----+\n");
	for (auto it = _vm->getImtGod()->_actors.begin(); it != _vm->getImtGod()->_actors.end(); ++it) {
		Actor *actor = it->_value;
		const Common::String actorName = _vm->formatActorName(actor, false);
		const char *activeStateText = actor->isActive() ? "yes" : "no";

		if (actor->isSpatialActor()) {
			SpatialEntity *spatialActor = static_cast<SpatialEntity *>(actor);
			const Common::Rect bbox = spatialActor->getBbox();
			const int width = bbox.width();
			const int height = bbox.height();
			const uint stageId = spatialActor->getParentStage() ? spatialActor->getParentStage()->id() : 0;

			debugPrintf("|%4u|%-30.30s|%-10.10s|%5u|%3s|%6.6s|%5d|%5d|%5d|%5d|%5d|%5u|\n",
				actor->id(), actorName.c_str(), actorTypeToStr(actor->type()), actor->contextId(),
				spatialActor->isVisible() ? "yes" : "no", activeStateText, bbox.left, bbox.top, width, height,
				spatialActor->zIndex(), stageId);
		} else {
			// Since we don't have a spatial actor, just fill all the spatial fields with blanks.
			debugPrintf("|%4u|%-30.30s|%-10.10s|%5u|%3s|%6.6s|%5s|%5s|%5s|%5s|%5s|%5s|\n",
				actor->id(), actorName.c_str(), actorTypeToStr(actor->type()), actor->contextId(),
				"", activeStateText, "", "", "", "", "", "");
		}
	}
	debugPrintf("\n");
	return true;
}

bool Debugger::Cmd_PrintActor(int argc, const char **argv) {
	if (argc != 2) {
		debugPrintf("Usage: %s <actorId>\n", argv[0]);
		return true;
	}

	// TODO: Also accept actor names.
	uint actorId = atoi(argv[1]);
	Actor *actor = _vm->getImtGod()->getActorById(actorId);
	if (!actor) {
		debugPrintf("<actor %u doesn't exist or not loaded>\n", actorId);
		return true;
	}

	debugPrintf("%s\n\n", actor->debugString().c_str());

	// Print script response information.
	const Common::HashMap<uint, Common::Array<ScriptResponse *> > &scriptResponses = actor->scriptResponses();
	if (!scriptResponses.empty()) {
		for (auto it = scriptResponses.begin(); it != scriptResponses.end(); ++it) {
			const Common::Array<ScriptResponse *> &responses = it->_value;
			for (const ScriptResponse *response : responses) {
				Common::String decompiledCode = response->decompile();
				debugPrintf("%s\n", decompiledCode.c_str());
			}
		}
	} else {
		debugPrintf("<no script responses>\n");
	}

	return true;
}

bool Debugger::Cmd_ListFunctions(int argc, const char **argv) {
	debugPrintf("Currently loaded script functions\n");
	debugPrintf("+------+------------------------------+-----+-------+\n");
	debugPrintf("| id   | name                         | ctx | bytes |\n");
	debugPrintf("+------+------------------------------+-----+-------+\n");
	for (auto it = _vm->getFunctionManager()->_functions.begin(); it != _vm->getFunctionManager()->_functions.end(); ++it) {
		ScriptFunction *function = it->_value;
		const Common::String functionName = _vm->formatFunctionName(function->_id, false);

		debugPrintf("|%6u|%-30.30s|%5u|%7u|\n", function->_id, functionName.c_str(), function->_contextId, function->bytecodeSize());
	}
	debugPrintf("\n");
	return true;
}

bool Debugger::Cmd_GetDocumentInfo(int argc, const char **argv) {
	Document *document = g_engine->getDocument();
	debugPrintf(
		"currentScreen: %s\n"
		"loadingScreen: %s\n"
		"loadingContext: %s\n"
		"entryScreen: %s\n"
		"entryStream: %u\n\n",
		g_engine->formatActorName(document->_currentScreenActorId, true).c_str(),
		g_engine->formatActorName(document->_loadingScreenActorId, true).c_str(),
		g_engine->formatActorName(document->_loadingContextId, true).c_str(),
		g_engine->formatActorName(document->_entryPointScreenId, true).c_str(),
		g_engine->getDocument()->_entryPointStreamId
	);
	return true;
}

bool Debugger::Cmd_BranchToScreen(int argc, const char **argv) {
	if (argc != 2) {
		debugPrintf("Usage: %s [screenId]\n", argv[0]);
		debugPrintf("  screenId - The numeric screen ID\n");
		return true;
	}

	uint contextId = atoi(argv[1]);
	_vm->getDocument()->branchToScreen(contextId, false);
	// Close the debugger immediately to execute the branch.
	return false;
}

bool Debugger::Cmd_DecompileFunction(int argc, const char **argv) {
	if (argc != 2 || atoi(argv[1]) <= 0) {
		debugPrintf("Usage: %s <functionId>\n", argv[0]);
		return true;
	}

	int functionId = atoi(argv[1]);
	ScriptFunction *function = _vm->getFunctionManager()->getFunctionById(functionId);
	if (function == nullptr) {
		debugPrintf("<function %d doens't exist or not loaded>\n", functionId);
		return true;
	}

	debugPrintf("%s\n", function->decompile().c_str());
	return true;
}

bool Debugger::Cmd_ListVariables(int argc, const char **argv) {
	debugPrintf("Currently loaded global variables\n");
	debugPrintf("+------+-----+------------------------------+------------+------------------------------+\n");
	debugPrintf("| id   | ctx | name                         | type       | value                        |\n");
	debugPrintf("+------+-----+------------------------------+------------+------------------------------+\n");
	for (auto contextIt = _vm->getImtGod()->_loadedContexts.begin(); contextIt != _vm->getImtGod()->_loadedContexts.end(); ++contextIt) {
		const Context *context = contextIt->_value;
		for (auto variableIt = context->_variables.begin(); variableIt != context->_variables.end(); ++variableIt) {
			const uint variableId = variableIt->_key;
			const ScriptValue *value = variableIt->_value;
			const Common::String variableName = _vm->formatVariableName(variableId, false);
			const char *typeName = scriptValueTypeToStr(value->getType());
			const Common::String valueText = value->getDebugString(false);
			debugPrintf("|%6u|%5u|%-30.30s|%-12.12s|%-30.30s|\n",
				variableId, context->_id, variableName.c_str(), typeName, valueText.c_str());
		}
	}
	debugPrintf("\n");
	return true;
}

} // End of namespace MediaStation
